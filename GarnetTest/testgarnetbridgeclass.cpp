#include "testgarnetbridgeclass.h"
#include "test.h"
#include "garnet/engine.h"
#include <QQmlEngine>
#include <QQmlComponent>

TestGarnetBridgeClass::TestGarnetBridgeClass(QObject *parent) :
    QObject(parent)
{
}

void TestGarnetBridgeClass::testMethod()
{
    Garnet::Engine engine;
    TestObject testObject;
    engine.registerObject("test_object", &testObject);
    auto result = engine.evaluate("test_object.testMethod(1234, 13.4, false, '45')").toDouble();
    QCOMPARE(result, 16585.6);
}

void TestGarnetBridgeClass::testVariantMethod()
{
    Garnet::Engine engine;
    TestObject testObject;
    engine.registerObject("test_object", &testObject);
    auto result = engine.evaluate("test_object.testVariantMethod(1234, 13.4, 'Hello')").toString();
    QCOMPARE(result, QString("1247.4-Hello"));
}

void TestGarnetBridgeClass::testVariadicMethod()
{
    Garnet::Engine engine;
    TestObject testObject;
    engine.registerObject("test_object", &testObject);
    int result = engine.evaluate("test_object.testVariadicMethod('1234', 13.4, 999)").toInt();
    QCOMPARE(result, 14620);
}

void TestGarnetBridgeClass::testOverloadedMethod()
{
    Garnet::Engine engine;
    TestObject testObject;
    engine.registerObject("test_object", &testObject);
    int r1 = engine.evaluate("test_object.testOverloadedMethod").toInt();
    int r2 = engine.evaluate("test_object.testOverloadedMethod(2)").toInt();
    QCOMPARE(r1, 1);
    QCOMPARE(r2, 2);
}

void TestGarnetBridgeClass::testProperty()
{
    Garnet::Engine engine;
    TestObject testObject;
    engine.registerObject("test_object", &testObject);
    auto result = engine.evaluate("test_object.text = 'zzzz'; test_object.text").toString();
    QCOMPARE(result, QString("zzzz"));
}

void TestGarnetBridgeClass::testConstructor()
{
    Garnet::Engine engine;
    engine.registerClass<TestObject>();
    auto result = engine.evaluate("TestObject.new('zzzz').text").toString();
    QCOMPARE(result, QString("zzzz"));
}

void TestGarnetBridgeClass::testObjectOwnership()
{
    QPointer<QObject> testCpp = new TestObject();
    QPointer<QObject> testMrb;
    {
        Garnet::Engine engine;
        engine.registerObject("test_cpp", testCpp);
        engine.registerClass<TestObject>();
        testMrb = engine.evaluate("test_cpp; TestObject.new").value<QObject *>();
    }
    QVERIFY(!testCpp.isNull());
    QVERIFY(testMrb.isNull());
}

void TestGarnetBridgeClass::testQml()
{
    QQmlEngine qmlEngine;
    QQmlComponent component(&qmlEngine, QUrl("qrc:///test.qml"));
    auto object = component.create();
    Garnet::Engine engine;
    engine.registerObject("test_object", object);
    auto result = engine.evaluate("test_object.method(12, 3.4)").toString();
    QCOMPARE(result, QString("123.4"));
    delete object;
}

ADD_TEST_CLASS(TestGarnetBridgeClass)
