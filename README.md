# Propagating sudoku solver

This is a very efficient C++ implementation of a propagating sudoku solver.  Can solve batches of 1000 puzzles in under 10 miliseconds.

Some data sets and results are included for testing.

In addition to regular propagation, the hidden single rule is employed to reduce searching.

# Compilation

Run `make` in the project directory.

If you want to compile with debug information run

```bash
g++ -g --std=c++11 sudoku.cc -o sudoku
```
