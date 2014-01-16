#include "engine.h"
#include "variadicargument.h"
#include "bridgeclass.h"
#include "conversion.h"
#include "utils.h"

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

    QString error_;
    QStringList backtrace_;

    static QHash<mrb_state *, Engine *> engines_;

    static void initializeGlobal();

    void dumpError()
    {
        if (!mrb_->exc)
            return;

        ArenaSaver as(mrb_);

        auto exc = mrb_obj_value(mrb_->exc);

        auto backtraceVlist = Conversion::toQVariant(mrb_, mrb_funcall(mrb_, exc, "backtrace", 0)).toList();
        backtrace_.reserve(backtraceVlist.size());
        for (const auto &v : backtraceVlist) {
            backtrace_ << v.toString();
        }

        error_ = Conversion::toQVariant(mrb_, mrb_funcall(mrb_, exc, "inspect", 0)).toString();

        mrb_->exc = nullptr;
    }
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

QVariant Engine::evaluate(const QString &script, const QString &fileName)
{
    auto mrb = d->mrb_;
    ArenaSaver as(mrb);

    auto context = mrbc_context_new(mrb);

    mrbc_filename(mrb, context, fileName.toUtf8().data());
    auto value = mrb_load_string_cxt(mrb, script.toUtf8().data(), context);
    mrbc_context_free(mrb, context);
    d->dumpError();

    return Conversion::toQVariant(mrb, value);
}

QString Engine::error()
{
    return d->error_;
}

QStringList Engine::backtrace()
{
    return d->backtrace_;
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
        return Conversion::toMrbValue(mrb, findByMrb(mrb)->d->registeredVariants_[name]);
    };
    mrb_define_method(d->mrb_, d->mrb_->object_class, byteArray, methodImpl, ARGS_NONE());
}

}
