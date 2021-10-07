//
// Created by henrique debarba on 07/10/2021.
//

#include <assert.h>
#include "CustomFrameBuffer.h"

CustomFrameBuffer::CustomFrameBuffer(uint32_t width, uint32_t height) : W(width), H(height){
    if (buffer != nullptr) // ensure that there is no buffer allocated ...
        delete buffer;
    int size = W * H * 3 * 3; // (the hardcoded 3s are the width and height of each "pixel")
    buffer = new Colors::color[size]; // ... and allocate one
    clearBuffer();
}

CustomFrameBuffer::~CustomFrameBuffer(){
    delete[] buffer; // clean our memory
}

void CustomFrameBuffer::clearBuffer(Colors::color col){
    int size = W * H * 3 * 3;
    for(int i = 0; i < size; i++)
        buffer[i] = col;
}

void CustomFrameBuffer::paintAt(uint32_t x, uint32_t y, Colors::color col, CustomFrameBuffer::fill fillOption){
    assert (x < W && y < H); // ensure valid position, crash if not (sooo dramatic!)

    // location of the center pixel in the buffer
    int px = (x * 3 + 1) + (y * 3 + 1) * W * 3;

    if (fillOption != CustomFrameBuffer::fill::center) {
        // fill out the 3x3 range
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                if (fillOption == CustomFrameBuffer::fill::cross && abs(i) + abs(j) > 1)
                    continue; // skip corners
                if (fillOption == CustomFrameBuffer::fill::checkboard && abs(i) + abs(j) != 1)
                    continue;
                if (fillOption == CustomFrameBuffer::fill::frame && abs(i) + abs(j) == 0)
                    continue;
                int currentPx = px + j * 3 * W + i;
                buffer[currentPx] = col;
            }
        }
    } else { // fill out only the center
        buffer[px] = col;
    }
}