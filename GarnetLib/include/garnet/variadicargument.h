#ifndef GARNET_VARIADICARGUMENT_H
#define GARNET_VARIADICARGUMENT_H

#include <QVariant>

namespace Garnet {

class VariadicArgument
{
public:
    VariadicArgument() = default;
    VariadicArgument(const QVariantList &vlist) : list_(vlist) {}
    QVariantList toList() const { return list_; }

private:
    QVariantList list_;
};

} // namespace Garnet

Q_DECLARE_METATYPE(Garnet::VariadicArgument)

#endif // GARNET_VARIADICARGUMENT_H
