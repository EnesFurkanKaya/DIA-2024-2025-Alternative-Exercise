# Exercise 3 - Speed Test and compare all 3 implementations


#######################################################
###################### Libraries ######################

import subprocess
from python.test import test_matching
import re
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

# Run our opt_core solution for Exercise 2 - Optimal for bigger data
def run_solution_opt_core():
    test_matching(which=1)

# Run our opt_core solution for Exercise 2 - Optimal for smaller data
def run_solution_opt_core2():
    test_matching(which=2)

# Run our apache solution for Exercise 3
def run_solution_dask():
    test_matching(which=3)

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

        elif label == "results/result_opt_core_v2.txt":
            function_names.append("opt_core v2")

        elif label == "results/result_dask.txt":
            function_names.append("dask")

    # Plot the results
    plt.bar(function_names, values, color=['blue', 'green', 'red', 'orange', 'purple'])
    plt.yscale('log')  # Set y-axis to logarithmic scale
    plt.ylabel('Throughput (documents/second) [log scale]')
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

        elif label == "results/result_opt_core_v2.txt":
            function_names.append("opt_core v2")
        
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
        "results/result_opt_core_v2.txt": None,
        "results/result_dask.txt": None
    }

data_path = "test_data/small_test.txt"
command = ["./testdriver", data_path]
print("Running reference solution...")
run_reference_core(command)
print("Done.")
print("Running ex. 1: core solution...")
run_solution_core()
print("Done.")
print("Running ex. 2: optimized solution...")
run_solution_opt_core()
print("Done.")
print("Running ex. 2: optimized solution vers. 2...")
run_solution_opt_core2()
print("Done.")
print("Running ex. 3: data-parallel solution...")
run_solution_dask()
print("Done.")

for file in result_files:
    result_files[file] = extract_throughput(file)

visualize_throughput(result_files)

visualize_throughput_2(result_files)



