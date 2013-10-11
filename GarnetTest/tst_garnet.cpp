#include <QString>
#include <QtTest>

#include <garnet.h>

#include <mruby.h>
#include <mruby/compile.h>

class GarnetTest : public QObject
{
    Q_OBJECT
    
public:
    GarnetTest();
    ~GarnetTest();

private:
    QVariant runScript(const QString& script);
    
private Q_SLOTS:
    void variantArgsTest();
    void numericArgsTest();
    void variadicArgsTest();
    void propertyTest();

private:
    mrb_state *mrb_ = mrb_open();
};

class TestObject : public Garnet::Object {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER name_)

public:
    Q_INVOKABLE QVariant variantArgs(const QVariant& v1, const QVariant& v2, const QVariant& v3);
    Q_INVOKABLE double numericArgs(int v1, double v2, bool v3, const QString& v4);
    Q_INVOKABLE int variadicArgs(const QString& v1, const QVariantList& vr);

private:
    QString name_;

};

QVariant TestObject::variantArgs(const QVariant& v1, const QVariant& v2, const QVariant& v3)
{
    return QString("%1-%2").arg(v1.toInt() + v2.toDouble()).arg(v3.toString());
}

double TestObject::numericArgs(int v1, double v2, bool v3, const QString& v4)
{
    return v1 * v2 + (v3 ? -5 : 5) + v4.toInt();
}

int TestObject::variadicArgs(const QString& v1, const QVariantList& vr)
{
    return v1.toInt() + vr[0].toDouble() * vr[1].toDouble();
}

GarnetTest::GarnetTest()
{
    mrb_ = mrb_open();
    Garnet::registerClass<TestObject>(mrb_);
}

GarnetTest::~GarnetTest()
{
    mrb_close(mrb_);
}

QVariant GarnetTest::runScript(const QString& script)
{
    return Garnet::Variant(mrb_, mrb_load_string(mrb_, script.toUtf8().data()));
}

void GarnetTest::variantArgsTest()
{
    QString result = runScript("TestObject.new.variantArgs(1234, 13.4, 'Hello')").toString();
    QCOMPARE(result, QString("1247.4-Hello"));
}

void GarnetTest::numericArgsTest()
{
    double result = runScript("TestObject.new.numericArgs(1234, 13.4, false, '45')").toDouble();
    QCOMPARE(result, 16585.6);
}

void GarnetTest::variadicArgsTest()
{
    int result = runScript("TestObject.new.variadicArgs('1234', 13.4, 999)").toInt();
    QCOMPARE(result, 14620);
}

void GarnetTest::propertyTest()
{
    QString result = runScript("o = TestObject.new; o.name = 'zzzz'; o.name").toString();
    QCOMPARE(result, QString("zzzz"));
}

QTEST_APPLESS_MAIN(GarnetTest)

#include "tst_garnet.moc"
