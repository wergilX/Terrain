#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "src/core/Shader.hpp"
#include <vector>

class Mesh {
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    Mesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals,  std::vector<unsigned int> indices);
    void Draw(Shader &shader);
    void SetupMesh();

private:
    unsigned int VBO, NBO, EBO, VAO;
};

#endif //MESH_HPP
