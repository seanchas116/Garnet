#ifndef GARNET_TYPE_H
#define GARNET_TYPE_H

#include <functional>
#include <mruby.h>
#include <QVariant>
#include <QMap>
#include <QDebug>

#include "garnetlib_global.h"

namespace Garnet {

class GARNETLIBSHARED_EXPORT Variant {
public:
    Variant();
    Variant(const QVariant& variant);
    Variant(mrb_state *mrb, mrb_value value);
    operator QVariant();
    mrb_value toValue(mrb_state *mrb);

    typedef std::function<bool(mrb_state*, const mrb_value&, QVariant*)> ValueToVariant;
    typedef std::function<mrb_value(mrb_state*, const QVariant&)> VariantToValue;

    static void registerConverter(const ValueToVariant& val2var);

    template <class T>
    static void registerConverter(const VariantToValue& var2val)
    {
        var2val_converters.insert(qMetaTypeId<T>(), var2val);
    }

private:
    QVariant data_;
    static QList<ValueToVariant> val2var_converters;
    static QMap<int, VariantToValue> var2val_converters;

};

} // namespace Garnet

#endif // GARNET_TYPE_H
