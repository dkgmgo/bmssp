#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <fstream>
#include <string>
#include <boost/graph/graphml.hpp>

#include  "utils.hpp"

using namespace std;

struct BGP_Info {
    struct Vertex {
        string Country;
        string name;
        long prefixNum;
        long prefixAll;
        long asTime;
        long addAll;
        long addNum;
        string asNumber;
        long pathNum;
        bool active=true;
        string country;
    };

    struct Edge {
        long pathCount;
        long edgeTime;
        long count;
        long  prefcount;
        long addCount=0;
        string cableName;
        string id;
        double oot=0.0;
        double ot=0.0;
        double ocurv=0.0;
        double curv=0.0;
        double dist=1.0;
        double odistance=1.0;
        double distance=1.0;
        long weight=1;
    };

    static boost::dynamic_properties get_properties(boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Vertex, Edge> &g) {
        boost::dynamic_properties dp;
        dp.property("asNumber", get(&Vertex::asNumber, g));
        dp.property("pathNum", get(&Vertex::pathNum, g));
        dp.property("Country", get(&Vertex::Country, g));
        dp.property("country", get(&Vertex::country, g));
        dp.property("Name", get(&Vertex::name, g));
        dp.property("asTime", get(&Vertex::asTime, g));
        dp.property("prefixNum", get(&Vertex::prefixNum, g));
        dp.property("prefixAll", get(&Vertex::prefixAll, g));
        dp.property("addAll", get(&Vertex::addAll, g));
        dp.property("addNum", get(&Vertex::addNum, g));

        dp.property("pathCount", get(&Edge::pathCount, g));
        dp.property("addCount", get(&Edge::addCount, g));
        dp.property("edgeTime", get(&Edge::edgeTime, g));
        dp.property("weight", get(&Edge::weight, g));
        dp.property("prefCount", get(&Edge::prefcount, g));
        dp.property("count", get(&Edge::count, g));
        dp.property("cable",get(&Edge::cableName, g));
        dp.property("id",get(&Edge::id, g));

        dp.property("ot", get(&Edge::ot, g));
        dp.property("distance", get(&Edge::distance, g));
        dp.property("curv", get(&Edge::curv, g));
        dp.property("oot",get(&Edge::oot, g));
        dp.property("ocurv",get(&Edge::ocurv, g));
        dp.property("odistance",get(&Edge::odistance, g));
        
        return dp;
    }

    static double get_edge_weight(const Edge &e) {
        return e.distance; //TODO distance or weight ?
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

        Graph adjacencyMap;
        for (auto e : boost::make_iterator_range(boost::edges(g))) {
            Node_id_T src = boost::source(e, g);
            Node_id_T dest = boost::target(e, g);
            Dist_T weight = Info::get_edge_weight(g[e]);

            adjacencyMap[src][dest] = weight;
            adjacencyMap[dest][src] = weight; //it's undirected
        }

        if (verbose) cout << "Graph read successfully" << endl;

        return {adjacencyMap, num_vertices(g)};
    }

    static pair<Graph, int> read_bgp_graphml(const string &filename, bool verbose=true) {
        return read_graphml<BGP_Info>(filename, verbose);
    }
};

#endif