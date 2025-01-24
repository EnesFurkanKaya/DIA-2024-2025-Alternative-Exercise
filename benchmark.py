#######################################################
###################### Libraries ######################

import subprocess
from python.test import test_matching
import re
import matplotlib.pyplot as plt
import time

#######################################################
################### Helper Functions ##################


# Run the given reference implementation ref_core.cpp with powermetrics
def run_reference_core(command):
    try:
        with open("powermetrics_reference.log", "w") as log_file:
            powermetrics_process = subprocess.Popen(["sudo", "powermetrics"], stdout=log_file, stderr=subprocess.STDOUT)
            start_time = time.time()
            subprocess.run(command, check=True)
            end_time = time.time()
            powermetrics_process.terminate()
            return end_time - start_time
    except subprocess.CalledProcessError as e:
        print(f"Execution failed with error {e}")
        return None

# Run our core solution for Exercise 1 with powermetrics
def run_solution_core():
    with open("powermetrics_core.log", "w") as log_file:
        powermetrics_process = subprocess.Popen(["sudo", "powermetrics"], stdout=log_file, stderr=subprocess.STDOUT)
        start_time = time.time()
        test_matching(which=0)
        end_time = time.time()
        time.sleep(10)  # Add a small delay to ensure powermetrics captures the data
        powermetrics_process.terminate()
        return end_time - start_time

# Run our opt_core solution for Exercise 2 with powermetrics
def run_solution_opt_core():
    with open("powermetrics_opt_core.log", "w") as log_file:
        powermetrics_process = subprocess.Popen(["sudo", "powermetrics"], stdout=log_file, stderr=subprocess.STDOUT)
        start_time = time.time()
        test_matching(which=1)
        end_time = time.time()
        time.sleep(10)  # Add a small delay to ensure powermetrics captures the data
        powermetrics_process.terminate()
        return end_time - start_time

# Run our apache solution for Exercise 3 with powermetrics
def run_solution_dask():
    with open("powermetrics_dask.log", "w") as log_file:
        powermetrics_process = subprocess.Popen(["sudo", "powermetrics"], stdout=log_file, stderr=subprocess.STDOUT)
        start_time = time.time()
        test_matching(which=2)
        end_time = time.time()
        time.sleep(10)  # Add a small delay to ensure powermetrics captures the data
        powermetrics_process.terminate()
        return end_time - start_time

# Extract the speed of the implementations from their txt output
def extract_throughput(file_path):

    with open(file_path, 'r') as file:
        for line in file:
            if "Throughput" in line:
                match = re.search(r"Throughput\s*=\s*([\d.]+)", line)
                if match:
                    return float(match.group(1))
    return None

# Extract the combined power consumption from the powermetrics logs
def extract_combined_power(file_path):
    with open(file_path, 'r') as file:
        for line in file:
            if "Combined Power" in line:
                match = re.search(r"Combined Power \(CPU \+ GPU \+ ANE\): (\d+) mW", line)
                if match:
                    return int(match.group(1))
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

# Visualize and Plot the combined power consumption
def visualize_combined_power(results):
    labels = list(results.keys())
    values = list(results.values())
    function_names = []
    for label in labels:
        if label == "powermetrics_reference.log":
            function_names.append("reference")
        
        elif label == "powermetrics_core.log":
            function_names.append("core")

        elif label == "powermetrics_opt_core.log":
            function_names.append("opt_core")

        elif label == "powermetrics_dask.log":
            function_names.append("dask")

    # Plot the results
    plt.bar(function_names, values, color=['blue', 'green', 'red', 'purple'])
    plt.ylabel('Combined Power (mW)')
    plt.title('Combined Power Consumption Comparison')
    plt.show()

#######################################################
#################### Main Functions ###################

def main():
    result_files = {
        "result.txt": None,
        "results/result_core.txt": None,
        "results/result_opt_core.txt": None,
        "results/result_dask.txt": None
    }

    power_files = {
        "powermetrics_reference.log": None,
        "powermetrics_core.log": None,
        "powermetrics_opt_core.log": None,
        "powermetrics_dask.log": None
    }

    execution_times = {}

    data_path = "test_data/small_test.txt"
    command = ["./testdriver", data_path]
    print("Running reference solution...")
    reference_time = run_reference_core(command)
    execution_times["reference"] = reference_time
    print(f"Done. Time taken: {reference_time:.2f} seconds.")
    print("Running ex. 1: core solution...")
    core_time = run_solution_core()
    execution_times["core"] = core_time
    print(f"Done. Time taken: {core_time:.2f} seconds.")
    print("Running ex. 2: optimized solution...")
    opt_core_time = run_solution_opt_core()
    execution_times["opt_core"] = opt_core_time
    print(f"Done. Time taken: {opt_core_time:.2f} seconds.")
    print("Running ex. 3: data-parallel solution...")
    dask_time = run_solution_dask()
    execution_times["dask"] = dask_time
    print(f"Done. Time taken: {dask_time:.2f} seconds.")

    for file in result_files:
        result_files[file] = extract_throughput(file)

    for file in power_files:
        power_files[file] = extract_combined_power(file)

    visualize_throughput(result_files)
    visualize_throughput_2(result_files)
    visualize_combined_power(power_files)

    print("\nExecution Times:")
    for key, value in execution_times.items():
        print(f"{key}: {value:.2f} seconds")

if __name__ == "__main__":
    main()
