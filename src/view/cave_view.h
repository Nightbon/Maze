#pragma once

#include <QDoubleSpinBox>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include "cave_controller.h"

namespace s21 {

class CaveWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit CaveWindow(CaveController* controller, QPushButton* caveButtonMaze,
                      QWidget* parent = nullptr);
  ~CaveWindow();

 protected:
  void paintEvent(QPaintEvent*) override;

 private slots:
  void onInitPush();
  void onLoadPush();
  void onSavePush();
  void onStepPush();
  void onAutoToggle();

 public slots:
  void OnError(const QString& message);

 private:
  void setupUi();
  void drawCave(QPainter& painter);

  CaveController* controller_;

  QSpinBox* rows_spinbox_;
  QSpinBox* cols_spinbox_;
  QSpinBox* birth_spinbox_;
  QSpinBox* death_spinbox_;
  QDoubleSpinBox* chance_spinbox_;
  QSpinBox* interval_spinbox_;
  QPushButton* auto_button_;
  QPushButton* caveButtonMaze_;
  QPushButton* save_button_;
  QPushButton* step_button_;

  static constexpr int sizeConvas_ = 500;
  static constexpr int offset_ = 10;

  QTimer* auto_timer_;
  bool auto_running_ = false;
  int cell_height_;
  int cell_width_;
};

}  // namespace s21
