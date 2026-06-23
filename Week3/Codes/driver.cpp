#include<iostream>
#include<fstream>
#include<string>
#include<algorithm>
#include"json.hpp"
#include<chrono>
#include"graph.hpp"
#include"search.hpp"

// Runs one of the three algorithms and packages timing + result into JSON.
static nlohmann::json run_algorithm(const Graph& map, Node start, Node goal,
                                     const Heuristic& h) {
    auto t_start = std::chrono::high_resolution_clock::now();
    SearchResult res = a_star_search(map, start, goal, h);
    auto t_end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

    nlohmann::json out;
    out["path_found"] = res.path_found;
    out["path_length"] = res.path_length;
    out["nodes_explored"] = res.nodes_explored;
    out["time_ms"] = elapsed_ms;
    return out;
}

int main(int argc, char* argv[]){
    if (argc < 4) {
        std::cerr << "Usage: ./{executable} <graph.json> <queries.json> <output.json>\n";
        return 1;
    }
    std::string graph_json_file = argv[1];
    std::string query_json_file = argv[2];
    std::string output_file = argv[3];
    std::ifstream file1(graph_json_file);
    if (!file1.is_open()) {
        std::cerr << "Error: Could not open " << graph_json_file << '\n';
        return 1;
    }
    nlohmann::json graph_json;
    file1 >> graph_json; // reading the graph_json file into json

    Graph map(graph_json);

    std::ifstream file2(query_json_file);
    if (!file2.is_open()) {
        std::cerr << "Error: Could not open " << query_json_file << '\n';
        return 1;
    }
    nlohmann::json query_json;
    file2 >> query_json; // reading the query_json file into json object
    nlohmann::json output_json;
    output_json["meta"] = {{"id", query_json["meta"]["id"]}};
    output_json["results"] = nlohmann::json::array();
    std::string type;
    for(auto event : query_json["events"]){
        type = event["type"];

        if (type == "find_path") {
            Node start{ event["start"]["y"], event["start"]["x"] };
            Node goal { event["goal"]["y"],  event["goal"]["x"]  };

            nlohmann::json out;
            out["id"] = event["id"];
            out["dijkstra"]        = run_algorithm(map, start, goal, heuristic_zero);
            out["astar_euclidean"] = run_algorithm(map, start, goal, heuristic_euclidean);
            out["astar_manhattan"] = run_algorithm(map, start, goal, heuristic_manhattan);

            output_json["results"].push_back(out);
        }
    }

    std::ofstream out_stream(output_file);
    if (!out_stream.is_open()) {
        std::cerr << "Error: Could not open " << output_file << " for writing\n";
        return 1;
    }
    out_stream << output_json.dump(4);

    std::cout << "Wrote results to " << output_file << std::endl;

    return 0;
}
