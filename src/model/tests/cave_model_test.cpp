#include "cave_model.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>

namespace {

void writeTempFile(const std::filesystem::path& path,
                   std::string_view content) {
  std::ofstream f(path);
  f << content;
}

constexpr const char* kExample3x3 =
    "3 3\n"
    "0 1 0\n"
    "1 0 1\n"
    "0 1 0\n";

constexpr const char* kExample4x4 =
    "4 4\n"
    "0 1 0 1\n"
    "1 0 0 1\n"
    "0 1 0 0\n"
    "0 0 1 1\n";

constexpr const char* k1x1 =
    "1 1\n"
    "1\n";

constexpr int kDefaultBirth = 4;
constexpr int kDefaultDeath = 3;

}  // namespace

class InitCaveTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
};

TEST_F(InitCaveTest, ValidDimensionsProduceCorrectSize) {
  model_.initCave(10, 15, 0.5);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave.size(), 10u);
  for (const auto& row : cave) EXPECT_EQ(row.size(), 15u);
}

TEST_F(InitCaveTest, MinimumSize1x1) {
  model_.initCave(1, 1, 0.5);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave.size(), 1u);
  EXPECT_EQ(cave[0].size(), 1u);
}

TEST_F(InitCaveTest, MaximumSize50x50) {
  model_.initCave(50, 50, 0.5);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave.size(), 50u);
  for (const auto& row : cave) EXPECT_EQ(row.size(), 50u);
}

TEST_F(InitCaveTest, ThrowsOnZeroRows) {
  EXPECT_THROW(model_.initCave(0, 10, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnZeroCols) {
  EXPECT_THROW(model_.initCave(10, 0, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnNegativeRows) {
  EXPECT_THROW(model_.initCave(-1, 10, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnNegativeCols) {
  EXPECT_THROW(model_.initCave(10, -1, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnRowsExceed50) {
  EXPECT_THROW(model_.initCave(51, 10, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnColsExceed50) {
  EXPECT_THROW(model_.initCave(10, 51, 0.5), std::out_of_range);
}

TEST_F(InitCaveTest, ThrowsOnChanceBelowZero) {
  EXPECT_THROW(model_.initCave(10, 10, -0.01), std::invalid_argument);
}

TEST_F(InitCaveTest, ThrowsOnChanceAboveOne) {
  EXPECT_THROW(model_.initCave(10, 10, 1.01), std::invalid_argument);
}

TEST_F(InitCaveTest, ChanceZeroProducesAllDeadCave) {
  model_.initCave(10, 10, 0.0);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 0);
}

TEST_F(InitCaveTest, ChanceOneProducesAllAliveCave) {
  model_.initCave(10, 10, 1.0);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
}

TEST_F(InitCaveTest, CellsAreBinary) {
  model_.initCave(20, 20, 0.5);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_TRUE(val == 0 || val == 1);
}

TEST_F(InitCaveTest, RectangularCave) {
  model_.initCave(3, 10, 0.5);
  EXPECT_EQ(model_.getCave().size(), 3u);
  for (const auto& row : model_.getCave()) EXPECT_EQ(row.size(), 10u);
}

class GenerateStepTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "maze_save_test";
  std::filesystem::path tmp_file_ = tmp_dir_ / "path_maze.txt";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(GenerateStepTest, ThrowsOnNonExistentFile) {
  EXPECT_THROW(model_.saveCave(tmp_dir_ / "nope.txt"), std::out_of_range);
  EXPECT_THROW(model_.initCave(0, 0, 1.0), std::out_of_range);
  EXPECT_THROW(model_.initCave(-1, -1, 1.0), std::out_of_range);
  EXPECT_THROW(model_.initCave(51, 51, 1.0), std::out_of_range);
  constexpr const char* kOverMaze = "51 51\n";
  writeTempFile(tmp_file_, kOverMaze);
  EXPECT_THROW(model_.loadCave(tmp_file_), std::out_of_range);
  EXPECT_THROW(model_.saveCave(tmp_dir_ / "nope.txt"), std::out_of_range);
  constexpr const char* kOneMaze = "1 1\n 5\n";
  writeTempFile(tmp_file_, kOneMaze);
  EXPECT_THROW(model_.loadCave(tmp_file_), std::invalid_argument);
  EXPECT_THROW(model_.saveCave(tmp_dir_ / ""), std::runtime_error);
}

TEST_F(GenerateStepTest, EmptyCaveNoOp) {
  EXPECT_NO_THROW(model_.generateStep(kDefaultBirth, kDefaultDeath));
}

TEST_F(GenerateStepTest, AllAliveCaveStaysAllAlive) {
  model_.initCave(5, 5, 1.0);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
}

TEST_F(GenerateStepTest, AllDeadCaveProducesAliveCornersOnly) {
  model_.initCave(5, 5, 0.0);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  const auto& cave = model_.getCave();

  EXPECT_EQ(cave[0][0], 1);
  EXPECT_EQ(cave[0][4], 1);
  EXPECT_EQ(cave[4][0], 1);
  EXPECT_EQ(cave[4][4], 1);

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      bool is_corner = (i == 0 || i == 4) && (j == 0 || j == 4);
      if (!is_corner) {
        EXPECT_EQ(cave[i][j], 0)
            << "Non-corner cell (" << i << "," << j << ") should be dead";
      }
    }
  }
}

TEST_F(GenerateStepTest, BirthRuleKicksInWhenAboveLimit) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_birth_test.txt";
  writeTempFile(tmp,
                "3 3\n"
                "0 0 0\n"
                "0 1 0\n"
                "0 0 0\n");
  model_.loadCave(tmp);

  model_.generateStep(0, 0);
  const auto& cave = model_.getCave();

  EXPECT_EQ(cave[1][1], 1);

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      if (i != 1 || j != 1)
        EXPECT_EQ(cave[i][j], 1)
            << "Cell (" << i << "," << j << ") should be alive";
  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, DeathRuleKillsIsolatedCell) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_death_test.txt";
  writeTempFile(tmp,
                "3 3\n"
                "0 0 0\n"
                "0 1 0\n"
                "0 0 0\n");
  model_.loadCave(tmp);

  model_.generateStep(7, 3);
  EXPECT_EQ(model_.getCave()[1][1], 0);

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      if (i != 1 || j != 1) EXPECT_EQ(model_.getCave()[i][j], 0);
  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, OutOfBoundsCellsCountAsAlive) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_oob_test.txt";
  writeTempFile(tmp, "1 1\n1\n");
  model_.loadCave(tmp);
  model_.generateStep(7, 3);
  EXPECT_EQ(model_.getCave()[0][0], 1)
      << "1x1 alive cell with 8 OOB neighbors should stay alive";

  writeTempFile(tmp, "1 1\n0\n");
  model_.loadCave(tmp);
  model_.generateStep(7, 3);
  EXPECT_EQ(model_.getCave()[0][0], 1)
      << "1x1 dead cell with 8 OOB alive neighbors should become alive";
  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, DoesNotMutateWhileComputing) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_buffer_test.txt";
  writeTempFile(tmp, "1 3\n1 0 1\n");
  model_.loadCave(tmp);
  model_.generateStep(4, 3);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave[0][0], 1);
  EXPECT_EQ(cave[0][1], 1);
  EXPECT_EQ(cave[0][2], 1);
  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, MultipleStepsConverge) {
  model_.initCave(20, 20, 0.5);
  for (int i = 0; i < 50; ++i) {
    model_.generateStep(kDefaultBirth, kDefaultDeath);
    for (const auto& row : model_.getCave())
      for (int val : row) EXPECT_TRUE(val == 0 || val == 1);
  }
}

TEST_F(GenerateStepTest, SizePreservedAfterStep) {
  model_.initCave(7, 11, 0.5);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  EXPECT_EQ(model_.getCave().size(), 7u);
  for (const auto& row : model_.getCave()) EXPECT_EQ(row.size(), 11u);
}

TEST_F(GenerateStepTest, DifferentLimitsProduceDifferentResults) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_limits_test.txt";
  writeTempFile(tmp, "3 3\n0 0 0\n0 0 0\n0 0 0\n");

  s21::CaveModel model1;
  model1.loadCave(tmp);
  model1.generateStep(0, 7);
  EXPECT_EQ(model1.getCave()[0][0], 1);

  s21::CaveModel model2;
  model2.loadCave(tmp);
  model2.generateStep(7, 7);
  EXPECT_EQ(model2.getCave()[0][0], 0);

  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, BirthLimitZeroRevivesAllDeadCells) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_revive_test.txt";
  writeTempFile(tmp,
                "3 3\n"
                "0 0 0\n"
                "0 1 0\n"
                "0 0 0\n");
  model_.loadCave(tmp);
  model_.generateStep(0, 0);

  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
  std::filesystem::remove(tmp);
}

TEST_F(GenerateStepTest, DeathLimitZeroKeepsAllAlive) {
  model_.initCave(5, 5, 1.0);
  model_.generateStep(7, 0);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
}

TEST_F(GenerateStepTest, BirthLimit7PreventsBirth) {
  model_.initCave(5, 5, 0.0);
  model_.generateStep(7, 0);

  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 0);
}

class LoadCaveTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "cave_test";
  std::filesystem::path tmp_file_ = tmp_dir_ / "test_cave.txt";

  void SetUp() override {
    std::filesystem::create_directories(tmp_dir_);
    writeTempFile(tmp_file_, kExample3x3);
  }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(LoadCaveTest, LoadsValidFileCorrectly) {
  model_.loadCave(tmp_file_);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave.size(), 3u);
  for (const auto& row : cave) EXPECT_EQ(row.size(), 3u);

  EXPECT_EQ(cave[0][0], 0);
  EXPECT_EQ(cave[0][1], 1);
  EXPECT_EQ(cave[0][2], 0);
  EXPECT_EQ(cave[1][0], 1);
  EXPECT_EQ(cave[1][1], 0);
  EXPECT_EQ(cave[1][2], 1);
  EXPECT_EQ(cave[2][0], 0);
  EXPECT_EQ(cave[2][1], 1);
  EXPECT_EQ(cave[2][2], 0);
}

TEST_F(LoadCaveTest, Loads4x4ExampleFromSpec) {
  auto f = tmp_dir_ / "4x4.txt";
  writeTempFile(f, kExample4x4);
  model_.loadCave(f);
  const auto& cave = model_.getCave();
  EXPECT_EQ(cave.size(), 4u);
  for (const auto& row : cave) EXPECT_EQ(row.size(), 4u);
  EXPECT_EQ(cave[0], (std::vector<int>{0, 1, 0, 1}));
  EXPECT_EQ(cave[1], (std::vector<int>{1, 0, 0, 1}));
  EXPECT_EQ(cave[2], (std::vector<int>{0, 1, 0, 0}));
  EXPECT_EQ(cave[3], (std::vector<int>{0, 0, 1, 1}));
}

TEST_F(LoadCaveTest, ThrowsOnNonExistentFile) {
  EXPECT_THROW(model_.loadCave(tmp_dir_ / "nonexistent.txt"),
               std::runtime_error);
}

TEST_F(LoadCaveTest, ThrowsOnDirectoryNotFile) {
  EXPECT_THROW(model_.loadCave(tmp_dir_), std::runtime_error);
}

TEST_F(LoadCaveTest, ThrowsOnEmptyFile) {
  auto f = tmp_dir_ / "empty.txt";
  writeTempFile(f, "");
  EXPECT_THROW(model_.loadCave(f), std::invalid_argument);
}

TEST_F(LoadCaveTest, ThrowsOnZeroDimensions) {
  auto f = tmp_dir_ / "zero_dim.txt";
  writeTempFile(f, "0 5\n");
  EXPECT_THROW(model_.loadCave(f), std::out_of_range);
}

TEST_F(LoadCaveTest, ThrowsOnDimensionsExceed50) {
  auto f = tmp_dir_ / "big_dim.txt";
  writeTempFile(f, "51 5\n");
  EXPECT_THROW(model_.loadCave(f), std::out_of_range);
}

TEST_F(LoadCaveTest, ThrowsOnNegativeDimensions) {
  auto f = tmp_dir_ / "neg_dim.txt";
  writeTempFile(f, "-3 5\n");
  EXPECT_THROW(model_.loadCave(f), std::out_of_range);
}

TEST_F(LoadCaveTest, ThrowsOnInsufficientCellData) {
  auto f = tmp_dir_ / "truncated.txt";
  writeTempFile(f, "3 3\n0 1 0\n1 0\n");
  EXPECT_THROW(model_.loadCave(f), std::invalid_argument);
}

TEST_F(LoadCaveTest, ThrowsOnInvalidCellValue2) {
  auto f = tmp_dir_ / "bad_value.txt";
  writeTempFile(f, "2 2\n0 1\n2 0\n");
  EXPECT_THROW(model_.loadCave(f), std::invalid_argument);
}

TEST_F(LoadCaveTest, Loads1x1Cave) {
  auto f = tmp_dir_ / "1x1.txt";
  writeTempFile(f, k1x1);
  model_.loadCave(f);
  EXPECT_EQ(model_.getCave().size(), 1u);
  EXPECT_EQ(model_.getCave()[0].size(), 1u);
  EXPECT_EQ(model_.getCave()[0][0], 1);
}

TEST_F(LoadCaveTest, LoadsMaximumSizeCave) {
  auto f = tmp_dir_ / "50x50.txt";
  std::string content = "50 50\n";
  for (int i = 0; i < 50; ++i) {
    for (int j = 0; j < 50; ++j) {
      content += std::to_string((i + j) % 2);
      content += (j == 49 ? "\n" : " ");
    }
  }
  writeTempFile(f, content);
  model_.loadCave(f);
  EXPECT_EQ(model_.getCave().size(), 50u);
  for (const auto& row : model_.getCave()) EXPECT_EQ(row.size(), 50u);
}

TEST_F(LoadCaveTest, ReplacesPreviousCave) {
  model_.loadCave(tmp_file_);
  ASSERT_EQ(model_.getCave().size(), 3u);

  auto f = tmp_dir_ / "small.txt";
  writeTempFile(f, "1 1\n0\n");
  model_.loadCave(f);
  EXPECT_EQ(model_.getCave().size(), 1u);
  EXPECT_EQ(model_.getCave()[0].size(), 1u);
  EXPECT_EQ(model_.getCave()[0][0], 0);
}

TEST_F(LoadCaveTest, CanStepAfterLoad) {
  model_.loadCave(tmp_file_);
  EXPECT_NO_THROW(model_.generateStep(kDefaultBirth, kDefaultDeath));
}

class SaveCaveTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "cave_save_test";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(SaveCaveTest, ThrowsOnNonExistentFile) {
  model_.initCave(3, 3, 0.5);
  EXPECT_THROW(model_.saveCave(tmp_dir_ / ""), std::runtime_error);
}

TEST_F(SaveCaveTest, SaveAndReloadProduceSameData) {
  model_.initCave(4, 6, 1.0);
  auto save_path = tmp_dir_ / "saved.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  s21::CaveModel loaded;
  loaded.loadCave(save_path);

  EXPECT_EQ(loaded.getCave(), model_.getCave());
}

TEST_F(SaveCaveTest, SavedFileStartsWithDimensions) {
  model_.initCave(7, 3, 0.0);
  auto save_path = tmp_dir_ / "dims.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  std::ifstream f(save_path);
  int rows, cols;
  f >> rows >> cols;
  EXPECT_EQ(rows, 7);
  EXPECT_EQ(cols, 3);
}

TEST_F(SaveCaveTest, RoundTripWithCustomCave) {
  std::filesystem::path src = tmp_dir_ / "src.txt";
  writeTempFile(src, kExample4x4);
  model_.loadCave(src);

  auto save_path = tmp_dir_ / "roundtrip.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  s21::CaveModel loaded;
  loaded.loadCave(save_path);

  EXPECT_EQ(loaded.getCave(), model_.getCave());

  EXPECT_EQ(loaded.getCave()[0], (std::vector<int>{0, 1, 0, 1}));
  EXPECT_EQ(loaded.getCave()[3], (std::vector<int>{0, 0, 1, 1}));
}

TEST_F(SaveCaveTest, SaveAfterStepRoundTrip) {
  model_.initCave(5, 5, 1.0);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  auto save_path = tmp_dir_ / "after_step.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  s21::CaveModel loaded;
  loaded.loadCave(save_path);
  EXPECT_EQ(loaded.getCave(), model_.getCave());
}

class IntegrationTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
  std::filesystem::path tmp_dir_ =
      std::filesystem::temp_directory_path() / "cave_integration_test";

  void SetUp() override { std::filesystem::create_directories(tmp_dir_); }
  void TearDown() override { std::filesystem::remove_all(tmp_dir_); }
};

TEST_F(IntegrationTest, LoadStepSaveRoundTripPreservesFormat) {
  std::filesystem::path src = tmp_dir_ / "src.txt";
  writeTempFile(src, kExample3x3);
  model_.loadCave(src);

  model_.generateStep(kDefaultBirth, kDefaultDeath);

  auto save_path = tmp_dir_ / "after_step.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  s21::CaveModel loaded;
  loaded.loadCave(save_path);

  EXPECT_EQ(loaded.getCave(), model_.getCave());
  EXPECT_EQ(loaded.getCave().size(), 3u);
  for (const auto& row : loaded.getCave()) EXPECT_EQ(row.size(), 3u);
}

TEST_F(IntegrationTest, InitStepSaveRoundTrip) {
  model_.initCave(10, 10, 1.0);
  model_.generateStep(kDefaultBirth, kDefaultDeath);

  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);

  auto save_path = tmp_dir_ / "all_alive.txt";
  writeTempFile(save_path, "");
  model_.saveCave(save_path);

  s21::CaveModel loaded;
  loaded.loadCave(save_path);
  for (const auto& row : loaded.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
}

TEST_F(IntegrationTest, ChangingLimitsAffectsSubsequentSteps) {
  model_.initCave(5, 5, 0.5);
  model_.generateStep(0, 0);
  auto cave_lenient = model_.getCave();

  s21::CaveModel model2;
  model2.initCave(5, 5, 0.5);
  model2.generateStep(7, 7);
  auto cave_strict = model2.getCave();

  for (const auto& row : cave_lenient)
    for (int v : row) EXPECT_TRUE(v == 0 || v == 1);
  for (const auto& row : cave_strict)
    for (int v : row) EXPECT_TRUE(v == 0 || v == 1);
}

TEST_F(IntegrationTest, StepWithDifferentLimitsOnSameCave) {
  std::filesystem::path src = tmp_dir_ / "src.txt";
  writeTempFile(src,
                "5 5\n0 0 0 0 0\n0 1 1 1 0\n0 1 1 1 0\n0 1 1 1 0\n0 0 0 0 0\n");

  s21::CaveModel m1;
  m1.loadCave(src);
  m1.generateStep(0, 0);

  s21::CaveModel m2;
  m2.loadCave(src);
  m2.generateStep(7, 7);

  int alive_count_m1 = 0, alive_count_m2 = 0;

  alive_count_m1 = std::ranges::count(m1.getCave() | std::views::join, 1);
  alive_count_m2 = std::ranges::count(m2.getCave() | std::views::join, 1);

  EXPECT_GT(alive_count_m1, alive_count_m2)
      << "Lenient limits should produce more alive cells than strict limits";
  std::filesystem::remove(src);
}

class EdgeCaseTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
};

TEST_F(EdgeCaseTest, StepOn1x1AliveCave) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_1x1.txt";
  writeTempFile(tmp, "1 1\n1\n");
  model_.loadCave(tmp);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  EXPECT_EQ(model_.getCave()[0][0], 1);
  std::filesystem::remove(tmp);
}

TEST_F(EdgeCaseTest, StepOn1x1DeadCaveBecomesAlive) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_1x1_dead.txt";
  writeTempFile(tmp, "1 1\n0\n");
  model_.loadCave(tmp);
  model_.generateStep(7, kDefaultDeath);
  EXPECT_EQ(model_.getCave()[0][0], 1);
  std::filesystem::remove(tmp);
}

TEST_F(EdgeCaseTest, StepOn1x5Cave) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_1x5.txt";
  writeTempFile(tmp, "1 5\n0 1 0 1 0\n");
  model_.loadCave(tmp);
  EXPECT_NO_THROW(model_.generateStep(kDefaultBirth, kDefaultDeath));
  EXPECT_EQ(model_.getCave().size(), 1u);
  EXPECT_EQ(model_.getCave()[0].size(), 5u);
  std::filesystem::remove(tmp);
}

TEST_F(EdgeCaseTest, StepOn5x1Cave) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_5x1.txt";
  writeTempFile(tmp, "5 1\n0\n1\n0\n1\n0\n");
  model_.loadCave(tmp);
  EXPECT_NO_THROW(model_.generateStep(kDefaultBirth, kDefaultDeath));
  EXPECT_EQ(model_.getCave().size(), 5u);
  for (const auto& row : model_.getCave()) EXPECT_EQ(row.size(), 1u);
  std::filesystem::remove(tmp);
}

TEST_F(EdgeCaseTest, StepOn2x2Cave) {
  std::filesystem::path tmp =
      std::filesystem::temp_directory_path() / "cave_2x2.txt";
  writeTempFile(tmp, "2 2\n1 0\n0 1\n");
  model_.loadCave(tmp);
  EXPECT_NO_THROW(model_.generateStep(kDefaultBirth, kDefaultDeath));
  EXPECT_EQ(model_.getCave().size(), 2u);
  EXPECT_EQ(model_.getCave()[0].size(), 2u);

  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 1);
  std::filesystem::remove(tmp);
}

class DeterministicTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
};

TEST_F(DeterministicTest, AllAliveCaveStaysAliveForManySteps) {
  model_.initCave(10, 10, 1.0);
  for (int i = 0; i < 20; ++i) {
    model_.generateStep(kDefaultBirth, kDefaultDeath);
    for (const auto& row : model_.getCave())
      for (int val : row) EXPECT_EQ(val, 1);
  }
}

TEST_F(DeterministicTest, AllDeadCaveProducesAliveCornersAfterOneStep) {
  model_.initCave(5, 5, 0.0);
  model_.generateStep(kDefaultBirth, kDefaultDeath);
  const auto& cave = model_.getCave();

  EXPECT_EQ(cave[0][0], 1);
  EXPECT_EQ(cave[0][4], 1);
  EXPECT_EQ(cave[4][0], 1);
  EXPECT_EQ(cave[4][4], 1);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      bool is_corner = (i == 0 || i == 4) && (j == 0 || j == 4);
      if (!is_corner) {
        EXPECT_EQ(cave[i][j], 0);
      }
    }
  }
}

TEST_F(DeterministicTest, AllDeadCaveStaysDeadWithHighBirthLimit) {
  model_.initCave(5, 5, 0.0);
  model_.generateStep(7, kDefaultDeath);
  for (const auto& row : model_.getCave())
    for (int val : row) EXPECT_EQ(val, 0);
}

class GenerateStepLimitsTest : public ::testing::Test {
 protected:
  s21::CaveModel model_;
};

TEST_F(GenerateStepLimitsTest, AcceptsMinLimits) {
  model_.initCave(3, 3, 0.5);
  EXPECT_NO_THROW(model_.generateStep(0, 0));
}

TEST_F(GenerateStepLimitsTest, AcceptsMaxLimits) {
  model_.initCave(3, 3, 0.5);
  EXPECT_NO_THROW(model_.generateStep(7, 7));
}

TEST_F(GenerateStepLimitsTest, AcceptsNegativeBirthLimit) {
  model_.initCave(3, 3, 0.5);
  EXPECT_NO_THROW(model_.generateStep(-1, 0));
}

TEST_F(GenerateStepLimitsTest, AcceptsLimitsAbove7) {
  model_.initCave(3, 3, 0.5);
  EXPECT_NO_THROW(model_.generateStep(8, 8));
}
