#pragma once

#include <QObject>
#include <QString>
#include <vector>

#include "qlearning_model.h"

namespace s21 {

class QLearningController : public QObject {
  Q_OBJECT
 public:
  explicit QLearningController(QLearningModel* model);
  ~QLearningController();

  void loadMaze(const QString& path);
  void setEndPoint(const QPoint& finish);
  void train(int episodes, double learning_rate, double epsilon);
  std::vector<Point> getBestPath(const QPoint& start);

  auto& getRightBorders() { return model_->getRightBorders(); }
  auto& getBottomBorders() { return model_->getBottomBorders(); }
  bool isTrained() { return model_->isTrained(); }
  QPoint getEndPoint();

 signals:
  void ErrorOccurred(const QString& message);

 private:
  QLearningModel* model_;
};

}  // namespace s21
