#include "garnet/value.h"
#include "bridgeclass.h"
#include "garnet/engine.h"
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <QVector>

namespace Garnet {

namespace {

namespace Converters {

QHash<int, Value::FromVariant> convertersFromVariant;
QList<Value::ToVariant> convertersToVariant;

mrb_value qVariantToValue(mrb_state *mrb, const QVariant &value);
QVariant qVariantFromValue(mrb_state *mrb, mrb_value value);

mrb_value qStringToString(mrb_state *mrb, const QString &str)
{
    auto utf8 = str.toUtf8();
    return mrb_str_new(mrb, utf8.data(), utf8.size());
}

QString qStringFromString(mrb_state *mrb, mrb_value value)
{
    Q_UNUSED(mrb);
    return QString::fromUtf8(RSTRING_PTR(value), RSTRING_LEN(value));
}

QString qStringFromSymbol(mrb_state *mrb, mrb_value value)
{
    size_t len;
    auto data = mrb_sym2name_len(mrb, mrb_obj_to_sym(mrb, value), &len);
    return QString::fromUtf8(data, len);
}

mrb_value qVariantListToArray(mrb_state *mrb, const QVariantList &list)
{
    QVector<mrb_value> values(list.size());
    std::transform(list.cbegin(), list.cend(), values.begin(), [&](const QVariant &variant) {
        return qVariantToValue(mrb, variant);
    });
    return mrb_ary_new_from_values(mrb, values.size(), values.data());
}

QVariantList qVariantListFromArray(mrb_state *mrb, mrb_value value)
{
    auto values = RARRAY_PTR(value);
    int size = RARRAY_LEN(value);

    QVariantList list;
    list.reserve(size);
    for (int i = 0; i < size; ++i) {
        list << qVariantFromValue(mrb, values[i]);
    }
    return list;
}

template <class T>
mrb_value qVariantHashLikeToHash(mrb_state *mrb, const T &hash)
{
    auto value = mrb_hash_new(mrb);
    for (auto i = hash.cbegin(); i != hash.cend(); ++i) {
        mrb_hash_set(mrb, value, qStringToString(mrb, i.key()), qVariantToValue(mrb, i.value()));
    }
    return value;
}

template <class T>
QVariantHash qVariantHashLikeFromHash(mrb_state *mrb, mrb_value hash)
{
    T resultHash;

    auto keys = mrb_hash_keys(mrb, hash);
    auto keyPtr = RARRAY_PTR(keys);
    int count = RARRAY_LEN(keys);
    resultHash.reserve(count);

    for (int i = 0; i < count; ++i) {

        auto key = keyPtr[i];
        auto value = mrb_hash_get(mrb, hash, key);

        QString qStringKey;

        switch (mrb_type(key)) {
        case MRB_TT_SYMBOL:
            qStringKey = qStringFromSymbol(mrb, key);
            break;
        case MRB_TT_STRING:
            qStringKey = qStringFromString(mrb, key);
            break;
        default:
            qStringKey = qStringFromString(mrb, mrb_funcall(mrb, key, "to_s", 0));
            break;
        }

        resultHash[qStringKey] = qVariantFromValue(mrb, value);
    }

    return resultHash;
}

QObject *qObjectStarFromBridgeClass(mrb_state *mrb, mrb_value value)
{
    Q_UNUSED(mrb);
    return BridgeData::getObject(value);
}

mrb_value qObjectStarToBridgeClass(mrb_state *mrb, QObject *object)
{
    return Engine::findByMrb(mrb)->bridgeClass().newFromObject(object, false);
}

mrb_value qVariantToValue(mrb_state *mrb, const QVariant &variant)
{
    auto type = variant.userType();
    switch (type) {

    case QMetaType::QObjectStar:
        return qObjectStarToBridgeClass(mrb, variant.value<QObject *>());

    case QMetaType::Bool:
        return mrb_bool_value(variant.toBool());

    case QMetaType::UInt:
    case QMetaType::Int:

    case QMetaType::ULong:
    case QMetaType::Long:

    case QMetaType::LongLong:
    case QMetaType::ULongLong:

    case QMetaType::UShort:
    case QMetaType::Short:

    case QMetaType::UChar:
    case QMetaType::SChar:
    case QMetaType::Char:

    case QMetaType::QChar:
        return mrb_fixnum_value(variant.toInt());

    case QMetaType::Float:
    case QMetaType::Double:
        return mrb_float_value(mrb, variant.toDouble());

    case QMetaType::QString:
    case QMetaType::QByteArray:
        return qStringToString(mrb, variant.toString());

    case QMetaType::QVariantList:
        return qVariantListToArray(mrb, variant.toList());

    case QMetaType::QVariantHash:
        return qVariantHashLikeToHash(mrb, variant.toHash());

    case QMetaType::QVariantMap:
        return qVariantHashLikeToHash(mrb, variant.toMap());

    case QMetaType::QVariant:
        return qVariantToValue(mrb, variant.value<QVariant>());

    default:
        if (convertersFromVariant.contains(type)) {
            return convertersFromVariant[type](mrb, variant);
        }
        return mrb_nil_value();
    }
}

QVariant qVariantFromValue(mrb_state *mrb, mrb_value value)
{
    if (mrb_string_p(value)) {
        return qStringFromString(mrb, value);
    }

    switch (mrb_type(value)) {
    case MRB_TT_TRUE:
        return true;
    case MRB_TT_FALSE:
        return false;
    case MRB_TT_FIXNUM:
        return mrb_fixnum(value);
    case MRB_TT_FLOAT:
        return mrb_float(value);
    case MRB_TT_STRING:
        return qStringFromString(mrb, value);
    case MRB_TT_SYMBOL:
        return qStringFromSymbol(mrb, value);
    case MRB_TT_ARRAY:
        return qVariantListFromArray(mrb, value);
    case MRB_TT_HASH:
        return qVariantHashLikeFromHash<QVariantHash>(mrb, value);
    case MRB_TT_DATA:
        if (DATA_TYPE(value) == &BridgeData::dataType) {
            return QVariant::fromValue(qObjectStarFromBridgeClass(mrb, value));
        }
    default:
    {
        QVariant result;
        for (const auto &toVariant : convertersToVariant) {
            if (toVariant(mrb, value, &result))
                return result;
        }
        return QVariant();
    }
    }
}

} // namespace Converters

} // anonymous namespace

class Value::Private
{
public:
    mrb_state *mrb_;
    mrb_value value_;
};

Value::Value(mrb_state *mrb, mrb_value value) :
    d(new Private())
{
    d->mrb_ = mrb;
    d->value_ = value;
}

Value::~Value()
{
}

Value::Value(const Value &other) :
    d(new Private(*other.d))
{
}

Value &Value::operator =(const Value &other)
{
    d.reset(new Private(*other.d));
    return *this;
}

Value Value::fromObject(mrb_state *mrb, QObject *object)
{
    return Value::fromVariant(mrb, QVariant::fromValue(object));
}

Value Value::fromVariant(mrb_state *mrb, const QVariant &variant)
{
    return Value(mrb, Converters::qVariantToValue(mrb, variant));
}

mrb_value Value::mrbValueFromVariant(mrb_state *mrb, const QVariant &variant)
{
    return Converters::qVariantToValue(mrb, variant);
}

mrb_state *Value::mrbState()
{
    return d->mrb_;
}

mrb_value Value::mrbValue()
{
    return d->value_;
}

QVariant Value::toVariant()
{
    return Converters::qVariantFromValue(d->mrb_, d->value_);
}

QObject *Value::toQObject()
{
    auto variant = toVariant();
    if (variant.userType() == QMetaType::QObjectStar) {
        return variant.value<QObject *>();
    } else {
        return nullptr;
    }
}

void Value::registerConverter(const QList<int> &metaTypes, const FromVariant func)
{
    for (int metaType : metaTypes) {
        Converters::convertersFromVariant[metaType] = func;
    }
}

void Value::registerConverter(const ToVariant func)
{
    Converters::convertersToVariant << func;
}

} // namespace Garnet
