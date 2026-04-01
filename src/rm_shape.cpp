#include "rm_shape.h"

namespace godot {
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

    void RMShape::set_operation_type(GDExtensionInt p_type) { m_operation_type = static_cast<RMShapeOperationType>(p_type); }

    float RMShape::get_smoothing_amount() const { return m_smoothing_amount; }

    void RMShape::set_smoothing_amount(float p_amount) { m_smoothing_amount = p_amount; }
}
