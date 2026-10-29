#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <queue>
#include <random>
#include <ranges>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#define private public
#include "maze_model.h"
#include "qlearning_model.h"
#undef private

namespace s21 {

inline std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "Point{x=" << p.x << ", y=" << p.y << '}';
}

inline std::ostream& operator<<(std::ostream& os, Action a) {
  switch (a) {
    case Action::kUp:
      return os << "Action::kUp";
    case Action::kRight:
      return os << "Action::kRight";
    case Action::kDown:
      return os << "Action::kDown";
    case Action::kLeft:
      return os << "Action::kLeft";
  }
  return os << "Action(" << static_cast<int>(a) << ')';
}

}  // namespace s21

using namespace s21;

class QLearningModelTest : public ::testing::Test {
 protected:
  static void setManualMaze(QLearningModel& m, int rows, int cols,
                            std::vector<std::vector<int>> right,
                            std::vector<std::vector<int>> bottom) {
    m.maze_.rows_ = rows;
    m.maze_.cols_ = cols;
    m.maze_.right_borders_ = std::move(right);
    m.maze_.bottom_borders_ = std::move(bottom);
    m.init();
  }

  static void make1x1(QLearningModel& m) {
    setManualMaze(m, 1, 1, {{1}}, {{1}});
  }

  static void makeOpen2x2(QLearningModel& m) {
    setManualMaze(m, 2, 2, {{0, 1}, {0, 1}}, {{0, 0}, {1, 1}});
  }

  static void makeClosed1x2(QLearningModel& m) {
    setManualMaze(m, 1, 2, {{1, 1}}, {{1, 1}});
  }

  static void makeOpen1x2(QLearningModel& m) {
    setManualMaze(m, 1, 2, {{0, 1}}, {{1, 1}});
  }

  static void makeOpen1x3(QLearningModel& m) {
    setManualMaze(m, 1, 3, {{0, 0, 1}}, {{1, 1, 1}});
  }

  static void makeClosed2x1(QLearningModel& m) {
    setManualMaze(m, 2, 1, {{1}, {1}}, {{1}, {1}});
  }

  static void setQ(QLearningModel& m, int x, int y, Action a, double value) {
    m.q_table_[static_cast<size_t>(y)][static_cast<size_t>(x)]
              [static_cast<int>(a)] = value;
  }

  static bool allQZero(const QLearningModel& m) {
    auto flat_view = m.q_table_ | std::views::join;
    auto flat_flat_view = flat_view | std::views::join;
    return std::ranges::all_of(flat_flat_view,
                               [](double value) { return value == 0.0; });
  }

  static std::string makeMazeContent(
      int rows, int cols, const std::vector<std::vector<int>>& right,
      const std::vector<std::vector<int>>& bottom) {
    MazeModel mz;
    mz.rows_ = rows;
    mz.cols_ = cols;
    mz.right_borders_ = right;
    mz.bottom_borders_ = bottom;
    return mz.saveMazeToString();
  }
};

TEST_F(QLearningModelTest, InitThrowsWhenMazeEmpty) {
  QLearningModel m;
  EXPECT_THROW(m.init(), std::runtime_error);
}

TEST_F(QLearningModelTest, InitThrowsWhenColsZero) {
  QLearningModel m;

  m.maze_.rows_ = 1;
  m.maze_.cols_ = 0;
  m.maze_.right_borders_ = std::vector<std::vector<int>>{{}};
  m.maze_.bottom_borders_ = std::vector<std::vector<int>>{{}};

  EXPECT_THROW(m.init(), std::runtime_error);
}

TEST_F(QLearningModelTest, LoadMazeFromStringValid) {
  const std::string content =
      makeMazeContent(2, 2, {{0, 1}, {0, 1}}, {{0, 0}, {1, 1}});

  QLearningModel m;
  ASSERT_NO_THROW(m.loadMazeFromString(content));

  EXPECT_EQ(m.rows_, 2u);
  EXPECT_EQ(m.cols_, 2u);
  EXPECT_FALSE(m.isTrained());
}

TEST_F(QLearningModelTest, LoadMazeFromFileValid) {
  const std::string content = makeMazeContent(1, 3, {{0, 0, 1}}, {{1, 1, 1}});

  const auto path =
      std::filesystem::temp_directory_path() / "qlearning_model_test_maze.txt";

  {
    std::ofstream out(path, std::ios::trunc);
    ASSERT_TRUE(out.is_open());
    out << content;
    ASSERT_TRUE(out.good());
  }

  QLearningModel m;
  ASSERT_NO_THROW(m.loadMazeFromFile(path));

  EXPECT_EQ(m.rows_, 1u);
  EXPECT_EQ(m.cols_, 3u);
  EXPECT_FALSE(m.isTrained());

  std::error_code ec;
  std::filesystem::remove(path, ec);
}

TEST_F(QLearningModelTest, SetEndPointThrowsWhenMazeNotLoaded) {
  QLearningModel m;
  EXPECT_THROW(m.setEndPoint(Point{0, 0}), std::runtime_error);
}

TEST_F(QLearningModelTest, SetEndPointThrowsWhenColsZero) {
  QLearningModel m;
  m.rows_ = 1u;
  m.cols_ = 0u;

  EXPECT_THROW(m.setEndPoint(Point{0, 0}), std::runtime_error);
}

TEST_F(QLearningModelTest, SetEndPointOutOfRange) {
  QLearningModel m;
  makeOpen2x2(m);

  EXPECT_THROW(m.setEndPoint(Point{-1, 0}), std::out_of_range);
  EXPECT_THROW(m.setEndPoint(Point{2, 0}), std::out_of_range);
  EXPECT_THROW(m.setEndPoint(Point{0, -1}), std::out_of_range);
  EXPECT_THROW(m.setEndPoint(Point{0, 2}), std::out_of_range);
}

TEST_F(QLearningModelTest, SetEndPointResetsStateAndQTable) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 1});
  m.trained_ = true;
  setQ(m, 0, 0, Action::kRight, 42.0);

  ASSERT_FALSE(allQZero(m));

  m.setEndPoint(Point{0, 1});

  EXPECT_FALSE(m.isTrained());
  EXPECT_EQ(m.getEndPoint(), Point(0, 1));
  EXPECT_TRUE(allQZero(m));
}

TEST_F(QLearningModelTest, GetQValueReturnsStoredValue) {
  QLearningModel m;
  makeOpen2x2(m);

  setQ(m, 0, 0, Action::kRight, 3.25);

  EXPECT_DOUBLE_EQ(m.getQValue(Point{0, 0}, Action::kRight), 3.25);
}

TEST_F(QLearningModelTest, CanMoveCoversAllDirectionsAndWalls) {
  QLearningModel m;
  makeOpen2x2(m);

  EXPECT_FALSE(m.canMove(Point{0, 0}, Action::kUp));
  EXPECT_TRUE(m.canMove(Point{0, 0}, Action::kRight));
  EXPECT_TRUE(m.canMove(Point{0, 0}, Action::kDown));
  EXPECT_FALSE(m.canMove(Point{0, 0}, Action::kLeft));

  EXPECT_TRUE(m.canMove(Point{1, 1}, Action::kUp));
  EXPECT_FALSE(m.canMove(Point{1, 1}, Action::kRight));
  EXPECT_FALSE(m.canMove(Point{1, 1}, Action::kDown));
  EXPECT_TRUE(m.canMove(Point{1, 1}, Action::kLeft));

  makeClosed1x2(m);
  EXPECT_FALSE(m.canMove(Point{0, 0}, Action::kRight));
  EXPECT_FALSE(m.canMove(Point{1, 0}, Action::kLeft));

  makeClosed2x1(m);
  EXPECT_FALSE(m.canMove(Point{0, 0}, Action::kDown));
  EXPECT_FALSE(m.canMove(Point{0, 1}, Action::kUp));

  EXPECT_FALSE(m.canMove(Point{0, 0}, static_cast<Action>(99)));
}

TEST_F(QLearningModelTest, GetNextStateMovesByAction) {
  QLearningModel m;
  make1x1(m);

  EXPECT_EQ(m.getNextState(Point{1, 2}, Action::kUp), Point(1, 1));
  EXPECT_EQ(m.getNextState(Point{1, 2}, Action::kDown), Point(1, 3));
  EXPECT_EQ(m.getNextState(Point{1, 2}, Action::kLeft), Point(0, 2));
  EXPECT_EQ(m.getNextState(Point{1, 2}, Action::kRight), Point(2, 2));
}

TEST_F(QLearningModelTest, GetBestActionChoosesMaxQAndKeepsUpOnTie) {
  QLearningModel m;
  makeOpen2x2(m);

  const Point p{0, 0};

  EXPECT_EQ(m.getBestAction(p), Action::kUp);

  setQ(m, 0, 0, Action::kRight, 1.0);
  EXPECT_EQ(m.getBestAction(p), Action::kRight);

  setQ(m, 0, 0, Action::kDown, 2.0);
  EXPECT_EQ(m.getBestAction(p), Action::kDown);

  setQ(m, 0, 0, Action::kLeft, 3.0);
  EXPECT_EQ(m.getBestAction(p), Action::kLeft);
}

TEST_F(QLearningModelTest, ChooseActionGreedyUsesBestAction) {
  QLearningModel m;
  makeOpen2x2(m);

  setQ(m, 0, 0, Action::kLeft, 10.0);

  EXPECT_EQ(m.chooseAction(Point{0, 0}, 0.0), Action::kLeft);
}

TEST_F(QLearningModelTest, TrainEpisodesThrowsWhenMazeNotLoaded) {
  QLearningModel m;
  EXPECT_THROW(m.trainEpisodes(1, 0.1, 0.0), std::runtime_error);
}

TEST_F(QLearningModelTest, TrainEpisodesThrowsWhenColsZero) {
  QLearningModel m;
  m.rows_ = 1u;
  m.cols_ = 0u;

  EXPECT_THROW(m.trainEpisodes(1, 0.1, 0.0), std::runtime_error);
}

TEST_F(QLearningModelTest, TrainEpisodesThrowsWhenEndNotSet) {
  QLearningModel m;
  makeOpen2x2(m);

  EXPECT_THROW(m.trainEpisodes(1, 0.1, 0.0), std::runtime_error);
}

TEST_F(QLearningModelTest, TrainEpisodesDoesNothingForNonPositiveEpisodes) {
  QLearningModel m;
  makeOpen2x2(m);
  m.setEndPoint(Point{1, 1});

  m.trainEpisodes(0, 0.1, 0.0);
  EXPECT_FALSE(m.isTrained());

  m.trainEpisodes(-5, 0.1, 0.0);
  EXPECT_FALSE(m.isTrained());
}

TEST_F(QLearningModelTest, TrainEpisodesForSingleCellMazeMarksTrained) {
  QLearningModel m;
  make1x1(m);

  m.setEndPoint(Point{0, 0});
  m.trainEpisodes(10, 0.1, 0.0);

  EXPECT_TRUE(m.isTrained());

  const auto path = m.getBestPath(Point{0, 0});
  const std::vector<Point> expected{Point{0, 0}};
  EXPECT_EQ(path, expected);
}

TEST_F(QLearningModelTest, TrainEpisodesReachesGoalAndMarksTrained) {
  QLearningModel m;
  makeOpen1x2(m);

  m.setEndPoint(Point{1, 0});

  setQ(m, 0, 0, Action::kRight, 10.0);

  ASSERT_NO_THROW(m.trainEpisodes(5, 0.5, 0.0));

  EXPECT_TRUE(m.isTrained());
  EXPECT_GT(m.getQValue(Point{0, 0}, Action::kRight), 10.0);
}

TEST_F(QLearningModelTest, TrainEpisodesHandlesWallPenalty) {
  QLearningModel m;
  makeClosed1x2(m);

  m.setEndPoint(Point{1, 0});

  setQ(m, 0, 0, Action::kUp, 10.0);

  ASSERT_NO_THROW(m.trainEpisodes(1, 0.1, 0.0));

  EXPECT_TRUE(m.isTrained());
  EXPECT_LT(m.getQValue(Point{0, 0}, Action::kUp), 10.0);
}

TEST_F(QLearningModelTest, TrainEpisodesHandlesStepPenaltyWithoutGoal) {
  QLearningModel m;
  makeOpen1x3(m);

  m.setEndPoint(Point{2, 0});
  setQ(m, 0, 0, Action::kRight, 10.0);
  setQ(m, 1, 0, Action::kLeft, 10.0);

  ASSERT_NO_THROW(m.trainEpisodes(1, 0.1, 0.0));

  EXPECT_TRUE(m.isTrained());
}

TEST_F(QLearningModelTest, TrainEpisodesWithFullExploration) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 1});

  ASSERT_NO_THROW(m.trainEpisodes(200, 0.2, 1.0));

  EXPECT_TRUE(m.isTrained());
}

TEST_F(QLearningModelTest, GetBestPathThrowsWhenMazeNotLoaded) {
  QLearningModel m;
  EXPECT_THROW(m.getBestPath(Point{0, 0}), std::runtime_error);
}

TEST_F(QLearningModelTest, GetBestPathThrowsWhenColsZero) {
  QLearningModel m;
  m.rows_ = 1u;
  m.cols_ = 0u;

  EXPECT_THROW(m.getBestPath(Point{0, 0}), std::runtime_error);
}

TEST_F(QLearningModelTest, GetBestPathThrowsWhenStartOutOfRange) {
  QLearningModel m;
  makeOpen2x2(m);

  EXPECT_THROW(m.getBestPath(Point{-1, 0}), std::out_of_range);
  EXPECT_THROW(m.getBestPath(Point{2, 0}), std::out_of_range);
  EXPECT_THROW(m.getBestPath(Point{0, -1}), std::out_of_range);
  EXPECT_THROW(m.getBestPath(Point{0, 2}), std::out_of_range);
}

TEST_F(QLearningModelTest, GetBestPathThrowsWhenNotTrained) {
  QLearningModel m;
  makeOpen2x2(m);

  EXPECT_THROW(m.getBestPath(Point{0, 0}), std::runtime_error);
}

TEST_F(QLearningModelTest, GetBestPathReturnsSinglePointWhenStartEqualsEnd) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 1});
  m.trained_ = true;

  const auto path = m.getBestPath(Point{1, 1});
  const std::vector<Point> expected{Point{1, 1}};

  EXPECT_EQ(path, expected);
}

TEST_F(QLearningModelTest, GetBestPathFollowsLearnedQValuesToGoal) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 1});
  m.trained_ = true;

  setQ(m, 0, 0, Action::kDown, 10.0);
  setQ(m, 0, 1, Action::kRight, 10.0);

  const auto path = m.getBestPath(Point{0, 0});

  const std::vector<Point> expected{Point{0, 0}, Point{0, 1}, Point{1, 1}};

  EXPECT_EQ(path, expected);
}

TEST_F(QLearningModelTest, GetBestPathStopsWhenBestActionBlocked) {
  QLearningModel m;
  makeClosed1x2(m);

  m.setEndPoint(Point{1, 0});
  m.trained_ = true;

  const auto path = m.getBestPath(Point{0, 0});

  const std::vector<Point> expected{Point{0, 0}};

  EXPECT_EQ(path, expected);
}

TEST_F(QLearningModelTest, GetBestPathStopsWhenEnteringVisitedCell) {
  QLearningModel m;
  makeOpen1x3(m);

  m.setEndPoint(Point{2, 0});
  m.trained_ = true;

  setQ(m, 0, 0, Action::kRight, 10.0);
  setQ(m, 1, 0, Action::kLeft, 10.0);

  const auto path = m.getBestPath(Point{0, 0});

  const std::vector<Point> expected{Point{0, 0}, Point{1, 0}};

  EXPECT_EQ(path, expected);
}

TEST_F(QLearningModelTest, GettersExposeBordersAndEndPoint) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 0});

  EXPECT_EQ(m.getRightBorders().size(), 2u);
  EXPECT_EQ(m.getBottomBorders().size(), 2u);
  EXPECT_EQ(m.getEndPoint(), Point(1, 0));
  EXPECT_FALSE(m.isTrained());
}

TEST_F(QLearningModelTest, TrainedModelReturnsNonEmptyPathStartingAtStart) {
  QLearningModel m;
  makeOpen2x2(m);

  m.setEndPoint(Point{1, 1});

  m.trainEpisodes(300, 0.3, 0.4);

  const auto path = m.getBestPath(Point{0, 0});

  ASSERT_FALSE(path.empty());
  EXPECT_EQ(path.front(), Point(0, 0));
}