#ifndef GEDE_FINAL_SM_SPHERE_H
#define GEDE_FINAL_SM_SPHERE_H
#include "../godot-cpp/gen/include/godot_cpp/classes/node3d.hpp"

namespace godot {
    class RMSphere final : public Node3D {
        GDCLASS(RMSphere, Node3D)
    public:
        RMSphere();
        static void _bind_methods();
        [[nodiscard]] String sdf() const;
        [[nodiscard]] float get_radius() const;
        void set_radius(float value);
    protected:
        float m_radius;
    };
}


#endif //GEDE_FINAL_SM_SPHERE_H