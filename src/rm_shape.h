#ifndef RAYMARCHER_SHAPE_H
#define RAYMARCHER_SHAPE_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/render_data.hpp"

namespace godot {
    enum class RMShapeOperationType : GDExtensionInt {
        Add,
        Subtract,
        Intersect,
        SmoothUnion,
    };
    class RMShape : public Node3D {
        GDCLASS(RMShape, Node3D)
    public:
        [[nodiscard]] virtual String gen_sdf() const = 0;
        static void _bind_methods();
        GDExtensionInt get_operation_type() const;
        void set_operation_type(GDExtensionInt p_type);
        float get_smoothing_amount() const;
        void set_smoothing_amount(float p_amount);
    protected:
        RMShapeOperationType m_operation_type;
        float m_smoothing_amount;
    };
}

#endif