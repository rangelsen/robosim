#ifndef VIEWPOINT_HPP
#define VIEWPOINT_HPP

#include <glm/glm.hpp>
#include <GL/gl.h>

#include <Visualization/visualization.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/RenderBundle.h>
#include <Graphics/Cube.hpp>

// Frustum frame is defined as negative z pointing forwars and x right
typedef struct Frustum {

    float nearz, farz;
    float fovy;
    float aspect;

    // near: lower left, lower right, upper right, upper left
    // far: same as near
    glm::vec3 corners[8];

    // glm::vec3 corners[8];

    // near, right, far, left, bottom, top
    glm::vec3 normals[6];

    GLuint vao, vbo, ebo;
    glm::vec3 color;
    Shader* shader;
    glm::mat4 model_matrix;
    GLuint n_indices;
} Frustum;

void frustum_test();

////////////////////////////////////////////////////////////////////////////////
std::vector<Frustum> viewpoint_generate(Cube cube, uint16_t n_viewpoints,
                                        Frustum* sensor_frustum);

std::vector<glm::mat4> viewpoint_distribute_frustums(std::vector<float> z_dist,
                                                     glm::vec3 center);

////////////////////////////////////////////////////////////////////////////////
Frustum frustum_create(float nearz, float farz, float fovy, float aspect);

void frustum_destroy(void* ventity);

/**
 * @brief Checks if the point is contained within the given frustum.
 * @param point Point to check in the frustum local coordinate frame.
 */
bool frustum_is_inside(Frustum* fr, glm::vec3 point);

float frustum_get_z_containing_points(Frustum* fr, glm::vec3* points, uint32_t n_points);

float frustum_get_z_height(Frustum* fr, float height);

float frustum_get_z_width(Frustum* fr, float width);

void frustum_setup_vertex_buffers(Frustum* fr, std::vector<glm::vec3> vertices,
                                  std::vector<GLuint> indices);

std::vector<glm::vec3> frustum_generate_vertices(Frustum* fr);

std::vector<GLuint> frustum_generate_indices();

void frustum_render(void* ventity, RenderInfo* rinfo);

RenderBundle* frustum_create_renderbundle(Frustum* fr);

#endif // VIEWPOINT_HPP

/// @file

