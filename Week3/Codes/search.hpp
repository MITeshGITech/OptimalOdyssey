#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <cmath>
#include <limits>
#include "graph.hpp"

// Result bundle for a single search run.
struct SearchResult {
    bool path_found = false;
    int path_length = 0;       // number of nodes in the path (start..goal inclusive)
    int nodes_explored = 0;    // number of nodes popped off the open set (settled)
    double time_ms = 0.0;      // filled in by the caller using <chrono>
};

// Heuristic function signature: takes a node and the goal, returns an estimate.
using Heuristic = std::function<double(const Node&, const Node&)>;

// h(n) = 0 everywhere -> turns A* into plain Dijkstra.
inline double heuristic_zero(const Node& /*a*/, const Node& /*b*/) {
    return 0.0;
}

// Straight-line ("as the crow flies") distance.
inline double heuristic_euclidean(const Node& a, const Node& b) {
    double dy = static_cast<double>(a.y - b.y);
    double dx = static_cast<double>(a.x - b.x);
    return std::sqrt(dy * dy + dx * dx);
}

// Grid distance (sum of absolute differences) — admissible for 4-connected grids.
inline double heuristic_manhattan(const Node& a, const Node& b) {
    return std::abs(a.y - b.y) + std::abs(a.x - b.x);
}

// Generic A* search. Passing heuristic_zero makes this behave as Dijkstra.
// All moves between adjacent grid cells have a uniform cost of 1.
inline SearchResult a_star_search(const Graph& graph, Node start, Node goal,
                                   const Heuristic& h) {
    SearchResult result;

    struct OpenEntry {
        double f;
        int64_t id;
        Node node;
    };
    struct Compare {
        bool operator()(const OpenEntry& a, const OpenEntry& b) const {
            return a.f > b.f; // smallest f at the top of the priority_queue
        }
    };

    std::priority_queue<OpenEntry, std::vector<OpenEntry>, Compare> open_set;
    std::unordered_map<int64_t, double> g_score;
    std::unordered_map<int64_t, int64_t> came_from;
    std::unordered_set<int64_t> closed;

    int64_t start_id = graph.encode(start);
    int64_t goal_id = graph.encode(goal);

    g_score[start_id] = 0.0;
    open_set.push({h(start, goal), start_id, start});

    while (!open_set.empty()) {
        OpenEntry current = open_set.top();
        open_set.pop();

        // Skip stale entries (we may have pushed a node multiple times
        // with different f-scores before it was finalized).
        if (closed.find(current.id) != closed.end()) {
            continue;
        }
        closed.insert(current.id);
        result.nodes_explored++;

        if (current.id == goal_id) {
            // path_length is the number of steps (edges) from start to goal,
            // i.e. the path cost -- matches the assignment's expected output
            // convention (start==goal would be 0 steps).
            int steps = 0;
            int64_t walker = current.id;
            while (came_from.find(walker) != came_from.end()) {
                walker = came_from[walker];
                steps++;
            }
            result.path_found = true;
            result.path_length = steps;
            return result;
        }

        for (const Node& neighbor : graph.get_neighbors(current.node)) {
            int64_t neighbor_id = graph.encode(neighbor);
            double tentative_g = g_score[current.id] + 1.0; // uniform edge cost

            auto it = g_score.find(neighbor_id);
            if (it == g_score.end() || tentative_g < it->second) {
                g_score[neighbor_id] = tentative_g;
                came_from[neighbor_id] = current.id;
                double f = tentative_g + h(neighbor, goal);
                open_set.push({f, neighbor_id, neighbor});
            }
        }
    }

    // Open set exhausted without reaching goal: no path exists.
    result.path_found = false;
    result.path_length = 0;
    return result;
}
