#include "App.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>
#include "Game.h"


App::App() {
	// default constructor
	// nothing to do here (so far...)
	//std::cout << "Constructed...\n";
}

bool App::init() {
	try {
		if (red_tracker_on) {
			// TODO - toto pro Windows
			capture = cv::VideoCapture(cv::CAP_DSHOW);
			// TODO - toto pro macOS (AVFoundation)
			//capture = cv::VideoCapture(0, cv::CAP_AVFOUNDATION);
			//capture.open(0);

			if (!capture.isOpened()) {
				//std::cerr << "no camera source? Fallback to video..." << std::endl;
				std::cout << "no camera source?" << std::endl;
				return false;
			}
			else {
				std::cout << "Source: " <<
					": width=" << capture.get(cv::CAP_PROP_FRAME_WIDTH) <<
					", height=" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << '\n';
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "Init failed : " << e.what() << std::endl;
		throw;
	}
	return true;
}

int App::run(void) {
	try {
		if (!game.init()) {
			throw std::runtime_error("Failed to init Game.");
		}
		end = false;
		std::thread camera_thread(&App::camera_thread_code, this);
		game.run(queue, red_tracker_on);
		end = true;
		camera_thread.join();
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}

void App::camera_thread_code(void) {
	cv::Mat frame;
	try {
		if (red_tracker_on) {
			while (!end) {
				capture >> frame;

				if (frame.empty()) {
					//throw std::exception("Empty file? Wrong path?");
					throw std::runtime_error("Empty file? Wrong path?");
				}
				// Zobrazit snímaný obraz
				//cv::imshow("Camera Frame", frame);
				//std::cout << "HERE !\n";
				bool red = red_colour_tracker.check(frame);
				//std::cout << "red: " << red << "\n";
				if (!tracking_red && red) {
					queue.push_back(red);
					tracking_red = true;
					std::cout << "Semafor changed - red tracked!\n";
				}
				else if (tracking_red && !red) {
					queue.push_back(red);
					tracking_red = false;
					std::cout << "Semafor changed - red is not visible\n";
				}
				int key = cv::waitKey(1);
				if (key == 27) {
					end = true;
					break;
				}
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
	}
}

App::~App() {
	if (capture.isOpened()) {
		capture.release();
	}
	cv::destroyAllWindows();
	std::cout << "Bye...\n";
}