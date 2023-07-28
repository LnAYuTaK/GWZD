#include "func_node.h"

FuncNode::FuncNode(const NodeFunc &func, const std::string &help) 
                :Node(help)
                ,func_(func)
{ 

}

void FuncNode::execute(const Session &s, const Args &a) const
{
    if (func_){
      func_(s,a);
    }
}

