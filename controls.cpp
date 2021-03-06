#include "controls.hpp"

#include <stdio.h>
// Include GLFW
// #include <GLFW/glfw3.h>
// #include "gfx.hpp"

extern GLFWwindow* window;


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
  return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
  return ProjectionMatrix;
}

bool doRun = false;
bool showOct = false;

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 0 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = -3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 10.0f; // units / second
float mouseSpeed = 0.005f;



void computeMatricesFromInputs(){
  static double xpos, ypos;
  static bool space_released = false;
  static bool t_released = false;
  static bool p_released = false;
  static bool doMouseHold = true;

  // glfwGetTime is called only once, the first time this function is called
  static double lastTime = glfwGetTime();
  if (lastTime < 1.0f)
    glfwSetCursorPos(window, 0, 0);

  // Compute time difference between current and last frame
  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  if (doMouseHold)
  {
    // Get mouse position
    glfwGetCursorPos(window, &xpos, &ypos);
    // printf("%g %g\n", xpos, ypos);
    if (xpos < -100.0f ) { xpos = -100.0f; }
    else if (xpos > 100.0f ) { xpos = 100.0f; }
    if (ypos < -100.0f ) { ypos = -100.0f; }
    else if (ypos > 100.0f ) { ypos = 100.0f; }

    // Reset mouse position for next frame
    glfwSetCursorPos(window, 0, 0);

    // Compute new orientation
    horizontalAngle += mouseSpeed * float(-xpos);
    verticalAngle   += mouseSpeed * float(-ypos);
    if (horizontalAngle > 2.0*pi<float>())
      horizontalAngle -= 2.0*pi<float>();
    else if (horizontalAngle < -2.0*pi<float>())
      horizontalAngle += 2.0*pi<float>();

    if (verticalAngle > half_pi<float>())
      verticalAngle = half_pi<float>();
    else if (verticalAngle < -half_pi<float>())
      verticalAngle = -half_pi<float>();
  }

  // printf("H:%g V:%g S:%g\n", horizontalAngle, verticalAngle);

  // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
    cos(verticalAngle) * sin(horizontalAngle), 
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
  );

  
  // Right vector
  glm::vec3 right = glm::vec3(
    sin(horizontalAngle - 3.14f/2.0f), 
    0,
    cos(horizontalAngle - 3.14f/2.0f)
  );
  
  // Up vector
  glm::vec3 up = glm::cross( right, direction );

  // Move forward
  if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
    position += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
    position -= direction * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
    position += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
    position -= right * deltaTime * speed;
  }

  // Pause/Run simulation, only one toggle per spacebar down
  if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
    if (space_released){
      doRun = !doRun;
    }
    space_released = false;
  }
  else if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_RELEASE){
    space_released = true;
  }

  // Show/Hide tree
  if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_PRESS){
    if (t_released){
      showOct = !showOct;
    }
    t_released = false;
  }
  else if (glfwGetKey( window, GLFW_KEY_T ) == GLFW_RELEASE){
    t_released = true;
  }

  // Pause key capture
  if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS){
    if (p_released){
      doMouseHold = !doMouseHold;
      if (doMouseHold)
      {
        printf("Mouse hidden\n");
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        // Reset mouse position for next frame
        glfwSetCursorPos(window, 0, 0);
      }
      else
      {
        printf("Mouse visible\n");
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
      }
      // glfwSetInputMode(window, GLFW_CURSOR, (doMouseHold ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED) );
    }
    p_released = false;
  }
  else if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_RELEASE){
    p_released = true;
  }

  


  float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  ViewMatrix       = glm::lookAt(
                position,           // Camera is here
                position+direction, // and looks here : at the same position, plus "direction"
                up                  // Head is up (set to 0,-1,0 to look upside-down)
               );

  // For the next frame, the "last time" will be "now"
  lastTime = currentTime;
}