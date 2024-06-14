#ifndef TEXTURE_H
#define TEXTURE_H

#include <opencv2/opencv.hpp>
#include <GL/glew.h>

// GL texture z image file
GLuint textureInit(cv::String filepath, const bool mirror);
// GL texture z OpenCV image
GLuint tex_gen(cv::Mat& image);

#endif //TEXTURE_H
