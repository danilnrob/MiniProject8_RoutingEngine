#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <string>
using namespace std;

struct Edge {
    int to;
    double bandwidth;
};

struct Node {
    int id;
    vector<Edge> edges;
};

class RoutingEngine {
private:
    vector<unique_ptr<Node>> nodes;
    int numNodes;

    bool dfs(int curr, int dest, double minBW,
             vector<bool>& visited, vector<int>& path) {
        visited[curr] = true;
        path.push_back(curr);

        if (curr == dest) {
            return true;
        }

        for (const auto& edge : nodes[curr]->edges) {
            if (!visited[edge.to] && edge.bandwidth >= minBW) {
                if (dfs(edge.to, dest, minBW, visited, path)) {
                    return true;
                }
            }
        }

        path.pop_back();   // backtrack
        return false;
    }

public:
    RoutingEngine(int n) : numNodes(n) {
        for (int i = 0; i < n; i++) {
            unique_ptr<Node> node = make_unique<Node>();
            node->id = i;
            nodes.push_back(move(node));
        }
    }

    void loadFromFile(const string& filename) {
        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        int u, v;
        double bw;

        while (file >> u >> v >> bw) {
            if (u >= 0 && u < numNodes && v >= 0 && v < numNodes) {
                nodes[u]->edges.push_back({v, bw});
            } else {
                cerr << "Skipping invalid edge: "
                     << u << " " << v << " " << bw << endl;
            }
        }

        file.close();
    }

    void printGraph() const {
        for (const auto& node : nodes) {
            cout << "Node " << node->id << ": ";
            for (const auto& e : node->edges) {
                cout << "-(" << e.bandwidth << ")-> " << e.to << "  ";
            }
            cout << endl;
        }
    }

    vector<int> findPath(int source, int dest, double minBW) {
        vector<bool> visited(numNodes, false);
        vector<int> path;

        if (dfs(source, dest, minBW, visited, path)) {
            cout << "Path found (min BW >= " << minBW << "): ";
            for (int i = 0; i < path.size(); i++) {
                cout << path[i];
                if (i < path.size() - 1) {
                    cout << " -> ";
                }
            }
            cout << endl;
        } else {
            cout << "No feasible path found." << endl;
        }

        return path;
    }
};

int main() {
    RoutingEngine engine(6);   // nodes 0 through 5
    engine.loadFromFile("network.txt");

    cout << "=== Network Topology ===" << endl;
    engine.printGraph();

    cout << endl;
    cout << "=== Finding paths ===" << endl;
    engine.findPath(0, 5, 50);
    engine.findPath(0, 5, 100);
    engine.findPath(0, 5, 200);

    return 0;
}

/*  
Why does RoutingEngine use unique_ptr<Node>? (sole ownership)
- RoutingEngine uses unique_ptr<Node> because no other component needs to share ownership of the Node objects. 
  Unique_ptr represents this relationship and it automatically handles memory cleanup, preventing memory leaks. 

Why are edges stored by value in vector<Edge>? (small, copyable)
- Edges are stored by value in vector<Edge> because the Edge itself is small and copyable. Storing this small
  Edge struct is effective because it is simplier and requires less overhead compared to using pointers. 

Why is visited on the stack? (short-lived, small)
- Visted is on the stack because it is only needed during the depth first search. Once the function ends the
visited vector is no longer needed and will be destroyed/cleaned up automatically. 

Where would shared_ptr be useful? (shared adjacency, multi-path search)
- Shared_ptr would be useful in multiple situations that have multiple shared owners. This use could help to avoid
  premature deletion, but in our case, we do not need it because we only have one owner and don't need to share
  ownership.

What would break if you used raw new without cleanup?
- If we used raw new without any cleanup, the nodes would never be deleted, causing a memory leaks. Memory leaks would
  then lead to a discrepency in memory usage and could eventually cause the program to crash if it runs out of memory.
*/