#include "garnet/utils.h"
#include "garnet/variant.h"

#include <memory>
#include <mruby/string.h>

namespace Garnet {

void registerMethods(mrb_state *mrb, RClass *garnet_class,const QMetaObject& object)
{
    for (int i = 0; i < object.methodCount(); ++i) {
        QMetaMethod method = object.method(i);

        if (method.access() == QMetaMethod::Public) {
            const char *method_name = method.name();

            auto method_impl = [](mrb_state *mrb, mrb_value self) -> mrb_value {
                mrb_value *argv = nullptr;
                int argc = 0;
                mrb_get_args(mrb, "*", &argv, &argc);

                const char *method_name = mrb_sym2name(mrb, mrb->c->ci->mid);

                std::shared_ptr<QObject> q_object = Object::fromValue(self);
                const QMetaObject *object = q_object->metaObject();

                QMetaMethod found_method;
                for (int i = 0; i < object->methodCount(); ++i) {
                    QMetaMethod method = object->method(i);
                    if (strcmp(method_name, method.name()) == 0) {
                        found_method = method;
                    }
                }

                if (!found_method.isValid()) {
                    mrb_raisef(mrb, E_NOMETHOD_ERROR,
                               "undefined method '%S' for %S",
                               mrb_str_new_cstr(mrb, method_name),
                               mrb_str_new_cstr(mrb, object->className()));
                }

                int object_argc = found_method.parameterCount();

                bool has_vlist = (object_argc > 0 && (found_method.parameterType(object_argc - 1) == qMetaTypeId<QVariantList>()));
                if (has_vlist) object_argc--;

                if (argc < object_argc || (argc > object_argc && !has_vlist)) {
                    mrb_raisef(mrb, E_ARGUMENT_ERROR,
                               "wrong number of arguments (%S for %S)",
                               mrb_fixnum_value(argc),
                               mrb_fixnum_value(object_argc));
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

                found_method.invoke(q_object.get(), QGenericReturnArgument(QMetaType::typeName(return_type), buffer),
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

                return variant.toValue(mrb);
            };

            mrb_define_method(mrb, garnet_class, method_name, method_impl, ARGS_NONE());

            qDebug() << "Register method: " << method_name;
        }
    }

    for (int i = 0; i < object.propertyCount(); ++i) {
        QMetaProperty property = object.property(i);
        const char *property_name = property.name();

        auto method_impl = [](mrb_state *mrb, mrb_value self) -> mrb_value {
            bool setter = false;
            QString method_name = mrb_sym2name(mrb, mrb->c->ci->mid);
            if (method_name.endsWith("=")) {
                setter = true;
                method_name = method_name.mid(0, method_name.size() - 1);
            }

            std::shared_ptr<QObject> q_object = Object::fromValue(self);
            const QMetaObject *object = q_object->metaObject();

            QMetaProperty found_property;
            for (int i = 0; i < object->propertyCount(); ++i) {
                QMetaProperty property = object->property(i);
                if (method_name == property.name()) {
                    found_property = property;
                }
            }

            if (!found_property.isValid()) {
                mrb_raisef(mrb, E_NOMETHOD_ERROR,
                           "undefined method '%S' for %S",
                           mrb_str_new_cstr(mrb, method_name.toUtf8().data()),
                           mrb_str_new_cstr(mrb, object->className()));
            }

            if (setter) {
                mrb_value value;
                mrb_get_args(mrb, "o", &value);
                found_property.write(q_object.get(), Variant(mrb, value));
            }

            return Variant(found_property.read(q_object.get())).toValue(mrb);
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

} // namespace Garnet
