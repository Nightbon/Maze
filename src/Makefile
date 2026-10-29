.PHONY: all clean dist style test gcov_report check valgrind cppcheck install uninstall dvi

BUILD_DIR = ../build
PREFIX ?= ../build
INSTALL_DIR = $(PREFIX)/Maze

all: build

build:
	mkdir -p $(BUILD_DIR) && cmake -S . -B $(BUILD_DIR) && cmake $(BUILD_DIR) -DBUILD_TESTING=OFF -DBUILD_DVI=OFF -DRelease=ON && cd $(BUILD_DIR) && make && ./Maze

test:
	mkdir -p $(BUILD_DIR) && cmake -S . -B $(BUILD_DIR) && cmake $(BUILD_DIR) -DBUILD_TESTING=ON -DBUILD_DVI=OFF -DRelease=OFF && cd $(BUILD_DIR) && make && ./run_tests --gtest_color=yes

gcov_report:
	mkdir -p $(BUILD_DIR)/report
	cmake -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON -DBUILD_DVI=OFF -DRelease=OFF -DCMAKE_CXX_FLAGS="--coverage -g" -DCMAKE_EXE_LINKER_FLAGS="--coverage" 
	cmake --build $(BUILD_DIR)
	./$(BUILD_DIR)/run_tests 
	gcovr -r . --gcov-object-directory $(BUILD_DIR)/CMakeFiles/Model.dir/model --html --html-details -o $(BUILD_DIR)/report/index.html --print-summary --exclude ".*test.*" --exclude-throw-branches --exclude-unreachable-branches

clean:
	rm -rf $(BUILD_DIR)
	rm -rf ../dist

install: all
	@if [ ! -d $(INSTALL_DIR) ]; then mkdir -p $(INSTALL_DIR); fi
	install -d $(INSTALL_DIR)
	install -m 755 $(BUILD_DIR)/Maze $(INSTALL_DIR)
	@if [ -d ./agents ]; then cp -r ./agents $(INSTALL_DIR)/; fi
	@echo "Installed to $(INSTALL_DIR)"

uninstall:
	rm -f $(INSTALL_DIR)/Maze
	@echo "Uninstalled from $(INSTALL_DIR)"

dvi:
	mkdir -p $(BUILD_DIR) && cmake -S . -B $(BUILD_DIR) && cmake $(BUILD_DIR) -DBUILD_DVI=ON -DRelease=OFF -DBUILD_TESTING=OFF && cd $(BUILD_DIR) && make doxygen
	@echo "Documentation ready at $(BUILD_DIR)/docs/html/index.html"

dist:
	mkdir -p ../dist
	cp -r ../src ../dist/
	cd ../dist && tar -czf ../dist/Maze.tar.gz src
	rm -rf ../dist/src

style:
	find . \( -name "*.cpp" -o -name "*.h" \) | xargs clang-format -n --style=Google

cppcheck:
	cppcheck --library=qt --enable=all --suppress=missingIncludeSystem --suppress=missingInclude --suppress=unusedFunction --suppress=unusedStructMember ../src/model ../src/controller ../src/view ../src/main.cc

valgrind: test
	cd $(BUILD_DIR) && valgrind --leak-check=full ./run_tests

check: clean gcov_report style cppcheck valgrind dvi
	@echo "All checks passed!"