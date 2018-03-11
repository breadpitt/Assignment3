#include "Scheduler.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>


Scheduler::Scheduler(std::vector<std::string> &file_names_, mem::MMU &memory_,
                     PageFrameAllocator &allocator_, int time_slice_)
        : memory(memory_), allocator(allocator_), TIME_SLICE(time_slice_) {
    NUM_PROCESSES = file_names_.size();
    ParseFiles(file_names_); //initialize processes
}

// Also do these need void or anything?
Scheduler::~Scheduler() {
    // Does this need anything?
}


void Scheduler::ParseFiles(vector<std::string> &file_names_) {

    int id = 1;

    for(std::string s : file_names_){
        ProcessTrace* temp = new ProcessTrace(memory, allocator, s, id);
        processes.push_back(temp);
        ++id;
    }
}


void Scheduler::Execute() {
    int index = 0;
    int lines;

    ProcessTrace* current_proc;
    while(!processes.empty()){
        current_proc = processes.at(index);
        lines = current_proc->Execute(TIME_SLICE);

        if(lines != TIME_SLICE){
            std::cout << std::dec << (current_proc->getLinesExecuted())
                      << ":" << +current_proc->getID() << ":TERMINATED" << std::endl;
            processes.erase(processes.begin()+index);
            --NUM_PROCESSES;
        }
        getNextIndex(index);
    }
}


void Scheduler::getNextIndex(int& currentIndex) {

    if (currentIndex + 1 >= NUM_PROCESSES) {
        currentIndex = 0;
    } else {
        ++currentIndex;
    }
}