#include "test.hpp"

namespace {

template <int X>
struct Dummy { };

void test_select2Pow() {
    {
        bool called = false;
        select2Pow<10>(0, [&](auto sel) {
            checkEqual(sel.Exp, 0);
            checkEqual(sel.Val, 1);
            called = true;
        });
        checkTrue(called);
    }
    {
        bool called = false;
        select2Pow<10>(1, [&](auto sel) {
            checkEqual(sel.Exp, 0);
            checkEqual(sel.Val, 1);
            called = true;
        });
        checkTrue(called);
    }
    
    for(int e = 1; e <= 10; ++e) {
        for(int i = (1 << (e - 1)) + 1; i <= (1 << e); ++i) {
            bool called = false;
            select2Pow<10>(i, [&](auto sel) {
                Dummy<sel.Exp>();
                Dummy<sel.Val>();
                checkEqual(sel.Exp, e);
                checkEqual(sel.Val, 1 << e);
                called = true;
            });
            checkTrue(called);
        }
    }
}

template <int N>
void test_fastMemoryComparison() {
    test_fastMemoryComparison<N - 1>();
    
    typedef array<char, max(N, 1)> A;
    vector<A> vec;
    
    for(int t = 0; t < 100; ++t) {
        A a = {};
        for(int i = 0; i < N; ++i) {
            a[i] = UnifInt<char>()(rng);
        }
        vec.push_back(a);
    }
    
    sort(vec.begin(), vec.end());
    vec.erase(unique(vec.begin(), vec.end()), vec.end());
    
    shuffle(vec.begin(), vec.end(), rng);
    
    sort(vec.begin(), vec.end(), [&](const A& a, const A& b) {
        return fastMemoryComparisonLess<N>((void*)a.data(), (void*)b.data());
    });
    
    for(int i = 0; i < (int)vec.size(); ++i) {
        A a = vec[i];
        A a2 = a;
        assert(fastMemoryComparisonEqual<N>((void*)a.data(), (void*)a2.data()));
        assert(!fastMemoryComparisonLess<N>((void*)a.data(), (void*)a2.data()));
        for(int j = i + 1; j < (int)vec.size(); ++j) {
            A b = vec[j];
            assert(fastMemoryComparisonLess<N>((void*)a.data(), (void*)b.data()));
            assert(!fastMemoryComparisonLess<N>((void*)b.data(), (void*)a.data()));
            assert(!fastMemoryComparisonEqual<N>((void*)a.data(), (void*)b.data()));
        }
    }
}

template <>
void test_fastMemoryComparison<-1>() { }

}

int main() {
    test_select2Pow();
    test_fastMemoryComparison<20>();
    
    return 0;
}
