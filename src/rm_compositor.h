#ifndef RAYMARCHER_COMPOSITOR_H
#define RAYMARCHER_COMPOSITOR_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/render_data.hpp"
#include "rm_shape.h"

#include <vector>

namespace godot {
    class RMCompositor : public Node3D {
        GDCLASS(RMCompositor, Node3D)
    public:
        RMCompositor();
        virtual ~RMCompositor();

        String gen_sdf() const;
        static void _bind_methods();

        std::vector<RMShape*> get_shapes() const;
    };
}

#endif
