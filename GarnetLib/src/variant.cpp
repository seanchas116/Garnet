#include "garnet/variant.h"
#include <QDebug>

namespace Garnet {

QList<Variant::ValueToVariant> Variant::val2var_converters;
QMap<int, Variant::VariantToValue> Variant::var2val_converters;

Variant::Variant()
{

}

Variant::Variant(const QVariant& variant) :
    data_(variant)
{

}

Variant::Variant(mrb_state *mrb, mrb_value value)
{
    foreach (const ValueToVariant& func, val2var_converters) {
        if (func(mrb, value, &data_)) {
            break;
        }
    }
}

Variant::operator QVariant()
{
    return data_;
}

mrb_value Variant::toValue(mrb_state *mrb)
{
    int type = data_.userType();
    if (var2val_converters.contains(type)) {
        return var2val_converters[type](mrb, data_);
    }
    else {
        return mrb_nil_value();
    }
}

void Variant::registerConverter(const ValueToVariant& val2var)
{
    val2var_converters.push_back(val2var);
}

} // namespace Garnet
