#pragma once

#include <QObject>
#include <QString>

#include "cave_model.h"

namespace s21 {

class CaveController : public QObject {
  Q_OBJECT
 public:
  explicit CaveController(CaveModel* model);
  ~CaveController();

  void initCave(int rows, int cols, double chance);
  void generateStep(int birth_limit, int death_limit);
  void loadCave(const QString& path);
  void saveCave(const QString& path);
  auto& getCave() { return model_->getCave(); }

 signals:
  void ErrorOccurred(const QString& message);

 private:
  CaveModel* model_;
};

}  // namespace s21
