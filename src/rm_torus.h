#ifndef RAYMARCHER_TORUS_H
#define RAYMARCHER_TORUS_H

#include "rm_shape.h"

namespace godot {
    class RMTorus final : public RMShape {
        GDCLASS(RMTorus, RMShape)
    public:
        RMTorus();
        static void _bind_methods();
        [[nodiscard]] float get_thickness() const;
        void set_thickness(float val);
        [[nodiscard]] float get_radius() const;
        void set_radius(float val);
        [[nodiscard]] String gen_sdf() const override;
    private:
        float m_thickness;
        float m_radius;
    };
}


#endif