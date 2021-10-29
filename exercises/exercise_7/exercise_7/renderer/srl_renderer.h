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

            // TODO exercise 7 / assignment 3
            //  to make the Software Render Library work, you have to call all methods
            //  in this class, in the right order and with the right parameters.

            std::vector<vertex> _vts = vts; // copy all vertices from vts to _vts (since vts is a const)
            std::vector<fragment> _frs;    // vector that will store the fragments
            glm::mat4 modelViewProjection = vp * m; // the matrix that transform points from local space to clipping space

            //  MIND THAT THE METHODS BELOW ARE NOT DECLARED/DEFINED IN THE RIGHT ORDER!

        }

        virtual ~Renderer(){};
    private:

        virtual void assemblePrimitives(const std::vector<vertex> &vts) = 0;
        // performs the perspective division

        // remove all geometry outside the visible volume (performed in clipping space)
        virtual void clipPrimitives() = 0;
        // test if the surface of the primitive is visible to the camera
        // only used when rendering triangles.
        virtual void backfaceCulling(){};

        // (i.e. transforms from the clipping space to the normalized device coordinates)
        virtual void divideByW() = 0;
        // transform from normalized device coordinates to window coordinates
        virtual void toScreenSpace(int width, int height) = 0;
        // generate the fragments, with final window pixel locations, used to render the primitives
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
        // blending test and z/depth-buffer can come here
        static void writeToFrameBuffer(const std::vector<fragment> &frs, CustomFrameBuffer <uint32_t> &fb, CustomFrameBuffer <float> &db) {
			int width = fb.W;
			int height = fb.H;
            for (int i = 0, size = frs.size(); i < size; i++) {
                glm::ivec2 pos = frs[i].pos;

                // make sure it is within framebuffer range (it won't be if we do not clip)
				if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height)
					continue;

				// z/depth-test algorithm:
				if (frs[i].depth < db.valueAt(pos.x, pos.y)) {
                    // is the new fragment closer? Then update the color and the depth buffer
					fb.paintAt(pos.x, pos.y, Colors::toRGBA32(frs[i].col));
                    db.paintAt(pos.x, pos.y, frs[i].depth);
				}
            }
        }
    };
}

#endif //GRAPHICSPROGRAMMINGEXERCISES_RENDERER_H
