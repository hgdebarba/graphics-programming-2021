//
// Created by henrique on 04/11/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_RT_RENDERER_H
#define ITU_GRAPHICS_PROGRAMMING_RT_RENDERER_H

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "rt_types.h"
#include "frame_buffer.h"

namespace rt{
    using namespace Colors;
    using namespace glm;

    class Renderer{

        const unsigned int max_recursion = 5;
        float p_rg = 0.4f;

    public:
        void render(const std::vector<vertex> &vts,
                    const glm::mat4 &m,
                    const glm::mat4 &v,
                    const float fov_degrees,
                    unsigned int depth,
                    FrameBuffer <uint32_t> &fb) {

            float aspect_ratio = fb.H / fb.W;
            float bottom = - tan(abs(radians(fov_degrees)) * 0.5f);

            mat4 view_to_model = inverse(v * m);
            // the bottom left corner of the image plane/sensor
            vec4 lower_left_corner = vec4(bottom * aspect_ratio, bottom, -1, 1);
            vec4 cam_pos = view_to_model * vec4(0,0,0,1);

            float pixel_size = abs(bottom) * 2 / fb.H;

            for (int c = 0; c < fb.W; c++){
                for(int r = 0; r < fb.H; r++){
                    vec4 pixel_pos = lower_left_corner + vec4 (c * pixel_size, r * pixel_size,0, 0);
                    pixel_pos = view_to_model * pixel_pos;
                    Ray ray(cam_pos, normalize(pixel_pos - cam_pos));
                    color col = TraceRay(ray, depth, vts);
                    fb.paintAt(c, r, toRGBA32(col));
                }
            }
        }


        color TraceRay(const Ray & ray, unsigned int depth, const std::vector<vertex> &vts){
            depth = depth > max_recursion ? max_recursion : depth;
            color col = black;

            int hit_ID = -1;
            vec3 barycentric;
            float dist = FLT_MAX;
            for (int i = 0; i < vts.size(); i+=3)
            {
                float dist_temp = FLT_MAX;
                vec3 barycentric_temp;
                if (RayTriangleIntersection(ray, vts[i], vts[i+1], vts[i+2], dist_temp, barycentric_temp) && dist_temp < dist)
                {
                    hit_ID = i;
                    dist=dist_temp;
                    barycentric = barycentric_temp;
                }
            }
            if (hit_ID < 0) return col; // no hit
            vec3 i_normal = vts[hit_ID].norm * barycentric.x + vts[hit_ID+1].norm * barycentric.y + vts[hit_ID+2].norm * barycentric.z;
            color i_col = vts[hit_ID].col * barycentric.x + vts[hit_ID+1].col * barycentric.y + vts[hit_ID+2].col * barycentric.z;
           // vec3 i_normal = vts[hit_ID].norm * barycentric.z + vts[hit_ID+1].norm * barycentric.x + vts[hit_ID+2].norm * barycentric.y;
           // color i_col = vts[hit_ID].col * barycentric.z + vts[hit_ID+1].col * barycentric.x + vts[hit_ID+2].col * barycentric.y;
            vec3 i_pos = ray.origin + ray.direction * dist;

            // local light (color computation)
            vec3 light_pos(0,1.9f,0);
            vec3 light_dir = normalize(light_pos - i_pos);

            float ambient = 0.1f, diffuse = 0.5f, specular = 0.5f, shininess = 10;
            col = ambient * i_col +
                    diffuse * i_col * max(dot(light_dir, i_normal), .0f) +
                    specular * pow(max(dot(light_dir, i_normal), .0f), shininess);



            // reflection / recursion
            if (depth > 1) {
                Ray reflected_ray(i_pos, reflect(ray.direction, i_normal));
                reflected_ray.origin -= ray.direction * .001f; // this is a small offset to address numerical precision issues
                col += p_rg * TraceRay(reflected_ray, depth - 1, vts);
            }

            return col;
        }



        static bool RayTriangleIntersection(const Ray & ray, const vertex & p1, const vertex & p2, const vertex & p3,
                                            float & t, vec3 & barycentric)
        {
            vec3 e1 = p2.pos - p1.pos;
            vec3 e2 = p3.pos - p1.pos;
            vec3 q = cross(ray.direction, e2);
            float a = dot(e1, q);

            float tolerance = 10e-7f;
            // for numerical stability, a = 0 means that triangle plane and ray are parallel
            if (abs(a) < tolerance) return false;

            float f = 1.0f / a;
            vec3 s = ray.origin - vec3(p1.pos);
            float u = f * dot(s, q);

            // if u < 0, intersection with plane is not within the triangle
            if (u < -tolerance) return false;

            vec3 r = cross(s, e1);
            float v = f * dot(ray.direction, r);

            // if v < 0 or u+v > 1, intersection with plane is not within the triangle
            if (v < -tolerance || u + v > 1) return false;

            t = f * dot(e2, r);

            if (t < 0)
                return false;

            barycentric = vec3(1.0f - u - v, u, v);

            return true;
        }
    };
}


#endif //ITU_GRAPHICS_PROGRAMMING_RT_RENDERER_H
