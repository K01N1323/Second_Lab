CXX = c++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -I.
TEST_SOURCES = Tests/Tests.cpp Tests/PerformanceTests.cpp
TEST_TARGET = container_tests
SANITIZER_TARGET = container_tests_sanitize

.PHONY: all test sanitize clean

all: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

sanitize:
	$(CXX) $(CXXFLAGS) -g -fno-omit-frame-pointer -fsanitize=address,undefined $(TEST_SOURCES) -o $(SANITIZER_TARGET)
	./$(SANITIZER_TARGET)

clean:
	rm -f $(TEST_TARGET) $(SANITIZER_TARGET)
