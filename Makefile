# Build targets (your implementation targets should go in IMPL_O)
TEST_O=test_driver/test.o 
IMPL_O=implementation/ref_core.o implementation/queries.o implementation/cache.o implementation/distance.o implementation/cwrapper.o

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=-O3 -fPIC -Wall -g -I. -I./include
CXXFLAGS=$(CFLAGS) -std=c++17
LDFLAGS=-lpthread

# The programs that will be built
PROGRAMS = testdriver

# The name of the library that will be built
LIBRARY = core

# Python-related variables
PYTHON = python3
VENV_DIR = .venv
REQUIREMENTS = requirements.txt

# Build all programs
all: $(PROGRAMS)

lib: $(IMPL_O)
	$(CXX) $(CXXFLAGS) -shared -o lib$(LIBRARY).so $(IMPL_O)

testdriver: lib $(TEST_O)
	$(CXX) $(CXXFLAGS) -o testdriver $(TEST_O) ./lib$(LIBRARY).so

# Create Python virtual environment and install dependencies
.PHONY: venv
venv: $(VENV_DIR)/.installed

$(VENV_DIR):
	$(PYTHON) -m venv $(VENV_DIR)
	$(VENV_DIR)/bin/pip install --upgrade pip

$(VENV_DIR)/.installed: $(REQUIREMENTS) | $(VENV_DIR)
	$(VENV_DIR)/bin/pip install -r $(REQUIREMENTS)
	touch $(VENV_DIR)/.installed

# Clean build files
clean:
	rm -f $(PROGRAMS) lib$(LIBRARY).so
	find . -name '*.o' -print | xargs rm -f
	rm -rf $(VENV_DIR)
	rm -f result.txt
	rm -rf results
	find . -name "__pycache__" -exec rm -rf {} +

# Setup for Python environment (no C programs)
.PHONY: setup_python
setup_python: venv

# Build everything (C programs) without setting up Python environment
.PHONY: setup
setup: all