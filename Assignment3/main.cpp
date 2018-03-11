/*
 * COMP3361 Winter 2018
 * Programming Assignment 2 Sample Solution
 */
/*
 * Assignment 3
 *
 * Group 5 v4  Robert Zachrich & James Shannon
 */

#include "PageFrameAllocator.h"
#include "ProcessTrace.h"
#include "Scheduler.h"

#include <MMU.h>

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>


int main(int argc, char* argv[]) {
  // Use command line argument as file name
  if (argc != 2) {
    std::cerr << "usage: Lab2 trace_file\n";
    exit(1);
  }


  // Creates an instance with 1024 page frames
  mem::MMU memory(1024);
  std::vector<std::string> file_names;

  // Each process will have its own set of frames
  PageFrameAllocator allocator(memory);

  // stoi = string object
  int time_slice = std::stoi(argv[1]);
  for (int i = 2; i < argc; ++i) {
    file_names.push_back(argv[i]);
  }


  //ProcessTrace trace(memory, allocator, argv[1]);
  //trace.Execute();

  // Executer
  Scheduler scheduler(file_names, memory, allocator, time_slice);
    scheduler.Execute();

  return 0;
}