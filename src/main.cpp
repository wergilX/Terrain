#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lib/imgui/imgui.h"
#include "lib/imgui/imgui_impl_sdl2.h"
#include "lib/imgui/imgui_impl_opengl3.h"
#include "lib/FastNoiseLite.h"

#include <iostream>
#include <stdio.h>

#include "Settings.hpp"
#include "src/core/Shader.hpp"
#include "src/core/Camera.hpp"
#include "src/entities/Mesh.hpp"

namespace {
    SDL_Window *window = nullptr;
    SDL_GLContext context;

    const char *glsl_version = "#version 130";
    bool mouse = true;

    void init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        window = SDL_CreateWindow(
            "OpenGl Terrain",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCR_WIDTH, SCR_HEIGHT,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

        if (window == nullptr) {
            fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }

        SDL_SetRelativeMouseMode(SDL_TRUE);

        // Setu GL context
        context = SDL_GL_CreateContext(window);
        if (context == nullptr) {
            fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
        SDL_GL_MakeCurrent(window, context);
        SDL_GL_SetSwapInterval(1); // Enable vsync
        // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

        GLenum glewError = glewInit();
        if (glewError != GLEW_OK) {
            fprintf(stderr, "Error initializing GLEW! %s\n", glewGetErrorString(glewError));
            exit(EXIT_FAILURE);
        }

        glEnable(GL_DEPTH_TEST);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Lines
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Setup ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(window, context);
        ImGui_ImplOpenGL3_Init(glsl_version);

        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        std::cout << "SDL and OpenGl inited" << std::endl;
    }
} // init

void processInput(float deltaTime) {
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keystate[SDL_SCANCODE_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keystate[SDL_SCANCODE_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keystate[SDL_SCANCODE_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (keystate[SDL_SCANCODE_LCTRL])
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (keystate[SDL_SCANCODE_SPACE])
        camera.ProcessKeyboard(UP, deltaTime);
}

void mouse_motion() {
    static int xoffset, yoffset;

    SDL_GetRelativeMouseState(&xoffset, &yoffset);
    if (mouse)
        camera.ProcessMouseMovement(xoffset, -yoffset);
}

void imgui_window() {
    ImGui::SetNextWindowSize(ImVec2(300, 450));
    ImGui::Begin("Settings");

    ImGui::Text("Camera");
    ImGui::Text("Position: X=%.2f Y=%.2f Z=%.2f", camera.GetPosition().x, camera.GetPosition().y,
                camera.GetPosition().z);
    ImGui::Text("Direction: Yaw=%.2f Pitch=%.2f", camera.Yaw, camera.Pitch);

    ImGui::Text("Background");
    ImGui::ColorEdit3("clear color", (float *) &clear_color);

    ImGui::Text("Terrain");
    ImGui::ColorEdit3("High color", (float *) &high_color);
    ImGui::ColorEdit3("Middle color", (float *) &middle_color);
    ImGui::ColorEdit3("Low Color", (float *) &low_color);

    ImGui::Text("Noise");
    ImGui::InputInt("seed", &seed, 1, 10);
    ImGui::InputFloat("frequency", &frequency, 0.01f, 1.0f, "%.3f");
    ImGui::InputInt("fractalOctaves", &fractalOctaves, 1, 10);
    ImGui::InputFloat("fractalLacunarity", &fractalLacunarity, 0.01f, 1.0f, "%.3f");
    ImGui::InputFloat("fractalGain", &fractalGain, 0.01f, 1.0f, "%.3f");

    ImGui::Text("Dir Light");
    ImGui::SliderFloat("x", &light_pos[0], -5, 5);
    ImGui::SliderFloat("y", &light_pos[1], -5, 5);
    ImGui::SliderFloat("z", &light_pos[2], -5, 5);
    ImGui::End();
}


namespace {
    std::vector<unsigned int> CreateIndexGrid(int sizeOfGrid) {
        std::vector<unsigned int> indices;

        for (int z = 0; z < sizeOfGrid; ++z) {
            for (int x = 0; x < sizeOfGrid; ++x) {
                int row1 = z * (sizeOfGrid + 1);
                int row2 = (z + 1) * (sizeOfGrid + 1);

                unsigned int topLeft = row1 + x;
                unsigned int topRight = row1 + x + 1;
                unsigned int bottomLeft = row2 + x;
                unsigned int bottomRight = row2 + x + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
        return std::move(indices);
    }
}


int main(int argc, char *args[]) {
    init();

    // Create and configure FastNoise object
    FastNoiseLite noise;
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    //===========================

    int gridSize = 50;
    auto indices = CreateIndexGrid(gridSize);
    float spacing = 1.0f;

    //===========================

    Shader triangleShader("../shaders/triangle.vert", "../shaders/triangle.frag");

    // Loop
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        float currentFrame = static_cast<float>(SDL_GetTicks());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update terrain

        noise.SetSeed(seed);
        noise.SetFrequency(frequency);
        noise.SetFractalOctaves(fractalOctaves);
        noise.SetFractalLacunarity(fractalLacunarity);
        noise.SetFractalGain(fractalGain);

        std::vector<glm::vec3> vertices;
        for (int z = 0; z <= gridSize; ++z) {
            for (int x = 0; x <= gridSize; ++x) {
                float xpos = x * spacing;
                float ypos = noise.GetNoise((float)x, (float)z) * 20;
                float zpos = z * spacing;

                vertices.push_back({xpos, ypos, zpos});
            }
        }

        std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));
        for (int i = 0; i < indices.size(); i += 3)
        {
            unsigned int i0 = indices[i];
            unsigned int i1 = indices[i+1];
            unsigned int i2 = indices[i+2];

            glm::vec3 v0 = vertices[i0];
            glm::vec3 v1 = vertices[i1];
            glm::vec3 v2 = vertices[i2];

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            normals[i0] += normal;
            normals[i1] += normal;
            normals[i2] += normal;
        }

        Mesh triangleMesh(vertices, normals, indices);

        while (SDL_PollEvent(&e) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_MOUSEMOTION) {
                mouse_motion();
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                if (e.key.keysym.sym == SDLK_LALT) {
                    mouse = !mouse;
                    if (mouse) {
                        SDL_SetRelativeMouseMode(SDL_TRUE);
                    } else {
                        SDL_WarpMouseInWindow(window, 1920 / 2, 1080 / 2);
                        SDL_SetRelativeMouseMode(SDL_FALSE);
                    }
                }
            }
        }
        processInput(deltaTime);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        imgui_window();
        ImGui::Render();

        // Clear screen
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Triangle
        glm::mat4 proectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                                      0.1f,
                                                      100.0f);

        triangleShader.setMat4("projection", proectionMatrix);
        triangleShader.setMat4("view", camera.GetViewMatrix());
        triangleShader.setMat4("model", glm::mat4(1.0f));
        triangleShader.setVec3("highColor", high_color);
        triangleShader.setVec3("middleColor", middle_color);
        triangleShader.setVec3("lowColor", low_color);

        // Material
        triangleShader.setVec3("material.ambient", {0.3f, 0.3f, 0.3f});
        triangleShader.setVec3( "material.specular", {0.1f, 0.1f, 0.1f});
        triangleShader.setFloat("material.shininess", 4.f);

        // Light
        triangleShader.setVec3("light.direction", light_pos);
        triangleShader.setVec3("light.ambient", {0.2f, 0.2f, 0.2f});
        triangleShader.setVec3( "light.diffuse", {0.5f, 0.5f, 0.5f});
        triangleShader.setVec3("light.specular", {1.0f, 1.0f, 1.0f});

        triangleShader.Activate();
        triangleMesh.Draw(triangleShader);

        // Render
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Destroy
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
