

#include <bits/stdc++.h>
using namespace std;


struct JsonValue {
    enum Type { NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT } type = NUL;
    bool b = false;
    double num = 0;
    string str;
    vector<JsonValue> arr;
    vector<pair<string, JsonValue>> obj; // preserves insertion order

    bool isNull() const { return type == NUL; }

    const JsonValue* find(const string& key) const {
        for (auto& kv : obj) if (kv.first == key) return &kv.second;
        return nullptr;
    }

    double asNumber(double def = 0) const { return type == NUMBER ? num : def; }
    long long asInt(long long def = 0) const { return type == NUMBER ? (long long)llround(num) : def; }
    string asString(const string& def = "") const { return type == STRING ? str : def; }
};

class JsonParser {
public:
    explicit JsonParser(const string& s) : s_(s), i_(0), n_(s.size()) {}

    JsonValue parse() {
        skipWs();
        JsonValue v = parseValue();
        return v;
    }

private:
    const string& s_;
    size_t i_, n_;

    void skipWs() {
        while (i_ < n_ && (s_[i_] == ' ' || s_[i_] == '\t' || s_[i_] == '\n' || s_[i_] == '\r')) i_++;
    }

    char peek() const { return i_ < n_ ? s_[i_] : '\0'; }

    JsonValue parseValue() {
        skipWs();
        char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return parseString();
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        return parseNumber();
    }

    JsonValue parseObject() {
        JsonValue v; v.type = JsonValue::OBJECT;
        i_++; // {
        skipWs();
        if (peek() == '}') { i_++; return v; }
        while (true) {
            skipWs();
            JsonValue key = parseString();
            skipWs();
            
            if (peek() == ':') i_++;
            JsonValue val = parseValue();
            v.obj.emplace_back(key.str, val);
            skipWs();
            if (peek() == ',') { i_++; continue; }
            if (peek() == '}') { i_++; break; }
            break;
        }
        return v;
    }

    JsonValue parseArray() {
        JsonValue v; v.type = JsonValue::ARRAY;
        i_++; // [
        skipWs();
        if (peek() == ']') { i_++; return v; }
        while (true) {
            JsonValue val = parseValue();
            v.arr.push_back(val);
            skipWs();
            if (peek() == ',') { i_++; continue; }
            if (peek() == ']') { i_++; break; }
            break;
        }
        return v;
    }

    JsonValue parseString() {
        JsonValue v; v.type = JsonValue::STRING;
     
        i_++; 
        string out;
        while (i_ < n_ && s_[i_] != '"') {
            char c = s_[i_];
            if (c == '\\' && i_ + 1 < n_) {
                char nx = s_[i_ + 1];
                switch (nx) {
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'r': out += '\r'; break;
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case '/': out += '/'; break;
                    default: out += nx; break;
                }
                i_ += 2;
            } else {
                out += c;
                i_++;
            }
        }
        if (i_ < n_) i_++; 
        v.str = out;
        return v;
    }

    JsonValue parseBool() {
        JsonValue v; v.type = JsonValue::BOOL;
        if (s_.compare(i_, 4, "true") == 0) { v.b = true; i_ += 4; }
        else if (s_.compare(i_, 5, "false") == 0) { v.b = false; i_ += 5; }
        return v;
    }

    JsonValue parseNull() {
        JsonValue v; v.type = JsonValue::NUL;
        if (s_.compare(i_, 4, "null") == 0) i_ += 4;
        return v;
    }

    JsonValue parseNumber() {
        JsonValue v; v.type = JsonValue::NUMBER;
        size_t start = i_;
        if (peek() == '-' || peek() == '+') i_++;
        while (i_ < n_ && (isdigit((unsigned char)s_[i_]) || s_[i_] == '.' || s_[i_] == 'e' || s_[i_] == 'E' || s_[i_] == '+' || s_[i_] == '-')) {
          
            if ((s_[i_] == '+' || s_[i_] == '-') && i_ != start && !(s_[i_-1]=='e' || s_[i_-1]=='E')) break;
            i_++;
        }
        v.num = atof(s_.substr(start, i_ - start).c_str());
        return v;
    }
};

static string readFile(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) {
        throw runtime_error("Could not open file: " + path);
    }
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}


//for JSON


static string jsonEscape(const string& s) {
    string out;
    for (char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}


// Graph representation + Dijkstra 

struct Graph {
    int numNodes = 0;
    // adjacency list
    vector<vector<pair<int,double>>> adj;
    vector<int> nodeIds;         
    unordered_map<int,int> idToIdx; 

    int indexOf(int originalId) const {
        auto it = idToIdx.find(originalId);
        if (it == idToIdx.end()) return -1;
        return it->second;
    }

    static Graph fromJson(const JsonValue& root) {
        Graph g;
        const JsonValue* nodesVal = root.find("nodes");
        const JsonValue* edgesVal = root.find("edges");

        if (nodesVal && nodesVal->type == JsonValue::ARRAY) {
            for (auto& nv : nodesVal->arr) {
                int id = (int)nv.asInt();
                g.idToIdx[id] = g.nodeIds.size();
                g.nodeIds.push_back(id);
            }
        }
        g.numNodes = g.nodeIds.size();
        g.adj.assign(g.numNodes, {});

        if (edgesVal && edgesVal->type == JsonValue::ARRAY) {
            for (auto& ev : edgesVal->arr) {
                int u = (int)ev.find("u")->asInt();
                int v = (int)ev.find("v")->asInt();
                double w = ev.find("w")->asNumber();
                int ui = g.indexOf(u);
                int vi = g.indexOf(v);
                if (ui < 0 || vi < 0) continue; // skip malformed edge
                g.adj[ui].push_back({vi, w});
                g.adj[vi].push_back({ui, w});
            }
        }
        return g;
    }

    // Dijkstra shortest path 
    vector<double> dijkstra(int srcIdx) const {
        vector<double> dist(numNodes, numeric_limits<double>::infinity());
        if (srcIdx < 0 || srcIdx >= numNodes) return dist;
        dist[srcIdx] = 0.0;
        using PQItem = pair<double,int>;
        priority_queue<PQItem, vector<PQItem>, greater<PQItem>> pq;
        pq.push({0.0, srcIdx});
        vector<bool> visited(numNodes, false);
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (visited[u]) continue;
            visited[u] = true;
            for (auto& [v, w] : adj[u]) {
                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }
};


// TSP solver
struct TSPResult {
    double cost = 0.0;
    vector<int> tour; 
    bool feasible = true; 
};

static const double INF = numeric_limits<double>::infinity();

// TSPBruteForce

class TSPBruteForce {
public:

    static TSPResult solve(const vector<vector<double>>& dist, const vector<int>& origIds) {
        int n = (int)dist.size();
        TSPResult result;

        if (n <= 1) {
            result.cost = 0;
            if (n == 1) result.tour = {origIds[0], origIds[0]};
            return result;
        }

        vector<int> perm;
        for (int i = 1; i < n; i++) perm.push_back(i);

        double bestCost = INF;
        vector<int> bestPerm;

        do {
            double cost = 0;
            int prev = 0;
            bool ok = true;
            for (int city : perm) {
                if (!isfinite(dist[prev][city])) { ok = false; break; }
                cost += dist[prev][city];
                prev = city;
            }
            if (ok) {
                if (!isfinite(dist[prev][0])) ok = false;
                else cost += dist[prev][0];
            }
            if (ok && cost < bestCost) {
                bestCost = cost;
                bestPerm = perm;
            }
        } while (next_permutation(perm.begin(), perm.end()));

        if (!isfinite(bestCost)) {
            result.feasible = false;
            result.cost = -1;
            return result;
        }

        result.cost = bestCost;
        result.tour.push_back(origIds[0]);
        for (int city : bestPerm) result.tour.push_back(origIds[city]);
        result.tour.push_back(origIds[0]);
        return result;
    }
};

// TSPOptimized: Held-Karp 
class TSPOptimized {
public:
    static TSPResult solve(const vector<vector<double>>& dist, const vector<int>& origIds) {
        int n = (int)dist.size();
        TSPResult result;

        if (n <= 1) {
            result.cost = 0;
            if (n == 1) result.tour = {origIds[0], origIds[0]};
            return result;
        }

        int full = 1 << n;
        vector<vector<double>> dp(full, vector<double>(n, INF));
        vector<vector<int>> parent(full, vector<int>(n, -1));

        dp[1 /* {0} */][0] = 0.0;

        for (int mask = 1; mask < full; mask++) {
            if (!(mask & 1)) continue; 
            for (int last = 0; last < n; last++) {
                if (!(mask & (1 << last))) continue;
                double curCost = dp[mask][last];
                if (!isfinite(curCost)) continue;
                for (int nxt = 0; nxt < n; nxt++) {
                    if (mask & (1 << nxt)) continue; // already visited
                    if (!isfinite(dist[last][nxt])) continue;
                    int nmask = mask | (1 << nxt);
                    double ncost = curCost + dist[last][nxt];
                    if (ncost < dp[nmask][nxt]) {
                        dp[nmask][nxt] = ncost;
                        parent[nmask][nxt] = last;
                    }
                }
            }
        }

        int finalMask = full - 1;
        double bestCost = INF;
        int bestLast = -1;
        for (int last = 1; last < n; last++) {
            if (!isfinite(dp[finalMask][last])) continue;
            if (!isfinite(dist[last][0])) continue;
            double total = dp[finalMask][last] + dist[last][0];
            if (total < bestCost) {
                bestCost = total;
                bestLast = last;
            }
        }

        if (n >= 2 && bestLast == -1) {
            result.feasible = false;
            result.cost = -1;
            return result;
        }

        result.cost = bestCost;


        vector<int> path;
        int mask = finalMask, cur = bestLast;
        while (cur != -1) {
            path.push_back(cur);
            int p = parent[mask][cur];
            mask ^= (1 << cur);
            cur = p;
        }
        reverse(path.begin(), path.end())

        result.tour.push_back(origIds[0]);
        for (size_t k = 1; k < path.size(); k++) result.tour.push_back(origIds[path[k]]);
        result.tour.push_back(origIds[0]);
        return result;
    }
};

// Build NxN distance matrix 

static vector<vector<double>> buildDistanceMatrix(const Graph& g, const vector<int>& queryNodeIds) {
    int n = (int)queryNodeIds.size();
    vector<vector<double>> dist(n, vector<double>(n, INF));

    vector<int> localIdx(n);
    for (int i = 0; i < n; i++) localIdx[i] = g.indexOf(queryNodeIds[i]);

    for (int i = 0; i < n; i++) {
        if (localIdx[i] < 0) continue; 
        vector<double> fromI = g.dijkstra(localIdx[i]);
        for (int j = 0; j < n; j++) {
            if (localIdx[j] < 0) continue;
            dist[i][j] = fromI[localIdx[j]];
        }
    }
    return dist;
}


// Output 


static string tourToJsonArray(const vector<int>& tour) {
    string out = "[";
    for (size_t i = 0; i < tour.size(); i++) {
        out += to_string(tour[i]);
        if (i + 1 < tour.size()) out += ",";
    }
    out += "]";
    return out;
}

static string costToJson(double cost) {
    if (!isfinite(cost)) return "null";
    if (cost == llround(cost)) return to_string((long long)llround(cost));
    ostringstream oss;
    oss << cost;
    return oss.str();
}

// main


int main(int argc, char** argv) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <graph.json> <queries.json> <output.json> [both|brute|opt]\n";
        return 1;
    }

    string graphPath = argv[1];
    string queryPath = argv[2];
    string outPath = argv[3];
    string solverMode = (argc >= 5) ? argv[4] : "both";

    try {
        string graphText = readFile(graphPath);
        string queryText = readFile(queryPath);

        JsonValue graphJson = JsonParser(graphText).parse();
        JsonValue queryJson = JsonParser(queryText).parse();

        Graph g = Graph::fromJson(graphJson);

        const JsonValue* eventsVal = queryJson.find("events");
        const JsonValue* metaVal = queryJson.find("meta");
        string runId = metaVal && metaVal->find("id") ? metaVal->find("id")->asString("assignment_02") : "assignment_02";

        ostringstream out;
        out << "{\n";
        out << "    \"meta\": {\n";
        out << "        \"id\": \"" << jsonEscape(runId) << "\"\n";
        out << "    },\n";
        out << "    \"results\": [\n";

        bool firstResult = true;

        if (eventsVal && eventsVal->type == JsonValue::ARRAY) {
            for (auto& ev : eventsVal->arr) {
                const JsonValue* typeVal = ev.find("type");
                string type = typeVal ? typeVal->asString("") : "";

                if (type != "tsp") {
                    continue;
                }

                int id = (int)(ev.find("id") ? ev.find("id")->asInt() : 0);
                const JsonValue* nodesVal = ev.find("nodes");
                vector<int> queryNodes;
                if (nodesVal && nodesVal->type == JsonValue::ARRAY) {
                    for (auto& nv : nodesVal->arr) queryNodes.push_back((int)nv.asInt());
                }

                //  NxN distance matrix 
                vector<vector<double>> distMatrix = buildDistanceMatrix(g, queryNodes);

                // Pass matrix tosolver
                TSPResult bruteRes, optRes;
                bool runBrute = (solverMode == "both" || solverMode == "brute");
                bool runOpt   = (solverMode == "both" || solverMode == "opt");

                if (runBrute) bruteRes = TSPBruteForce::solve(distMatrix, queryNodes);
                if (runOpt)   optRes   = TSPOptimized::solve(distMatrix, queryNodes);

                if (!firstResult) out << ",\n";
                firstResult = false;

                out << "        {\n";
                out << "            \"id\": " << id << ",\n";
                out << "            \"query_id\": " << id << ",\n";

                if (solverMode == "both") {
                    out << "            \"brute_force\": {\n";
                    out << "                \"optimal_cost\": " << costToJson(bruteRes.cost) << ",\n";
                    out << "                \"tour\": " << tourToJsonArray(bruteRes.tour) << "\n";
                    out << "            },\n";
                    out << "            \"held_karp\": {\n";
                    out << "                \"optimal_cost\": " << costToJson(optRes.cost) << ",\n";
                    out << "                \"tour\": " << tourToJsonArray(optRes.tour) << "\n";
                    out << "            },\n";
                    
                    out << "            \"cost\": " << costToJson(optRes.cost) << "\n";
                } else if (solverMode == "brute") {
                    out << "            \"algorithm\": \"brute_force\",\n";
                    out << "            \"cost\": " << costToJson(bruteRes.cost) << ",\n";
                    out << "            \"tour\": " << tourToJsonArray(bruteRes.tour) << "\n";
                } else { // "opt"
                    out << "            \"algorithm\": \"held_karp\",\n";
                    out << "            \"cost\": " << costToJson(optRes.cost) << ",\n";
                    out << "            \"tour\": " << tourToJsonArray(optRes.tour) << "\n";
                }

                out << "        }";
            }
        }

        out << "\n    ]\n";
        out << "}\n";

        ofstream ofs(outPath);
        ofs << out.str();
        ofs.close();

        cerr << "Wrote results to " << outPath << " (solver mode: " << solverMode << ")\n";

    } catch (exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
