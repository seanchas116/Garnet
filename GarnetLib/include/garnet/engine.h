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

    void collectGarbage();
    Value evaluate(const QString &script);
    QVariant evaluateIntoVariant(const QString &script);

    template <class T> void registerClass() { registerClass(&T::staticMetaObject); }
    void registerClass(const QMetaObject *metaObject);
    void registerObject(const QString &name, QObject *object);
    void registerVariant(const QString &name, const QVariant &variant);

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
