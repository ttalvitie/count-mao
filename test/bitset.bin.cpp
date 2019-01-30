#include "test.hpp"

#include "bitset.hpp"

namespace {

void test_selectBitSetParam() {
    const int M = 1234;
    for(int i = 0; i <= M; ++i) {
        bool called = false;
        selectBitSetParam<M>(i, [&](auto sel) {
            const int W = sel.Val;
            checkLessOrEqual(i, BitSet<W>::BitCount);
            if(i == 0) {
                checkEqual(W, 1);
            } else {
                checkGreater(2 * i + 63, BitSet<W>::BitCount);
            }
            called = true;
        });
        checkTrue(called);
    }
}

template <int A, int B>
void test_conversions() {
    for(int t = 0; t < 100; ++t) {
        int commonBits = min(BitSet<A>::BitCount, BitSet<B>::BitCount);
        BitSet<A> a = setIntersection(BitSet<A>::random(), BitSet<A>::range(commonBits));
        BitSet<B> b = (const BitSet<A>&)a;
        
        while(!a.isEmpty()) {
            int x = a.min();
            checkEqual(b.min(), x);
            a.del(x);
            b.del(x);
        }
        checkEqual(b, BitSet<B>());
    }
};

template <int W>
struct TestBitSet {
    typedef BitSet<W> B;
    
    static B randomBitSet() {
        int a = UnifInt<int>(0, 9)(rng);
        if(a == 0) {
            B ret;
            int x = UnifInt<int>(-10, B::BitCount - 1)(rng);
            if(x >= 0) {
                ret.add(x);
            }
            if(rng() & 1) {
                ret.invert();
            }
            return ret;
        } else {
            B ret = B::random();
            int c = UnifInt<int>(-4, 8)(rng);
            for(int i = 0; i < c; ++i) {
                ret = setIntersection(ret, B::random());
            }
            if(rng() & 1) {
                ret.invert();
            }
            int x = UnifInt<int>(-B::BitCount, B::BitCount)(rng);
            int y = UnifInt<int>(0, 2 * B::BitCount)(rng);
            if(x >= 0) {
                ret = setDifference(ret, B::range(x));
            }
            if(y <= B::BitCount) {
                ret = setIntersection(ret, B::range(y));
            }
            return ret;
        }
    }
    
    static void testConstructors() {
        {
            B x;
            x.iterate([&](int) {
                failTest();
            });
        }
        
        for(int i = 0; i <= B::BitCount; ++i) {
            B x = B::range(i);
            for(int j = 0; j < B::BitCount; ++j) {
                checkEqual(x.has(j), j < i);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            int a = UnifInt<int>(0, B::BitCount)(rng);
            int b = UnifInt<int>(0, B::BitCount)(rng);
            B x = B::range(a, b);
            for(int j = 0; j < B::BitCount; ++j) {
                checkEqual(x.has(j), j >= a && j < b);
            }
        }
        
        checkEqual(B::ones(), B::range(B::BitCount));
        
        {
            int a = 0;
            int b = 0;
            B in = B::range(B::BitCount);
            B un;
            for(int t = 0; t < 200; ++t) {
                B x = B::random();
                a += x.count();
                b += B::BitCount;
                in = setIntersection(in, x);
                un = setUnion(un, x);
            }
            double f = (double)a / (double)b;
            checkBetweenExclusive(0.4, f, 0.6);
            checkEqual(in, B());
            checkEqual(un, B::range(B::BitCount));
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            uint64_t m = x.bottomWord();
            B y = B::fromWord(m);
            checkEqual(y, setIntersection(x, B::range(64)));
        }
    }
    
    static void testAccessors() {
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            bool seen[B::BitCount] = {};
            int prev = -1;
            x.iterate([&](int i) {
                checkGreater(i, prev);
                checkBetweenHalfOpen(0, i, B::BitCount);
                seen[i] = true;
                prev = i;
            });
            
            for(int i = 0; i < B::BitCount; ++i) {
                checkEqual(x.has(i), seen[i]);
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            B y;
            int prev = B::BitCount;
            x.iterateBackwards([&](int i) {
                checkLess(i, prev);
                checkBetweenHalfOpen(0, i, B::BitCount);
                y.add(i);
                prev = i;
            });
            checkEqual(x, y);
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            int i = UnifInt<int>(0, B::BitCount - 1)(rng);
            
            B y = x;
            y.add(i);
            checkEqual(x.with(i), y);
            checkTrue(y.has(i));
            
            B z = x;
            z.set(i, true);
            checkEqual(y, z);
            
            for(int j = 0; j < B::BitCount; ++j) {
                if(j != i) {
                    checkEqual(x.has(j), y.has(j));
                }
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            int i = UnifInt<int>(0, B::BitCount - 1)(rng);
            
            B y = x;
            y.del(i);
            checkEqual(x.without(i), y);
            checkFalse(y.has(i));
            
            B z = x;
            z.set(i, false);
            checkEqual(y, z);
            
            for(int j = 0; j < B::BitCount; ++j) {
                if(j != i) {
                    checkEqual(x.has(j), y.has(j));
                }
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            int i = UnifInt<int>(0, B::BitCount - 1)(rng);
            
            B y = x;
            y.toggle(i);
            checkNotEqual(x.has(i), y.has(i));
            
            for(int j = 0; j < B::BitCount; ++j) {
                if(j != i) {
                    checkEqual(x.has(j), y.has(j));
                }
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            B y;
            x.iterateWhile([&](int i) {
                y.add(i);
                return true;
            });
            checkEqual(x, y);
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            B y;
            x.iterateUntil([&](int i) {
                y.add(i);
                return false;
            });
            checkEqual(x, y);
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            int c = UnifInt<int>(0, 20)(rng);
            
            {
                int i = 0;
                x.iterateWhile([&](int) {
                    if(i > 0 && i >= c) failTest();
                    ++i;
                    return i < c;
                });
            }
            {
                int i = 0;
                x.iterateUntil([&](int) {
                    if(i > 0 && i >= c) failTest();
                    ++i;
                    return i >= c;
                });
            }
        }
    }
    
    static void testComparisons() {
        for(int t = 0; t < 1000; ++t) {
            B x = randomBitSet();
            B y = randomBitSet();
            if(UnifInt<int>(0, 3)(rng) == 0) {
                y = x;
            }
            
            checkNotEqual(x == y, x != y);
            checkNotEqual(x < y, x >= y);
            checkNotEqual(x <= y, x > y);
            checkNotEqual(x > y, x <= y);
            checkNotEqual(x >= y, x < y);
            if(x == y) {
                checkTrue(x <= y);
                checkTrue(x >= y);
            } else {
                checkTrue(x < y || x > y);
            }
            
            vector<int> xv;
            x.iterateBackwards([&](int i) {
                xv.push_back(i);
            });
            vector<int> yv;
            y.iterateBackwards([&](int i) {
                yv.push_back(i);
            });
            
            checkEqual(x < y, xv < yv);
            
            B z = randomBitSet();
            checkFalse(x < y && y < z && z < x);
        }
    }
    
    static void testQueries() {
        for(int t = 0; t < 500; ++t) {
            B x = randomBitSet();
            
            int a = -1;
            int b = -1;
            int c = 0;
            x.iterate([&](int v) {
                checkEqual(x.nth(c), v);
                if(a == -1) {
                    a = v;
                }
                b = v;
                ++c;
            });
            checkEqual(c, x.count());
            
            checkEqual(c == 0, x.isEmpty());
            checkEqual(c > 0, x.isNonEmpty());
            checkEqual(c == 1, x.isSingleton());
            checkEqual(c <= 1, x.isSingletonOrEmpty());
            
            if(c) {
                checkEqual(x.min(), a);
                checkEqual(x.max(), b);
            } else {
                checkEqual(a, -1);
                checkEqual(b, -1);
            }
        }
    }
    
    static void testOperations() {
        for(int t = 0; t < 500; ++t) {
            B x = randomBitSet();
            int d = UnifInt<int>(-2 * B::BitCount, 2 * B::BitCount)(rng);
            
            B y = x;
            y.shift(d);
            checkEqual(y, x.shifted(d));
            
            for(int i = 0; i < B::BitCount; ++i) {
                int j = i - d;
                if(j >= 0 && j < B::BitCount) {
                    checkEqual(y.has(i), x.has(j));
                } else {
                    checkFalse(y.has(i));
                }
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            
            B y = x;
            y.invert();
            checkEqual(y, x.inverted());
            
            for(int i = 0; i < B::BitCount; ++i) {
                checkNotEqual(x.has(i), y.has(i));
            }
        }
        
        for(int t = 0; t < 500; ++t) {
            B x = randomBitSet();
            B y = randomBitSet();
            
            B z;
            int j = 0;
            y.iterate([&](int i) {
                if(x.has(j)) {
                    z.add(i);
                }
                ++j;
            });
            
            checkEqual(z, B::unpack(x, y));
        }
        
        for(int t = 0; t < 500; ++t) {
            B x = randomBitSet();
            B y = randomBitSet();
            
            B z;
            int j = 0;
            y.iterate([&](int i) {
                if(x.has(i)) {
                    z.add(j);
                }
                ++j;
            });
            
            checkEqual(z, B::pack(x, y));
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            B y = randomBitSet();
            
            B in = setIntersection(x, y);
            B un = setUnion(x, y);
            B diff = setDifference(x, y);
            B sym = setSymmetricDifference(x, y);
            
            for(int i = 0; i < B::BitCount; ++i) {
                checkEqual(in.has(i), x.has(i) && y.has(i));
                checkEqual(un.has(i), x.has(i) || y.has(i));
                checkEqual(diff.has(i), x.has(i) && !y.has(i));
                checkEqual(sym.has(i), x.has(i) != y.has(i));
            }
        }
        
        for(int t = 0; t < 100; ++t) {
            B x = randomBitSet();
            B y = randomBitSet();
            
            if(rng() & 1) {
                x = setIntersection(x, y);
            }
            
            checkEqual(isSubset(x, y), setDifference(x, y).isEmpty());
        }
    }
    
    static void testAll() {
        testConstructors();
        testAccessors();
        testComparisons();
        testQueries();
        testOperations();
    }
};

}

int main() {
    test_selectBitSetParam();
    TestBitSet<1>::testAll();
    TestBitSet<2>::testAll();
    TestBitSet<3>::testAll();
    
    test_conversions<1, 1>();
    test_conversions<1, 2>();
    test_conversions<1, 3>();
    test_conversions<2, 1>();
    test_conversions<2, 2>();
    test_conversions<2, 3>();
    test_conversions<3, 1>();
    test_conversions<3, 2>();
    test_conversions<3, 3>();
    
    return 0;
}
