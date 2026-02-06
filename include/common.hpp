#ifndef COMMON_HPP
#define COMMON_HPP

#include <boost/graph/adjacency_list.hpp>

#define INF 10000000
using Dist_T = double;
using Node_id_T = int;
using VertexProp = boost::property<boost::vertex_name_t, Node_id_T>;
using EdgeProp = boost::property<boost::edge_weight_t, Dist_T>;
using Dist_List_T = std::vector<Dist_T>;
using Prev_List_T = std::vector<Node_id_T>;
using Graph = boost::adjacency_list <boost::vecS, boost::vecS, boost::directedS, VertexProp, EdgeProp>; //TODO try CSR graphs

#endif //COMMON_HPP
