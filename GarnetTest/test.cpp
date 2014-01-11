#include "test.h"
#include <QCoreApplication>

namespace {

QObjectList testObjects;

}

void addTestObject(QObject *testObject)
{
    testObjects << testObject;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    int result = 0;
    for (auto test : testObjects) {
        result += QTest::qExec(test);
    }
    return result;
}
