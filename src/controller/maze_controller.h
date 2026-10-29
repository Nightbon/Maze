#pragma once
#include <QObject>
#include <QPainterPath>
#include <QPoint>
#include <QString>
#include <string>
#include <utility>

#include "maze_model.h"

namespace s21 {

class MazeController : public QObject {
  Q_OBJECT
 public:
  explicit MazeController(MazeModel* model);
  ~MazeController();

  void loadMaze(const QString& path);
  void saveMaze(const QString& path);
  void genMaze(int rows, int cols);
  QPainterPath getPath(QPoint start, QPoint finish);
  auto& getRightBorders() { return model_->getRightBorders(); }
  auto& getBottomBorders() { return model_->getBottomBorders(); }

 signals:
  void ErrorOccurred(const QString& message);

 private:
  MazeModel* model_;
};

}  // namespace s21