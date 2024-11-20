make clean
make
sudo perf record -g -F 99 --call-graph dwarf ./testdriver test_data/big_test.txt
sudo perf report --symbol-filter=MatchDocument