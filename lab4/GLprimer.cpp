/*
 * A C++ framework for OpenGL programming in TNM046 for MT1 2015.
 *
 * This is a small, limited framework, designed to be easy to use
 * for students in an introductory computer graphics course in
 * the first year of a M Sc curriculum. It uses custom code
 * for some things that are better solved by external libraries
 * like GLEW and GLM, but the emphasis is on simplicity and
 * readability, not generality.
 * For the window management, GLFW 3.x is used for convenience.
 * The framework should work in Windows, MacOS X and Linux.
 * Some Windows-specific stuff for extension loading is still
 * here. GLEW could have been used instead, but for clarity
 * and minimal dependence on other code, we rolled our own extension
 * loading for the things we need. That code is short-circuited on
 * platforms other than Windows. This code is dependent only on
 * the GLFW and OpenGL libraries. OpenGL 3.3 or higher is required.
 *
 * Author: Stefan Gustavson (stegu@itn.liu.se) 2013-2015
 * This code is in the public domain.
 */
#if defined(WIN32) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
// File and console I/O for logging and error reporting
#include <iostream>
// Math header for trigonometric functions
#include <cmath>
#include <GL/glew.h>
// In MacOS X, tell GLFW to include the modern OpenGL headers.
// Windows does not want this, so we make this Mac-only.
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif

// In Linux, tell GLFW to include the modern OpenGL functions.
// Windows does not want this, so we make this Linux-only.
#ifdef __linux__
#define GL_GLEXT_PROTOTYPES
#endif

// GLFW 3.x, to handle the OpenGL window
#include <GLFW/glfw3.h>
#include "Utilities.hpp"
#include "Shader.hpp"
#include "TriangleSoup.hpp"

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
-M is the matrix we want to create(an output argument)
-vfov is the vertical field of view(in the y direction)
-aspect is the aspect ratio of the viewport(width/height)
-znear is the distance to the near clip plane(znear>0)
-zfar is the distance to the far clip plane(zfar>znear)
*/
void mat4perspective(float M[], float vfov, float aspect, float znear, float zfar) {
    float f = cos(vfov / 2) / sin(vfov / 2);
    M[0] = f / aspect;
    M[5] = f;
    M[10] = -((zfar + znear) / (zfar - znear));
    M[11] = -1;
    M[14] = -((2 * zfar * znear) / (zfar - znear));
    M[15] = 0;
}

/*
 * main(argc, argv) - the standard C++ entry point for the program
 */
int main(int argc, char* argv[]) {

    using namespace std;

    TriangleSoup myShapeS;
    // TriangleSoup myShapeK;
    // TriangleSoup myShapeT;

    GLfloat Spin[16], SpinX[16], SpinY[16], Orbit[16], Scale[16], Trans[16], View[16], MV[16],
        P[16];

    mat4scale(Scale, 0.5);
    mat4translate(Trans, 0.0, 0.0, -0.7);
    mat4rotX(View, -M_PI_4);

    int width, height;

    Shader myShader;

    // create the shaders
    myShader.createShader("vertex.glsl", "fragment.glsl");

    float time;

    GLint location_time, location_P, location_MV;

    const GLFWvidmode* vidmode;  // GLFW struct to hold information about the display
    GLFWwindow* window;          // GLFW struct to hold information about the window

    // Initialise GLFW
    glfwInit();

    // Determine the desktop size
    vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // Make sure we are getting a GL context of at least version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Exclude old legacy cruft from the context. We don't need it, and we don't want it.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Open a square window (aspect 1:1) to fill half the screen height
    window = glfwCreateWindow(vidmode->height / 2, vidmode->height / 2, "GLprimer", NULL, NULL);
    if (!window) {
        cout << "Unable to open window. Terminating." << endl;
        glfwTerminate();  // No window was opened, so we can't continue in any useful way
        return -1;
    }

    // Make the newly created window the "current context" for OpenGL
    // (This step is strictly required, or things will simply not work)
    glfwMakeContextCurrent(window);

    // Show some useful information on the GL context
    cout << "GL vendor:       " << glGetString(GL_VENDOR) << endl;
    cout << "GL renderer:     " << glGetString(GL_RENDERER) << endl;
    cout << "GL version:      " << glGetString(GL_VERSION) << endl;
    cout << "Desktop size:    " << vidmode->width << "x" << vidmode->height << " pixels" << endl;

    

    // Deactivate the vertex array object again to be nice
    glBindVertexArray(0);

    


    // Create a warning when the variable is either not used or not found.
    location_time = glGetUniformLocation(myShader.id(), "time");
    if (location_time == -1) {
        cout << "Unable to locate the variable 'time' in shader!" << endl;
    }
    location_P = glGetUniformLocation(myShader.id(), "P");
    if(location_P == -1){
        cout << "Unable to locate the variable 'P' in shader!" << endl;
    }
    
    location_MV = glGetUniformLocation(myShader.id(), "MV");
    if (location_MV == -1) {
        cout << "Unable to locate the variable 'MV' in shader!" << endl;
    }

    // myShapeS.createSphere(1, 10);
    myShapeS.createBox(0.2, 0.2, 1.0);
    // myShapeT.createTriangle();

    // glEnable(GL_CULL_FACE);

    glfwSwapInterval(0);  // Do not wait for screen refresh between frames

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Get window size. It may start out different from the requested
        // size, and will change if the user resizes the window.
        glfwGetWindowSize(window, &width, &height);
        // Set viewport. This is the pixel rectangle we want to draw into.
        glViewport(0, 0, width, height);  // The entire window

        // Set the clear color and depth, and clear the buffers for drawing
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* ---- Rendering code should go here ---- */
        util::displayFPS(window);
        time = (float)glfwGetTime();       // Number of seconds since the program was started
        glUseProgram(myShader.id());  // Activate the shader to set it's variables
        glUniform1f(location_time, time);  // copy the value to the shader program

        // Making orbit animation
        mat4identity(MV);
        mat4perspective(MV, 3 * M_PI / 2, 1, 0.1, 100);
        mat4rotY(Spin, time * 1.5);
        mat4rotX(SpinX, time);
        mat4rotY(SpinY, time * 0.5);
        // mat4rotY(Orbit,time);
        // mat4mult(MV, View, MV);
        // mat4mult(MV, Orbit, MV);
        mat4mult(MV, Trans, MV);
        mat4mult(MV, SpinY, MV);
        // mat4mult(MV, Scale, MV);

        glUniformMatrix4fv(location_MV, 1, GL_FALSE, MV);  // copy the value
        glUniformMatrix4fv(location_P, 1, GL_FALSE, P);  // copy the value

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        myShapeS.render();
        // myShapeK.render();
        // myShapeT.render();

        glfwSwapBuffers(window);

        // Poll events (read keyboard and mouse input)
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_SPACE)) {
            myShader.createShader("vertex.glsl", "fragment.glsl");
        }

        // Exit if the ESC key is pressed (and also if the window is closed).
        if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    // Close the OpenGL window and terminate GLFW.
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
