#ifndef GARNET_VALUE_H
#define GARNET_VALUE_H

#include <QVariant>
#include <functional>

class QObject;
struct mrb_state;
struct mrb_value;

namespace Garnet {

class Engine;

class Value
{
public:
    Value(mrb_state *mrb, mrb_value value);
    ~Value();
    Value(const Value &other);
    Value &operator=(const Value &other);

    bool isQObject() const;
    bool isBoolean() const;
    bool isFixnum() const;
    bool isFloat() const;
    bool isString() const;
    bool isSymbol() const;
    bool isArray() const;
    bool isHash() const;

    QVariant toVariant();

    QObject *toQObject();
    int toInt() { return toVariant().toInt(); }
    double toDouble() { return toVariant().toDouble(); }
    QString toString() { return toVariant().toString(); }
    QVariantList toList() { return toVariant().toList(); }
    QVariantHash toHash() { return toVariant().toHash(); }

    mrb_state *mrbState();
    mrb_value mrbValue();

    static Value fromVariant(mrb_state *mrb, const QVariant &variant);
    static Value fromObject(mrb_state *mrb, QObject *object);

    static mrb_value mrbValueFromVariant(mrb_state *mrb, const QVariant &variant);

    using FromVariant = std::function<mrb_value(mrb_state *, const QVariant &)>;
    using ToVariant = std::function<bool(mrb_state *, mrb_value, QVariant *)>;
    static void registerConverter(const QList<int> &metaTypes, const FromVariant func);
    static void registerConverter(const ToVariant func);

private:
    class Private;
    QScopedPointer<Private> d;
};

} // namespace Garnet

#endif // GARNET_VALUE_H
