#include "test.hpp"

#include "mao.hpp"

namespace {

template <int N>
struct TestMAOFundamentals {
    typedef Graph<N> G;
    typedef typename G::B B;
    
    static void test_isDirectedEdgeInVStructure() {
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, N)(rng);
            G g(n);
            double p = UnifReal<double>(0.0, 1.0)(rng);
            for(int i = 0; i < n; ++i) {
                for(int j = 0; j < n; ++j) {
                    if(i != j && UnifReal<double>(0.0, 1.0)(rng) < p) {
                        g.addD(i, j);
                    }
                }
            }
            
            if(n >= 2) {
                int i = UnifInt<int>(0, n - 1)(rng);
                int j = i;
                while(j == i) {
                    j = UnifInt<int>(0, n - 1)(rng);
                }
                
                bool cmp = false;
                for(int k = 0; k < n; ++k) {
                    if(k == i || k == j) {
                        continue;
                    }
                    
                    if(g(k, j) && !g(j, k) && !g(i, k) && !g(k, i)) {
                        cmp = true;
                    }
                }
                
                checkEqual(isDirectedEdgeInVStructure(g, i, j), cmp);
            }
        }
    }
    
    static void test_isValidEssentialGraph() {
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, N)(rng);
            G g(n);
            double p = UnifReal<double>(0.0, 1.0)(rng);
            for(int i = 0; i < n; ++i) {
                for(int j = 0; j < n; ++j) {
                    if(i != j && UnifReal<double>(0.0, 1.0)(rng) < p) {
                        g.addD(i, j);
                    }
                }
            }
            if(n >= 3) {
                vector<int> perm(n);
                for(int i = 0; i < n; ++i) {
                    perm[i] = i;
                }
                shuffle(perm.begin(), perm.end(), rng);
                int k = UnifInt<int>(3, n)(rng);
                for(int i = 0; i < k; ++i) {
                    int a = perm[i];
                    int b = perm[(i + 1) % k];
                    g.addD(a, b);
                }
                int i = UnifInt<int>(0, k - 1)(rng);
                int a = perm[i];
                int b = perm[(i + 1) % k];
                g.delD(b, a);
                checkFalse(isValidEssentialGraph(g));
            }
        }
        for(int t = 0; t < 100; ++t) {
            int n = UnifInt<int>(0, min(N, 30))(rng);
            int e = 0;
            if(n > 0) {
                e = UnifInt<int>(n - 1, n * (n - 1) / 2)(rng);
            }
            G g = G::randomConnectedChordal(n, e);
            checkTrue(isValidEssentialGraph(g));
        }
    }
    
    static void testAll() {
        test_isDirectedEdgeInVStructure();
        test_isValidEssentialGraph();
    }
};

template <int N>
struct TestCountIterateMAO {
    typedef Graph<N> G;
    
    struct GraphCmp {
        bool operator()(const G& a, const G& b) {
            if(a.size() != b.size()) {
                return a.size() < b.size();
            }
            int n = a.size();
            for(int i = 0; i < n; ++i) {
                for(int j = 0; j < n; ++j) {
                    if(a(i, j) != b(i, j)) {
                        return (int)a(i, j) < (int)b(i, j);
                    }
                }
            }
            return false;
        }
    };
    
    static void testAll() {
        {
            bool called = false;
            iterateMAO(G(), [&](const G& g) {
                checkEqual(g.size(), 0);
                checkFalse(called);
                called = true;
            });
            checkTrue(called);
        }
        
        for(int t = 0; t < 1000; ++t) {
            double p = UnifReal<double>(0.0, 1.0)(rng);
            G g;
            
            if(rng() & 1) {
                int e = UnifInt<int>(N - 1, N * (N - 1) / 2)(rng);
                g = G::randomConnectedChordal(N, e);
            } else {
                while(true) {
                    g = G(N);
                    for(int i = 0; i < N; ++i) {
                        for(int j = 0; j < N; ++j) {
                            if(i != j && UnifReal<double>(0.0, 1.0)(rng) < p) {
                                g.addD(i, j);
                            }
                        }
                    }
                    if(isValidEssentialGraph(g)) {
                        break;
                    }
                }
            }
            
            set<G, GraphCmp> results;
            G g2(N);
            iterateMAO(g, [&](const G& h) {
                checkEqual(h.size(), N);
                
                for(int a = 0; a < N; ++a) {
                    for(int b = 0; b < N; ++b) {
                        checkFalse(h(a, b) && h(b, a));
                        if(!g(a, b)) {
                            checkFalse(h(a, b));
                        }
                    }
                }
                
                for(int v = 0; v < N; ++v) {
                    checkTrue(setIntersection(h.reachableVertices(v), h.edgesIn(v)).isEmpty());
                }
                
                for(int a = 0; a < N; ++a) {
                    for(int b = 0; b < N; ++b) {
                        if(h(a, b)) {
                            g2.addD(a, b);
                        }
                        if(b == a) {
                            continue;
                        }
                        for(int c = 0; c < N; ++c) {
                            if(c == a || c == b) {
                                continue;
                            }
                            
                            bool vg = g(a, c) && !g(c, a) && g(b, c) && !g(c, b) && !g(a, b) && !g(b, a);
                            bool vh = h(a, c) && !h(c, a) && h(b, c) && !h(c, b) && !h(a, b) && !h(b, a);
                            
                            checkEqual(vg, vh);
                        }
                    }
                }
                
                checkTrue(results.insert(h).second);
            });
            checkEqual(g, g2);
            checkFalse(results.empty());
            for(const auto& p : countMAOMethods) {
                checkEqual((Z)results.size(), p.second(g));
            }
        }
    }
};

}

int main() {
    TestMAOFundamentals<1>::testAll();
    TestMAOFundamentals<2>::testAll();
    TestMAOFundamentals<3>::testAll();
    TestMAOFundamentals<7>::testAll();
    TestMAOFundamentals<11>::testAll();
    TestMAOFundamentals<23>::testAll();
    TestMAOFundamentals<51>::testAll();
    TestMAOFundamentals<128>::testAll();
    TestMAOFundamentals<201>::testAll();
    
    TestCountIterateMAO<1>::testAll();
    TestCountIterateMAO<2>::testAll();
    TestCountIterateMAO<3>::testAll();
    TestCountIterateMAO<4>::testAll();
    TestCountIterateMAO<5>::testAll();
    TestCountIterateMAO<6>::testAll();
    
    return 0;
}
