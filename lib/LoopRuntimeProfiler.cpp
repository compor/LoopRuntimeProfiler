//
//
//

#include "LoopRuntimeProfiler.hpp"

namespace icsa {
namespace LoopRuntimeProfiler {

std::string ProfilerProgramEntryFuncName = "main";
std::string ProfilerProgramStartFuncName = "lrp_program_start";
std::string ProfilerProgramStopFuncName = "lrp_program_stop";
std::string ProfilerLoopStartFuncName = "lrp_loop_start";
std::string ProfilerLoopStopFuncName = "lrp_loop_stop";

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock &BB) {
  auto &curCtx = BB.getContext();
  auto *curModule = BB.getParent()->getParent();
  auto *insertionPoint = BB.getFirstNonPHIOrDbg();

  auto *func = curModule->getOrInsertFunction(
      FuncName, llvm::Type::getVoidTy(curCtx), nullptr);

  llvm::CallInst::Create(llvm::cast<llvm::Function>(func), "", insertionPoint);

  return;
}

} // namespace LoopRuntimeProfiler
} // namespace icsa end
