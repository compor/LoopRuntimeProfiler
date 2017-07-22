//
//
//

#ifndef LOOPRUNTIMEPROFILER_HPP
#define LOOPRUNTIMEPROFILER_HPP

#include <string>
// using std::string

namespace llvm {
class BasicBlock;
} // namespace llvm end

namespace icsa {
namespace LoopRuntimeProfiler {

extern std::string ProfilerProgramEntryFuncName;
extern std::string ProfilerProgramStartFuncName;
extern std::string ProfilerProgramStopFuncName;

extern int IdBits;

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock &BB);

} // namespace LoopRuntimeProfiler
} // namespace icsa end

#endif
