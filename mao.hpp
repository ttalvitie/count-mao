#pragma once

#include "graph.hpp"

template <int N>
bool isDirectedEdgeInVStructure(const Graph<N>& graph, int i, int j) {
    assert(i >= 0 && i < graph.size());
    assert(j >= 0 && j < graph.size());
    assert(i != j);
    
    return setDifference(
        graph.directedEdgesIn(j).without(i),
        graph.neighbors(i)
    ).isNonEmpty();
}

template <int N>
bool isValidEssentialGraph(const Graph<N>& graph) {
    for(int u = 0; u < graph.size(); ++u) {
        bool ret = true;
        graph.bidirectionalNeighbors(u).iterateWhile([&](int v) {
            ret = setDifference(graph.directedEdgesIn(v), graph.neighbors(u)).isEmpty();
            return ret;
        });
        if(!ret) {
            return false;
        }
    }
    
    for(int u = 0; u < graph.size(); ++u) {
        bool ret = true;
        graph.directedEdgesOut(u).iterateWhile([&](int v) {
            ret = false;
            ret = ret || setDifference(graph.directedEdgesIn(v), graph.neighbors(u)).without(u).isNonEmpty();
            ret = ret || setDifference(graph.directedEdgesIn(u), graph.neighbors(v)).isNonEmpty();
            ret = ret || setIntersection(graph.directedEdgesOut(u), graph.directedEdgesIn(v)).isNonEmpty();
            ret = ret || !setIntersection(graph.bidirectionalNeighbors(u), graph.directedEdgesIn(v)).isSingletonOrEmpty();
            return ret;
        });
        if(!ret) {
            return false;
        }
    }
    
    for(int v = 0; v < graph.size(); ++v) {
        if(setIntersection(graph.reachableVertices(v), graph.directedEdgesIn(v)).isNonEmpty()) {
            return false;
        }
    }
    
    bool ret = true;
    graph.iterateBidirectionalComponents([&](const auto& comp) {
        if(ret) {
            Graph subgraph = graph.inducedSubgraph(comp);
            ret = subgraph.isUndirected() && subgraph.isChordal();
        }
    });
    return ret;
}

template <int N, typename F>
void iterateMAO_(
    Graph<N>& graph,
    F& f,
    int i,
    int j
) {
    int n = graph.size();
    
    if(i >= n) {
        f((const Graph<N>&)graph);
        return;
    }
    if(j >= n) {
        iterateMAO_(graph, f, i + 1, i + 2);
        return;
    }
    
    if(graph(i, j) && graph(j, i)) {
        if(
            !isDirectedEdgeInVStructure(graph, i, j) &&
            !graph.isDirectedReachable(j, i)
        ) {
            graph.delD(j, i);
            iterateMAO_(graph, f, i, j + 1);
            graph.addD(j, i);
        }
        if(
            !isDirectedEdgeInVStructure(graph, j, i) &&
            !graph.isDirectedReachable(i, j)
        ) {
            graph.delD(i, j);
            iterateMAO_(graph, f, i, j + 1);
            graph.addD(i, j);
        }
    } else {
        iterateMAO_(graph, f, i, j + 1);
    }
}

template <int N, typename F>
void iterateMAO(Graph<N> graph, F f) {
    assert(isValidEssentialGraph(graph));
    iterateMAO_(graph, f, 0, 1);
}

// mao/enumeration.cpp
Z countMAOUsingEnumeration(const GraphData& graphData);

// mao/he_et_al_2015.cpp
Z countMAOUsingHeEtAl2015(const GraphData& graphData);

// mao/he_et_al_2016.cpp
Z countMAOUsingHeEtAl2016(const GraphData& graphData);

// mao/dynamic_programming.cpp
Z countMAOUsingDynamicProgramming(const GraphData& graphData);

// mao/treedecomp.cpp
Z countMAOUsingTreeDecompositionDP(const GraphData& graphData);

// mao/treedecomp_sym.cpp
Z countMAOUsingTreeDecompositionDPSymmetryReduction(const GraphData& graphData);

static const vector<pair<string, Z (*)(const GraphData&)>> countMAOMethods = {
    {"Enumeration", countMAOUsingEnumeration},
    {"He et al. 2015", countMAOUsingHeEtAl2015},
    {"He et al. 2016", countMAOUsingHeEtAl2016},
    {"Dynamic Programming", countMAOUsingDynamicProgramming},
    {"Tree Decomposition DP", countMAOUsingTreeDecompositionDP},
    {"Tree Decomposition DP with symmetry reduction", countMAOUsingTreeDecompositionDPSymmetryReduction}
};
