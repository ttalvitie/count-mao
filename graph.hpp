#pragma once

#include "bitset.hpp"

static const int DefaultMaxGraphSize = 4096;

template <int N>
class Graph {
public:
    static_assert(N > 0, "The maximum number of elements in Graph must be positive");
    static const int ParamN = N;
    typedef BitSet<bitSetParam(N)> B;
    
    Graph() : size_(0) { }
    Graph(int size)
      : size_(size)
    {
        assert(size >= 0);
        assert(size <= N);
    }
    
    static Graph read(istream& in) {
        auto exceptionMask = in.exceptions();
        in.exceptions(in.failbit | in.badbit);
        
        int n;
        in >> n;
        if(n < 0) {
            fail("Invalid input in Graph::read");
        }
        if(n > N) {
            fail("Graph is too large for Graph<", N, ">");
        }
        
        Graph graph(n);
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n; ++j) {
                int x;
                in >> x;
                if((x != 0 && x != 1) || (i == j && x)) {
                    fail("Invalid input in readGraph");
                }
                if(x) {
                    graph.addD(i, j);
                }
            }
        }
        
        in.exceptions(exceptionMask);
        return graph;
    }
    
    static Graph randomConnectedChordal(int vertCount, int edgeCount) {
        if(vertCount == 0) {
            if(edgeCount != 0) {
                fail("Given numbers of vertices and edges are not compatible");
            }
            return Graph();
        }
        Graph graph(vertCount);
        
        vector<int> connected;
        vector<int> isolated;
        
        int initial = UnifInt<int>(0, vertCount - 1)(rng);
        connected.push_back(initial);
        for(int i = 0; i < vertCount; ++i) {
            if(i != initial) {
                isolated.push_back(i);
            }
        }
        
        int edgesAdded = 0;
        while(!isolated.empty()) {
            int ai = UnifInt<int>(0, connected.size() - 1)(rng);
            int bi = UnifInt<int>(0, isolated.size() - 1)(rng);
            
            int a = connected[ai];
            int b = isolated[bi];
            
            graph.addU(a, b);
            
            swap(isolated[bi], isolated.back());
            isolated.pop_back();
            
            connected.push_back(b);
            ++edgesAdded;
        }
        
        if(edgesAdded > edgeCount) {
            fail("Given numbers of vertices and edges are not compatible");
        }
        
        vector<pair<int, int>> pool;
        for(int i = 0; i < vertCount; ++i) {
            for(int j = i + 1; j < vertCount; ++j) {
                if(!graph(i, j)) {
                    pool.emplace_back(i, j);
                }
            }
        }
        
        while(edgesAdded < edgeCount) {
            bool found = false;
            for(int i = 0; i < (int)pool.size(); ++i) {
                swap(pool[i], pool[UnifInt<int>(i, pool.size() - 1)(rng)]);
                
                int a = pool[i].first;
                int b = pool[i].second;
                
                graph.addU(a, b);
                if(graph.isChordal()) {
                    found = true;
                    swap(pool[i], pool.back());
                    pool.pop_back();
                    ++edgesAdded;
                    break;
                } else {
                    graph.delU(a, b);
                }
            }
            
            if(!found) {
                fail("Given numbers of vertices and edges are not compatible");
            }
        }
        
        return graph;
    }
    
    int size() const {
        return size_;
    }
    
    B vertexSet() const {
        return B::range(size());
    }
    
    bool operator()(int a, int b) const {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        return data_[a].second.has(b);
    }
    
    B edgesIn(int v) const {
        assert(v >= 0 && v < size());
        return data_[v].first;
    }
    
    B edgesOut(int v) const {
        assert(v >= 0 && v < size());
        return data_[v].second;
    }
    
    B directedEdgesIn(int v) const {
        return setDifference(edgesIn(v), edgesOut(v));
    }
    
    B directedEdgesOut(int v) const {
        return setDifference(edgesOut(v), edgesIn(v));
    }
    
    B neighbors(int v) const {
        return setUnion(edgesIn(v), edgesOut(v));
    }
    
    B bidirectionalNeighbors(int v) const {
        return setIntersection(edgesIn(v), edgesOut(v));
    }
    
    bool operator==(const Graph& other) const {
        if(size() != other.size()) {
            return false;
        }
        
        for(int v = 0; v < size(); ++v) {
            if(edgesOut(v) != other.edgesOut(v)) {
                return false;
            }
        }
        
        return true;
    }
    
    bool operator!=(const Graph& other) const {
        return !(*this == other);
    }
    
    void setD(int a, int b, bool val) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        assert(!val || a != b);
        data_[a].second.set(b, val);
        data_[b].first.set(a, val);
    }
    
    void addD(int a, int b) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        assert(a != b);
        data_[a].second.add(b);
        data_[b].first.add(a);
    }
    
    void delD(int a, int b) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        data_[a].second.del(b);
        data_[b].first.del(a);
    }
    
    void setU(int a, int b, bool val) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        assert(!val || a != b);
        setD(a, b, val);
        setD(b, a, val);
    }
    
    void addU(int a, int b) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        assert(a != b);
        addD(a, b);
        addD(b, a);
    }
    
    void delU(int a, int b) {
        assert(a >= 0 && a < size());
        assert(b >= 0 && b < size());
        delD(a, b);
        delD(b, a);
    }
    
    bool isUndirected() const {
        for(int a = 0; a < size(); ++a) {
            for(int b = a + 1; b < size(); ++b) {
                if((*this)(a, b) != (*this)(b, a)) {
                    return false;
                }
            }
        }
        return true;
    }
    
    bool isClique(B verts) const {
        bool ret = true;
        verts.iterateWhile([&](int v) {
            if(isSubset(verts.without(v), neighbors(v))) {
                return true;
            } else {
                ret = false;
                return false;
            }
        });
        return ret;
    }
    
    bool isIndependentSet(B verts) const {
        bool ret = true;
        verts.iterateWhile([&](int v) {
            if(setIntersection(verts, neighbors(v)).isEmpty()) {
                return true;
            } else {
                ret = false;
                return false;
            }
        });
        return ret;
    }
    
    bool isBidirectionalClique(B verts) const {
        bool ret = true;
        verts.iterateWhile([&](int v) {
            if(isSubset(verts.without(v), bidirectionalNeighbors(v))) {
                return true;
            } else {
                ret = false;
                return false;
            }
        });
        return ret;
    }
    
    bool isBidirectionalIndependentSet(B verts) const {
        bool ret = true;
        verts.iterateWhile([&](int v) {
            if(setIntersection(verts, bidirectionalNeighbors(v)).isEmpty()) {
                return true;
            } else {
                ret = false;
                return false;
            }
        });
        return ret;
    }
    
    Graph inducedSubgraph(B verts) const {
        assert(isSubset(verts, vertexSet()));
        Graph ret(verts.count());
        int i = 0;
        verts.iterate([&](int v) {
            ret.data_[i] = make_pair(
                B::pack(data_[v].first, verts),
                B::pack(data_[v].second, verts)
            );
            ++i;
        });
        return ret;
    }
    
    template <typename F>
    void inducedSubgraphReSelectN(B verts, F f) const {
        assert(isSubset(verts, vertexSet()));
        int n = verts.count();
        selectGraphParam(n, [&](auto sel) {
            Graph<sel.Val> ret(n);
            int i = 0;
            verts.iterate([&](int v) {
                ret.data_[i] = pair<typename Graph<sel.Val>::B, typename Graph<sel.Val>::B>(
                    B::pack(data_[v].first, verts),
                    B::pack(data_[v].second, verts)
                );
                ++i;
            });
            f(ret);
        });
    }
    
    B bidirectionalComponent(B verts, int v) const {
        assert(isSubset(verts, vertexSet()));
        assert(verts.has(v));
        
        B comp(v);
        B queue(v);
        
        while(!queue.isEmpty()) {
            int x = queue.min();
            queue.del(x);
            
            B add = setDifference(
                setIntersection(bidirectionalNeighbors(x), verts),
                comp
            );
            comp = setUnion(comp, add);
            queue = setUnion(queue, add);
        }
        
        return comp;
    }
    
    B bidirectionalComponent(int v) const {
        return bidirectionalComponent(vertexSet(), v);
    }
    
    template <typename F>
    void iterateBidirectionalComponents(B verts, F f) const {
        while(verts.isNonEmpty()) {
            int v = verts.min();
            B comp = bidirectionalComponent(verts, v);
            f(comp);
            verts = setDifference(verts, comp);
        }
    }
    
    template <typename F>
    void iterateBidirectionalComponents(F f) const {
        iterateBidirectionalComponents(vertexSet(), f);
    }
    
    B component(B verts, int v) const {
        assert(isSubset(verts, vertexSet()));
        assert(verts.has(v));
        
        B comp(v);
        B queue(v);
        
        while(!queue.isEmpty()) {
            int x = queue.min();
            queue.del(x);
            
            B add = setDifference(
                setIntersection(neighbors(x), verts),
                comp
            );
            comp = setUnion(comp, add);
            queue = setUnion(queue, add);
        }
        
        return comp;
    }
    
    B component(int v) const {
        return component(vertexSet(), v);
    }
    
    template <typename F>
    void iterateComponents(B verts, F f) const {
        while(verts.isNonEmpty()) {
            int v = verts.min();
            B comp = component(verts, v);
            f(comp);
            verts = setDifference(verts, comp);
        }
    }
    
    template <typename F>
    void iterateComponents(F f) const {
        iterateComponents(vertexSet(), f);
    }
    
    B reachableVertices(B verts, int v) const {
        assert(isSubset(verts, vertexSet()));
        assert(verts.has(v));
        
        B comp(v);
        B queue(v);
        
        while(!queue.isEmpty()) {
            int x = queue.min();
            queue.del(x);
            
            B add = setDifference(
                setIntersection(edgesOut(x), verts),
                comp
            );
            comp = setUnion(comp, add);
            queue = setUnion(queue, add);
        }
        
        return comp;
    }
    
    B reachableVertices(int v) const {
        return reachableVertices(vertexSet(), v);
    }
    
    B directedReachableVertices(B verts, int v) const {
        assert(isSubset(verts, vertexSet()));
        assert(verts.has(v));
        
        B comp(v);
        B queue(v);
        
        while(!queue.isEmpty()) {
            int x = queue.min();
            queue.del(x);
            
            B add = setDifference(
                setIntersection(directedEdgesOut(x), verts),
                comp
            );
            comp = setUnion(comp, add);
            queue = setUnion(queue, add);
        }
        
        return comp;
    }
    
    B directedReachableVertices(int v) const {
        return directedReachableVertices(vertexSet(), v);
    }
    
    bool isDirectedReachable(B verts, int a, int b) const {
        assert(isSubset(verts, vertexSet()));
        assert(verts.has(a));
        assert(verts.has(b));
        
        if(a == b) {
            return true;
        }
        
        B comp(a);
        B queue(a);
        
        while(!queue.isEmpty()) {
            int x = queue.min();
            queue.del(x);
            
            B add = setDifference(
                setIntersection(directedEdgesOut(x), verts),
                comp
            );
            if(add.has(b)) {
                return true;
            }
            
            comp = setUnion(comp, add);
            queue = setUnion(queue, add);
        }
        
        return false;
    }
    
    bool isDirectedReachable(int a, int b) const {
        return isDirectedReachable(vertexSet(), a, b);
    }
    
    Graph<N> withShuffledVertices() const {
        const Graph& graph = *this;
        
        vector<int> perm(graph.size());
        for(int i = 0; i < graph.size(); ++i) {
            perm[i] = i;
        }
        shuffle(perm.begin(), perm.end(), rng);
        
        Graph ret(graph.size());
        for(int i = 0; i < graph.size(); ++i) {
            for(int j = 0; j < graph.size(); ++j) {
                if(graph(i, j)) {
                    ret.addD(perm[i], perm[j]);
                }
            }
        }
        
        return ret;
    }
    
    void write(ostream& out) const {
        out << size() << '\n';
        for(int i = 0; i < size(); ++i) {
            for(int j = 0; j < size(); ++j) {
                if(j) {
                    out << ' ';
                }
                out << (int)(*this)(i, j);
            }
            out << '\n';
        }
    }
    
    bool tryFindPerfectEliminationOrdering(int* output) const {
        return tryFindPerfectEliminationOrdering_([&](int i, int v, const B&, const B&) {
            output[i] = v;
        });
    }
    
    pair<bool, vector<int>> tryFindPerfectEliminationOrdering() const {
        vector<int> order(size());
        if(tryFindPerfectEliminationOrdering(order.data())) {
            return make_pair(true, order);
        } else {
            return make_pair(false, vector<int>());
        }
    }
    
    bool isChordal() const {
        return tryFindPerfectEliminationOrdering_([&](int, int, const B&, const B&) { });
    }
    
    void findPerfectEliminationOrderingInChordal(int* output) const {
        if(!tryFindPerfectEliminationOrdering(output)) {
            fail("Could not find perfect elimination ordering: the graph is not chordal");
        }
    }
    
    vector<int> findPerfectEliminationOrderingInChordal() const {
        vector<int> order(size());
        findPerfectEliminationOrderingInChordal(order.data());
        return order;
    }
    
    template <typename F>
    void iterateMaximalCliquesInChordal(F f) const {
        findPerfectEliminationOrderingInChordal_([&](int, int v, const B& neigh, const B& done) {
            B cand = setIntersection(neigh, done).with(v);
            
            bool ok = true;
            setDifference(neigh, cand).iterateWhile([&](int x) {
                if(isSubset(cand, neighbors(x))) {
                    ok = false;
                    return false;
                } else {
                    return true;
                }
            });
            
            if(ok) {
                f(cand);
            }
        });
    }
    
    void toDot(ostream& out) const {
        out << "digraph {\n";
        for(int i = 0; i < size(); ++i) {
            out << "  " << i << "\n";
        }
        for(int i = 0; i < size(); ++i) {
            for(int j = i + 1; j < size(); ++j) {
                if((*this)(i, j)) {
                    if((*this)(j, i)) {
                        out << "  " << i << " -> " << j << " [dir=none]\n";
                    } else {
                        out << "  " << i << " -> " << j << "\n";
                    }
                } else if((*this)(j, i)) {
                    out << "  " << j << " -> " << i << "\n";
                }
            }
        }
        out << "}\n";
    }
    
    void print(ostream& out) const {
        out << "Graph<" << N << ">(" << size() << ") [";
        bool first = true;
        for(int i = 0; i < size(); ++i) {
            for(int j = 0; j < size(); ++j) {
                if((*this)(i, j)) {
                    if((*this)(j, i)) {
                        if(i < j) {
                            if(!first) {
                                out << ", ";
                            }
                            first = false;
                            out << i << " <-> " << j;
                        }
                    } else {
                        if(!first) {
                            out << ", ";
                        }
                        first = false;
                        out << i << " --> " << j;
                    }
                }
            }
        }
        out << "]";
    }
    
    struct TreeDecomposition {
        Graph<N> structure;
        B cliques[N];
    };
    
    TreeDecomposition findTreeDecompositionInChordal() const {
        assert(isChordal());
        
        TreeDecomposition ret;
        int size = 0;
        iterateMaximalCliquesInChordal([&](B clique) {
            assert(size != N);
            ret.cliques[size++] = clique;
        });
        ret.structure = Graph<N>(size);
        
        typedef pair<int, pair<int, int>> E;
        vector<E> edges;
        for(int i = 0; i < size; ++i) {
            B a = ret.cliques[i];
            for(int j = i + 1; j < size; ++j) {
                B b = ret.cliques[j];
                int weight = setIntersection(a, b).count();
                edges.emplace_back(weight, make_pair(i, j));
            }
        }
        
        sort(edges.begin(), edges.end(), [&](E a, E b) { return a.first > b.first; });
        
        struct UnionFind {
            UnionFind(int size) {
                for(int i = 0; i < size; ++i) {
                    parent[i] = i;
                }
            }
            
            int find(int x) {
                if(parent[x] != x) {
                    parent[x] = find(parent[x]);
                }
                return parent[x];
            }
            
            void merge(int x, int y) {
                parent[find(x)] = find(y);
            }
            
            int parent[N];
        };
        
        UnionFind uf(size);
        for(E e : edges) {
            int x = e.second.first;
            int y = e.second.second;
            if(uf.find(x) != uf.find(y)) {
                ret.structure.addU(x, y);
                uf.merge(x, y);
            }
        }
        
        return ret;
    }
    
private:
    int size_;
    pair<B, B> data_[N];
    
    template <typename F>
    bool findEliminationOrdering_(F f) const {
        int P[N] = {};
        B Q[N + 1];
        Q[0] = vertexSet();
        int h = 0;
        
        B done;
        for(int i = size() - 1; i >= 0; --i) {
            while(Q[h].isEmpty()) {
                --h;
            }
            
            int v = Q[h].min();
            Q[h].del(v);
            
            B neigh = neighbors(v);
            if(!f(i, v, neigh, done)) {
                return false;
            }
            
            setDifference(neigh, done).iterate([&](int x) {
                Q[P[x]].del(x);
                ++P[x];
                Q[P[x]].add(x);
            });
            
            ++h;
            done.add(v);
        }
        
        return true;
    }
    
    template <typename F>
    void findPerfectEliminationOrderingInChordal_(F f) const {
        findEliminationOrdering_([&](int i, int v, B neigh, B done) {
            assert(isClique(setIntersection(neigh, done)));
            f(i, v, neigh, done);
            return true;
        });
    }
    
    template <typename F>
    bool tryFindPerfectEliminationOrdering_(F f) const {
        return findEliminationOrdering_([&](int i, int v, B neigh, B done) {
            if(!isClique(setIntersection(neigh, done))) {
                return false;
            }
            f(i, v, neigh, done);
            return true;
        });
    }
    
    template <int N2>
    friend class Graph;
};

template <int M = DefaultMaxGraphSize, typename F>
void selectGraphParam(int n, F f) {
    select2Pow<ceilLog2(M)>(n, f);
}

class GraphData {
public:
    GraphData() : size_(0) { }
    
    template <int N>
    GraphData(const Graph<N>& graph)
        : size_(graph.size())
    {
        data_.resize(size_ * size_);
        int p = 0;
        for(int i = 0; i < size_; ++i) {
            for(int j = 0; j < size_; ++j) {
                data_[p++] = graph(i, j);
            }
        }
    }
    
    static GraphData read(istream& in) {
        auto exceptionMask = in.exceptions();
        in.exceptions(in.failbit | in.badbit);
        
        int n;
        in >> n;
        if(n < 0) {
            fail("Invalid input in GraphData::read");
        }
        
        GraphData ret;
        ret.size_ = n;
        ret.data_.resize(n * n);
        
        int p = 0;
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n; ++j) {
                int x;
                in >> x;
                if((x != 0 && x != 1) || (i == j && x)) {
                    fail("Invalid input in readGraph");
                }
                ret.data_[p++] = (bool)x;
            }
        }
        
        in.exceptions(exceptionMask);
        return ret;
    }
    
    template <int M = DefaultMaxGraphSize, typename F>
    void accessGraph(F f) const {
        selectGraphParam<M>(size_, [&](auto sel) {
            Graph<sel.Val> graph(size_);
            int p = 0;
            for(int i = 0; i < size_; ++i) {
                for(int j = 0; j < size_; ++j) {
                    if(data_[p++]) {
                        graph.addD(i, j);
                    }
                }
            }
            f(graph);
        });
    }
    
private:
    int size_;
    vector<bool> data_;
};

template <int M = DefaultMaxGraphSize, typename F>
void readGraph(istream& in, F f) {
    GraphData graphData = GraphData::read(in);
    graphData.accessGraph<M>(f);
}
