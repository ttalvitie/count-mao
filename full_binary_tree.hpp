#pragma once

#include "common.hpp"

template <typename NodeData, typename LeafData>
class FullBinaryTree {
public:
    typedef FullBinaryTree<NodeData, LeafData> Self;
    
    struct Node : public NodeData {
        FullBinaryTree<NodeData, LeafData> left;
        FullBinaryTree<NodeData, LeafData> right;
        
        Node(
            NodeData nodeData,
            FullBinaryTree<NodeData, LeafData> left,
            FullBinaryTree<NodeData, LeafData> right
        )
            : NodeData(move(nodeData)),
              left(move(left)),
              right(move(right))
        { }
    };
    
    typedef LeafData Leaf;
    
    FullBinaryTree() { }
    
    FullBinaryTree(NodeData nodeData, Self left, Self right)
        : tree_(make_unique<variant<Node, Leaf>>(Node(move(nodeData), move(left), move(right))))
    { }
    
    FullBinaryTree(LeafData leafData)
        : tree_(make_unique<variant<Node, Leaf>>(Leaf(move(leafData))))
    { }
    
    template <typename NodeVisitor, typename LeafVisitor>
    auto visit(NodeVisitor nodeVisitor, LeafVisitor leafVisitor) const {
        assert(tree_);
        return lambdaVisit(*tree_,
            [&](const Node& node) {
                return nodeVisitor(node);
            },
            [&](const Leaf& node) {
                return leafVisitor(node);
            }
        );
    }
    template <typename NodeVisitor, typename LeafVisitor>
    auto visit(NodeVisitor nodeVisitor, LeafVisitor leafVisitor) {
        assert(tree_);
        return lambdaVisit(*tree_,
            [&](Node& node) {
                return nodeVisitor(node);
            },
            [&](Leaf& node) {
                return leafVisitor(node);
            }
        );
    }
    
private:
    unique_ptr<variant<Node, Leaf>> tree_;
};
