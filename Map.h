#pragma once
#include <string>
#include <vector>
#include <utility>
#include <tuple>

class Map {
private:
    std::string map_path;
public:
    std::vector<std::pair<int, int>> positions;
    std::vector<std::tuple<int, int, int>> colors;
    std::pair<int, int> start_position = { 0, 0 };
    std::pair<int, int> dimensions = { 0, 0 };
    Map(std::string map_path);
    bool load();
};
