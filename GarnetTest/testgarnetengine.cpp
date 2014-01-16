#include "testgarnetengine.h"
#include "test.h"
#include <Garnet/Engine>

TestGarnetEngine::TestGarnetEngine(QObject *parent) :
    QObject(parent)
{
}

void TestGarnetEngine::testError()
{
    Garnet::Engine engine;
    engine.evaluate("5.times do\n"
                    "raise 'error'\n"
                    "end", "*script*");
    QCOMPARE(engine.error(), QString("*script*:2: error (RuntimeError)"));
    QVERIFY(!engine.backtrace().isEmpty());
}

ADD_TEST_CLASS(TestGarnetEngine)
