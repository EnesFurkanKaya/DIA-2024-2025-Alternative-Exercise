# Start powermetrics in the background and save its PID
sudo powermetrics -i 3000 -o powermetrics_output.txt & 
POWERMETRICS_PID=$!

# Run the test driver
./testdriver test_data/small_test.txt

# Sleep for a while to allow powermetrics to collect data
sleep 5

# Stop powermetrics
sudo kill $POWERMETRICS_PID

# Display the result
cat result.txt