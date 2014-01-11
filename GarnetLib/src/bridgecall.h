#ifndef GARNET_BRIDGECALL_H
#define GARNET_BRIDGECALL_H

#include <mruby.h>
struct QMetaObject;
class QMetaProperty;
class QObject;

namespace Garnet {

// To prevent memory leaks caused by mruby exceptions (which use longjmp),
// all local variables that are used in every scope which may be jumped out by them
// mustMethodCal have trivial destructors
class BridgeCall
{
public:

    BridgeCall(mrb_state *mrb, mrb_value self);

    // for constructors
    BridgeCall(mrb_state *mrb, const QMetaObject *metaObject);

    QObject *callConstructor();
    mrb_value callMethod();
    mrb_value accessProperty(bool setter);

    static mrb_value methodFunc(mrb_state *mrb, mrb_value self);
    static mrb_value setterFunc(mrb_state *mrb, mrb_value self);
    static mrb_value getterFunc(mrb_state *mrb, mrb_value self);

private:

    QMetaProperty accessedProperty(bool setter);

    // all member variables have trivial destructors
    // to prevent memory leaks caused by mruby exceptions
    mrb_state *mrb_;
    const char *methodName_;
    const QMetaObject *metaObject_;
    QObject *object_;
};


} // namespace Garnet

#endif // GARNET_BRIDGECALL_H
