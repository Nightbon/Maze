#include "web_controller.h"

#include <QFile>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrlQuery>

namespace {
auto to_json_array = [](const std::vector<std::vector<int>>& matrix) {
  QJsonArray matrix_arr;
  for (const auto& row : matrix) {
    QJsonArray row_arr;
    std::copy(row.begin(), row.end(), std::back_inserter(row_arr));
    matrix_arr.append(row_arr);
  }
  return matrix_arr;
};
}  // namespace

namespace s21 {

WebController::WebController(MazeModel* model, QObject* parent)
    : QObject(parent), model_(model) {
  initRoutes();
}

bool WebController::listen(quint16 port) {
  return server_.listen(QHostAddress::Any, port) == port;
}

QHttpServerResponse WebController::sendMazeResponse() const {
  const auto& right = model_->getRightBorders();
  const auto& bottom = model_->getBottomBorders();

  int rows = right.size();
  int cols = rows > 0 ? right[0].size() : 0;

  QHttpServerResponse response(
      QJsonObject{{"rows", rows},
                  {"cols", cols},
                  {"right_borders", to_json_array(right)},
                  {"bottom_borders", to_json_array(bottom)}});

  return response;
}

void WebController::initRoutes() {
  server_.route(
      "/", QHttpServerRequest::Method::Get, [](const QHttpServerRequest&) {
        QFile file(":view/view.html");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          return QHttpServerResponse(
              "text/plain; charset=utf-8",
              "Внутренняя ошибка: файл интерфейса не найден",
              QHttpServerResponse::StatusCode::InternalServerError);
        }
        QByteArray html_content = file.readAll();

        return QHttpServerResponse("text/html; charset=utf-8", html_content);
      });

  server_.route("/api/generate", QHttpServerRequest::Method::Get,
                [this](const QHttpServerRequest& request) {
                  QUrlQuery query(request.url().query());
                  int rows = query.hasQueryItem("rows")
                                 ? query.queryItemValue("rows").toInt()
                                 : 10;
                  int cols = query.hasQueryItem("cols")
                                 ? query.queryItemValue("cols").toInt()
                                 : 10;

                  try {
                    model_->GenerateMaze(rows, cols);
                  } catch (...) {
                    return QHttpServerResponse(
                        "Ошибка генерации лабиринта",
                        QHttpServerResponse::StatusCode::BadRequest);
                  }

                  return sendMazeResponse();
                });

  server_.route("/api/path", QHttpServerRequest::Method::Get,
                [this](const QHttpServerRequest& request) {
                  QUrlQuery query(request.url().query());

                  int sx = query.queryItemValue("sx").toInt();
                  int sy = query.queryItemValue("sy").toInt();
                  int fx = query.queryItemValue("fx").toInt();
                  int fy = query.queryItemValue("fy").toInt();

                  std::vector<Point> path;
                  try {
                    path = model_->getPath({sx, sy}, {fx, fy});
                  } catch (...) {
                    return QHttpServerResponse(
                        "Ошибка поиска пути",
                        QHttpServerResponse::StatusCode::BadRequest);
                  }

                  QJsonArray path_arr;
                  for (const auto& pt : path) {
                    QJsonObject pt_obj;
                    pt_obj["x"] = pt.x;
                    pt_obj["y"] = pt.y;
                    path_arr.append(pt_obj);
                  }

                  QJsonObject json_res;
                  json_res["path"] = path_arr;

                  QHttpServerResponse response(json_res);
                  return response;
                });

  server_.route("/api/save", QHttpServerRequest::Method::Get,
                [this](const QHttpServerRequest&) {
                  std::string maze_data;

                  try {
                    maze_data = model_->saveMazeToString();
                  } catch (...) {
                    return QHttpServerResponse(
                        "Ошибка сохранения файла",
                        QHttpServerResponse::StatusCode::InternalServerError);
                  }

                  QHttpServerResponse response(
                      "text/plain", QByteArray::fromStdString(maze_data));
                  response.setHeader("Content-Disposition",
                                     "attachment; filename=\"Maze.txt\"");
                  return response;
                });

  server_.route("/api/load", QHttpServerRequest::Method::Post,
                [this](const QHttpServerRequest& request) {
                  QByteArray file_body = request.body();

                  try {
                    model_->loadMazeFromString(file_body.toStdString());
                  } catch (...) {
                    return QHttpServerResponse(
                        "Ошибка загрузки файла",
                        QHttpServerResponse::StatusCode::BadRequest);
                  }

                  return sendMazeResponse();
                });
}

}  // namespace s21
