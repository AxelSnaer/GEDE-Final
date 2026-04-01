#ifndef RAYMARCHER_H
#define RAYMARCHER_H

#include <vector>
#include <godot_cpp/classes/compositor_effect.hpp>

#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/rendering_device.hpp"
#include "godot_cpp/classes/render_data.hpp"

#include "rm_shape.h"
#include "rm_compositor.h"

namespace godot {
    class Raymarcher final : public CompositorEffect {
        GDCLASS(Raymarcher, CompositorEffect)

    protected:
        static void _bind_methods();
        void _notification(int p_what) const;
        bool _check_shader();
        String _generate_shader_code();

    public:
        Raymarcher();
        ~Raymarcher() override;
        void _render_callback(int32_t p_effect_callback_type, RenderData* p_render_data) override;

        void register_compositor(RMCompositor* shape);
        void unregister_compositor(RMCompositor* shape);
        void invalidate_cache();

        static Raymarcher* get_singleton();

    private:
        RenderingDevice* m_rd;
        RID m_shader;
        RID m_pipeline;
        RID m_sampler;
        Ref<Mutex> m_mutex;
        std::vector<RMCompositor*> m_compositors;
        bool m_dirty;

        static Raymarcher* m_singleton;
    };

}

#endif