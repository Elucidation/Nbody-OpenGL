// Hard-code screen size for now
#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1200

GLuint VertexArrayID;
GLuint programID;
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint ViewProjMatrixID;
GLuint TextureID;
GLuint Texture;
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;

static GLfloat* g_particle_position_size_data;
static GLubyte* g_particle_color_data;


void updateGfx(GLuint programID, GLfloat* g_particle_position_size_data, GLubyte* g_particle_color_data,
               glm::vec3* CameraPosition, unsigned long ParticlesCount)
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
  glUseProgram(programID);

  // Bind texture to Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, Texture);
  glUniform1i(TextureID, 0);

  glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
  glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
  
  glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

  

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

  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
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
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Play", NULL, NULL);

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
    programID = LoadShaders("BasicVertexShader.vert", 
                                   "BasicFragmentShader.frag");

    // Vertex shader
    CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
    CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
    ViewProjMatrixID = glGetUniformLocation(programID, "VP");

    // Fragment shader
    TextureID  = glGetUniformLocation(programID, "myTextureSampler");

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

    billboard_vertex_buffer;
    // Generate 1 buffer, put the resulting identifier in billboard_vertex_buffer
    glGenBuffers(1, &billboard_vertex_buffer);
    // The following commands will talk about our 'billboard_vertex_buffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), 
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    particles_position_buffer;
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    particles_color_buffer;
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}