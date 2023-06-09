#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <limits>
#include <filesystem>

#include "raylib-cpp.hpp"

const uint32_t SCREEN_WIDTH = 800;
const uint32_t SCREEN_HEIGHT = 600;
const uint32_t UI_FONT_SIZE = 18;
const raylib::Color UI_FONT_COLOR = raylib::Color::LightGray();


using std::chrono::system_clock;
std::default_random_engine generator(system_clock::now().time_since_epoch().count());


struct Point {
    int x, y;
    Point(const int x = 0, const int y = 0) : x(x), y(y) {}

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
    Point v0 = { p0.x - p1.x, p0.y - p1.y }; 
    Point v1 = { p2.x - p1.x, p2.y - p1.y }; 

    // Calculate the dot product
    float dotProduct = v0.x * v1.x + v0.y * v1.y;

    // Calculate the cross product magnitude
    float crossProductMagnitude = v0.x * v1.y - v0.y * v1.x;

    // Calculate the angle between the vectors
    float angle = atan2(crossProductMagnitude, dotProduct);

    // Convert the angle to degrees, maybe part under is not needed
    angle = angle * 180.0 / M_PI;

    //Ensure the angle is within [0, 360)
    while (angle < 0.0) {
        angle += 360.0;
    }
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    return angle;
}

std::pair<int, int> split_string(const std::string& str, char delimiter) {
    std::string token1, token2;
    std::istringstream iss(str);

    std::getline(iss, token1, delimiter);
    std::getline(iss, token2, delimiter);
    return std::make_pair(std::stoi(token1), std::stoi(token2));
}


class Plane {
    std::vector<Point> points;
    std::vector<Point> path;
public:
    Plane(const std::vector<Point>& points) : points(points) {}
    Plane(uint32_t points_amount) : points(points_amount, Point{}) {
        randomize_points();
    }
    Plane(const std::filesystem::path& filename) {
        if (!std::filesystem::exists(filename)) {
            throw std::runtime_error("File not found: " + filename.string() + ". Check if file is in proper directory.");
        }
        std::ifstream f(filename);
        std::string line;
        while (std::getline(f, line)) {
            auto [x, y] = split_string(line, ' ');
            std::cout << Point{x, y} << std::endl;
            points.push_back(Point{x, y});
        }
        
    }

    void randomize_points() {
        for(auto& p : points) {
            // 20 to SIZE-20, to center it more
            std::uniform_int_distribution<int> x_distribution(20, SCREEN_WIDTH - 20);
            std::uniform_int_distribution<int> y_distribution(20, SCREEN_HEIGHT - 20);
            p.x = x_distribution(generator);
            p.y = y_distribution(generator);
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
        auto second_last = Point{-200, get_bottommost_point().y};
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
        std::cout  << std::endl;
                
    }

    void draw_points() {
        for(const auto& p: points) {
            DrawCircleV(p.asVec(), 2.f, raylib::Color::White());
        }
    }
    void draw_path() {
        for(auto it = path.begin(); it != std::prev(path.end());) {
            auto prev = it;
            ++it;
            DrawLineEx(prev->asVec(), it->asVec(), 1.f, raylib::Color::Gray());
        }
    }

};


void draw_info(Plane &p) {
    raylib::DrawText("Number of points: " + std::to_string(p.get_points().size()), 20, 560, UI_FONT_SIZE, UI_FONT_COLOR);
    raylib::DrawText("[Esc] Quit", 20, 20, UI_FONT_SIZE, UI_FONT_COLOR);
    raylib::DrawText("[Spacebar] Randomize", 120, 20, UI_FONT_SIZE, UI_FONT_COLOR);
}

int main() {
    raylib::Window window(SCREEN_WIDTH, SCREEN_HEIGHT, "Jarvis - Convex Hull");

    Plane plane (200);
    plane.make_path();


    while(!window.ShouldClose()) {
        if(IsKeyReleased(KeyboardKey::KEY_SPACE)) {
            plane.randomize_points();
            plane.make_path();
        }


        BeginDrawing();
        {
            plane.draw_points();
            plane.draw_path();
            draw_info(plane);

            window.ClearBackground(raylib::Color::Black());
        }
        EndDrawing();
    }


    return 0;
}