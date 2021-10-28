//
// Created by henrique debarba on 07/10/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_CUSTOMFRAMEBUFFER_H
#define ITU_GRAPHICS_PROGRAMMING_CUSTOMFRAMEBUFFER_H

#include <cstdint>
#include <stdlib.h>

namespace Colors{
    // colors are 32 bits unsigned ints, so it is easy to upload to the GPU as a texture
    typedef uint32_t color;
    // hardcoded some colors for convenience
    // 8bits: alpha (not used) | 8bits: blue | 8bits: green | 8bits: red
    const color white = 0xFFFFFFFF;
    const color grey  = 0xFF808080;
    const color dark  = 0xFF202020;
    const color black = 0xFF000000;
    const color blue  = 0xFFFF0000;
    const color green = 0xFF00FF00;
    const color red   = 0xFF0000FF;
}


class CustomFrameBuffer {
public:
    // solid paints in a 3x3 range, center paints only the pixel in the center, cross don't paint the corners
    enum fill {solid, cross, center, checkboard, frame};
    uint32_t W = 16;
    uint32_t H = 16;
    Colors::color *buffer = nullptr;

    CustomFrameBuffer(uint32_t width, uint32_t height);
    ~CustomFrameBuffer();

    void clearBuffer(Colors::color col = Colors::black) const;
    void paintAt(uint32_t x, uint32_t y, Colors::color col, fill fillOption = fill::center) const;

};


#endif //ITU_GRAPHICS_PROGRAMMING_CUSTOMFRAMEBUFFER_H