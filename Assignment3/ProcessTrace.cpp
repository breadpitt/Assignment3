/*
 * ProcessTrace implementation 
 */

/* 
 * File:   ProcessTrace.cpp
 * Author: Mike Goss <mikegoss@cs.du.edu>
 * 
 */

#include "ProcessTrace.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace mem;

using std::cin;
using std::cout;
using std::cerr;
using std::getline;
using std::istringstream;
using std::string;
using std::vector;

ProcessTrace::ProcessTrace(MMU &memory_, 
                           PageFrameAllocator &allocator_, 
                           string file_name_) 
: memory(memory_), allocator(allocator_), file_name(file_name_), line_number(0) {
  // Open the trace file.  Abort program if can't open.
  trace.open(file_name, std::ios_base::in);
  if (!trace.is_open()) {
    cerr << "ERROR: failed to open trace file: " << file_name << "\n";
    exit(2);
  }
}

ProcessTrace::~ProcessTrace() {
  trace.close();
}

void ProcessTrace::Execute(void) {
  // Read and process commands
  string line;                // text line read
  string cmd;                 // command from line
  vector<uint32_t> cmdArgs;   // arguments from line
  
  // Set up PMCB and empty 1st level page table
  vector<Addr> allocated;
  allocator.Allocate(1, allocated);
  vmem_pmcb = mem::PMCB(true, allocated[0]);  // initialize PMCB
  memory.set_PMCB(vmem_pmcb);
  
  // Select the command to execute
  while (ParseCommand(line, cmd, cmdArgs)) {
    if (cmd == "alloc" ) {
      CmdAlloc(line, cmd, cmdArgs);    // allocate memory
    } else if (cmd == "compare") {
      CmdCompare(line, cmd, cmdArgs);  // get and compare multiple bytes
    } else if (cmd == "put") {
      CmdPut(line, cmd, cmdArgs);      // put bytes
    } else if (cmd == "fill") {
      CmdFill(line, cmd, cmdArgs);     // fill bytes with value
    } else if (cmd == "copy") {
      CmdCopy(line, cmd, cmdArgs);     // copy bytes to dest from source
    } else if (cmd == "dump") {
      CmdDump(line, cmd, cmdArgs);     // dump byte values to output
    } else if (cmd == "writable") {
      CmdWritable(line, cmd, cmdArgs); // change writable status of page(s)
    } else {
      if(!cmd.empty()) { // if not comment
        cerr << "ERROR: invalid command at line " << line_number << ":\n"
                << line << "\n";
        exit(2);
      }
    }
  }
}

bool ProcessTrace::ParseCommand(
    string &line, string &cmd, vector<uint32_t> &cmdArgs) {
  cmdArgs.clear();
  line.clear();
  cmd.clear();
  
  // Read next line
  if (std::getline(trace, line)) {
    ++line_number;
    cout << std::dec << line_number << ":" << line << "\n";
    
    // If not comment
    if(line.at(0) != '#') {

      // Make a string stream from command line
      istringstream lineStream(line);

      // Get command
      lineStream >> cmd;

      // Get arguments
      uint32_t arg;
      while(lineStream >> std::hex >> arg) {
        cmdArgs.push_back(arg);
      }
    }
    return true;
  } else if (trace.eof()) {
      return false;
  } else {
    cerr << "ERROR: getline failed on trace file: " << file_name 
            << "at line " << line_number << "\n";
    exit(2);
  }
}

void ProcessTrace::CmdAlloc(const string &line, 
                            const string &cmd, 
                            const vector<uint32_t> &cmdArgs) {
  // Get arguments
  Addr vaddr = cmdArgs.at(0);
  int count = cmdArgs.at(1) / kPageSize;
  
  // Switch to physical mode
  memory.get_PMCB(vmem_pmcb);
  memory.set_PMCB(pmem_pmcb);
  
  Addr pt_base = vmem_pmcb.page_table_base;
  
  // Allocate pages, initialized to writable
  while (count-- > 0) {
    AllocateAndMapPage(vaddr);
    vaddr += 0x1000;
  }
  
  // Switch back to virtual mode
  memory.set_PMCB(vmem_pmcb); 
}

void ProcessTrace::CmdCompare(const string &line,
                              const string &cmd,
                              const vector<uint32_t> &cmdArgs) {
  uint32_t addr = cmdArgs.at(0);

  // Compare specified byte values
  size_t num_bytes = cmdArgs.size() - 1;
  uint8_t buffer[num_bytes];
  try {
    memory.get_bytes(buffer, addr, num_bytes);
    for(int i = 1; i < cmdArgs.size(); ++i) {
      if(buffer[i - 1] != cmdArgs.at(i)) {
        cout << "compare error at address " << std::hex << addr
                << ", expected " << static_cast<uint32_t> (cmdArgs.at(i))
                << ", actual is " << static_cast<uint32_t> (buffer[i - 1]) << "\n";
      }
      ++addr;
    }
  }  catch(PageFaultException e) {
    PrintAndClearException("PageFaultException", e);
  }
}

void ProcessTrace::CmdPut(const string &line,
                          const string &cmd,
                          const vector<uint32_t> &cmdArgs) {
  // Put multiple bytes starting at specified address
  uint32_t addr = cmdArgs.at(0);
  size_t num_bytes = cmdArgs.size() - 1;
  uint8_t buffer[num_bytes];
  try {
    for(int i = 1; i < cmdArgs.size(); ++i) {
      buffer[i - 1] = cmdArgs.at(i);
    }
    memory.put_bytes(addr, num_bytes, buffer);
  }  catch(PageFaultException e) {
    PrintAndClearException("PageFaultException", e);
  }  catch(WritePermissionFaultException e) {
    PrintAndClearException("WritePermissionFaultException", e);
  }
}

void ProcessTrace::CmdCopy(const string &line,
                           const string &cmd,
                           const vector<uint32_t> &cmdArgs) {
  // Copy specified number of bytes to destination from source
  Addr dst = cmdArgs.at(0);
  Addr src = cmdArgs.at(1);
  Addr num_bytes = cmdArgs.at(2);
  uint8_t buffer[num_bytes];

  // Try reading bytes
  Addr bytes_read = 0;  // number of successfully read bytes
  try {
    memory.get_bytes(buffer, src, num_bytes);
    bytes_read = num_bytes;  // all bytes read
  }  catch(PageFaultException e) {
    PrintAndClearException("PageFaultException on read", e);
    memory.get_PMCB(vmem_pmcb);  // get address which cause exception
    bytes_read = vmem_pmcb.next_vaddress - src;  // number of bytes read
  }
  
  // Try writing bytes
  if(bytes_read != 0) {
    try {
      memory.put_bytes(dst, bytes_read, buffer);
    } catch(PageFaultException e) {
      PrintAndClearException("PageFaultException on write", e);
    } catch(WritePermissionFaultException e) {
      PrintAndClearException("WritePermissionFaultException", e);
    }
  }
}

void ProcessTrace::CmdFill(const string &line,
                           const string &cmd,
                           const vector<uint32_t> &cmdArgs) {
  // Fill a sequence of bytes with the specified value
  Addr addr = cmdArgs.at(0);
  Addr num_bytes = cmdArgs.at(1);
  uint8_t val = cmdArgs.at(2);
  try {
    for(int i = 0; i < num_bytes; ++i) {
      memory.put_byte(addr++, &val);
    }
  } catch(PageFaultException e) {
    PrintAndClearException("PageFaultException", e);
  } catch(WritePermissionFaultException e) {
    PrintAndClearException("WritePermissionFaultException", e);
  }
}

void ProcessTrace::CmdDump(const string &line,
                           const string &cmd,
                           const vector<uint32_t> &cmdArgs) {
  uint32_t addr = cmdArgs.at(0);
  uint32_t count = cmdArgs.at(1);

  // Output the address
  cout << std::hex << addr;

  // Output the specified number of bytes starting at the address
  try {
    for(int i = 0; i < count; ++i) {
      if((i % 16) == 0) { // line break every 16 bytes
        cout << "\n";
      }
      uint8_t byte_val;
      memory.get_byte(&byte_val, addr++);
      cout << " " << std::setfill('0') << std::setw(2)
              << static_cast<uint32_t> (byte_val);
    }
    cout << "\n";
  } catch(PageFaultException e) {
    cout << "\n";
    PrintAndClearException("PageFaultException", e);
  }
}

void ProcessTrace::CmdWritable(const string &line,
                               const string &cmd,
                               const vector<uint32_t> &cmdArgs) {
 // Get arguments
  Addr vaddr = cmdArgs.at(0);
  int count = cmdArgs.at(1) / kPageSize;
  bool writable = cmdArgs.at(2) != 0;
  
  // Switch to physical mode
  memory.get_PMCB(vmem_pmcb);
  memory.set_PMCB(pmem_pmcb);
  
  Addr pt_base = vmem_pmcb.page_table_base;
  
  // Modify pages in range
  while (count-- > 0) {
    SetWritableStatus(vaddr, writable);
    vaddr += 0x1000;
  }
  
  // Switch back to virtual mode
  memory.set_PMCB(vmem_pmcb); 
}

void ProcessTrace::PrintAndClearException(const string &type, 
                                          MemorySubsystemException e) {
  memory.get_PMCB(vmem_pmcb);
  cout << "Exception type " << type 
          << " occurred at input line " << std::dec << std::setw(1) 
          << line_number << " at virtual address 0x" 
          << std::hex << std::setw(8) << std::setfill('0') 
          << vmem_pmcb.next_vaddress 
          << ": " << e.what() << "\n";
  vmem_pmcb.operation_state = PMCB::NONE;
  memory.set_PMCB(vmem_pmcb);
}

void ProcessTrace::AllocateAndMapPage(Addr vaddr) {
  // Get offset in L1 table of L2 entry for vaddr  
  Addr pt_base = vmem_pmcb.page_table_base;
  Addr pt_l1_offset = vaddr >> (kPageSizeBits + kPageTableSizeBits);

  // Get L1 page table entry
  Addr l1_entry_addr = pt_base + sizeof(PageTableEntry) * pt_l1_offset;
  PageTableEntry l1_entry;
  memory.get_bytes(reinterpret_cast<uint8_t*> (&l1_entry),
                 l1_entry_addr, sizeof(PageTableEntry));

  // If no L1 entry for page, allocate and map one
  if((l1_entry & kPTE_PresentMask) == 0) {
    vector<Addr> allocated;
    allocator.Allocate(1, allocated);
    l1_entry = allocated[0] | kPTE_PresentMask | kPTE_WritableMask;
    memory.put_bytes(l1_entry_addr, sizeof(PageTableEntry),
                     reinterpret_cast<uint8_t*> (&l1_entry));
  }
  
  // Get L2 page table entry
  Addr pt_l2_addr = l1_entry & kPageNumberMask;
  Addr pt_l2_offset = (vaddr >> kPageSizeBits) & kPageTableIndexMask;
  Addr l2_entry_addr = pt_l2_addr + sizeof(PageTableEntry) * pt_l2_offset;
  PageTableEntry l2_entry;
  memory.get_bytes(reinterpret_cast<uint8_t*> (&l2_entry),
                 l2_entry_addr, sizeof(PageTableEntry));
  
  // Error if page already allocated
  if ((l2_entry & kPTE_PresentMask) != 0) {
    cerr << "ERROR: duplicate allocated at vaddr = 0x" 
            << std::hex << vaddr << "\n";
    throw std::bad_alloc();
  }
  
  // Allocate a page and set up page table entry
  vector<Addr> allocated;
  allocator.Allocate(1, allocated);
  l2_entry = allocated[0] | kPTE_PresentMask | kPTE_WritableMask;
  memory.put_bytes(l2_entry_addr, sizeof(PageTableEntry),
                 reinterpret_cast<uint8_t*> (&l2_entry));
}

void ProcessTrace::SetWritableStatus(Addr vaddr, bool writable) {
  // Get offset in L1 table of L2 entry for vaddr  
  Addr pt_base = vmem_pmcb.page_table_base;
  Addr pt_l1_offset = vaddr >> (kPageSizeBits + kPageTableSizeBits);

  // Get L1 page table entry
  Addr l1_entry_addr = pt_base + sizeof(PageTableEntry) * pt_l1_offset;
  PageTableEntry l1_entry;
  memory.get_bytes(reinterpret_cast<uint8_t*> (&l1_entry),
                 l1_entry_addr, sizeof(PageTableEntry));

  // If no L1 entry for page, ignore request
  if((l1_entry & kPTE_PresentMask) == 0) {
    return;
  }
  
  // Get L2 page table entry
  Addr pt_l2_addr = l1_entry & kPageNumberMask;
  Addr pt_l2_offset = (vaddr >> kPageSizeBits) & kPageTableIndexMask;
  Addr l2_entry_addr = pt_l2_addr + sizeof(PageTableEntry) * pt_l2_offset;
  PageTableEntry l2_entry;
  memory.get_bytes(reinterpret_cast<uint8_t*> (&l2_entry),
                 l2_entry_addr, sizeof(PageTableEntry));
  
  // Ignore request if page not present
  if ((l2_entry & kPTE_PresentMask) == 0) {
    return;
  }
  
  // Set status to requested value and rewrite entry
  l2_entry = (l2_entry & ~kPTE_WritableMask) | (writable ? kPTE_WritableMask : 0);
  memory.put_bytes(l2_entry_addr, sizeof(PageTableEntry),
                 reinterpret_cast<uint8_t*> (&l2_entry));
}