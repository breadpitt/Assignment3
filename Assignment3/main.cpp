/*
 * COMP3361 Winter 2018
 * Programming Assignment 2 Sample Solution
 */

/* 
 * File:   main.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 *
 * Created on October 28, 2017, 10:08 PM
 */

#include "PageFrameAllocator.h"
#include "ProcessTrace.h"

#include <MMU.h>

#include <cstdlib>
#include <iostream>

/*
 * 
 */
int main(int argc, char* argv[]) {
  // Use command line argument as file name
  if (argc != 2) {
    std::cerr << "usage: Lab2 trace_file\n";
    exit(1);
  }
  
  mem::MMU memory(256);
  PageFrameAllocator allocator(memory);
  ProcessTrace trace(memory, allocator, argv[1]);
  trace.Execute();
  return 0;
}