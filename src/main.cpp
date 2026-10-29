#include <QApplication>

#include "maze_view.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  s21::MazeModel model;
  s21::MazeController maze_controller(&model);
  s21::CaveModel cave_model;
  s21::CaveController cave_controller(&cave_model);
  s21::QLearningModel ql_model;
  s21::QLearningController ql_controller(&ql_model);

  s21::MainWindow window(&maze_controller, &cave_controller, &ql_controller);
  window.show();
  return app.exec();
}