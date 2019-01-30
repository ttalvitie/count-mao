#include "graph.hpp"

using namespace std;

int main(int, char**) {
    cin.sync_with_stdio(false);
    cin.tie(nullptr);
    
    readGraph(cin, [&](auto graph) {
        graph.toDot(cout);
    });
    
    return 0;
}
