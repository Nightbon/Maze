#include <QCoreApplication>
#include <QDebug>

#include "controller/web_controller.h"
#include "maze_model.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  s21::MazeModel model;
  s21::WebController web_controller(&model);

  if (web_controller.listen(8080)) {
    qDebug()
        << "Режим веб-интерфейса. Сервер Qt запущен на http://localhost:8080";
  } else {
    qCritical() << "Не удалось запустить сервер на порту 8080";
    return -1;
  }

  return app.exec();
}
