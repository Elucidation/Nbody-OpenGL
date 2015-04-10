#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>


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


int main(int argc, char const *argv[])
{
    // Setup
    GLFWwindow* window = setupGL();

    // The meat
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);
    
    glfwSetKeyCallback(window, key_callback);


    while (!glfwWindowShouldClose(window))
    {
        // Keep running
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();

        double gTime = glfwGetTime();
        printf("Hi there again at time %g\n", gTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Wrap up
    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}   