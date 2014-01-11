#ifndef TESTGARNETBRIDGECLASS_H
#define TESTGARNETBRIDGECLASS_H

#include <QObject>

class TestGarnetBridgeClass : public QObject
{
    Q_OBJECT
public:
    explicit TestGarnetBridgeClass(QObject *parent = 0);

private slots:
    void testMethod();
    void testVariantMethod();
    void testVariadicMethod();
    void testOverloadedMethod();
    void testProperty();
    void testConstructor();
    void testObjectOwnership();
    void testQml();
};

#endif // TESTGARNETBRIDGECLASS_H
