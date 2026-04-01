#include "rm_sphere.h"
#include "godot_cpp/classes/engine.hpp"

namespace godot {
    RMSphere::RMSphere() : m_radius(1.0f) {
        m_last_position = get_global_position();
        m_last_scale = get_scale();
        m_last_rotation = get_global_rotation();
    }

    void RMSphere::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_radius"), &RMSphere::get_radius);
        ClassDB::bind_method(D_METHOD("set_radius", "radius"), &RMSphere::set_radius);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_radius", "get_radius");
    }

    String RMSphere::gen_sdf() const {
        const auto format = String("depth = length(p + vec3(%d, %d, %d)) - %d;");
        const Vector3 gl_pos = get_global_position();
        const Array args = { gl_pos.x, gl_pos.y, gl_pos.z, m_radius };
        return format.format(args, "%d");
    }

    float RMSphere::get_radius() const { return m_radius; }

    void RMSphere::set_radius(const float value) { m_radius = value; }

    void RMSphere::_process(double p_delta) {
        RMShape::_process(p_delta);
        Vector3 gl_pos = get_global_position();
        Vector3 gl_rot = get_global_rotation();
        Vector3 gl_scale = get_scale();
        if (gl_pos != m_last_position || gl_rot != m_last_rotation || gl_scale != m_last_scale) {
            // TODO: FIXME: Invalidate shape shader
        }
        m_last_position = gl_pos;
        m_last_scale = gl_scale;
        m_last_rotation = gl_rot;
    }
}
