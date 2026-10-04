CXX = c++
CPPFLAGS = -I.
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
SOURCES = Src/main.cpp Ui/Menu.cpp
HEADERS = $(wildcard Include/*.hpp BaseStructures/*.hpp Sequences/*.hpp Tests/*.hpp Ui/*.hpp)
TARGET = alphabetical_index
SANITIZER_TARGET = build/sanitize/alphabetical_index
RELEASE_TARGET = build/release/alphabetical_index

.PHONY: all test performance sanitize release clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) Makefile
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(SOURCES) -o $@

test: $(TARGET)
	./$(TARGET) --test

$(RELEASE_TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O2 -DNDEBUG $(SOURCES) -o $@

release: $(RELEASE_TARGET)

performance: $(RELEASE_TARGET)
	./$(RELEASE_TARGET) --performance

$(SANITIZER_TARGET): $(SOURCES) $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=undefined $(SOURCES) -o $@

sanitize: $(SANITIZER_TARGET)
	./$(SANITIZER_TARGET) --test

clean:
	rm -f $(TARGET) $(SANITIZER_TARGET) $(RELEASE_TARGET)
