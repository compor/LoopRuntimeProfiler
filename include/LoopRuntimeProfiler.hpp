//
//
//

#ifndef LOOPRUNTIMEPROFILER_HPP
#define LOOPRUNTIMEPROFILER_HPP

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/Value.h"
// using llvm::Value

#include "llvm/IR/Constants.h"
// using llvm::ConstantInt

#include "llvm/IR/Instructions.h"
// using llvm::CallInst

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Casting.h"
// using llvm::cast

#include <cstdint>
// using std::uint32_t

#include <limits>
// using std::numeric_limits

#include <string>
// using std::string

#include <cassert>
// using assert

namespace icsa {
namespace LoopRuntimeProfiler {

using LoopInstrumentationID_t = std::uint32_t;

extern std::string ProfilerProgramEntryFuncName;
extern std::string ProfilerProgramStartFuncName;
extern std::string ProfilerProgramStopFuncName;

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock &BB);

struct IncrementLoopInstrumentationPolicy final {
  IncrementLoopInstrumentationPolicy() : id(0) {}

  LoopInstrumentationID_t getInstrumentationID(const llvm::Loop &CurLoop) {
    return ++id;
  }

private:
  LoopInstrumentationID_t id;
};

template <typename LoopInstrumentationPolicy =
              IncrementLoopInstrumentationPolicy>
class Instrumenter : private LoopInstrumentationPolicy {
public:
  void instrumentLoop(const std::string &StartFuncName,
                      const std::string &EndFuncName, llvm::Loop &CurLoop) {
    assert(CurLoop.getLoopPreheader() && "Loop does not have a preheader!");
    assert(CurLoop.getExitingBlock() &&
           "Loop does not have a single exiting block!");

    auto &curCtx = CurLoop.getHeader()->getContext();
    auto *curModule = CurLoop.getHeader()->getParent()->getParent();
    auto *startInsertionPoint =
        CurLoop.getLoopPreheader()->getFirstNonPHIOrDbg();
    auto *endInsertionPoint = CurLoop.getExitBlock()->getFirstNonPHIOrDbg();

    auto *startFunc = curModule->getOrInsertFunction(
        StartFuncName, llvm::Type::getVoidTy(curCtx),
        llvm::Type::getInt32Ty(curCtx), nullptr);

    auto *endFunc = curModule->getOrInsertFunction(
        EndFuncName, llvm::Type::getVoidTy(curCtx),
        llvm::Type::getInt32Ty(curCtx), nullptr);

    auto id = LoopInstrumentationPolicy::getInstrumentationID(CurLoop);

    llvm::SmallVector<llvm::Value *, 1> args;

    args.push_back(llvm::ConstantInt::get(
        llvm::IntegerType::get(curCtx, 32),
        std::numeric_limits<LoopInstrumentationID_t>::digits));

    llvm::CallInst::Create(llvm::cast<llvm::Function>(startFunc), "",
                           startInsertionPoint);

    llvm::CallInst::Create(llvm::cast<llvm::Function>(endFunc), "",
                           endInsertionPoint);
    return;
  }
};

} // namespace LoopRuntimeProfiler
} // namespace icsa end

#endif
