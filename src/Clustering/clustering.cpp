#include "clustering.hpp"
#include <Common/Utility.hpp>

////////////////////////////////////////////////////////////////////////////////
static void clustering_get_min_max(std::vector<Eigen::Vector3f>* points,
                                Eigen::Vector3f* min, Eigen::Vector3f* max);

static std::vector<Eigen::Vector3f> clustering_get_points_by_dendro_index(
    uint32_t dendro_index, alglib::ahcreport* rep,
    std::vector<Eigen::Vector3f>* dataset);

static void clustering_get_largest_under_threshold(uint32_t dendrogram_index, 
    alglib::ahcreport* rep, std::vector<Eigen::Vector3f>* dataset,
    Eigen::Vector3f box_threshold, std::vector<Eigen::Vector3f>* mins_out,
    std::vector<Eigen::Vector3f>* lens_out);

////////////////////////////////////////////////////////////////////////////////
float clustering_compute_gain(std::vector<Cluster>* clusters, Eigen::Vector3f p0) {
    
    float gain = 0.0;
    
    for (uint32_t i = 0; i < clusters->size(); i++) {

        Eigen::Vector3f dp0 = p0 - (*clusters)[i].p0;
        gain += ((*clusters)[i].data.size() - 1) * pow(dp0.norm(), 2);
    }

    return gain;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Cluster> clustering_optimal_clusters(
    std::vector<Eigen::Vector3f>* dataset, alglib::ahcreport* rep,
    std::vector<Eigen::Vector3f>* mins, std::vector<Eigen::Vector3f>* lens) {

    // Non-singleton clusters
    std::vector<Cluster> clusters;

    // Reconstruction of data set after merges are applied
    std::vector<Cluster> rec_clusters;

    // Optimal reconstruction of the data set merges
    std::vector<Cluster> optimal_clustering;
    float optimal_gain = 0.0;

    Eigen::Vector3f global_centroid = Utility::ComputeCentroid(dataset);

    for (uint32_t i = 0; i < rep->z.rows(); i++) {

        clustering_apply_merge(&clusters, &rec_clusters, rep->z[i][0], rep->z[i][1], dataset);

        float gain = clustering_compute_gain(&rec_clusters, global_centroid);

        if (gain > optimal_gain) {

            optimal_gain = gain;
            optimal_clustering = rec_clusters;
        }
    }

    /*
    std::cout << 1 << std::endl;
    static const Eigen::Vector3f BOX_THRESHOLD = Eigen::Vector3f(1.0f, 1.0f, 1.0f) * 10.0f;
    for (uint32_t i = clusters.size() - 1; i > 0; i--) {

        std::cout << 1.2 << std::endl;
        std::cout << "i: " << i << "/" << clusters.size() << std::endl;
        cluster_print(&clusters[i]);
        Eigen::Vector3f min, max;
        std::cout << 1.3 << std::endl;
        cluster_get_min_max(&clusters[i], &min, &max);

        std::cout << "min: " << std::endl << min << std::endl;
        std::cout << "max: " << std::endl << max << std::endl;

        std::cout << 1.1 << std::endl;
        Eigen::Vector3f length = max - min;

        Eigen::Vector3i is_smaller(0, 0, 0);
        std::cout << 2 << std::endl;

        for (uint8_t j = 0; j < 3; j++) {
            
            if (length[j] < BOX_THRESHOLD[j]) {

                is_smaller[j] = 1;
            }
        }

        if (is_smaller[0] && is_smaller[1] && is_smaller[2]) {
            std::cout << 3 << std::endl;

            mins->push_back(min);
            lens->push_back(length + Eigen::Vector3f(1.0f, 1.0f, 1.0f));
        }
    }
    */

    return optimal_clustering;
}

////////////////////////////////////////////////////////////////////////////////
void clustering_apply_merge(std::vector<Cluster>* clusters,
    std::vector<Cluster>* rec, uint32_t merge_a, uint32_t merge_b,
    std::vector<Eigen::Vector3f>* dataset) {
    
    Cluster cluster_a, cluster_b;
    uint32_t n = dataset->size();
    Cluster* delete_a = NULL;
    Cluster* delete_b = NULL;

    if (merge_a < n)
        cluster_a = cluster_create(std::vector<Eigen::Vector3f> { (*dataset)[merge_a] });
    else {
        cluster_a = (*clusters)[merge_a - n];
        delete_a = &cluster_a;
    }

     if (merge_b < n)
        cluster_b = cluster_create(std::vector<Eigen::Vector3f> { (*dataset)[merge_b] });
    else {
        cluster_b = (*clusters)[merge_b - n];
        delete_b = &cluster_b;
    }
   
    Cluster cluster = cluster_merge(cluster_a, cluster_b);

    clusters->push_back(cluster);

    // Delete clusters that have been merged from the clustering reconstruction
    if (delete_a || delete_b) {

        for (uint32_t i = 0; i < rec->size(); i++) {

            if (cluster_cmp(delete_a, &(*rec)[i]))
                rec->erase(rec->begin() + i);

            if (cluster_cmp(delete_b, &(*rec)[i]))
                rec->erase(rec->begin() + i);
        }
    }

    rec->push_back(cluster);
}

////////////////////////////////////////////////////////////////////////////////
alglib::ahcreport clustering_ahc(std::string datastring) {

    alglib::clusterizerstate s;
    alglib::ahcreport rep;
    alglib::ae_int_t disttype;
    alglib::real_2d_array xy = datastring.c_str();

    try {
        alglib::clusterizercreate(s);

        disttype = 2;
        alglib::clusterizersetpoints(s, xy, disttype);
        alglib::clusterizerrunahc(s, rep);
    }
    catch (alglib::ap_error e) {

        printf("ALGLIB Error: %s\n", e.msg.c_str());
    }

    return rep;
}

////////////////////////////////////////////////////////////////////////////////
std::vector<Cluster> clustering_clusterize(std::vector<Eigen::Vector3f>* dataset,
    std::vector<Eigen::Vector3f>* min, std::vector<Eigen::Vector3f>* len) {

    std::string datastring = clustering_dataset_to_string(dataset);
    alglib::ahcreport report = clustering_ahc(datastring);
    return clustering_optimal_clusters(dataset, &report, min, len);
}

////////////////////////////////////////////////////////////////////////////////
std::string clustering_dataset_to_string(std::vector<Eigen::Vector3f>* dataset) {

    std::string out = "[";

    for (uint32_t i = 0; i < dataset->size(); i++) {
        
        if (out.length() > 1)
            out += ", ";

        std::string point = "[" + std::to_string((*dataset)[i][0]) + ", ";
        point += std::to_string((*dataset)[i][1]) + ", ";
        point += std::to_string((*dataset)[i][2]) + "]";
        out += point;
    }

    out += "]";

    return out;
}

////////////////////////////////////////////////////////////////////////////////
void clustering_get_boxes(
    std::vector<Eigen::Vector3f>* dataset, std::vector<Eigen::Vector3f>* box_mins,
    std::vector<Eigen::Vector3f>* box_lengths, Eigen::Vector3f box_threshold) {
    
    std::string datastring = clustering_dataset_to_string(dataset);
    alglib::ahcreport rep = clustering_ahc(datastring);

    clustering_get_largest_under_threshold(dataset->size() + rep.z.rows() - 1,
        &rep, dataset, box_threshold, box_mins, box_lengths);
}

////////////////////////////////////////////////////////////////////////////////
static void clustering_get_largest_under_threshold(uint32_t dendrogram_index, 
    alglib::ahcreport* rep, std::vector<Eigen::Vector3f>* dataset,
    Eigen::Vector3f box_threshold, std::vector<Eigen::Vector3f>* mins_out,
    std::vector<Eigen::Vector3f>* lens_out) {

    std::vector<Eigen::Vector3f> cluster_points =
        clustering_get_points_by_dendro_index(dendrogram_index, rep, dataset);

    Eigen::Vector3f min, max;
    clustering_get_min_max(&cluster_points, &min, &max);
    Eigen::Vector3f length = max - min + Eigen::Vector3f::Ones();

    if (length[0] > box_threshold[0] || length[1] > box_threshold[1] || 
        length[2] > box_threshold[2]) {
        
        clustering_get_largest_under_threshold(
            rep->z[dendrogram_index - dataset->size()][0], rep, dataset,
            box_threshold, mins_out, lens_out);

        clustering_get_largest_under_threshold(
            rep->z[dendrogram_index - dataset->size()][1], rep, dataset,
            box_threshold, mins_out, lens_out);
    }
    else {

        if (cluster_points.size() > 15) {
            mins_out->push_back(min);
            lens_out->push_back(length);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static std::vector<Eigen::Vector3f> clustering_get_points_by_dendro_index(
    uint32_t dendro_index, alglib::ahcreport* rep,
    std::vector<Eigen::Vector3f>* dataset) {

    std::vector<Eigen::Vector3f> cluster_points;
    
    uint32_t c;

    if (dendro_index >= dataset->size()) {

        for (uint8_t i = 0; i < 2; i++) {

            c = rep->z[dendro_index - dataset->size()][i];

            std::vector<Eigen::Vector3f> new_points =
                clustering_get_points_by_dendro_index(c, rep, dataset);

            cluster_points.insert(cluster_points.end(), new_points.begin(),
                new_points.end());
        }
    }
    else{

        c = dendro_index;
        cluster_points.push_back((*dataset)[c]);
    }

    return cluster_points;
}

////////////////////////////////////////////////////////////////////////////////
static void clustering_get_min_max(std::vector<Eigen::Vector3f>* points,
                                Eigen::Vector3f* min,
                                Eigen::Vector3f* max) {

    if (points->size() == 0) return;
    Eigen::Vector3f min_tmp(9999.9f, 9999.9f, 9999.9f);
    Eigen::Vector3f max_tmp(0.0f, 0.0f, 0.0f);

    for (uint32_t i = 0; i < points->size(); i++) {

        for (uint8_t j = 0; j < 3; j++) {

            min_tmp[j] = fminf(min_tmp[j], (*points)[i][j]);
            max_tmp[j] = fmaxf(max_tmp[j], (*points)[i][j]);
        }
    }

    *min = min_tmp;
    *max = max_tmp;
}

////////////////////////////////////////////////////////////////////////////////
Cluster cluster_create(std::vector<Eigen::Vector3f> data) {

    Cluster cluster;
    cluster.data = std::vector<Eigen::Vector3f>(data);

    cluster.p0 = Utility::ComputeCentroid(&cluster.data);

    return cluster;
}

////////////////////////////////////////////////////////////////////////////////
Cluster cluster_merge(Cluster a, Cluster b) {

    Cluster cluster;
    cluster.data.insert(cluster.data.end(), a.data.begin(), a.data.end());
    cluster.data.insert(cluster.data.end(), b.data.begin(), b.data.end());

    cluster.p0 = Utility::ComputeCentroid(&cluster.data);

    return cluster;
}

////////////////////////////////////////////////////////////////////////////////
bool cluster_cmp(Cluster* a, Cluster* b) {

    bool equal = true;

    if (!a || !b) {
        equal = false;
    }
    else if (a->data.size() != b->data.size() || !a->p0.isApprox(b->p0)) {
        
        equal = false;
    }
    else {

        for (uint32_t i = 0; i < a->data.size(); i++) {

            if (!a->data[i].isApprox(b->data[i])) {

                equal = false;
                break;
            }
        }
    }
        
    return equal;
}

////////////////////////////////////////////////////////////////////////////////
void cluster_print(Cluster* c) {

    std::cout << "cluster: " << c << std::endl;
    std::cout << "centroid: " << std::endl << c->p0 << std::endl;

    std::cout << "data: " << std::endl;
    for (uint32_t i = 0; i < c->data.size(); i++)
        std::cout << c->data[i] << std::endl;
}

/// @file

