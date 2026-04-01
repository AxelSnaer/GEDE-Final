#include "rm_sphere.h"
#include "godot_cpp/classes/engine.hpp"
#include "raymarcher.h"

namespace godot {
    RMSphere::RMSphere() : m_radius(1.0f) {
    }

    void RMSphere::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_radius"), &RMSphere::get_radius);
        ClassDB::bind_method(D_METHOD("set_radius", "radius"), &RMSphere::set_radius);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_radius", "get_radius");
    }

    String RMSphere::gen_sdf() const {
        const auto format = String("depth = length(pos) - %d;");
        const Array args = { m_radius };
        return format.format(args, "%d");
    }

    float RMSphere::get_radius() const { return m_radius; }

    void RMSphere::set_radius(const float value) {
        m_radius = value;
        invalidate_cache();
    }
}
