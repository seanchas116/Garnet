#ifndef GARNET_OBJECT_H
#define GARNET_OBJECT_H

#include <memory>
#include <QObject>
#include <mruby.h>
#include <mruby/data.h>

#include "garnetlib_global.h"

namespace Garnet {

class Object;
typedef std::shared_ptr<Object> ObjectPtr;

class GARNETLIBSHARED_EXPORT Object :
        public QObject,
        public std::enable_shared_from_this<Object>
{
    Q_OBJECT
    Q_DISABLE_COPY(Object)
public:
    Object();
    virtual ~Object();
    QVariant send(const QVariant& variant);

    static ObjectPtr fromValue(const mrb_value& self);
    static mrb_data_type class_type;

};

} // namespace Garnet

Q_DECLARE_METATYPE(Garnet::ObjectPtr)

#endif // GARNET_OBJECT_H
