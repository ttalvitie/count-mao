#pragma once

#include "graph.hpp"

namespace he_et_al_2016 {

static const Q ZeroQ = 0;

struct Polynomial {
    vector<Q> coef;
    
    Polynomial() { }
    Polynomial(vector<Q> coef) : coef(move(coef)) { }
    
    const Q& operator[](int i) const {
        if(i >= (int)coef.size()) {
            return ZeroQ;
        } else {
            return coef[i];
        }
    }
    Q& operator[](int i) {
        if(i >= (int)coef.size()) {
            coef.resize(i + 1);
        }
        return coef[i];
    }
    
    Z operator()(int x) {
        Q ret = 0;
        if(x == 0) {
            ret = (*this)[0];
        } else {
            for(int i = (int)coef.size() - 1; i >= 0; --i) {
                ret *= x;
                ret += coef[i];
            }
        }
        assert(ret.get_den() == 1);
        return ret.get_num();
    }
    
    Polynomial translate(int d) {
        Polynomial ret;
        ret.coef.resize(coef.size());
        for(int i = 0; i < (int)coef.size(); ++i) {
            Z binom = 1;
            Z ds = 1;
            for(int j = 0; j <= i; ++j) {
                if(j > 0) {
                    binom = binom * (i - j + 1) / j;
                    ds *= d;
                }
                ret.coef[i - j] += binom * ds * coef[i];
            }
        }
        for(int i = 1; i <= d; ++i) {
            ret.mulByXPlusD(i);
        }
        return ret;
    }
    
    // Multiply by x + d
    void mulByXPlusD(int d) {
        coef.resize(coef.size() + 1);
        for(int i = (int)coef.size() - 2; i >= 0; --i) {
            coef[i + 1] += coef[i];
            coef[i] *= d;
        }
    }
    
    void addMul(const Polynomial& poly, const Q& c) {
        coef.resize(max(coef.size(), poly.coef.size()));
        for(int i = 0; i < (int)poly.coef.size(); ++i) {
            coef[i] += c * poly.coef[i];
        }
    }
    
    void mul(const Q& c) {
        for(Q& x : coef) {
            x *= c;
        }
    }
};

template <int N>
Z computeSize(const Graph<N>& C);

template <int N>
Polynomial computeSizeGF(const Graph<N>& C);

template <int N>
Polynomial computeSizeF(const Graph<N>& K) {
    typedef typename Graph<N>::B B;
    
    int p = K.size();
    
    if(p == 0) {
        return Polynomial({1});
    }
    
    int edgeCount = 0;
    for(int v = 0; v < p; ++v) {
        edgeCount += K.neighbors(v).count();
    }
    edgeCount /= 2;
    
    if(K.component(0) == K.vertexSet() && edgeCount == p - 1) {
        return Polynomial({p, 2 * p - 1, p - 1});
    }
    
    if(K.component(0) == K.vertexSet() && edgeCount == p) {
        return Polynomial({2 * p, 4 * p - 1, 2 * p, 1});
    }
    
    bool isolatedEdgeGraph = true;
    for(int v = 0; v < p; ++v) {
        if(!K.neighbors(v).isSingleton()) {
            isolatedEdgeGraph = false;
            break;
        }
    }
    if(isolatedEdgeGraph) {
        Z e = (Z)1 << (p / 2 - 1);
        return Polynomial({2 * e, Q(3 * p * e) / 2, Q(p * e) / 2});
    }
    
    B dom = K.vertexSet();
    for(int v = 0; v < K.size(); ++v) {
        dom = setIntersection(dom, K.neighbors(v).with(v));
    }
    if(dom.isNonEmpty()) {
        B nonDom = setDifference(K.vertexSet(), dom);
        Graph<N> newK = K.inducedSubgraph(nonDom);
        int w = dom.count();
        return computeSizeF(newK).translate(w);
    }
    
    B isol;
    for(int v = 0; v < K.size(); ++v) {
        if(K.neighbors(v).isEmpty()) {
            isol.add(v);
        }
    }
    if(isol.isNonEmpty()) {
        B nonIsol = setDifference(K.vertexSet(), isol);
        Graph<N> newK = K.inducedSubgraph(nonIsol);
        int k = isol.count();
        Polynomial poly = computeSizeF(newK);
        poly[1] += computeSize(newK) * k;
        return poly;
    }
    
    return computeSizeGF(K);
}

template <int N>
Z computeSize(const Graph<N>& C) {
    typedef typename Graph<N>::B B;
    
    Z ret = 1;
    C.iterateBidirectionalComponents([&](const auto& comp) {
        Graph<N> Cj = C.inducedSubgraph(comp);
        B dom = Cj.vertexSet();
        for(int v = 0; v < Cj.size(); ++v) {
            dom = setIntersection(dom, Cj.neighbors(v).with(v));
        }
        B nonDom = setDifference(Cj.vertexSet(), dom);
        
        Graph<N> Kj = Cj.inducedSubgraph(nonDom);
        int mj = dom.count();
        
        ret *= computeSizeF(Kj)(mj) * FactorialTable<N>::get(mj);
    });
    assert(ret > 0);
    return ret;
}

template <int N>
Z computeRootedSize(const Graph<N>& graph, int v) {
    typedef typename Graph<N>::B B;
    Graph<N> newGraph = graph;
    
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
        val *= computeSize(newGraph.inducedSubgraph(comp));
    });
    return val;
}

template <int N>
Polynomial computeSizeGF(const Graph<N>& K) {
    Z SK = 1;
    
    Polynomial bpoly;
    K.iterateBidirectionalComponents([&](const auto& comp) {
        Graph<N> Kj = K.inducedSubgraph(comp);
        
        Z SKj = 0;
        
        Polynomial poly;
        for(int v = 0; v < Kj.size(); ++v) {
            Z SKjv = computeRootedSize(Kj, v);
            Graph<N> KjNv = Kj.inducedSubgraph(Kj.neighbors(v));
            Polynomial SFKjNv = computeSizeF(KjNv);
            SKj += SKjv;
            
            poly.addMul(SFKjNv, Q(SKjv) / SFKjNv(0));
        }
        
        bpoly.addMul(poly, Q(1) / SKj);
        
        SK *= SKj;
    });
    bpoly.mul(SK);
    
    int d = bpoly.coef.size() - 1;
    assert(d >= 0);
    
    Polynomial poly;
    poly.coef.resize(d + 2);
    poly.coef[0] = SK;
    poly.coef[d + 1] = bpoly.coef[d] / (d + 1);
    for(int i = d; i >= 1; --i) {
        Q val = bpoly.coef[i - 1];
        
        Z aval = i;
        for(int j = i + 1; j <= d + 1; ++j) {
            aval = -(aval * j) / (j - i + 1);
            val -= aval * poly.coef[j];
        }
        val /= i;
        
        poly.coef[i] = val;
    }
    
    return poly;
}

}
