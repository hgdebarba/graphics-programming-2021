#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "trianglerasterizer.h"
#include "linerasterizer.h"
#include "CustomFrameBuffer.h"

void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);
void print_instructions();

// rasterization grid resolution
const int max_W = 32, max_H = 32;

// window resolution
const unsigned int SCR_WIDTH = max_W * 20;
const unsigned int SCR_HEIGHT = max_H * 20;

// triangle vertices with initial positions
int x_1  = max_W * .1, y_1  = max_H * .1 ;
int x_2  = max_W * .9, y_2  = max_H * .8;
int x_3  = max_W * .4, y_3  = max_H * .7;


bool showTriangleLines = false;
bool showTriangleFill = true;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 6", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_input_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    print_instructions();

    // initialize our custom frame buffer
    // ----------------------------------
    // every frame we will: draw to it, upload it to a texture, and copy the texture to the window frame buffer.
    CustomFrameBuffer customBuffer(max_W, max_H);


    // initialize texture we will use to upload our buffer to GPU
    // ----------------------------------------------------------
    unsigned int bufferTexture;

    glGenTextures(1, &bufferTexture);
    glBindTexture(GL_TEXTURE_2D, bufferTexture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // initialize openGL frame buffer object
    // ------------------------------------
    // this helps handling writing the texture content into the window frame buffer
    GLuint oglFrameBuffer = 0;
    glGenFramebuffers(1, &oglFrameBuffer);

    // render loop
    // -----------
    // render every loopInterval seconds
    float loopInterval = 1.f/60.f;
    auto begin = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;


        // render to our custom frame buffer
        // ---------------------------------
        customBuffer.clearBuffer(Colors::black);

        // paint the dots in the middle of the pixels
        for(int i = 0; i < customBuffer.W; i++){
            for(int j = 0; j < customBuffer.H; j++)
                customBuffer.paintAt(i, j, Colors::dark, CustomFrameBuffer::fill::frame);
        }


        if (showTriangleFill) {
            // paint the filled pixels (triangle rasterization)
            std::vector<glm::ivec2> pixels;
            triangle_rasterizer triangle(x_1, y_1, x_2, y_2, x_3, y_3);
            // run rasterization
            pixels = triangle.all_pixels();
            for (auto it = pixels.begin(); it < pixels.end(); it++) {
                customBuffer.paintAt(it->x, it->y, Colors::green, CustomFrameBuffer::fill::center);
            }
        }

        if (showTriangleLines) {
            // paint the lines connecting the vertices (line rasterizer)
            std::vector<glm::ivec2> pixelsLine;
            // run rasterization
            LineRasterizer lines[3] = {LineRasterizer(x_1, y_1, x_2, y_2),
                                       LineRasterizer(x_2, y_2, x_3, y_3),
                                       LineRasterizer(x_3, y_3, x_1, y_1)};
            for (auto &l: lines) {
                pixelsLine = l.all_pixels();
                for (auto &p: pixelsLine) {
                    customBuffer.paintAt(p.x, p.y, Colors::white, CustomFrameBuffer::fill::center);
                }
            }
        }

        // paint the triangle vertices
        customBuffer.paintAt(x_1,y_1, Colors::blue, CustomFrameBuffer::fill::frame);
        customBuffer.paintAt(x_2,y_2, Colors::blue, CustomFrameBuffer::fill::frame);
        customBuffer.paintAt(x_3,y_3, Colors::blue, CustomFrameBuffer::fill::frame);


        // show our rendered triangle
        // --------------------------

        // upload the custom color buffer to the GPU using the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, max_W*3, max_H*3, 0, GL_RGBA, GL_UNSIGNED_BYTE, customBuffer.buffer);

        // set opengl frame buffer object to read from our texture, we will copy from it
        glBindFramebuffer(GL_READ_FRAMEBUFFER, oglFrameBuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTexture, 0);

        // bind the window frame buffer, where we want to copy the contents of the texture to
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // copy from opengl the frame buffer object (access the texture) to the window frame buffer
        glBlitFramebuffer(0,0, max_W * 3, max_H * 3,0,0,SCR_WIDTH,SCR_HEIGHT,GL_COLOR_BUFFER_BIT, GL_NEAREST);


        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency (busy wait)
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void print_instructions(){
    // print the use instructions
    // --------------------------
    std::cout << std::endl;
    std::cout << "***********************************************************" << std::endl;
    std::cout << "* Use the keys to manipulate the vertices of the triangle *" << std::endl;
    std::cout << "* All three vertices can be modified:                     *" << std::endl;
    std::cout << "*                                                         *" << std::endl;
    std::cout << "*     vertex 1     |     vertex 2     |     vertex 3      *" << std::endl;
    std::cout << "*                  |                  |                   *" << std::endl;
    std::cout << "*        W         |        T         |        I          *" << std::endl;
    std::cout << "*      A S D       |      F G H       |      J K L        *" << std::endl;
    std::cout << "*                                                         *" << std::endl;
    std::cout << "* Press 1 to toggle triangle lines (" << (showTriangleLines ? "ON " : "OFF") << ")                  *" << std::endl;
    std::cout << "* Press 2 to toggle triangle fill  (" << (showTriangleFill  ? "ON " : "OFF") << ")                  *" << std::endl;
    std::cout << "* Press ESC to finish the program                         *" << std::endl;
    std::cout << "***********************************************************" << std::endl;
    std::cout << std::endl;
}

void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    if (button == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (action != GLFW_PRESS)
        return;

    if (button == GLFW_KEY_1) showTriangleLines = !showTriangleLines, print_instructions();
    if (button == GLFW_KEY_2) showTriangleFill = !showTriangleFill, print_instructions();

    // move triangle vertices
    if (button == GLFW_KEY_A) x_1 -= 1;
    if (button == GLFW_KEY_D) x_1 += 1;
    if (button == GLFW_KEY_S) y_1 -= 1;
    if (button == GLFW_KEY_W) y_1 += 1;

    if (button == GLFW_KEY_F) x_2 -= 1;
    if (button == GLFW_KEY_H) x_2 += 1;
    if (button == GLFW_KEY_G) y_2 -= 1;
    if (button == GLFW_KEY_T) y_2 += 1;

    if (button == GLFW_KEY_J) x_3 -= 1;
    if (button == GLFW_KEY_L) x_3 += 1;
    if (button == GLFW_KEY_K) y_3 -= 1;
    if (button == GLFW_KEY_I) y_3 += 1;

    // ensure that the vertices are in a valid range
    x_1 = x_1 >= max_W ? max_W-1 : (x_1 < 0 ? 0 : x_1);
    y_1 = y_1 >= max_H ? max_H-1 : (y_1 < 0 ? 0 : y_1);
    x_2 = x_2 >= max_W ? max_W-1 : (x_2 < 0 ? 0 : x_2);
    y_2 = y_2 >= max_H ? max_H-1 : (y_2 < 0 ? 0 : y_2);
    x_3 = x_3 >= max_W ? max_W-1 : (x_3 < 0 ? 0 : x_3);
    y_3 = y_3 >= max_H ? max_H-1 : (y_3 < 0 ? 0 : y_3);

}

