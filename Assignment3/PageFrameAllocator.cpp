/*  PageAllocator - allocate pages in MMU memory
 * 
 * File:   PageFrameAllocator.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * 
 * Created on February 2, 2018, 3:47 PM
 */

#include "PageFrameAllocator.h"

#include <array>
#include <cstring>
#include <sstream>

using mem::Addr;
using mem::kPageSize;

PageFrameAllocator::PageFrameAllocator(mem::MMU &mmu) 
: memory(mmu),
  page_frames_total(memory.get_frame_count()),
  page_frames_free(memory.get_frame_count()),
  free_list_head(0)
{
  // Add all page frames to free list
  Addr last_page_addr = (page_frames_total - 1) * kPageSize;
  for (Addr frame_addr = 0; frame_addr < last_page_addr; frame_addr += kPageSize) {
    Addr next = frame_addr + kPageSize;
    memory.put_bytes(frame_addr, sizeof(Addr), reinterpret_cast<uint8_t*>(&next));
  }
  
  // Last page frame has end of list marker
  Addr end_list = kEndList;
  memory.put_bytes(last_page_addr, sizeof(Addr), 
                   reinterpret_cast<uint8_t*>(&end_list));
}

bool PageFrameAllocator::Allocate(Addr count, 
                                  std::vector<Addr> &page_frames) {
  if (count <= page_frames_free) {  // if enough to allocate
    std::array<uint8_t, kPageSize> zero_page;  // page of all 0
    zero_page.fill(0);
    
    while (count-- > 0) {
      // Return next free frame to caller
      page_frames.push_back(free_list_head);
      
      // De-link frame from head of free list
      memory.get_bytes(reinterpret_cast<uint8_t*>(&free_list_head), 
                       free_list_head, sizeof(Addr));
      
      // Clear allocated page to all 0
      memory.put_bytes(page_frames.back(), kPageSize, &zero_page[0]);
      
      --page_frames_free;
    }
    return true;
  } else {
    return false;  // do nothing and return error
  }
}

bool PageFrameAllocator::Deallocate(Addr count,
                                    std::vector<Addr> &page_frames) {
  // If enough to deallocate
  if(count <= page_frames.size()) {
    while(count-- > 0) {
      // Return next frame to head of free list
      Addr frame = page_frames.back();
      page_frames.pop_back();
      memory.put_bytes(frame, sizeof(Addr), 
                       reinterpret_cast<uint8_t*>(&free_list_head));
      free_list_head = frame;
      ++page_frames_free;
    }
    return true;
  } else {
    return false; // do nothing and return error
  }
}

std::string PageFrameAllocator::FreeListToString(void) const {
  std::ostringstream out_string;
  
  Addr next_free = free_list_head;
  
  while (next_free != kEndList) {
    out_string << " " << std::hex << next_free;
    memory.get_bytes(reinterpret_cast<uint8_t*>(&next_free), 
                     sizeof(Addr), next_free);
  }
  
  return out_string.str();
}