#include "test.hpp"

#include "mao.hpp"
#include "mao/he_et_al_2016.hpp"

const int N = 32;
typedef Graph<N> G;
typedef typename G::B B;

using namespace he_et_al_2016;

void testGraph(const G& graph) {
    checkTrue(isValidEssentialGraph(graph));
    int n = graph.size();
    
    G graph4(n + 4);
    for(int i = 0; i < n; ++i) {
        for(int j = 0; j < n; ++j) {
            graph4.setD(i, j, graph(i, j));
        }
    }
    for(int i = 0; i < n + 4; ++i) {
        for(int j = max(n, i + 1); j < n + 4; ++j) {
            if(i != j) {
                graph4.addU(i, j);
            }
        }
    }
    
    for(int m1 = 0; m1 <= 4; ++m1) {
        G graphM1 = graph4.inducedSubgraph(B::range(n + m1));
        graphM1 = graphM1.withShuffledVertices();
        Polynomial poly = computeSizeF(graphM1);
        
        for(int m2 = m1; m2 <= 4; ++m2) {
            G graphM2 = graph4.inducedSubgraph(B::range(n + m2));
            graphM2 = graphM2.withShuffledVertices();
            
            checkEqual(
                (Z)(poly(m2 - m1) * FactorialTable<N>::get(m2 - m1)),
                countMAOUsingTreeDecompositionDP(graphM2)
            );
        }
    }
}

int main() {
    testGraph(G(0));
    
    for(int t = 0; t < 100; ++t) {
        int n = UnifInt<int>(0, 12)(rng);
        G graph(n);
        for(int i = 1; i < n; ++i) {
            graph.addU(UnifInt<int>(0, i - 1)(rng), i);
        }
        testGraph(graph);
    }
    
    {
        G graph(5);
        graph.addU(1, 2);
        graph.addU(1, 3);
        graph.addU(1, 4);
        graph.addU(3, 4);
        testGraph(graph);
    }
    
    for(int t = 0; t < 100; ++t) {
        int n = UnifInt<int>(3, 12)(rng);
        G graph(n);
        for(int i = 1; i < n; ++i) {
            graph.addU(UnifInt<int>(0, i - 1)(rng), i);
        }
        while(true) {
            int a = UnifInt<int>(0, n - 1)(rng);
            int b = UnifInt<int>(0, n - 1)(rng);
            if(a == b || graph(a, b)) {
                continue;
            }
            graph.addU(a, b);
            if(isValidEssentialGraph(graph)) {
                break;
            }
            graph.delU(a, b);
        }
        testGraph(graph);
    }
    
    for(int n = 0; n <= 12; n += 2) {
        G graph(n);
        for(int i = 1; i < n; i += 2) {
            graph.addU(i - 1, i);
        }
        testGraph(graph);
    }
    
    for(int t = 0; t < 100; ++t) {
        int n = UnifInt<int>(1, 12)(rng);
        int e = UnifInt<int>(n - 1, n * (n - 1) / 2)(rng);
        G graph = G::randomConnectedChordal(n, e);
        testGraph(graph);
    }
    
    return 0;
}
