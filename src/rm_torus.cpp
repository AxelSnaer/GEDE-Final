#include "rm_torus.h"

namespace godot {
    RMTorus::RMTorus(): m_thickness(1.0f), m_radius(3.0f) {
        m_last_position = get_global_position();
        m_last_scale = get_scale();
        m_last_rotation = get_rotation();
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

    void RMTorus::set_thickness(float val) { m_thickness = val; }

    float RMTorus::get_radius() const { return m_radius; }

    void RMTorus::set_radius(float val) { m_radius = val; }

    String RMTorus::gen_sdf() const {
        const auto placeholder = String("vec3 pos = p + vec3(%d, %d, %d);\nvec2 q = vec2(length(pos.xz) - %d, pos.y);\ndepth = length(q) - %d;");
        const Vector3 gl_pos = get_global_position();
        const Array args = { gl_pos.x, gl_pos.y, gl_pos.z, m_thickness, m_radius };
        return placeholder.format(args, "%d");
    }

    void RMTorus::_process(double p_delta) {
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