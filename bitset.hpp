#pragma once

#include "common.hpp"

// Forward-declarations
template <int W>
class BitSet;
template <int W>
BitSet<W> setIntersection(const BitSet<W>& a, const BitSet<W>& b);
template <int W>
BitSet<W> setUnion(const BitSet<W>& a, const BitSet<W>& b);
template <int W>
BitSet<W> setDifference(const BitSet<W>& a, const BitSet<W>& b);
template <int W>
BitSet<W> setSymmetricDifference(const BitSet<W>& a, const BitSet<W>& b);
template <int W>
bool isSubset(const BitSet<W>& a, const BitSet<W>& b);

template <int W>
class BitSet {
public:
    static_assert(W > 0, "The number of words in BitSet must be positive");
    
    static const int ParamW = W;
    static const int BitCount = W << 6;
    
    template <typename... T>
    explicit BitSet(int x, T... rest)
        : BitSet(rest...)
    {
        add(x);
    }
    
    BitSet(const BitSet<W>&) = default;
    
    template <int SrcW>
    BitSet(const BitSet<SrcW>& src) {
        int commonW = ::min(W, SrcW);
        copy(src.words_, src.words_ + commonW, words_);
        fill(words_ + commonW, words_ + W, 0);
        for(int i = commonW; i < SrcW; ++i) {
            assert(!src.words_[i]);
        }
    }
    
    BitSet() : words_() { }
    
    static BitSet range(int n) {
        assert(n >= 0 && n <= BitCount);
        
        BitSet ret(uninitializedTag_);
        for(int w = 0; w < W; ++w) {
            if(n >= 64) {
                ret.words_[w] = (uint64_t)-1;
            } else if(n >= 0) {
                ret.words_[w] = ((uint64_t)1 << n) - (uint64_t)1;
            } else {
                ret.words_[w] = 0;
            }
            n -= 64;
        }
        return ret;
    }
    static BitSet range(int a, int b) {
        if(a < b) {
            return setSymmetricDifference(range(a), range(b));
        } else {
            return BitSet();
        }
    }
    
    static BitSet random() {
        BitSet ret(uninitializedTag_);
        for(int w = 0; w < W; ++w) {
            ret.words_[w] = UnifInt<uint64_t>()(rng);
        }
        return ret;
    }
    
    static BitSet fromWord(uint64_t w) {
        BitSet ret;
        ret.words_[0] = w;
        return ret;
    }
    
    static BitSet ones() {
        BitSet ret(uninitializedTag_);
        for(int w = 0; w < W; ++w) {
            ret.words_[w] = (uint64_t)-1;
        }
        return ret;
    }
    
    bool has(int x) const {
        assert(x >= 0 && x < BitCount);
        
        return (bool)(words_[x >> 6] & ((uint64_t)1 << (x & 63)));
    }
    
    template <typename F>
    void iterate(F f) const {
        int base = 0;
        for(int w = 0; w < W; ++w, base += 64) {
            uint64_t x = words_[w];
            while(x) {
                int i = __builtin_ctzll(x);
                f(base + i);
                x ^= (uint64_t)1 << i;
            }
        }
    }
    
    template <typename F>
    void iterateBackwards(F f) const {
        int base = (W - 1) << 6;
        for(int w = W - 1; w >= 0; --w, base -= 64) {
            uint64_t x = words_[w];
            while(x) {
                int i = 63 - __builtin_clzll(x);
                f(base + i);
                x ^= (uint64_t)1 << i;
            }
        }
    }
    
    template <typename F>
    void iterateWhile(F f) const {
        int base = 0;
        for(int w = 0; w < W; ++w, base += 64) {
            uint64_t x = words_[w];
            while(x) {
                int i = __builtin_ctzll(x);
                if(!f(base + i)) {
                    return;
                }
                x ^= (uint64_t)1 << i;
            }
        }
    }
    
    template <typename F>
    void iterateUntil(F f) const {
        int base = 0;
        for(int w = 0; w < W; ++w, base += 64) {
            uint64_t x = words_[w];
            while(x) {
                int i = __builtin_ctzll(x);
                if(f(base + i)) {
                    return;
                }
                x ^= (uint64_t)1 << i;
            }
        }
    }
    
    void add(int x) {
        assert(x >= 0 && x < BitCount);
        
        words_[x >> 6] |= (uint64_t)1 << (x & 63);
    }
    
    void del(int x) {
        assert(x >= 0 && x < BitCount);
        
        words_[x >> 6] &= ~((uint64_t)1 << (x & 63));
    }
    
    void set(int x, bool val) {
        assert(x >= 0 && x < BitCount);
        
        words_[x >> 6] &= ~((uint64_t)1 << (x & 63));
        words_[x >> 6] |= (uint64_t)val << (x & 63);
    }
    
    void toggle(int x) {
        assert(x >= 0 && x < BitCount);
        
        words_[x >> 6] ^= (uint64_t)1 << (x & 63);
    }
    
    BitSet with(int x) const {
        assert(x >= 0 && x < BitCount);
        
        BitSet ret = *this;
        ret.add(x);
        return ret;
    }
    
    BitSet without(int x) const {
        assert(x >= 0 && x < BitCount);
        
        BitSet ret = *this;
        ret.del(x);
        return ret;
    }
    
    BitSet shifted(int d) const {
        if(d == 0) {
            return *this;
        }
        int a = (uint64_t)d >> 6;
        int b = (uint64_t)d & 63;
        
        BitSet ret(uninitializedTag_);
        
        if(b) {
            for(int w = 0; w < W; ++w) {
                int s1 = w - a - 1;
                int s2 = w - a;
                uint64_t word1 = (s1 >= 0 && s1 < W) ? words_[s1] : 0;
                uint64_t word2 = (s2 >= 0 && s2 < W) ? words_[s2] : 0;
                
                ret.words_[w] = (word1 >> (64 - b)) | (word2 << b);
            }
        } else {
            for(int w = 0; w < W; ++w) {
                int s = w - a;
                ret.words_[w] = (s >= 0 && s < W) ? words_[s] : 0;
            }
        }
        
        return ret;
    }
    
    void shift(int d) {
        *this = this->shifted(d);
    }
    
    BitSet inverted() const {
        BitSet ret = *this;
        ret.invert();
        return ret;
    }
    
    void invert() {
        for(int w = 0; w < W; ++w) {
            words_[w] = ~words_[w];
        }
    }
    
    int count() const {
        int ret = 0;
        for(int w = 0; w < W; ++w) {
            ret += __builtin_popcountll(words_[w]);
        }
        return ret;
    }
    
    uint64_t bottomWord() const {
        return words_[0];
    }
    
    int min() const {
        for(int w = 0; w < W; ++w) {
            if(words_[w]) {
                return 64 * w + __builtin_ctzll(words_[w]);
            }
        }
        assert(false);
        return -1;
    }
    
    int max() const {
        for(int w = W - 1; w >= 0; --w) {
            if(words_[w]) {
                return 64 * w + 63 - __builtin_clzll(words_[w]);
            }
        }
        assert(false);
        return -1;
    }
    
    int nth(int n) {
        for(int w = 0; w < W; ++w) {
            int c = __builtin_popcountll(words_[w]);
            if(n < c) {
                return 64 * w + __builtin_ctzll(_pdep_u64((uint64_t)1 << n, words_[w]));
            } else {
                n -= c;
            }
        }
        assert(false);
        return -1;
    }
    
    bool isEmpty() const {
        for(int w = 0; w < W; ++w) {
            if(words_[w]) {
                return false;
            }
        }
        return true;
    }
    
    bool isNonEmpty() const {
        return !isEmpty();
    }
    
    bool isSingleton() const {
        int w;
        for(w = 0; w < W; ++w) {
            if(words_[w]) {
                if(words_[w] != (words_[w] & -words_[w])) {
                    return false;
                }
                break;
            }
        }
        if(w == W) {
            return false;
        }
        ++w;
        for(; w < W; ++w) {
            if(words_[w]) {
                return false;
            }
        }
        return true;
    }
    
    bool isSingletonOrEmpty() const {
        int w;
        for(w = 0; w < W; ++w) {
            if(words_[w]) {
                if(words_[w] != (words_[w] & -words_[w])) {
                    return false;
                }
                break;
            }
        }
        ++w;
        for(; w < W; ++w) {
            if(words_[w]) {
                return false;
            }
        }
        return true;
    }
    
    bool operator==(const BitSet& other) const {
        return equal(words_, words_ + W, other.words_);
    }
    bool operator!=(const BitSet& other) const {
        return !equal(words_, words_ + W, other.words_);
    }
    bool operator<(const BitSet& other) const {
        for(int w = W - 1; w >= 0; --w) {
            if(words_[w] != other.words_[w]) {
                return words_[w] < other.words_[w];
            }
        }
        return false;
    }
    bool operator>(const BitSet& other) const {
        return other < *this;
    }
    bool operator<=(const BitSet& other) const {
        return !(*this > other);
    }
    bool operator>=(const BitSet& other) const {
        return !(*this < other);
    }
    
    static BitSet unpack(BitSet packed, const BitSet& mask) {
        BitSet ret(uninitializedTag_);
        for(int w = 0; w < W; ++w) {
            ret.words_[w] = _pdep_u64(packed.words_[0], mask.words_[w]);
            if(w != W - 1) {
                packed.shift(-__builtin_popcountll(mask.words_[w]));
            }
        }
        return ret;
    }
    
    static BitSet pack(const BitSet& unpacked, const BitSet& mask) {
        BitSet ret;
        for(int w = W - 1; w >= 0; --w) {
            if(w != W - 1) {
                ret.shift(__builtin_popcountll(mask.words_[w]));
            }
            ret.words_[0] |= _pext_u64(unpacked.words_[w], mask.words_[w]);
        }
        return ret;
    }
    
    void print(ostream& out) const {
        out << "BitSet<" << W << ">(";
        bool first = true;
        iterate([&](int x) {
            if(first) {
                first = false;
            } else {
                out << ", ";
            }
            out << x;
        });
        out << ")";
    }
    
private:
    struct UninitializedTag { };
    static const UninitializedTag uninitializedTag_;
    BitSet(UninitializedTag) { }
    
    uint64_t words_[W];
    
    friend BitSet setIntersection<W>(const BitSet<W>& a, const BitSet<W>& b);
    friend BitSet setUnion<W>(const BitSet<W>& a, const BitSet<W>& b);
    friend BitSet setDifference<W>(const BitSet<W>& a, const BitSet<W>& b);
    friend BitSet setSymmetricDifference<W>(const BitSet<W>& a, const BitSet<W>& b);
    friend bool isSubset<W>(const BitSet<W>& a, const BitSet<W>& b);
    friend class hash<BitSet<W>>;
    
    template <int W2>
    friend class BitSet;
};

constexpr int bitSetParam(int n) {
    assert(n >= 0);
    return (n + 63) >> 6;
}

template <int W>
struct BitSetParam {
    static const int Val = W;
};

template <int M, typename F>
void selectBitSetParam(int n, F f) {
    assert(n >= 0);
    select2Pow<ceilLog2(bitSetParam(M))>(bitSetParam(n), [&](auto sel) {
        f(BitSetParam<sel.Val>());
    });
};

template <int W>
BitSet<W> setIntersection(const BitSet<W>& a, const BitSet<W>& b) {
    BitSet<W> ret(BitSet<W>::uninitializedTag_);
    for(int w = 0; w < W; ++w) {
        ret.words_[w] = a.words_[w] & b.words_[w];
    }
    return ret;
}

template <int W>
BitSet<W> setUnion(const BitSet<W>& a, const BitSet<W>& b) {
    BitSet<W> ret(BitSet<W>::uninitializedTag_);
    for(int w = 0; w < W; ++w) {
        ret.words_[w] = a.words_[w] | b.words_[w];
    }
    return ret;
}

template <int W>
BitSet<W> setDifference(const BitSet<W>& a, const BitSet<W>& b) {
    BitSet<W> ret(BitSet<W>::uninitializedTag_);
    for(int w = 0; w < W; ++w) {
        ret.words_[w] = a.words_[w] & ~b.words_[w];
    }
    return ret;
}

template <int W>
BitSet<W> setSymmetricDifference(const BitSet<W>& a, const BitSet<W>& b) {
    BitSet<W> ret(BitSet<W>::uninitializedTag_);
    for(int w = 0; w < W; ++w) {
        ret.words_[w] = a.words_[w] ^ b.words_[w];
    }
    return ret;
}

template <int W>
bool isSubset(const BitSet<W>& a, const BitSet<W>& b) {
    for(int w = 0; w < W; ++w) {
        if(a.words_[w] & ~b.words_[w]) {
            return false;
        }
    }
    return true;
}

namespace std {

template <int W>
struct hash<BitSet<W>> {
    size_t operator()(const BitSet<W>& s) const {
        uint64_t h = 0;
        for(int i = 0; i < W; ++i) {
            hashCombine(h, s.words_[i]);
        }
        return (size_t)h;
    }
};

}
