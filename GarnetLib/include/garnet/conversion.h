#ifndef GARNET_CONVERSION_H
#define GARNET_CONVERSION_H

#include <QVariant>
#include <mruby.h>

namespace Garnet {

namespace Conversion {

mrb_value toMrbValue(mrb_state *mrb, const QVariant &value);
QVariant toQVariant(mrb_state *mrb, mrb_value value);
QObject *toQObject(mrb_state *mrb, mrb_value value);

using ToValue = std::function<mrb_value(mrb_state *, const QVariant &)>;
using ToVariant = std::function<bool(mrb_state *, mrb_value, QVariant *)>;
void registerConverter(const QList<int> &metaTypes, const ToValue func);
void registerConverter(const ToVariant func);

} // namespace Conversion

} // namespace Garnet

#endif // GARNET_CONVERSION_H
