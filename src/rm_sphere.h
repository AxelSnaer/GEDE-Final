#ifndef RAYMARCHER_SPHERE_H
#define RAYMARCHER_SPHERE_H

#include "rm_shape.h"

namespace godot {
    class RMSphere final : public RMShape {
        GDCLASS(RMSphere, RMShape)
    public:
        RMSphere();
        static void _bind_methods();
        [[nodiscard]] String gen_sdf() const override;
        [[nodiscard]] float get_radius() const;
        void set_radius(float value);
    protected:
        float m_radius;
    };
}

#endif