#version 330 core
in vec3 vertexPosition_modelspace;
uniform mat4 MVP;
 
void main(){
 
    // Output position of the vertex, in clip space : MVP * position
    // Transform an homogeneous 4D vector, remember ?
    vec4 v = vec4(vertexPosition_modelspace,1); 
    gl_Position = MVP * v;
}