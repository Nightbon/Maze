#pragma once

#include <QMainWindow>
#include <QPainterPath>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include "qlearning_controller.h"

namespace s21 {

class QLearningWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit QLearningWindow(QLearningController* controller,
                           QPushButton* rlButtonMaze,
                           QWidget* parent = nullptr);
  ~QLearningWindow();

 protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;

 private slots:
  void onLoadPush();
  void onTrainToggle();
  void onTrainTick();
  void onSpinbox();

 public slots:
  void OnError(const QString& message);

 private:
  void setupUi();
  void drawMaze(QPainter& painter);
  void drawPath(QPainter& painter, QPainterPath cellPath);
  void drawCell(QPainter& painter, QPoint pos, QColor color);
  QPoint pixelToCell(QPoint pos);

  QLearningController* controller_;

  QSpinBox* end_x_spinbox_;
  QSpinBox* end_y_spinbox_;
  QSpinBox* episodes_spinbox_;
  QDoubleSpinBox* lr_spinbox_;
  QDoubleSpinBox* epsilon_spinbox_;
  QProgressBar* progress_bar_;
  QPushButton* train_button_;
  QPushButton* rlButtonMaze_;

  QTimer* train_timer_;
  int total_episodes_ = 0;
  int completed_episodes_ = 0;
  double learning_rate_ = 0.1;
  double epsilon_ = 0.3;
  static constexpr int kEpisodesPerTick = 200;
  static constexpr int sizeConvas_ = 500;
  static constexpr int offset_ = 10;

  QPoint start_point_;
  bool has_start_ = false;
  int cell_height_;
  int cell_width_;
};

}  // namespace s21
