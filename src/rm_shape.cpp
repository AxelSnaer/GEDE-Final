#include "rm_shape.h"
#include "raymarcher.h"

namespace godot {
    RMShape::RMShape(): m_operation_type(RMShapeOperationType::Add) {
        m_last_position = get_global_position();
        m_last_scale = get_scale();
        m_last_rotation = get_global_rotation();
    }

    RMShape::~RMShape() {
    }

    void RMShape::_process(double p_delta)
    {
        Vector3 gl_pos = get_global_position();
        Vector3 gl_rot = get_global_rotation();
        Vector3 gl_scale = get_scale();
        
        if (gl_pos != m_last_position || gl_rot != m_last_rotation || gl_scale != m_last_scale) {
            invalidate_cache();
        }
        m_last_position = gl_pos;
        m_last_scale = gl_scale;
        m_last_rotation = gl_rot;
    }

    void RMShape::_bind_methods() {
        ClassDB::add_virtual_method("RMShape", MethodInfo(Variant::STRING, "gen_sdf"), {});

        ClassDB::bind_method(D_METHOD("get_operation_type"), &RMShape::get_operation_type);
        ClassDB::bind_method(D_METHOD("set_operation_type", "operation_type"), &RMShape::set_operation_type);
        ADD_PROPERTY(PropertyInfo(Variant::INT, "operation_type", PROPERTY_HINT_ENUM, "Add,Subtract,Intersect,Smooth Union"), "set_operation_type", "get_operation_type");

        ClassDB::bind_method(D_METHOD("get_smoothing_amount"), &RMShape::get_smoothing_amount);
        ClassDB::bind_method(D_METHOD("set_smoothing_amount", "smoothing_amount"), &RMShape::set_smoothing_amount);
        ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "smoothing_amount", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_smoothing_amount", "get_smoothing_amount");
    }

    GDExtensionInt RMShape::get_operation_type() const { return static_cast<GDExtensionInt>(m_operation_type); }

    void RMShape::set_operation_type(GDExtensionInt p_type) {
        m_operation_type = static_cast<RMShapeOperationType>(p_type);
        invalidate_cache();
    }

    float RMShape::get_smoothing_amount() const { return m_smoothing_amount; }

    void RMShape::set_smoothing_amount(float p_amount) {
        m_smoothing_amount = p_amount;
        invalidate_cache();
    }

    void RMShape::invalidate_cache()
    {
        Raymarcher* rm = Raymarcher::get_singleton();
        if (rm != nullptr)
            rm->invalidate_cache();
    }
}
