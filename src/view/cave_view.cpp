#include "cave_view.h"

#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <ranges>

namespace s21 {

CaveWindow::CaveWindow(CaveController* controller, QPushButton* caveButtonMaze,
                       QWidget* parent)
    : controller_(controller),
      caveButtonMaze_(caveButtonMaze),
      QMainWindow(parent) {
  setWindowTitle("Cave");
  setupUi();
}

CaveWindow::~CaveWindow() {
  if (auto_timer_) auto_timer_->stop();
  caveButtonMaze_->setEnabled(true);
}

void CaveWindow::setupUi() {
  auto* central = new QWidget(this);
  setCentralWidget(central);
  resize(800, 520);

  auto* main_layout = new QHBoxLayout(central);

  auto* sidebar = new QFrame();
  sidebar->setFrameShape(QFrame::StyledPanel);
  sidebar->setFixedWidth(250);
  auto* sidebar_layout = new QVBoxLayout(sidebar);

  auto* load_button = new QPushButton("Load");
  sidebar_layout->addWidget(load_button);

  sidebar_layout->addSpacing(20);

  auto* dim_grid = new QGridLayout();
  rows_spinbox_ = new QSpinBox();
  rows_spinbox_->setRange(1, 50);
  rows_spinbox_->setValue(20);
  cols_spinbox_ = new QSpinBox();
  cols_spinbox_->setRange(1, 50);
  cols_spinbox_->setValue(20);
  dim_grid->addWidget(new QLabel("Rows:"), 0, 0);
  dim_grid->addWidget(rows_spinbox_, 0, 1);
  dim_grid->addWidget(new QLabel("Cols:"), 1, 0);
  dim_grid->addWidget(cols_spinbox_, 1, 1);
  sidebar_layout->addLayout(dim_grid);

  auto* init_button = new QPushButton("Init cave");
  sidebar_layout->addWidget(init_button);

  save_button_ = new QPushButton("Save");
  sidebar_layout->addWidget(save_button_);
  save_button_->setEnabled(false);

  sidebar_layout->addSpacing(20);

  auto* chance_grid = new QGridLayout();
  chance_spinbox_ = new QDoubleSpinBox();
  chance_spinbox_->setRange(0.0, 1.0);
  chance_spinbox_->setSingleStep(0.01);
  chance_spinbox_->setDecimals(2);
  chance_spinbox_->setValue(0.5);

  chance_grid->addWidget(new QLabel("Init chance:"), 0, 0);
  chance_grid->addWidget(chance_spinbox_, 0, 1);
  sidebar_layout->addLayout(chance_grid);

  auto* limits_group = new QFrame();
  limits_group->setFrameShape(QFrame::StyledPanel);
  auto* limits_layout = new QGridLayout(limits_group);

  birth_spinbox_ = new QSpinBox();
  birth_spinbox_->setRange(0, 7);
  birth_spinbox_->setValue(4);
  death_spinbox_ = new QSpinBox();
  death_spinbox_->setRange(0, 7);
  death_spinbox_->setValue(3);

  limits_layout->addWidget(new QLabel("Birth limit:"), 0, 0);
  limits_layout->addWidget(birth_spinbox_, 0, 1);
  limits_layout->addWidget(new QLabel("Death limit:"), 1, 0);
  limits_layout->addWidget(death_spinbox_, 1, 1);
  sidebar_layout->addWidget(limits_group);

  sidebar_layout->addSpacing(10);

  step_button_ = new QPushButton("Step");
  sidebar_layout->addWidget(step_button_);
  step_button_->setEnabled(false);

  auto_button_ = new QPushButton("Auto");
  sidebar_layout->addWidget(auto_button_);
  auto_button_->setEnabled(false);

  auto* interval_grid = new QGridLayout();
  interval_spinbox_ = new QSpinBox();
  interval_spinbox_->setRange(1, 10000);
  interval_spinbox_->setValue(100);
  interval_spinbox_->setSuffix(" ms");
  interval_grid->addWidget(new QLabel("Interval:"), 0, 0);
  interval_grid->addWidget(interval_spinbox_, 0, 1);
  sidebar_layout->addLayout(interval_grid);

  sidebar_layout->addStretch();

  main_layout->addStretch();
  main_layout->addWidget(sidebar);
  central->setFixedSize(800, 520);

  QObject::connect(init_button, &QPushButton::clicked, this,
                   &CaveWindow::onInitPush);
  QObject::connect(load_button, &QPushButton::clicked, this,
                   &CaveWindow::onLoadPush);
  QObject::connect(save_button_, &QPushButton::clicked, this,
                   &CaveWindow::onSavePush);
  QObject::connect(step_button_, &QPushButton::clicked, this,
                   &CaveWindow::onStepPush);
  QObject::connect(auto_button_, &QPushButton::clicked, this,
                   &CaveWindow::onAutoToggle);
  QObject::connect(controller_, &CaveController::ErrorOccurred, this,
                   &CaveWindow::OnError);

  auto_timer_ = new QTimer(this);
  QObject::connect(auto_timer_, &QTimer::timeout, this, [this]() {
    controller_->generateStep(birth_spinbox_->value(), death_spinbox_->value());
    update();
  });
}

void CaveWindow::onInitPush() {
  controller_->initCave(rows_spinbox_->value(), cols_spinbox_->value(),
                        chance_spinbox_->value());
  if (auto_running_) {
    auto_timer_->stop();
    auto_running_ = false;
    auto_button_->setText("Auto");
  }
  auto_button_->setEnabled(true);
  save_button_->setEnabled(true);
  step_button_->setEnabled(true);
  update();
}

void CaveWindow::onLoadPush() {
  QString path = QFileDialog::getOpenFileName(this, "загрузка пещеры", "",
                                              "Текстовые файлы (*.txt)",
                                              nullptr, QFileDialog::ReadOnly);
  if (path.isEmpty()) return;
  controller_->loadCave(path);
  rows_spinbox_->setValue(controller_->getCave().size());
  if (!controller_->getCave().empty()) {
    cols_spinbox_->setValue(static_cast<int>(controller_->getCave()[0].size()));
  }
  auto_button_->setEnabled(true);
  save_button_->setEnabled(true);
  step_button_->setEnabled(true);

  update();
}

void CaveWindow::onSavePush() {
  QString path =
      QFileDialog::getSaveFileName(this, "сохранение пещеры", "cave.txt",
                                   "Текстовые файлы (*.txt)", nullptr);
  if (!path.isEmpty()) {
    controller_->saveCave(path);
  }
}

void CaveWindow::onStepPush() {
  controller_->generateStep(birth_spinbox_->value(), death_spinbox_->value());
  update();
}

void CaveWindow::onAutoToggle() {
  if (auto_running_) {
    auto_timer_->stop();
    auto_running_ = false;
    auto_button_->setText("Auto");
  } else {
    auto_timer_->start(interval_spinbox_->value());
    auto_running_ = true;
    auto_button_->setText("Stop");
  }
}

void CaveWindow::OnError(const QString& message) {
  QMessageBox::critical(this, "Error", message);
}

void CaveWindow::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setPen({Qt::black, 2});

  const QRect grid_rect(offset_, offset_, sizeConvas_, sizeConvas_);
  painter.drawRect(grid_rect);

  if (controller_->getCave().empty()) return;

  drawCave(painter);
}

void CaveWindow::drawCave(QPainter& painter) {
  const auto& cave = controller_->getCave();
  const size_t rows = cave.size();
  const size_t cols = rows ? cave[0].size() : 0;
  if (!rows || !cols) return;
  cell_height_ = sizeConvas_ / rows;
  cell_width_ = sizeConvas_ / cols;

  painter.setPen(Qt::NoPen);

  for (size_t i : std::views::iota(0u, rows)) {
    for (size_t j : std::views::iota(0u, cols)) {
      if (cave[i][j] == 1) {
        int x = offset_ + j * cell_width_;
        int y = offset_ + i * cell_height_;
        painter.fillRect(x, y, cell_width_, cell_height_, Qt::black);
      }
    }
  }
}

}  // namespace s21
