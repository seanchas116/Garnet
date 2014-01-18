Garnet
======

Garnet provides mruby bindings for Qt objects.

## Features

* Run mruby scripts in Qt
* Value conversions between Qt values and mruby values
* Access properties and methods of QML objects and QObject-derived C++ objects


## How to use

### Basic

```cpp
// printer.h
class Printer : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE void print(const QString& str)
    {
        qDebug() << str;
    }
};
```

```cpp
// main.cpp
#include "printer.h"
#include <Garnet/Engine>

int main(int argc, char *argv[])
{
    Garnet::Engine engine;
    Printer printer;
    engine.registerObject("printer", &printer);

    QString script = "printer.print('Hello, world!')";
    engine.evaluate(script);

    return 0;
}
```

### Use existing objects

```cpp
// C++

Garnet::Engine engine;
Person alice;
engine.registerObject("alice", &alice);

engine.evaluate("alice.greet");
```

### Create objects in mruby

```cpp
// person.h

class Person : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE Person(const QString &name);
};
```

```cpp
// C++

Garnet::Engine engine;
engine.registerClass<Person>();

engine.evaluate("Person.new('Bob').greet");
```

### Properties

```cpp
// C++
class Friend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER name_)
    Q_PROPERTY(int age MEMBER age_)
private:
    QString name_;
    int age_;
};
```

```ruby
# mruby
f = Friend.new
f.name = "Daniel"
f.age = 24
```

### Return value
```cpp
// C++
class Calculator : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE double add(double a, double b)
    {
        return a + b;
    }
};
```

```cpp
// C++
Garnet::Engine engine;

QString script = " Calculator.new.add(12.3, 22.8) ";
Garnet::Value result = engine.evaluate(script);
qDebug() <<  result.toDouble();
```

### Variant Arguments
```cpp
// C++
class JukeBox : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void insertCoin(const QVariant& value)
    {
        bool ok = false;
        value.toInt(&ok);
        if (ok) {
            qDebug() << "Thanks!";
        }
        else {
            qDebug() << "Please insert coin!";
        }
    }
};
```

```ruby
# mruby
j = JukeBox.new
j.insertCoin(100)
j.insertCoin('zzz...')
```

### Variadic Arguments

```cpp
// C++
class CommandList : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE void add(QString& name, const Garnet::VariadicArgument& options)
    {
        qDebug() << name.toStdString();
        
        qDebug() << "with options: ";
        foreach (const QVariant& option, options.toList()) {
            qDebug() << option.toString();
        }
    }
};
```

```ruby
# mruby
t = CommandList.new
t.add("grep", "-r", "-n")
```

### Use QML objects

```qml
// Product.qml
QtObject {
    property int price: 50
    function discount(diff) {
        price -= Math.floor(diff * Math.random())
    }
}
```

```cpp
// C++
QQmlEngine qmlEngine;
QQmlComponent component(&qmlEngine, QUrl("qrc:///Product.qml"));
auto product = component.create();

Garnet::Engine engine;
engine.registerObject("product", product);
```

```ruby
// mruby
product.price = 100
product.discount(10)
```

### Use Garnet in QML

```cpp
// C++
#include <Garnet/Engine>

qmlRegisterType<Garnet::Engine>("Garnet", 1, 0, "Engine");
```

```qml
// QML
import Garnet 1.0 as Garnet

Rectangle {
    width: 360
    height: 360
    Text {
        id: resultText
        anchors.fill: parent
    }
    Garnet.Engine {
        Component.onCompleted: {
            registerObject("result_text", resultText)
            engine.evaluate("result_text.text = 'Hello, world!'")
        }
    }
}
```

### Value conversion between Qt and mruby

* bool <-> TrueClass, FalseClass
* int, other integer types <-> Fixnum
* double, float <-> Float
* QString <-> String
* QVariantList <-> Array
* QVariantHash, QVariantMap <-> Hash

When mruby Hash objects are converted into QVariantHash / QVariantMap objects,
non-string keys are converted into string by the to_s method.

### Garbage collection

QObject instences used in mruby are garbage collected
only if they are created in mruby and they have no parent.

```cpp
// C++

Garnet::Engine engine;
Person person;
engine.registerObject("person", &person);
engine.registerClass<Person>();
```

```ruby
# mruby

p1 = person     # will never be garbage collected
p2 = Person.new # will be garbage collected
```

## License

Garnet is available under a dual-licensing (LGPL and MIT).

