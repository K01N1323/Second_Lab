CXX = c++
PYTHON ?= python3
CPPFLAGS = -I.
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
SOURCES = Src/main.cpp
HEADERS = $(wildcard Include/*.hpp BaseStructures/*.hpp Sequences/*.hpp Tests/*.hpp)
TARGET = build/tests/alphabetical_index_tests
SANITIZER_TARGET = build/sanitize/alphabetical_index_tests
RELEASE_TARGET = build/release/alphabetical_index_tests

.PHONY: all run test performance index-performance generate-text sanitize release clean

all: run

run:
	$(PYTHON) gui.py

$(TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@

test: $(TARGET)
	./$(TARGET) --test
	$(PYTHON) Tests/GuiAdapterTests.py

$(RELEASE_TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O2 -DNDEBUG $(SOURCES) -o $@

release: $(RELEASE_TARGET)

performance: $(RELEASE_TARGET)
	./$(RELEASE_TARGET) --performance

index-performance: $(RELEASE_TARGET)
	./$(RELEASE_TARGET) --index-performance

generate-text:
	$(PYTHON) generate_text.py

$(SANITIZER_TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined $(SOURCES) -o $@

sanitize: $(SANITIZER_TARGET)
	./$(SANITIZER_TARGET) --test

clean:
	rm -f $(TARGET) $(SANITIZER_TARGET) $(RELEASE_TARGET)
