#pragma once
#ifndef REDCOLOURTRACKER_H
#define REDCOLOURTRACKER_H


// OpenCV
#include <opencv2/opencv.hpp>

// OpenGL Extension Wrangler
#include <GL/glew.h>
//#include <GL/wglew.h> //WGLEW = Windows GL Extension Wrangler (change for different platform)

// GLFW toolkit
#include <GLFW/glfw3.h>

// OpenGL math
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


class RedColourTracker {
public:
    RedColourTracker();
    ~RedColourTracker();
    bool check(cv::Mat frame);
private:
    bool tracking_red = false;
    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);
    cv::Point2f find_center_normalized(cv::Mat& frame);
    cv::Point2f find_center_normalized_hsv(cv::Mat& frame);
};

#endif //REDCOLOURTRACKER_H
