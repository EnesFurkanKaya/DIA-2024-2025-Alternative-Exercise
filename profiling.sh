#!/bin/bash

# Output file for powermetrics
OUTPUT_FILE=$1

# Start powermetrics in the background and save its PID
sudo powermetrics -o "$OUTPUT_FILE" & 
POWERMETRICS_PID=$!

# Run the test driver
./testdriver test_data/small_test.txt

# Sleep for a while to allow powermetrics to collect data
sleep 6

# Stop powermetrics
sudo kill $POWERMETRICS_PID

# Display the result
cat result.txt