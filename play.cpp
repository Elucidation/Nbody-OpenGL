#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;
#include "loadShader.cpp"


void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLFWwindow* setupGL()
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

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

    // Default, handles basic shaders for us
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); 

    GLFWwindow* window;
    window = glfwCreateWindow(640, 480, "Play", NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    return window;
}

void initGlew()
{
    glewExperimental=GL_TRUE; // Needed in core profile 

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[])
{
    // Setup
    GLFWwindow* window = setupGL();


    // The meat
    glfwMakeContextCurrent(window);
    initGlew();
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    // Set up arrays
    // Do this once your window is created (= after the OpenGL Context creation)
    // and before any other OpenGL call.
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,
    };

    // This will identify our vertex buffer
    GLuint vertexbuffer;
     
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
     
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
     
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("BasicVertexShader.vert", 
                                   "BasicFragmentShader.frag");

    while (!glfwWindowShouldClose(window))
    {
        // Keep running
        // float ratio;
        // int width, height;
        // glfwGetFramebufferSize(window, &width, &height);
        // ratio = width / (float) height;

        // glViewport(0, 0, width, height);
        // glClear(GL_COLOR_BUFFER_BIT);

        // glMatrixMode(GL_PROJECTION);
        // glLoadIdentity();
        // glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        // glMatrixMode(GL_MODELVIEW);
        // glLoadIdentity();
        // glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        // glBegin(GL_TRIANGLES);
        // glColor3f(1.f, 0.f, 0.f);
        // glVertex3f(-0.6f, -0.4f, 0.f);
        // glColor3f(0.f, 1.f, 0.f);
        // glVertex3f(0.6f, -0.4f, 0.f);
        // glColor3f(0.f, 0.f, 1.f);
        // glVertex3f(0.f, 0.6f, 0.f);
        // glEnd();

        // double gTime = glfwGetTime();
        // printf("Hi there again at time %g\n", gTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
           0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
           3,                  // size
           GL_FLOAT,           // type
           GL_FALSE,           // normalized?
           0,                  // stride
           (void*)0            // array buffer offset
        );
         
        // Draw the triangle !
        glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
         
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Wrap up
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}   