#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <sstream>
#include "Color.hpp"
#include "Functions.hpp"
#include "Vector2.hpp"
#include "raylib-cpp.hpp"
#include "raylib.h"

 
const uint32_t HEIGHT = 600;
const uint32_t WIDTH = 800;
 
using std::chrono::system_clock;
std::default_random_engine generator(system_clock::now().time_since_epoch().count());






struct Point {
    int x, y;
    Point(const int x, const int y) : x(x), y(y) {}
    Point() {
        std::uniform_int_distribution<int> x_distribution(0, WIDTH);
        std::uniform_int_distribution<int> y_distribution(0, HEIGHT);
        x = x_distribution(generator);
        y = y_distribution(generator);
    }


    bool operator==(const Point& p) const {
        return (x == p.x && y == p.y);
    }
    bool operator!=(const Point& p) const {
        return (x != p.x || y != p.y);
    }

    friend std::ostream & operator<<(std::ostream &os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }

    // for better usage in raylib
    const raylib::Vector2 asVec() const {
        return raylib::Vector2{static_cast<float>(x), static_cast<float>(y)};
    }
};
 
float find_angle(const Point& p0, const Point& p1, const Point& p2) {
    float b = std::pow(abs(p1.x - p0.x), 2) + std::pow(abs(p1.y - p0.y), 2);
    float a = std::pow(abs(p1.x - p2.x), 2) + std::pow(abs(p1.y - p2.y), 2);
    float c = std::pow(abs(p2.x - p0.x), 2) + std::pow(abs(p2.y - p0.y), 2);

    float denominator = std::sqrt(4 * a * b);
    if (denominator == 0.f) {
        // Handle collinear points (special case)
        return 0.0f;
    }

    float angle = (a + b - c) / denominator;
    return std::acos(angle);
}


class Plane {
    std::vector<Point> points;
    uint32_t points_amount;
    std::vector<Point> path;
public:
    Plane(const std::vector<Point>& points) : points(points) {}
    Plane(uint32_t points_amount) : points_amount(points_amount) {
        for(uint32_t i = 0; i < points_amount; i++) {
            points.push_back(Point{});
        }
    }

    void randomize_points() {
        points.clear();
        for(uint32_t i = 0; i < points_amount; i++) {
            points.push_back(Point{});
        }
    }

    Point get_bottommost_point() const {
        auto it = std::max_element(points.begin(), points.end(),  [](const Point& a, const Point& b) {
            return a.y < b.y;
        });
        return *it;
    }
    std::vector<Point>& get_points() {
        return points;
    }

    std::vector<Point>& get_path() {
        return path;
    }
    void make_path() {
        path.clear();
        std::vector<Point> remaining_points {points};
        path.push_back(get_bottommost_point());

        // create first fake point as reference for calculating angle 
        auto second_last = Point{std::numeric_limits<int>::min(), get_bottommost_point().y};
        auto last = path.back();

        while(path.front() != path.back() || path.size() == 1) {
            auto max_angle_element = std::max_element(remaining_points.begin(), remaining_points.end(), [&second_last, &last](const Point& p1, const Point& p2) {
                float a1 = find_angle(second_last, last, p1);
                float a2 = find_angle(second_last, last, p2);
                return a1 < a2;
            });

            path.push_back(*max_angle_element);
            remaining_points.erase(max_angle_element);
                      
            second_last = *std::prev(path.end(), 2);
            last = *std::prev(path.end());
        }  
    }

    void draw_points() {
        for(const auto& p: points) {
            DrawCircleV(p.asVec(), 2.f, raylib::Color::White());
        }
    }
    void draw_path() {
        for(auto it = path.begin(); it != std::prev(path.end());) {
            ++it;
            auto prev = std::prev(it);
            DrawLineEx(prev->asVec(), it->asVec(), 1.f, raylib::Color::Gray());
        }
    }

};
 

std::pair<int, int> split(const std::string& str, char delimiter) {
    std::string token1, token2;
    std::istringstream iss(str);

    std::getline(iss, token1, delimiter);
    std::getline(iss, token2, delimiter);

    return std::make_pair(std::stoi(token1), std::stoi(token2));
}

std::vector<Point> load_from_file(const std::string& filename) {
    std::vector<Point> points;
    std::ifstream f(filename);
    std::string line;
    while (std::getline(f, line)) {
        auto [x, y] = split(line, ' ');
        std::cout << Point{x, y} << std::endl;
        points.push_back(Point{x, y});
    }
    return points;
}


int main() {
    raylib::Window window(WIDTH, HEIGHT, "Jarvis - Convex Hull");
    SetTargetFPS(60);
    
    Plane plane {16};
    plane.make_path();
    

    while(!window.ShouldClose()) {
        if(IsKeyReleased(KEY_F5)) {

            plane.randomize_points();
            plane.make_path();
        }


        BeginDrawing();
        {
            raylib::DrawText("Punktów: " + std::to_string(plane.get_points().size()), 20, 20, 22, raylib::Color::Beige());
            plane.draw_points();
            plane.draw_path();

            window.ClearBackground(raylib::Color::Black());
        }
        EndDrawing();
    }


    return 0;
}