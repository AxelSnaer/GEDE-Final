#ifndef RAYMARCHER_COMPOSITOR_H
#define RAYMARCHER_COMPOSITOR_H

#include "godot_cpp/variant/vector3.hpp"
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

        [[nodiscard]] Vector3 get_repeat() const;
        void set_repeat(Vector3 val);

        std::vector<RMShape*> get_shapes() const;

    protected:
        void invalidate_cache() const;

    private:
        Vector3 m_repeat;
    };
}

#endif
