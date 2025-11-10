CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -lncurses

# Source files for main program
SOURCES = main.c ds.c game.c persist.c utils.c visualize.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = guess_animal

# Source files for tests
TEST_SOURCES = tests.c ds.c persist.c utils.c test_globals.c
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)
TEST_EXECUTABLE = run_tests

# Default target: build the main program
all: $(EXECUTABLE)

# Build the main executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile .c files to .o files
%.o: %.c lab5.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build the test executable
tests: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CC) $(TEST_OBJECTS) -o $@ $(LDFLAGS)

# Clean up build artifacts
clean:
	rm -f $(OBJECTS) $(TEST_OBJECTS) $(EXECUTABLE) $(TEST_EXECUTABLE)
	rm -f animals.dat test.dat test2.dat
	rm -f *.o

# Run the main program
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Run the tests
test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

# Run valgrind on the main program
valgrind: $(EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(EXECUTABLE)

# Run valgrind on the tests
valgrind-test: $(TEST_EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_EXECUTABLE)

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the main program (default)"
	@echo "  tests         - Build the test suite"
	@echo "  clean         - Remove all build files"
	@echo "  run           - Build and run the main program"
	@echo "  test          - Build and run the test suite"
	@echo "  valgrind      - Run main program with valgrind"
	@echo "  valgrind-test - Run tests with valgrind"
	@echo "  help          - Show this help message"

# Phony targets (not actual files)
.PHONY: all clean run test valgrind valgrind-test tests help
