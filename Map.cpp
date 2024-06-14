#include "Map.h"
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>

Map::Map(std::string map_path) :map_path(map_path) {}

bool Map::load() {
    cv::Mat level = cv::imread(map_path, 1);
    if (level.empty()) {
        std::cout << "!!! Failed imread(): image not found" << std::endl;
        return false;
    }

    dimensions = { level.rows,level.cols };
    for (int row = 0; row < level.rows; row++) {
        for (int col = 0; col < level.cols; col++) {
            int blue = (int)(level.Mat::at<cv::Vec3b>(row, col)[0]);
            int green = (int)(level.Mat::at<cv::Vec3b>(row, col)[1]);
            int red = (int)(level.Mat::at<cv::Vec3b>(row, col)[2]);
            if (blue == 255 && green == 255 && red == 255) {
                continue;
            }
            // vynechat ohraničení trati
            if (blue == 0 && green == 0 && red == 0) {
                continue;
            }
            // vynechat cílovou čáru
            if (blue == 0 && green == 255 && red == 0) {
                continue;
            }
            if (blue == 0 && green == 0 && red == 255) {
                std::cout << "Spawn position at " << row << " " << col << "\n";
                start_position = {row,col};
            }
            else {
                positions.push_back(std::make_pair(row, col));
                //positions.push_back(std::make_pair(col, row));
                colors.push_back(std::make_tuple(red, green, blue));
            }
        }
    }
    return true;
}
