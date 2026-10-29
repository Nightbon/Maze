#include "qlearning_view.h"

#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <ranges>

namespace s21 {

QLearningWindow::QLearningWindow(QLearningController* controller,
                                 QPushButton* rlButtonMaze, QWidget* parent)
    : controller_(controller),
      rlButtonMaze_(rlButtonMaze),
      QMainWindow(parent) {
  setWindowTitle("RL Agent");
  setupUi();
}

QLearningWindow::~QLearningWindow() {
  if (train_timer_) train_timer_->stop();
  rlButtonMaze_->setEnabled(true);
}

void QLearningWindow::setupUi() {
  auto* central = new QWidget(this);
  setCentralWidget(central);
  resize(800, 520);

  auto* main_layout = new QHBoxLayout(central);

  auto* sidebar = new QFrame();
  sidebar->setFrameShape(QFrame::StyledPanel);
  sidebar->setFixedWidth(250);
  auto* sidebar_layout = new QVBoxLayout(sidebar);

  // Load button
  auto* load_button = new QPushButton("Load Maze");
  sidebar_layout->addWidget(load_button);

  sidebar_layout->addSpacing(15);

  // Endpoint controls
  auto* endpoint_group = new QFrame();
  endpoint_group->setFrameShape(QFrame::StyledPanel);
  auto* endpoint_layout = new QGridLayout(endpoint_group);

  end_x_spinbox_ = new QSpinBox();
  end_x_spinbox_->setRange(0, 49);
  end_x_spinbox_->setValue(0);
  end_y_spinbox_ = new QSpinBox();
  end_y_spinbox_->setRange(0, 49);
  end_y_spinbox_->setValue(0);

  endpoint_layout->addWidget(new QLabel("End X:"), 0, 0);
  endpoint_layout->addWidget(end_x_spinbox_, 0, 1);
  endpoint_layout->addWidget(new QLabel("End Y:"), 1, 0);
  endpoint_layout->addWidget(end_y_spinbox_, 1, 1);
  sidebar_layout->addWidget(new QLabel("Endpoint:"));
  sidebar_layout->addWidget(endpoint_group);

  sidebar_layout->addSpacing(15);

  // Training parameters
  auto* params_group = new QFrame();
  params_group->setFrameShape(QFrame::StyledPanel);
  auto* params_layout = new QGridLayout(params_group);

  episodes_spinbox_ = new QSpinBox();
  episodes_spinbox_->setRange(1, 1000000);
  episodes_spinbox_->setValue(10000);
  episodes_spinbox_->setSingleStep(1000);

  lr_spinbox_ = new QDoubleSpinBox();
  lr_spinbox_->setRange(0.01, 1.0);
  lr_spinbox_->setSingleStep(0.01);
  lr_spinbox_->setDecimals(2);
  lr_spinbox_->setValue(0.1);

  epsilon_spinbox_ = new QDoubleSpinBox();
  epsilon_spinbox_->setRange(0.0, 1.0);
  epsilon_spinbox_->setSingleStep(0.05);
  epsilon_spinbox_->setDecimals(2);
  epsilon_spinbox_->setValue(0.3);

  params_layout->addWidget(new QLabel("Episodes:"), 0, 0);
  params_layout->addWidget(episodes_spinbox_, 0, 1);
  params_layout->addWidget(new QLabel("Learning rate:"), 1, 0);
  params_layout->addWidget(lr_spinbox_, 1, 1);
  params_layout->addWidget(new QLabel("Epsilon:"), 2, 0);
  params_layout->addWidget(epsilon_spinbox_, 2, 1);
  sidebar_layout->addWidget(new QLabel("Training:"));
  sidebar_layout->addWidget(params_group);

  sidebar_layout->addSpacing(10);

  // Train button
  train_button_ = new QPushButton("Train");
  sidebar_layout->addWidget(train_button_);
  train_button_->setEnabled(false);

  // Progress bar
  progress_bar_ = new QProgressBar();
  progress_bar_->setRange(0, 100);
  progress_bar_->setValue(0);
  sidebar_layout->addWidget(progress_bar_);
  progress_bar_->setEnabled(false);

  sidebar_layout->addSpacing(15);

  sidebar_layout->addStretch();

  main_layout->addStretch();
  main_layout->addWidget(sidebar);
  central->setFixedSize(800, 520);

  // Connections
  QObject::connect(load_button, &QPushButton::clicked, this,
                   &QLearningWindow::onLoadPush);
  QObject::connect(train_button_, &QPushButton::clicked, this,
                   &QLearningWindow::onTrainToggle);
  QObject::connect(controller_, &QLearningController::ErrorOccurred, this,
                   &QLearningWindow::OnError);
  QObject::connect(end_x_spinbox_, &QSpinBox::valueChanged, this,
                   &QLearningWindow::onSpinbox);
  QObject::connect(end_y_spinbox_, &QSpinBox::valueChanged, this,
                   &QLearningWindow::onSpinbox);

  // Training timer
  train_timer_ = new QTimer(this);
  QObject::connect(train_timer_, &QTimer::timeout, this,
                   &QLearningWindow::onTrainTick);
}

void QLearningWindow::onSpinbox() {
  controller_->setEndPoint({end_x_spinbox_->value(), end_y_spinbox_->value()});
  update();
}

void QLearningWindow::onLoadPush() {
  QString path = QFileDialog::getOpenFileName(this, "загрузка лабиринта", "",
                                              "Текстовые файлы (*.txt)",
                                              nullptr, QFileDialog::ReadOnly);
  if (path.isEmpty()) return;
  controller_->loadMaze(path);
  const auto& rightBorders = controller_->getRightBorders();
  const size_t rows = rightBorders.size();
  const size_t cols = rows ? rightBorders[0].size() : 0;
  if (!rows || !cols) return;
  end_x_spinbox_->setRange(0, cols - 1);
  end_y_spinbox_->setRange(0, rows - 1);
  end_x_spinbox_->setValue(0);
  end_y_spinbox_->setValue(0);
  has_start_ = false;
  train_button_->setEnabled(true);
  progress_bar_->setEnabled(true);
  train_button_->setText("Train");
  if (train_timer_->isActive()) {
    train_timer_->stop();
  }
  progress_bar_->setValue(0);
  update();
}

void QLearningWindow::onTrainToggle() {
  if (train_timer_->isActive()) {
    train_timer_->stop();
    train_button_->setText("Train");
    update();
  } else {
    controller_->setEndPoint(
        {end_x_spinbox_->value(), end_y_spinbox_->value()});
    total_episodes_ = episodes_spinbox_->value();
    completed_episodes_ = 0;
    learning_rate_ = lr_spinbox_->value();
    epsilon_ = epsilon_spinbox_->value();
    progress_bar_->setValue(0);
    train_button_->setText("Stop");

    train_timer_->start(0);
  }
}

void QLearningWindow::onTrainTick() {
  int batch = std::min(kEpisodesPerTick, total_episodes_ - completed_episodes_);
  if (batch <= 0) {
    train_timer_->stop();
    train_button_->setText("Train");
    update();
    return;
  }

  controller_->train(batch, learning_rate_, epsilon_);
  completed_episodes_ += batch;

  int progress = (100 * completed_episodes_) / total_episodes_;
  progress_bar_->setValue(progress);

  if (completed_episodes_ >= total_episodes_) {
    train_timer_->stop();
    train_button_->setEnabled(false);
    has_start_ = false;
    update();
  }
}

void QLearningWindow::OnError(const QString& message) {
  QMessageBox::critical(this, "Error", message);
}

void QLearningWindow::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setPen({Qt::black, 2});

  const QRect grid_rect(offset_, offset_, sizeConvas_, sizeConvas_);
  painter.drawRect(grid_rect);

  if (controller_->getRightBorders().empty() ||
      controller_->getBottomBorders().empty())
    return;

  drawMaze(painter);
  drawCell(painter, {end_x_spinbox_->value(), end_y_spinbox_->value()},
           Qt::green);

  if (!has_start_) return;

  drawCell(painter, start_point_, Qt::red);

  if (!controller_->isTrained()) return;

  auto path = controller_->getBestPath({start_point_.x(), start_point_.y()});
  if (path.empty()) {
    has_start_ = false;
    start_point_ = {0, 0};
    return;
  }

  QPainterPath cellPath;
  cellPath.moveTo(path[0].x, path[0].y);
  for (size_t i = 1; i < path.size(); ++i) {
    cellPath.lineTo(path[i].x, path[i].y);
  }

  drawPath(painter, cellPath);
}

void QLearningWindow::drawMaze(QPainter& painter) {
  const auto& rightBorders = controller_->getRightBorders();
  const auto& downBorders = controller_->getBottomBorders();

  const size_t rows = rightBorders.size();
  const size_t cols = rows ? rightBorders[0].size() : 0;
  if (!rows || !cols) return;
  cell_height_ = sizeConvas_ / static_cast<int>(rows);
  cell_width_ = sizeConvas_ / static_cast<int>(cols);

  for (size_t i : std::views::iota(0u, rows)) {
    for (size_t j : std::views::iota(0u, cols)) {
      const int xLeft = offset_ + static_cast<int>(j) * cell_width_;
      const int xRight = xLeft + cell_width_;
      const int yTop = offset_ + static_cast<int>(i) * cell_height_;
      const int yBottom = yTop + cell_height_;

      if (downBorders[i][j] == 1) {
        painter.drawLine(xLeft, yBottom, xRight, yBottom);
      }
      if (rightBorders[i][j] == 1) {
        painter.drawLine(xRight, yTop, xRight, yBottom);
      }
    }
  }
}

void QLearningWindow::drawPath(QPainter& painter, QPainterPath cellPath) {
  QTransform transform;
  transform.translate(offset_, offset_);
  transform.scale(cell_width_, cell_height_);
  transform.translate(0.5, 0.5);

  painter.setPen({Qt::blue, 1});
  painter.drawPath(transform.map(cellPath));
}

void QLearningWindow::drawCell(QPainter& painter, QPoint pos, QColor color) {
  int x = pos.x() * cell_width_ + offset_;
  int y = pos.y() * cell_height_ + offset_;
  painter.fillRect(x, y, cell_width_, cell_height_, color);
}

QPoint QLearningWindow::pixelToCell(QPoint pos) {
  int x = pos.x() - ((pos.x() - offset_) % cell_width_);
  int y = pos.y() - ((pos.y() - offset_) % cell_height_);
  return QPoint((x - offset_) / cell_width_, (y - offset_) / cell_height_);
}

void QLearningWindow::mousePressEvent(QMouseEvent* event) {
  if (controller_->getRightBorders().empty()) return;
  bool leftBotton = event->buttons() == Qt::LeftButton;
  bool rightBotton = event->buttons() == Qt::RightButton;
  if (!leftBotton && !rightBotton) return;

  if (event->pos().x() <= offset_ ||
      event->pos().x() >= sizeConvas_ + offset_ ||
      event->pos().y() <= offset_ || event->pos().y() >= sizeConvas_ + offset_)
    return;

  if (!controller_->isTrained() && leftBotton) {
    auto finish_point = pixelToCell(event->pos());
    end_x_spinbox_->setValue(finish_point.x());
    end_y_spinbox_->setValue(finish_point.y());
    update();
    return;
  }

  if (leftBotton) {
    start_point_ = pixelToCell(event->pos());
    has_start_ = true;
    update();
  } else if (rightBotton) {
    has_start_ = false;
    update();
  }
}

}  // namespace s21
