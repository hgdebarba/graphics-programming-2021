//
// Created by Henrique Galvan Debarba on 2019-09-27.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_RENDERER_H
#define GRAPHICSPROGRAMMINGEXERCISES_RENDERER_H

#include <vector>
#include <algorithm>
#include "glm/glm.hpp"
#include "srl_types.h"



namespace srl {


    class Renderer {

    public:

        // render vertices with mvp transformation in the fb framebuffer
        void render(const std::vector<vertex> &vts,
                            const glm::mat4 &m,
                            const glm::mat4 &vp,
                            CustomFrameBuffer <uint32_t> &fb,
                            CustomFrameBuffer <float> &db) {

            // will copy all vertices since vts is const
            std::vector<vertex> m_vts = vts;


            // 1. vertex shader equivalent
            processVertices(vp * m, m_vts);

            // 2. the fixed part of the pipeline
            std::vector<fragment> m_frs;

            // 2.1. create the primitives
            assemblePrimitives(m_vts);

            // 2.2. keep primitives in the visible volume
            clipPrimitives();


            // 2.3. move vertices to normalized device coordinates
            divideByW();

            // 2.4. normalized device coordinates to screen space
            toScreenSpace(fb.W, fb.H);

            // 2.5. reject primitives that are not facing towards the camera
            //if(m_cullBackFaces)
            backfaceCulling();

            // 2.6. rasterization (generate fragments)
            rasterPrimitives(m_frs);


            processFragments(m_frs);

            // 4. fragment operations and copy color to the frame buffer
            writeToFrameBuffer(m_frs, fb, db);
        }

        virtual ~Renderer(){};
    private:

        virtual void assemblePrimitives(const std::vector<vertex> &vts){};

        virtual void clipPrimitives() = 0;

        virtual void divideByW() = 0; // aka perspective divide

        virtual void toScreenSpace(int width, int height) = 0;

        virtual void backfaceCulling(){};

        virtual void rasterPrimitives(std::vector<fragment> &outFrs) = 0;

        // perform vertex operations in the vertex stream (i.e. the equivalent to a vertex shader)
        static void processVertices(const glm::mat4 &mvp, std::vector<vertex> &vInOut) {
            for (auto &vtx : vInOut){
                // this is the equivalent to a vertex shader
                vtx.pos = mvp * vtx.pos;
            }
        }

        // perform fragment operations in the fragment stream (i.e. fragment shader)
        static void processFragments(std::vector<fragment>& fInOut) {
            // fragment shader - not necessary for now since we are not modifying the color
            for (auto &frg : fInOut){
                // example: uncomment this to make all fragments darker
                // frg.col = frg.col * 0.5f;
            }
        }

        // fragment operations and copy color to frame buffer
        static void writeToFrameBuffer(const std::vector<fragment> &frs, CustomFrameBuffer <uint32_t> &fb, CustomFrameBuffer <float> &db) {
			int width = fb.W;
			int height = fb.H;
            for (int i = 0, size = frs.size(); i < size; i++) {
                glm::ivec2 pos = frs[i].pos;

                // make sure it is within framebuffer range (it won't be if we do not clip)
				if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height)
					continue;
				
				// blending test and z/depth-buffer will come here
				// set the color of the pixel in the frame buffer
				if (frs[i].depth < db.valueAt(pos.x, pos.y)) {
					fb.paintAt(pos.x, pos.y, Colors::toRGBA32(frs[i].col));
                    db.paintAt(pos.x, pos.y, frs[i].depth);
				}
            }
        }

    };



}

#endif //GRAPHICSPROGRAMMINGEXERCISES_RENDERER_H
