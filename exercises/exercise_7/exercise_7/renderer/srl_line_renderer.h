//
// Created by Henrique Galvan Debarba on 2019-09-27.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_OGLLINERENDERER_H
#define GRAPHICSPROGRAMMINGEXERCISES_OGLLINERENDERER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "srl_renderer.h"
#include "rasterizer/linerasterizer.h"
#include "srl_types.h"

namespace srl {
    class LineRenderer : public Renderer {
    private:
        // create line primitives
        void assemblePrimitives(const std::vector<vertex> &vts) {
            m_primitives.clear();
            // make sure a single allocation will happen
            m_primitives.reserve(vts.size()/3 * (wireframe ? 3 : 1));
            int increment =  wireframe ? 3 : 2;
            for(int i = 0, size = vts.size()-1; i < size; i += increment){
                line l;
                l.v1 = vts[i];
                l.v2 = vts[i+1];
                m_primitives.push_back(l);
                if(wireframe) {
                    l.v1 = vts[i + 1];
                    l.v2 = vts[i + 2];
                    m_primitives.push_back(l);
                    l.v1 = vts[i + 2];
                    l.v2 = vts[i];
                    m_primitives.push_back(l);
                }
            }
        }

        void clipLine(line &l, int side){
            vertex &v1 = l.v1;
            vertex &v2 = l.v2;

            // index to x, y or z coordinate (x=0, y=1, z=2)
            int idx = side % 3;
            // we check if the variable is in the range of the clipping plane using w
            // we need to test if w >= x,y,z >= -w, and clip when x,y,z > w or x,y,z < -w
            // we can rewrite the latter with x,y,z * -1 > w
            // so we need to multiply x,y,z by -1 when testing against the planes at -w
            // planes 0, 1 and 2 are positive w, planes 3, 4 and 5 are negative w
            float wMult = side > 2 ? -1.0f : 1.0f;

            glm::vec4 p1 = v1.pos;
            glm::vec4 p2 = v2.pos;
            int outCount = (p1[idx] * wMult > p1.w) + (p2[idx] * wMult > p2.w);
            if( outCount == 2){
                // the line is outside the frustum, we don't need to draw it
                l.rejected = true;
                return;
            }
            else if (outCount == 0){
                // no need to clip against this plane
                return;
            }
            else { // (outCount == 1)  one vertex is inside and the other is outside

                // vector from position p1 to p2
                glm::vec4 p1p2vec = p2 - p1;

                // proportion t that added to p1 will give the point where coordinates p1[idx] + p1p2vec[idx]*t == w , for idx = x, y or z
                float denom = p1p2vec.w * wMult - p1p2vec[idx];
                float t = (p1[idx] - p1.w * wMult) / denom;

                // interpolate and update the value of one of the variables
                vertex &vTarget = p1[idx] * wMult > p1.w ? v1 : v2;
                vTarget = v1 + (v2 - v1) * t;
            }
        }

        // clip primitives so that they are contained within the render frustum
        void clipPrimitives()  {
            // repeat for the six planes of the viewing frustum
            for (int side = 0; side < 6; side ++){
                for(int i = 0, size = m_primitives.size(); i < size; i++){
                    if (!m_primitives[i].rejected)
                        clipLine(m_primitives[i], side);
                }
            }
        }

        // perspective division (canonical perspective volume to normalized device coordinates)
        void divideByW() {
            for(auto &line : m_primitives) {
                line.v1.pos.z /= line.v1.pos.w;
                line.v1 = line.v1 / line.v1.pos.w;
                line.v2.pos.z /= line.v2.pos.w;
                line.v2 = line.v2 / line.v2.pos.w;
            }
        }

        // normalized device coordinates to screen space
        void toScreenSpace(int width, int height)  {
            float halfW = width / 2;
            float halfH = height / 2;
            glm::mat4 toWindowSpace = glm::scale(glm::vec3(halfW, halfH, 1.f)) * glm::translate(glm::vec3(1.f, 1.f, 0.f));
            for(auto &line : m_primitives) {
                line.v1.pos = toWindowSpace * line.v1.pos;
                line.v2.pos = toWindowSpace * line.v2.pos;
            }
        }

        // rasterization (generate fragments)
        void rasterPrimitives(std::vector<fragment> &outFrs) {
            outFrs.clear();

            for(auto &line : m_primitives) {
                // is current primitive visible?
                if(line.rejected)
                    continue;

                // vertices of the line rounded to the closest integer (aka pixel location)
                glm::ivec2 iv1(line.v1.pos.x + .5f, line.v1.pos.y + .5f);
                glm::ivec2 iv2(line.v2.pos.x + .5f, line.v2.pos.y + .5f);
                // run the rasterization and collect all pixel locations
                LineRasterizer rasterizer(iv1.x, iv1.y, iv2.x, iv2.y);
                std::vector<glm::ivec2> pixels = rasterizer.all_pixels();

                // create a fragment for each pixel in the rasterization
                for (auto &pxl : pixels){
                    fragment frag;

                    frag.pos = pxl;
                    // screen space interpolation factor
                    float interp = glm::length(glm::vec2(pxl - iv1)) / glm::length(glm::vec2(iv2 - iv1));
                    // hyperbolic interpolation correction
                    float hypInterp = interp * line.v2.hypInterp + (1.f-interp) * line.v1.hypInterp;
                    // interpolate and then apply the correction
                    frag.depth = (interp * line.v2.pos.z + (1.f-interp) * line.v1.pos.z) / hypInterp;
                    frag.col = (interp * line.v2.col + (1.f-interp) *line.v1.col) / hypInterp;

                    outFrs.push_back(frag);
                }
            }
        }

        // lists of line primitives.
        std::vector<line> m_primitives;
        bool wireframe = true;
    };

}

#endif //GRAPHICSPROGRAMMINGEXERCISES_OGLLINERENDERER_H
