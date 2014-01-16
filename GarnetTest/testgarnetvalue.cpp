#include "testgarnetvalue.h"
#include "test.h"
#include "garnet/engine.h"
#include "garnet/value.h"

TestGarnetValue::TestGarnetValue(QObject *parent) :
    QObject(parent)
{
}

void TestGarnetValue::testQObject()
{
    Garnet::Engine engine;
    auto object = new QObject();
    auto result = Garnet::Value::fromObject(engine.mrbState(), object).toQObject();
    QCOMPARE(result, object);
}

void TestGarnetValue::testInt()
{
    Garnet::Engine engine;
    auto expected = 8128;
    auto result = Garnet::Value::fromVariant(engine.mrbState(), expected).toInt();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testDouble()
{
    Garnet::Engine engine;
    auto expected = 3.1415;
    auto result = Garnet::Value::fromVariant(engine.mrbState(), expected).toDouble();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testString()
{
    Garnet::Engine engine;
    auto expected = QString("lorem ipsum");
    auto result = Garnet::Value::fromVariant(engine.mrbState(), expected).toString();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testList()
{
    Garnet::Engine engine;
    auto expected = QVariantList { 1, "234", 5.6 };
    auto result = Garnet::Value::fromVariant(engine.mrbState(), expected).toList();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testHash()
{
    Garnet::Engine engine;
    auto expected = QVariantHash { { "one", 1}, { "two", 2.0} };
    auto result = Garnet::Value::fromVariant(engine.mrbState(), expected).toHash();
    QCOMPARE(result, expected);

    auto expected2 = QVariantHash { { "alpha", "str"}, { "bravo", 123}, { "3", 4.56 } };
    auto result2 = engine.evaluate("{ :alpha => 'str', 'bravo' => 123, 3 => 4.56 }").toHash();
    QCOMPARE(result2, expected2);
}

void TestGarnetValue::testSymbolToString()
{
    Garnet::Engine engine;
    auto result = engine.evaluate(":some_symbol").toString();
    QCOMPARE(result, QString("some_symbol"));
}

ADD_TEST_CLASS(TestGarnetValue)
