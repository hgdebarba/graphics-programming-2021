#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord; // here for completness, but we are not using it just yet

uniform mat4 model; // represents model coordinates in the world coord space
uniform mat4 view;  // represents the world coordinates in the camera coord space
uniform mat4 invTranspModel; // inverse of the transpose of model (used to multiply vectors while preserving angles)
uniform mat4 projection; // camera projection matrix

// TODO exercise 8.4 - make the 'out' variables that will be used in the fragment shader



void main() {
   // vertex in world space (for lighting computation)
   vec4 P = model * vec4(vertex, 1.0);
   // normal in world space (for lighting computation)
   vec3 N = normalize((invTranspModel * vec4(normal, 0.0)).xyz);

   // TODO 8.4 pass the positions in world space to the fragment shader



   // final vertex position (for opengl rendering, not for lighting)
   gl_Position = projection * view * P;
}