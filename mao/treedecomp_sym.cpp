#include "mao.hpp"
#include "full_binary_tree.hpp"

#include <deque>
#include <cstdlib>
#include <new>

namespace {

template <int N>
using BP = BitSet<bitSetParam(N)>;

// Pre-Merge Tree, phase 1: The tree where leaves are the cliques in the graph and inner nodes specify the order in which they are merged
template <int N>
struct PreMergeNode1 {
    BP<N> verts;
};
template <int N>
struct PreMergeLeaf1 {
    BP<N> verts;
};
template <int N>
using PreMergeTree1 = FullBinaryTree<PreMergeNode1<N>, PreMergeLeaf1<N>>;

template <int N>
PreMergeTree1<N> createPreMergeTree1_(const typename Graph<N>::TreeDecomposition& dec, int decNode, int decParent) {
    BP<N> verts = dec.cliques[decNode];
    
    PreMergeTree1<N> ret({verts});
    
    dec.structure.neighbors(decNode).iterate([&](int decChild) {
        if(decChild == decParent) {
            return;
        }
        
        PreMergeTree1<N> left = move(ret);
        PreMergeTree1<N> right = createPreMergeTree1_<N>(dec, decChild, decNode);
        ret = PreMergeTree1<N>({verts}, move(left), move(right));
    });
    
    return ret;
}

template <int N>
PreMergeTree1<N> createPreMergeTree1(const typename Graph<N>::TreeDecomposition& dec) {
    return createPreMergeTree1_<N>(dec, 0, -1);
}

// Pre-Merge Tree, phase 2: For each node, the vertices that are not present in the tree outside the subtree are removed from the output early.
template <int N>
struct PreMergeNode2 {
    BP<N> outputVerts;
    BP<N> rightVerts;
    BP<N> verts;
};
template <int N>
struct PreMergeLeaf2 {
    BP<N> outputVerts;
    int extraCount;
};
template <int N>
using PreMergeTree2 = FullBinaryTree<PreMergeNode2<N>, PreMergeLeaf2<N>>;

template <int N>
pair<PreMergeTree2<N>, BP<N>> createPreMergeTree2_(
    const PreMergeTree1<N>& src,
    BP<N> outputVerts
) {
    return src.visit(
        [&](const auto& srcNode) {
            PreMergeNode2<N> node;
            
            node.outputVerts = setIntersection(srcNode.verts, outputVerts);
            
            PreMergeTree2<N> right;
            tie(right, node.rightVerts) = createPreMergeTree2_<N>(srcNode.right, srcNode.verts);
            
            PreMergeTree2<N> left;
            tie(left, node.verts) = createPreMergeTree2_<N>(srcNode.left, setUnion(node.rightVerts, outputVerts));
            
            return make_pair(
                PreMergeTree2<N>(node, move(left), move(right)),
                node.outputVerts
            );
        },
        [&](const auto& srcLeaf) {
            PreMergeLeaf2<N> leaf;
            leaf.outputVerts = setIntersection(srcLeaf.verts, outputVerts);
            leaf.extraCount = setDifference(srcLeaf.verts, leaf.outputVerts).count();
            return make_pair(
                PreMergeTree2<N>(leaf),
                leaf.outputVerts
            );
        }
    );
}

template <int N>
PreMergeTree2<N> createPreMergeTree2(const PreMergeTree1<N>& src) {
    return createPreMergeTree2_<N>(src, {}).first;
}

template <int N>
PreMergeTree2<N> createPreMergeTree2(const typename Graph<N>::TreeDecomposition& dec) {
    return createPreMergeTree2<N>(createPreMergeTree1<N>(dec));
}

// Pre-Merge Tree, phase 3: For inner nodes, the symmetries are given.
template <int N>
struct PreMergeNode3 {
    BP<N> outputVerts;
    BP<N> rightVerts;
    BP<N> verts;
    vector<BP<N>> leftSymmetries;
    vector<BP<N>> rightSymmetries;
    vector<BP<N>> symmetries;
    Z symmetryMultiplier;
};
template <int N>
struct PreMergeLeaf3 {
    BP<N> outputVerts;
    int extraCount;
};
template <int N>
using PreMergeTree3 = FullBinaryTree<PreMergeNode3<N>, PreMergeLeaf3<N>>;

template <int N>
pair<PreMergeTree3<N>, vector<BP<N>>> createPreMergeTree3_(const PreMergeTree2<N>& src) {
    typedef BP<N> B;
    
    return src.visit(
        [&](const auto& srcNode) {
            PreMergeNode3<N> node = {
                srcNode.outputVerts,
                srcNode.rightVerts,
                srcNode.verts
            };
            
            B comp[N];
            node.verts.iterate([&](int v) {
                comp[v] = node.verts;
            });
            
            PreMergeTree3<N> left;
            tie(left, node.leftSymmetries) = createPreMergeTree3_(srcNode.left);
            
            PreMergeTree3<N> right;
            tie(right, node.rightSymmetries) = createPreMergeTree3_(srcNode.right);
            
            auto chop = [&](B s) {
                assert(isSubset(s, node.verts));
                s.iterate([&](int v) {
                    comp[v] = setIntersection(comp[v], s);
                });
            };
            
            for(B s : node.leftSymmetries) {
                chop(s);
            }
            for(B s : node.rightSymmetries) {
                chop(s);
            }
            chop(setDifference(node.verts, node.rightVerts));
            chop(node.outputVerts);
            chop(setDifference(node.verts, node.outputVerts));
            
            node.verts.iterate([&](int v) {
                if(comp[v].min() == v) {
                    node.symmetries.push_back(comp[v]);
                }
            });
            
            node.symmetryMultiplier = 1;
            vector<B> outputSymmetries;
            for(B s : node.symmetries) {
                if(isSubset(s, node.outputVerts)) {
                    outputSymmetries.push_back(s);
                } else {
                    assert(setIntersection(s, node.outputVerts).isEmpty());
                    node.symmetryMultiplier *= FactorialTable<N>::get(s.count());
                }
            }
            
            return make_pair(
                PreMergeTree3<N>(move(node), move(left), move(right)),
                outputSymmetries
            );
        },
        [&](const auto& srcLeaf) {
            PreMergeLeaf3<N> leaf = {
                srcLeaf.outputVerts,
                srcLeaf.extraCount
            };
            
            vector<B> symmetries;
            symmetries.push_back(leaf.outputVerts);
            
            return make_pair(
                PreMergeTree3<N>(leaf),
                move(symmetries)
            );
        }
    );
}

template <int N>
PreMergeTree3<N> createPreMergeTree3(const PreMergeTree2<N>& src) {
    return createPreMergeTree3_<N>(src).first;
}

template <int N>
PreMergeTree3<N> createPreMergeTree3(const typename Graph<N>::TreeDecomposition& dec) {
    return createPreMergeTree3<N>(createPreMergeTree2<N>(dec));
}

// Final Merge Tree: For leaves, only the number of output and extra vertices are given, and for inner nodes, the output and right vertices and symmetries are only given as masks relative to the local set of vertices.
template <int N>
struct MergeNode {
    int vertCount;
    BP<N> outputMask;
    BP<N> rightMask;
    vector<BP<N>> leftSymmetries;
    vector<BP<N>> rightSymmetries;
    vector<BP<N>> symmetries;
    Z symmetryMultiplier;
};
struct MergeLeaf {
    int outputCount;
    int extraCount;
};
template <int N>
using MergeTree = FullBinaryTree<MergeNode<N>, MergeLeaf>;

template <int N>
MergeTree<N> createMergeTree(const PreMergeTree3<N>& src) {
    typedef BP<N> B;
    
    return src.visit(
        [&](const auto& srcNode) {
            auto packVec = [&](const vector<B>& v, B mask) {
                vector<B> ret;
                for(B s : v) {
                    assert(isSubset(s, mask));
                    ret.push_back(B::pack(s, mask));
                }
                return ret;
            };
            return MergeTree<N>(
                {
                    srcNode.verts.count(),
                    B::pack(srcNode.outputVerts, srcNode.verts),
                    B::pack(srcNode.rightVerts, srcNode.verts),
                    packVec(srcNode.leftSymmetries, srcNode.verts),
                    packVec(srcNode.rightSymmetries, srcNode.rightVerts),
                    packVec(srcNode.symmetries, srcNode.verts),
                    srcNode.symmetryMultiplier
                },
                createMergeTree(srcNode.left),
                createMergeTree(srcNode.right)
            );
        },
        [&](const auto& srcLeaf) {
            return MergeTree<N>({
                srcLeaf.outputVerts.count(),
                srcLeaf.extraCount
            });
        }
    );
}

template <int N>
MergeTree<N> createMergeTree(const typename Graph<N>::TreeDecomposition& dec) {
    return createMergeTree<N>(createPreMergeTree3<N>(dec));
}

template <int N, int SrcN>
MergeTree<N> convertMergeTree(const MergeTree<SrcN>& src) {
    return src.visit(
        [&](const auto& srcNode) {
            return MergeTree<N>(
                {
                    srcNode.vertCount,
                    srcNode.outputMask,
                    srcNode.rightMask,
                    vector<BP<N>>(srcNode.leftSymmetries.begin(), srcNode.leftSymmetries.end()),
                    vector<BP<N>>(srcNode.rightSymmetries.begin(), srcNode.rightSymmetries.end()),
                    vector<BP<N>>(srcNode.symmetries.begin(), srcNode.symmetries.end()),
                    srcNode.symmetryMultiplier
                },
                convertMergeTree<N>(srcNode.left),
                convertMergeTree<N>(srcNode.right)
            );
        },
        [&](const auto& srcLeaf) {
            return MergeTree<N>(srcLeaf);
        }
    );
}

template <int N>
int maxVertCount(const MergeTree<N>& mergeTree) {
    return mergeTree.visit(
        [&](const auto& node) {
            return max(
                node.vertCount,
                max(maxVertCount(node.left), maxVertCount(node.right))
            );
        },
        [&](const auto& leaf) {
            return leaf.outputCount;
        }
    );
}

template <int N>
struct Seq {
    typedef typename conditional<N <= 256, uint8_t, int>::type Elem;
    Elem elems[N];
    
    Seq() : elems() { }
    
    const Elem& operator[](int i) const {
        return elems[i];
    }
    Elem& operator[](int i) {
        return elems[i];
    }
    
    bool operator<(const Seq& b) const {
        return fastMemoryComparisonLess<N * sizeof(Elem)>((void*)elems, (void*)b.elems);
    }
    bool operator==(const Seq& b) const {
        return fastMemoryComparisonEqual<N * sizeof(Elem)>((void*)elems, (void*)b.elems);
    }
};

template <int N>
using State = pair<Seq<N>, BP<N>>;

template <int N>
using Dist = map<State<N>, Z, less<State<N>>, PoolAllocator<pair<const State<N>, Z>>>;

template <int N>
struct Contractor {
    typedef BP<N> B;
    
    Contractor(int srcSize, B mask)
        : srcSize(srcSize),
          size(mask.count()),
          mask(mask)
    {
        assert(srcSize <= N);
        assert(isSubset(mask, B::range(srcSize)));
        int pos = 0;
        
        mask.iterate([&](int j) {
            vertexMap[j] = pos++;
        });
    }
    
    Contractor(int srcSize, B mask, const vector<B>& symmetries)
        : srcSize(srcSize),
          size(mask.count()),
          mask(mask)
    {
        assert(srcSize <= N);
        assert(isSubset(mask, B::range(srcSize)));
        int pos = 0;
        
        int symMap[N];
        for(B s : symmetries) {
            int v = s.min();
            s.iterate([&](int x) {
                symMap[x] = v;
            });
        }
        
        mask.iterate([&](int j) {
            vertexMap[j] = symMap[pos++];
        });
    }
    
    int srcSize;
    int size;
    B mask;
    typename Seq<N>::Elem vertexMap[N];
    
    struct Result {
        Seq<N> seq;
        B origLostPred;
        B chosenMask;
    };
    
    Result operator()(const Seq<N>& seq) {
        Result ret;
        
        bool removed = false;
        int pos = 0;
        
        for(int i = 0; i < srcSize; ++i) {
            int j = seq[i];
            assert(j >= 0 && j < srcSize);
            if(mask.has(j)) {
                if(removed) {
                    ret.origLostPred.add(i);
                }
                ret.chosenMask.add(i);
                ret.seq[pos++] = vertexMap[j];
            } else {
                removed = true;
            }
        }
        
        assert(pos == size);
        
        return ret;
    }
};

template <int N>
struct Expander {
    typedef BP<N> B;
    
    Expander(int vertCount, const vector<B>& srcSymmetries, const vector<B>& destSymmetries)
        : vertCount(vertCount)
    {
        B destSymMap[N];
        for(B s : destSymmetries) {
            s.iterate([&](int v) {
                destSymMap[v] = s;
            });
        }
        subCount = 0;
        for(B s : srcSymmetries) {
            int subsStart = subCount;
            s.iterate([&](int v) {
                B s2 = destSymMap[v];
                assert(isSubset(s2, s));
                if(s2.min() == v) {
                    subVert[subCount] = v;
                    subSize[subCount] = s2.count();
                    ++subCount;
                }
            });
            int subsEnd = subCount;
            subRange[s.min()] = make_pair(subsStart, subsEnd);
        }
        this->srcSymmetries = srcSymmetries;
    }
    
    int vertCount;
    int subCount;
    int subVert[N];
    int subSize[N];
    pair<int, int> subRange[N];
    vector<B> srcSymmetries;
    
    template <typename F>
    void expand_(const Seq<N>& src, Seq<N>& seq, F& f, int* subLeft, int i) {
        if(i == vertCount) {
            f((const Seq<N>&)seq);
            return;
        }
        
        pair<int, int> range = subRange[src[i]];
        
        for(int sub = range.first; sub < range.second; ++sub) {
            if(subLeft[sub]) {
                --subLeft[sub];
                seq[i] = subVert[sub];
                expand_(src, seq, f, subLeft, i + 1);
                ++subLeft[sub];
            }
        }
    }
    
    template <typename F>
    void operator()(const Seq<N>& src, F f) {
        Seq<N> seq;
        int subLeft[N];
        copy(subSize, subSize + subCount, subLeft);
        
        expand_(src, seq, f, subLeft, 0);
    }
};


template <int N>
Dist<N> singletonDist(int outputCount, int extraCount) {
    typedef BP<N> B;
    
    int a = outputCount;
    int b = extraCount;
    int n = a + b;
    
    assert(outputCount <= N);
    Seq<N> seq;
    for(int i = 0; i < outputCount; ++i) {
        seq[i] = 0;
    }
    
    Dist<N> dist;
    
    Z bFact = 1;
    for(int i = 2; i <= b; ++i) {
        bFact *= i;
    }
    
    if(b == 0) {
        dist[{seq, B()}] = bFact;
    } else {
        pair<B, Z> vals[N + 1];
        vals[a] = make_pair(B(), bFact);
        for(int i = a - 1; i >= 0; --i) {
            vals[i] = make_pair(
                vals[i + 1].first.with(i),
                vals[i + 1].second * (n - i - 1) / (a - i)
            );
        }
        
        for(int i = 0; i <= a; ++i) {
            dist[{seq, vals[i].first}] = vals[i].second;
        }
    }
    
    return dist;
}

template <int N>
Dist<N> combine(
    const Dist<N>& a,
    const Dist<N>& b,
    int vertCount,
    BP<N> bMask,
    BP<N> outputMask,
    const vector<BP<N>>& symmetries,
    const vector<BP<N>>& aSymmetries,
    const vector<BP<N>>& bSymmetries
) {
    typedef BP<N> B;
    
    Dist<N> dist;
    
    Expander<N> aExpander(vertCount, aSymmetries, symmetries);
    Contractor<N> bContractor(vertCount, bMask, bSymmetries);
    Contractor<N> outputContractor(vertCount, outputMask);
    
    auto as = a.begin();
    while(as != a.end()) {
        const Seq<N>& aSeq = as->first.first;
        
        auto ae = a.upper_bound({aSeq, B::ones()});
        
        aExpander(aSeq, [&](const Seq<N>& seq) {
            typename Contractor<N>::Result bContraction = bContractor(seq);
            typename Contractor<N>::Result outputContraction = outputContractor(seq);
            
            auto bs = b.lower_bound({bContraction.seq, B()});
            auto be = b.upper_bound({bContraction.seq, B::ones()});
            
            for(auto ai = as; ai != ae; ++ai) {
                for(auto bi = bs; bi != be; ++bi) {
                    if(setIntersection(
                        setUnion(ai->first.second, bContraction.origLostPred),
                        B::unpack(bi->first.second, bContraction.chosenMask)
                    ).isEmpty()) {
                        B lostPred = setUnion(
                            setUnion(
                                ai->first.second,
                                B::unpack(bi->first.second, bContraction.chosenMask)
                            ),
                            outputContraction.origLostPred
                        );
                        lostPred = B::pack(lostPred, outputContraction.chosenMask);
                        Z count = ai->second * bi->second;
                        dist[{outputContraction.seq, lostPred}] += count;
                    }
                }
            }
        });
        
        as = ae;
    }
    
    return dist;
}

template <int N>
pair<Dist<N>, Z> dpSubtree(const MergeTree<N>& mergeTree) {
    return mergeTree.visit(
        [&](const auto& node) {
            Dist<N> left;
            Z leftMul;
            tie(left, leftMul) = dpSubtree(node.left);
            Dist<N> right;
            Z rightMul;
            tie(right, rightMul) = dpSubtree(node.right);
            
            Dist<N> output = combine(
                left, right,
                node.vertCount,
                node.rightMask,
                node.outputMask,
                node.symmetries,
                node.leftSymmetries,
                node.rightSymmetries
            );
            Z mul = node.symmetryMultiplier * leftMul * rightMul;
            
            return make_pair(output, mul);
        },
        [&](const auto& leaf) {
            return make_pair(
                singletonDist<N>(leaf.outputCount, leaf.extraCount),
                (Z)1
            );
        }
    );
}

template <int N, int SrcN>
Z count(const MergeTree<SrcN>& srcMergeTree) {
    MergeTree<N> mergeTree = convertMergeTree<N>(srcMergeTree);
    
    Z mul;
    Dist<N> dist;
    tie(dist, mul) = dpSubtree(mergeTree);
    
    Z ret = 0;
    for(const pair<State<N>, Z>& p : dist) {
        ret += p.second;
    }
    
    ret *= mul;
    
    return ret;
}

template <int N>
Z count(const Graph<N>& graph) {
    assert(isValidEssentialGraph(graph));
    
    Z ret = 1;
    graph.iterateBidirectionalComponents([&](const auto& comp) {
        Graph<N> subgraph = graph.inducedSubgraph(comp);
        auto decomp = subgraph.findTreeDecompositionInChordal();
        MergeTree<N> mergeTree = createMergeTree<N>(decomp);
        select2Pow<ceilLog2(N)>(maxVertCount(mergeTree), [&](auto sel) {
            ret *= count<sel.Val>(mergeTree);
        });
    });
    return ret;
}

}

Z countMAOUsingTreeDecompositionDPSymmetryReduction(const GraphData& graphData) {
    Z ret;
    graphData.accessGraph([&](auto graph) {
        ret = count(graph);
    });
    return ret;
}
