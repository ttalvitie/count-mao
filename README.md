# Counting Markov Equivalent Directed Acyclic Graphs
The code for the AAAI'19 paper "Counting and Sampling Markov Equivalent Directed Acyclic Graphs" (Topi Talvitie, Mikko Koivisto, University of Helsinki).

## Usage
Compile with `make`.

Generate UCCGs with `./gen_uccg.release`. Arguments: number of vertices and number of edges. Writes UCCG as an adjacency matrix to standard output.

Count the Markov equivalent directed acyclic graphs for a given essential graph using `./count_mao.release`. Argument: the name of the method. Reads the graph as an adjacency matrix from the standard input and writes the count to standard output. Supported methods:
- "Enumeration"
- "He et al. 2015"
- "He et al. 2016"
- "Dynamic Programming"
- "Tree Decomposition DP"
- "Tree Decomposition DP with symmetry reduction"

Generate a visualization of a graph as a DOT file using `./graph_to_dot.release`. Reads the graph as an adjacency matrix from the standard input and writes the DOT file to standard output.

## Example
```
# Count the MAOs of a randomly generated sparse UCCG (100 vertices, 200 edges)
./gen_uccg.release 100 200 | ./count_mao.release "Symmetry Reduction Tree Decomposition DP"

# Count the MAOs of a randomly generated dense UCCG (100 vertices, 800 edges)
./gen_uccg.release 100 800 | time ./count_mao.release "Dynamic Programming"

# Visualize a randomly generated UCCG (requires dot)
./gen_uccg.release 10 20 | ./graph_to_dot.release | dot -Tx11
```
