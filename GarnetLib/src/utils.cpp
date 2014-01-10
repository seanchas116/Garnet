#include "garnet/utils.h"
#include "garnet/variant.h"

#include <memory>
#include <mruby/string.h>

namespace Garnet {

namespace {

class MethodCall
{
public:

    enum class Mode
    {
        Static,
        Dynamic
    };

    MethodCall(mrb_state *mrb, mrb_value self, Mode mode) :
        mrb_(mrb),
        self_(self),
        mode_(mode)
    {
        if (mode == Mode::Static) {
            auto ptr = Object::fromValue(self);
            object_ = ptr.get();
            method_name_ = mrb_sym2name(mrb, mrb->c->ci->mid);
        } else {
            object_ = static_cast<QObject *>(DATA_PTR(self));
            mrb_sym method_sym;
            mrb_value *argv;
            int argc;
            mrb_get_args(mrb, "n*", &method_sym, &argv, &argc);
            method_name_ = mrb_sym2name(mrb, method_sym);
        }
        meta_object_ = object_->metaObject();
    }

    void callMethod(mrb_value *result)
    {
        if (!tryCallMethod(result)) {
            setNoMethodError();
        }
    }

    void accessProperty(mrb_value *result)
    {
        if (!tryAccessProperty(result)) {
            setNoMethodError();
        }
    }

    void callAutomatically(mrb_value *result)
    {
        if (!tryAccessProperty(result) && !tryCallMethod(result)) {
            setNoMethodError();
        }
    }

    QMetaMethod calledMethod()
    {
        for (int i = 0; i < meta_object_->methodCount(); ++i) {
            QMetaMethod method = meta_object_->method(i);
            if (strcmp(method_name_, method.name()) == 0) {
                return method;
            }
        }
        return QMetaMethod();
    }

    QMetaProperty accessedProperty(bool *is_setter)
    {
        QByteArray property_name = method_name_;
        if (property_name.endsWith("=")) {
            *is_setter = true;
            property_name = property_name.mid(0, property_name.size() - 1);
        } else {
            *is_setter = false;
        }
        return meta_object_->property(meta_object_->indexOfProperty(property_name.data()));
    }

    // this function may prevent destructors of local variables from not being called properly
    // because mrb_raisef uses longjmp
    static void handleError(mrb_state *mrb)
    {
        if (error.occured) {
            error.occured = false;
            mrb_raisef(mrb, error.klass, error.format, error.arg1, error.arg2);
        }
    }

private:

    bool tryCallMethod(mrb_value *result)
    {
        auto found_method = calledMethod();

        if (!found_method.isValid()) {
            return false;
        }

        auto mrb = mrb_;
        mrb_value *argv = nullptr;
        int argc = 0;

        if (mode_ == Mode::Static) {
            mrb_get_args(mrb, "*", &argv, &argc);
        }
        else {
            mrb_sym method_sym;
            mrb_get_args(mrb, "n*", &method_sym, &argv, &argc);
        }

        int object_argc = found_method.parameterCount();

        bool has_vlist = (object_argc > 0 && (found_method.parameterType(object_argc - 1) == qMetaTypeId<QVariantList>()));
        if (has_vlist) object_argc--;

        if (argc < object_argc || (argc > object_argc && !has_vlist)) {
             setWrongNumberOfArgumentsError(argc, object_argc);
             return true;
        }

        QVariant variants[10];
        QGenericArgument gargs[10];
        QVariantList vlist;

        for (int i = 0; i < argc; ++i) {
            variants[i] = Variant(mrb, argv[i]);
            if (has_vlist && i >= object_argc) {
                vlist.push_back(variants[i]);
            }
            else {
                gargs[i] = Q_ARG(QVariant, variants[i]);
            }
        }

        if (has_vlist) {
            gargs[object_argc] = Q_ARG(QVariantList, vlist);
        }

        int return_type = found_method.returnType();

        void *buffer = nullptr;
        if (return_type != qMetaTypeId<void>()) {
            buffer = QMetaType::create(return_type);
        }

        found_method.invoke(object_, QGenericReturnArgument(QMetaType::typeName(return_type), buffer),
                            gargs[0], gargs[1], gargs[2], gargs[3], gargs[4],
                            gargs[5], gargs[6], gargs[7], gargs[8], gargs[9]);

        Variant variant;
        if (return_type == qMetaTypeId<QVariant>()) {
            variant = *reinterpret_cast<const QVariant*>(buffer);
        }
        else if (return_type != qMetaTypeId<void>()) {
            variant = Variant(QVariant(return_type, buffer));
        }

        QMetaType::destroy(return_type, buffer);

        *result = variant.toValue(mrb);
        return true;
    }

    bool tryAccessProperty(mrb_value *result)
    {
        bool setter;
        auto found_property = accessedProperty(&setter);

        if (!found_property.isValid()) {
            return false;
        }

        auto mrb = mrb_;

        if (setter) {
            mrb_value value;
            if (mode_ == Mode::Static) {
                mrb_get_args(mrb, "o", &value);
            } else {
                mrb_sym method_sym;
                mrb_get_args(mrb, "no", &method_sym, &value);
            }
            found_property.write(object_, Variant(mrb, value));
        }

        *result = Variant(found_property.read(object_)).toValue(mrb);
        return true;
    }

    void setNoMethodError()
    {
        auto mrb = mrb_;
        error.occured = true;
        error.klass = E_NOMETHOD_ERROR;
        error.format = "undefined method '%S' for %S";
        error.arg1 = mrb_str_new_cstr(mrb, method_name_);
        error.arg2 = mrb_str_new_cstr(mrb, meta_object_->className());
    }

    void setWrongNumberOfArgumentsError(int wrong, int correct)
    {
        auto mrb = mrb_;
        error.occured = true;
        error.klass = E_ARGUMENT_ERROR;
        error.format = "wrong number of arguments (%S for %S)";
        error.arg1 = mrb_fixnum_value(wrong);
        error.arg2 = mrb_fixnum_value(correct);
    }

    mrb_state *mrb_;
    mrb_value self_;
    Mode mode_;
    const char *method_name_;
    const QMetaObject *meta_object_;
    QObject *object_;

    struct Error {
        bool occured = false;
        RClass *klass;
        const char *format;
        mrb_value arg1, arg2;
    };
    static Error error;
};

MethodCall::Error MethodCall::error;

} // anonymous namespace

void registerMethods(mrb_state *mrb, RClass *garnet_class,const QMetaObject& object)
{
    for (int i = 0; i < object.methodCount(); ++i) {
        QMetaMethod method = object.method(i);

        if (method.access() == QMetaMethod::Public) {
            const char *method_name = method.name();

            auto method_impl = [](mrb_state *mrb, mrb_value self) -> mrb_value {
                mrb_value result;
                MethodCall(mrb, self, MethodCall::Mode::Static).callMethod(&result);
                MethodCall::handleError(mrb);
                return result;
            };

            mrb_define_method(mrb, garnet_class, method_name, method_impl, ARGS_NONE());

            qDebug() << "Register method: " << method_name;
        }
    }

    for (int i = 0; i < object.propertyCount(); ++i) {
        QMetaProperty property = object.property(i);
        const char *property_name = property.name();

        auto method_impl = [](mrb_state *mrb, mrb_value self) -> mrb_value {
            mrb_value result;
            MethodCall(mrb, self, MethodCall::Mode::Static).accessProperty(&result);
            MethodCall::handleError(mrb);
            return result;
        };

        QString getter_method_name = property_name;
        QString setter_method_name = getter_method_name + "=";

        mrb_define_method(mrb, garnet_class, getter_method_name.toUtf8().data(), method_impl, ARGS_NONE());
        mrb_define_method(mrb, garnet_class, setter_method_name.toUtf8().data(), method_impl, ARGS_NONE());

        qDebug() << "Register property: " << property_name;
    }
}

void initialize()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // int //
    Variant::registerConverter<int>([](mrb_state* mrb, const QVariant& variant) -> mrb_value {
        Q_UNUSED(mrb)
        return mrb_fixnum_value(variant.toInt());
    });

    Variant::registerConverter([](mrb_state* mrb, const mrb_value& value, QVariant *variant) -> bool {
        Q_UNUSED(mrb)
        if (mrb_type(value) == MRB_TT_FIXNUM) {
            *variant = mrb_fixnum(value);
            return true;
        }
        else {
            return false;
        }
    });

    // double //
    Variant::registerConverter<double>([](mrb_state* mrb, const QVariant& variant) -> mrb_value {
        return mrb_float_value(mrb, variant.toDouble());
    });

    Variant::registerConverter([](mrb_state* mrb, const mrb_value& value, QVariant *variant) -> bool {
        Q_UNUSED(mrb)
        if (mrb_float_p(value)) {
            *variant = static_cast<double>(mrb_float(value));
            return true;
        }
        else {
            return false;
        }
    });

    // QString //
    Variant::registerConverter<QString>([](mrb_state* mrb, const QVariant& variant) -> mrb_value {
        QByteArray utf8 = variant.toString().toUtf8();
        return mrb_str_new(mrb, utf8.data(), utf8.size());
    });

    Variant::registerConverter([](mrb_state* mrb, const mrb_value& value, QVariant *variant) -> bool {
        Q_UNUSED(mrb)
        if (mrb_string_p(value)) {
            *variant = QString::fromUtf8(RSTRING_PTR(value), RSTRING_LEN(value));
            return true;
        }
        else {
            return false;
        }
    });

}

namespace {

QHash<QByteArray, QObject*> dynamicObjectHash;
RClass *dynamicClass;

void initializeDynamicClass(mrb_state *mrb)
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    static mrb_data_type data_type = {"GarnetDynamicObject", [](mrb_state *, void *){} };

    auto dynamic_class = mrb_define_class(mrb, "GarnetDynamicObject", mrb->object_class);
    MRB_SET_INSTANCE_TT(dynamic_class, MRB_TT_DATA);

    auto initialize_impl = [](mrb_state *mrb, mrb_value self) {
        char *name;
        mrb_get_args(mrb, "z", &name);
        DATA_TYPE(self) = &data_type;
        DATA_PTR(self) = dynamicObjectHash[name];
        return self;
    };
    auto method_missing_impl = [](mrb_state *mrb, mrb_value self) {
        mrb_value result;
        MethodCall(mrb, self, MethodCall::Mode::Dynamic).callAutomatically(&result);
        MethodCall::handleError(mrb);
        return result;
    };
    auto respond_to_missing_p_impl = [](mrb_state *mrb, mrb_value self) {
        bool result;
        {
            MethodCall call(mrb, self, MethodCall::Mode::Dynamic);
            bool setter;
            result = call.accessedProperty(&setter).isValid() && call.calledMethod().isValid();
        }
        MethodCall::handleError(mrb);
        return mrb_bool_value(result);
    };

    mrb_define_method(mrb, dynamic_class, "initialize", initialize_impl, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, dynamic_class, "method_missing", method_missing_impl, MRB_ARGS_ANY());
    mrb_define_method(mrb, dynamic_class, "respond_to_missing?", respond_to_missing_p_impl, MRB_ARGS_REQ(2));
    dynamicClass = dynamic_class;
}

} // anonymous namespace

void registerDynamicObject(mrb_state *mrb, const QByteArray &name, QObject *object)
{
    initializeDynamicClass(mrb);
    dynamicObjectHash[name] = object;
    auto method_impl = [](mrb_state *mrb, mrb_value self) {
        Q_UNUSED(self);
        auto name = mrb_sym2str(mrb, mrb->c->ci->mid);
        return mrb_obj_new(mrb, dynamicClass, 1, &name);
    };
    mrb_define_method(mrb, mrb->object_class, name.data(), method_impl, ARGS_NONE());
}

} // namespace Garnet
