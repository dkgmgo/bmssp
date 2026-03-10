#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <fstream>
#include <string>
#include <boost/graph/graphml.hpp>
#include "../common.hpp"

using namespace std;

struct BGP_Info {
    struct Vertex {
        string name;
    };

    struct Edge {
        string id;
        double curv=0.0;
        double distance=1.0;
    };

    static boost::dynamic_properties get_properties(boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge> &g) {
        boost::dynamic_properties dp(boost::ignore_other_properties);
        dp.property("Name", get(&Vertex::name, g));
        dp.property("name", get(&Vertex::name, g));
        dp.property("id",get(&Edge::id, g));
        dp.property("distance", get(&Edge::distance, g));
        dp.property("curv", get(&Edge::curv, g));

        return dp;
    }

    static double get_edge_weight(const Edge &e) {
        return e.distance;
    }
};

class FileUtils {
public:
    template<typename Info>
    static pair<Graph, int> read_graphml(string filename, bool verbose) {
        boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, typename Info::Vertex, typename Info::Edge> g;
        boost::dynamic_properties dp = Info::get_properties(g);

        if (verbose) cout << "Reading graphml file: " << filename << endl;

        ifstream file(filename);
        if (!file) {
            throw runtime_error("Cannot open file: " + filename);
        }

        try {
            boost::read_graphml(file, g, dp);
        } catch (const exception& e) {
            throw runtime_error("Error reading GraphML: " + string(e.what()));
        }

        Graph sortie;
        for (auto e : boost::make_iterator_range(boost::edges(g))) {
            Node_id_T src = boost::source(e, g);
            Node_id_T dest = boost::target(e, g);
            Dist_T weight = Info::get_edge_weight(g[e]);

            boost::add_edge(src, dest, weight, sortie);
            boost::add_edge(dest, src, weight, sortie); //it's undirected
        }

        if (verbose) cout << "Graph read successfully" << endl;

        return {sortie, num_vertices(sortie)};
    }

    static pair<Graph, int> read_bgp_graphml(const string &filename, bool verbose=true) {
        return read_graphml<BGP_Info>(filename, verbose);
    }

    static void export_to_dot(const string &filename, const Graph& g) {
        ofstream file(filename);
        boost::write_graphviz(file, g, boost::default_writer(), boost::make_label_writer(boost::get(boost::edge_weight, g)));
    }
};

#endif //FILE_UTILS_HPP
