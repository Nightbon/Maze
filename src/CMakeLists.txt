cmake_minimum_required(VERSION 3.16)
project(Maze VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets HttpServer Concurrent)

option(Release "Build release" OFF)

if(Release)
    # add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    # add_link_options(-fsanitize=address)
    # 1. Модель
    file(GLOB_RECURSE MODEL_SOURSES CONFIGURE_DEPENDS "model/*")
    list(FILTER MODEL_SOURSES EXCLUDE REGEX ".*test.*") 
    add_library(Model STATIC ${MODEL_SOURSES})
    target_include_directories(Model PUBLIC model)
    target_link_libraries(Model PRIVATE Qt6::Core)

    # 2. Контроллер
    file(GLOB_RECURSE CONTROLLER_SOURSES CONFIGURE_DEPENDS "controller/*")
    add_library(Controller STATIC ${CONTROLLER_SOURSES})
    target_include_directories(Controller PUBLIC controller model)
    target_link_libraries(Controller PRIVATE Model Qt6::Core Qt6::Widgets Qt6::HttpServer)

    # 3. Вью
    file(GLOB_RECURSE VIEW_SOURSES CONFIGURE_DEPENDS "view/*")
    add_library(View STATIC ${VIEW_SOURSES})
    target_include_directories(View PUBLIC view)
    target_link_libraries(View PRIVATE Controller Qt6::Core Qt6::Widgets)

    # 4. Исполняемый файл
    add_executable(Maze main.cpp)
    target_link_libraries(Maze PRIVATE View Controller Model Qt6::Core Qt6::Widgets)

    add_executable(Maze_web
        web_main.cpp
        resources.qrc
    )

    # Линкуем модули
    target_link_libraries(Maze_web PRIVATE Model Controller Qt6::Core Qt6::HttpServer Qt6::Concurrent)
endif()

# ================ ТЕСТЫ ========================
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    # 1. Модель
    file(GLOB_RECURSE MODEL_SOURSES CONFIGURE_DEPENDS "model/*")
    list(FILTER MODEL_SOURSES EXCLUDE REGEX ".*test.*")  
    add_library(Model STATIC ${MODEL_SOURSES})
    target_include_directories(Model PUBLIC model)
    target_link_libraries(Model PRIVATE Qt6::Core)

    add_executable(run_tests model/tests/maze_model_test.cpp model/tests/cave_model_test.cpp model/tests/qlearning_model_test.cpp)
    target_link_libraries(run_tests PRIVATE GTest::gtest GTest::gtest_main GTest::gmock Model)
    enable_testing()
    add_test(NAME AllTests COMMAND run_tests)
endif()

# ================ Документация ========================
option(BUILD_DVI "Build docs" OFF)
if(BUILD_DVI)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME "Maze")
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX NO)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_OUTPUT_LANGUAGE Russian)
        set(DOXYGEN_GENERATE_TREEVIEW YES)
        set(DOXYGEN_CALL_GRAPH YES)
        set(DOXYGEN_HAVE_DOT YES)
        set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
        set(DOXYGEN_SOURCE_BROWSER YES)
        set(DOXYGEN_QUIET YES)
        set(DOXYGEN_EXCLUDE_PATTERNS "*test*")
        include(FindDoxygen)
        doxygen_add_docs(doxygen ${CMAKE_CURRENT_SOURCE_DIR} ALL WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMENT "Generating API documentation with Doxygen" )
    endif()
endif()
