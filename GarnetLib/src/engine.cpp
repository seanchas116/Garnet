#include "garnet/engine.h"
#include "garnet/value.h"
#include "garnet/variadicargument.h"
#include "bridgeclass.h"

#include <QVariant>

#include <mruby.h>
#include <mruby/compile.h>

namespace Garnet {

class Engine::Private
{
public:

    Private(Engine *engine, mrb_state *mrb);
    ~Private();

    Engine *q = nullptr;
    mrb_state *mrb_ = nullptr;
    bool ownsMrb_ = false;

    QScopedPointer<BridgeClass> bridgeClass;
    QScopedPointer<StaticBridgeClassManager> staticBridgeClassManager;
    QHash<QByteArray, QVariant> registeredVariants_;

    static QHash<mrb_state *, Engine *> engines_;

    static void initializeGlobal();
};

QHash<mrb_state *, Engine *> Engine::Private::engines_;

Engine::Private::Private(Engine *engine, mrb_state *mrb) :
    q(engine)
{
    initializeGlobal();

    if (!mrb) {
        ownsMrb_ = true;
        mrb = mrb_open();
    }
    mrb_ = mrb;
    engines_[mrb] = q;

    bridgeClass.reset(new BridgeClass(mrb));
    staticBridgeClassManager.reset(new StaticBridgeClassManager(mrb));
}

void Engine::Private::initializeGlobal()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    qRegisterMetaType<VariadicArgument>("Garnet::VariadicArgument");
}

Engine::Private::~Private()
{
    if (ownsMrb_) {
        mrb_close(mrb_);
    }
    engines_.remove(mrb_);
}

Engine::Engine(QObject *parent) :
    Engine(nullptr, parent)
{
}

Engine::Engine(mrb_state *mrb, QObject *parent) :
    QObject(parent),
    d(new Private(this, mrb))
{
}

Engine::~Engine()
{
}

Engine *Engine::findByMrb(mrb_state *mrb)
{
    return Private::engines_.value(mrb, nullptr);
}

mrb_state *Engine::mrbState()
{
    return d->mrb_;
}

BridgeClass &Engine::bridgeClass()
{
    return *d->bridgeClass;
}

StaticBridgeClassManager &Engine::staticBridgeClassManager()
{
    return *d->staticBridgeClassManager;
}

void Engine::collectGarbage()
{
    mrb_full_gc(d->mrb_);
}

Value Engine::evaluate(const QString &script)
{
    return Value(d->mrb_, mrb_load_string(d->mrb_, script.toUtf8().data()));
}

QVariant Engine::evaluateIntoVariant(const QString &script)
{
    return evaluate(script).toVariant();
}

void Engine::registerClass(const QMetaObject *metaObject)
{
    d->staticBridgeClassManager->define(metaObject);
}

void Engine::registerObject(const QString &name, QObject *object)
{
    registerVariant(name, QVariant::fromValue(object));
}

void Engine::registerVariant(const QString &name, const QVariant &variant)
{
    auto byteArray = name.toUtf8();
    d->registeredVariants_[byteArray] = variant;
    auto methodImpl = [](mrb_state *mrb, mrb_value self) {
        Q_UNUSED(self);
        auto name = mrb_sym2name(mrb, mrb->c->ci->mid);
        return Value::mrbValueFromVariant(mrb, findByMrb(mrb)->d->registeredVariants_[name]);
    };
    mrb_define_method(d->mrb_, d->mrb_->object_class, byteArray, methodImpl, ARGS_NONE());
}

}
