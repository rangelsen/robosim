#ifndef VOXEL_GRID_H
#define VOXEL_GRID_H

#include <vector>
#include <Eigen/Eigen>
#include <GL/glew.h>

#include "Label.h"
#include <Graphics/Vertex.hpp>
#include <Graphics/RenderBundle.h>
#include <Graphics/Shader.hpp>
#include <Graphics/Cube.hpp>
#include <Clustering/clustering.hpp>

////////////////////////////////////////////////////////////////////////////////
typedef struct Voxel {

    Label label;
    float tsdf;
    float weight;
    uint32_t n_hits = 0;
} Voxel;

typedef struct VoxelGrid {
    
    Voxel* data;
    float resolution;
    uint32_t n_voxels_per_side;
    float dim;

    GLuint vao, vbo, ebo, mbo;

    /**
     * @breif Shader uniforms for color intepolation
     */
    float maxy, miny;

    uint32_t n_indices;
    uint32_t n_occupied;
    uint32_t n_total;
    Shader* shader;
    Cube volume_bbox;
    Cube* rois;
    uint32_t n_rois;
} VoxelGrid;

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Allocate a grid of voxels
 * @param n_voxels_per_side number of voxels per side of the grid
 * @param resolution length of the sides of the voxels
 */
VoxelGrid* voxelgrid_create(const unsigned int n_voxels_per_side,
    const float resolution);

void voxelgrid_destroy(void* vgrid);

/**
 * @brief Get voxel at 3D world position
 * @param position world frame position
 */
Voxel* voxelgrid_get_voxel_at_position(VoxelGrid* grid,
    Eigen::Vector3f position);

/**
 * @brief Get grid index corresponding to 3D world frame position
 */
int32 voxelgrid_position_to_index(VoxelGrid* grid, Eigen::Vector3f position);

/**
 * @brief Goes through all the voxels in the grid an labels the ones with
 * absolute value less than a given threshold as occupied
 */
void voxelgrid_occupy_tsdf_surface(VoxelGrid* grid, float tsdf_threshold);

void voxelgrid_occupy_count(VoxelGrid* grid, uint32_t n_hits_thresh);

/**
 * @brief Generate a vector of vertices from a voxel used for rendering
 */
std::vector<Vertex> voxel_generate_vertices(Voxel* voxel, int i, int j, int k,
    float resolution);

/**
 * @brief Apply the TSDF update
 */
void voxel_update_tsdf(Voxel* voxel, float tsdf, float weight);

void voxelgrid_setup_for_rendering(VoxelGrid* voxelgrid);

RenderBundle* voxelgrid_create_renderbundle(VoxelGrid* voxelgrid);

void voxelgrid_setup_vertex_buffer(VoxelGrid* grid,
    std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices,
    std::vector<glm::mat4>& model_mats);

void voxelgrid_render(void* ventity, RenderInfo* renderinfo);

std::vector<Vertex> generate_cube_vertices(float side_len);

void voxelgrid_clear(VoxelGrid* grid);

void voxelgrid_to_file(VoxelGrid* grid, const std::string& dst_path);

void voxelgrid_render_boundary(VoxelGrid* grid);

void voxelgrid_add_bounding_box(VoxelGrid* grid, uint32_t x, uint32_t y,
    uint32_t z, Eigen::Vector3f box_length);

std::string voxelgrid_occupied_to_string(VoxelGrid* grid);

std::vector<Eigen::Vector3f> voxelgrid_get_occupied(VoxelGrid* grid);

std::vector<Cube> voxelgrid_clusterize(VoxelGrid* grid);

std::vector<Cube> voxelgrid_merge_cubes(VoxelGrid* grid,
    std::vector<Cube>* cubes);

char* voxelgrid_serialize(VoxelGrid* grid);

#endif // VOXEL_GRID_H

/// @file

