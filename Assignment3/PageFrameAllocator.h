/*  PageAllocator - allocate pages in memory
 * 
 * File:   PageFrameAllocator.h
 * Author: Mike Goss <mikegoss@cs.du.edu>
 *
 * Created on February 2, 2018, 3:47 PM
 */

#ifndef PAGEFRAMEALLOCATOR_H
#define PAGEFRAMEALLOCATOR_H

#include <MMU.h>

#include <cstdint>
#include <string>
#include <vector>

class PageFrameAllocator {
public:
  /**
   * Constructor
   * 
   * Builds free list of all page frames.
   * 
   * @param page_frame_count
   */
  PageFrameAllocator(mem::MMU &mmu);
  
  virtual ~PageFrameAllocator() {}  // empty destrucor
  
  // Disallow copy/move
  PageFrameAllocator(const PageFrameAllocator &other) = delete;
  PageFrameAllocator(PageFrameAllocator &&other) = delete;
  PageFrameAllocator &operator=(const PageFrameAllocator &other) = delete;
  PageFrameAllocator &operator=(PageFrameAllocator &&other) = delete;
  
  /**
   * Allocate - allocate page frames from the free list.  Allocated pages
   *   are cleared to all 0.
   * 
   * @param count number of page frames to allocate
   * @param page_frames page frame addresses allocated are pushed on back
   * @return true if success, false if insufficient page frames (no frames allocated)
   */
  bool Allocate(mem::Addr count, std::vector<mem::Addr> &page_frames);
  
  /**
   * Deallocate - return page frames to free list
   * 
   * @param count number of page frames to free
   * @param page_frames contains page frame addresses to deallocate; numbers are
   *   popped from back of vector
   * @return true if success, false if insufficient page frames in vector
   */
  bool Deallocate(mem::Addr count, std::vector<mem::Addr> &page_frames);
  
  // Access to private values
  mem::Addr get_page_frames_free(void) const { return page_frames_free; }
  
  /**
   * FreeListToString - get string representation of free list
   * 
   * @return hex numbers of all free pages
   */
  std::string FreeListToString(void) const;
  
  static const mem::Addr kPageSize = 0x1000;
private:
  // Memory to be allocated
  mem::MMU &memory;
  
  // Number of first free page frame
  mem::Addr free_list_head;
  
  // Total number of page frames
  mem::Addr page_frames_total;
  
  // Current number of free page frames
  mem::Addr page_frames_free;
  
  // End of list marker
  static const mem::Addr kEndList = 0xFFFFFFFF;
};

#endif /* PAGEFRAMEALLOCATOR_H */

