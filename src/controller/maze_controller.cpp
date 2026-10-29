#include "maze_controller.h"

#include <iostream>

namespace s21 {
MazeController::MazeController(MazeModel* model) : model_(model) {}

MazeController::~MazeController() = default;

void MazeController::loadMaze(const QString& path) {
  try {
    model_->loadMaze(path.toStdString());
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void MazeController::saveMaze(const QString& path) {
  try {
    model_->saveMaze(path.toStdString());
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void MazeController::genMaze(int rows, int cols) {
  try {
    model_->GenerateMaze(rows, cols);
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

QPainterPath MazeController::getPath(QPoint start, QPoint finish) {
  QPainterPath cellPath;
  try {
    auto pathIndexes =
        model_->getPath({start.x(), start.y()}, {finish.x(), finish.y()});
    cellPath.moveTo(pathIndexes[0].x, pathIndexes[0].y);

    for (int i = 1; i < pathIndexes.size(); ++i) {
      cellPath.lineTo(pathIndexes[i].x, pathIndexes[i].y);
    }
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
  return cellPath;
}
}  // namespace s21