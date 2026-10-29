#include "../maze_model.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

void writeTempFile(const std::filesystem::path& path,
                   std::string_view content) {
  std::ofstream f(path);
  f << content;
}

constexpr const char* kSimpleMaze =
    "2 2\n"
    "0 1\n"
    "0 1\n"
    "\n"
    "0 1\n"
    "1 1\n";

constexpr const char* kOpen3x3 =
    "3 3\n"
    "0 0 1\n"
    "0 0 1\n"
    "0 0 1\n"
    "\n"
    "0 0 0\n"
    "0 0 0\n"
    "1 1 1\n";

constexpr const char* k1x1 =
    "1 1\n"
    "1\n"
    "\n"
    "1\n";

}  // namespace

class GenerateMazeTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
};

TEST_F(GenerateMazeTest, ValidSmallMazeProducesCorrectDimensions) {
  model_.GenerateMaze(5, 7);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();
  EXPECT_EQ(rb.size(), 5u);
  EXPECT_EQ(bb.size(), 5u);
  for (const auto& row : rb) EXPECT_EQ(row.size(), 7u);
  for (const auto& row : bb) EXPECT_EQ(row.size(), 7u);
}

TEST_F(GenerateMazeTest, MinimumSize1x1) {
  model_.GenerateMaze(1, 1);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();
  EXPECT_EQ(rb.size(), 1u);
  EXPECT_EQ(bb.size(), 1u);

  EXPECT_EQ(rb[0][0], 1);
  EXPECT_EQ(bb[0][0], 1);
}

TEST_F(GenerateMazeTest, MaximumSize50x50) {
  model_.GenerateMaze(50, 50);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();
  EXPECT_EQ(rb.size(), 50u);
  EXPECT_EQ(bb.size(), 50u);
}

TEST_F(GenerateMazeTest, ThrowsOnZeroRows) {
  EXPECT_THROW(model_.GenerateMaze(0, 10), std::out_of_range);
}

TEST_F(GenerateMazeTest, ThrowsOnZeroCols) {
  EXPECT_THROW(model_.GenerateMaze(10, 0), std::out_of_range);
}

TEST_F(GenerateMazeTest, ThrowsOnNegativeRows) {
  EXPECT_THROW(model_.GenerateMaze(-1, 5), std::out_of_range);
}

TEST_F(GenerateMazeTest, ThrowsOnNegativeCols) {
  EXPECT_THROW(model_.GenerateMaze(5, -1), std::out_of_range);
}

TEST_F(GenerateMazeTest, ThrowsOnRowsExceed50) {
  EXPECT_THROW(model_.GenerateMaze(51, 10), std::out_of_range);
}

TEST_F(GenerateMazeTest, ThrowsOnColsExceed50) {
  EXPECT_THROW(model_.GenerateMaze(10, 51), std::out_of_range);
}

TEST_F(GenerateMazeTest, RightBordersAreBinary) {
  model_.GenerateMaze(10, 10);
  for (const auto& row : model_.getRightBorders())
    for (int val : row) EXPECT_TRUE(val == 0 || val == 1);
}

TEST_F(GenerateMazeTest, BottomBordersAreBinary) {
  model_.GenerateMaze(10, 10);
  for (const auto& row : model_.getBottomBorders())
    for (int val : row) EXPECT_TRUE(val == 0 || val == 1);
}

TEST_F(GenerateMazeTest, RightmostColumnAlwaysHasRightWall) {
  model_.GenerateMaze(5, 8);
  const auto& rb = model_.getRightBorders();
  for (const auto& row : rb)
    EXPECT_EQ(row.back(), 1) << "Right border of last column must always be 1";
}

TEST_F(GenerateMazeTest, LastRowAlwaysHasBottomWall) {
  model_.GenerateMaze(8, 5);
  const auto& bb = model_.getBottomBorders();
  const auto& lastRow = bb.back();
  for (int val : lastRow)
    EXPECT_EQ(val, 1) << "Bottom border of last row must always be 1";
}

TEST_F(GenerateMazeTest, PerfectMaze_AllCellsReachableFromTopLeft) {
  model_.GenerateMaze(10, 10);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();

  for (int r = 0; r < 10; ++r) {
    for (int c = 0; c < 10; ++c) {
      auto path = model_.getPath({0, 0}, {c, r});
      EXPECT_FALSE(path.empty())
          << "Path must exist from (0,0) to (" << c << "," << r << ")";
    }
  }
}

TEST_F(GenerateMazeTest, PerfectMaze_AllCellsReachable_Pairs) {
  model_.GenerateMaze(7, 7);
  s21::Point const corners[] = {{0, 0}, {6, 0}, {0, 6}, {6, 6}};
  for (const auto& a : corners)
    for (const auto& b : corners) EXPECT_FALSE(model_.getPath(a, b).empty());
}

class LoadMazeTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "maze_test";
  std::filesystem::path tmp_file_ = tmp_dir_ / "test_maze.txt";

  void SetUp() override {
    std::filesystem::create_directories(tmp_dir_);
    writeTempFile(tmp_file_, kSimpleMaze);
  }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(LoadMazeTest, LoadsValidFileCorrectly) {
  model_.loadMaze(tmp_file_);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();

  EXPECT_EQ(rb.size(), 2u);
  EXPECT_EQ(bb.size(), 2u);

  EXPECT_EQ(rb[0][0], 0);
  EXPECT_EQ(rb[0][1], 1);
  EXPECT_EQ(rb[1][0], 0);
  EXPECT_EQ(rb[1][1], 1);

  EXPECT_EQ(bb[0][0], 0);
  EXPECT_EQ(bb[0][1], 1);
  EXPECT_EQ(bb[1][0], 1);
  EXPECT_EQ(bb[1][1], 1);
}

TEST_F(LoadMazeTest, ThrowsOnNonExistentFile) {
  EXPECT_THROW(model_.loadMaze(tmp_dir_ / "nonexistent.txt"),
               std::runtime_error);
}

TEST_F(LoadMazeTest, ThrowsOnEmptyFile) {
  auto empty = tmp_dir_ / "empty.txt";
  writeTempFile(empty, "");
  EXPECT_THROW(model_.loadMaze(empty), std::invalid_argument);
}

TEST_F(LoadMazeTest, ThrowsOnInvalidDimensionsZero) {
  auto f = tmp_dir_ / "zero_dim.txt";
  writeTempFile(f, "0 5\n");
  EXPECT_THROW(model_.loadMaze(f), std::out_of_range);
}

TEST_F(LoadMazeTest, ThrowsOnDimensionsExceed50) {
  auto f = tmp_dir_ / "big_dim.txt";
  writeTempFile(f, "51 5\n");
  EXPECT_THROW(model_.loadMaze(f), std::out_of_range);
}

TEST_F(LoadMazeTest, ThrowsOnNegativeDimensions) {
  auto f = tmp_dir_ / "neg_dim.txt";
  writeTempFile(f, "-3 5\n");
  EXPECT_THROW(model_.loadMaze(f), std::out_of_range);
}

TEST_F(LoadMazeTest, ThrowsOnInsufficientBorderData) {
  auto f = tmp_dir_ / "truncated.txt";
  writeTempFile(f, "3 3\n0 0 1\n0 0\n");
  EXPECT_THROW(model_.loadMaze(f), std::invalid_argument);
}

TEST_F(LoadMazeTest, Loads1x1Maze) {
  auto f = tmp_dir_ / "1x1.txt";
  writeTempFile(f, k1x1);
  model_.loadMaze(f);
  EXPECT_EQ(model_.getRightBorders().size(), 1u);
  EXPECT_EQ(model_.getRightBorders()[0][0], 1);
  EXPECT_EQ(model_.getBottomBorders()[0][0], 1);
}

class SaveMazeTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "maze_save_test";
  std::filesystem::path tmp_file_ = tmp_dir_ / "path_maze.txt";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(SaveMazeTest, ThrowsOnNonExistentFile) {
  EXPECT_THROW(model_.saveMaze(tmp_dir_ / "nope.txt"), std::out_of_range);
  EXPECT_THROW(model_.GenerateMaze(0, 0), std::out_of_range);
  EXPECT_THROW(model_.GenerateMaze(-1, -1), std::out_of_range);
  EXPECT_THROW(model_.GenerateMaze(51, 51), std::out_of_range);
  constexpr const char* kOverMaze = "51 51\n";
  writeTempFile(tmp_file_, kOverMaze);
  EXPECT_THROW(model_.loadMaze(tmp_file_), std::out_of_range);
  EXPECT_THROW(model_.saveMaze(tmp_dir_ / "nope.txt"), std::out_of_range);
  constexpr const char* kOneMaze = "1 1\n 5\n";
  writeTempFile(tmp_file_, kOneMaze);
  EXPECT_THROW(model_.loadMaze(tmp_file_), std::invalid_argument);
  constexpr const char* kOneSecondMaze = "1 1\n 1\n 5\n";
  writeTempFile(tmp_file_, kOneSecondMaze);
  EXPECT_THROW(model_.loadMaze(tmp_file_), std::invalid_argument);
  constexpr const char* kOneSecondNonMaze = "1 1\n 1\n $\n";
  writeTempFile(tmp_file_, kOneSecondNonMaze);
  EXPECT_THROW(model_.loadMaze(tmp_file_), std::invalid_argument);
  EXPECT_THROW(model_.saveMaze(tmp_dir_ / ""), std::runtime_error);
}

TEST_F(SaveMazeTest, SaveAndReloadProduceSameData) {
  model_.GenerateMaze(4, 6);
  const auto orig_rb = model_.getRightBorders();
  const auto orig_bb = model_.getBottomBorders();

  auto save_path = tmp_dir_ / "saved.txt";
  writeTempFile(save_path, "");

  model_.saveMaze(save_path);

  s21::MazeModel loaded;
  loaded.loadMaze(save_path);

  EXPECT_EQ(loaded.getRightBorders(), orig_rb);
  EXPECT_EQ(loaded.getBottomBorders(), orig_bb);
}

TEST_F(SaveMazeTest, SaveThenLoadPathStillExists) {
  model_.GenerateMaze(5, 5);
  auto save_path = tmp_dir_ / "path_test.txt";
  writeTempFile(save_path, "");
  model_.saveMaze(save_path);

  s21::MazeModel loaded;
  loaded.loadMaze(save_path);

  auto path = loaded.getPath({0, 0}, {4, 4});
  EXPECT_FALSE(path.empty());
}

TEST_F(SaveMazeTest, SavedFileContainsDimensionsOnFirstLine) {
  model_.GenerateMaze(3, 7);
  auto save_path = tmp_dir_ / "check_format.txt";
  writeTempFile(save_path, "");
  model_.saveMaze(save_path);

  std::ifstream f(save_path);
  int rows, cols;
  f >> rows >> cols;
  EXPECT_EQ(rows, 3);
  EXPECT_EQ(cols, 7);
}

class GetPathTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "maze_path_test";
  std::filesystem::path tmp_file_ = tmp_dir_ / "path_maze.txt";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }

  void generateAndLoad(int rows, int cols) { model_.GenerateMaze(rows, cols); }
};

TEST_F(GetPathTest, StartEqualsFinishReturnsSinglePoint) {
  generateAndLoad(5, 5);
  s21::Point start{2, 2};
  auto path = model_.getPath(start, start);
  ASSERT_EQ(path.size(), 1u);
  s21::Point expected{2, 2};
  EXPECT_EQ(path[0], expected);
}

TEST_F(GetPathTest, ThrowsOnStartOutOfBounds) {
  generateAndLoad(5, 5);
  s21::Point bad_neg{-1, 0};
  s21::Point bad_x{5, 0};
  s21::Point bad_y{0, 5};
  s21::Point ok{2, 2};
  EXPECT_THROW(model_.getPath(bad_neg, ok), std::out_of_range);
  EXPECT_THROW(model_.getPath(bad_x, ok), std::out_of_range);
  EXPECT_THROW(model_.getPath(bad_y, ok), std::out_of_range);
}

TEST_F(GetPathTest, ThrowsOnFinishOutOfBounds) {
  generateAndLoad(5, 5);
  s21::Point origin{0, 0};
  s21::Point fneg{-1, 2};
  s21::Point fx{5, 2};
  s21::Point fy{0, 5};
  EXPECT_THROW(model_.getPath(origin, fneg), std::out_of_range);
  EXPECT_THROW(model_.getPath(origin, fx), std::out_of_range);
  EXPECT_THROW(model_.getPath(origin, fy), std::out_of_range);
}

TEST_F(GetPathTest, PathStartsWithStartAndEndsWithFinish) {
  generateAndLoad(10, 10);
  s21::Point p_start{1, 1};
  s21::Point p_finish{8, 8};
  auto path = model_.getPath(p_start, p_finish);
  ASSERT_FALSE(path.empty());
  EXPECT_EQ(path.front(), p_start);
  EXPECT_EQ(path.back(), p_finish);
}

TEST_F(GetPathTest, ConsecutivePointsAreAdjacent) {
  generateAndLoad(8, 8);
  s21::Point from{0, 0};
  s21::Point to{7, 7};
  auto path = model_.getPath(from, to);
  ASSERT_GT(path.size(), 1u);
  for (size_t i = 1; i < path.size(); ++i) {
    int dx = std::abs(path[i].x - path[i - 1].x);
    int dy = std::abs(path[i].y - path[i - 1].y);
    int manhattan = dx + dy;
    EXPECT_EQ(manhattan, 1)
        << "Step " << i << ": (" << path[i - 1].x << "," << path[i - 1].y
        << ") -> (" << path[i].x << "," << path[i].y << ") is not adjacent";
  }
}

TEST_F(GetPathTest, PathDoesNotCrossWalls) {
  generateAndLoad(6, 6);
  const auto& rb = model_.getRightBorders();
  const auto& bb = model_.getBottomBorders();

  s21::Point from{0, 0};
  s21::Point to{5, 5};
  auto path = model_.getPath(from, to);
  ASSERT_GT(path.size(), 1u);

  for (size_t i = 1; i < path.size(); ++i) {
    const auto& prev = path[i - 1];
    const auto& curr = path[i];

    int dx = curr.x - prev.x;
    int dy = curr.y - prev.y;

    if (dx == 1) {
      EXPECT_EQ(rb[prev.y][prev.x], 0)
          << "Path crosses right wall at (" << prev.x << "," << prev.y << ")";
    } else if (dx == -1) {
      EXPECT_EQ(rb[curr.y][curr.x], 0)
          << "Path crosses right wall at (" << curr.x << "," << curr.y << ")";
    } else if (dy == 1) {
      EXPECT_EQ(bb[prev.y][prev.x], 0)
          << "Path crosses bottom wall at (" << prev.x << "," << prev.y << ")";
    } else if (dy == -1) {
      EXPECT_EQ(bb[curr.y][curr.x], 0)
          << "Path crosses bottom wall at (" << curr.x << "," << curr.y << ")";
    }
  }
}

TEST_F(GetPathTest, NoPathInFullyWalledMazeReturnsEmpty) {
  constexpr const char* kFullyWalledMaze =
      "2 2\n"
      "1 1\n"
      "1 1\n"
      "\n"
      "1 1\n"
      "1 1\n";
  writeTempFile(tmp_file_, kFullyWalledMaze);
  model_.loadMaze(tmp_file_);

  s21::Point from{0, 0};
  s21::Point to{1, 1};
  auto path = model_.getPath(from, to);
  EXPECT_TRUE(path.empty())
      << "No path should exist when all walls are present";
}

TEST_F(GetPathTest, PathExistsInOpenMaze) {
  writeTempFile(tmp_file_, kOpen3x3);
  model_.loadMaze(tmp_file_);

  s21::Point from{0, 0};
  s21::Point to{2, 2};
  auto path = model_.getPath(from, to);
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.front(), from);
  EXPECT_EQ(path.back(), to);
}

TEST_F(GetPathTest, PathForAdjacentOpenCells) {
  writeTempFile(tmp_file_, kOpen3x3);
  model_.loadMaze(tmp_file_);

  s21::Point from{0, 0};
  s21::Point to{1, 0};
  auto path = model_.getPath(from, to);
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2u);
  EXPECT_EQ(path[0], from);
  EXPECT_EQ(path[1], to);
}

TEST_F(GetPathTest, MultiplePathsSearchedInLargerMaze) {
  generateAndLoad(20, 20);

  s21::Point const points[] = {{0, 0},   {19, 0},  {0, 19},
                               {19, 19}, {10, 10}, {5, 15}};
  for (const auto& a : points)
    for (const auto& b : points)
      if (a != b) EXPECT_FALSE(model_.getPath(a, b).empty());
}

class PointTest : public ::testing::Test {};

TEST_F(PointTest, DefaultConstructionIsZeroZero) {
  s21::Point p;
  EXPECT_EQ(p.x, 0);
  EXPECT_EQ(p.y, 0);
}

TEST_F(PointTest, AggregateInitialization) {
  s21::Point p{3, 7};
  EXPECT_EQ(p.x, 3);
  EXPECT_EQ(p.y, 7);
}

TEST_F(PointTest, Equality) {
  s21::Point a{1, 2};
  s21::Point b{1, 2};
  EXPECT_EQ(a, b);
}

TEST_F(PointTest, Inequality) {
  s21::Point a{1, 2};
  s21::Point b{2, 1};
  EXPECT_NE(a, b);
}

TEST_F(PointTest, LessThan) {
  s21::Point p00{0, 0};
  s21::Point p10{1, 0};
  s21::Point p01{0, 1};
  s21::Point p11{1, 1};
  EXPECT_LT(p00, p10);
  EXPECT_LT(p00, p01);
  EXPECT_LT(p00, p11);
  EXPECT_FALSE(p10 < p00);
}

TEST_F(PointTest, ThreeWayComparisonPartialOrdering) {
  s21::Point a{1, 2};
  s21::Point b{1, 2};
  s21::Point c{1, 3};
  auto cmp_ab = a <=> b;
  auto cmp_ac = a <=> c;
  auto cmp_ca = c <=> a;
  EXPECT_EQ(cmp_ab, std::strong_ordering::equal);
  EXPECT_EQ(cmp_ac, std::strong_ordering::less);
  EXPECT_EQ(cmp_ca, std::strong_ordering::greater);
}

class RoundTripTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "maze_roundtrip_test";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(RoundTripTest, GenerateSaveLoadProduceIdenticalMaze) {
  model_.GenerateMaze(15, 20);
  const auto orig_rb = model_.getRightBorders();
  const auto orig_bb = model_.getBottomBorders();

  auto f = tmp_dir_ / "roundtrip.txt";
  writeTempFile(f, "");
  model_.saveMaze(f);

  s21::MazeModel loaded;
  loaded.loadMaze(f);

  EXPECT_EQ(loaded.getRightBorders(), orig_rb);
  EXPECT_EQ(loaded.getBottomBorders(), orig_bb);
}

TEST_F(RoundTripTest, PathSurvivesRoundTrip) {
  model_.GenerateMaze(12, 12);
  s21::Point start{0, 0};
  s21::Point finish{11, 11};
  auto orig_path = model_.getPath(start, finish);

  auto f = tmp_dir_ / "rt_path.txt";
  writeTempFile(f, "");
  model_.saveMaze(f);

  s21::MazeModel loaded;
  loaded.loadMaze(f);
  auto loaded_path = loaded.getPath(start, finish);

  ASSERT_EQ(orig_path.size(), loaded_path.size());
  for (size_t i = 0; i < orig_path.size(); ++i)
    EXPECT_EQ(orig_path[i], loaded_path[i]);
}

class StructuralTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
};

TEST_F(StructuralTest, AllRightBorderColumnsHaveCorrectWidth) {
  for (int size : {1, 5, 25, 50}) {
    model_.GenerateMaze(size, size);
    for (const auto& row : model_.getRightBorders())
      EXPECT_EQ(static_cast<int>(row.size()), size);
  }
}

TEST_F(StructuralTest, AllBottomBorderRowsHaveCorrectHeight) {
  for (int size : {1, 5, 25, 50}) {
    model_.GenerateMaze(size, size);
    EXPECT_EQ(static_cast<int>(model_.getBottomBorders().size()), size);
  }
}

TEST_F(StructuralTest, RectangularMaze) {
  model_.GenerateMaze(3, 10);
  EXPECT_EQ(model_.getRightBorders().size(), 3u);
  EXPECT_EQ(model_.getBottomBorders().size(), 3u);
  for (const auto& row : model_.getRightBorders()) EXPECT_EQ(row.size(), 10u);
  for (const auto& row : model_.getBottomBorders()) EXPECT_EQ(row.size(), 10u);
}

TEST_F(StructuralTest, TallMaze) {
  model_.GenerateMaze(10, 3);
  EXPECT_EQ(model_.getRightBorders().size(), 10u);
  for (const auto& row : model_.getRightBorders()) EXPECT_EQ(row.size(), 3u);
}

class GetPathEdgeTest : public ::testing::Test {
 protected:
  s21::MazeModel model_;
};

TEST_F(GetPathEdgeTest, CornerToCornerIn1x1) {
  model_.GenerateMaze(1, 1);
  s21::Point origin{0, 0};
  auto path = model_.getPath(origin, origin);
  ASSERT_EQ(path.size(), 1u);
  EXPECT_EQ(path[0], origin);
}

TEST_F(GetPathEdgeTest, CornerToCornerIn1xN) {
  model_.GenerateMaze(1, 5);

  auto path = model_.getPath({0, 0}, {4, 0});
  EXPECT_FALSE(path.empty());
}

TEST_F(GetPathEdgeTest, CornerToCornerInNx1) {
  model_.GenerateMaze(5, 1);
  auto path = model_.getPath({0, 0}, {0, 4});
  EXPECT_FALSE(path.empty());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
