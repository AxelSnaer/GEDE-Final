#include "rm_compositor.h"
#include "raymarcher.h"

namespace godot {
    RMCompositor::RMCompositor()
    {
        Raymarcher* rm = Raymarcher::get_singleton();

        if (rm != nullptr)
            rm->register_compositor(this);
    }

    RMCompositor::~RMCompositor()
    {
        Raymarcher* rm = Raymarcher::get_singleton();

        if (rm != nullptr)
            rm->unregister_compositor(this);
    }

    String generate_mix_function(RMShapeOperationType op, float smoothing) {
        switch (op) {
            case RMShapeOperationType::Add:
                return "op_union(shape_depth, depth)";
            case RMShapeOperationType::Subtract:
                return "op_sub(shape_depth, depth)";
            case RMShapeOperationType::Intersect:
                return "op_intersect(shape_depth, depth)";
            case RMShapeOperationType::SmoothUnion:
                return String("op_smooth_union(shape_depth, depth, %f)").format(Array({ smoothing }), "%f");
        }

        print_error(String("Invalid operation type passed to generate_mix_function: ") + static_cast<int>(op));

        return "op_union(d, depth)";
    }

    String RMCompositor::gen_sdf() const
    {
        String sdf;
        auto shapes = get_shapes();

        for (const auto& shape : shapes) {
            Vector3 gp = -shape->get_global_position();
            Vector3 scale = shape->get_scale();
            Vector3 euler = shape->get_global_rotation();

            String shape_template = R"(
            {
                vec3 scale = vec3(%f, %f, %f);
                vec3 euler = vec3(%f, %f, %f);
                vec3 pos = pp + vec3(%f, %f, %f);
                pos = (rotate_z(euler.z) * rotate_x(euler.x) * rotate_y(euler.y) * vec4(pos, 1.0)).xyz;
                pos /= scale;
                float depth = shape_depth;
                %s
                depth *= min(scale.x, min(scale.y, scale.z));
                shape_depth = %s;
            })";

            sdf += shape_template
                .format(Array({
                    shape->gen_sdf(),
                    generate_mix_function(
                        static_cast<RMShapeOperationType>(shape->get_operation_type()),
                        shape->get_smoothing_amount()
                    )
                }), "%s")
                .format(Array({
                    scale.x, scale.y, scale.z,
                    euler.x, euler.y, euler.z,
                    gp.x, gp.y, gp.z
                }), "%f");
        }

        return String(R"({
            float shape_depth = MAX_DEPTH;
            vec3 c = vec3(%f, %f, %f);
            bool repeat = %s;
            vec3 pp = repeat ? mod(p + 0.5 * c, c) - 0.5 * c : p;
            #GEN
            d = min(d, shape_depth);
        })")
            .format(Array({ m_repeat.is_zero_approx() ? String("false") : String("true") }), "%s")
            .format(Array({ m_repeat.x, m_repeat.y, m_repeat.z }), "%f")
            .replace("#GEN", sdf);
    }

    void RMCompositor::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("get_repeat"), &RMCompositor::get_repeat);
        ClassDB::bind_method(D_METHOD("set_repeat", "repeat"), &RMCompositor::set_repeat);
        ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "repeat"), "set_repeat", "get_repeat");
    }

    Vector3 RMCompositor::get_repeat() const
    {
        return m_repeat;
    }

    void RMCompositor::set_repeat(Vector3 val)
    {
        m_repeat = val;
        invalidate_cache();
    }

    std::vector<RMShape*> RMCompositor::get_shapes() const
    {
        std::vector<RMShape*> shapes;

        for (int i = 0; i < get_child_count(); i++) {
            Node* child = get_child(i);
            RMShape* shape = dynamic_cast<RMShape*>(child);

            if (shape != nullptr)
                shapes.push_back(shape);
        }

        return shapes;
    }

    void RMCompositor::invalidate_cache() const
    {
        Raymarcher* rm = Raymarcher::get_singleton();
        if (rm != nullptr)
            rm->invalidate_cache();
    }
}
