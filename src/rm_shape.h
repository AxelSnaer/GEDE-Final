#ifndef RAYMARCHER_SHAPE_H
#define RAYMARCHER_SHAPE_H

#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/render_data.hpp"
#include "godot_cpp/core/gdvirtual.gen.inc"

namespace godot {
    enum class RMShapeOperationType : GDExtensionInt {
        Add,
        Subtract,
        Intersect,
        SmoothUnion,
    };
    class RMShape : public Node3D {
        GDCLASS(RMShape, Node3D);
    public:

        RMShape();
        virtual ~RMShape();

        void _process(double p_delta) override;

        [[nodiscard]] virtual String gen_sdf() const;

        static void _bind_methods();
        GDExtensionInt get_operation_type() const;
        void set_operation_type(GDExtensionInt p_type);
        float get_smoothing_amount() const;
        void set_smoothing_amount(float p_amount);
        static void invalidate_cache();
    protected:
        GDVIRTUAL0RC_REQUIRED(String, _gen_sdf);
        RMShapeOperationType m_operation_type;
        float m_smoothing_amount;


    private:
        Vector3 m_last_position;
        Vector3 m_last_scale;
        Vector3 m_last_rotation;
    };
}

#endif