#include <stdlib.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "VoxelGrid.hpp"
#include <Common/Utility.hpp>
#include <Graphics/ShaderAttributes.hpp>
#include <viewpoint.hpp>

////////////////////////////////////////////////////////////////////////////////
static std::vector<glm::vec3> generate_vertices(float side_len);
static std::vector<glm::mat4> generate_model_mats(VoxelGrid* grid);
static void voxelgrid_generate_vertex_buffers(VoxelGrid* grid);

////////////////////////////////////////////////////////////////////////////////
VoxelGrid* voxelgrid_create(const unsigned int n_voxels_per_side,
                            const float resolution) {

    VoxelGrid* grid = (VoxelGrid*) malloc(sizeof(VoxelGrid));

    grid->n_voxels_per_side = n_voxels_per_side;
    grid->n_total = pow(n_voxels_per_side, 3);
    grid->resolution = resolution;
    grid->rois = NULL;
    grid->n_rois = 0;

    grid->data = (Voxel*) malloc(grid->n_total * sizeof(Voxel));

    voxelgrid_clear(grid);

    if (glewInit() == GLEW_OK) {

        voxelgrid_generate_vertex_buffers(grid);
        
        grid->shader = new Shader("src/Shaders/voxel_instanced",
                                  ShaderAttributes::VOXEL_INSTANCED_ATTRIBUTES);

        grid->volume_bbox = cube_create(0.0f, 0.0f, 0.0f, glm::vec3(0.4f),
            0.5f * grid->n_voxels_per_side, 0.5f * grid->n_voxels_per_side,
            0.5f * grid->n_voxels_per_side);
    }

    return grid;
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_destroy(void* vgrid) {

    VoxelGrid* grid = (VoxelGrid*) vgrid;
    free(grid->data);
    free(grid);
}


////////////////////////////////////////////////////////////////////////////////
Voxel* voxelgrid_get_voxel_at_position(VoxelGrid* grid, Eigen::Vector3f position) {

    int32 index = voxelgrid_position_to_index(grid, position);
    Voxel* voxel = NULL;
    
    if (index != -1)
        voxel = &grid->data[index];

    return voxel;
}

////////////////////////////////////////////////////////////////////////////////
int32 voxelgrid_position_to_index(VoxelGrid* grid, Eigen::Vector3f position) {

    int32 i = position[0] / grid->resolution;
    int32 j = position[1] / grid->resolution;
    int32 k = position[2] / grid->resolution;
    int32 n = grid->n_voxels_per_side;

    if (i >= 0 && i < n && j >= 0 && j < n && k >= 0 && k < n)
        return Utility::Index3D(i, j, k, n);
    else
        return -1;
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_occupy_tsdf_surface(VoxelGrid* grid, float tsdf_threshold) {

    uint32 n = grid->n_voxels_per_side;

    for (uint32 i = 0; i < n; i++) {

        for (uint32 j = 0; j < n; j++) {

            for (uint32 k = 0; k < n; k++) {

                uint32 index = Utility::Index3D(i, j, k, n);

                Voxel* voxel = grid->data + index;

                if (fabsf(voxel->tsdf) < tsdf_threshold)
                    voxel->label = FILL;
                else
                    voxel->label = VOID;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_occupy_count(VoxelGrid* grid, uint32_t n_hits_thresh) {

    for (uint32_t i = 0; i < grid->n_total; i++) {

        Voxel* voxel = &grid->data[i];

        if (voxel->n_hits >= n_hits_thresh) {
            voxel->label = FILL;
        }
        else
            voxel->label = VOID;
    }
}

////////////////////////////////////////////////////////////////////////////////
void voxel_update_tsdf(Voxel* voxel, float tsdf, float weight) {

    if (voxel->tsdf == 2.0f)
        voxel->tsdf = 0.0f;

    voxel->tsdf    = (voxel->weight * voxel->tsdf + weight * tsdf) / (voxel->weight + weight);
    voxel->weight += weight;
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_setup_for_rendering(VoxelGrid* voxelgrid) {

    std::vector<GLuint> indices {

        0, 1, 4, 1, 4, 5,
        0, 4, 3, 4, 3, 7,
        4, 5, 7, 5, 7, 6,
        1, 5, 2, 5, 2, 6,
        3, 2, 7, 2, 7, 6,
        0, 1, 3, 1, 3, 2
    };

    std::vector<glm::vec3> vertices = generate_vertices(0.5f);
    std::vector<glm::mat4> model_mats = generate_model_mats(voxelgrid);
    voxelgrid_setup_vertex_buffer(voxelgrid, vertices, indices, model_mats);
}

////////////////////////////////////////////////////////////////////////////////
RenderBundle* voxelgrid_create_renderbundle(VoxelGrid* voxelgrid) {

    RenderBundle* rbundle = new RenderBundle;
    rbundle->entity = (void*) voxelgrid;
    rbundle->render_procedure = &voxelgrid_render;
    rbundle->destroy_procedure = &voxelgrid_destroy;

    return rbundle;
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_setup_vertex_buffer(VoxelGrid* grid, std::vector<glm::vec3>& vertices, 
                                   std::vector<GLuint>& indices, std::vector<glm::mat4>& model_mats) {

    grid->n_indices = indices.size();
    glBindVertexArray(grid->vao);
    glBindBuffer(GL_ARRAY_BUFFER, grid->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
                 &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, grid->mbo);
    glBufferData(GL_ARRAY_BUFFER, model_mats.size() * sizeof(glm::mat4), &model_mats[0], GL_STATIC_DRAW);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) 0);
    glEnableVertexAttribArray(1); 

    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) sizeof(glm::vec4));
    glEnableVertexAttribArray(2); 

    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) (2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(3); 

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (GLvoid*) (3 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(4); 

    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_render(void* ventity, RenderInfo* renderinfo) {

    VoxelGrid* grid = (VoxelGrid*) ventity;
    grid->shader->Bind();

    glm::mat4 vp = renderinfo->view_proj;

    glUniformMatrix4fv(glGetUniformLocation(grid->shader->Id(), "view_proj"), 1,
                       GL_FALSE, &vp[0][0]);

    glUniform1f(glGetUniformLocation(grid->shader->Id(), "miny"), grid->miny);
    glUniform1f(glGetUniformLocation(grid->shader->Id(), "maxy"), grid->maxy);

    glBindVertexArray(grid->vao);
    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, NULL, grid->n_occupied);
    glBindVertexArray(0);
    grid->shader->Unbind();
    cube_render((void*) &grid->volume_bbox, renderinfo);

    for (uint32_t i = 0; i < grid->n_rois; i++)
        cube_render((void*) &grid->rois[i], renderinfo);
}

////////////////////////////////////////////////////////////////////////////////
static std::vector<glm::vec3> generate_vertices(float side_len) {

    float min_x = 0.0f;
    float min_y = 0.0f;
    float min_z = 0.0f;

    float max_x = min_x + side_len;
    float max_y = min_y + side_len;
    float max_z = min_z + side_len;

    float x0 = min_x;
    float y0 = min_y;
    float z0 = min_z;

    float x1 = max_x;
    float y1 = max_y;
    float z1 = max_z;

    std::vector<glm::vec3> vertices {

        glm::vec3(x0, y0, z0),
        glm::vec3(x1, y0, z0),
        glm::vec3(x1, y1, z0),
        glm::vec3(x0, y1, z0),

        glm::vec3(x0, y0, z1),
        glm::vec3(x1, y0, z1),
        glm::vec3(x1, y1, z1),
        glm::vec3(x0, y1, z1)
    };

    return vertices;
}

////////////////////////////////////////////////////////////////////////////////
static std::vector<glm::mat4> generate_model_mats(VoxelGrid* grid) {

    // TODO: Use the same separation as the one used to generate the vertices
    static const float SIDE_LEN = 0.5f;
    const uint32_t n = grid->n_voxels_per_side;
    float miny = 99999.9f;
    float maxy = -99999.9f;

    std::vector<glm::mat4> model_mats;

    for (uint32_t i = 0; i < n; i++) {

        for (uint32_t j = 0; j < n; j++) {

            for (uint32_t k = 0; k < n; k++) {

                uint32_t index3d = Utility::Index3D(i, j, k, n);
                Voxel voxel = grid->data[index3d];
                if (voxel.label == VOID) continue;

                glm::mat4 m(1.0f);
                m = glm::translate(m, SIDE_LEN * glm::vec3(i, j , k));
                model_mats.push_back(m);

                miny = (m[3][1] < miny) ? m[3][1] : miny;
                maxy = (m[3][1] > maxy) ? m[3][1] : maxy;
            }
        }
    }

    grid->miny = miny;
    grid->maxy = maxy;
    std::cout << "min max: " << miny << ", " << maxy << std::endl;
    grid->n_occupied = model_mats.size();
    return model_mats;
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_clear(VoxelGrid* grid) {

    for (uint32_t i = 0; i < pow(grid->n_voxels_per_side, 3); i++) {

        Voxel voxel;
        voxel.label = VOID;
        voxel.tsdf = 2.0f;
        voxel.weight = 0.0f;
        voxel.n_hits = 0;
        grid->data[i] = voxel;
    }
}

////////////////////////////////////////////////////////////////////////////////
static void voxelgrid_generate_vertex_buffers(VoxelGrid* grid) {

    glGenVertexArrays(1, &grid->vao);
    glGenBuffers(1, &grid->vbo);
    glGenBuffers(1, &grid->mbo);
    glGenBuffers(1, &grid->ebo);
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_to_file(VoxelGrid* grid, const std::string& dst_path) {

    std::ofstream outfile;
    outfile.open(dst_path, std::ios::out);
    
    const uint32_t n = grid->n_voxels_per_side;

    for (uint32_t y = 0; y < n; y++) {
        for (uint32_t z = 0; z < n; z++) {
            for (uint32_t x = 0; x < n; x++) {
                
                uint32_t idx = Utility::Index3D(x, y, z, n);
                Voxel* voxel = &grid->data[idx];

                outfile << ((voxel->label == FILL) ? 1 : 0) << " ";
            }

            outfile << std::endl;
        }

        outfile << std::endl;
    }

    outfile.close();
}

////////////////////////////////////////////////////////////////////////////////
void voxelgrid_add_bounding_box(VoxelGrid* grid, Cube cube) {

    grid->n_rois++;
    grid->rois = (Cube*) realloc(grid->rois, grid->n_rois * sizeof(Cube));
    grid->rois[grid->n_rois - 1] = cube;
}

////////////////////////////////////////////////////////////////////////////////
std::string voxelgrid_occupied_to_string(VoxelGrid* grid) {

    std::string out = "[";

    const uint32_t n = grid->n_voxels_per_side;

    for (uint32_t j = 0; j < n; j++) {
        for (uint32_t k = 0; k < n; k++) {
            for (uint32_t i = 0; i < n; i++) {
                
                uint32_t idx = Utility::Index3D(i, j, k, n);
                Voxel* voxel = &grid->data[idx];
                if (voxel->label == VOID) continue;
                
                if (out.length() > 1)
                    out += ", ";

                std::string point = "[" + std::to_string(i) + ", ";
                point += std::to_string(j) + ", ";
                point += std::to_string(k) + "]";
                out += point;
            }
        }
    }

    out += "]";

    return out;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Eigen::Vector3f> voxelgrid_get_occupied(VoxelGrid* grid) {

    std::vector<Eigen::Vector3f> occupied;

    const uint32_t n = grid->n_voxels_per_side;

    for (uint32_t j = 0; j < n; j++) {
        for (uint32_t k = 0; k < n; k++) {
            for (uint32_t i = 0; i < n; i++) {
                
                uint32_t idx = Utility::Index3D(i, j, k, n);
                Voxel* voxel = &grid->data[idx];
                if (voxel->label == VOID) continue;
                
                occupied.push_back(Eigen::Vector3f(i, j, k));
            }
        }
    }

    return occupied;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Cube> voxelgrid_clusterize(VoxelGrid* grid) {

    std::cout << "Performing clustering" << std::endl;
    std::vector<Eigen::Vector3f> dataset = voxelgrid_get_occupied(grid);
    std::vector<Cube> out;

    if (dataset.size() == 0) {

        std::cout << "Warning: Dataset empty. Clustering omitted" << std::endl;
        return out;
    }

    static const Eigen::Vector3f MAX_BOX_LENGTH = Eigen::Vector3f::Ones() * 60.0f;
    std::vector<Eigen::Vector3f> min, len;
    clustering_get_boxes(&dataset, &min, &len, MAX_BOX_LENGTH);
    std::cout << "n clusters: " << min.size() << std::endl;

    // static const Eigen::Vector3f MIN_BOX_LENGTH = Eigen::Vector3f::Ones() * 2.0f;

    for (uint32_t i = 0; i < min.size(); i++) {

        /*
        if (len[i][0] > MIN_BOX_LENGTH[0] &&
            len[i][1] > MIN_BOX_LENGTH[1] &&
            len[i][2] > MIN_BOX_LENGTH[2]) {
        */

            Eigen::Vector3f min_adj = min[i] * 0.5f;
            Eigen::Vector3f len_adj = len[i] * 0.5f;
            Cube cube = cube_create(min_adj[0], min_adj[1], min_adj[2],
                glm::vec3(0.3f, 8.0f, 0.0f), len_adj[0], len_adj[1], len_adj[2]);
            
            voxelgrid_add_bounding_box(grid, cube);
            out.push_back(cube);
        // }
    }

    return out;

    /*
    std::vector<Cluster> optimal_clusters = clustering_clusterize(&dataset, &min, &len);
    std::cout << "n clusters: " << min.size() << std::endl;
    for (uint32_t i = 0; i < min.size(); i++) {

        Eigen::Vector3f min_adj = min[i] * 0.5f;
        Eigen::Vector3f len_adj = len[i] * 0.5f;
        Cube cube = cube_create(min_adj[0], min_adj[1], min_adj[2],
            glm::vec3(0.8f, 0.0f, 0.0f), len_adj[0], len_adj[1], len_adj[2]);
        
        voxelgrid_add_bounding_box(grid, cube);
    }
    */

    /*
    std::vector<Eigen::Vector3f> centers;
    std::cout << "n clusters: " << optimal_clusters.size() << std::endl;

    for (uint32_t i = 0; i < optimal_clusters.size(); i++) {

        Eigen::Vector3f box_max = Eigen::Vector3f(1.0f, 1.0f, 1.0f) * -99999.9f;
        Eigen::Vector3f box_min = Eigen::Vector3f(1.0f, 1.0f, 1.0f) * 99999.9f;

        for (uint32_t j = 0; j < optimal_clusters[i].data.size(); j++) {

            for (uint8_t k = 0; k < 3; k ++) {

                box_max[k] = fmaxf(box_max[k], optimal_clusters[i].data[j][k]);
                box_min[k] = fminf(box_min[k], optimal_clusters[i].data[j][k]);
            }
        }

        Eigen::Vector3f box_length = (box_max - box_min + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) * 0.5f;
        box_min = box_min * 0.5f;

        centers.push_back(Eigen::Vector3f(box_min + box_length / 2.0f));
        
        // voxelgrid_add_bounding_box(grid, cube);
        // cubes.push_back(cube);
    }

    std::vector<Cluster> optimal_centers = clustering_clusterize(&centers);
    std::cout << "n optimal centers: " << optimal_centers.size() << std::endl;
    */
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Cube> voxelgrid_merge_cubes(VoxelGrid* grid, std::vector<Cube>* cubes) {

    static const Eigen::Vector3f OVERLAP_THRESHOLD = Eigen::Vector3f(1.0f, 1.0f, 1.0f) * 0.2f;
    std::vector<Cube> clusters_merged;

    for (uint32_t i = 0; i < cubes->size() - 1; i++) {

        Cube* c1 = &(*cubes)[i];
        
        for (uint32_t j = i + 1; j < cubes->size(); j++) {

            Cube* c2 = &(*cubes)[j];

            float x_overlap, y_overlap, z_overlap;
            cube_overlap(c1, c2, &x_overlap, &y_overlap, &z_overlap);

            if (x_overlap >= OVERLAP_THRESHOLD[0] &&
                y_overlap >= OVERLAP_THRESHOLD[1] &&
                z_overlap >= OVERLAP_THRESHOLD[2]) {

                Cube merged = cube_merge(c1, c2);
                clusters_merged.push_back(merged);
            }
            else {

                clusters_merged.push_back(*c1);
            }
        }
    }

    return clusters_merged;
}

////////////////////////////////////////////////////////////////////////////////
char* voxelgrid_serialize(VoxelGrid* grid) {

    const uint32_t n = grid->n_voxels_per_side;
        
    char* serialized = NULL;
    uint32_t size = 1; // to hold null-terminator
    serialized = (char*) malloc(size);
    serialized[0] = '\0';

    for (uint32_t y = 0; y < n; y++) {

        for (uint32_t z = 0; z < n; z++) {

            for (uint32_t x = 0; x < n; x++) {

                uint32_t idx = Utility::Index3D(x, y, z, n);
                Voxel* voxel = &grid->data[idx];

                if (voxel->label != VOID) {

                    char voxel_status[512];
                    memset(voxel_status, 0, 512);

                    sprintf(voxel_status, "%u,%u,%u\n", x, y, z);

                    size += strlen(voxel_status);
                    serialized = (char*) realloc(serialized, size);

                    strcat(serialized, voxel_status);
                }
            }
        }
    }

    return serialized;
}

/// @file 

