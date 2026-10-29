#pragma once
#include <filesystem>
#include <queue>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace s21 {
struct Point {
  int x{};
  int y{};

  auto operator<=>(const Point&) const = default;
};

namespace fs = std::filesystem;
using Path = std::filesystem::path;

class MazeModel {
 public:
  void loadMaze(Path path);
  void loadMazeFromString(const std::string& content);
  void saveMaze(Path path);
  std::string saveMazeToString() const;
  void GenerateMaze(int rows, int cols);
  std::vector<Point> getPath(Point start, Point finish);
  auto& getRightBorders() const { return right_borders_; }
  auto& getBottomBorders() const { return bottom_borders_; }

 private:
  void genRightBorders(std::span<int> line, const int& i);
  void genBottomBorders(std::span<int> line, const int& i);
  void prepareNextLine(std::span<int> line, const int& i) const;
  void genLastLine(std::span<int> line);
  void parseMazeFromStream(std::istream& stream);
  void writeMazeToStream(std::ostream& stream) const;

  int cols_ = 0, rows_ = 0;
  std::vector<std::vector<int>> right_borders_, bottom_borders_;
};

}  // namespace s21