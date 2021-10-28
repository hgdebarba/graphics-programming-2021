//
// Created by Henrique Galvan Debarba on 2019-09-27.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_OGLTRIANGLERENDERER_H
#define GRAPHICSPROGRAMMINGEXERCISES_OGLTRIANGLERENDERER_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "srl_renderer.h"
#include "rasterizer/trianglerasterizer.h"
#include <glm/gtc/matrix_access.hpp>
#include <iostream>
#include "srl_types.h"

namespace srl {

    class TriangleRenderer : public Renderer {
    public:
        bool m_clipToFrustum = true;

    private:

        // create triangle primitives
        void assemblePrimitives(const std::vector<vertex> &vts) override {
            m_primitives.clear();
            m_primitives.reserve(vts.size()/3);

            for(int i = 0, size = vts.size()-2; i < size; i+=3){
                triangle t;
                t.v1 = vts[i];
                t.v2 = vts[i+1];
                t.v3 = vts[i+2];

                m_primitives.push_back(t);
            }
        }

        bool clipTriangle(triangle &tIn, int i){
            // index to x, y or z coordinate (x=0, y=1, z=2)
            int idx = i % 3;
            // we check if the variable is in the range of the clipping plane using w
            // we need to test if w >= x,y,z >= -w, and clip when x,y,z > w or x,y,z < -w
            // we can rewrite the latter with x,y,z * -1 > w
            // so we need to multiply x,y,z by -1 when testing against the planes at -w
            // planes 0, 1 and 2 are positive w, planes 3, 4 and 5 are negative w
            int wMult = i > 2 ? -1 : 1;

            // positions of the three vertex
            glm::vec4 p1 = tIn.v1.pos;
            glm::vec4 p2 = tIn.v2.pos;
            glm::vec4 p3 = tIn.v3.pos;

            // store a pointer to the vertices in and out the desired half-space
            vertex* inVts[3]; int inCount = 0;
            vertex* outVts[3]; int outCount = 0;
            int outIdx;

            // test if the points are in the valid
            if(p1[idx] * wMult > p1.w) {outVts[outCount] = &tIn.v1; outCount++; outIdx = 0;}
            else {inVts[inCount] = &tIn.v1; inCount++;}
            if(p2[idx] * wMult > p2.w) {outVts[outCount] = &tIn.v2; outCount++; outIdx = 1;}
            else {inVts[inCount] = &tIn.v2; inCount++;}
            if(p3[idx] * wMult > p3.w) {outVts[outCount] = &tIn.v3; outCount++; outIdx = 2;}
            else {inVts[inCount] = &tIn.v3; inCount++;}


            if (outCount == 0) {
                // whole triangle in the valid side of the half-space (or over the plane)
                return true;
            }
            else if (outCount == 3) {
                // whole triangle in the invalid side of the half-space
                // reject this triangle
                tIn.rejected = true;
                return false;
            }
            else if (outCount == 2) {   // two vertices in the invalid side of the half-space
                // vector from in position to first out position
                glm::vec4 inOutVec = outVts[0]->pos - inVts[0]->pos;
                // find the weight t
                float t = (inVts[0]->pos[idx] - inVts[0]->pos.w * wMult) / (inOutVec.w * wMult - inOutVec[idx]);
                // compute edge intersection 1
                vertex edgeVtx1 = (*inVts[0]) + (*outVts[0] - *inVts[0]) * t;

                // vector from in position to second out position
                inOutVec = outVts[1]->pos - inVts[0]->pos;
                // find the weight t
                t = (inVts[0]->pos[idx] - inVts[0]->pos.w * wMult) / (inOutVec.w * wMult - inOutVec[idx]);
                // compute edge intersection 2
                vertex edgeVtx2 = (*inVts[0]) + (*outVts[1] - *inVts[0]) * t;

                // update the two triangle vertices in the invalid half-space
                *(outVts[0]) = edgeVtx1;
                *(outVts[1]) = edgeVtx2;
            }
            else if (outCount == 1) {   // one vertex in the invalid side of the half-space

                // vector from first in position to out position
                glm::vec4 inOutVec = outVts[0]->pos - inVts[0]->pos;
                // find the weight t
                float t = (inVts[0]->pos[idx] - inVts[0]->pos.w * wMult) / (inOutVec.w * wMult - inOutVec[idx]);
                // compute edge intersection 1
                vertex edgeVtx1 = (*inVts[0]) + (*outVts[0] - *inVts[0]) * t;

                // vector from second in position to out position
                inOutVec = outVts[0]->pos - inVts[1]->pos;
                // find the weight t
                t = (inVts[1]->pos[idx] - inVts[1]->pos.w * wMult) / (inOutVec.w * wMult - inOutVec[idx]);
                // compute edge intersection 2
                vertex edgeVtx2 = (*inVts[1]) + (*outVts[0] - *inVts[1]) * t;

                // update the location of the vertex in the invalid side of the half-space
                *outVts[0] = edgeVtx1;

                // we have fixed the triangle that was already stored, now lets create the triangle that is missing
                // using the two edge points and the second in vertex
                triangle newT;
                // ensure the winding order of new triangles is correct (so that they are not culled during backface culling)
                if(outIdx == 0){newT.v1 = *inVts[1]; newT.v2 = edgeVtx2; newT.v3 = edgeVtx1;}
                else if(outIdx == 1){newT.v1 =  *inVts[1]; newT.v2 = edgeVtx1; newT.v3 = edgeVtx2;}
                else {newT.v1 = edgeVtx1; newT.v2 = *inVts[1]; newT.v3 = edgeVtx2;}

                m_primitives.push_back(newT);
            }

            return true;
        }


        // clip primitives so that they are contained within the render volume
        void clipPrimitives() override {
            for (int side = 0; side < 6; side ++){
                for(int i = 0, size = m_primitives.size(); i < size; i++){
                    if (!m_primitives[i].rejected)
                        clipTriangle(m_primitives[i], side);
                }
            }
        }

        // perspective division (canonical perspective volume to normalized device coordinates)
        void divideByW() override {
            for(auto &tri : m_primitives) {
                // the division of position x, y and z coordinates will place all vertices in the normalized device coordinates
                // however, we divide all parameters (not only position) to perform hyperbolic interpolation later on
                tri.v1.pos.z = tri.v1.pos.z / tri.v1.pos.w;
                tri.v1 = tri.v1 / tri.v1.pos.w;
                tri.v2.pos.z = tri.v2.pos.z / tri.v2.pos.w;
                tri.v2 = tri.v2 / tri.v2.pos.w;
                tri.v3.pos.z = tri.v3.pos.z / tri.v3.pos.w;
                tri.v3 = tri.v3 / tri.v3.pos.w;

            }
        }

        // normalized device coordinates to window coordinates
        void toScreenSpace(int width, int height) override  {
            float halfW = width / 2;
            float halfH = height / 2;
            glm::mat4 toWindowSpace = glm::scale(glm::vec3(halfW, halfH, 1.f)) * glm::translate(glm::vec3(1.f, 1.f, 0.f));
            for(auto &tri : m_primitives) {
                tri.v1.pos = toWindowSpace * tri.v1.pos;
                tri.v2.pos = toWindowSpace * tri.v2.pos;
                tri.v3.pos = toWindowSpace * tri.v3.pos;
            }
        }


        // only draw triangles in a counterclockwise winding order (which we define as facing the camera)
        void backfaceCulling() override{
            for(auto &tri : m_primitives) {
                // two vectors along the edges of the triangle
                glm::vec3 v1 = tri.v2.pos - tri.v1.pos;
                glm::vec3 v2 = tri.v3.pos - tri.v1.pos;

                // z component of the normal in the NDC
                float nz = v1.x * v2.y - v1.y * v2.x;

                // bigger than 0 means the normal is not pointing towards the camera
                if (nz < 0) {
                    tri.rejected = true;
                }
            }
        }

        // rasterize the triangle and generate the fragments (outFrs)
        void rasterPrimitives(std::vector<fragment> &outFrs) override {
            outFrs.clear();

            for(auto &tri : m_primitives) {
                // skip this primitive if it has been rejected during clipping or culling
                if(tri.rejected)
                    continue;

                // vertices of the triangle, rounded to the closest integer (aka pixel location)
                glm::ivec2 iv1(tri.v1.pos.x + .5f, tri.v1.pos.y + .5f);
                glm::ivec2 iv2(tri.v2.pos.x + .5f, tri.v2.pos.y + .5f);
                glm::ivec2 iv3(tri.v3.pos.x + .5f, tri.v3.pos.y + .5f);
                // run the rasterization and collect all pixel locations
                triangle_rasterizer rasterizer(iv1.x, iv1.y, iv2.x, iv2.y, iv3.x, iv3.y);
                std::vector<glm::ivec2> pixels = rasterizer.all_pixels();

                // create a fragment for each pixel
                for (auto &pxl : pixels){
                    fragment frag{};

                    frag.pos = pxl;

                    // barycentric coordinates (in 2D projected space)
                    glm::vec3 bar = tri.barycentricCoordinatesAt(pxl);
                    // hyperbolic interpolation correction
                    float hypInterp = bar.x * tri.v1.hypInterp + bar.y * tri.v2.hypInterp + bar.z * tri.v3.hypInterp;
                    bar = bar / hypInterp;
                    frag.depth = bar.x * tri.v1.pos.z + bar.y * tri.v2.pos.z + bar.z * tri.v3.pos.z;
                    frag.col = bar.x * tri.v1.col + bar.y * tri.v2.col + bar.z * tri.v3.col;
                    frag.norm = bar.x * tri.v1.norm + bar.y * tri.v2.norm + bar.z * tri.v3.norm;
                    frag.uv = bar.x * tri.v1.uv + bar.y * tri.v2.uv + bar.z * tri.v3.uv;


                    outFrs.push_back(frag);
                }
            }
        }


        // lists of triangle primitives, part of the class so that we avoid reallocating memory every frame
        std::vector<triangle> m_primitives;
    };

}

#endif //GRAPHICSPROGRAMMINGEXERCISES_OGLTRIANGLERENDERER_H
