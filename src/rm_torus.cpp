#include "rm_torus.h"
#include "raymarcher.h"

namespace godot {
    RMTorus::RMTorus(): m_thickness(1.0f), m_radius(3.0f) {
    }

    void RMTorus::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_thickness"), &RMTorus::get_thickness);
        ClassDB::bind_method(D_METHOD("set_thickness", "thickness"), &RMTorus::set_thickness);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "thickness", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_thickness", "get_thickness");

        ClassDB::bind_method(D_METHOD("get_radius"), &RMTorus::get_radius);
        ClassDB::bind_method(D_METHOD("set_radius", "radius"), &RMTorus::set_radius);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_radius", "get_radius");
    }

    float RMTorus::get_thickness() const { return m_thickness;}

    void RMTorus::set_thickness(float val) {
        m_thickness = val;
        invalidate_cache();
    }

    float RMTorus::get_radius() const { return m_radius; }

    void RMTorus::set_radius(float val) {
        m_radius = val;
        invalidate_cache();
    }

    String RMTorus::gen_sdf() const {
        const auto placeholder = String("vec2 q = vec2(length(pos.xz) - %d, pos.y);\ndepth = length(q) - %d;");
        const Array args = { m_radius, m_thickness };
        return placeholder.format(args, "%d");
    }
}