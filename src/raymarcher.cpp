#include "raymarcher.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Raymarcher::_bind_methods() {
}

Raymarcher::Raymarcher() {
    // Initialize any variables here.
    time_passed = 0.0;
}

Raymarcher::~Raymarcher() {
    // Add your cleanup here.
}

void Raymarcher::_process(double delta) {
    time_passed += delta;

    Vector2 new_position = Vector2(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));

    set_position(new_position);
}