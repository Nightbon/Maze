#pragma once
#include <QMainWindow>
#include <QPainterPath>
#include <QPushButton>
#include <QSpinBox>

#include "cave_view.h"
#include "maze_controller.h"
#include "qlearning_view.h"

namespace s21 {

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(MazeController* maze_controller = nullptr,
                      CaveController* cave_controller = nullptr,
                      QLearningController* ql_controller = nullptr,
                      QWidget* parent = nullptr);
  ~MainWindow();

 protected:
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent* event) override;
 private slots:
  void onLoadPush();
  void onGenPush();
  void onSavePush();
  void onCavePush();
  void onRLPush();

 public slots:
  void OnError(const QString& message);

 private:
  void SetupUi();
  void drawMaze(QPainter& painter);
  void drawCell(QPainter& painter, QPoint pos, QColor color);
  void drawPath(QPainter& painter, QPainterPath cellPath);

  MazeController* maze_controller_;
  CaveController* cave_controller_;
  QLearningController* ql_controller_;

  QSpinBox* rows_spinbox_;
  QSpinBox* cols_spinbox_;

  QPushButton* saveButton_;
  QPushButton* caveButton_;
  QPushButton* rlButton_;

  static constexpr int sizeConvas_ = 500;
  static constexpr int offset_ = 10;

  QPoint start_, finish_;
  bool has_start_ = false, has_finish_ = false;
  int cellHeight_, cellWidth_;
};

}  // namespace s21
