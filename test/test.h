#ifndef TEST_H
#define TEST_H

#include <QtTest>
#include <Garnet/VariadicArgument>

class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER text_)

public:

    Q_INVOKABLE TestObject() {}
    Q_INVOKABLE TestObject(const QString &text) : text_(text) {}

    Q_INVOKABLE double testMethod(int v1, double v2, bool v3, const QString& v4)
    {
        return v1 * v2 + (v3 ? -5 : 5) + v4.toInt();
    }

    Q_INVOKABLE QVariant testVariantMethod(const QVariant& v1, const QVariant& v2, const QVariant& v3)
    {
        return QString("%1-%2").arg(v1.toInt() + v2.toDouble()).arg(v3.toString());
    }

    Q_INVOKABLE int testVariadicMethod(const QString& v1, const Garnet::VariadicArgument& vr)
    {
        auto list = vr.toList();
        return v1.toInt() + list[0].toDouble() * list[1].toDouble();
    }

    Q_INVOKABLE int testOverloadedMethod()
    {
        return 1;
    }

    Q_INVOKABLE int testOverloadedMethod(int x)
    {
        return x;
    }

private:

    QString text_;
};

void addTestObject(QObject *testObject);

#define ADD_TEST_CLASS(klass) \
    static int _dummyValue = [](){ addTestObject(new klass()); return 0; }();

#endif // TEST_H
