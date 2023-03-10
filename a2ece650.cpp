// Compile with c++ a2ece650.cpp -std=c++11 -o a2ece650
#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <set>
#include <iterator>
#include <queue>

int maxIndex;
int start, end;
int distance = 0;
char command;
bool foundPath = false;
std::string line;
std::string output = "";
std::vector<int> res;
std::regex regex(R"(-?\d+)");
std::smatch match;
std::unordered_map<int, std::vector<int>> graph;
std::unordered_map<int, int> prevNode;
std::set<int> visited;

void parseCommandV() {
    output = "";
    res.clear();
    visited.clear();
    foundPath = false;
    std::regex_search(line, match, regex);
    maxIndex = std::stoi(match.str());
    if (maxIndex <= 0) {
        std::cerr << "Error: invalid command!";
    }
}

void parseCommandE() {
    std::string temp = line; 
    // check the validity of this command
    while (std::regex_search(temp, match, regex)) {
        int m = std::stoi(match.str());
        temp = match.suffix().str();
        std::regex_search(temp, match, regex);
        int n = std::stoi(match.str());
        temp = match.suffix().str();
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
        temp = match.suffix().str();
        std::regex_search(temp, match, regex);
        int n = std::stoi(match.str());
        temp = match.suffix().str();
        graph[m].push_back(n);
        graph[n].push_back(m);
    }
}

void generatePath() {
    std::queue<int> vertexQueue;
    std::vector<int>::iterator itr;

    prevNode.clear();
    if (start == end) {
        // handle the edge case where start == end
        for (itr = graph[start].begin(); itr != graph[start].end() && !foundPath; itr++) {
            distance = 0;
            visited.clear();
            prevNode.clear();
            while (!vertexQueue.empty()) {
                vertexQueue.pop();
            }
            vertexQueue.push(*itr);
            prevNode[*itr] = start;
            visited.insert(*itr);
            while (!vertexQueue.empty() && !foundPath) {
                distance++;
                int size = vertexQueue.size();
                for (int i = 0; i < size && !foundPath; i++) {
                    int cur = vertexQueue.front();
                    vertexQueue.pop();
                    std::vector<int>::iterator neighbor;
                    for(neighbor = graph[cur].begin(); neighbor != graph[cur].end(); neighbor++) {
                        if (visited.find(*neighbor) == visited.end()) {
                            if (*neighbor == start && distance == 1) {
                                continue;
                            }
                            if (*neighbor == end) {
                                foundPath = true;
                                prevNode[*neighbor] = cur;
                                break;
                            }
                            visited.insert(*neighbor);
                            prevNode[*neighbor] = cur;
                            vertexQueue.push(*neighbor);
                        }
                    }
                }
            }
        }
    } else {
        vertexQueue.push(start);
        visited.insert(start);
        while (!vertexQueue.empty() && !foundPath) {
            int cur = vertexQueue.front();
            vertexQueue.pop();
            for (itr = graph[cur].begin(); itr != graph[cur].end(); itr++) {
                if (visited.find(*itr) == visited.end()) {
                    if (*itr == end) {
                        foundPath = true;
                        prevNode[*itr] = cur;
                        break;
                    }
                    visited.insert(*itr);
                    prevNode[*itr] = cur;
                    vertexQueue.push(*itr);
                }
            }
        }
    }
}

int main() {
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        command = line[0];
        if (command == 'V') {
            parseCommandV();
        } else if (command == 'E') {
            parseCommandE();
        } else if (command == 's') {
            output = "";
            res.clear();
            visited.clear();
            foundPath = false;
            // parse starting and ending vertices
            std::string temp = line;
            std::regex_search(temp, match, regex);
            start = std::stoi(match.str());
            temp = match.suffix().str();
            std::regex_search(temp, match, regex);
            end = std::stoi(match.str());

            // check if vertices exist
            if (graph.find(start) == graph.end() || graph.find(end) == graph.end()) {
                std::cerr << "Error: vertex doesn't exist!";
                continue;
            }
            generatePath();
            if (foundPath) {
                int cur = end;
                res.push_back(cur);
                if (start != end) {
                    while (prevNode.find(cur) != prevNode.end()) {
                        cur = prevNode[cur];
                        res.push_back(cur);
                    }
                } else {
                    cur = prevNode[cur];
                    res.push_back(cur);
                    while (cur != start) {
                        cur = prevNode[cur];
                        res.push_back(cur);
                    }
                }
                std::vector<int>::iterator itr;
                for (itr = res.end() - 1; itr > res.begin(); itr--) {
                    output = output + std::to_string(*itr) + "-";
                }
                output = output + std::to_string(*res.begin()) + "\n";
                std::cout << output;
            } else {
                std::cerr << "Error: no path was found between vertices!";
            }
        } else {
            std::cerr << "Error: command not recognized!";
            continue;
        }
    }
    return 0;
}
