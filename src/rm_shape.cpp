#include "rm_shape.h"
#include "raymarcher.h"

namespace godot {
    RMShape::RMShape() {
        Raymarcher* rm = Raymarcher::get_singleton();

        if (rm != nullptr)
            rm->register_shape(this);
    }

    RMShape::~RMShape() {
        Raymarcher* rm = Raymarcher::get_singleton();

        if (rm != nullptr)
            rm->unregister_shape(this);
    }

    void RMShape::_bind_methods() {
        ClassDB::add_virtual_method("RMShape", MethodInfo(Variant::STRING, "gen_sdf"), {});
    }
}
