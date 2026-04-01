#include "rm_mandelbrot.h"
#include "godot_cpp/classes/engine.hpp"
#include "raymarcher.h"

namespace godot {
    RMMandelbrot::RMMandelbrot() {
    }

    void RMMandelbrot::_bind_methods() {
    }

    String RMMandelbrot::gen_sdf() const {
        return String(R"(
            vec3 z = pos;
            float dr = 1.0;
            float r = 0.0;
            for (int i = 0; i < 10 ; i++) {
                r = length(z);
                if (r>2.0) break;
                
                // convert to polar coordinates
                float theta = acos(z.z/r);
                float phi = atan(z.y,z.x);
                float Power = smoothstep(0.0, 1.0, 100.0)*6.0+2.0;
                
                dr =  pow( r, Power-1.0)*Power*dr + 1.0;
                // scale and rotate the point
                float zr = pow( r,Power);
                theta = theta*Power;
                phi = phi*Power;
                
                // convert back to cartesian coordinates
                z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
                z+=pos;
            }
            depth = 0.5*log(r)*r/dr;
        )");
    }
}
