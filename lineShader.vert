#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec4 xyzs; // Position 
layout(location = 1) in vec4 color; // Colors

// Output data ; will be interpolated for each fragment.
out vec4 fragmentColor;

// Values that stay constant for the whole mesh.
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

void main()
{
  // Output position of the vertex
  gl_Position = VP * vec4(xyzs.xyz, 1.0f);

  fragmentColor = color;
}
