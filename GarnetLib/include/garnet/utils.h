#ifndef GARNET_UTILS_H
#define GARNET_UTILS_H

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>

#include <QString>
#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>

#include "object.h"

namespace Garnet {

void initialize();
void registerMethods(mrb_state *mrb, RClass *garnet_class, const QMetaObject& object);

namespace detail {

template <class T>
RClass* createClass(mrb_state *mrb)
{
    QMetaObject object = T::staticMetaObject;
    const char *class_name = object.className();

    RClass *garnet_class = mrb_define_class(mrb, class_name, mrb->object_class);
    MRB_SET_INSTANCE_TT(garnet_class, MRB_TT_DATA);
    qDebug() << "Register class: " << class_name;

    return garnet_class;
}

template <class T>
void registerConstructor(mrb_state *mrb, RClass *garnet_class)
{
    auto initialize_impl = [](mrb_state *mrb, mrb_value self) -> mrb_value {
        Q_UNUSED(mrb)
        DATA_TYPE(self) = &Object::class_type;
        DATA_PTR(self) = new std::shared_ptr<T>(new T);
        return self;
    };

    mrb_define_method(mrb, garnet_class, "initialize", initialize_impl, ARGS_NONE());
}

} // namespace detail

template <class T>
void registerClass(mrb_state *mrb)
{
    initialize();
    RClass *garnet_class = detail::createClass<T>(mrb);
    detail::registerConstructor<T>(mrb, garnet_class);
    registerMethods(mrb, garnet_class, T::staticMetaObject);
}

void registerDynamicObject(mrb_state *mrb, const QByteArray &name, QObject *object);

} // namespace Garnet

#endif // GARNET_UTILS_H
