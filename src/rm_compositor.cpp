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
                vec3 pos = p + vec3(%f, %f, %f);
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
            #GEN
            d = min(d, shape_depth);
        })").replace("#GEN", sdf);
    }

    void RMCompositor::_bind_methods()
    {
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
}
