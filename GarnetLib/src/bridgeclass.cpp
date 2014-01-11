#include "bridgeclass.h"
#include "garnet/value.h"
#include "bridgecall.h"
#include "garnet/engine.h"
#include <mruby/class.h>
#include <QMetaMethod>

namespace Garnet {

BridgeData::BridgeData(QObject *object, bool ownsObject) :
    object_(object), ownsObject_(ownsObject)
{
}

BridgeData::~BridgeData()
{
    if (ownsObject_ && object_ && object_->parent() == nullptr) {
        delete object_;
    }
}

QObject *BridgeData::getObject(mrb_value bridgeValue)
{
    if (DATA_TYPE(bridgeValue) != &dataType) {
        return nullptr;
    }
    auto data = static_cast<BridgeData *>(DATA_PTR(bridgeValue));
    return data->object_;
}

mrb_data_type BridgeData::dataType = {
    "GarnetBridge",
    [](mrb_state *, void *ptr) {
        delete static_cast<BridgeData *>(ptr);
    }
};

namespace {

template <class F>
void registerMethods(const QMetaObject *metaObject, const F &defineFunc)
{
    for (int i = 0; i < metaObject->methodCount(); ++i) {
        auto method = metaObject->method(i);
        if (method.access() == QMetaMethod::Public) {
            defineFunc(method.name(), &BridgeCall::methodFunc);
        }
    }
    for (int i = 0; i < metaObject->propertyCount(); ++i) {
        auto property = metaObject->property(i);
        QByteArray getterName = property.name();
        QByteArray setterName = getterName + "=";
        defineFunc(getterName, &BridgeCall::getterFunc);
        defineFunc(setterName, &BridgeCall::setterFunc);
    }
}

} // anonymous namespace

BridgeClass::BridgeClass(mrb_state *mrb) :
    mrb_(mrb)
{
    define();
}

void BridgeClass::define()
{
    auto mrb = mrb_;
    auto klass = mrb_define_class(mrb, "GarnetBridge", mrb->object_class);

    MRB_SET_INSTANCE_TT(klass, MRB_TT_DATA);

    auto initialize_impl = [](mrb_state *mrb, mrb_value self) {
        Q_UNUSED(mrb);

        DATA_TYPE(self) = &BridgeData::dataType;
        DATA_PTR(self) = new BridgeData(nullptr, false);
        return self;
    };

    mrb_define_method(mrb, klass, "initialize", initialize_impl, MRB_ARGS_REQ(1));

    klass_ = klass;
}

mrb_value BridgeClass::newFromObject(QObject *object, bool willOwnObject)
{
    auto mrb = mrb_;
    mrb_value value = mrb_obj_new(mrb, klass_, 0, nullptr);

    auto data = static_cast<BridgeData *>(DATA_PTR(value));
    *data = BridgeData(object, willOwnObject);

    registerMethods(object->metaObject(), [&](const QByteArray &name, mrb_func_t impl) {
        mrb_define_singleton_method(mrb, mrb_object(value), name, impl, ARGS_NONE());
    });

    return value;
}

StaticBridgeClassManager::StaticBridgeClassManager(mrb_state *mrb) :
    mrb_(mrb)
{
}

void StaticBridgeClassManager::define(const QMetaObject *metaObject)
{
    auto mrb = mrb_;
    auto klass = mrb_define_class(mrb, metaObject->className(), mrb->object_class);

    MRB_SET_INSTANCE_TT(klass, MRB_TT_DATA);

    auto initialize_impl = [](mrb_state *mrb, mrb_value self) {
        Q_UNUSED(mrb);

        DATA_TYPE(self) = &BridgeData::dataType;
        auto className = mrb_obj_classname(mrb, self);
        auto metaObject = Engine::findByMrb(mrb)->staticBridgeClassManager().metaClassHash_[className];
        if (metaObject) {
            auto object = BridgeCall(mrb, metaObject).callConstructor();
            DATA_PTR(self) = new BridgeData(object, true);
        } else {
            DATA_PTR(self) = new BridgeData(nullptr, false);
        }
        return self;
    };

    mrb_define_method(mrb, klass, "initialize", initialize_impl, MRB_ARGS_REQ(1));
    registerMethods(metaObject, [&](const QByteArray &name, mrb_func_t impl) {
        mrb_define_method(mrb, klass, name, impl, ARGS_NONE());
    });

    metaClassHash_[metaObject->className()] = metaObject;
}


} // namespace Garnet
