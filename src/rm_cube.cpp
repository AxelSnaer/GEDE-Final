#include "rm_cube.h"

namespace godot {
    RMCube::RMCube(): m_width(1.0f), m_height(1.0f), m_length(1.0f) {}

    void RMCube::_bind_methods() {
        ClassDB::bind_method(D_METHOD("get_width"), &RMCube::get_width);
        ClassDB::bind_method(D_METHOD("set_width", "width"), &RMCube::set_width);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "width", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_width", "get_width");

        ClassDB::bind_method(D_METHOD("get_height"), &RMCube::get_height);
        ClassDB::bind_method(D_METHOD("set_height", "height"), &RMCube::set_height);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_height", "get_height");

        ClassDB::bind_method(D_METHOD("get_length"), &RMCube::get_length);
        ClassDB::bind_method(D_METHOD("set_length", "length"), &RMCube::set_length);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_RANGE, "0.0,180.0,0.1,or_greater"), "set_length", "get_length");
    }

    float RMCube::get_width() const { return m_width; }

    void RMCube::set_width(float val) {
        m_width = val;
        invalidate_cache();
    }

    float RMCube::get_height() const { return m_height;}

    void RMCube::set_height(float val) {
        m_height = val;
        invalidate_cache();
    }

    float RMCube::get_length() const { return m_length;}

    void RMCube::set_length(float val) {
        m_length = val;
        invalidate_cache();
    }

    String RMCube::gen_sdf() const {
        // Derived from https://iquilezles.org/articles/distfunctions/
        const auto placeholder = String(R"(
            vec3 boundaries = vec3(%f, %f, %f);
            vec3 q = abs(pos) - boundaries;
            depth = length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);)"
        );
        return placeholder.format(Array({m_width, m_height, m_length}), "%f");
    }
}
