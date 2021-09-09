#version 330 core
layout (location = 0) in vec2 pos;   // the position variable has attribute position 0
// TODO 2.2 add velocity and timeOfBirth as vertex attributes

// TODO 2.3 create and use a float uniform for currentTime

// TODO 2.6 create out variable to send the age of the particle to the fragment shader

void main()
{
    // TODO 2.3 use the currentTime to control the particle in different stages of its lifetime

    // TODO 2.6 send the age of the particle to the fragment shader using the out variable you have created

    // this is the output position and and point size (this time we are rendering points, instad of triangles!)
    gl_Position = vec4(pos, 0.0, 1.0);
    gl_PointSize = 10.0;
}