#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 Normal;
out vec3 Position;
out mat3 TBN;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
   // fragment position (world space)
   Position = (model * vec4(aPos, 1.0)).xyz;
   TexCoords = aTexCoords;

   // fragment normal (world space)
   mat3 modelInvTra = transpose(inverse(mat3(model)));
   Normal = modelInvTra * aNormal;

   // matrix to transform from tangent space to model space
   vec3 T = normalize(modelInvTra * aTangent);
   T = normalize(T - dot(T, Normal) * Normal);
   vec3 B = -cross(Normal, T);
   TBN = mat3(T, B, Normal);

   gl_Position = projection * view * model * vec4(aPos, 1.0);
}

