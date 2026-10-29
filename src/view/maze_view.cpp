#include "maze_view.h"

#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QSpinBox>
#include <QVBoxLayout>
#include <iostream>
#include <ranges>

namespace s21 {

MainWindow::MainWindow(MazeController* maze_controller,
                       CaveController* cave_controller,
                       QLearningController* ql_controller, QWidget* parent)
    : maze_controller_(maze_controller),
      cave_controller_(cave_controller),
      ql_controller_(ql_controller),
      QMainWindow(parent) {
  setWindowTitle("Maze");
  SetupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::SetupUi() {
  auto* central = new QWidget(this);
  setCentralWidget(central);
  resize(800, 520);
  auto* mainLayout = new QHBoxLayout(central);

  auto* sidebarWidget = new QFrame();
  sidebarWidget->setFrameShape(QFrame::StyledPanel);
  sidebarWidget->setFixedWidth(250);
  auto* sidebarLayout = new QVBoxLayout(sidebarWidget);

  auto* loadButton = new QPushButton("Load");
  sidebarLayout->addWidget(loadButton);
  sidebarLayout->addSpacing(20);

  auto* gridForm = new QGridLayout();
  rows_spinbox_ = new QSpinBox();
  rows_spinbox_->setRange(1, 50);
  rows_spinbox_->setValue(20);
  cols_spinbox_ = new QSpinBox();
  cols_spinbox_->setRange(1, 50);
  cols_spinbox_->setValue(20);
  gridForm->addWidget(new QLabel("Rows:", sidebarWidget), 0, 0);
  gridForm->addWidget(rows_spinbox_, 0, 1);
  gridForm->addWidget(new QLabel("Cols:", sidebarWidget), 1, 0);
  gridForm->addWidget(cols_spinbox_, 1, 1);
  sidebarLayout->addLayout(gridForm);

  auto* generateButton = new QPushButton("Generate maze");
  sidebarLayout->addWidget(generateButton);

  saveButton_ = new QPushButton("Save");
  saveButton_->setEnabled(false);
  sidebarLayout->addWidget(saveButton_);

  sidebarLayout->addStretch();
  caveButton_ = new QPushButton("Cave");
  sidebarLayout->addWidget(caveButton_);
  rlButton_ = new QPushButton("RL Agent");
  sidebarLayout->addWidget(rlButton_);
  mainLayout->addStretch();
  mainLayout->addWidget(sidebarWidget);
  central->setFixedSize(800, 520);

  QObject::connect(loadButton, &QPushButton::clicked, this,
                   &MainWindow::onLoadPush);
  QObject::connect(generateButton, &QPushButton::clicked, this,
                   &MainWindow::onGenPush);
  QObject::connect(saveButton_, &QPushButton::clicked, this,
                   &MainWindow::onSavePush);
  QObject::connect(caveButton_, &QPushButton::clicked, this,
                   &MainWindow::onCavePush);
  QObject::connect(rlButton_, &QPushButton::clicked, this,
                   &MainWindow::onRLPush);
  QObject::connect(maze_controller_, &MazeController::ErrorOccurred, this,
                   &MainWindow::OnError);
}

void MainWindow::onLoadPush() {
  QString path = QFileDialog::getOpenFileName(this, "загрузка лабиринта", "",
                                              "Текстовые файлы (*.txt)",
                                              nullptr, QFileDialog::ReadOnly);
  if (!path.isEmpty() && maze_controller_) {
    maze_controller_->loadMaze(path);
  }
  saveButton_->setEnabled(true);
  update();
}

void MainWindow::onSavePush() {
  QString path =
      QFileDialog::getSaveFileName(this, "сохранение лабиринта", "mazeTest.txt",
                                   "Текстовые файлы (*.txt)", nullptr);
  if (!path.isEmpty() && maze_controller_) {
    maze_controller_->saveMaze(path);
  }
}

void MainWindow::onGenPush() {
  maze_controller_->genMaze(rows_spinbox_->value(), cols_spinbox_->value());
  has_start_ = false;
  has_finish_ = false;
  saveButton_->setEnabled(true);
  update();
}

void MainWindow::onCavePush() {
  QPointer<CaveWindow> cave_window = nullptr;
  cave_window = new CaveWindow(cave_controller_, caveButton_, this);
  cave_window->setAttribute(Qt::WA_DeleteOnClose);

  cave_window->show();
  cave_window->raise();
  cave_window->activateWindow();
  caveButton_->setEnabled(false);
}

void MainWindow::onRLPush() {
  QPointer<QLearningWindow> rl_window = nullptr;
  rl_window = new QLearningWindow(ql_controller_, rlButton_, this);
  rl_window->setAttribute(Qt::WA_DeleteOnClose);

  rl_window->show();
  rl_window->raise();
  rl_window->activateWindow();
  rlButton_->setEnabled(false);
}

void MainWindow::OnError(const QString& message) {
  QMessageBox::critical(this, "Error", message);
}

void MainWindow::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setPen({Qt::black, 2});

  const QRect gridRect(offset_, offset_, sizeConvas_, sizeConvas_);
  painter.drawRect(gridRect);

  if (maze_controller_->getRightBorders().empty() ||
      maze_controller_->getBottomBorders().empty())
    return;

  drawMaze(painter);

  if (!has_start_) return;

  auto pixToCell = [&](QPoint pos) {
    int x = pos.x() - ((pos.x() - offset_) % cellWidth_);
    int y = pos.y() - ((pos.y() - offset_) % cellHeight_);
    return QPoint((x - offset_) / cellWidth_, (y - offset_) / cellHeight_);
  };

  auto startIdx = pixToCell(start_);
  drawCell(painter, startIdx, Qt::red);

  if (!has_finish_) return;

  auto finishIdx = pixToCell(finish_);
  drawCell(painter, finishIdx, Qt::green);

  auto pathIndexes = maze_controller_->getPath(startIdx, finishIdx);

  if (pathIndexes.isEmpty()) return;

  drawPath(painter, pathIndexes);
}

void MainWindow::drawMaze(QPainter& painter) {
  const auto& rightBorders = maze_controller_->getRightBorders();
  const auto& downBorders = maze_controller_->getBottomBorders();

  const size_t rows = rightBorders.size();
  const size_t cols = rows ? rightBorders[0].size() : 0;
  if (!rows || !cols) return;
  cellHeight_ = sizeConvas_ / rows;
  cellWidth_ = sizeConvas_ / cols;

  for (size_t i : std::views::iota(0u, rows)) {
    for (size_t j : std::views::iota(0u, cols)) {
      const int xLeft = offset_ + j * cellWidth_;
      const int xRight = xLeft + cellWidth_;
      const int yTop = offset_ + i * cellHeight_;
      const int yBottom = yTop + cellHeight_;

      if (downBorders[i][j] == 1) {
        painter.drawLine(xLeft, yBottom, xRight, yBottom);
      }
      if (rightBorders[i][j] == 1) {
        painter.drawLine(xRight, yTop, xRight, yBottom);
      }
    }
  }
}

void MainWindow::drawCell(QPainter& painter, QPoint pos, QColor color) {
  int posLeftUpCorn_x = pos.x() * cellWidth_ + offset_;
  int posLeftUpCorn_y = pos.y() * cellHeight_ + offset_;
  painter.fillRect(posLeftUpCorn_x, posLeftUpCorn_y, cellWidth_, cellHeight_,
                   color);
}

void MainWindow::drawPath(QPainter& painter, QPainterPath cellPath) {
  QTransform transform;
  transform.translate(offset_, offset_);
  transform.scale(cellWidth_, cellHeight_);
  transform.translate(0.5, 0.5);

  painter.setPen({Qt::red, 1});
  painter.drawPath(transform.map(cellPath));
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
  if (maze_controller_->getRightBorders().empty()) return;
  bool leftBotton = event->buttons() == Qt::LeftButton;
  bool rightBotton = event->buttons() == Qt::RightButton;
  if (!leftBotton && !rightBotton) return;

  if (event->pos().x() <= offset_ ||
      event->pos().x() >= sizeConvas_ + offset_ ||
      event->pos().y() <= offset_ || event->pos().y() >= sizeConvas_ + offset_)
    return;

  if (leftBotton && !has_start_) {
    start_ = event->pos();
    has_start_ = true;
  } else if (leftBotton && has_start_) {
    finish_ = event->pos();
    has_finish_ = true;
  } else if (rightBotton && has_start_) {
    has_start_ = false;
    has_finish_ = false;
  }

  update();
}

}  // namespace s21
