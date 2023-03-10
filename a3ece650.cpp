#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <signal.h>
#include <sys/wait.h>



int main (int argc, char **argv) {
    // check the validity of in-line arguments
    int option;

    while ( (option = getopt(argc, argv, "s:n:l:c:?")) != -1) {
        switch (option) {
            case 's': {
                int ks = atoi(optarg);
                if (ks < 2) {
                    std::cerr << "Error: s is less than 2!" << std::endl;
                    return 1;
                }
                break;
            }
            case 'n': {
                int kn = atoi(optarg);
                if (kn < 1) {
                    std::cerr << "Error: n is less than 1!" << std::endl;
                    return 1;
                }
                break;
            }
            case 'l': {
                int kl = atoi(optarg);
                if (kl < 5) {
                    std::cerr << "Error: l is less than 5!" << std::endl;
                    return 1;
                }
                break;
            }
            case 'c': {
                int kc = atoi(optarg);
                if (kc < 1) {
                    std::cerr << "Error: c is less than 1!" << std::endl;
                    return 1;
                }
                break;
            }
            case '?': {
                std::cerr << "Error: unrecognized argument!" << std::endl;
                return 1;
            }
            default: {
                return 0;
            }
        }
    }


    // The read end is at fds[0]
    // The write end is at fds[1]
    // declare fd
    std::vector<pid_t> children;
    pid_t pid;
    int rgenToA1[2];
    int a1ToA2[2];
    pipe(rgenToA1);
    pipe(a1ToA2);

    //######################
    // run rgen concurrently
    //######################
    pid = fork();
    if (pid == 0) {
        //inside child
        close(rgenToA1[0]);
        //redirect stdoutput to pipe
        dup2(rgenToA1[1], STDOUT_FILENO);
        close(rgenToA1[1]);
        char *argRgen[] = {(char*)"rgen", NULL};
        execv("rgen", argRgen);
        perror("Error: could not execute rgen!");
        return 1;
    } else if (pid < 0) {
        std::cerr << "Error: could not fork rgen" << std::endl;
        return 1;  
    }
    children.push_back(pid);

    //######################
    // run a1 concurrently
    //######################
    pid = fork();
    if (pid == 0) {
        close(rgenToA1[1]);
        //redirect stdin from the pipe
        dup2(rgenToA1[0], STDIN_FILENO);
        close(rgenToA1[0]);

        //redirect stdout to pipe
        close(a1ToA2[0]);
        dup2(a1ToA2[1], STDOUT_FILENO);
        close(a1ToA2[1]);
        execlp("python3", "python3", "a1ece650.py", NULL);
        perror("Error: could not execute a1ece650!");
        return 1;
    } else if (pid < 0) {
        std::cerr << "Error: could not fork a1ece650" << std::endl;
        return 1;  
    }
    children.push_back(pid);

    //######################
    // run a2 concurrently
    //######################
    pid = fork();
    if (pid == 0) {
        close(a1ToA2[1]);
        dup2(a1ToA2[0], STDIN_FILENO);
        close(a1ToA2[0]);
        char *argA2[] = {(char*)"a2ece650", NULL};
        execv("a2ece650", argA2);
        perror("Error: could not execute a2ece650!");
        return 1;
    } else if (pid < 0) {
        std::cerr << "Error: could not fork a2ece650" << std::endl;
        return 1;  
    }
    children.push_back(pid);

    //######################
    // run a program to take a3's input concurrently
    //######################
    pid = fork();
    if (pid == 0) {
        close(a1ToA2[0]);
        dup2(a1ToA2[1], STDOUT_FILENO);
        close(a1ToA2[1]);

        while (!std::cin.eof()) {
            std::string line;
            std::getline(std::cin, line);
            if (line.size() > 0) {
                std::cout << line << std::endl;
            }
        }
        return 0;
    } else if (pid < 0) {
        std::cerr << "Error: could not fork input reader" << std::endl;
        return 1;  
    }
    children.push_back(pid);

    int status;
    wait(&status);

    // kill all children
    for (pid_t c : children) {
        int cStatus;
        kill(c, SIGTERM);
        waitpid(c, &cStatus, 0);
    }

    return 0;
}
