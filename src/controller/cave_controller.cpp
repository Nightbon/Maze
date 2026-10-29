#include "cave_controller.h"

namespace s21 {

CaveController::CaveController(CaveModel* model) : model_(model) {}

CaveController::~CaveController() = default;

void CaveController::initCave(int rows, int cols, double chance) {
  try {
    model_->initCave(rows, cols, chance);
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void CaveController::generateStep(int birth_limit, int death_limit) {
  try {
    model_->generateStep(birth_limit, death_limit);
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void CaveController::loadCave(const QString& path) {
  try {
    model_->loadCave(path.toStdString());
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

void CaveController::saveCave(const QString& path) {
  try {
    model_->saveCave(path.toStdString());
  } catch (const std::exception& e) {
    emit ErrorOccurred(QString::fromStdString(e.what()));
  }
}

}  // namespace s21
