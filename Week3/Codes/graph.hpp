#pragma once

#include <vector>
#include <unordered_set>
#include <cstdint>
#include "json.hpp"

// A single cell on the grid.
// y = row, x = col (matches the {"y":.., "x":..} convention in the JSON files).
struct Node {
    int y;
    int x;

    bool operator==(const Node& other) const {
        return y == other.y && x == other.x;
    }
};

// Graph represents the grid map loaded from graph.json.
// It stores the dimensions and which cells are blocked (obstacles),
// and can produce the valid neighbors of any given cell.
class Graph {
public:
    int rows;
    int cols;

    // Build the graph directly from the parsed graph.json object.
    explicit Graph(const nlohmann::json& graph_json) {
        rows = graph_json["grid_size"]["rows"];
        cols = graph_json["grid_size"]["cols"];

        if (graph_json.contains("obstacles")) {
            for (const auto& obs : graph_json["obstacles"]) {
                int y = obs["y"];
                int x = obs["x"];
                obstacles_.insert(encode(y, x));
            }
        }
    }

    // True if (y, x) is inside the grid bounds.
    bool in_bounds(int y, int x) const {
        return y >= 0 && y < rows && x >= 0 && x < cols;
    }

    // True if (y, x) is marked as an obstacle.
    bool is_obstacle(int y, int x) const {
        return obstacles_.find(encode(y, x)) != obstacles_.end();
    }

    // Returns every neighbor of `current` that is in bounds and walkable.
    // Uses 4-connectivity (up, down, left, right) since the heuristics in
    // this assignment (Manhattan / Euclidean) are designed for that.
    std::vector<Node> get_neighbors(Node current) const {
        std::vector<Node> neighbors;
        // dy, dx pairs for up, down, left, right
        static const int dy[4] = {-1, 1, 0, 0};
        static const int dx[4] = {0, 0, -1, 1};

        for (int i = 0; i < 4; ++i) {
            int ny = current.y + dy[i];
            int nx = current.x + dx[i];
            if (in_bounds(ny, nx) && !is_obstacle(ny, nx)) {
                neighbors.push_back(Node{ny, nx});
            }
        }
        return neighbors;
    }

    // Encode (y, x) into a single integer id, useful for hashing/visited sets.
    int64_t encode(int y, int x) const {
        return static_cast<int64_t>(y) * cols + x;
    }

    int64_t encode(Node n) const {
        return encode(n.y, n.x);
    }

private:
    std::unordered_set<int64_t> obstacles_;
};
