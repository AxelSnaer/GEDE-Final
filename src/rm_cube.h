#ifndef RAYMARCHER_CUBE_H
#define RAYMARCHER_CUBE_H
#include "rm_shape.h"

namespace godot {
    class RMCube final : public godot::RMShape {
            GDCLASS(RMCube, RMShape)
        public:
            RMCube();
            static void _bind_methods();
            [[nodiscard]] float get_width() const;
            void set_width(float val);
            [[nodiscard]] float get_height() const;
            void set_height(float val);
            [[nodiscard]] float get_length() const;
            void set_length(float val);
            [[nodiscard]] godot::String gen_sdf() const override;
        private:
            float m_width;
            float m_height;
            float m_length;
    };
}

#endif