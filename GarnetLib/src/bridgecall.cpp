#include "bridgecall.h"
#include "garnet/value.h"
#include "garnet/variadicargument.h"
#include <QObject>
#include <QMetaObject>
#include <QMetaMethod>
#include <array>

namespace Garnet {

namespace {

QVariantList getVariantParams(mrb_state *mrb)
{
    mrb_value *argv = nullptr;
    int argc = 0;
    mrb_get_args(mrb, "*", &argv, &argc);

    QVariantList params;
    params.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        params << Value(mrb, argv[i]).toVariant();
    }
    return params;
}

template <class InvokeFunc>
bool tryInvoke(const QMetaMethod &method, const QVariantList &originalParams, const InvokeFunc &invokeFunc)
{
    // pack variadic args
    auto params = originalParams;
    bool hasVArg = method.parameterType(method.parameterCount() - 1) == qMetaTypeId<VariadicArgument>();
    if (hasVArg) {
        if (method.parameterCount() - 1 > originalParams.size())
            return false;
        auto vargs = originalParams.mid(method.parameterCount() - 1);
        params = originalParams.mid(0, method.parameterCount() - 1);
        params << QVariant::fromValue(VariadicArgument(vargs));
    } else {
        if (method.parameterCount() != params.size())
            return false;
    }

    // convert arguments
    int argc = params.size();
    std::array<int, 10> metaTypes;
    std::array<void *, 10> buffers;

    for (int i = 0; i < argc; ++i) {
        auto metaType = method.parameterType(i);
        metaTypes[i] = metaType;

        if (metaType == QMetaType::QVariant) {
            buffers[i] = QMetaType::create(metaType, &params[i]);
        } else {
            auto converted = params[i];
            if (!converted.convert(metaType)) {
                return false;
            }
            buffers[i] = QMetaType::create(metaType, converted.data());
        }
    }

    std::array<QGenericArgument, 10> args;
    for (int i = 0; i < argc; ++i) {
        args[i] = QGenericArgument(QMetaType::typeName(metaTypes[i]), buffers[i]);
    }

    auto result = invokeFunc(args);

    for (int i = 0; i < argc; ++i) {
        QMetaType::destroy(metaTypes[i], buffers[i]);
    }

    return result;
}


QObject *invokeConstructor(const QMetaObject *metaObject, const QMetaMethod &method, const QVariantList &params)
{
    QObject *object;
    auto result = tryInvoke(method, params,
                           [&](const std::array<QGenericArgument, 10> &args) {
        object = metaObject->newInstance(args[0],args[1],args[2],args[3],args[4],
                                         args[5],args[6],args[7],args[8],args[9]);
        return object;
    });
    return result ? object : nullptr;
}

bool tryInvokeMethod(QObject *object, const QMetaMethod &method, const QVariantList &params, QVariant *returnValue)
{
    return tryInvoke(method, params,
                    [&](const std::array<QGenericArgument, 10> &args) {

        int returnType = method.returnType();
        auto returnBuffer = QMetaType::create(returnType);
        bool result = method.invoke(object,
                                    QGenericReturnArgument(QMetaType::typeName(returnType), returnBuffer),
                                    args[0],args[1],args[2],args[3],args[4],
                                    args[5],args[6],args[7],args[8],args[9]);
        if (result) {
            *returnValue = QVariant(returnType, returnBuffer);
        }
        QMetaType::destroy(returnType, returnBuffer);
        return result;
    });
}

}

BridgeCall::BridgeCall(mrb_state *mrb, mrb_value self) :
    mrb_(mrb)
{
    object_ = Value(mrb, self).toQObject();
    methodName_ = mrb_sym2name(mrb, mrb->c->ci->mid);
    metaObject_ = object_->metaObject();
}

BridgeCall::BridgeCall(mrb_state *mrb, const QMetaObject *metaObject) :
    mrb_(mrb)
{
    object_ = nullptr;
    methodName_ = nullptr;
    metaObject_ = metaObject;
}

QObject *BridgeCall::callConstructor()
{
    auto mrb = mrb_;

    {
        auto params = getVariantParams(mrb);
        QObject *object = nullptr;

        for (int ctorIndex = 0; ctorIndex < metaObject_->constructorCount(); ++ctorIndex) {
            auto metaMethod = metaObject_->constructor(ctorIndex);
            if (metaMethod.access() != QMetaMethod::Public) {
                continue;
            }
            object = invokeConstructor(metaObject_, metaMethod, params);
            if (object) {
                break;
            }
        }

        if (object) {
            return object;
        }
    }

    mrb_raisef(mrb, E_NOMETHOD_ERROR,
               "no matching constructor for Qt class %S",
               mrb_str_new_cstr(mrb, metaObject_->className()));

    return nullptr;
}

mrb_value BridgeCall::callMethod()
{
    auto mrb = mrb_;

    {
        QVariantList params = getVariantParams(mrb);
        QVariant returnValue;
        bool invoked = false;

        for (int methodIndex = 0; methodIndex < metaObject_->methodCount(); ++methodIndex) {
            auto metaMethod = metaObject_->method(methodIndex);
            if (metaMethod.access() != QMetaMethod::Public) {
                continue;
            }
            if (std::strcmp(metaMethod.name(), methodName_) != 0) {
                continue;
            }
            if (tryInvokeMethod(object_, metaMethod, params, &returnValue)) {
                invoked = true;
                break;
            }
        }
        if (invoked) {
            return Value::fromVariant(mrb, returnValue).mrbValue();
        }
    }

    mrb_raisef(mrb, E_NOMETHOD_ERROR,
               "no matching method '%S' for Qt class %S",
               mrb_str_new_cstr(mrb, methodName_),
               mrb_str_new_cstr(mrb, metaObject_->className()));

    return mrb_nil_value();
}

mrb_value BridgeCall::accessProperty(bool setter)
{
    auto mrb = mrb_;
    bool noPropertyError = false;
    bool noSetterError = false;
    const char *propertyName = nullptr;

    do {
        auto property = accessedProperty(setter);
        propertyName = property.name();
        if (!property.isValid()) {
            noPropertyError = true;
            break;
        }
        if (setter) {
            if (!property.isWritable()) {
                noSetterError = true;
                break;
            }
            mrb_value value;
            mrb_get_args(mrb, "o", &value);
            property.write(object_, Value(mrb, value).toVariant());
        }
        return Value::mrbValueFromVariant(mrb, property.read(object_));
    }
    while (false);

    if (noPropertyError) {
        mrb_raisef(mrb, E_NOMETHOD_ERROR,
                   "undefined property '%S' for Qt class %S",
                   mrb_str_new_cstr(mrb, propertyName),
                   mrb_str_new_cstr(mrb, metaObject_->className()));
    }

    if (noSetterError) {
        mrb_raisef(mrb, E_NOMETHOD_ERROR,
                   "read-only property '%S' for Qt class %S",
                   mrb_str_new_cstr(mrb, propertyName),
                   mrb_str_new_cstr(mrb, metaObject_->className()));
    }

    return mrb_nil_value();
}

QMetaProperty BridgeCall::accessedProperty(bool setter)
{
    QByteArray property_name = methodName_;
    if (setter) {
        property_name = property_name.left(property_name.size() - 1);
    }
    return metaObject_->property(metaObject_->indexOfProperty(property_name.data()));
}

mrb_value BridgeCall::methodFunc(mrb_state *mrb, mrb_value self)
{
    return BridgeCall(mrb, self).callMethod();
}
mrb_value BridgeCall::setterFunc(mrb_state *mrb, mrb_value self)
{
    return BridgeCall(mrb, self).accessProperty(true);
}
mrb_value BridgeCall::getterFunc(mrb_state *mrb, mrb_value self)
{
    return BridgeCall(mrb, self).accessProperty(false);
}

} // namespace Garnet
