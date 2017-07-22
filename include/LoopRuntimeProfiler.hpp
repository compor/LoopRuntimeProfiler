//
//
//

#ifndef LOOPRUNTIMEPROFILER_HPP
#define LOOPRUNTIMEPROFILER_HPP

#include <cstdint>
// using std::uint32_t

#include <limits>
// using std::numeric_limits

#include <string>
// using std::string

namespace llvm {
class BasicBlock;
class Loop;
} // namespace llvm end

namespace icsa {
namespace LoopRuntimeProfiler {

extern std::string ProfilerProgramEntryFuncName;
extern std::string ProfilerProgramStartFuncName;
extern std::string ProfilerProgramStopFuncName;

extern int IdBits;

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock &BB);

void instrumentLoop(const std::string &StartFuncName,
                    const std::string &EndFuncName, llvm::Loop &CurLoop);

} // namespace LoopRuntimeProfiler
} // namespace icsa end

#endif
