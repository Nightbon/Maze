#include "maze_model.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

bool getRandom() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::bernoulli_distribution dist(0.5);
  return dist(gen);
}

void assignUniqueSets(std::span<int> line, int& cols, int& set_counter) {
  for (int j : std::views::iota(0, cols)) {
    if (line[j] == 0) {
      line[j] = set_counter++;
    }
  }
}

}  // namespace

namespace s21 {

void MazeModel::writeMazeToStream(std::ostream& stream) const {
  if (rows_ <= 0 || cols_ <= 0 || rows_ > 50 || cols_ > 50) {
    throw std::out_of_range("Неверные размеры лабиринта (допустимо: 1-50).");
  }

  stream << rows_ << " " << cols_ << "\n";

  for (size_t i : std::views::iota(0, rows_)) {
    for (size_t j : std::views::iota(0, cols_)) {
      stream << right_borders_[i][j] << (j == cols_ - 1 ? "\n" : " ");
    }
  }

  stream << "\n";

  for (size_t i : std::views::iota(0, rows_)) {
    for (size_t j : std::views::iota(0, cols_)) {
      stream << bottom_borders_[i][j] << (j == cols_ - 1 ? "\n" : " ");
    }
  }
}

void MazeModel::parseMazeFromStream(std::istream& stream) {
  if (!(stream >> rows_ >> cols_)) {
    throw std::invalid_argument(
        "Ошибка чтения размеров лабиринта. Неверный формат.");
  }

  if (rows_ <= 0 || cols_ <= 0 || rows_ > 50 || cols_ > 50) {
    throw std::out_of_range("Неверные размеры лабиринта (допустимо: 1-50).");
  }

  right_borders_ = std::vector(rows_, std::vector<int>(cols_, 0));
  for (auto& col : right_borders_ | std::views::join) {
    if (!(stream >> col))
      throw std::invalid_argument("Ошибка чтения данных правых границ.");
    if (col != 0 && col != 1)
      throw std::invalid_argument(
          "Неверное значение клетки: допустимы только 0 и 1.");
  }

  bottom_borders_ = std::vector(rows_, std::vector<int>(cols_, 0));

  stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  for (auto& col : bottom_borders_ | std::views::join) {
    if (!(stream >> col))
      throw std::invalid_argument("Ошибка чтения данных нижних границ.");
    if (col != 0 && col != 1)
      throw std::invalid_argument(
          "Неверное значение клетки: допустимы только 0 и 1.");
  }
}

void MazeModel::loadMaze(Path path) {
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error("Файл не найден или не является файлом: " +
                             path.string());
  }

  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Не удалось открыть файл лабиринта.");
  }

  parseMazeFromStream(file);
}

void MazeModel::loadMazeFromString(const std::string& content) {
  if (content.empty()) {
    throw std::runtime_error("файл пустой");
  }
  std::stringstream string_stream(content);
  parseMazeFromStream(string_stream);
}

void MazeModel::saveMaze(Path path) {
  std::ofstream file(path);
  if (!file) {
    throw std::runtime_error("Не удалось открыть файл лабиринта.");
  }

  writeMazeToStream(file);
}

std::string MazeModel::saveMazeToString() const {
  std::ostringstream string_stream;

  writeMazeToStream(string_stream);

  return string_stream.str();
}

void MazeModel::GenerateMaze(int rows, int cols) {
  if (rows <= 0 || cols <= 0 || rows > 50 || cols > 50) {
    throw std::out_of_range("Неверные размеры лабиринта (допустимо: 1-50).");
  }

  rows_ = rows;
  cols_ = cols;
  right_borders_ = std::vector(rows_, std::vector<int>(cols_, 0));
  bottom_borders_ = std::vector(rows_, std::vector<int>(cols_, 0));
  std::vector<int> line(cols_, 0);
  int set_counter = 1;

  for (int i : std::views::iota(0, rows_ - 1)) {
    assignUniqueSets(line, cols_, set_counter);
    genRightBorders(line, i);
    genBottomBorders(line, i);
    prepareNextLine(line, i);
  }
  assignUniqueSets(line, cols_, set_counter);
  genLastLine(line);
}

void MazeModel::genRightBorders(std::span<int> line, const int& i) {
  for (int j : std::views::iota(0, cols_ - 1)) {
    bool put_wall = getRandom();

    // Если ячейки в одном множестве, стена обязательна
    if (put_wall || line[j] == line[j + 1]) {
      right_borders_[i][j] = 1;
    } else {
      // Объединяем множества
      int old_set = line[j + 1];
      int new_set = line[j];
      std::ranges::replace(line, old_set, new_set);
    }
  }
  right_borders_[i][cols_ - 1] = 1;  // Крайняя правая стена
}

void MazeModel::genBottomBorders(std::span<int> line, const int& i) {
  for (int j : std::views::iota(0, cols_)) {
    bool put_wall = getRandom();

    // Считаем, сколько ячеек этого множества в текущей строке
    auto same_set_count = std::ranges::count(line, line[j]);

    // Нельзя ставить стену, если это единственная ячейка множества
    if (put_wall && same_set_count > 1) {
      bottom_borders_[i][j] = 1;
    }
  }

  // Проверка: у каждого множества должен быть минимум один проход вниз
  for (int j : std::views::iota(0, cols_)) {
    int current_set = line[j];

    // Проверяем, есть ли хоть один выход вниз для этого множества
    bool has_passage =
        std::ranges::any_of(std::views::iota(0, cols_), [&](int k) {
          return line[k] == current_set && bottom_borders_[i][k] == 0;
        });

    // Если выходов нет, принудительно открываем текущую ячейку вниз
    if (!has_passage) {
      bottom_borders_[i][j] = 0;
    }
  }
}

void MazeModel::prepareNextLine(std::span<int> line, const int& i) const {
  for (int j : std::views::iota(0, cols_)) {
    if (bottom_borders_[i][j] == 1) {
      line[j] = 0;  // Обнуляем ячейку со стеной сверху
    }
  }
}

void MazeModel::genLastLine(std::span<int> line) {
  for (int j : std::views::iota(0, cols_ - 1)) {
    // Если ячейки в разных множествах, убираем стену и объединяем их
    if (line[j] != line[j + 1]) {
      right_borders_[rows_ - 1][j] = 0;
      int old_set = line[j + 1];
      int new_set = line[j];
      std::ranges::replace(line, old_set, new_set);
    } else {
      right_borders_[rows_ - 1][j] = 1;
    }
    // Нижняя граница лабиринта всегда закрыта
    bottom_borders_[rows_ - 1][j] = 1;
  }

  right_borders_[rows_ - 1][cols_ - 1] = 1;
  bottom_borders_[rows_ - 1][cols_ - 1] = 1;
}

std::vector<Point> MazeModel::getPath(Point start, Point finish) {
  if (start.x < 0 || start.y < 0 || start.x >= cols_ || start.y >= rows_ ||
      finish.x < 0 || finish.y < 0 || finish.x >= cols_ || finish.y >= rows_) {
    throw std::out_of_range("Неверные координаты старта или финиша");
  }

  if (start == finish) {
    return {start};
  }

  constexpr Point undefined{-1, -1};
  std::vector tmp_matrix(rows_, std::vector<Point>(cols_, undefined));
  tmp_matrix[start.y][start.x] = start;

  std::queue<Point> queue;
  queue.push(start);

  bool path_found = false;

  while (!queue.empty()) {
    const Point current = queue.front();
    queue.pop();

    if (current == finish) {
      path_found = true;
      break;
    }

    const auto [x, y] = current;

    // Вправо
    if (x < cols_ - 1 && !right_borders_[y][x] &&
        tmp_matrix[y][x + 1] == undefined) {
      tmp_matrix[y][x + 1] = current;
      queue.push({x + 1, y});
    }
    // Влево
    if (x > 0 && !right_borders_[y][x - 1] &&
        tmp_matrix[y][x - 1] == undefined) {
      tmp_matrix[y][x - 1] = current;
      queue.push({x - 1, y});
    }
    // Вверх
    if (y > 0 && !bottom_borders_[y - 1][x] &&
        tmp_matrix[y - 1][x] == undefined) {
      tmp_matrix[y - 1][x] = current;
      queue.push({x, y - 1});
    }
    // Вниз
    if (y < rows_ - 1 && !bottom_borders_[y][x] &&
        tmp_matrix[y + 1][x] == undefined) {
      tmp_matrix[y + 1][x] = current;
      queue.push({x, y + 1});
    }
  }

  std::vector<Point> path = {};
  if (!path_found) {
    return path;
  }

  Point curr = finish;
  while (curr != start) {
    path.push_back(curr);
    curr = tmp_matrix[curr.y][curr.x];
  }
  path.push_back(start);

  std::ranges::reverse(path);

  return path;
}

}  // namespace s21