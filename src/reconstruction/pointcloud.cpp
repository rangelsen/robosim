#include <stdlib.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "pointcloud.hpp"
#include <Graphics/ShaderAttributes.hpp>

////////////////////////////////////////////////////////////////////////////////
static std::vector<Vertex> eigen_to_vertex(std::vector<Eigen::Vector3f>* points,
    glm::vec3 color) {

    std::vector<Vertex> vertices;

    for (uint32_t i = 0; i < points->size(); i++) {

        Vertex vertex;
        vertex.Position = glm::vec3((*points)[i][0], (*points)[i][1],
            (*points)[i][2]);
        
        vertices.push_back(vertex);
    }

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////
static void pcloud_update_min_max(Pointcloud* pcloud);

////////////////////////////////////////////////////////////////////////////////
Pointcloud* pcloud_create() {

    Pointcloud* pcloud = new Pointcloud;
    
    glGenVertexArrays(1, &pcloud->vao);
    glGenBuffers(1, &pcloud->vbo);
    pcloud->color = glm::vec3(0.1f, 0.6f, 0.4f);
    pcloud->shader = new Shader("src/Shaders/pointcloud",
        ShaderAttributes::PCLOUD_ATTRIBUTES);

    return pcloud;
}

////////////////////////////////////////////////////////////////////////////////
void pcloud_destroy(void* ventity) {

    Pointcloud* pcloud = (Pointcloud*) ventity;
    delete pcloud->shader;
    delete pcloud;
}

////////////////////////////////////////////////////////////////////////////////
void pcloud_setup_for_rendering(Pointcloud* pcloud) {

    std::vector<Vertex> vertices = eigen_to_vertex(&pcloud->points,
        glm::vec3(0.4f, 0.1f, 0.1f));

    glBindVertexArray(pcloud->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pcloud->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (GLvoid*) offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////
void pcloud_render(void* ventity, RenderInfo* rinfo) {

    Pointcloud* pcloud = (Pointcloud*) ventity;
    pcloud->shader->Bind();

    glUniform3f(glGetUniformLocation(pcloud->shader->Id(), "color"), 
        pcloud->color.r, pcloud->color.g, pcloud->color.b);

    glm::mat4 vp = rinfo->view_proj;
    glUniformMatrix4fv(glGetUniformLocation(pcloud->shader->Id(), "view_proj"), 1,
                       GL_FALSE, &vp[0][0]);

    glBindVertexArray(pcloud->vao);
    glDrawArrays(GL_POINTS, 0, pcloud->n_points);
    glBindVertexArray(0);
    pcloud->shader->Unbind();
}

////////////////////////////////////////////////////////////////////////////////
RenderBundle* pcloud_create_renderbundle(Pointcloud* pcloud) {

    RenderBundle* rbundle = new RenderBundle;
    rbundle->entity = (void*) pcloud;
    rbundle->render_procedure = &pcloud_render;
    rbundle->destroy_procedure = &pcloud_destroy;

    return rbundle;
}

////////////////////////////////////////////////////////////////////////////////
void pcloud_add_points(Pointcloud* pcloud, std::vector<Eigen::Vector3f>* points) {

    pcloud->points.insert(pcloud->points.end(), points->begin(), points->end());
    pcloud->n_points = pcloud->points.size();
    pcloud_update_min_max(pcloud);
}

////////////////////////////////////////////////////////////////////////////////
static void pcloud_update_min_max(Pointcloud* pcloud) {
    
    glm::vec3 min = 99999.9f * glm::vec3(1.0f);
    glm::vec3 max = -99999.9f * glm::vec3(1.0f);

    for (uint32_t i = 0; i < pcloud->points.size(); i++) {

        for (uint8_t j = 0; j < 3; j++) {

            min[j] = (pcloud->points[i][j] < min[j]) ?
                pcloud->points[i][j] : min[j];

            max[j] = (pcloud->points[i][j] > max[j]) ?
                pcloud->points[i][j] : max[j];
        }
    }

    pcloud->min = min;
    pcloud->max = max;
}

/// @file

