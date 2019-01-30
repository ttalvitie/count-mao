#include "mao.hpp"

namespace {

template <int N>
Z countInChordal(const Graph<N>& graph) {
    typedef Graph<N> G;
    typedef typename G::B B;
    
    assert(graph.isUndirected());
    assert(graph.isChordal());
    
    int n = graph.size();
    if(n <= 1) {
        return 1;
    }
    
    int e = 0;
    for(int v = 0; v < n; ++v) {
        e += graph.edgesOut(v).count();
    }
    e /= 2;
    
    if(e == n - 1) {
        return n;
    }
    if(e == n) {
        return 2 * n;
    }
    int a = n * (n - 1) / 2;
    if(e == a - 2) {
        return (n * (n - 1) - 4) * FactorialTable<N>::get(n - 3);
    }
    if(e == a - 1) {
        return FactorialTable<N>::get(n - 2) * (2 * n - 3);
    }
    if(e == a) {
        return FactorialTable<N>::get(n);
    }
    
    Z ret = 0;
    for(int v = 0; v < n; ++v) {
        G newGraph = graph;
        
        B prev(v);
        B cur = graph.neighbors(v);
        
        while(cur.isNonEmpty()) {
            cur.iterate([&](int y) {
                setIntersection(newGraph.bidirectionalNeighbors(y), prev)
                    .iterate([&](int x) {
                        newGraph.delD(y, x);
                    });
            });
            
            while(true) {
                bool changed = false;
                
                cur.iterate([&](int y) {
                    setIntersection(newGraph.bidirectionalNeighbors(y), cur)
                        .iterate([&](int z) {
                            if(setDifference(newGraph.directedEdgesIn(y), newGraph.neighbors(z)).isNonEmpty()) {
                                newGraph.delD(z, y);
                                changed = true;
                            }
                        });
                });
                
                if(!changed) {
                    break;
                }
            }
            
            B next;
            cur.iterate([&](int x) {
                next = setUnion(next, graph.neighbors(x));
            });
            next = setDifference(next, setUnion(prev, cur));
            
            prev = cur;
            cur = next;
        }
        
        Z val = 1;
        newGraph.iterateBidirectionalComponents([&](const auto& comp) {
            val *= countInChordal(newGraph.inducedSubgraph(comp));
        });
        ret += val;
    }
    
    return ret;
}

template <int N>
Z count(const Graph<N>& graph) {
    assert(isValidEssentialGraph(graph));
    Z ret = 1;
    graph.iterateBidirectionalComponents([&](const auto& comp) {
        graph.inducedSubgraphReSelectN(comp, [&](const auto& subgraph) {
            ret *= countInChordal(subgraph);
        });
    });
    return ret;
}

}

Z countMAOUsingHeEtAl2015(const GraphData& graphData) {
    Z ret;
    graphData.accessGraph([&](auto graph) {
        ret = count(graph);
    });
    return ret;
}
