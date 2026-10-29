#pragma once

#include <filesystem>
#include <random>
#include <vector>

namespace s21 {

namespace fs = std::filesystem;
using Path = std::filesystem::path;

class CaveModel {
 public:
  void initCave(int rows, int cols, double chance);
  void generateStep(int birth_limit, int death_limit);
  void loadCave(Path path);
  void saveCave(Path path);

  auto& getCave() { return cave_; }

 private:
  int countNeighbors(int row, int col) const;

  int rows_ = 0;
  int cols_ = 0;
  std::vector<std::vector<int>> cave_;
};

}  // namespace s21
