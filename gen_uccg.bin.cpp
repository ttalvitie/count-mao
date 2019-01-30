#include "graph.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    cin.sync_with_stdio(false);
    cin.tie(nullptr);
    
    removeStackLimit();
    
    if(argc != 3) {
        fail("Usage: <vertices> <edges>");
    }
    
    int n = fromString<int>(argv[1]);
    int e = fromString<int>(argv[2]);
    
    if(n < 0 || e < 0) {
        fail("Number of vertices and edges must not be negative");
    }
    
    selectGraphParam(n, [&](auto sel) {
        const int N = sel.Val;
        Graph<N>::randomConnectedChordal(n, e).write(cout);
    });
    
    return 0;
}
