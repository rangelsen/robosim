#include <GL/glew.h>
#include <vector>
#include <Graphics/Vertex.hpp>
#include <Graphics/ShaderAttributes.hpp>

#include "grid.hpp"

////////////////////////////////////////////////////////////////////////////////
static std::vector<Vertex> grid_gen_vertices(Grid* grid, float x0, float z0,
    float xlen, float zlen);

////////////////////////////////////////////////////////////////////////////////
Grid* grid_create(float x0, float z0, float xlen, float zlen) {

    Grid* grid = new Grid;

    std::vector<Vertex> vertices = grid_gen_vertices(grid, x0, z0, xlen, zlen);
    grid->n_vertices = vertices.size();

    glGenVertexArrays(1, &grid->vao);
    glGenBuffers(1, &grid->vbo);

    glBindVertexArray(grid->vao);
    glBindBuffer(GL_ARRAY_BUFFER, grid->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (GLvoid*) offsetof(Vertex, Position));
    glEnableVertexAttribArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);

    grid->shader = new Shader("src/Shaders/pointcloud",
        ShaderAttributes::PCLOUD_ATTRIBUTES);

    grid->color = glm::vec3(0.4f);
    return grid;
}

////////////////////////////////////////////////////////////////////////////////
void grid_destroy(void* ventity) {

    Grid* grid = (Grid*) ventity;
    delete grid->shader;
    delete grid;
}

////////////////////////////////////////////////////////////////////////////////
static std::vector<Vertex> grid_gen_vertices(Grid* grid, float x0, float z0,
    float xlen, float zlen) {

    std::vector<Vertex> vertices;
    
    const uint16_t NSEP_X = 100;
    const uint16_t NSEP_Z = 100;

    const float DX = xlen / NSEP_X;
    const float DZ = zlen / NSEP_Z;

    // Place grid lines along x axis
    for (uint32_t i = 0; i < NSEP_X + 1; i++) {

        Vertex v0;
        v0.Position = glm::vec3(x0, 0.0f, z0 + (DZ * i));
        
        Vertex v1;
        v1.Position = glm::vec3(x0 + xlen, 0.0f, v0.Position.z);

        vertices.push_back(v0);
        vertices.push_back(v1);
    }

    // Place grid lines along z axis
    for (uint32_t i = 0; i < NSEP_Z + 1; i++) {

        Vertex v0;
        v0.Position = glm::vec3(x0 + DX * i, 0.0f, z0);
        
        Vertex v1;
        v1.Position = glm::vec3(v0.Position.x, 0.0f, z0 + zlen);

        vertices.push_back(v0);
        vertices.push_back(v1);
    }

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////
void grid_render(void* ventity, RenderInfo* rinfo) {

    Grid* grid = (Grid*) ventity;
    grid->shader->Bind();

    glUniform3f(glGetUniformLocation(grid->shader->Id(), "color"), 
        grid->color.r, grid->color.g, grid->color.b);

    glm::mat4 vp = rinfo->view_proj;
    glUniformMatrix4fv(glGetUniformLocation(grid->shader->Id(), "view_proj"), 1,
                       GL_FALSE, &vp[0][0]);

    glBindVertexArray(grid->vao);
    glDrawArrays(GL_LINES, 0, grid->n_vertices);
    glBindVertexArray(0);
    grid->shader->Unbind();
}

////////////////////////////////////////////////////////////////////////////////
RenderBundle* grid_create_renderbundle(Grid* grid) {

    RenderBundle* rbundle = new RenderBundle;
    rbundle->entity = (void*) grid;
    rbundle->render_procedure = &grid_render;
    rbundle->destroy_procedure = &grid_destroy;

    return rbundle;
}

/// @file

