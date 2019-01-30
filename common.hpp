#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include <immintrin.h>

#include <sys/resource.h>

#include <gmpxx.h>

using namespace std;

static thread_local mt19937 rng(random_device{}());

template <typename T>
using UnifInt = uniform_int_distribution<T>;

template <typename T>
using UnifReal = uniform_real_distribution<T>;

typedef mpz_class Z;
typedef mpq_class Q;

inline void stderrPrint() {
    cerr << "\n";
}
template <typename F, typename... T>
void stderrPrint(const F& f, const T&... p) {
    cerr << f;
    stderrPrint(p...);
}
template <typename... T>
void fail(const T&... p) {
    stderrPrint("FAIL: ", p...);
    abort();
}

template <typename T>
T fromString(const string& str) {
    T ret;
    stringstream ss(str);
    ss >> ret;
    if(ss.fail()) fail("fromString: Could not convert string '", str, "' to given type.");
    return ret;
}

template <typename T>
string toString(const T& obj) {
    stringstream ss;
    ss << obj;
    return ss.str();
}

constexpr int ceilLog2(int x) {
    assert(x >= 0);
    if(x <= 1) {
        return 0;
    } else {
        return 32 - __builtin_clz(x - 1);
    }
}

template <int M, typename F>
class Select2Pow_ {
public:
    static_assert(M >= 0 && M <= 30, "Maximum select2Pow exponent must be between 0 and 30");
    
    template <int E>
    struct Value {
        static const int Exp = E;
        static const int Val = 1 << E;
    };
    
    static void selectRun(int x, F& f) {
        assert(x >= 0);
        int e = ceilLog2(x);
        if(e > M) {
            fail("Maximum exponent exceeded in select2Pow, increase template parameter M");
        }
        runs_[e](f);
    }
    
private:
    template <int E>
    static void run_(F& f) {
        f(Value<E>());
    }
    
    template <int E>
    struct Index_ { };
    
    static void initializeRunsRecursion_(array<void (*)(F&), M + 1>&, Index_<-1>) { }
    template <int E>
    static void initializeRunsRecursion_(array<void (*)(F&), M + 1>& runs, Index_<E>) {
        initializeRunsRecursion_(runs, Index_<E - 1>());
        runs[E] = run_<E>;
    }
    
    static array<void (*)(F&), M + 1> initializeRuns_() {
        array<void (*)(F&), M + 1> runs;
        initializeRunsRecursion_(runs, Index_<M>());
        return runs;
    }
    
    static array<void (*)(F&), M + 1> runs_;
};

template <int M, typename F>
array<void (*)(F&), M + 1> Select2Pow_<M, F>::runs_ = Select2Pow_<M, F>::initializeRuns_();

template <int M, typename F>
void select2Pow(int x, F f) {
    Select2Pow_<M, F>::selectRun(x, f);
}

template <typename T>
ostream& operator<<(ostream& out, const vector<T>& vec) {
    out << '[';
    bool first = true;
    for(const T& elem : vec) {
        if(!first) {
            out << ", ";
        }
        first = false;
        out << elem;
    }
    out << ']';
    return out;
}
template <typename T, typename... R>
ostream& operator<<(ostream& out, const set<T, R...>& s) {
    out << '{';
    bool first = true;
    for(const T& elem : s) {
        if(!first) {
            out << ", ";
        }
        first = false;
        out << elem;
    }
    out << '}';
    return out;
}

template <typename A, typename B, typename... R>
ostream& operator<<(ostream& out, const map<A, B, R...>& m) {
    out << '{';
    bool first = true;
    for(const pair<A, B>& elem : m) {
        if(!first) {
            out << ", ";
        }
        first = false;
        out << elem.first << " -> " << elem.second;
    }
    out << '}';
    return out;
}

template <typename A, typename B>
ostream& operator<<(ostream& out, const pair<A, B>& p) {
    out << '(' << p.first << ", " << p.second << ')';
    return out;
}

template <typename T>
class HasPrint {
private:
    typedef char Yes;
    typedef Yes No[2];
    
    template<typename C> static auto test(void*)
        -> decltype(declval<const C>().print(declval<ostream&>()), Yes{});
    
    template<typename> static No& test(...);
    
public:
    static const bool Value = sizeof(test<T>(0)) == sizeof(Yes);
};

template <typename T, typename enable_if<HasPrint<T>::Value, int>::type = 0>
ostream& operator<<(ostream& out, const T& obj) {
    obj.print(out);
    return out;
}

class FactorialTable_ {
private:
    static vector<Z>& getTable_() {
        static vector<Z> table;
        return table;
    }
    
    template <int N>
    friend class FactorialTable;
};

template <int N>
class FactorialTable {
public:
    static const Z& get(int x) {
        assert(x >= 0 && x <= N);
        assert(x < (int)table_.size());
        return table_[x];
    }
    
private:
    static vector<Z>& initializeTable_() {
        vector<Z>& table = FactorialTable_::getTable_();
        while((int)table.size() < N + 1) {
            int i = table.size();
            Z val;
            if(i == 0) {
                val = 1;
            } else {
                val = table.back() * i;
            }
            table.push_back(val);
        }
        return table;
    }
    
    static vector<Z>& table_;
};

template <int N>
vector<Z>& FactorialTable<N>::table_ = FactorialTable<N>::initializeTable_();

template<typename... T> struct LambdaVisitor : T... { using T::operator()...; };
template<typename... T> LambdaVisitor(T...) -> LambdaVisitor<T...>;

template <typename Variant, typename... Visitor>
constexpr auto lambdaVisit(Variant&& var, Visitor&&... vis) {
    return visit(LambdaVisitor{vis...}, var);
}

template <int Size>
bool fastMemoryComparisonLess(void* a, void* b) {
    return memcmp(a, b, Size) < 0;
}
template <int Size>
bool fastMemoryComparisonEqual(void* a, void* b) {
    return memcmp(a, b, Size) == 0;
}

#define SPECIALIZE_FAST_MEMORY_COMPARISONS(type) \
template <> \
inline bool fastMemoryComparisonLess<sizeof(type)>(void* a, void* b) { \
    type aVal; \
    type bVal; \
    memcpy(&aVal, a, sizeof(type)); \
    memcpy(&bVal, b, sizeof(type)); \
    return aVal < bVal; \
} \
template <> \
inline bool fastMemoryComparisonEqual<sizeof(type)>(void* a, void* b) { \
    type aVal; \
    type bVal; \
    memcpy(&aVal, a, sizeof(type)); \
    memcpy(&bVal, b, sizeof(type)); \
    return aVal == bVal; \
}

SPECIALIZE_FAST_MEMORY_COMPARISONS(uint8_t)
SPECIALIZE_FAST_MEMORY_COMPARISONS(uint16_t)
SPECIALIZE_FAST_MEMORY_COMPARISONS(uint32_t)
SPECIALIZE_FAST_MEMORY_COMPARISONS(uint64_t)

struct PoolAllocatorState {
    PoolAllocatorState() : poolLeft(0), poolSize(0) { }
    
    unique_ptr<char[]> pool;
    char* poolPos;
    size_t poolLeft;
    size_t poolSize;
    vector<unique_ptr<char[]>> oldPools;
};

template <typename T>
class PoolAllocator {
public:
    typedef T value_type;
    
    constexpr PoolAllocator() : state_(make_shared<PoolAllocatorState>()) { }
    
    template <class X>
    constexpr PoolAllocator(const PoolAllocator<X>& x) noexcept : state_(x.state_) { }
    
    T* allocate(size_t n) {
        n *= sizeof(T);
        PoolAllocatorState& state = *state_;
        if(state.poolLeft < n) {
            state.oldPools.push_back(move(state.pool));
            
            state.poolSize = max(n, 2 * state.poolSize);
            state.poolLeft = state.poolSize;
            
            state.pool = make_unique<char[]>(state.poolSize);
            state.poolPos = state.pool.get();
        }
        T* ret = (T*)state.poolPos;
        state.poolPos += n;
        state.poolLeft -= n;
        return ret;
    }
    void deallocate(T*, size_t) noexcept { }
    
    template <typename X>
    bool operator==(const PoolAllocator<X>& x) {
        return state_.get() == x.state_.get();
    }
    
    template <typename X>
    bool operator!=(const PoolAllocator<X>& x) {
        return state_.get() != x.state_.get();
    }
    
private:
    shared_ptr<PoolAllocatorState> state_;
    
    template <typename X>
    friend class PoolAllocator;
};

inline void removeStackLimit() {
    rlimit rl;
    if(getrlimit(RLIMIT_STACK, &rl)) fail("Getting stack limit failed");
    rl.rlim_cur = rl.rlim_max;
    if(setrlimit(RLIMIT_STACK, &rl)) fail("Setting stack limit failed");
}

inline void hashCombine(uint64_t& hash, uint64_t elem) {
    const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
    elem *= m;
    elem ^= elem >> 47;
    elem *= m;
    hash ^= elem;
    hash *= m;
}
