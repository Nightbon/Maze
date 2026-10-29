#include "cave_model.h"

#include <algorithm>
#include <fstream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

namespace s21 {

void CaveModel::initCave(int rows, int cols, double chance) {
  if (rows <= 0 || cols <= 0 || rows > 50 || cols > 50) {
    throw std::out_of_range("Неверные размеры пещеры (допустимо: 1-50).");
  }

  if (chance < 0.0 || chance > 1.0) {
    throw std::invalid_argument(
        "Вероятность должна быть в диапазоне [0.0, 1.0].");
  }

  rows_ = rows;
  cols_ = cols;
  cave_ = std::vector(rows_, std::vector<int>(cols_, 0));

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::bernoulli_distribution dist(chance);

  auto gen_cell = [&]() { return dist(gen); };
  std::ranges::generate(cave_ | std::views::join, gen_cell);
}

int CaveModel::countNeighbors(int row, int col) const {
  int count = 0;
  for (int dy : std::views::iota(-1, 2)) {
    for (int dx : std::views::iota(-1, 2)) {
      if (dy == 0 && dx == 0) continue;
      int ny = row + dy;
      int nx = col + dx;
      if (ny < 0 || ny >= rows_ || nx < 0 || nx >= cols_) {
        ++count;
      } else {
        count += cave_[ny][nx];
      }
    }
  }
  return count;
}

void CaveModel::generateStep(int birth_limit, int death_limit) {
  if (cave_.empty() || rows_ <= 0 || cols_ <= 0) return;

  auto new_cave = cave_;

  for (int i : std::views::iota(0, rows_)) {
    for (int j : std::views::iota(0, cols_)) {
      int neighbors = countNeighbors(i, j);
      if (cave_[i][j] == 0) {
        new_cave[i][j] = (neighbors > birth_limit) ? 1 : 0;
      } else {
        new_cave[i][j] = (neighbors < death_limit) ? 0 : 1;
      }
    }
  }

  cave_ = std::move(new_cave);
}

void CaveModel::loadCave(Path path) {
  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error("Файл не найден или не является файлом: " +
                             path.string());
  }

  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("Не удалось открыть файл пещеры.");
  }

  if (!(file >> rows_ >> cols_)) {
    throw std::invalid_argument(
        "Ошибка чтения размеров пещеры. Неверный формат.");
  }

  if (rows_ <= 0 || cols_ <= 0 || rows_ > 50 || cols_ > 50) {
    throw std::out_of_range("Неверные размеры пещеры (допустимо: 1-50).");
  }

  cave_ = std::vector(rows_, std::vector<int>(cols_, 0));

  for (auto& col : cave_ | std::views::join) {
    if (!(file >> col))
      throw std::invalid_argument("Ошибка чтения данных правых границ.");
    if (col != 0 && col != 1)
      throw std::invalid_argument(
          "Неверное значение клетки: допустимы только 0 и 1.");
  }
}

void CaveModel::saveCave(Path path) {
  if (rows_ <= 0 || cols_ <= 0 || rows_ > 50 || cols_ > 50) {
    throw std::out_of_range("Неверные размеры пещеры (допустимо: 1-50).");
  }

  std::ofstream file(path);
  if (!file) {
    throw std::runtime_error("Не удалось открыть файл пещеры.");
  }

  file << rows_ << " " << cols_ << "\n";

  for (size_t i : std::views::iota(0, rows_)) {
    for (size_t j : std::views::iota(0, cols_)) {
      file << cave_[i][j] << (j == cols_ - 1 ? "\n" : " ");
    }
  }
}

}  // namespace s21
