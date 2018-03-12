#ifndef ASSIGNMENT3_SCHEDULER_H
#define ASSIGNMENT3_SCHEDULER_H

#include "PageFrameAllocator.h"
#include "ProcessTrace.h"

#include <MMU.h>

#include <queue>
#include <vector>
#include <string>
#include <fstream>


class Scheduler {

public:

    //Constructor - initialize processing
    Scheduler(std::vector<std::string> &file_names_, mem::MMU &memory_,
              PageFrameAllocator &allocator_, int time_slice_);

    //Destructor - clean up processing
    virtual ~Scheduler();

    //Rule of 5:
    Scheduler(const Scheduler &other) = delete;
    Scheduler(Scheduler &&other) = delete;
    Scheduler operator=(const Scheduler &other) = delete;
    Scheduler operator=(Scheduler &&other) = delete;

    // Executer
    void Execute();

private:

    int TIME_SLICE; //number of lines to process
    int NUM_PROCESSES; //number of processes currently running
    // Memory contents
    mem::MMU &memory;
    // Memory allocator
    PageFrameAllocator &allocator;
    //vector of all processes
    std::vector<ProcessTrace*> processes;

    void ParseFiles(std::vector<std::string> &file_names_);
    void getNextIndex(int& currentIndex);

    
};

#endif //ASSIGNMENT3_SCHEDULER_H
