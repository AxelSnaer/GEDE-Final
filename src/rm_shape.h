#ifndef RAYMARCHER_SHAPE_H
#define RAYMARCHER_SHAPE_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/render_data.hpp"

namespace godot {
    class RMShape : public Node3D {
        GDCLASS(RMShape, Node3D)
    public:
        RMShape();
        virtual ~RMShape();

        [[nodiscard]] virtual String gen_sdf() const = 0;
        static void _bind_methods();
    };
}

#endif