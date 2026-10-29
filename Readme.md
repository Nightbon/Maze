\mainpage Maze v1.0 — Генератор и решатель лабиринтов и пещер

@tableofcontents

@section intro 📌 Описание проекта

**Maze v1.0** — это desktop-приложение на C++20 и Qt6 для генерации, отрисовки и решения идеальных лабиринтов, а также процедурной генерации пещер с помощью клеточных автоматов.

Программа поддерживает:
- **Генерацию идеальных лабиринтов** размеров до 50×50 по алгоритму Эллера
- **Загрузку и сохранение** лабиринтов в текстовом формате (матрицы правых и нижних границ)
- **Поиск кратчайшего пути** между двумя точками методом волновой трассировки (BFS)
- **Интерактивный выбор** старта (ЛКМ) и финиша (ЛКМ) с подсветкой маршрута
- **Процедурную генерацию пещер** клеточным автоматом с параметрами Birth/Death и шагом эволюции
- **Автоматический режим** эволюции пещеры через `QTimer` с настраиваемым интервалом
- **MVC-архитектуру** с чётким разделением Model, Controller и View
- **Полную валидацию входных данных** с информативными исключениями и сигналами об ошибках

@section features ⭐ Ключевые возможности

@subsection features_maze 🌀 Модуль лабиринтов

| Возможность | Описание |
|:---|:---|
| **Генерация (Эллер)** | Линейный по памяти алгоритм с одной строкой состояния и системой непересекающихся множеств |
| **Идеальность** | Гарантированно существует ровно один путь между любой парой ячеек |
| **Размеры** | От 1×1 до 50×50 (с проверкой границ через `std::out_of_range`) |
| **Загрузка** | Текстовый формат: `rows cols` + матрица правых границ + матрица нижних границ |
| **Сохранение** | Аналогичный формат с выравниванием строк и пустой строкой-разделителем |
| **Решение (BFS)** | Волновая трассировка с восстановлением маршрута через матрицу предков |
| **Интерактивность** | ЛКМ — установка старта/финиша, ПКМ — сброс маршрута |

@subsection features_cave 🕳 Модуль пещер

| Параметр | Диапазон | Описание |
|:---|:---|:---|
| **Init chance** | [0.0, 1.0] | Начальная вероятность живой клетки при инициализации |
| **Birth limit** | [0, 7] | Порог рождения: клетка оживает, если соседей больше порога |
| **Death limit** | [0, 7] | Порог смерти: клетка умирает, если соседей меньше порога |
| **Interval** | [1, 10000] мс | Период автоматической эволюции через `QTimer` |
| **Step** | — | Ручное применение одного шага клеточного автомата |
| **Auto** | — | Запуск/остановка таймера непрерывной генерации |

@subsection features_ui 🖥 Пользовательский интерфейс
- **Главное окно `MainWindow`**: canvas 500×500 px, боковая панель с кнопками Load/Save/Generate и спинбоксами Rows/Cols
- **Окно пещер `CaveWindow`**: отдельное окно с расширенной панелью (Init chance, Birth/Death limits, Step, Auto, Interval)
- **Визуализация лабиринта**: отрисовка стен через `QPainter::drawLine`, подсветка старта (красный) и финиша (зелёный)
- **Визуализация маршрута**: красная ломаная через центры ячеек с использованием `QPainterPath` и `QTransform`
- **Визуализация пещеры**: заливка живых клеток чёрным через `QPainter::fillRect`
- **Обработка ошибок**: всплывающие `QMessageBox::critical` при ошибках парсинга или валидации

@section layers 🏗 Архитектура проекта (MVC)

Проект построен по классической схеме **Model–View–Controller**. Каждый слой оформлен как отдельная статическая библиотека в CMake, что обеспечивает жёсткое разделение ответственности и независимость модели от Qt:

| Слой | Библиотека | Описание | Основные компоненты | Паттерны |
|:---|:---|:---|:---|:---|
| **Model** | `Model` | Бизнес-логика: генерация, парсинг, поиск пути | `Model`, `CaveModel`, `Point` | Фасад |
| **Controller** | `Controller` | Тонкий посредник: делегирует Model, ловит исключения, эмитит сигналы | `MazeController`, `CaveController` | Наблюдатель, Фасад |
| **View** | `View` | Qt6-виджеты, отрисовка, ввод пользователя | `MainWindow`, `CaveWindow` | Компоновщик, Команда |
| **Tests** | — | Unit-тесты бизнес-логики (GTest) | `maze_model_test`, `cave_model_test` | — |

@section tech_specs 🔧 Технические детали

@subsection tech_eller 🌀 Алгоритм Эллера
- **Состояние**: одна строка `std::vector<int>`, хранящая ID множества для каждой ячейки
- **Шаг генерации**:
  1. Присвоение уникальных ID ячейкам без множества (`assignUniqueSets`)
  2. Случайное построение правых стен с обязательным разделением разных множеств (`genRightBorders`)
  3. Случайное построение нижних стен с гарантией хотя бы одного прохода вниз для каждого множества (`genBottomBorders`)
  4. Перенос состояния на следующую строку с обнулением ячеек, закрытых снизу (`prepareNextLine`)
- **Последняя строка** (`genLastLine`): объединение всех различных множеств, заполнение нижней границы
- **Гарантия идеальности**: на каждом шаге каждое множество имеет минимум один проход вниз, а в последней строке все множества сливаются
- **Случайность**: `std::random_device` → `std::mt19937` → `std::bernoulli_distribution(0.5)`

@subsection tech_bfs 🧭 Поиск пути (BFS)
- **Алгоритм**: классический поиск в ширину через `std::queue<Point>`
- **Матрица предков**: `std::vector<std::vector<Point>>`, инициализированная `undefined = {-1, -1}`
- **Соседи**: 4-связность (вправо/влево/вверх/вниз) с учётом стен (`right_borders_` и `bottom_borders_`)
- **Восстановление пути**: обратный проход от финиша к старту через матрицу предков
- **Гарантия**: BFS на невзвешенном графе всегда находит кратчайший путь
- **Обработка граничных случаев**: совпадение старта и финиша возвращает вектор из одной точки; отсутствие пути — пустой вектор

@subsection tech_cave 🕳 Клеточный автомат (4-5 rule)
- **Инициализация**: каждая клетка заполняется 1 с вероятностью `chance` (`std::bernoulli_distribution`)
- **Подсчёт соседей**: 8-связное окно 3×3, причём клетки за границей считаются живыми (`++count` для out-of-range)
- **Правила эволюции**:
  - Мёртвая клетка оживает, если `neighbors > birth_limit`
  - Живая клетка умирает, если `neighbors < death_limit`
- **Двойная буферизация**: шаг применяется к копии `new_cave`, затем атомарно заменяет `cave_`

@subsection tech_cpp 🧬 Современный C++20
- **`std::span`**: передача строк матрицы без копирования (`genRightBorders(std::span<int> line, ...)`)
- **`std::views::iota`**: итерирование по индексам вместо классических циклов `for`
- **`std::ranges`**: функциональный стиль (`std::ranges::replace`, `std::ranges::count`, `std::ranges::any_of`, `std::ranges::generate`, `std::ranges::reverse`)
- **`operator<=>`**: автоматическая генерация всех сравнений для `Point` (`auto operator<=>(const Point&) const = default;`)
- **Structured bindings**: `const auto [x, y] = current;` для декомпозиции координат
- **`std::filesystem::path`**: типобезопасная работа с путями (`using Path = std::filesystem::path;`)

@subsection tech_validation 🛡 Валидация и обработка ошибок
- **Проверка размеров**: `rows` и `cols` должны быть в диапазоне [1, 50], иначе `std::out_of_range`
- **Проверка вероятности**: параметр `chance` должен быть в [0.0, 1.0], иначе `std::invalid_argument`
- **Проверка файла**: существование и тип (`is_regular_file`) через `std::filesystem`, иначе `std::runtime_error`
- **Проверка данных**: значения матриц должны быть `0` или `1`, иначе `std::invalid_argument`
- **Перехват в Controller**: все вызовы Model обёрнуты в `try/catch(const std::exception&)` с эмиссией сигнала `ErrorOccurred`
- **Отображение в View**: подписка на `ErrorOccurred` через `QMessageBox::critical`

@section patterns 🧩 Применённые паттерны проектирования

- **MVC (Model–View–Controller):** Чёткое разделение ответственности. Model не зависит от Qt (только STL). View не знает о Model. Controller связывает их через сигналы/слоты и преобразует типы (например, `QPoint` → `Point`, `QString` → `Path`).
- **Facade (Фасад):** `Model` и `CaveModel` инкапсулируют сложность алгоритмов Эллера, BFS и клеточного автомата behind простым API из 4–5 методов.
- **Observer (Наблюдатель):** `MazeController` и `CaveController` эмитят сигналы `ErrorOccurred(const QString&)`, на которые подписываются `MainWindow::OnError` и `CaveWindow::OnError` через механизм `QObject::connect`.
- **Command (Команда):** Каждая кнопка (Load, Save, Generate, Step, Auto, Init) порождает «команду» к Controller через слот (`onLoadPush`, `onGenPush`, `onStepPush` и т.д.), не зная деталей реализации.
- **Strategy (Стратегия):** Единый интерфейс генерации (метод `GenerateMaze` для лабиринтов, `initCave` + `generateStep` для пещер) скрывает различные алгоритмические стратегии под одним API.
- **Adapter (Адаптер):** `MazeController::getPath(QPoint, QPoint)` адаптирует типы Qt (`QPoint`, `QPainterPath`) к доменным типам (`Point`, `std::vector<Point>`) и обратно.

@section build 🛠 Инструкции по сборке

@subsection build_req 📋 Требования
- **C++20** совместимый компилятор (GCC 10+, Clang 12+, MSVC 2019+)
- **Qt6** (Core, Widgets)
- **CMake** 3.16+
- **GTest** (для сборки тестов)
- **Doxygen + Graphviz** (для генерации документации)

@subsection build_commands 🔨 Команды сборки
@code{.bash}
# Сборка и запуск приложения (через Makefile)
cd src && make build

# Или вручную через CMake
mkdir build && cd build
cmake .. -DRelease=ON -DBUILD_TESTING=OFF -DBUILD_DVI=OFF
cmake --build . --parallel
./Maze

# Сборка и запуск тестов
cd src && make test
# или
cmake .. -DBUILD_TESTING=ON -DRelease=OFF
cmake --build . --parallel
ctest --output-on-failure

# Генерация документации (требуется Doxygen + Graphviz)
cd src && make dvi
# или
cmake .. -DBUILD_DVI=ON -DRelease=OFF -DBUILD_TESTING=OFF
cmake --build . --target doxygen

# Отчёт покрытия (gcovr)
cd src && make gcov_report
@endcode

@section file_structure 📁 Структура проекта

@code
src/
├── main.cpp                              # Точка входа, инициализация MVC
├── CMakeLists.txt                        # Конфигурация сборки (Model/Controller/View/Tests/Docs)
├── Makefile                              # Обёртки: build / test / dvi / gcov_report / style / cppcheck / valgrind
├── model/
│   ├── maze_model.h/.cpp                 # Model: алгоритм Эллера, BFS, загрузка/сохранение
│   ├── cave_model.h/.cpp                 # CaveModel: клеточный автомат 4-5
│   └── tests/
│       ├── maze_model_test.cpp           # Unit-тесты лабиринта (GTest)
│       └── cave_model_test.cpp           # Unit-тесты пещеры (GTest)
├── controller/
│   ├── maze_controller.h/.cpp            # MazeController: фасад над Model, сигналы ошибок
│   └── cave_controller.h/.cpp            # CaveController: фасад над CaveModel, сигналы ошибок
└── view/
    ├── maze_view.h/.cpp                  # MainWindow: главное окно, canvas, mouse events
    └── cave_view.h/.cpp                  # CaveWindow: окно пещеры, QTimer для auto-режима
@endcode

@section class_diagram 📊 Полная диаграмма классов

\dot
digraph ClassDiagram {
    rankdir=TB;
    compound=true;
    node [shape=record, style=filled, fontname="Verdana", fontsize=10];
    edge [fontname="Verdana", fontsize=9, arrowhead=onormal];

    subgraph cluster_model {
        label="MODEL LAYER (s21::)"; color=forestgreen; style=bold;

        Point [label="{Point (struct)|+ x : int\l+ y : int\l+ operator\<=\> = default\l}", fillcolor=palegreen];

        Model [label="{Model|+ loadMaze(path)\l+ saveMaze(path)\l+ GenerateMaze(rows, cols)\l+ getPath(start, finish) : vector\<Point\>\l+ getRightBorders() : vector\<vector\<int\>\>\&\l+ getBottomBorders() : vector\<vector\<int\>\>\&\l- genRightBorders(span, i)\l- genBottomBorders(span, i)\l- prepareNextLine(span, i)\l- genLastLine(span)\l- rows_, cols_ : int\l- right_borders_, bottom_borders_ : vector\l}", fillcolor=palegreen];

        CaveModel [label="{CaveModel|+ initCave(rows, cols, chance)\l+ generateStep(birth, death)\l+ loadCave(path)\l+ saveCave(path)\l+ getCave() : vector\<vector\<int\>\>\&\l- countNeighbors(row, col) : int\l- rows_, cols_ : int\l- cave_ : vector\l}", fillcolor=palegreen];

        Model -> Point [arrowhead=vee, label=" returns"];
    }

    subgraph cluster_controller {
        label="CONTROLLER LAYER (s21::, QObject)"; color=blue; style=bold;

        MazeController [label="{MazeController (Facade)|+ loadMaze(QString)\l+ saveMaze(QString)\l+ genMaze(rows, cols)\l+ getPath(QPoint, QPoint) : QPainterPath\l+ getRightBorders()\l+ getBottomBorders()\l- model_ : Model*\l}", fillcolor=lightblue];

        CaveController [label="{CaveController (Facade)|+ initCave(rows, cols, chance)\l+ generateStep(birth, death)\l+ loadCave(QString)\l+ saveCave(QString)\l+ getCave()\l- model_ : CaveModel*\l}", fillcolor=lightblue];

        MazeController -> Model [arrowhead=diamond, label=" owns ptr"];
        CaveController -> CaveModel [arrowhead=diamond, label=" owns ptr"];
    }

    subgraph cluster_view {
        label="VIEW LAYER (s21::, Qt6 Widgets)"; color=purple; style=bold;

        MainWindow [label="{MainWindow (QMainWindow)|+ MainWindow(MazeController*, CaveController*)\l+ OnError(QString)\l# paintEvent / mousePressEvent\l- onLoadPush / onGenPush / onSavePush\l- onCavePush()\l- drawMaze / drawCell / drawPath\l- SetupUi()\l- maze_controller_ : MazeController*\l- cave_controller_ : CaveController*\l- start_, finish_ : QPoint\l- rows_spinbox_, cols_spinbox_\l}", fillcolor=plum1];

        CaveWindow [label="{CaveWindow (QMainWindow)|+ CaveWindow(CaveController*)\l+ OnError(QString)\l# paintEvent\l- onInitPush / onLoadPush / onSavePush\l- onStepPush / onAutoToggle\l- drawCave()\l- setupUi()\l- controller_ : CaveController*\l- auto_timer_ : QTimer*\l- birth_, death_, interval_ : QSpinBox\l- chance_ : QDoubleSpinBox\l}", fillcolor=plum1];

        MainWindow -> CaveWindow [arrowhead=vee, style=dashed, label=" creates"];
    }

    MainWindow -> MazeController [arrowhead=vee, label=" uses"];
    MainWindow -> CaveController [arrowhead=vee, label=" uses"];
    CaveWindow -> CaveController [arrowhead=vee, label=" uses"];

    MazeController -> MainWindow [arrowhead=vee, label=" ErrorOccurred", color=red, fontcolor=red, style=dashed];
    CaveController -> CaveWindow [arrowhead=vee, label=" ErrorOccurred", color=red, fontcolor=red, style=dashed];
}
\enddot

@section changelog 📝 История изменений

| Версия | Дата | Изменения |
|:---|:---|:---|
| **v1.0** | 2025 | Релиз: генерация лабиринта (Эллер), поиск пути (BFS), клеточный автомат пещер, MVC на Qt6, C++20, GTest, Doxygen |

@section authors 👥 Авторы

Проект разработан в рамках образовательной программы **School 21**.