#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "src/core/Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Window
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Camera
Camera camera(glm::vec3(0.0f, 30.0f, 5.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Noise
int seed = 1337;
float frequency = 0.010;
int fractalOctaves = 3;
float fractalLacunarity = 2.000 ;
float fractalGain = 0.5;

// Colors
glm::vec3 high_color = glm::vec3(0.9f, 0.9f, 0.9f);
glm::vec3 middle_color = glm::vec3(0.1f, 0.6f, 0.2f);
glm::vec3 low_color = glm::vec3(0.2f, 0.3f, 0.8f);

glm::vec4 clear_color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
glm::vec3 light_pos{-0.2f, -1.0f, -0.3f};

#endif //SETTINGS_HPP
