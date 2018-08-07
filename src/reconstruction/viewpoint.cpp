#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <GL/glew.h>
#include <GL/gl.h>

#include <viewpoint.hpp>
#include <Graphics/ShaderAttributes.hpp>

////////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265f
#define VIEWPOINT_HEIGHT 15.0f
#define VIEWPOINT_ANGLE 0.79f // 45 degrees downwards from horizontal
#define VIEWPOINT_RADIUS 20.0f

enum {

    NEAR_LL,
    NEAR_LR,
    NEAR_UR,
    NEAR_UL,
    FAR_LL,
    FAR_LR,
    FAR_UR,
    FAR_UL
};

////////////////////////////////////////////////////////////////////////////////
std::vector<Frustum> viewpoint_generate(Cube cube, uint16_t n_viewpoints,
                                        Frustum* sensor_frustum) {

    std::vector<Frustum> out;
    glm::vec3 cube_center = cube.pos + cube.length / 2.0f;

    std::vector<glm::mat4> dummy_poses = viewpoint_distribute_frustums(
        std::vector<float>(n_viewpoints, 0.0f), cube_center);

    std::vector<float> adjusted_frustum_dist;

    for (uint32_t i = 0; i < dummy_poses.size(); i++) {
        
        glm::vec3 cube_corners[8];

        glm::mat4 inv_model = glm::inverse(dummy_poses[i]);

        for (uint8_t x = 0; x < 2; x++) {
            for (uint8_t y = 0; y < 2; y++) {
                for (uint8_t z = 0; z < 2; z++) {
                    
                    uint8_t index = x + 2 * (y + 2 * z);
                    cube_corners[index] = glm::vec3(inv_model * glm::vec4(cube.pos +
                        glm::vec3(x, y, z) * cube.length, 1.0f));
                }
            }
        }

        float z_adj = frustum_get_z_containing_points(sensor_frustum,
                                                      cube_corners, 8);

        if (z_adj < sensor_frustum->farz) {

            z_adj = fmaxf(z_adj, sensor_frustum->nearz);
            adjusted_frustum_dist.push_back(z_adj);
        }
    }

    std::vector<glm::mat4> frustum_poses = viewpoint_distribute_frustums(
        adjusted_frustum_dist, cube_center);

    for (uint32_t i = 0; i < frustum_poses.size(); i++) {

        Frustum frustum = *sensor_frustum;
        frustum.model_matrix = frustum_poses[i];

        out.push_back(frustum);
    }

    return out;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<glm::mat4> viewpoint_distribute_frustums(std::vector<float> z_dist,
                                                     glm::vec3 center) {

    float dt = 2.0f * PI / z_dist.size();
    float angle = 0.0f;

    std::vector<glm::mat4> out;
    for (uint16_t i = 0; i < z_dist.size(); i++) {

        glm::mat4 pose(1.0f);

        glm::vec3 pos = center + glm::vec3(z_dist[i] * cosf(angle), z_dist[i],
                                           -z_dist[i] * sinf(angle));

        pose = glm::translate(pose, pos);    

        // Yaw then tilt
        pose = glm::rotate(pose, angle + PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        pose = glm::rotate(pose, -VIEWPOINT_ANGLE, glm::vec3(1.0f, 0.0f, 0.0f));

        out.push_back(pose);
        angle += dt;
    }

    return out;
}

////////////////////////////////////////////////////////////////////////////////
Frustum frustum_create(float nearz, float farz, float fovy, float aspect) {
    
    Frustum frustum;
    frustum.nearz = nearz;
    frustum.farz = farz;
    frustum.fovy = fovy;
    frustum.aspect = aspect;

    float half_near_height = nearz * tanf(fovy/ 2.0f);
    float half_near_width  = aspect * half_near_height;
    float half_far_height  = farz * tanf(fovy / 2.0f);
    float half_far_width   = aspect * half_far_height;

    frustum.corners[NEAR_LL] = glm::vec3(-half_near_width, -half_near_height, -nearz);
    frustum.corners[NEAR_LR] = glm::vec3( half_near_width, -half_near_height, -nearz);
    frustum.corners[NEAR_UR] = glm::vec3( half_near_width,  half_near_height, -nearz);
    frustum.corners[NEAR_UL] = glm::vec3(-half_near_width,  half_near_height, -nearz);
    frustum.corners[FAR_LL]  = glm::vec3(-half_far_width,  -half_far_height,  -farz);
    frustum.corners[FAR_LR]  = glm::vec3( half_far_width,  -half_far_height,  -farz);
    frustum.corners[FAR_UR]  = glm::vec3( half_far_width,   half_far_height,  -farz);
    frustum.corners[FAR_UL]  = glm::vec3(-half_far_width,   half_far_height,  -farz);

    // Placeholders
    glm::vec3 v1, v2;

    // Near
    frustum.normals[0] = glm::vec3(0.0f, 0.0f, 1.0f);

    // Right
    v1 = frustum.corners[FAR_LR] - frustum.corners[NEAR_LR];
    v2 = frustum.corners[NEAR_UR] - frustum.corners[NEAR_LR];
    frustum.normals[1] = glm::normalize(glm::cross(v1, v2));

    // Far
    frustum.normals[2] = -frustum.normals[0];

    // Left
    v1 = frustum.corners[FAR_UL] - frustum.corners[NEAR_UL];
    v2 = frustum.corners[NEAR_LL] - frustum.corners[NEAR_UL];
    frustum.normals[3] = glm::normalize(glm::cross(v1, v2));

    // Bottom
    v1 = frustum.corners[FAR_LL] - frustum.corners[NEAR_LL];
    v2 = frustum.corners[NEAR_LR] - frustum.corners[NEAR_LL];
    frustum.normals[4] = glm::normalize(glm::cross(v1, v2));

    // Top
    v1 = frustum.corners[FAR_UR] - frustum.corners[NEAR_UR];
    v2 = frustum.corners[NEAR_UL] - frustum.corners[NEAR_UR];
    frustum.normals[5] = glm::normalize(glm::cross(v1, v2));

    frustum.model_matrix = glm::mat4(1.0f);
    frustum.color = glm::vec3(1.0f) * 0.2f;

    std::vector<glm::vec3> vertices = frustum_generate_vertices(&frustum);
    std::vector<GLuint> indices = frustum_generate_indices();
    frustum.n_indices = indices.size();

    frustum_setup_vertex_buffers(&frustum, vertices, indices);

    return frustum;
}

////////////////////////////////////////////////////////////////////////////////
void frustum_destroy(void* ventity) {

    Frustum* frustum = (Frustum*) ventity;

    delete frustum->shader;
}

////////////////////////////////////////////////////////////////////////////////
float frustum_get_z_containing_points(Frustum* fr, glm::vec3* points, uint32_t n_points) {

    assert(n_points > 2);

    float x_max = fabs(points[0].x);
    float y_max = fabs(points[0].y);

    for (uint32_t i = 1; i < n_points; i++) {

        // assert(points[0].z < fr->farz && points[0].z > fr->nearz);

        x_max = fmaxf(x_max, fabs(points[i].x));
        y_max = fmaxf(y_max, fabs(points[i].y));
    }

    float z_height = frustum_get_z_height(fr, 2.0f * y_max);
    float z_width  = frustum_get_z_width(fr, 2.0f * x_max);

    float z = fmaxf(z_height, z_width);

    // assert(z < fr->farz && z > fr->nearz);

    return z;
}

////////////////////////////////////////////////////////////////////////////////
float frustum_get_z_height(Frustum* fr, float height) {

    return height / (2.0f * tanf(fr->fovy / 2.0f));
}

////////////////////////////////////////////////////////////////////////////////
float frustum_get_z_width(Frustum* fr, float width) {

    float height = width / fr->aspect;
    return frustum_get_z_height(fr, height);
}

////////////////////////////////////////////////////////////////////////////////
void frustum_setup_vertex_buffers(Frustum* fr, std::vector<glm::vec3> vertices,
                                  std::vector<GLuint> indices) {

    glGenVertexArrays(1, &fr->vao);
    glGenBuffers(1, &fr->vbo);
    glGenBuffers(1, &fr->ebo);

    glBindVertexArray(fr->vao);
    glBindBuffer(GL_ARRAY_BUFFER, fr->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fr->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 &indices[0], GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);

    fr->shader = new Shader("src/Shaders/cube",
                            ShaderAttributes::CUBE_ATTRIBUTES);

}

////////////////////////////////////////////////////////////////////////////////
std::vector<glm::vec3> frustum_generate_vertices(Frustum* fr) {

    std::vector<glm::vec3> vertices;

    vertices.push_back(glm::vec3(0.0f));

    for (uint8_t i = 0; i < 8; i++)
        vertices.push_back(fr->corners[i]);

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<GLuint> frustum_generate_indices() {

    return std::vector<GLuint> {
        1, 2, 2, 3, 3, 4, 4, 1,
        5, 6, 6, 7, 7, 8, 8, 5,
        0, 5, 0, 6, 0, 7, 0, 8
    };
}

////////////////////////////////////////////////////////////////////////////////
void frustum_render(void* ventity, RenderInfo* rinfo) {

    Frustum* fr = (Frustum*) ventity;
    fr->shader->Bind();

    glm::mat4 mvp = rinfo->view_proj * fr->model_matrix;

    glUniformMatrix4fv(glGetUniformLocation(fr->shader->Id(), "mvp"), 1,
                       GL_FALSE, &mvp[0][0]);

    glUniform3f(glGetUniformLocation(fr->shader->Id(), "color"),
        fr->color.r, fr->color.g, fr->color.b);

    glBindVertexArray(fr->vao);
    glDrawElements(GL_LINES, fr->n_indices, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
    fr->shader->Unbind();
}

////////////////////////////////////////////////////////////////////////////////
RenderBundle* frustum_create_renderbundle(Frustum* fr) {

    RenderBundle* rbundle = new RenderBundle;
    rbundle->entity = (void*) fr;
    rbundle->render_procedure = &frustum_render;
    rbundle->destroy_procedure = &frustum_destroy;

    return rbundle;
}

/// @file

