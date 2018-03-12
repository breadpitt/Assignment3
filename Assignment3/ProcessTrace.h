
#ifndef PROCESSTRACE_H
#define PROCESSTRACE_H

#include "PageFrameAllocator.h"

#include <MMU.h>

#include <fstream>
#include <string>
#include <vector>


class ProcessTrace {
public:
  /**
   * Constructor - open trace file, initialize processing
   * 
   * @param memory_ MMU to use for memory
   * @param file_name_ source of trace commands
   */
  ProcessTrace(mem::MMU &memory_,
               PageFrameAllocator &allocator,
               std::string file_name_, int id);
  
  /**
   * Destructor - close trace file, clean up processing
   */
  virtual ~ProcessTrace(void);

  // Other constructors, assignment
  ProcessTrace(const ProcessTrace &other) = delete;
  ProcessTrace(ProcessTrace &&other) = delete;
  ProcessTrace operator=(const ProcessTrace &other) = delete;
  ProcessTrace operator=(ProcessTrace &&other) = delete;
  
  /**
   * Execute - read and process commands from trace file
   * 
   */
  int Execute(int num_lines);
    int getID() {return id_number;}
    int getLinesExecuted() {return line_number; }
  
private:
  // Trace file
  std::string file_name;
  std::fstream trace;
  long line_number;
  //long process_number;
    int id_number;
    int allocated_pages;
  long quota; // Number of pages a process is allowed
    int QUOTA;
  //long num_pages_alloc; // Keeps track of how many pages have been allocated

    // Memory contents
  mem::MMU &memory;
  
  // Virtual and physical mode PMCBs
  mem::PMCB vmem_pmcb;
  mem::PMCB pmem_pmcb;
  
  // Memory allocator
  PageFrameAllocator &allocator;
  
  /**
   * ParseCommand - parse a trace file command.
   *   Aborts program if invalid trace file.
   * 
   * @param line return the original command line
   * @param cmd return the command name
   * @param cmdArgs returns a vector of argument bytes
   * @return true if command parsed, false if end of file
   */
  bool ParseCommand(
      std::string &line, std::string &cmd, std::vector<uint32_t> &cmdArgs);
  
  /**
   * Command executors. Arguments are the same for each command.
   *   Form of the function is CmdX, where "X' is the command name, capitalized.
   * @param line original text of command line
   * @param cmd command, converted to all lower case
   * @param cmdArgs arguments to command
   */

  void CmdQuota(const std::string &line, 
                const std::string &cmd, 
                const std::vector<uint32_t> &cmdArgs);
  //void CmdAlloc(const Addr addr);
  void CmdCompare(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  bool CmdPut(const std::string &line, 
              const std::string &cmd, 
              const std::vector<uint32_t> &cmdArgs);
  bool CmdFill(const std::string &line, 
               const std::string &cmd, 
               const std::vector<uint32_t> &cmdArgs);
  bool CmdCopy(const std::string &line, 
               const std::string &cmd, 
               const std::vector<uint32_t> &cmdArgs);
  void CmdDump(const std::string &line, 
               const std::string &cmd, 
               const std::vector<uint32_t> &cmdArgs);
  void CmdWritable(const std::string &line, 
                   const std::string &cmd, 
                   const std::vector<uint32_t> &cmdArgs);
  
  /**
   * PrintAndClearException - print a memory exception and clear operation
   *   in PMCB.
   * 
   * @param type description of exception
   * @param e exception object
   */
  void PrintAndClearException(const std::string &type, 
                              mem::MemorySubsystemException e);
  
  /**
   * AllocateAndMapPage - allocate a new user page and add it to the page table
   * 
   * @param vaddr virtual address of page to be mapped
   */
  void AllocateAndMapPage(mem::Addr vaddr);
  
  /**
   * SetWritableStatus - set the writable status of the page
   * 
   * @param vaddr virtual address of page to modify
   * @param writable true to make writable, false to make read-only
   */
  void SetWritableStatus(mem::Addr vaddr, bool writable);
};

#endif /* PROCESSTRACE_H */
