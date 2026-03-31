#ifndef RAYMARCHER_H
#define RAYMARCHER_H

#include <godot_cpp/classes/compositor_effect.hpp>

#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/classes/render_data.hpp"

namespace godot {
    constexpr char template_shader[] = R"(
        #version 450

        // Invocations in the (x, y, z) dimension
        layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

        layout(rgba16f, set = 0, binding = 0) uniform image2D color_image;
        // setup layout for usage of more than 128 bypassing Vulkan limit
        layout(set = 0, binding = 1) uniform sampler2D depth_image;

        // Our push constant
        layout(push_constant, std430) uniform Params {
            vec2 raster_size;
            vec2 reserved;
            vec4 camera_pos;
            vec4 camera_forward;
            vec4 camera_up;
            mat4 projection_matrix;
        } params;

        // The code we want to execute in each invocation
        void main() {
            ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
            ivec2 size = ivec2(params.raster_size);

            if (uv.x >= size.x || uv.y >= size.y) {
                return;
            }

            vec4 color = imageLoad(color_image, uv);
            float depth = texelFetch(depth_image, uv, 0).r;

    #COMPUTE_CODE

            imageStore(color_image, uv, color);
        }
    )";

    class Raymarcher final : public CompositorEffect {
        GDCLASS(Raymarcher, CompositorEffect)

    protected:
        static void _bind_methods();
        [[nodiscard]] String get_shader_code();
        void set_shader_code(String code);
        void _notification(int p_what) const;
        bool _check_shader();

    public:
        Raymarcher();
        ~Raymarcher() override;
        void _render_callback(int32_t p_effect_callback_type, RenderData* p_render_data) override;
    private:
        String m_shader_code;
        RenderingDevice* m_rd;
        RID m_shader;
        RID m_pipeline;
        RID m_sampler;
        Ref<Mutex> m_mutex;
        bool m_dirty;
        GDExtensionInt m_test = 11;
    };

}

#endif