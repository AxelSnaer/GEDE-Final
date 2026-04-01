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
        void _process(double p_delta) override;
    protected:
        float m_radius;
        Vector3 m_last_position;
    };
}

#endif