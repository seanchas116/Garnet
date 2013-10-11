#include "garnet.h"

namespace Garnet {

namespace {

void free_class(mrb_state *mrb, void *ptr)
{
    Q_UNUSED(mrb)
    delete static_cast<ObjectPtr*>(ptr);
}

} // anonymous namespace

mrb_data_type Object::class_type = { "GarnetObject", free_class };

Object::Object()
{

}

Object::~Object()
{

}

QVariant Object::send(const QVariant& variant)
{
    return QVariant();
}

ObjectPtr Object::fromValue(const mrb_value& self)
{
    if (DATA_PTR(self)) {
        return *static_cast<ObjectPtr*>(DATA_PTR(self));
    }
    else {
        return ObjectPtr();
    }
}

} // namespace Garnet
