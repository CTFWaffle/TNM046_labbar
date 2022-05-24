/*
 * A C++ framework for OpenGL programming in TNM046 for MT1
 *
 * This is a small, limited framework, designed to be easy to use for students in an introductory
 * computer graphics course in the first year of a M Sc curriculum. It uses custom code for some
 * things that are better solved by external libraries like GLM, but the emphasis is on simplicity
 * andreadability, not generality.
 *
 * For the window management, GLFW 3.x is used for convenience.
 * The framework should work in Windows, MacOS X and Linux.
 * GLEW is used for handling platform specific OpenGL extensions.
 * This code depends only on GLFW, GLEW, and OpenGL libraries.
 * OpenGL 3.3 or higher is required.
 *
 * Authors: Stefan Gustavson (stegu@itn.liu.se) 2013-2015
 *          Martin Falk (martin.falk@liu.se) 2021
 *
 * This code is in the public domain.
 */
#if defined(WIN32) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif

// File and console I/O for logging and error reporting
#include <iostream>
// Math header for trigonometric functions
#include <cmath>

// glew provides easy access to advanced OpenGL functions and extensions
#include <GL/glew.h>

// GLFW 3.x, to handle the OpenGL window
#include <GLFW/glfw3.h>
#include "Utilities.hpp"
#include "Shader.hpp"
#include "TriangleSoup.hpp"

//Extra functions
void mat4mult(float M1[], float M2[], float Mout[]) {

    float Mtemp[16];

    for (int i = 0; i < 16; ++i) {
        if (i < 4) {
            Mtemp[i] = M1[i] * M2[0] + M1[i + 4] * M2[1] + M1[i + 8] * M2[2] + M1[i + 12] * M2[3];
        } else if (i < 8) {
            Mtemp[i] = M1[i - 4] * M2[4] + M1[i] * M2[5] + M1[i + 4] * M2[6] + M1[i + 8] * M2[7];
        } else if (i < 12) {
            Mtemp[i] = M1[i - 8] * M2[8] + M1[i - 4] * M2[9] + M1[i] * M2[10] + M1[i + 4] * M2[11];
        } else {
            Mtemp[i] =
                M1[i - 12] * M2[12] + M1[i - 8] * M2[13] + M1[i - 4] * M2[14] + M1[i] * M2[15];
        }
    }

    for (int i = 0; i < 16; ++i) {
        Mout[i] = Mtemp[i];
    }
}

void mat4identity(float M[]) {
    for (int i = 0; i < 16; ++i) {
        if (i % 5 == 0 || i == 0) {
            M[i] = 1.0;
        } else {
            M[i] = 0.0;
        }
    }
    /*
    M[0] = 1.0;
    M[1] = 0.0;
    M[2] = 0.0;
    M[3] = 0.0;
    M[4] = 0.0;
    M[5] = 1.0;
    M[6] = 0.0;
    M[7] = 0.0;
    M[8] = 0.0;
    M[9] = 0.0;
    M[10] = 1.0;
    M[11] = 0.0;
    M[12] = 0.0;
    M[13] = 0.0;
    M[14] = 0.0;
    M[15] = 1.0;
    */
}

void mat4print(float M[]) {

    printf("Matrix :\n");
    printf("%6.2f %6.2f %6.2f %6.2f\n", M[0], M[4], M[8], M[12]);
    printf("%6.2f %6.2f %6.2f %6.2f\n", M[1], M[5], M[9], M[13]);
    printf("%6.2f %6.2f %6.2f %6.2f\n", M[2], M[6], M[10], M[14]);
    printf("%6.2f %6.2f %6.2f %6.2f\n", M[3], M[7], M[11], M[15]);
    printf("\n");
}

void mat4rotX(float M[], float angle) {
    mat4identity(M);
    M[5] = cos(angle);
    M[6] = sin(angle);
    M[9] = -sin(angle);
    M[10] = cos(angle);
}

void mat4rotY(float M[], float angle) {
    mat4identity(M);
    M[0] = cos(angle);
    M[2] = -sin(angle);
    M[8] = sin(angle);
    M[10] = cos(angle);
}

void mat4rotZ(float M[], float angle) {
    mat4identity(M);
    M[0] = cos(angle);
    M[1] = sin(angle);
    M[4] = -sin(angle);
    M[5] = cos(angle);
}

void mat4scale(float M[], float scale) {
    mat4identity(M);
    M[0] = scale;
    M[5] = scale;
    M[10] = scale;
}

void mat4translate(float M[], float x, float y, float z) {
    mat4identity(M);
    M[12] = x;
    M[13] = y;
    M[14] = z;
}

/*
 * main(int argc, char* argv[]) - the standard C++ entry point for the program
 */
int main(int, char*[]) {
    // Initialise GLFW
    glfwInit();

     TriangleSoup myShapeS;
    // TriangleSoup myShapeK;
    // TriangleSoup myShapeT;

    GLfloat B[16], Spin[16], Spin2[16], Spin3[16], Orbit[16], Scale[16], Trans[16], View[16];

    mat4scale(Scale, 0.2);
    mat4translate(Trans, 0.5, 0.0, 0.0);
    mat4rotX(View, -M_PI_4);

    Shader myShader;

    float time;

    GLint location_time, location_B;

    const GLFWvidmode* vidmode;  // GLFW struct to hold information about the display
    // Determine the desktop size
    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Make sure we are getting a GL context of at least version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Enable the OpenGL core profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Open a square window (aspect 1:1) to fill half the screen height
    GLFWwindow* window =
        glfwCreateWindow(vidmode->height / 2, vidmode->height / 2, "GLprimer", nullptr, nullptr);
    if (!window) {
        std::cout << "Unable to open window. Terminating.\n";
        glfwTerminate();  // No window was opened, so we can't continue in any useful way
        return -1;
    }

    // Make the newly created window the "current context" for OpenGL
    // (This step is strictly required or things will simply not work)
    glfwMakeContextCurrent(window);

    // Initialize glew
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Error: " << glewGetErrorString(err) << "\n";
        glfwTerminate();
        return -1;
    }

    // Show some useful information on the GL context
    std::cout << "GL vendor:       " << glGetString(GL_VENDOR)
              << "\nGL renderer:     " << glGetString(GL_RENDERER)
              << "\nGL version:      " << glGetString(GL_VERSION)
              << "\nDesktop size:    " << vidmode->width << " x " << vidmode->height << "\n";

    // Get window size. It may start out different from the requested size and
    // will change if the user resizes the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    // Set viewport. This is the pixel rectangle we want to draw into
    glViewport(0, 0, width, height);  // The entire window

    // Deactivate the vertex array object again to be nice
    glBindVertexArray(0);

    myShader.createShader("vertex.glsl", "fragment.glsl");

    location_time = glGetUniformLocation(myShader.id(), "time");
    if (location_time == -1) {
        std::cout << "Unable to locate the variable 'time' in shader!" << std::endl;
    }

    location_B = glGetUniformLocation(myShader.id(), "B");
    if (location_B == -1) {
        std::cout << "Unable to locate the variable 'B' in shader!" << std::endl;
    }

    myShapeS.createSphere(1, 7);
    // myShapeK.createBox(1, 1, 1);
    // myShapeT.createTriangle();

    glEnable(GL_CULL_FACE);

    glfwSwapInterval(0);  // Do not wait for screen refresh between frames

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Set the clear color to a dark gray (RGBA)
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
        // Clear the color and depth buffers for drawing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* ---- Rendering code should go here ---- */
        util::displayFPS(window);
        time = (float)glfwGetTime();       // Number of seconds since the program was started
        glUseProgram(myShader.id());  // Activate the shader to set it's variables
        glUniform1f(location_time, time);  // copy the value to the shader program

        // Making orbit animation
        mat4identity(B);
        //mat4rotZ(Spin3,time*1.5);
        //mat4rotY(Spin2,-time*1.5);
        mat4rotY(Spin, time * 1.5);
        mat4rotY(Orbit, time);
        mat4mult(B, View, B);
        mat4mult(B, Orbit, B);
        mat4mult(B, Trans, B);

        //mat4mult(B, Spin3, B);
        // mat4mult(B, Spin2, B);
        mat4mult(B, Spin, B);
        mat4mult(B, Scale, B);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        myShapeS.render();
        // myShapeK.render();
        // myShapeT.render();

        glUniformMatrix4fv(location_B, 1, GL_FALSE,
                           B);  // copy the value
                                // Swap buffers, i.e. display the image and prepare for next frame.
        // Swap buffers, display the image and prepare for next frame
        glfwSwapBuffers(window);

        // Poll events (read keyboard and mouse input)
        glfwPollEvents();

        // Exit if the ESC key is pressed (and also if the window is closed)
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    // Close the OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
}
