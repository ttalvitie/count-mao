#include "mao.hpp"
#include "he_et_al_2016.hpp"

Z countMAOUsingHeEtAl2016(const GraphData& graphData) {
    Z ret;
    graphData.accessGraph([&](auto graph) {
        assert(isValidEssentialGraph(graph));
        ret = he_et_al_2016::computeSize(graph);
    });
    return ret;
}
