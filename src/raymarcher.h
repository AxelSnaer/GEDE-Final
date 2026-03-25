#ifndef RAYMARCHER_H
#define RAYMARCHER_H

#include <godot_cpp/classes/sprite2d.hpp>

namespace godot {

    class Raymarcher : public Sprite2D {
        GDCLASS(Raymarcher, Sprite2D)

    private:
        double time_passed;

    protected:
        static void _bind_methods();

    public:
        Raymarcher();
        ~Raymarcher();

        void _process(double delta) override;
    };

}

#endif