#ifndef CLUSTERING_HPP
#define CLUSTERING_HPP

#include <vector>
#include <Eigen/Eigen>
#include <ALGLIB/dataanalysis.h>

typedef struct Cluster {

    Eigen::Vector3f p0;
    std::vector<Eigen::Vector3f> data;
} Cluster;

////////////////////////////////////////////////////////////////////////////////
std::vector<Cluster> clustering_optimal_clusters(
    std::vector<Eigen::Vector3f>* dataset,alglib::ahcreport* rep,
    std::vector<Eigen::Vector3f>* min, std::vector<Eigen::Vector3f>* len);

void clustering_apply_merge(std::vector<Cluster>* clusters,
                            std::vector<Cluster>* rec, uint32_t merge_a,
                            uint32_t merge_b,
                            std::vector<Eigen::Vector3f>* dataset);

float clustering_gain(std::vector<Cluster>* clusters, double p0);

alglib::ahcreport clustering_ahc(std::string datastring);

std::vector<Cluster> clustering_clusterize(std::vector<Eigen::Vector3f>* dataset,
    std::vector<Eigen::Vector3f>* min, std::vector<Eigen::Vector3f>* len);

std::string clustering_dataset_to_string(std::vector<Eigen::Vector3f>* dataset);

void clustering_get_boxes(
    std::vector<Eigen::Vector3f>* dataset,
    std::vector<Eigen::Vector3f>* box_mins,
    std::vector<Eigen::Vector3f>* box_lengths, Eigen::Vector3f box_threshold);

////////////////////////////////////////////////////////////////////////////////
Cluster cluster_create(std::vector<Eigen::Vector3f> data);

Cluster cluster_merge(Cluster a, Cluster b);

bool cluster_cmp(Cluster* a, Cluster* b);

void cluster_print(Cluster* c); 

#endif // CLUSTERING_HPP

/// @file

