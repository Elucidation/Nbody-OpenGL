// Hard-code screen size for now
#include "gfx.hpp"

#include "loadShader.hpp" // Shader
#include "texture.hpp" // TextureIDs

#include "octree.hpp"

#include <stdio.h>
#include <stdlib.h>


#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

GLuint VertexArrayID;
GLuint billboardProgramID;
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint BillboardViewProjMatrixID;
GLuint TextureID;
GLuint Texture;
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;

GLfloat* g_particle_position_size_data;
GLubyte* g_particle_color_data;

///
GLuint quadProgramID;
GLuint QuadViewProjMatrixID;
GLuint line_position_buffer;
GLuint line_color_buffer;
static const GLfloat originPoints[] = {
  1, 0, -20, 0.6, // x
  0, 0, -20, 0.6,
  0, 1, -20, 0.6, // y
  0, 0, -20, 0.6,
  0, 0, -19, 0.6 // z
};
static const GLubyte originColors[] = {
  255, 0, 0, 255, // R for x
  255, 255, 255, 255,
  0, 0, 255, 255, // G for y
  255, 255, 255, 255,
  0, 255, 0, 255 // B for z
};

// Colors for 8 octants
static const GLubyte octantColors[] = {
  0, 0, 0, 150,
  255, 0, 0, 150,
  0, 255, 0, 150,
  0, 0, 255, 150,

  255, 255, 0, 150,
  0, 255, 255, 150,
  255, 0, 255, 150,
  255, 255, 255, 150,
};

static const GLubyte markerColors[] = {
  255, 0, 0, 255,
  255, 0, 0, 255,
  0, 255, 0, 255,
};


const unsigned int maxLinePoints = 16; // 12 lines to a box, 4 overlapping for strip

GLFWwindow* window; // Needed by controls.cpp

///


void updateGfx(GLfloat* g_particle_position_size_data, 
    GLubyte* g_particle_color_data, unsigned long ParticlesCount, Octree* oct)
{
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw Origin
  drawOrigin();

  // Draw Oct-tree
  if (showOct)
  {
    
    drawTree(oct, 0, 20); // maxdepth of 20
    // drawOctCOM(oct); // For simple visualization of node center of masses
  }  
  // Color Particles based on Oct-tree
  // colorParticles(oct); // Color particles based on oct-tree quadrant

  // Draw Particles
  drawParticles(g_particle_color_data, ParticlesCount);

  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
}

void colorParticles(Octree* oct)
{
  for(int i=0; i<MaxParticles; i++)
    {
        Particle& p = ParticlesContainer[i]; // shortcut        
        if(p.life > 0.0f)
        {
          int idx = oct->getChildIndex(p.pos);
          // printf("%d %d %d %d\n", octantColors[ 4*idx ], octantColors[ 4*idx +1], octantColors[ 4*idx +2], octantColors[ 4*idx +3]);
          p.r = octantColors[ 4*idx ];
          p.g = octantColors[ 4*idx + 1 ];
          p.b = octantColors[ 4*idx + 2 ];
          p.a = 255; //octantColors[ 4*idx + 3 ];

        }
    }
}

// Draws a small marker (triangle) at each CoM of each node, decreasing in size
// for simple visualization of node center of masses
void drawOctCOM(Octree* root, const float size, const int depth, const int max_depth)
{
  if (depth > max_depth) return;

  if (!root->bods.empty())
    drawMarker(root->com.xyz(),size);

  if (root->interior)
  {
    for (int i = 0; i < 8; ++i)
    {
      drawOctCOM(root->children[i], size/1.5f, depth+1, max_depth);
    }
  }
}

void drawParticles(GLubyte* g_particle_color_data, unsigned long ParticlesCount)
{
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;    

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU, 
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particle_position_size_data);

  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particle_color_data);


  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  // Use our shader
  glUseProgram(billboardProgramID);

  // Bind texture to Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glUniform1i(TextureID, 0);

  glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
  glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
  
  glUniformMatrix4fv(BillboardViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  

  // 1st attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glVertexAttribPointer(
     0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
     3,                  // size
     GL_FLOAT,           // type
     GL_FALSE,           // normalized?
     0,                  // stride
     (void*)0            // array buffer offset
  );

  // 2nd attribute buffer : positions of particles' centers
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glVertexAttribPointer(
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : x + y + z + size => 4
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // 3rd attribute buffer : particles' colors
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glVertexAttribPointer(
      2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : r + g + b + a => 4
      GL_UNSIGNED_BYTE,                 // type
      GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
  glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
  glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
  glVertexAttribDivisor(2, 1); // color : one per quad       
   
  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
  // but faster.
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

void drawOrigin()
{
  const unsigned int numPoints = 5; // for origin
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;    

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU, 
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, numPoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, numPoints * sizeof(GLfloat) * 4, originPoints);

  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, numPoints * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, numPoints * sizeof(GLubyte) * 4, originColors);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  // Use our shader
  glUseProgram(quadProgramID);

  glUniformMatrix4fv(QuadViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  // 1st attribute buffer : positions
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glVertexAttribPointer(
      0,                                // attribute. No particular reason for 0, but must match the layout in the shader.
      4,                                // size : x + y + z + size => 4
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // 2nd attribute buffer : colors
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glVertexAttribPointer(
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : r + g + b + a => 4
      GL_UNSIGNED_BYTE,                 // type
      GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // no divisors
  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);

  glDrawArrays(GL_LINE_STRIP, 0, numPoints);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

void drawTree(Octree* root, int depth /*=0*/, int maxDepth /*=5*/)
{
  if (depth > maxDepth)
    return;
  int color_idx = depth % 8;
  const GLubyte* color = octantColors+color_idx*4;
  // static const GLubyte color[] = {0, 255, 0, 255};

  if (root->interior)
  {
    for (int i = 0; i < 8; ++i)
    {
      drawTree(root->children[i], depth+1);
    }
  }
  else if (!root->bods.empty())
  {
    // drawBounds(*(root->getBounds()), color);
    drawBounds(*(root->bbox), color);
  }
}

void drawBounds(const Bounds& bbox, const GLubyte* color)
{
  glm::vec3 bmin = bbox.center - bbox.half_width*0.99f;
  glm::vec3 bmax = bbox.center + bbox.half_width*0.99f;
  drawBox(bmin, bmax, color);
}

void drawMarker(const glm::vec3 point, const float half_width)
{
  static const int numMarkerPoints = 3;
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

  // Generate vertices from Bounds
  const GLfloat markerPoints[] = {
    // start front
    (GLfloat)(point.x-half_width*.5f), (GLfloat)(point.y-half_width), (GLfloat)(point.z), 1.0,
    (GLfloat)(point.x+half_width*.5f), (GLfloat)(point.y-half_width), (GLfloat)(point.z), 1.0,
    (GLfloat)(point.x), (GLfloat)(point.y), (GLfloat)(point.z), 1.0,
  };

  // Update the buffers that OpenGL uses for rendering.
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, numMarkerPoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, numMarkerPoints * sizeof(GLfloat) * 4, markerPoints);

  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, numMarkerPoints * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, numMarkerPoints * sizeof(GLubyte) * 4, markerColors);


  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  // Use our shader
  glUseProgram(quadProgramID);

  glUniformMatrix4fv(QuadViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  // 1st attribute buffer : positions
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glVertexAttribPointer(
      0,                                // attribute. No particular reason for 0, but must match the layout in the shader.
      4,                                // size : x + y + z + size => 4
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // 2nd attribute buffer : colors
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glVertexAttribPointer(
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : r + g + b + a => 4
      GL_UNSIGNED_BYTE,                 // type
      GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // no divisors
  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);

  glDrawArrays(GL_TRIANGLES, 0, numMarkerPoints);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

// Box min, box max points
void drawBox(const glm::vec3 bmin, const glm::vec3 bmax, const GLubyte* color)
{
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

  // Generate vertices from Bounds
  const GLfloat boxPoints[] = {
    // start front
    (GLfloat)(bmin.x), (GLfloat)(bmin.y), (GLfloat)(bmin.z), 0.6,
    
    // front 4
    (GLfloat)(bmax.x), (GLfloat)(bmin.y), (GLfloat)(bmin.z), 0.6,
    (GLfloat)(bmax.x), (GLfloat)(bmax.y), (GLfloat)(bmin.z), 0.6,
    (GLfloat)(bmin.x), (GLfloat)(bmax.y), (GLfloat)(bmin.z), 0.6,
    (GLfloat)(bmin.x), (GLfloat)(bmin.y), (GLfloat)(bmin.z), 0.6,

    // start back
    (GLfloat)(bmin.x), (GLfloat)(bmin.y), (GLfloat)(bmax.z), 0.6,
    
    // back 4
    (GLfloat)(bmax.x), (GLfloat)(bmin.y), (GLfloat)(bmax.z), 0.6,
    (GLfloat)(bmax.x), (GLfloat)(bmax.y), (GLfloat)(bmax.z), 0.6,
    (GLfloat)(bmin.x), (GLfloat)(bmax.y), (GLfloat)(bmax.z), 0.6,
    (GLfloat)(bmin.x), (GLfloat)(bmin.y), (GLfloat)(bmax.z), 0.6,

    // sides
    (GLfloat)(bmax.x), (GLfloat)(bmin.y), (GLfloat)(bmax.z), 0.6,
    (GLfloat)(bmax.x), (GLfloat)(bmin.y), (GLfloat)(bmin.z), 0.6,
    (GLfloat)(bmax.x), (GLfloat)(bmax.y), (GLfloat)(bmin.z), 0.6,
    (GLfloat)(bmax.x), (GLfloat)(bmax.y), (GLfloat)(bmax.z), 0.6,

    (GLfloat)(bmin.x), (GLfloat)(bmax.y), (GLfloat)(bmax.z), 0.6,
    (GLfloat)(bmin.x), (GLfloat)(bmax.y), (GLfloat)(bmin.z), 0.6,
  };

  GLubyte* boxColors = new GLubyte[maxLinePoints * 4];
  for (int i = 0; i < maxLinePoints; ++i)
  {
    boxColors[i*4 + 0] = color[0];
    boxColors[i*4 + 1] = color[1];
    boxColors[i*4 + 2] = color[2];
    boxColors[i*4 + 3] = color[3];
  }

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU, 
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, maxLinePoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, maxLinePoints * sizeof(GLfloat) * 4, boxPoints);

  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, maxLinePoints * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, maxLinePoints * sizeof(GLubyte) * 4, boxColors);


  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  // Use our shader
  glUseProgram(quadProgramID);

  glUniformMatrix4fv(QuadViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  // 1st attribute buffer : positions
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
  glVertexAttribPointer(
      0,                                // attribute. No particular reason for 0, but must match the layout in the shader.
      4,                                // size : x + y + z + size => 4
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // 2nd attribute buffer : colors
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
  glVertexAttribPointer(
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      4,                                // size : r + g + b + a => 4
      GL_UNSIGNED_BYTE,                 // type
      GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
      0,                                // stride
      (void*)0                          // array buffer offset
  );

  // no divisors
  glVertexAttribDivisor(0, 0);
  glVertexAttribDivisor(1, 0);

  glDrawArrays(GL_LINE_STRIP, 0, maxLinePoints);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
}

void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static bool window_size = false;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void setupWindowHints()
{
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing

    // If you are crashing here it probably means your machine doesn't
    // support OpenGL 3.3, so comment out these lines
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 

    // Use Core profile
    // Using the following line requires full vertex and fragment shaders
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
}

void initGlew()
{
    glewExperimental=GL_TRUE; // Needed in core profile 

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(EXIT_FAILURE);
    }
}

GLFWwindow* setupGL()
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    setupWindowHints();

    // Default, handles basic shaders for us
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); 

    GLFWwindow* window;
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Barnes-Hut N-Body Simulation", NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    return window;
}

void initGfx()
{
  // Setup
    window = setupGL();

    glfwMakeContextCurrent(window);
    // Ensure we can capture the escape key being pressed below
    // glfwSwapInterval(1); // breaks getCursor/setCursor stuff

    // Initialize GLEW
    initGlew();

    // Set update rate & keyboard callbacks
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Set up vertex array
    VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);   

    // Create and compile our GLSL program from the shaders
    billboardProgramID = LoadShaders("BasicVertexShader.vert", 
                                   "BasicFragmentShader.frag");

    // Vertex shader
    CameraRight_worldspace_ID  = glGetUniformLocation(billboardProgramID, "CameraRight_worldspace");
    CameraUp_worldspace_ID  = glGetUniformLocation(billboardProgramID, "CameraUp_worldspace");
    BillboardViewProjMatrixID = glGetUniformLocation(billboardProgramID, "VP");

    // Fragment shader
    TextureID  = glGetUniformLocation(billboardProgramID, "myTextureSampler");

    g_particle_position_size_data = new GLfloat[MaxParticles * 4];
    g_particle_color_data         = new GLubyte[MaxParticles * 4];

    for(int i=0; i<MaxParticles; i++){
        ParticlesContainer[i].life = -1.0f;
        ParticlesContainer[i].cameradistance = -1.0f;
    }

    Texture = loadDDS("point2.DDS"); // ( coord.u, 1.0-coord.v)

     
    // The VBO containing the 4 vertices of the particles.
    // Thanks to instancing, they will be shared by all particles.
    static const GLfloat g_vertex_buffer_data[] = { 
         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };

    // Generate 1 buffer, put the resulting identifier in billboard_vertex_buffer
    glGenBuffers(1, &billboard_vertex_buffer);
    // The following commands will talk about our 'billboard_vertex_buffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), 
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// Shader #2
    quadProgramID = LoadShaders("lineShader.vert", 
                                "lineShader.frag");

    QuadViewProjMatrixID = glGetUniformLocation(quadProgramID, "VP");

    // Set up origin

    // The VBO containing the positions and sizes of the particles
    glGenBuffers(1, &line_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, maxLinePoints * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    // Give buffer
    // glBufferData(GL_ARRAY_BUFFER, sizeof(originPoints), originPoints, GL_STATIC_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &line_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, line_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, maxLinePoints * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
    // Give buffer
    // glBufferData(GL_ARRAY_BUFFER, sizeof(originColors), originColors, GL_STATIC_DRAW);

}

void gfxCleanup()
{
  glfwDestroyWindow(window);

  delete[] g_particle_position_size_data;
  delete[] g_particle_color_data;

  // Cleanup VBO and shader
  glDeleteBuffers(1, &particles_color_buffer);
  glDeleteBuffers(1, &particles_position_buffer);
  glDeleteBuffers(1, &line_position_buffer);
  glDeleteBuffers(1, &line_color_buffer);
  glDeleteBuffers(1, &billboard_vertex_buffer);
  glDeleteProgram(billboardProgramID);
  glDeleteProgram(quadProgramID);
  glDeleteTextures(1, &Texture);
  glDeleteVertexArrays(1, &VertexArrayID);

  glfwTerminate();
}