#include "test.hpp"

#include "graph.hpp"

namespace {

void test_selectGraphParam() {
    const int M = 1234;
    for(int i = 0; i <= M; ++i) {
        bool called = false;
        selectGraphParam<M>(i, [&](auto sel) {
            const int N = sel.Val;
            checkLessOrEqual(i, N);
            if(i == 0) {
                checkEqual(N, 1);
            } else {
                checkGreater(2 * i, N);
            }
            checkLessOrEqual(i, Graph<N>::B::BitCount);
            called = true;
        });
        checkTrue(called);
    }
}

void test_readGraph() {
    for(int t = 0; t < 100; ++t) {
        int n = UnifInt<int>(0, min(DefaultMaxGraphSize, 64))(rng);
        
        double p = UnifReal<double>(-0.05, 1.05)(rng);
        
        vector<vector<bool>> vals(n);
        for(int i = 0; i < n; ++i) {
            vals[i].resize(n);
            for(int j = 0; j < n; ++j) {
                if(i != j && UnifReal<double>(0.0, 1.0)(rng) < p) {
                    vals[i][j] = true;
                }
            }
        }
        
        stringstream ss;
        ss << n << '\n';
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n; ++j) {
                if(j) {
                    ss << ' ';
                }
                ss << (int)vals[i][j];
            }
            ss << '\n';
        }
        
        ss.seekg(0);
        bool called = false;
        readGraph(ss, [&](auto g) {
            checkEqual(g.size(), n);
            checkLessOrEqual(n, g.ParamN);
            
            for(int i = 0; i < n; ++i) {
                for(int j = 0; j < n; ++j) {
                    checkEqual(g(i, j), (bool)vals[i][j]);
                }
            }
            
            ss.seekg(0);
            Graph<g.ParamN> g2 = Graph<g.ParamN>::read(ss);
            
            checkEqual(g, g2);
            
            called = true;
        });
        checkTrue(called);
        
        ss.seekg(0);
        GraphData graphData = GraphData::read(ss);
        graphData.accessGraph([&](auto g) {
            ss.seekg(0);
            Graph<g.ParamN> g2 = Graph<g.ParamN>::read(ss);
            checkEqual(g, g2);
        });
    }
}

template <int N>
struct TestGraph {
    typedef Graph<N> G;
    typedef typename G::B B;
    
    static G randomGraph() {
        int n = UnifInt<int>(0, N)(rng);
        G g(n);
        
        double p = UnifReal<double>(-0.05, 1.05)(rng);
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n; ++j) {
                if(i != j && UnifReal<double>(0.0, 1.0)(rng) < p) {
                    g.addD(i, j);
                }
            }
        }
        
        return g;
    }
    
    static void testConstructors() {
        {
            G g;
            checkEqual(g.size(), 0);
        }
        
        for(int n = 0; n <= N; ++n) {
            G g(n);
            checkEqual(g.size(), n);
            
            for(int i = 0; i < n; ++i) {
                for(int j = 0; j < n; ++j) {
                    checkFalse(g(i, j));
                }
            }
        }
    }
    
    static void testAccessors() {
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            for(int i = 0; i < g.size(); ++i) {
                for(int j = 0; j < g.size(); ++j) {
                    checkEqual(g(i, j), g.edgesOut(i).has(j));
                    checkEqual(g(j, i), g.edgesOut(j).has(i));
                    checkEqual(g(i, j), g.edgesIn(j).has(i));
                    checkEqual(g(j, i), g.edgesIn(i).has(j));
                }
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            for(int v = 0; v < g.size(); ++v) {
                checkTrue(isSubset(g.edgesIn(v), B::range(g.size())));
                checkTrue(isSubset(g.edgesOut(v), B::range(g.size())));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            G h(g.size());
            
            for(int i = 0; i < g.size(); ++i) {
                for(int j = 0; j < g.size(); ++j) {
                    if(g(i, j)) {
                        h.addD(i, j);
                    }
                }
            }
            
            checkEqual(g, h);
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.addD(x, y);
                
                G b = g;
                b.setD(x, y, true);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.delD(x, y);
                
                G b = g;
                b.setD(x, y, false);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.addU(x, y);
                
                G b = g;
                b.setU(x, y, true);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.delU(x, y);
                
                G b = g;
                b.setU(x, y, false);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.addU(x, y);
                
                G b = g;
                b.addD(x, y);
                b.addD(y, x);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                
                G a = g;
                a.delU(x, y);
                
                G b = g;
                b.delD(x, y);
                b.delD(y, x);
                
                checkEqual(a, b);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            checkEqual(g.vertexSet(), B::range(g.size()));
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            for(int i = 0; i < g.size(); ++i) {
                for(int j = 0; j < g.size(); ++j) {
                    checkEqual(g.neighbors(i).has(j), g(i, j) || g(j, i));
                    checkEqual(g.bidirectionalNeighbors(i).has(j), g(i, j) && g(j, i));
                    checkEqual(g.directedEdgesOut(i).has(j), g(i, j) && !g(j, i));
                    checkEqual(g.directedEdgesIn(i).has(j), g(j, i) && !g(i, j));
                }
            }
        }
    }
    
    static void testComparisons() {
        for(int t = 0; t < 100; ++t) {
            G a = randomGraph();
            G b = a;
            
            checkTrue(a == b);
            checkFalse(a != b);
        }
        
        for(int t = 0; t < 100; ++t) {
            G a = randomGraph();
            G b = randomGraph();
            
            bool eq = a.size() == b.size();
            if(eq) {
                for(int i = 0; i < a.size(); ++i) {
                    for(int j = 0; j < b.size(); ++j) {
                        if(a(i, j) != b(i, j)) {
                            eq = false;
                        }
                    }
                }
            }
            
            checkEqual(a == b, eq);
            checkNotEqual(a != b, eq);
        }
        
        for(int t = 0; t < 100; ++t) {
            G a = randomGraph();
            if(a.size() >= 2) {
                G b = a;
                
                int x = UnifInt<int>(0, a.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, a.size() - 1)(rng);
                }
                
                b.setD(x, y, !b(x, y));
                
                checkFalse(a == b);
                checkTrue(a != b);
            }
        }
    }
    
    static void testQueries() {
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            if(g.size() >= 2) {
                int x = UnifInt<int>(0, g.size() - 1)(rng);
                int y = x;
                while(y == x) {
                    y = UnifInt<int>(0, g.size() - 1)(rng);
                }
                g.addD(x, y);
                g.delD(y, x);
                
                checkFalse(g.isUndirected());
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            for(int x = 0; x < g.size(); ++x) {
                for(int y = x + 1; y < g.size(); ++y) {
                    g.setD(y, x, g(x, y));
                }
            }
            
            checkTrue(g.isUndirected());
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            verts.iterate([&](int a) {
                verts.iterate([&](int b) {
                    if(a < b) {
                        int r = UnifInt<int>(-1, 1)(rng);
                        if(r >= 0) {
                            g.addD(a, b);
                        }
                        if(r <= 0) {
                            g.addD(b, a);
                        }
                    }
                });
            });
            
            checkTrue(g.isClique(verts));
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                
                g.delU(verts.nth(i), verts.nth(j));
                
                checkFalse(g.isClique(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            verts.iterate([&](int a) {
                verts.iterate([&](int b) {
                    if(a < b) {
                        g.delU(a, b);
                    }
                });
            });
            
            checkTrue(g.isIndependentSet(verts));
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                
                g.addD(verts.nth(i), verts.nth(j));
                
                checkFalse(g.isIndependentSet(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            verts.iterate([&](int a) {
                verts.iterate([&](int b) {
                    if(a < b) {
                        g.addU(a, b);
                    }
                });
            });
            
            checkTrue(g.isBidirectionalClique(verts));
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                
                g.delD(verts.nth(i), verts.nth(j));
                
                checkFalse(g.isBidirectionalClique(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            verts.iterate([&](int a) {
                verts.iterate([&](int b) {
                    if(a < b) {
                        int r = UnifInt<int>(-1, 1)(rng);
                        if(r >= 0) {
                            g.delD(a, b);
                        }
                        if(r <= 0) {
                            g.delD(b, a);
                        }
                    }
                });
            });
            
            checkTrue(g.isBidirectionalIndependentSet(verts));
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                
                g.addU(verts.nth(i), verts.nth(j));
                
                checkFalse(g.isBidirectionalIndependentSet(verts));
            }
        }
    }
    
    static void testOperations() {
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            {
                G h = g.inducedSubgraph(verts);
                checkEqual(h.size(), verts.count());
                
                for(int i = 0; i < h.size(); ++i) {
                    for(int j = 0; j < h.size(); ++j) {
                        checkEqual(h(i, j), g(verts.nth(i), verts.nth(j)));
                    }
                }
            }
            
            g.inducedSubgraphReSelectN(verts, [&](const auto& h) {
                checkEqual(h.size(), verts.count());
                
                for(int i = 0; i < h.size(); ++i) {
                    for(int j = 0; j < h.size(); ++j) {
                        checkEqual(h(i, j), g(verts.nth(i), verts.nth(j)));
                    }
                }
            });
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            int cc = UnifInt<int>(1, 5)(rng);
            vector<B> comps(cc);
            
            for(int i = 0; i < g.size(); ++i) {
                comps[UnifInt<int>(0, cc - 1)(rng)].add(i);
            }
            
            for(int i = 0; i < cc; ++i) {
                for(int j = i + 1; j < cc; ++j) {
                    comps[i].iterate([&](int a) {
                        comps[j].iterate([&](int b) {
                            if(rng() & 1) {
                                g.delD(a, b);
                            } else {
                                g.delD(b, a);
                            }
                        });
                    });
                }
            }
            
            int nonEmptyCount = 0;
            for(B comp : comps) {
                if(comp.isNonEmpty()) {
                    ++nonEmptyCount;
                }
            }
            
            B un;
            int count = 0;
            g.iterateBidirectionalComponents([&](B comp) {
                checkTrue(comp.isNonEmpty());
                checkTrue(setIntersection(un, comp).isEmpty());
                un = setUnion(un, comp);
                
                comp.iterate([&](int v) {
                    checkTrue(isSubset(g.bidirectionalNeighbors(v), comp));
                    checkEqual(g.bidirectionalComponent(v), comp);
                });
                
                ++count;
            });
            checkEqual(un, g.vertexSet());
            checkGreaterOrEqual(count, nonEmptyCount);
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            int cc = UnifInt<int>(1, 5)(rng);
            vector<B> comps(cc);
            
            for(int i = 0; i < g.size(); ++i) {
                comps[UnifInt<int>(0, cc - 1)(rng)].add(i);
            }
            
            for(int i = 0; i < cc; ++i) {
                for(int j = i + 1; j < cc; ++j) {
                    comps[i].iterate([&](int a) {
                        comps[j].iterate([&](int b) {
                            if(rng() & 1) {
                                g.delD(a, b);
                            } else {
                                g.delD(b, a);
                            }
                        });
                    });
                }
            }
            
            B verts = setIntersection(B::random(), g.vertexSet());
            
            vector<B> a;
            g.iterateBidirectionalComponents(verts, [&](B comp) {
                a.push_back(comp);
            });
            
            vector<B> b;
            g.inducedSubgraph(verts).iterateBidirectionalComponents([&](B comp) {
                b.push_back(B::unpack(comp, verts));
            });
            
            sort(a.begin(), a.end());
            sort(b.begin(), b.end());
            
            checkEqual(a, b);
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            int cc = UnifInt<int>(1, 5)(rng);
            vector<B> comps(cc);
            
            for(int i = 0; i < g.size(); ++i) {
                comps[UnifInt<int>(0, cc - 1)(rng)].add(i);
            }
            
            for(int i = 0; i < cc; ++i) {
                for(int j = i + 1; j < cc; ++j) {
                    comps[i].iterate([&](int a) {
                        comps[j].iterate([&](int b) {
                            g.delU(a, b);
                        });
                    });
                }
            }
            
            int nonEmptyCount = 0;
            for(B comp : comps) {
                if(comp.isNonEmpty()) {
                    ++nonEmptyCount;
                }
            }
            
            B un;
            int count = 0;
            g.iterateComponents([&](B comp) {
                checkTrue(comp.isNonEmpty());
                checkTrue(setIntersection(un, comp).isEmpty());
                un = setUnion(un, comp);
                
                comp.iterate([&](int v) {
                    checkTrue(isSubset(g.neighbors(v), comp));
                    checkEqual(g.component(v), comp);
                });
                
                ++count;
            });
            checkEqual(un, g.vertexSet());
            checkGreaterOrEqual(count, nonEmptyCount);
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            int cc = UnifInt<int>(1, 5)(rng);
            vector<B> comps(cc);
            
            for(int i = 0; i < g.size(); ++i) {
                comps[UnifInt<int>(0, cc - 1)(rng)].add(i);
            }
            
            for(int i = 0; i < cc; ++i) {
                for(int j = i + 1; j < cc; ++j) {
                    comps[i].iterate([&](int a) {
                        comps[j].iterate([&](int b) {
                            if(rng() & 1) {
                                g.delD(a, b);
                            } else {
                                g.delD(b, a);
                            }
                        });
                    });
                }
            }
            
            B verts = setIntersection(B::random(), g.vertexSet());
            
            vector<B> a;
            g.iterateComponents(verts, [&](B comp) {
                a.push_back(comp);
            });
            
            vector<B> b;
            g.inducedSubgraph(verts).iterateComponents([&](B comp) {
                b.push_back(B::unpack(comp, verts));
            });
            
            sort(a.begin(), a.end());
            sort(b.begin(), b.end());
            
            checkEqual(a, b);
        }
        
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, N)(rng);
            G g(n);
            
            for(int i = 1; i < n; ++i) {
                int j = UnifInt<int>(0, i - 1)(rng);
                g.addU(i, j);
            }
            
            for(int v = 0; v < n; ++v) {
                checkEqual(g.bidirectionalComponent(v), g.vertexSet());
            }
            
            g = g.withShuffledVertices();
            for(int v = 0; v < n; ++v) {
                checkEqual(g.bidirectionalComponent(v), g.vertexSet());
            }
            
        }
        
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, N)(rng);
            G g(n);
            
            for(int i = 1; i < n; ++i) {
                int j = UnifInt<int>(0, i - 1)(rng);
                
                switch(UnifInt<int>(0, 2)(rng)) {
                    case 0:
                        g.addD(i, j);
                        break;
                    
                    case 1:
                        g.addD(j, i);
                        break;
                    
                    case 2:
                        g.addU(i, j);
                        break;
                }
            }
            
            for(int v = 0; v < n; ++v) {
                checkEqual(g.component(v), g.vertexSet());
            }
            
            g = g.withShuffledVertices();
            for(int v = 0; v < n; ++v) {
                checkEqual(g.component(v), g.vertexSet());
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            for(int v = 0; v < g.size(); ++v) {
                B verts = setIntersection(B::random(), g.vertexSet()).with(v);
                B reachable = g.reachableVertices(verts, v);
                checkTrue(isSubset(reachable, verts));
                checkTrue(reachable.has(v));
                reachable.iterate([&](int i) {
                    setIntersection(g.edgesOut(i), verts).iterate([&](int j) {
                        checkTrue(reachable.has(j));
                    });
                    if(i != v) {
                        bool found = false;
                        setIntersection(g.edgesIn(i), verts).iterate([&](int j) {
                            if(reachable.has(j)) {
                                found = true;
                            }
                        });
                        checkTrue(found);
                    }
                });
                int x = setIntersection(verts, B::range(v)).count();
                checkEqual(reachable, B::unpack(g.inducedSubgraph(verts).reachableVertices(x), verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            for(int v = 0; v < g.size(); ++v) {
                B verts = setIntersection(B::random(), g.vertexSet()).with(v);
                B reachable = g.directedReachableVertices(verts, v);
                checkTrue(isSubset(reachable, verts));
                checkTrue(reachable.has(v));
                reachable.iterate([&](int i) {
                    setIntersection(g.directedEdgesOut(i), verts).iterate([&](int j) {
                        checkTrue(reachable.has(j));
                    });
                    if(i != v) {
                        bool found = false;
                        setIntersection(g.directedEdgesIn(i), verts).iterate([&](int j) {
                            if(reachable.has(j)) {
                                found = true;
                            }
                        });
                        checkTrue(found);
                    }
                });
                G subg = g.inducedSubgraph(verts);
                verts.iterate([&](int x) {
                    checkEqual(reachable.has(x), g.isDirectedReachable(verts, v, x));
                    checkEqual(reachable.has(x), subg.isDirectedReachable(setIntersection(B::range(v), verts).count(), setIntersection(B::range(x), verts).count()));
                });
                int x = setIntersection(verts, B::range(v)).count();
                checkEqual(reachable, B::unpack(subg.directedReachableVertices(x), verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            G h = g.withShuffledVertices();
            
            checkEqual(g.size(), h.size());
            int n = g.size();
            
            vector<int> inDegreesG(n);
            vector<int> outDegreesG(n);
            for(int i = 0; i < n; ++i) {
                inDegreesG[i] = g.edgesIn(i).count();
                outDegreesG[i] = g.edgesOut(i).count();
            }
            
            vector<int> inDegreesH(n);
            vector<int> outDegreesH(n);
            for(int i = 0; i < n; ++i) {
                inDegreesH[i] = h.edgesIn(i).count();
                outDegreesH[i] = h.edgesOut(i).count();
            }
            
            sort(inDegreesG.begin(), inDegreesG.end());
            sort(outDegreesG.begin(), outDegreesG.end());
            
            sort(inDegreesH.begin(), inDegreesH.end());
            sort(outDegreesH.begin(), outDegreesH.end());
            
            checkEqual(inDegreesG, inDegreesH);
            checkEqual(outDegreesG, outDegreesH);
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                g.delU(verts.nth(i), verts.nth(j));
                checkFalse(g.isClique(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                g.addD(verts.nth(i), verts.nth(j));
                checkFalse(g.isIndependentSet(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                g.delD(verts.nth(i), verts.nth(j));
                checkFalse(g.isBidirectionalClique(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            B verts = setIntersection(B::random(), g.vertexSet());
            
            int vc = verts.count();
            if(vc >= 2) {
                int i = UnifInt<int>(0, vc - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, vc - 1)(rng);
                }
                g.addU(verts.nth(i), verts.nth(j));
                checkFalse(g.isBidirectionalIndependentSet(verts));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            stringstream ss;
            g.write(ss);
            
            ss.seekg(0);
            
            checkEqual(g, G::read(ss));
        }
    }
    
    static void testChordal() {
        for(int t = 0; t < 500; ++t) {
            G g = randomGraph();
            
            bool isChordal = g.isChordal();
            checkEqual(isChordal, g.withShuffledVertices().isChordal());
            
            if(isChordal) {
                vector<int> order = g.findPerfectEliminationOrderingInChordal();
                checkEqual(order.size(), (size_t)g.size());
                
                {
                    vector<int> cmp(g.size());
                    g.findPerfectEliminationOrderingInChordal(cmp.data());
                    checkEqual(order, cmp);
                }
                
                {
                    bool res;
                    vector<int> cmp;
                    tie(res, cmp) = g.tryFindPerfectEliminationOrdering();
                    checkTrue(res);
                    checkEqual(order, cmp);
                }
                
                {
                    bool res;
                    vector<int> cmp(g.size());
                    res = g.tryFindPerfectEliminationOrdering(cmp.data());
                    checkTrue(res);
                    checkEqual(order, cmp);
                }
                
                {
                    vector<int> sortedOrder = order;
                    sort(sortedOrder.begin(), sortedOrder.end());
                    
                    vector<int> cmp(g.size());
                    for(int i = 0; i < g.size(); ++i) {
                        cmp[i] = i;
                    }
                    
                    checkEqual(sortedOrder, cmp);
                }
                
                B seen;
                for(int i = g.size() - 1; i >= 0; --i) {
                    int v = order[i];
                    checkTrue(g.isClique(setIntersection(g.neighbors(v), seen)));
                    seen.add(v);
                }
            } else {
                checkEqual(g.tryFindPerfectEliminationOrdering(), make_pair(false, vector<int>()));
                
                vector<int> ignore(g.size());
                checkFalse(g.tryFindPerfectEliminationOrdering(ignore.data()));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g = randomGraph();
            
            if(g.size() >= 4) {
                vector<int> perm(g.size());
                for(int i = 0; i < g.size(); ++i) {
                    perm[i] = i;
                }
                shuffle(perm.begin(), perm.end(), rng);
                
                int k = UnifInt<int>(4, g.size())(rng);
                
                for(int i = 0; i < k; ++i) {
                    int j = (i + 1) % k;
                    if(rng() & 1) {
                        g.addD(perm[i], perm[j]);
                    } else {
                        g.addD(perm[j], perm[i]);
                    }
                }
                
                for(int i = 0; i < k; ++i) {
                    for(int j = i + 2; j < k; ++j) {
                        if((j + 1) % k != i) {
                            g.delU(perm[i], perm[j]);
                        }
                    }
                }
                
                checkFalse(g.isChordal());
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            G g;
            do {
                g = randomGraph();
            } while(!g.isChordal());
            
            B un;
            vector<B> cliques;
            g.iterateMaximalCliquesInChordal([&](B clique) {
                checkTrue(g.isClique(clique));
                checkEqual(clique.isEmpty(), g.size() == 0);
                setDifference(g.vertexSet(), clique).iterate([&](int v) {
                    checkFalse(g.isClique(clique.with(v)));
                });
                un = setUnion(un, clique);
                cliques.push_back(clique);
            });
            checkEqual(un, g.vertexSet());
            
            sort(cliques.begin(), cliques.end());
            for(int i = 1; i < (int)cliques.size(); ++i) {
                checkNotEqual(cliques[i - 1], cliques[i]);
            }
            
            if(g.size() <= 8) {
                vector<B> cmp;
                
                for(int c = 1; c < (1 << g.size()); ++c) {
                    B cand = B::fromWord(c);
                    if(g.isClique(cand)) {
                        bool ok = true;
                        setDifference(g.vertexSet(), cand).iterateWhile([&](int v) {
                            if(g.isClique(cand.with(v))) {
                                ok = false;
                                return false;
                            } else {
                                return true;
                            }
                        });
                        if(ok) {
                            cmp.push_back(cand);
                        }
                    }
                }
                
                sort(cmp.begin(), cmp.end());
                checkEqual(cliques, cmp);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, min(N, 30))(rng);
            int e = 0;
            if(n > 0) {
                e = UnifInt<int>(n - 1, n * (n - 1) / 2)(rng);
            }
            
            G g = G::randomConnectedChordal(n, e);
            checkEqual(g.size(), n);
            
            int edgeCount = 0;
            for(int v = 0; v < n; ++v) {
                edgeCount += g.edgesIn(v).count();
            }
            checkEqual(edgeCount, 2 * e);
            
            checkTrue(g.isUndirected());
            checkTrue(g.isChordal());
            if(n > 0) {
                checkEqual(g.component(0), g.vertexSet());
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, min(N, 30))(rng);
            int e = 0;
            if(n > 0) {
                e = UnifInt<int>(n - 1, n * (n - 1) / 2)(rng);
            }
            
            G g = G::randomConnectedChordal(n, e);
            
            auto decomp = g.findTreeDecompositionInChordal();
            int size = decomp.structure.size();
            checkTrue(decomp.structure.isUndirected());
            
            for(int i = 0; i < size; ++i) {
                checkTrue(isSubset(decomp.cliques[i], B::range(n)));
            }
            
            G h(n);
            for(int i = 0; i < size; ++i) {
                decomp.cliques[i].iterate([&](int a) {
                    decomp.cliques[i].iterate([&](int b) {
                        if(a != b) {
                            h.addD(a, b);
                        }
                    });
                });
            }
            checkEqual(g, h);
            
            for(int i = 0; i < size; ++i) {
                for(int j = 0; j < size; ++j) {
                    if(i != j) {
                        checkFalse(isSubset(decomp.cliques[i], decomp.cliques[j]));
                    }
                }
            }
            
            int edgeCount = 0;
            for(int i = 0; i < size; ++i) {
                edgeCount += decomp.structure.edgesOut(i).count();
            }
            checkEqual(edgeCount, n == 0 ? 0 : 2 * (size - 1));
            
            if(n != 0) {
                checkEqual(decomp.structure.component(0), B::range(size));
            }
            
            for(int v = 0; v < n; ++v) {
                B c;
                for(int i = 0; i < size; ++i) {
                    if(decomp.cliques[i].has(v)) {
                        c.add(i);
                    }
                }
                checkTrue(c.isNonEmpty());
                checkEqual(decomp.structure.component(c, c.min()), c);
            }
        }
    }
    
    static void testConversions() {
        for(int t = 0; t < 100; ++t) {
            G graph = randomGraph();
            GraphData graphData(graph);
            graphData.accessGraph<N>([&](auto graph2) {
                checkEqual(graph.size(), graph2.size());
                for(int i = 0; i < graph.size(); ++i) {
                    for(int j = 0; j < graph.size(); ++j) {
                        checkEqual(graph(i, j), graph2(i, j));
                    }
                }
            });
        }
    }
    
    static void testAll() {
        testConstructors();
        testAccessors();
        testComparisons();
        testQueries();
        testOperations();
        testChordal();
        testConversions();
    }
};

}

int main() {
    test_selectGraphParam();
    
    TestGraph<1>::testAll();
    TestGraph<2>::testAll();
    TestGraph<3>::testAll();
    TestGraph<5>::testAll();
    TestGraph<8>::testAll();
    TestGraph<23>::testAll();
    TestGraph<121>::testAll();
    
    test_readGraph();

    return 0;
}
