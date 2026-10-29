#pragma once
#include <QHttpServer>
#include <QObject>

#include "maze_model.h"

namespace s21 {

class WebController : public QObject {
  Q_OBJECT
 public:
  explicit WebController(MazeModel* model, QObject* parent = nullptr);
  bool listen(quint16 port = 8080);

 private:
  QHttpServerResponse sendMazeResponse() const;
  void initRoutes();

  QHttpServer server_;
  MazeModel* model_;
};

}  // namespace s21
