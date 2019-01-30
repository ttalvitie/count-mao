#include "mao.hpp"

namespace {

template <int N>
Z count(const Graph<N>& graph) {
    uint64_t ret = 0;
    iterateMAO(graph, [&](const Graph<N>&) {
        ++ret;
    });
    return (Z)ret;
}

}

Z countMAOUsingEnumeration(const GraphData& graphData) {
    Z ret;
    graphData.accessGraph([&](auto graph) {
        ret = count(graph);
    });
    return ret;
}
