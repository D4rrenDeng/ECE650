#include <iostream>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <memory>
#include <chrono>
#include <future>
#include <atomic>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include "helper.h"


void *threadOfCNF_SAT_VC(void*);
void *threadOfTimeoutCNF_SAT_VC(void*);
void *threadOfCNF_3_SAT_VC(void*);
void *threadOfTimeoutCNF_3_SAT_VC(void*);
void *threadOfAPPROX_VC_1(void*);
void *threadOfREFINED_APPROX_VC_1(void*);
void *threadOfAPPROX_VC_2(void*);
void *threadOfREFINED_APPROX_VC_2(void*);

int maxIndex;
int timeout_sec = 9999; // TIMEOUT THRESHOLD!!!
std::atomic<pthread_t> child_thread_cnf_sat;
std::atomic<pthread_t> child_thread_cnf_3_sat;
std::unordered_map<int, std::vector<int>> graph;
std::vector<std::pair<int,int>> originalEdge;
std::vector<int> resultCNF_SAT_VC, 
    resultCNF_3_SAT_VC,
    resultAPPROX_VC_1, 
    resultREFINED_APPROX_VC_1,
    resultAPPROX_VC_2, 
    resultREFINED_APPROX_VC_2;

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

void parseCommandE(std::string line) {
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

void printResult() {
    // # original
    if (resultCNF_SAT_VC.size() == 0) {
        printf("CNF-SAT-VC: Timeout!\n");
    } else {
        std::cout << "CNF-SAT-VC: ";
        for (int i = 0; i < resultCNF_SAT_VC.size() - 1; i++) {
            std::cout << resultCNF_SAT_VC[i] << ',';
        }
        std::cout << resultCNF_SAT_VC[resultCNF_SAT_VC.size() - 1] << std::endl;
    }

    // # 1
    if (resultCNF_3_SAT_VC.size() == 0) {
        printf("CNF-3-SAT-VC: Timeout!\n");
    } else {
        std::cout << "CNF-3-SAT-VC: ";
        for (int i = 0; i < resultCNF_3_SAT_VC.size() - 1; i++) {
            std::cout << resultCNF_3_SAT_VC[i] << ',';
        }
        std::cout << resultCNF_3_SAT_VC[resultCNF_3_SAT_VC.size() - 1] << std::endl;
    }

    // # 2
    std::cout << "APPROX-VC-1: ";
    for (int i = 0; i < resultAPPROX_VC_1.size() - 1; i++) {
        std::cout << resultAPPROX_VC_1[i] << ',';
    }
    std::cout << resultAPPROX_VC_1[resultAPPROX_VC_1.size() - 1] << std::endl;

    // # 3
    std::cout << "APPROX-VC-2: ";
    for (int i = 0; i < resultAPPROX_VC_2.size() - 1; i++) {
        std::cout << resultAPPROX_VC_2[i] << ',';
    }
    std::cout << resultAPPROX_VC_2[resultAPPROX_VC_2.size() - 1] << std::endl;

    // # 4
    std::cout << "REFINED-APPROX-VC-1: ";
    for (int i = 0; i < resultREFINED_APPROX_VC_1.size() - 1; i++) {
        std::cout << resultREFINED_APPROX_VC_1[i] << ',';
    }
    std::cout << resultREFINED_APPROX_VC_1[resultREFINED_APPROX_VC_1.size() - 1] << std::endl;

    // # 5
    std::cout << "REFINED-APPROX-VC-2: ";
    for (int i = 0; i < resultREFINED_APPROX_VC_2.size() - 1; i++) {
        std::cout << resultREFINED_APPROX_VC_2[i] << ',';
    }
    std::cout << resultREFINED_APPROX_VC_2[resultREFINED_APPROX_VC_2.size() - 1] << std::endl;
}

void printResultAprox() {
    // # original
    if (resultCNF_3_SAT_VC.size() != 0){

    
        std::cout << maxIndex << ": CNF-SAT-VC: ";
        
        std::cout << (float)resultCNF_SAT_VC.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
        
        std::cout << maxIndex << ": CNF-3-SAT-VC: ";
        
        std::cout << (float)resultCNF_3_SAT_VC.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
    

        
        std::cout << maxIndex << ": APPROX-VC-1: ";
        
        std::cout << (float)resultAPPROX_VC_1.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
        

        
        std::cout << maxIndex << ": APPROX-VC-2: ";
    
        std::cout << (float)resultAPPROX_VC_2.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
        
        std::cout << maxIndex << ": REFINED-APPROX-VC-1: ";
        
        std::cout << (float)resultREFINED_APPROX_VC_1.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
    

        
        std::cout << maxIndex << ": REFINED-APPROX-VC-2: ";
        
        std::cout << (float)resultREFINED_APPROX_VC_2.size()/(float)resultCNF_3_SAT_VC.size() << std::endl;
        
    } 
    else {

        std::cout << maxIndex << ": CNF-SAT-VC: ";
    
        std::cout << 0 << std::endl;
    
        std::cout << maxIndex << ": CNF-3-SAT-VC: ";
    
        std::cout << 0 << std::endl;
    
        std::cout << maxIndex << ": APPROX-VC-1: ";
        
        std::cout << 0 << std::endl;
    
        std::cout << maxIndex << ": APPROX-VC-2: ";
        
        std::cout << 0 << std::endl;
        
        std::cout << maxIndex << ": REFINED-APPROX-VC-1: ";
        
        std::cout << 0 << std::endl;
    
        std::cout << maxIndex << ": REFINED-APPROX-VC-2: ";
    
        std::cout << 0 << std::endl;
    

    }
}

//-------------- Thread I/O -----------------------------
void *threadOfIO(void *arg) {
    std::string line;
    pthread_t thread_timeoutcnf_sat, thread_timeoutcnf_3_sat, thread_vc_1, thread_r_vc_1, thread_vc_2, thread_r_vc_2;
    std::regex regV ("(-?\\d+)");
    std::regex regE ("\\<\\s*(-?\\d+)\\s*,\\s*(-?\\d+)\\s*>");
    std::regex regS ("(-?\\d+)\\s+(-?\\d+)");
    int start;
    int end;
    
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        char command = line[0];
        if (command == 'V') {
            maxIndex = parseCommandV(line);
        } else if (command == 'E') {
            parseCommandE(line);
            originalEdge.clear();
               std::smatch e;

            // loop through each <,> and obtain number using regex
            originalEdge.clear();
            while (std::regex_search(line, e, regE)) {
                start = stoi(e.str(0).substr(e.str(0).find("<") + 1,e.str(0).find(",")-1));
                end = stoi(e.str(0).substr(e.str(0).find(",")+1,e.str(0).find(">") - (e.str(0).find(",") + 1) ));
                line = e.suffix().str();
                originalEdge.push_back(std::make_pair(start,end));
            }
            
            // start all the threads
            pthread_create(&thread_timeoutcnf_sat, NULL, &threadOfTimeoutCNF_SAT_VC, NULL);
            pthread_create(&thread_timeoutcnf_3_sat, NULL, &threadOfTimeoutCNF_3_SAT_VC, NULL);
            pthread_create(&thread_vc_1, NULL, &threadOfAPPROX_VC_1, NULL);
            pthread_create(&thread_r_vc_1, NULL, &threadOfREFINED_APPROX_VC_1, NULL);
            pthread_create(&thread_vc_2, NULL, &threadOfAPPROX_VC_2, NULL);
            pthread_create(&thread_r_vc_2, NULL, &threadOfREFINED_APPROX_VC_2, NULL);

            pthread_join(thread_timeoutcnf_sat, NULL);
            pthread_join(thread_timeoutcnf_3_sat, NULL);
            pthread_join(thread_vc_1, NULL);
            pthread_join(thread_r_vc_1, NULL);
            pthread_join(thread_vc_2, NULL);
            pthread_join(thread_r_vc_2, NULL);

            printResult();
            // printResultAprox();

            resultCNF_SAT_VC.clear();
            resultCNF_3_SAT_VC.clear();
            resultAPPROX_VC_1.clear();
            resultREFINED_APPROX_VC_1.clear();
            resultAPPROX_VC_2.clear();
            resultREFINED_APPROX_VC_2.clear();
        } else {
            std::cerr << "Error: command not recognized!";
            continue;
        }
    }
}
//-------------------------------------------------------

//-------------- Thread CNF-SAT-VC ----------------------
std::vector<int> getVertexCover(int k, std::unordered_map<int, std::vector<int>> &tempGraph) {
    Minisat::Solver solver;
    Minisat::Var prop[maxIndex][k];
    std::unordered_set<int> visitedVertex;

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
    for (const auto& [key, value] : tempGraph) {
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

void getMinVertexCover(int V, std::vector<std::pair<int,int>> Edge){
    std::vector<int> result;
    for (int k = 0;k <= V-1; k ++){

        std::unique_ptr<Minisat::Solver> MiniSatSolver(new Minisat::Solver());
        //initialize each proposition of size Vxk
        Minisat::Lit vertexLit [V][k];

        
        for (int i = 0; i <= V-1; i++){
            for (int j = 0; j <= k; j++){
                vertexLit[i][j] = Minisat::mkLit(MiniSatSolver->newVar());
            }
        }

        //incident edge clause
        for (long i = 0; (unsigned)i <= Edge.size()-1; i++){
            Minisat::vec<Minisat::Lit> c1;
            for (int j = 0; j <= k-1; j ++){
                c1.push(vertexLit[Edge[i].first][j]);
                c1.push(vertexLit[Edge[i].second][j]);
            }
            MiniSatSolver -> addClause (c1);

        }

        //solve the formula
        bool SAToutput = MiniSatSolver -> solve();
        //if not sat then continue with next iteration
        if (SAToutput == 0){
            continue;
        } 


        if (SAToutput == 1){
            //No one vertex can appear twice clausees
            for (int i = 0; i <= V-1; i++){
                
                    for (int j = 0; j <= k-2; j++ ){
                        
                        for (int l = j; l <= k-2; l++){
                            Minisat::vec<Minisat::Lit> c2;
                            c2.push(~vertexLit[i][j]);
                            c2.push(~vertexLit[i][l+1]);
                            MiniSatSolver -> addClause (c2);

                        }
                        
                    }

            }

        }

       
        
        //solve the formula
        SAToutput = MiniSatSolver -> solve();
        //if not sat then continue with next iteration
        if (SAToutput == 0){
            continue;
        } 


        if (SAToutput == 1){
            //No more than one vertex appears in the mth position clauses
            for (int i = 0; i <= k-1; i++){
                
                    for (int j = 0; j <= V-2; j++ ){
                        
                        for (int l = j; l < V-2; l++){
                            Minisat::vec<Minisat::Lit> c3;
                            c3.push(~vertexLit[j][i]);
                            c3.push(~vertexLit[l+1][i]);
                            MiniSatSolver -> addClause (c3);
                        }
                        
                        
                    }

            }

        }

        

        //solve the formula
        SAToutput = MiniSatSolver -> solve();
        //if not sat then continue with next iteration
        if (SAToutput == 0){
            continue;
        }

        if (SAToutput == 1){
            //At least one vertex is the ith vertex clause
            for (int i = 0; i <= k; i++){
                
                Minisat::vec <Minisat::Lit> c4;
                for (int j = 0; j <=V-1; j++){
                    c4.push(vertexLit[j][i]);
                }
                MiniSatSolver -> addClause (c4);

            }

        }
        

        //solve the formula
        SAToutput = MiniSatSolver -> solve();   

        //if satisfied then find the i value
        //output should already be sorted
        if (SAToutput == 1){
            for (int i = 0; i <= V-1; i++){
                for (int j = 0; j <= k; j++){
                    
                    if (toInt( MiniSatSolver -> modelValue(vertexLit[i][j])) == 0 && j != k){
                        // std::cout << i << " ";
                        result.push_back(i);
                    }

                }
            }

            //reset solver
            MiniSatSolver.reset(new Minisat::Solver());

            //exit for loop
            break;

        } 
      
        
    }

    resultCNF_SAT_VC = result;

}

void* threadOfTimeoutCNF_SAT_VC(void* arg) {
    if (maxIndex >= 20) {
        printf("%d, CNF_SAT_VC, Timeout!\n", maxIndex);
    } else {
        /* Process */
        std::unordered_map<int, std::vector<int>> tempGraph = graph;

        // getMinVertexCover(tempGraph);
        getMinVertexCover(maxIndex, originalEdge);
        /* Process */
        
        // timespec rt;
        // clockid_t cid;
        // pthread_getcpuclockid(pthread_self(), &cid);
        // clock_gettime(cid, &rt);
        // printf("%d, CNF_SAT_VC, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
    }
}
//-------------------------------------------------------

//-------------- Thread APPROX_3_VC_1 -------------------
std::vector<int> FindMinVertexCover_3CNF(int V, std::vector<std::pair<int,int>> Edge){
    std::vector<int> result;
    for (int k = 0;k <= V-1; k ++){
        std::unique_ptr<Minisat::Solver> MiniSatSolver(new Minisat::Solver());
        //initialize each proposition of size Vxk
        Minisat::Lit vertexLit [V][k];

        for (int i = 0; i <= V-1; i++){
            for (int j = 0; j <= k; j++){
                vertexLit[i][j] = Minisat::mkLit(MiniSatSolver->newVar());
            }
        }

        //incident edge clause
        int t = ((int)Edge.size()) * (k+1) * 2;
        int s = (k+1) * V * 2;
        int e = (int)Edge.size() - 1;
        Minisat::Lit tempVar1 [t];
        Minisat::Lit tempVar2 [s];
        
        for (int i = 0; i <= t-1; i++){
            tempVar1 [i] = Minisat::mkLit(MiniSatSolver->newVar());
        }

        for (int i = 0; i <= s-1; i++){ 
            tempVar2 [i] = Minisat::mkLit(MiniSatSolver->newVar());
        }

        int u = 0;
        int v = 0;
       
        for (int i = 0; i <= k; i++){
                
            //cout << "v:" << v << " x(" << 0 << "," << i << ")" <<  " t(" << v << ")" << endl;
            // MiniSatSolver -> addClause(vertexLit[0][i],tempVar2[v]);
            // v = v + 1;

            // vec <Lit> c4;
            for (int j = 0; j <=V-1; j++){
                if ( j == 0){
                    MiniSatSolver -> addClause(vertexLit[j][i],tempVar2[v]);
                    v = v + 1;
                }
                else if (j == V - 1){

                    //cout << "v:" << v <<  " ~t(" << v-1 << ")" << " x(" << j << "," << i << ")" << endl;
                    MiniSatSolver -> addClause(~tempVar2[v-1],vertexLit[j][i]);
                    v = v + 1;

                } else {

                     //cout << "v:" << v <<  " ~t(" << v-1 << ")" << " x(" << j << "," << i << ")" <<  " t(" << v << ")" << endl;
                    MiniSatSolver -> addClause(~tempVar2[v-1],vertexLit[j][i],tempVar2[v]);
                    v = v + 1;

                }
            }
        }

        for (int i = 0; i <= V-1; i++){   
            for (int j = 0; j <= k-2; j++ ){      
                for (int l = j; l <= k-2; l++){ 
                    MiniSatSolver -> addClause(~vertexLit[i][j],~vertexLit[i][l+1]);
                }  
            }
        }

        for (int i = 0; i <= k-1; i++){
            for (int j = 0; j <= V-2; j++ ){
                for (int l = j; l <= V-2; l++){     
                    MiniSatSolver -> addClause (~vertexLit[j][i],~vertexLit[l+1][i]);
                }         
            }
        }

         //incident edge clause
        for (long i = 0; (unsigned)i <= Edge.size()-1; i++){
            for (int j = 0; j <= k-1; j ++){
                if (k - 1 == 0){
                    MiniSatSolver -> addClause(vertexLit[Edge[i].first][j],vertexLit[Edge[i].second][j]);
                }
                else if (j == 0){   
                    MiniSatSolver -> addClause(vertexLit[Edge[i].first][j],tempVar1[u]);
                    u = u + 1;      
                    MiniSatSolver -> addClause(~tempVar1[u-1],vertexLit[Edge[i].second][j],tempVar1[u]);
                    u = u + 1;
                }
                else if (j == k-1){
                    MiniSatSolver -> addClause(~tempVar1[u-1],vertexLit[Edge[i].first][j],tempVar1[u]);
                    u = u + 1;
                    MiniSatSolver -> addClause(~tempVar1[u-1],vertexLit[Edge[i].second][j]);
                    u = u + 1;
                } else {
                    MiniSatSolver -> addClause(~tempVar1[u-1],vertexLit[Edge[i].first][j],tempVar1[u]);
                    u = u + 1;
                    MiniSatSolver -> addClause(~tempVar1[u-1],vertexLit[Edge[i].second][j],tempVar1[u]);
                    u = u + 1;
                }          
            }
        }

        //solve the formula
        bool SAToutput = MiniSatSolver -> solve();   

        //if satisfied then find the i value
        //output should already be sorted
        if (SAToutput == 1){
            if (k == 0){
                int num = 0;
                for (int i = 0; i <= V-1; i++){
                    // cout << "k:" << k << endl;
                    for (int j = 0; j <= k; j++){
                        // && j != k
                        if (toInt( MiniSatSolver -> modelValue(vertexLit[i][j])) == 0 && j != k){
                            num = num + 1;
                        }
                    }
                }
                if (num != 0){
                    int count = 0;
                    for (int i = 0; i <= V-1; i++){
                        for (int j = 0; j <= k; j++){
                            if (toInt( MiniSatSolver -> modelValue(vertexLit[i][j])) == 0 && j != k){
                                count = count + 1;
                                result.push_back(i);
                            }                     
                        }
                    }
                    return result;
                }
            } else {
                int count = 0;
                for (int i = 0; i <= V-1; i++){
                    for (int j = 0; j <= k; j++){
                        if (toInt( MiniSatSolver -> modelValue(vertexLit[i][j])) == 0 && j != k){
                            count = count + 1;
                            result.push_back(i);      
                        }
                    }
                }
                return result;
            }
            MiniSatSolver.reset(new Minisat::Solver());
        }    
    }
}

void* threadOfTimeoutCNF_3_SAT_VC(void* arg) {
    if (maxIndex >= 25) {
        printf("%d, CNF_3_SAT_VC, Timeout!\n", maxIndex);
    } else {
        /* Process */
        resultCNF_3_SAT_VC = FindMinVertexCover_3CNF(maxIndex, originalEdge);
        /* Process */
        
        // timespec rt;
        // clockid_t cid;
        // pthread_getcpuclockid(pthread_self(), &cid);
        // clock_gettime(cid, &rt);
        // printf("%d, CNF_3_SAT_VC, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
    }
}
//-------------------------------------------------------

//-------------- Thread APPROX_VC_1 ---------------------
int findHighest(std::unordered_map<int, std::vector<int>>& tempGraph) {
    int vertex = -1;
    int size = -1;

    for (const auto& [key, value] : tempGraph) {
        if ((int)value.size() > size) {
            size = (int)value.size();
            vertex = key;
        }
    }

    return vertex;
}

std::vector<int> deleteEdges(int vertex, std::unordered_map<int, std::vector<int>>& tempGraph) {
    std::vector<int> deleteList;
    for (auto& [key, value] : tempGraph) {
        int index = -1;
        for (int i = 0; i < (int)value.size(); i++) {
            if (value[i] == vertex) {
                index = i;
            }
        }
        if (index != -1) {
            if ((int)value.size() == 1) {
                deleteList.push_back(key);
                continue;
            }
            value.erase(value.begin() + index);
        }
    }
    return deleteList;
}

void cleanGraph(std::vector<int>& deleteList, std::unordered_map<int, std::vector<int>>& tempGraph) {
    for (const auto& key : deleteList) {
        tempGraph.erase(key);
    }
}

void APPROX_VC_1(std::unordered_map<int, std::vector<int>>& tempGraph) {
    std::vector<int> result;
    std::vector<int> deleteList;
    int high;
    while (!tempGraph.empty()) {
        high = findHighest(tempGraph);
        result.push_back(high);
        deleteList = deleteEdges(high, tempGraph);
        cleanGraph(deleteList, tempGraph);
        deleteList.clear();
        tempGraph.erase(high);
    }
    std::sort(result.begin(), result.end());
    resultAPPROX_VC_1 = result;
}

void *threadOfAPPROX_VC_1(void *arg) {
    /* Process */
    std::unordered_map<int, std::vector<int>> tempGraph = graph;

    APPROX_VC_1(tempGraph);
    /* Process */
    
    // timespec rt;
    // clockid_t cid;
    // pthread_getcpuclockid(pthread_self(), &cid);
    // clock_gettime(cid, &rt);
    // printf("%d, APPROX_VC_1, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
}
//-------------------------------------------------------

//-------------- Thread REFINED_APPROX_VC_1 -------------
std::vector<int> REFINED_APPROX_VC_1(std::unordered_map<int, std::vector<int>>& tempGraph) {
    std::vector<int> result;
    std::vector<int> deleteList;
    int high;

    while (!tempGraph.empty()) {
        high = findHighest(tempGraph);
        result.push_back(high);
        deleteList = deleteEdges(high, tempGraph);
        cleanGraph(deleteList, tempGraph);
        deleteList.clear();
        tempGraph.erase(high);
    }
    return result;
}

bool isValidVertexCover(int vertex, std::vector<int>& result, std::unordered_map<int, std::vector<int>>& tempGraph) {
    std::unordered_map<int, std::vector<int>> curGraph = tempGraph;
    std::vector<int> deleteList;
    
    for (const auto& value : result) {
        if (value != vertex) {
            deleteList = deleteEdges(value, curGraph);
            cleanGraph(deleteList, curGraph);
            deleteList.clear();
            curGraph.erase(value);
        }
    }

    return curGraph.empty();
}

void refineVertexCover_VC_1(std::vector<int>& result, std::unordered_map<int, std::vector<int>>& tempGraph) {
    bool flag = true;

    while (flag) {
        flag = false;
        int index = -1;

        for (int i = 0; i < (int)result.size(); i++) {
            if (isValidVertexCover(result[i], result, tempGraph)) {
                index = i;
            }
        }
        if (index != -1) {
            result.erase(result.begin() + index);
            flag = true;
        }
    }
    std::sort(result.begin(), result.end());
    resultREFINED_APPROX_VC_1 = result;
}

void *threadOfREFINED_APPROX_VC_1(void *arg) {
    /* Process */
    std::unordered_map<int, std::vector<int>> tempGraph = graph;
    std::vector<int> result;

    result = REFINED_APPROX_VC_1(tempGraph);
    refineVertexCover_VC_1(result, graph);
    /* Process */
    
    // timespec rt;
    // clockid_t cid;
    // pthread_getcpuclockid(pthread_self(), &cid);
    // clock_gettime(cid, &rt);
    // printf("%d, REFINED_APPROX_VC_1, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
}
//-------------------------------------------------------

//-------------- Thread APPROX_VC_2 ---------------------
std::vector<int> FindVertexCover_APROX_VC_2(int V, std::vector<std::pair<int,int>> Edge){
    std::vector<std::pair<int,int>> tempEdge = Edge;
    std::vector<std::pair<int,int>> newEdge;
    std::vector<int> APROX_VC_2_List;
    std::vector<int> result;
    int v = -1;
    int u = -1;
    bool match;
    
        for (long i = 0; (unsigned)i <= tempEdge.size()-1; i++){
            if (i == 0){
                v = tempEdge[i].first;
                u = tempEdge[i].second;
                newEdge.push_back(std::make_pair(tempEdge[i].first,tempEdge[i].second));
                APROX_VC_2_List.push_back(tempEdge[i].first);
                APROX_VC_2_List.push_back(tempEdge[i].second);   
            }
         
            match = false;
            for (int j = 0; j <= (int)newEdge.size()-1; j++){
                if (newEdge[j].first  == tempEdge[i].first || newEdge[j].first  == tempEdge[i].second)
                {
                    match = true;
                }

                if (newEdge[j].second  == tempEdge[i].first || newEdge[j].second == tempEdge[i].second)
                {
                    match = true;
                }
            }

            if (match == false){
                newEdge.push_back(std::make_pair(tempEdge[i].first,tempEdge[i].second));
                APROX_VC_2_List.push_back(tempEdge[i].first);
                APROX_VC_2_List.push_back(tempEdge[i].second);
            }
        }
        sort(APROX_VC_2_List.begin(),APROX_VC_2_List.end());
        for (long i = 0; (unsigned)i <= APROX_VC_2_List.size()-1; i++){ 
            result.push_back(APROX_VC_2_List[i]);
        }

    return result;
}

void *threadOfAPPROX_VC_2(void *arg) {
    /* Process */
    resultAPPROX_VC_2 = FindVertexCover_APROX_VC_2(maxIndex, originalEdge);
    /* Process */
    
    // timespec rt;
    // clockid_t cid;
    // pthread_getcpuclockid(pthread_self(), &cid);
    // clock_gettime(cid, &rt);
    // printf("%d, APPROX_VC_2, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
}

//-------------------------------------------------------

//-------------- Thread REFINED_APPROX_VC_2 -------------
void refineVertexCover_VC_2(std::vector<int>& result, std::unordered_map<int, std::vector<int>>& tempGraph) {
    bool flag = true;

    while (flag) {
        flag = false;
        int index = -1;

        for (int i = 0; i < (int)result.size(); i++) {
            if (isValidVertexCover(result[i], result, tempGraph)) {
                index = i;
            }
        }
        if (index != -1) {
            result.erase(result.begin() + index);
            flag = true;
        }
    }
    std::sort(result.begin(), result.end());
    resultREFINED_APPROX_VC_2 = result;
}



void *threadOfREFINED_APPROX_VC_2(void *arg) {
    /* Process */
    // resultREFINED_APPROX_VC_2 = FindVertexCover_REFINE_APROX_VC_2(maxIndex, originalEdge);
    std::vector<int> result = FindVertexCover_APROX_VC_2(maxIndex, originalEdge);
    refineVertexCover_VC_2(result, graph);
    /* Process */

    // timespec rt;
    // clockid_t cid;
    // pthread_getcpuclockid(pthread_self(), &cid);
    // clock_gettime(cid, &rt);
    // printf("%d, REFINED_APPROX_VC_2, %ld.%09ld\n", maxIndex, rt.tv_sec, rt.tv_nsec);
}
//-------------------------------------------------------

int main() {
    pthread_t thread_io;

    pthread_create(&thread_io, NULL, &threadOfIO, NULL);
    pthread_join(thread_io, NULL);

    return 0;
}
