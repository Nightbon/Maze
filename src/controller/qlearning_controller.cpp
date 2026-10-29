#include "qlearning_controller.h"

#include <QPoint>

namespace s21 {

QLearningController::QLearningController(QLearningModel* model)
    : model_(model) {}

QLearningController::~QLearningController() = default;

void QLearningController::loadMaze(const QString& path) {
  try {
    model_->loadMazeFromFile(path.toStdString());
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void QLearningController::setEndPoint(const QPoint& finish) {
  try {
    model_->setEndPoint({finish.x(), finish.y()});
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

QPoint QLearningController::getEndPoint() {
  auto finish = model_->getEndPoint();
  return {finish.x, finish.y};
}

void QLearningController::train(int episodes, double learning_rate,
                                double epsilon) {
  try {
    model_->trainEpisodes(episodes, learning_rate, epsilon);
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

std::vector<Point> QLearningController::getBestPath(const QPoint& start) {
  try {
    return model_->getBestPath({start.x(), start.y()});
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
    return {};
  }
}

}  // namespace s21
