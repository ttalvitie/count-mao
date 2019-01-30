#include "graph.hpp"
#include "mao.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    cin.sync_with_stdio(false);
    cin.tie(nullptr);
    
    removeStackLimit();
    
    if(argc != 2) {
        cerr << "Usage: <method name prefix>\n";
        cerr << "Available methods:\n";
        for(auto p : countMAOMethods) {
            cerr << "  \"" << p.first << "\"\n";
        }
        fail("Invalid command line");
    }
    string namePrefix = argv[1];
    
    Z (*method)(const GraphData&) = nullptr;
    for(auto p : countMAOMethods) {
        if(p.first.substr(0, namePrefix.size()) == namePrefix) {
            if(method != nullptr) {
                fail("Ambiguous method name");
            }
            method = p.second;
        }
    }
    if(method == nullptr) {
        fail("Method not found");
    }
    
    GraphData graphData = GraphData::read(cin);
    cout << method(graphData) << "\n";
    
    return 0;
}
