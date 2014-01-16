#ifndef GARNET_UTILS_H
#define GARNET_UTILS_H

#include <mruby.h>

namespace Garnet {

class ArenaSaver
{
public:
    ArenaSaver(mrb_state *mrb) :
        mrb_(mrb),
        arena_(mrb_gc_arena_save(mrb))
    {
    }

    ~ArenaSaver()
    {
        mrb_gc_arena_restore(mrb_, arena_);
    }

private:
    mrb_state *mrb_;
    int arena_;
};

} // namespace Garnet

#endif // GARNET_UTILS_H
