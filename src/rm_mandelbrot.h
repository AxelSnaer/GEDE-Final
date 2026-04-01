#ifndef RAYMARCHER_MANDELBROT_H
#define RAYMARCHER_MANDELBROT_H

#include "rm_shape.h"

namespace godot {
    class RMMandelbrot final : public RMShape {
        GDCLASS(RMMandelbrot, RMShape)
    public:
        RMMandelbrot();
        static void _bind_methods();
        [[nodiscard]] String gen_sdf() const override;
    };
}

#endif