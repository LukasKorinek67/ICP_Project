#include "RedColourTracker.h"
#include "App.h"

RedColourTracker::RedColourTracker() {}
RedColourTracker::~RedColourTracker() {}

bool RedColourTracker::check(cv::Mat frame) {
    //auto start = std::chrono::steady_clock::now();

    cv::Mat frame2;
    frame.copyTo(frame2);

    cv::Point2f center_normalized = find_center_normalized_hsv(frame);

    /*
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "sec" << std::endl;
    */

    draw_cross_normalized(frame2, center_normalized, 25);

    //cv::namedWindow("frame2");
    //cv::imshow("frame2", frame2);
    return tracking_red;
}

void RedColourTracker::draw_cross(cv::Mat& img, int x, int y, int size) {
    cv::Point p1(x - size / 2, y);
    cv::Point p2(x + size / 2, y);
    cv::Point p3(x, y - size / 2);
    cv::Point p4(x, y + size / 2);

    cv::line(img, p1, p2, CV_RGB(255, 0, 0), 3);
    cv::line(img, p3, p4, CV_RGB(255, 0, 0), 3);
}

cv::Point2f RedColourTracker::find_center_normalized(cv::Mat& frame) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    cv::Point2f center;
    cv::Moments m = cv::moments(gray, true);
    center.x = m.m10 / m.m00;
    center.y = m.m01 / m.m00;

    cv::Point2f center_normalized;
    center_normalized.x = center.x / frame.cols;
    center_normalized.y = center.y / frame.rows;

    //std::cout << "Center absolute: " << center << '\n';
    //std::cout << "Center normalized: " << center_normalized << '\n';

    return center_normalized;
}

cv::Point2f RedColourTracker::find_center_normalized_hsv(cv::Mat& frame) {
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::Mat mask;
    // CYAN
    //cv::inRange(hsv, cv::Scalar(80, 50, 50), cv::Scalar(100, 255, 255), mask);

    // GREEN
    //cv::inRange(hsv, cv::Scalar(80, 50, 50), cv::Scalar(100, 255, 255), mask);

    // YELLOW
    //cv::inRange(hsv, cv::Scalar(25, 150, 150), cv::Scalar(35, 255, 255), mask);

    // PINK
    //cv::inRange(hsv, cv::Scalar(145, 50, 50), cv::Scalar(165, 255, 255), mask);
    //std::cout << mask << "\n";

    cv::Mat mask1, mask2;
    // RED - první rozsah
    //cv::inRange(hsv, cv::Scalar(0, 200, 200), cv::Scalar(10, 255, 255), mask1);
    //// RED - druhý rozsah
    //cv::inRange(hsv, cv::Scalar(170, 200, 200), cv::Scalar(180, 255, 255), mask2);

    // RED - první rozsah
    cv::inRange(hsv, cv::Scalar(0, 180, 180), cv::Scalar(15, 255, 255), mask1);
    // RED - druhý rozsah
    cv::inRange(hsv, cv::Scalar(165, 150, 150), cv::Scalar(180, 255, 255), mask2);

    // Spojení masek
    mask = mask1 | mask2;

    cv::Point2f center;
    cv::Moments m = cv::moments(mask, true);
    center.x = m.m10 / m.m00;
    center.y = m.m01 / m.m00;

    if (m.m00 == 0) {
        tracking_red = false;
    }
    else {
        tracking_red = true;
    }

    cv::Point2f center_normalized;
    center_normalized.x = center.x / frame.cols;
    center_normalized.y = center.y / frame.rows;

    //std::cout << "Center absolute: " << center << '\n';
    //std::cout << "Center normalized: " << center_normalized << '\n';
    //std::cout << "Tracking red: " << tracking_red << '\n';

    return center_normalized;
}

void RedColourTracker::draw_cross_normalized(cv::Mat& img, cv::Point2f center_normalized, int size) {
    center_normalized.x = std::clamp(center_normalized.x, 0.0f, 1.0f);
    center_normalized.y = std::clamp(center_normalized.y, 0.0f, 1.0f);
    size = std::clamp(size, 1, std::min(img.cols, img.rows));

    cv::Point2f center_absolute(center_normalized.x * img.cols, center_normalized.y * img.rows);

    cv::Point2f p1(center_absolute.x - size / 2, center_absolute.y);
    cv::Point2f p2(center_absolute.x + size / 2, center_absolute.y);
    cv::Point2f p3(center_absolute.x, center_absolute.y - size / 2);
    cv::Point2f p4(center_absolute.x, center_absolute.y + size / 2);

    cv::line(img, p1, p2, CV_RGB(0, 0, 255), 3);
    cv::line(img, p3, p4, CV_RGB(0, 0, 255), 3);
}