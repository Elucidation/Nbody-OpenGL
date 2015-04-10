#version 330 core

// Notice that the "1" here equals the "1" in glVertexAttribPointer
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

uniform mat4 MVP;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
 
void main(){
 
    // Output position of the vertex, in clip space : MVP * position
    // Transform an homogeneous 4D vector, remember ?
    vec4 v = vec4(vertexPosition_modelspace,1); 
    gl_Position = MVP * v;

    // UV of the vertex, pass it on but inverted
    UV[0] = vertexUV[0]; // u
    UV[1] = 1.0 - vertexUV[1]; // v
}