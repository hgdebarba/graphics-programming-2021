#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

out VS_OUT {
   vec3 CamPos_tangent;
   vec3 Pos_tangent;
   vec3 LightDir_tangent;
   vec3 Norm_tangent;
   vec2 textCoord;
   mat3 invTBN;
} vs_out;

// transformations
uniform mat4 projection;   // camera projection matrix
uniform mat4 view;         // represents the world in the eye coord space
uniform mat4 model;        // represents model in the world coord space
uniform mat3 modelInvTra;  // inverse of the transpose of the model matrix, used to rotate vectors while preserving angles

// light uniform variables
uniform vec3 lightDirection;
uniform vec3 viewPosition;


void main() {
   // send text coord to fragment shader
   vs_out.textCoord = textCoord;

   // vertex normal in world space
   vec3 N = normalize(modelInvTra * normal);

   // TODO exercise 10.4 compute the TBN matrix, which maps from world space to Tangent space
   //  notice that tangent and bitangent are given as vertex properties
   //  try to ensure that the 3 vectors you use to define TBN are perpecndicular
   mat3 TBN =  mat3(1);


   // variables we wanna send to the fragment shader
   // inverse of TBN, to map from tangent space to world space (needed for reflections)
   vs_out.invTBN = inverse(TBN);
   // light direction, view position, vertex position, and normal in tangent space
   vs_out.LightDir_tangent = TBN * lightDirection;
   vs_out.CamPos_tangent = TBN * viewPosition;
   vs_out.Pos_tangent  = vec3(0,0,0); // the position of the vertex in the tangent space is the origin of the space (we could ommit that)
   vs_out.Norm_tangent = TBN * N;

   // final vertex transform (for opengl rendering)
   gl_Position = projection * view * model * vec4(vertex, 1.0);
}