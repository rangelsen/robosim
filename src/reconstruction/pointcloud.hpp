#include <vector>
#include <Eigen/Eigen>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <Graphics/Vertex.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/RenderBundle.h>

////////////////////////////////////////////////////////////////////////////////
typedef struct Pointcloud {

    std::vector<Eigen::Vector3f> points;
    glm::vec3 color;
    GLuint vbo, vao;
    Shader* shader;
    glm::vec3 min;
    glm::vec3 max;
    uint32_t n_points;
} Pointcloud;

////////////////////////////////////////////////////////////////////////////////
Pointcloud* pcloud_create();

void pcloud_setup_for_rendering(Pointcloud* pcloud);

void pcloud_render(void* ventity, RenderInfo* rinfo);

RenderBundle* pcloud_create_renderbundle(Pointcloud* pcloud);

void pcloud_add_points(Pointcloud* pcloud, std::vector<Eigen::Vector3f>* points);

/// @file
