//
// Created by captain on 3/24/21.
//
#include <iostream>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <window.h>
#include <matrices.h>
#include <timer.h>
#include <player.h>
#include <game.h>

#include <font.h>

GLMatrices Matrices;
GLuint programID;
GLuint fontProgam;
GLFWwindow *window;
game amongus{};
float z = 15.0f;
int frame = 0;
Timer t60(1.0 / 120);

float screen_zoom = 1, screen_center_x = 0, screen_center_y = 0;

void reset_screen() {
    float top = screen_center_y + 4 / screen_zoom;
    float bottom = screen_center_y - 4 / screen_zoom;
    float left = screen_center_x - 4 / screen_zoom;
    float right = screen_center_x + 4 / screen_zoom;
    Matrices.projection = glm::ortho(left, right, bottom, top, -10.0f, 10.0f);
}


/* Add all the models to be created here */
void initGL(GLFWwindow *glfwWindow, int width, int height) {
    /* Objects should be created before any other gl function and shaders */
    // Create the models
    amongus.init();

    initFreetype();

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("../src/shaders/shader3d.vert", "../src/shaders/shaderlight.frag");
    fontProgam = LoadShaders("../src/shaders/fontshader.vert", "../src/shaders/fontshader.frag");
    // bind our shader programs
    glUseProgram(programID);
    Matrices.mvpId = glGetUniformLocation(programID, "MVP");
    Matrices.modelId = glGetUniformLocation(programID, "model");

    reshapeWindow(glfwWindow, width, height);

    // Background color of the scene
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // R, G, B, A
    glClearDepth(1.0f);

    // Enable Depth Test
//    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
//    glDepthFunc(GL_LEQUAL);

    std::cout << "VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "RENDERER: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "VERSION: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

}

void draw() {
    // clear the color and depth in the frame buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Target - Where is the camera looking at.
    Matrices.view = glm::lookAt(glm::vec3{amongus.p.position, z},
                                glm::vec3{amongus.p.position, 0},
                                glm::vec3{0, 1, 0});

    // Compute Camera matrix (view)
    //Matrices.projection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 0.1f);
    Matrices.projection = glm::perspective((float) glm::radians(45.0), 1.0f, 1.0f, -1.0f);

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    // Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    // Don't change unless you are sure!!

    // Scene render

    // use the loaded shader program
    glUseProgram(programID);
    setProgramVec3(programID, "lightPos", glm::vec3{amongus.p.position, 1.0f});
    setProgramVec3(programID, "viewPos", glm::vec3{amongus.p.position, 20.0f});
    setProgramFloat(programID, "ambientStrength", amongus.is_dark ? 0.0f : 0.8f);
    setProgramFloat(programID, "diffuseStrength", amongus.is_dark ? 1.0f : 0.0f);
    setProgramVec3(programID, "lightColor", glm::vec3{1.0f, 1.0f, 1.0f});
    amongus.draw(VP);

    // use the loaded shader program
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(fontProgam);
    glm::mat4 projection = glm::ortho(0.0f, 900.0f, 0.0f, 900.0f, 10.0f, -10.0f);
    glUniformMatrix4fv(glGetUniformLocation(fontProgam, "projection"), 1, GL_FALSE, &projection[0][0]);
    RenderText(fontProgam, "light: " + std::to_string(amongus.is_dark), 12.0f, 12.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    RenderText(fontProgam, "score: " + std::to_string(std::round(amongus.score)), 12.0f, 38.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    RenderText(fontProgam, "time: " + std::to_string(std::round(frame * 0.02)), 12.0f, 64.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    glDisable(GL_BLEND);

}


void tick_elements() {

}

void tick_input(GLFWwindow *glfwWindow) {
    int keys[5];
    keys[0] = glfwGetKey(glfwWindow, GLFW_KEY_A);
    keys[1] = glfwGetKey(glfwWindow, GLFW_KEY_D);
    keys[2] = glfwGetKey(glfwWindow, GLFW_KEY_W);
    keys[3] = glfwGetKey(glfwWindow, GLFW_KEY_S);
    keys[4] = glfwGetKey(glfwWindow, GLFW_KEY_K);
    if (glfwGetKey(glfwWindow, GLFW_KEY_UP)) {
        z -= 0.1f;
    }
    if (glfwGetKey(glfwWindow, GLFW_KEY_DOWN)) {
        z += 0.1f;
    }
    amongus.get_input(keys);
}

int main(int argc, char **argv) {
    int width = 900;
    int height = 900;

    window = initGLFW(width, height);

    initGL(window, width, height);
    frame = 0;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
        // Process timers

        if (t60.process_tick()) {
            // 60 fps
            // OpenGL Draw commands
            draw();
            // Swap Frame Buffer in double buffering
            glfwSwapBuffers(window);

            tick_elements();
            tick_input(window);
            if (amongus.lost) {
                quit(window);
            }
        }
        frame++;
        if (frame > 100000) {
            quit(window);
        }
        // Poll for Keyboard and mouse events
        glfwPollEvents();
    }

    quit(window);
}