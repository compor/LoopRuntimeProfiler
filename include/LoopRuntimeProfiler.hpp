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

extern std::string ProfileProgramStartFuncName;
extern std::string ProfileProgramStopFuncName;

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock *BB);

} // namespace LoopRuntimeProfiler
} // namespace icsa end

#endif
