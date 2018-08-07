#include <iostream>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <unistd.h>

#include <Graphics/Display.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/ShaderAttributes.hpp>
#include <Input/InputHandler.hpp>
#include "CameraSensor.hpp"
#include <Common/Timer.hpp>
#include <Surface/SurfaceReconstructor.hpp>
#include <Visualization/visualization.hpp>
#include "VoxelGrid.hpp"
#include "detection.hpp"

const int ALPHA_MAX = 100;
const int NOV_MAX = 100;
int alpha_slider_depth = 50;
int alpha_slider_ir    = 50;
int nov_slider_depth   = 20;
int nov_slider_ir      = 20;
float alpha_depth, alpha_ir, nov_depth, nov_ir;

////////////////////////////////////////////////////////////////////////////////
void on_trackbar_alpha_depth(int, void*) {

    alpha_depth = (float) alpha_slider_depth / ALPHA_MAX;
}

////////////////////////////////////////////////////////////////////////////////
void on_trackbar_alpha_ir(int, void*) {

    alpha_ir = (float) alpha_slider_ir / ALPHA_MAX;
}

////////////////////////////////////////////////////////////////////////////////
void on_trackbar_nov_depth(int, void*) {

    nov_depth = (float) nov_slider_depth / NOV_MAX;
}

////////////////////////////////////////////////////////////////////////////////
void on_trackbar_nov_ir(int, void*) {

    nov_ir = (float) nov_slider_ir / NOV_MAX;
}

////////////////////////////////////////////////////////////////////////////////
int detection_test_main() {

    const char* wname_nov_depth   = "Novelty depth";
    const char* wname_nov_ir      = "Novelty infrared";
    const char* wname_ir          = "Infrared";
    const char* wname_depth       = "Depth";
    cv::namedWindow(wname_ir, 0);
    cv::namedWindow(wname_depth, 0);
    cv::namedWindow(wname_nov_depth, 0);
    cv::namedWindow(wname_nov_ir, 0);

    char TrackbarName[50];
    sprintf(TrackbarName, "Alpha");
    cv::createTrackbar(TrackbarName, wname_depth, &alpha_slider_depth, ALPHA_MAX, on_trackbar_alpha_depth);
    cv::createTrackbar(TrackbarName, wname_ir, &alpha_slider_ir, ALPHA_MAX, on_trackbar_alpha_ir);

    sprintf(TrackbarName, "Novelty treshold");
    cv::createTrackbar(TrackbarName, wname_nov_depth, &nov_slider_depth, NOV_MAX, on_trackbar_nov_depth);
    cv::createTrackbar(TrackbarName, wname_nov_ir, &nov_slider_ir, NOV_MAX, on_trackbar_nov_ir);

    on_trackbar_alpha_depth(0, 0);
    on_trackbar_alpha_ir(0, 0);
    on_trackbar_nov_depth(0, 0);
    on_trackbar_nov_ir(0, 0);

    cv::Mat ir, depth, novelty_ir, novelty_depth;

    float* mean_depth = NULL;
    float* var_depth = NULL;
    float* mean_ir = NULL;
    float* var_ir = NULL;

    CameraSensor camera_sensor;
    float ir_scale = 0.001f;
    float depth_scale = camera_sensor.MeterScale() * 10.0f;
    std::cout << "depth scale: " << depth_scale << std::endl;

    while (true) {

        camera_sensor.CaptureDepth(&depth);
        camera_sensor.CaptureIr(&ir);

        detection_compute_intensity_pdf(&depth, &mean_depth, &var_depth, depth_scale, alpha_depth);
        detection_compute_intensity_pdf(&ir, &mean_ir, &var_ir, ir_scale, alpha_ir);

        if (novelty_ir.empty())
            novelty_ir = cv::Mat(cv::Size(ir.cols, ir.rows), CV_16UC1);

        if (novelty_depth.empty())
            novelty_depth = cv::Mat(cv::Size(depth.cols, depth.rows), CV_16UC1);

        detection_create_novelty_image(&depth, mean_depth, var_depth, depth_scale, nov_depth, &novelty_depth);
        detection_create_novelty_image(&ir, mean_ir, var_ir, ir_scale, nov_ir, &novelty_ir);

        cv::imshow(wname_nov_depth, novelty_depth);
        cv::imshow(wname_nov_ir, novelty_ir);
        cv::imshow(wname_depth, depth);
        cv::imshow(wname_ir, ir);
        cv::waitKey(50);
    }

    delete[] mean_depth;
    delete[] var_depth;
    delete[] mean_ir;
    delete[] var_ir;

    return 0;
}

/// @file

