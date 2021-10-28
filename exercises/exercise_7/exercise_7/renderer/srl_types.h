//
// Created by henrique debarba on 28/10/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_SRL_TYPES_H
#define ITU_GRAPHICS_PROGRAMMING_SRL_TYPES_H



namespace srl {

    template<class T>
    class CustomFrameBuffer {
    public:
        unsigned int W, H;
        T *buffer;

        CustomFrameBuffer(unsigned int width, unsigned int height): W(width), H(height) {
            buffer = new T[W * H];
        }

        ~CustomFrameBuffer(){delete[] buffer;} // clean our memory

        void clearBuffer(T value){
            int size = W * H;
            for (int i = 0; i < size; i++)
                buffer[i] = value;
        }

        void paintAt(unsigned int x, unsigned int y, T value){
            assert (x < W && y < H); // ensure valid position, crash if not (sooo dramatic!)
            buffer[x + y * W] = value;
        }

        T valueAt(unsigned int x, unsigned int y){
            assert (x < W && y < H);
            return buffer[x + y * W];
        }

    };

    namespace Colors {
        // colors are 32 bits unsigned ints, so it is easy to upload to the GPU as a texture
        //typedef uint32_t color;
        typedef glm::vec4 color;

        // hardcoded some colors for convenience
        const color white = glm::vec4(1, 1, 1, 1);
        const color grey = glm::vec4(.5, .5, .5, 1);;
        const color dark = glm::vec4(.15, .15, .15, 1);
        const color black = glm::vec4(0, 0, 0, 1);
        const color blue = glm::vec4(0, 0, 1, 1);
        const color green = glm::vec4(0, 1, 0, 1);
        const color red = glm::vec4(1, 0, 0, 1);

        inline std::uint32_t toRGBA32(color c) {
            // convert color to four 8 bits uint, packed in a 32 bits uint.
            // We do that because that is the proper format for the color buffer that renders to the screen
            return (uint32_t(255 * c.r)) + (uint32_t(255 * c.g) << 8) +
                   (uint32_t(255 * c.b) << 16) + (uint32_t(255 * c.a) << 24);
        }
    }

    // VERTEX AND FRAGMENT
    // -------------------
    struct vertex {
        glm::vec4 pos;
        glm::vec4 norm;
        Colors::color col;
        glm::vec2 uv;
        float hypInterp = 1.0;

        friend vertex operator/(vertex v, float sc) {
            return vertex{v.pos / sc, v.norm / sc, v.col / sc, v.uv / sc, v.hypInterp / sc};
        }

        friend vertex operator*(vertex v, float sc) {
            return vertex{v.pos * sc, v.norm * sc, v.col * sc, v.uv * sc, v.hypInterp * sc};
        }

        friend vertex operator-(vertex v1, const vertex &v2) {
            return vertex{v1.pos - v2.pos, v1.norm - v2.norm, v1.col - v2.col, v1.uv - v2.uv,
                          v1.hypInterp - v2.hypInterp};
        }

        friend vertex operator+(vertex v1, const vertex &v2) {
            return vertex{v1.pos + v2.pos, v1.norm + v2.norm, v1.col + v2.col, v1.uv + v2.uv,
                          v1.hypInterp + v2.hypInterp};
        }
    };

    struct fragment {
        glm::vec4 norm;
        Colors::color col;
        glm::ivec2 pos;
        glm::vec2 uv;
        float depth;
    };


    // PRIMITIVES
    // ----------
    struct point {
        vertex v1;
        bool rejected = false;
    };

    struct line {
        vertex v1;
        vertex v2;
        bool rejected = false;
    };

    struct triangle {
        vertex v1;
        vertex v2;
        vertex v3;
        glm::ivec2 p1, p2, p3;
        bool rejected = false;

        glm::mat2x2 inverse = glm::mat2x2(1.0f);
        bool inverseReady = false;

        glm::vec3 barycentricCoordinatesAt(glm::vec2 at){
            if(!inverseReady){
                // we only need to compute this inverse once per triangle
                inverse[0] = glm::vec2(v1.pos.x - v3.pos.x, v1.pos.y - v3.pos.y);
                inverse[1] = glm::vec2(v2.pos.x - v3.pos.x, v2.pos.y - v3.pos.y);
                inverse = glm::inverse(inverse);
            }
            glm::vec3 barycentric = glm::vec3(inverse * (at - glm::vec2(v3.pos.x, v3.pos.y)), 0);
            barycentric.z = 1.0f - barycentric.x - barycentric.y;

            return barycentric;
        }
    };
}

#endif //ITU_GRAPHICS_PROGRAMMING_SRL_TYPES_H
