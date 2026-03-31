#ifndef RAYMARCHER_H
#define RAYMARCHER_H

#include <godot_cpp/classes/compositor_effect.hpp>

#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/classes/render_data.hpp"

namespace godot {
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
    };

}

#endif