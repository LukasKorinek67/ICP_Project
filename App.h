
#ifndef APP_H
#define APP_H

// OpenCV
#include <opencv2/opencv.hpp>
#include "synced_dequeue.h"
#include "Game.h"
#include "RedColourTracker.h"

class App {
public:
    App();
    ~App();
    bool init(void);
    int run(void);
    bool end;
private:
    cv::VideoCapture capture;
    synced_deque<bool> queue;
    bool red_tracker_on = true;
    //bool red_tracker_on = false;
    bool tracking_red = false;
    void camera_thread_code(void);
    Game game;
    RedColourTracker red_colour_tracker;
};

#endif //APP_H

