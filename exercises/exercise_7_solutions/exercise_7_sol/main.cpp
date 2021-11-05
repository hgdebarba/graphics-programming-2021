#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <chrono>

#include "srl_point_renderer.h"
#include "srl_line_renderer.h"
#include "srl_triangle_renderer.h"
#include "primitives.h"

// glfw callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void button_input_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_input_callback(GLFWwindow* window, double posX, double posY);
void key_input_callback(GLFWwindow* window, int button, int other, int action, int mods);

// functions and global variables used for control
glm::mat4 trackballRotation();
void cursorInNdc(float screenX, float screenY, int screenW, int screenH, float &x, float &y);
glm::vec3 clickStart(0.0f), clickEnd(0.0f);
glm::mat4 storedRotation(1.0f);

// rasterization grid resolution
const int max_W = 64, max_H = 64;

// window resolution
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

srl::PointRenderer pRenderer;
srl::LineRenderer lRenderer;
srl::TriangleRenderer tRenderer;
srl::Renderer* srlRenderer = &tRenderer;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Exercise 7", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, button_input_callback);
    glfwSetCursorPosCallback(window, cursor_input_callback);
    glfwSetKeyCallback(window, key_input_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // load the 3D model
    // -----------------
    std::vector<glm::vec3> points;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    Primitives::makeCube(2.f, points, normals, uvs, colors);

    std::vector<srl::vertex> vtsCube;
    for (unsigned int i = 0; i < points.size(); i++){
        srl::vertex v{glm::vec4(points[i], 1.0f),
                    glm::vec4(normals[i], 0),
                    colors[i],
                    uvs[i]
        };
        vtsCube.push_back(v);
    }


    // camera
    // ------
    // create our camera pose and projection matrix, our camera is static, so we create it outside the loop
    glm::mat4 viewProj = glm::perspectiveFov<float>(glm::radians(70.0f),
                                                         (float)max_W , (float)max_H, .5f, 5.0f)
                         * glm::lookAt<float>(glm::vec3(.0f, .0f, 2.5f),
                                              glm::vec3(.0f, .0f, .0f),
                                              glm::vec3(.0f, 1.f, .0f));


    // initialize our custom frame buffer
    // ----------------------------------
    // every frame we will: draw to it, upload it to a texture, and copy the texture to the window frame buffer.
    srl::CustomFrameBuffer<std::uint32_t> customBuffer(max_W, max_H);
    srl::CustomFrameBuffer<float> customZBuffer(max_W, max_H);


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

    std::cout << "Key mapping:" << std::endl;
    std::cout << "1 - use point renderer" << std::endl;
    std::cout << "2 - use line renderer" << std::endl;
    std::cout << "3 - use triangle renderer" << std::endl;

    while (!glfwWindowShouldClose(window))
    {
        // update current time
        auto frameStart = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> appTime = frameStart - begin;


        // render to our custom frame buffer
        // ---------------------------------
        customBuffer.clearBuffer(srl::Colors::toRGBA32(srl::Colors::black));
        customZBuffer.clearBuffer(1.0f);

        srlRenderer->render(vtsCube, trackballRotation() * storedRotation, viewProj, customBuffer, customZBuffer);

        // show our rendered image
        // -----------------------
        // upload the custom color buffer to the GPU using the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bufferTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, max_W, max_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, customBuffer.buffer);

        // set opengl frame buffer object to read from our texture, we will copy from it
        glBindFramebuffer(GL_READ_FRAMEBUFFER, oglFrameBuffer);
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTexture, 0);

        // bind the window frame buffer, where we want to copy the contents of the texture to
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        // copy from the frame buffer object (access the texture) to the window frame buffer
        int size_W, size_H;
        glfwGetFramebufferSize(window, &size_W, &size_H);
        glBlitFramebuffer(0,0, max_W, max_H, 0, 0, size_W, size_H, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // display frame buffer
        glfwSwapBuffers(window);
        glfwPollEvents();

        // control render loop frequency (busy wait)
        std::chrono::duration<float> elapsed = std::chrono::high_resolution_clock::now()-frameStart;
        while (loopInterval > elapsed.count()) {
            elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        }
        glfwSetWindowTitle(window, ("Exercise 9 - FPS: " + std::to_string(int(1.0f/elapsed.count() + .5f))).c_str());
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


glm::mat4 trackballRotation(){
    glm::vec2 mouseVec = clickStart-clickEnd;
    if (glm::length(mouseVec) < 1e-5)
        return glm::mat4(1.0f);

    float dotProd, angle, r = 1.0;
    glm::vec3 u, crossProd;
    glm::vec3 pa, pc;

    // Anderson trackball
    pa = glm::length(clickStart) <= r/sqrt(2.0f) ?
         glm::vec3(clickStart.x, clickStart.y, sqrt(r*r - (clickStart.x*clickStart.x + clickStart.y*clickStart.y))) :
         glm::vec3(clickStart.x, clickStart.y, r*r/(glm::length(clickStart) * 2.0f));

    pc = glm::length(clickEnd) <= r/sqrt(2.0f) ?
         glm::vec3(clickEnd.x, clickEnd.y, sqrt(r*r - (clickEnd.x*clickEnd.x + clickEnd.y*clickEnd.y))) :
         glm::vec3(clickEnd.x, clickEnd.y, r*r/(glm::length(clickEnd) * 2.0f ));

    dotProd = glm::dot(pa, pc);
    crossProd = glm::cross(pa, pc);
    u = crossProd / glm::length(crossProd);
    angle = atan(glm::length(crossProd) / dotProd);

    // correction to the rotation angle
    angle += dotProd < 0.f ? glm::pi<float>() : 0.f;
    glm::mat4 rotation = glm::rotate(abs(angle), u);

    return rotation;
}

void cursorInNdc(float screenX, float screenY, int screenW, int screenH, float &x, float &y){
    float xNdc = (float) screenX / (float) screenW * 2.0f - 1.0f;
    float yNdc = (float) screenY / (float) screenH * 2.0f - 1.0f;
    x = xNdc;
    y = -yNdc;
}

void cursor_input_callback(GLFWwindow* window, double posX, double posY){
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
        int screenW, screenH;
        glfwGetWindowSize(window, &screenW, &screenH);
        cursorInNdc((float)posX, (float)posY, screenW, screenH, clickEnd.x, clickEnd.y);
    }
}

void button_input_callback(GLFWwindow* window, int button, int action, int mods){
    double screenX, screenY;
    int screenW, screenH;
    glfwGetCursorPos(window, &screenX, &screenY);
    glfwGetWindowSize(window, &screenW, &screenH);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        cursorInNdc((float)screenX, (float)screenY, screenW, screenH, clickStart.x, clickStart.y);
        // reset clickEnd position
        cursorInNdc((float)screenX, (float)screenY, screenW, screenH, clickEnd.x, clickEnd.y);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        cursorInNdc((float)screenX, (float)screenY, screenW, screenH, clickEnd.x, clickEnd.y);
        // store current rotation at the end of a click
        storedRotation = trackballRotation() * storedRotation;
        // reset click positions
        clickStart.x = clickStart.y = clickEnd.x = clickEnd.y = 0;
    }
}

void key_input_callback(GLFWwindow* window, int button, int other,int action, int mods){
    if (button == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (button == GLFW_KEY_1 && action == GLFW_PRESS) {
        srlRenderer = &pRenderer;
    }
    if (button == GLFW_KEY_2 && action == GLFW_PRESS) {
        srlRenderer = &lRenderer;
    }
    if (button == GLFW_KEY_3 && action == GLFW_PRESS){
        srlRenderer = &tRenderer;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}