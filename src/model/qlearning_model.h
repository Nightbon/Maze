#pragma once

#include <array>
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include "maze_model.h"

namespace s21 {

namespace fs = std::filesystem;
using Path = fs::path;

enum class Action { kUp = 0, kRight = 1, kDown = 2, kLeft = 3 };

class QLearningModel {
 public:
  static constexpr int kActionCount = 4;
  static constexpr std::array<Action, kActionCount> kAllActions = {
      Action::kUp, Action::kRight, Action::kDown, Action::kLeft};

  void loadMazeFromFile(const Path& path);
  void loadMazeFromString(const std::string& content);
  void setEndPoint(Point endPoint);
  void trainEpisodes(int episodes, double learning_rate, double epsilon);
  std::vector<Point> getBestPath(Point start) const;

  auto& getRightBorders() const { return maze_.getRightBorders(); }
  auto& getBottomBorders() const { return maze_.getBottomBorders(); }
  Point getEndPoint() const { return end_; }
  bool isTrained() const { return trained_; }

 private:
  double getQValue(Point p, const Action& action) const noexcept;
  bool canMove(Point p, const Action& action) const noexcept;
  Point getNextState(Point p, const Action& action) const noexcept;
  Action getBestAction(Point p) const noexcept;
  Action chooseAction(Point p, double epsilon);
  void init();

  MazeModel maze_;
  Point end_{-1, -1};

  size_t rows_ = 0;
  size_t cols_ = 0;
  bool trained_ = false;

  std::vector<std::vector<std::array<double, kActionCount>>> q_table_;
};

}  // namespace s21
