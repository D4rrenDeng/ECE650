#include <iostream>
#include <tuple>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <fstream>

std::vector<std::tuple<int, int, int, int>> segments;
std::vector<std::tuple<int, int>> coordinates;

int generateRand() {
    // open /dev/urandom to read
    std::ifstream urandom("/dev/urandom");

    // check that it did not fail
    if (urandom.fail()) {
    std::cerr << "Error: cannot open /dev/urandom\n";
    return 1;
    }

    // read a random unsigned int
    unsigned int num = 42;
    urandom.read((char *)&num, sizeof(int));

    // close random stream
    urandom.close();
    return num;
}

bool checkValidity(int x1, int y1, int x2, int y2) {
    // check if it overlaps with existing segments in the network
    for (auto curTuple : segments) {
        std::tuple<int, int, int, int> curSegment = curTuple;
        int x3 = std::get<0>(curSegment);
        int y3 = std::get<1>(curSegment);
        int x4 = std::get<2>(curSegment);
        int y4 = std::get<3>(curSegment);
        int p = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4);
        int q = (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2);
        int d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (p == 0 && q == 0 && d == 0) {
            return false;
        }
    }
    // check if it intersects with existing segments on the same street
    std::vector<std::tuple<int, int>>::iterator i;
    for (i = coordinates.begin(); i != coordinates.end() - 1; i++) {
        std::tuple<int, int> curCor = *i;
        std::tuple<int, int> nextCor = *(i + 1);
        int x3 = std::get<0>(curCor);
        int y3 = std::get<1>(curCor);
        int x4 = std::get<0>(nextCor);
        int y4 = std::get<1>(nextCor);
        int p = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4);
        int q = (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2);
        int d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (d != 0) {
            double t = p / (double)d;
            double u = q / (double)d;
            if (t > 0 && t < 1 && u > 0 && u < 1) {
                return false;
            }
        }
    }

    return true;
}

int main (int argc, char **argv) {
    // handle in-line arguments and update specs
    int ks = 10;
    int kn = 5;
    int kl = 5;
    int kc = 20;
    int N = 25;

    while (true) {
        int option;

        while ( (option = getopt(argc, argv, "s:n:l:c:?")) != -1) {
            switch (option) {
                case 's': {
                    ks = atoi(optarg);
                    break;
                }
                case 'n': {
                    kn = atoi(optarg);
                    break;
                }
                case 'l': {
                    kl = atoi(optarg);
                    break;
                }
                case 'c': {
                    kc = atoi(optarg);
                    break;
                }
                default: {
                    return 0;
                }
            }
        }

        // rgen number of streets
        int rand = abs(generateRand());
        int numStreet = 2 + (rand % (ks - 2 + 1));
        int indexSt = 0;
        int numFail = 0;
        while (indexSt < numStreet) {
            coordinates.clear();
            rand = abs(generateRand());
            int numSegment = 1 + (rand % (kn - 1 + 1));
            int indexSeg = 0;
            rand = abs(generateRand());
            int x1 = kc * (-1) + (rand % (2 * kc + 1));
            rand = abs(generateRand());
            int y1 = kc * (-1) + (rand % (2 * kc + 1));
            std::tuple<int, int> curCor;
            curCor = std::make_tuple(x1, y1);

            coordinates.push_back(curCor);
            while (indexSeg < numSegment) {
                rand = abs(generateRand());
                int x2 = kc * (-1) + (rand % (2 * kc + 1));
                rand = abs(generateRand());
                int y2 = kc * (-1) + (rand % (2 * kc + 1));
                std::tuple<int, int, int, int> curSegment;

                // detect if segment is valid or has distance of 0, and count of failures
                while (x1 == x2 && y1 == y2 || !checkValidity(x1, y1, x2, y2)) {
                    numFail++;
                    if (numFail == N) {
                        std::cerr << "Error: failed to generate valid input for " << N << " simultaneous attempts！" << std::endl;
                        exit(1);
                    }
                    rand = abs(generateRand());
                    x2 = kc * (-1) + (rand % (2 * kc + 1));
                    rand = abs(generateRand());
                    y2 = kc * (-1) + (rand % (2 * kc + 1));
                }
                numFail = 0;
                indexSeg++;
                curCor = std::make_tuple(x2, y2);
                coordinates.push_back(curCor);
                curSegment = std::make_tuple(x1, y1, x2, y2);
                segments.push_back(curSegment);
                x1 = x2;
                y1 = y2;
            }
            std::string output = "a \"Street " + std::to_string(indexSt) + "\"";
            for (auto curTuple : coordinates) {
                curCor = curTuple;
                output = output + " (" + std::to_string(std::get<0>(curCor)) + "," + std::to_string(std::get<1>(curCor)) + ")";
            }
            std::cout << output << std::endl;
            indexSt++;
        }
        std::cout << 'g' << std::endl;
        for (int i = 0; i < numStreet; i++) {
            std::string output = "r \"Street " + std::to_string(i) + "\"";
            std::cout << output << std::endl;
        }
        sleep(kl);
    }

    return 0;
}
