#include "rm_shape.h"

namespace godot {
    void RMShape::_bind_methods() {
        ClassDB::add_virtual_method("RMShape", MethodInfo(Variant::STRING, "gen_sdf"), {});
    }
}
