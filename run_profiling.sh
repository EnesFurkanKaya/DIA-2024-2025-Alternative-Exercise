#!/bin/bash

# Array of files to profile
files=("implementation/core.cpp" "implementation/core_only_structure.cpp" "implementation/core_only_multi_threading.cpp" "implementation/core_only_caching.cpp")
output_files=("powermetrics_output_core.txt" "powermetrics_output_only_structure.txt" "powermetrics_output_only_multi_threading.txt" "powermetrics_output_only_caching.txt")

# Iterate over each file and run profiling.sh
for i in "${!files[@]}"; do
    file="${files[$i]}"
    output_file="${output_files[$i]}"
    echo "Profiling $file"
    cp "$file" implementation/core.cpp
    ./profiling.sh "$output_file"
done
