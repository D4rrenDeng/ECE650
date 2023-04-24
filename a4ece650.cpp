#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>
#include <iterator>
#include <memory>
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"

void getMinVertexCover(int, std::unordered_map<int, std::vector<int>>& graph);
std::vector<int> getVertexCover(int, int, std::unordered_map<int, std::vector<int>>& graph);

int parseCommandV(std::string line) {
    std::regex regex(R"(-?\d+)");
    std::smatch match;
    std::regex_search(line, match, regex);
    int maxIndex = std::stoi(match.str());
    if (maxIndex <= 0) {
        std::cerr << "Error: invalid command!";
    }

    return maxIndex;
}

void parseCommandE(std::string line, int maxIndex, std::unordered_map<int, std::vector<int>>& graph) {
    std::regex regex(R"(-?\d+)");
    std::smatch match;
    std::string temp = line; 
    // check the validity of this command
    while (std::regex_search(temp, match, regex)) {
        int m = std::stoi(match.str());
        temp = match.suffix();
        std::regex_search(temp, match, regex);
        int n = std::stoi(match.str());
        temp = match.suffix();
        if (m < 0 || n < 0 || m >= maxIndex || n >= maxIndex) {
            std::cerr << "Error: invalid vertex index!";
            return;
        }
        if (m == n) {
            std::cerr << "Error: invalid edge!";
            return;
        }
    }
    temp = line;
    graph.clear();
    // create graph from input string
    while (std::regex_search(temp, match, regex)) {
        int m = std::stoi(match.str());
        temp = match.suffix();
        std::regex_search(temp, match, regex);
        int n = std::stoi(match.str());
        temp = match.suffix();
        graph[m].push_back(n);
        graph[n].push_back(m);
    }
}

std::vector<int> getVertexCover(int k, int maxIndex, std::unordered_map<int, std::vector<int>> &graph) {
    Minisat::Solver solver;
    Minisat::Var prop[maxIndex][k];
    std::set<int> visitedVertex;

    for (int i = 0; i < maxIndex; i++) {
        for (int j = 0; j < k; j++) {
            prop[i][j] = solver.newVar();
        }
    }

    //1. At least one vertex is the ith vertex in the vertex cover:
    for (int i = 0; i < k; i++) {
        Minisat::vec<Minisat::Lit> clause;
        for (int j = 0; j < maxIndex; j++) {
            clause.push(Minisat::mkLit(prop[j][i]));
        }
        solver.addClause(clause);
        clause.clear();
    }

    //2. No one vertex can appear twice in a vertex cover.
    for (int m = 0; m < maxIndex; m++) {
        for (int p = 0; p < k; p++) {
            for (int q = p + 1; q < k; q++) {
                solver.addClause(~Minisat::mkLit(prop[m][p]), ~Minisat::mkLit(prop[m][q]));
            }
        }
    }

    //3. No more than one vertex appears in the mth position of the vertex cover.
    for (int m = 0; m < k; m++) {
        for (int p = 0; p < maxIndex; p++) {
            for (int q = p + 1; q < maxIndex; q++) {
                solver.addClause(~Minisat::mkLit(prop[p][m]), ~Minisat::mkLit(prop[q][m]));
            }
        }
    }

    //4. Every edge is incident to at least one vertex in the vertex cover.
    visitedVertex.clear();
    for (const auto& [key, value] : graph) {
        visitedVertex.insert(key);
        for (const auto& v : value) {
            if (visitedVertex.find(v) == visitedVertex.end()) {
                Minisat::vec<Minisat::Lit> clause;
                for (int m = 0; m < k; m++) {
                    clause.push(Minisat::mkLit(prop[key][m]));
                    clause.push(Minisat::mkLit(prop[v][m]));
                }
                solver.addClause(clause);
                clause.clear();
            }
        }
    }

    std::vector<int> vertexCover;
    if (solver.solve()) {
        for (int i = 0; i < maxIndex; i++) {
            for (int j = 0; j < k; j++) {
                //#define l_True  (lbool((uint8_t)0)) in SolverTypes
                if (Minisat::toInt(solver.modelValue(prop[i][j])) == 0) {
                    vertexCover.push_back(i);
                }
            }
        }
        std::sort(vertexCover.begin(), vertexCover.end());
        return vertexCover;
    } else {
        return {};
    }
}

void getMinVertexCover(int maxIndex, std::unordered_map<int, std::vector<int>>& graph) {
    // check if graph is empty
    if (graph.empty()) {
        std::cout << std::endl;
        return;
    }
    
    int low = 1;
    int high = maxIndex;
    std::vector<int> temp, result;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        temp = getVertexCover(mid, maxIndex, graph);
        if (temp.empty()) {
            low = mid + 1;
        } else {
            result = temp;
            high = mid - 1;
        }
    }
    // for (int k = 1; k <= maxIndex; k++) {
    //     temp = getVertexCover(k);
    //     std::cout << "current k: " << k << "=> ";
    //     for (int i = 0; i < temp.size(); i++) {
    //         std::cout << temp[i] << ' ';
    //     }
    //     std::cout << '\n';
    // }
    std::cout << "CNF-SAT-VC: ";
    for (int i = 0; i < result.size() - 1; i++) {
        std::cout << result[i] << ',';
    }
    std::cout << result[result.size() - 1] << '\n';
}

int main() {
    std::string line;
    int maxIndex;
    std::unordered_map<int, std::vector<int>> graph;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        char command = line[0];
        if (command == 'V') {
            maxIndex = parseCommandV(line);
        } else if (command == 'E') {
            parseCommandE(line, maxIndex, graph);
            getMinVertexCover(maxIndex, graph);
        } else {
            std::cerr << "Error: command not recognized!";
            continue;
        }
    }
    return 0;
}
