#include "testgarnetvalue.h"
#include "test.h"
#include <Garnet/Engine>
#include <Garnet/Conversion>

TestGarnetValue::TestGarnetValue(QObject *parent) :
    QObject(parent)
{
}

namespace {

QVariant reconvert(mrb_state *mrb, const QVariant &variant)
{
    auto value = Garnet::Conversion::toMrbValue(mrb, variant);
    return Garnet::Conversion::toQVariant(mrb, value);
}

}

void TestGarnetValue::testQObject()
{
    Garnet::Engine engine;
    auto object = new QObject();
    auto result = reconvert(engine.mrbState(), QVariant::fromValue(object)).value<QObject *>();
    QCOMPARE(result, object);
}

void TestGarnetValue::testInt()
{
    Garnet::Engine engine;
    auto expected = 8128;
    auto result = reconvert(engine.mrbState(), expected).toInt();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testDouble()
{
    Garnet::Engine engine;
    auto expected = 3.1415;
    auto result = reconvert(engine.mrbState(), expected).toDouble();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testString()
{
    Garnet::Engine engine;
    auto expected = QString("lorem ipsum");
    auto result = reconvert(engine.mrbState(), expected).toString();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testList()
{
    Garnet::Engine engine;
    auto expected = QVariantList { 1, "234", 5.6 };
    auto result = reconvert(engine.mrbState(), expected).toList();
    QCOMPARE(result, expected);
}

void TestGarnetValue::testHash()
{
    Garnet::Engine engine;
    auto expected = QVariantHash { { "one", 1}, { "two", 2.0} };
    auto result = reconvert(engine.mrbState(), expected).toHash();
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
