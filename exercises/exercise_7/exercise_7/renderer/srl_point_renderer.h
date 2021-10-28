//
// Created by henrique debarba on 29/10/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_SRL_POINT_RENDERER_H
#define ITU_GRAPHICS_PROGRAMMING_SRL_POINT_RENDERER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "srl_renderer.h"
#include "srl_types.h"

namespace srl {
    class PointRenderer : public Renderer {
    private:

        // create point primitives
        void assemblePrimitives(const std::vector<vertex> &vts) override {
            m_primitives.clear();
            // preallocate
            m_primitives.reserve(vts.size());

            for(int i = 0, size = vts.size()-1; i < size; i ++){
                point p;
                p.v1 = vts[i];
                m_primitives.push_back(p);
            }
        }

        static void clipPoint(point &p, int side){
            // index to x, y or z coordinate (x=0, y=1, z=2)
            int idx = side % 3;
            // we check if the variable is in the range of the clipping plane using w
            // we need to test if w >= x,y,z >= -w, and clip when x,y,z > w or x,y,z < -w
            // we can rewrite the latter with x,y,z * -1 > w
            // so we need to multiply x,y,z by -1 when testing against the planes at -w
            // planes 0, 1 and 2 are positive w, planes 3, 4 and 5 are negative w
            float wMult = side > 2 ? -1.0f : 1.0f;

            glm::vec4 p1 = p.v1.pos;
            if(p1[idx] * wMult > p1.w){
                // point is outside the frustum
                p.rejected = true;
                return;
            }
        }

        // clip primitives so that they are contained within the render frustum
        void clipPrimitives() override  {
            // repeat for the six planes of the viewing frustum
            for (int side = 0; side < 6; side ++){
                for(auto & p : m_primitives){
                    if (!p.rejected)
                        clipPoint(p, side);
                }
            }
        }

        // perspective division (clipping space to normalized device coordinates)
        void divideByW() override {
            for(auto &p : m_primitives) {
                p.v1.pos.z /= p.v1.pos.w;
                p.v1 = p.v1 / p.v1.pos.w;
            }
        }

        // normalized device coordinates to screen space
        void toScreenSpace(int width, int height) override  {
            float halfW = width / 2;
            float halfH = height / 2;
            glm::mat4 toWindowSpace = glm::scale(glm::vec3(halfW, halfH, 1.f)) * glm::translate(glm::vec3(1.f, 1.f, 0.f));
            for(auto &p : m_primitives) {
                p.v1.pos = toWindowSpace * p.v1.pos;
            }
        }

        // rasterization (generate fragments)
        void rasterPrimitives(std::vector<fragment> &outFrs) override {
            outFrs.clear();

            for(auto &p : m_primitives) {
                // is current primitive visible?
                if(p.rejected)
                    continue;

                fragment frag{};
                frag.pos = glm::ivec2(p.v1.pos.x + .5f, p.v1.pos.y + .5f);
                frag.depth = p.v1.pos.z;
                frag.col = p.v1.col;
                frag.norm = p.v1.norm;
                frag.uv = p.v1.uv;

                outFrs.push_back(frag);
            }
        }


        // lists of point primitives, part of the class so that we avoid reallocating memory every frame
        std::vector<point> m_primitives;
    };

}
#endif //ITU_GRAPHICS_PROGRAMMING_SRL_POINT_RENDERER_H
