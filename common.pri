CONFIG += c++11

mruby_path = $$PWD/mruby

PRE_TARGETDEPS = $$mruby_path/build/host/lib/libmruby.a
INCLUDEPATH += $$mruby_path/include
LIBS += $$mruby_path/build/host/lib/libmruby.a

mruby.target = $$mruby_path/build/host/lib/libmruby.a
mruby.commands = cd $$mruby_path && make
mruby.depends = $$mruby_path/Makefile
QMAKE_EXTRA_TARGETS += mruby
