# DIA-2024-2025-Alternative-Exercise

We as a group (Furkan, Elias, Yetkin) try to:

Implement core API functions (StartQuery(), EndQuery(), MatchDocument(), GetNextAvailRes()) and create a test driver to validate them using sample data files.

Optimize the initial implementation for higher throughput using techniques like multithreading, caching, and data structures, aiming for at least a 20x speedup.

Develop a data-parallel version using a framework like Apache Spark, Flink, or Dask, and compare its performance with previous implementations.

Deliverables include the source code, test results, and reports detailing implementation, optimizations, and performance comparisons.

## Testing Instructions

To test the implementation, follow these steps:

```bash
cd DIA-2024-2025-Alternative-Exercise/
make
./testdriver test_data/small_test.txt
cat result.txt
