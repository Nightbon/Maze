#include "qlearning_model.h"

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace {
std::mt19937& getGen() {
  static std::mt19937 gen{std::random_device{}()};
  return gen;
}
}  // namespace

namespace s21 {

void QLearningModel::init() {
  rows_ = getRightBorders().size();
  cols_ = rows_ ? getRightBorders()[0].size() : 0;
  if (rows_ == 0 || cols_ == 0)
    throw std::runtime_error("Лабиринт не загружен.");

  q_table_ = std::vector(
      rows_, std::vector(cols_, std::array<double, kActionCount>{}));
  end_ = {-1, -1};
  trained_ = false;
}

void QLearningModel::loadMazeFromFile(const Path& path) {
  maze_.loadMaze(path);
  init();
}

void QLearningModel::loadMazeFromString(const std::string& content) {
  maze_.loadMazeFromString(content);
  init();
}

void QLearningModel::setEndPoint(Point endPoint) {
  if (rows_ == 0 || cols_ == 0)
    throw std::runtime_error("Лабиринт не загружен.");
  if (endPoint.x < 0 || endPoint.x >= cols_ || endPoint.y < 0 ||
      endPoint.y >= rows_) {
    throw std::out_of_range("Конечная точка вне границ лабиринта.");
  }
  end_ = endPoint;
  trained_ = false;

  for (auto& row : q_table_) {
    std::ranges::fill(row, std::array<double, kActionCount>{});
  }
}

double QLearningModel::getQValue(Point p, const Action& action) const noexcept {
  return q_table_[p.y][p.x][static_cast<int>(action)];
}

bool QLearningModel::canMove(Point p, const Action& action) const noexcept {
  switch (action) {
    case Action::kUp:
      return p.y > 0 && maze_.getBottomBorders()[p.y - 1][p.x] == 0;
    case Action::kRight:
      return p.x < cols_ - 1 && maze_.getRightBorders()[p.y][p.x] == 0;
    case Action::kDown:
      return p.y < rows_ - 1 && maze_.getBottomBorders()[p.y][p.x] == 0;
    case Action::kLeft:
      return p.x > 0 && maze_.getRightBorders()[p.y][p.x - 1] == 0;
    default:
      return false;
  }
}

Point QLearningModel::getNextState(Point p,
                                   const Action& action) const noexcept {
  if (action == Action::kRight) p.x += 1;
  if (action == Action::kLeft) p.x -= 1;
  if (action == Action::kDown) p.y += 1;
  if (action == Action::kUp) p.y -= 1;
  return p;
}

Action QLearningModel::chooseAction(Point p, double epsilon) {
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  if (dist(getGen()) < epsilon) {
    std::uniform_int_distribution<int> action_dist(0, kActionCount - 1);
    return static_cast<Action>(action_dist(getGen()));
  }

  return getBestAction(p);
}

Action QLearningModel::getBestAction(Point p) const noexcept {
  Action best_action = Action::kUp;
  double best_value = getQValue(p, Action::kUp);
  for (auto a : kAllActions) {
    if (double val = getQValue(p, a); val > best_value) {
      best_value = val;
      best_action = a;
    }
  }
  return best_action;
}

void QLearningModel::trainEpisodes(int episodes, double learning_rate,
                                   double epsilon) {
  if (rows_ == 0 || cols_ == 0)
    throw std::runtime_error("Лабиринт не загружен.");
  if (end_.x < 0 || end_.y < 0)
    throw std::runtime_error("Конечная точка не задана.");
  if (episodes <= 0) return;
  if (rows_ == 1 && cols_ == 1) {
    trained_ = true;
    return;
  }

  const int max_steps = rows_ * cols_ * kActionCount;
  constexpr double kDiscount = 0.99;
  constexpr double kWallPenalty = -5.0;
  constexpr double kStepPenalty = -1.0;
  constexpr double kGoalReward = 100.0;

  std::uniform_int_distribution<int> x_dist(0, cols_ - 1);
  std::uniform_int_distribution<int> y_dist(0, rows_ - 1);

  for (int _ : std::views::iota(0, episodes)) {
    Point curr;
    do {
      curr = {x_dist(getGen()), y_dist(getGen())};
    } while (curr == end_);

    for (int _ : std::views::iota(0, max_steps)) {
      const Action action = chooseAction(curr, epsilon);
      double reward = kStepPenalty;
      Point next = curr;

      if (canMove(curr, action)) {
        next = getNextState(curr, action);
        if (next == end_) reward = kGoalReward;
      } else {
        reward = kWallPenalty;
      }

      double max_next_q =
          *std::ranges::max_element(std::views::all(q_table_[next.y][next.x]));

      double current_q = q_table_[curr.y][curr.x][static_cast<int>(action)];
      q_table_[curr.y][curr.x][static_cast<int>(action)] +=
          learning_rate * (reward + kDiscount * max_next_q - current_q);

      curr = next;
      if (curr == end_) break;
    }
  }
  trained_ = true;
}

std::vector<Point> QLearningModel::getBestPath(Point start) const {
  if (rows_ == 0 || cols_ == 0)
    throw std::runtime_error("Лабиринт не загружен.");
  if (start.x < 0 || start.x >= cols_ || start.y < 0 || start.y >= rows_) {
    throw std::out_of_range("Стартовая точка вне границ лабиринта.");
  }
  if (!trained_) throw std::runtime_error("Агент не обучен.");
  if (start == end_) return {start};

  std::vector<Point> path{start};
  std::vector visited(rows_, std::vector<bool>(cols_, false));
  visited[start.y][start.x] = true;

  Point curr = start;
  const int max_steps = rows_ * cols_;

  for (int _ : std::views::iota(0, max_steps)) {
    Action best_action = getBestAction(curr);

    if (!canMove(curr, best_action)) break;

    curr = getNextState(curr, best_action);

    if (visited[curr.y][curr.x]) break;

    visited[curr.y][curr.x] = true;

    path.push_back(curr);

    if (curr == end_) break;
  }

  return path;
}

}  // namespace s21
