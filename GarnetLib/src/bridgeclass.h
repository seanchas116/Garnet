#ifndef GARNET_BRIDGECLASS_H
#define GARNET_BRIDGECLASS_H

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <QHash>
class QObject;
struct QMetaObject;

namespace Garnet {

class BridgeData
{
public:
    BridgeData(QObject *object, bool ownsObject);
    ~BridgeData();

    static mrb_data_type dataType;
    static QObject *getObject(mrb_value bridgeValue);

private:

    QObject *object_;
    bool ownsObject_;
};

class BridgeClass
{
public:
    BridgeClass(mrb_state *mrb);
    mrb_value newFromObject(QObject *object, bool willOwnObject);

private:
    void define();

    mrb_state *mrb_ = nullptr;
    RClass *klass_ = nullptr;
};

class StaticBridgeClassManager
{
public:
    StaticBridgeClassManager(mrb_state *mrb);

    void define(const QMetaObject *metaObject);

private:

    mrb_state *mrb_ = nullptr;
    QHash<QByteArray, const QMetaObject *> metaClassHash_;
};

} // namespace Garnet

#endif // GARNET_BRIDGECLASS_H
