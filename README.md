# DIA-2024-2025-Alternative-Exercise

We as a group (Furkan, Elias, Yetkin) try to:

Implement core API functions (StartQuery(), EndQuery(), MatchDocument(), GetNextAvailRes()) and create a test driver to validate them using sample data files.

Optimize the initial implementation for higher throughput using techniques like multithreading, caching, and data structures, aiming for at least a 20x speedup.

Develop a data-parallel version using a framework like Apache Spark, Flink, or Dask, and compare its performance with previous implementations.

Deliverables include the source code, test results, and reports detailing implementation, optimizations, and performance comparisons.

## Contents

Included are files and directories:
  - README                        This document
  - Makefile                      Makefile to compile
  - include/                      Header files
  - implementation/               Cpp files
  - old_implementations/          Contains our attempts and old implementations
  - test_driver/                  Contains the test driver implementation. (Not used)
  - python/                       Contains our python implementations
  - python/test.py                Contains our translated test driver in python
  - test_data/                    Contatins the test data
  - results/                      Created after running the benchmark, with each implementations' runtime
  - result.txt                    Created after running the benchmark, with reference solution's runtime
  - benchmark.py                  Main python file to run the test

## Minimum Requirements

GCC/G++ (Version supporting C++17)

Python 3 (Installed and accessible as python3)

Pip (For dependency installation)

### Testing Instructions

To test the implementation, follow these steps:

```bash
cd DIA-2024-2025-Alternative-Exercise/
make setup
```

Then, setup the python environment for the required libraries:

```bash
make setup_python
```

Make sure to update your python interpreter to the .venv path in your workspace:
".../DIA-2024-2025-Alternative-Exercise/.venv/bin/python"

For Visual Studio Code:

Press `Ctrl+Shift+P`(Windows) or `Cmd+Shift+P`(Mac) to open the command palette.
Search for "Python: Select Interpreter" and click on "Enter interpreter path...". And enter .venv path.

Close and Reopen Visual Studio Code if necessary

Start the speed test using:

```bash
python benchmark.py
```

This will run the given reference solution and all of our implementations, then plot their throughputs as bar and point graphs.

To clean the virtual environment and compiled c files:

```bash
make clean
```
