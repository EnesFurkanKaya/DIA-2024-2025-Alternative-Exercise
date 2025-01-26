# Exercise 3 - Speed Test and compare all 3 implementations


#######################################################
###################### Libraries ######################

import subprocess
from python.test import test_matching
import re
import os
import matplotlib.pyplot as plt

#######################################################
################### Helper Functions ##################


# Run the given reference implementation ref_core.cpp
def run_reference_core(command):
    try:
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Execution failed with error {e}")

# Run our core solution for Exercise 1
def run_solution_core():
    test_matching(which=0)

# Run our opt_core solution for Exercise 2
def run_solution_opt_core():
    test_matching(which=1)

# Run our apache solution for Exercise 3
def run_solution_dask():
    test_matching(which=2)

# Extract the speed of the implementations from their txt output
def extract_throughput(file_path):

    with open(file_path, 'r') as file:
        for line in file:
            if "Throughput" in line:
                match = re.search(r"Throughput\s*=\s*([\d.]+)", line)
                if match:
                    return float(match.group(1))
    return None

# Visualize and Plot the speed differences - Bar 
def visualize_throughput(results):
    labels: list[str] = list(results.keys())
    values = list(results.values())
    function_names = []
    for label in labels:
        if label == "result.txt":
            function_names.append("reference")
        
        elif label == "results/result_core.txt":
            function_names.append("core")

        elif label == "results/result_opt_core.txt":
            function_names.append("opt_core")

        elif label == "results/result_dask.txt":
            function_names.append("dask")

    # Plot the results
    plt.bar(function_names, values, color=['blue', 'green', 'red', 'purple'])
    plt.ylabel('Throughput (documents/second)')
    plt.title('Throughput Comparison')
    plt.show()

# Visualize and Plot the speed differences - Point
def visualize_throughput_2(results):
    labels = list(results.keys())
    values = list(results.values())
    function_names = []
    for label in labels:
        if label == "result.txt":
            function_names.append("reference")
        
        elif label == "results/result_core.txt":
            function_names.append("core")

        elif label == "results/result_opt_core.txt":
            function_names.append("opt_core")
        
        elif label == "results/result_dask.txt":
            function_names.append("dask")

    plt.figure(figsize=(8, 6))
    plt.scatter(function_names, values, color='blue', s=100)
    plt.ylabel('Throughput (documents/second)')
    plt.title('Throughput Comparison')
    
    # Add annotations for exact throughput values
    for i, value in enumerate(values):
        plt.annotate(f"{value:.1f}", (function_names[i], values[i]), textcoords="offset points", xytext=(0, 10), ha='center')

    plt.show()

#######################################################
#################### Main Functions ###################

result_files = {
        "result.txt": None,
        "results/result_core.txt": None,
        "results/result_opt_core.txt": None,
        "results/result_dask.txt": None
    }

test_data_dir = "test_data"
files = os.listdir(test_data_dir)
if files:
    data_path = os.path.join(test_data_dir, files[0])
else:
    raise FileNotFoundError(f"No files found in /{test_data_dir}. Please include the test data under this directory.")

command = ["./testdriver", data_path]
print("Running reference solution...")
run_reference_core(command) # Comment out this line to prevent long wait for the reference solution
print("Done.")
print("Running ex. 1: core solution...")
run_solution_core()
print("Done.")
print("Running ex. 2: optimized solution...")
run_solution_opt_core()
print("Done.")
print("Running ex. 3: data-parallel solution...")
run_solution_dask()
print("Done.")

for file in result_files:
    result_files[file] = extract_throughput(file)

visualize_throughput(result_files)

visualize_throughput_2(result_files)



