Garnet
======

mruby bindings for QObject

## Basic

```cpp
/* printer.h */

#include <iostream>
#include <garnet.h>

class Printer : public Garnet::Object {
    Q_OBJECT

public slots:
    void print(const QString& str)
    {
        std::cout << str.toStdString() << std::endl;
    }
};
```

```cpp
/* main.cpp */

#include <mruby.h>
#include <mruby/compile.h>
#include "printer.h"

int main(int argc, char *argv[])
{
    mrb_state *mrb = mrb_open();
    Garnet::registerClass<Printer>(mrb);

    const char *script = " Printer.new.print('Hello world!') ";
    mrb_load_string(mrb, script);

    mrb_close(mrb);
    return 0;
}
```

## Properties

```cpp
// C++
class Friend : public Garnet::Object {
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

## Return value
```cpp
// C++
class Calculator : public Garnet::Object {
    Q_OBJECT
public slots:
    double add(double a, double b)
    {
        return a + b;
    }
};
```

```cpp
// C++
const char *script = " Calculator.new.add(12.3, 22.8) ";
mrb_value value = mrb_load_string(mrb, script);
QVariant result = Garnet::Variant(mrb, value);
std::cout << result.toDouble() << std::endl;
```

## Variant Arguments
```cpp
// C++
class JukeBox : public Garnet::Object {
    Q_OBJECT
public slots:
    void insertCoin(const QVariant& value)
    {
        using namespace std;
        
        bool ok = false;
        value.toInt(&ok);
        if (ok) {
            cout << "Thanks!" << endl;
        }
        else {
            cout << "Please insert coin!" << endl;
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

## Variable Arguments

```cpp
// C++
class CommandList : public Garnet::Object {
    Q_OBJECT
public slots:
    void add(QString& name, const QVariantList& options)
    {
        using namespace std;
        cout << name.toStdString() << endl;
        
        cout << "with options: " << endl;
        foreach (const QVariant& option, options) {
            cout << option.toString().toStdString() << endl;
        }
    }
};
```

```ruby
# mruby
t = CommandList.new
t.add("grep", "-r", "-n")
```
