#include <fstream>
#include <iostream>
#include <iterator>
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
    std::vector<Point> path;
public:
    Plane(const std::vector<Point>& points) : points(points) {}
    Plane(int amount_points) {
        for(int i = 0; i < amount_points; i++) {
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
        std::vector<Point> new_points {points};
        Point bottommost = get_bottommost_point();
        path = {Point{0, bottommost.y}, bottommost};

        while(path.front() != path.back()) {

            auto second_last = *std::prev(path.end(), 2);
            auto last = *std::prev(path.end());
            auto max_angle_element = std::max_element(new_points.begin(), new_points.end(), [&second_last, &last](const Point& p1, const Point& p2) {
                float a1 = find_angle(second_last, last, p1);
                float a2 = find_angle(second_last, last, p2);
                std::cout << "second_last: " << second_last << std::endl;
                std::cout << "last: " << last << std::endl;
                std::cout << "p1: " << p1 << std::endl;
                std::cout << "a1: " << a1 << std::endl;
                std::cout << "a2: " << a2 << std::endl;
                return a1 < a2;
            });

            path.push_back(*max_angle_element);
            new_points.erase(max_angle_element);
            // remove faked first point
            if(path.begin()->x == 0) { 
                path.erase(path.begin());
            }
            
        }  
    }
};
 
const std::vector<Point> ps {{100, 50}, 
                            {200, 100},
                            {200, 200},
                            {100, 200}};
//  const std::vector<Point> ps {{111, 111}, 
//                             {200, 90},
//                             {31, 200},
//                             {50, 80},
//                             {100, 100}, 
//                             {213,52},
//                             {113,223}};
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
    auto points = plane.get_points();
    auto path = plane.get_path();
    
    while(!window.ShouldClose()) {
        BeginDrawing();
        raylib::DrawText("Punktów: " + std::to_string(points.size()), 20, 20, 22, raylib::Color::Beige());
        for(auto& p: points) {
            DrawCircle(p.x, p.y, 2.f, raylib::Color::White());
        }
        for(auto it = path.begin(); it != path.end();) {
            ++it;
            if(it != path.end()) {
                auto prev = std::prev(it);
                DrawLine(prev->x, prev->y, it->x, it->y, raylib::Color::Gray());

            }
        }

        window.ClearBackground(raylib::Color::Black());
        EndDrawing();
    }
    // auto fp = load_from_file("points.txt");
    // Plane pl{fp};
    // auto path = pl.get_path();
    // for(auto& p: path) {
    //     std::cout << p << std::endl;
    // }
    raylib::Vector2 a;
    return 0;
}