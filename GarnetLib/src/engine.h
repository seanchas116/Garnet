#ifndef GARNET_ENGINE_H
#define GARNET_ENGINE_H

#include <QObject>
#include <QVariant>

struct mrb_state;

namespace Garnet {

class BridgeClass;
class StaticBridgeClassManager;
class Value;

class Engine : public QObject
{
    Q_OBJECT

public:

    explicit Engine(QObject *parent = 0);
    explicit Engine(mrb_state *mrb, QObject *parent = 0);
    ~Engine();

    Q_INVOKABLE void collectGarbage();
    Q_INVOKABLE QVariant evaluate(const QString &script, const QString &fileName = "*script*");

    Q_INVOKABLE QString error() const;
    Q_INVOKABLE QStringList backtrace() const;

    template <class T> void registerClass() { registerClass(&T::staticMetaObject); }
    void registerClass(const QMetaObject *metaObject);
    Q_INVOKABLE void registerObject(const QString &name, QObject *object);
    Q_INVOKABLE void registerVariant(const QString &name, const QVariant &variant);

    mrb_state *mrbState();
    BridgeClass &bridgeClass();
    StaticBridgeClassManager &staticBridgeClassManager();
    static Engine *findByMrb(mrb_state *mrb);

private:
    class Private;
    QScopedPointer<Private> d;
};

}

#endif // GARNET_ENGINE_H
