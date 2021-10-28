#version 330 core

uniform vec3 camPosition; // so we can compute the view vector
out vec4 FragColor; // the output color of this fragment

// TODO exercise 8.4 setup the 'uniform' variables needed for lighting
// light uniforms

// material uniforms

// attenuation uniforms

// TODO exercise 8.4 add the 'in' variables to receive the interpolated Position and Normal from the vertex shader


void main()
{

   // TODO exercise 8.4 - phong shading (i.e. Phong reflection model computed in the fragment shader)
   // ambient component

   // diffuse component for light 1

   // specular component for light 1


   // TODO exercise 8.5 - multiple lights, compute diffuse and specular of light 2


   // TODO exercuse 8.6 - attenuation - light 1


   // TODO exercuse 8.6 - attenuation - light 2


   // TODO compute the final shaded color (e.g. add contribution of the (attenuated) lights 1 and 2)


   // TODO set the output color to the shaded color that you have computed
   FragColor = vec4(.8, .8, .8, 1.0);
}
// you might have noticed that the shading contribution of multiple lights can fit a for loop nicely
// we will be doing that later on