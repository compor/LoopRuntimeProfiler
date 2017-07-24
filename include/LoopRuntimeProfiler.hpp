//
//
//

#ifndef LOOPRUNTIMEPROFILER_HPP
#define LOOPRUNTIMEPROFILER_HPP

#include "Config.hpp"

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
#include "AnnotateLoops.hpp"
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

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
extern std::string ProfilerLoopStartFuncName;
extern std::string ProfilerLoopStopFuncName;

struct IncrementLoopInstrumentationPolicy {
  IncrementLoopInstrumentationPolicy() : m_ID(0) {}

  LoopInstrumentationID_t getInstrumentationID(const llvm::Loop &CurLoop) {
    return ++m_ID;
  }

private:
  LoopInstrumentationID_t m_ID;
};

struct DefaultRuntimeCallbacksPolicy {
  DefaultRuntimeCallbacksPolicy()
      : m_PrgStartFnName("lrp_program_start"),
        m_PrgStopFnName("lrp_program_stop"),
        m_LoopStartFnName("lrp_loop_start"), m_LoopStopFnName("lrp_loop_stop"),
        m_PrgEntryFnName("main") {}

  constexpr const std::string &programStart() const { return m_PrgStartFnName; }
  constexpr const std::string &programStop() const { return m_PrgStopFnName; }
  constexpr const std::string &loopStart() const { return m_LoopStartFnName; }
  constexpr const std::string &loopStop() const { return m_LoopStopFnName; }
  constexpr const std::string &programEntry() const { return m_PrgEntryFnName; }

private:
  const std::string m_PrgStartFnName;
  const std::string m_PrgStopFnName;
  const std::string m_LoopStartFnName;
  const std::string m_LoopStopFnName;
  const std::string m_PrgEntryFnName;
};

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
struct AnnotatatedLoopInstrumentationPolicy {
  AnnotatatedLoopInstrumentationPolicy() {}

  LoopInstrumentationID_t getInstrumentationID(const llvm::Loop &CurLoop) {
    return m_AL.hasAnnotatedId(CurLoop) ? m_AL.getAnnotatedId(CurLoop) : 0;
  }

private:
  icsa::AnnotateLoops m_AL;
};
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

template <typename RuntimeCallbacksPolicy = DefaultRuntimeCallbacksPolicy,
          typename LoopInstrumentationPolicy =
              IncrementLoopInstrumentationPolicy>
class Instrumenter : private LoopInstrumentationPolicy,
                     private RuntimeCallbacksPolicy {
public:
  void instrumentProgram(llvm::Module &CurMod) {
    auto *func = CurMod.getFunction(RuntimeCallbacksPolicy::programEntry());

    if (!func->isDeclaration())
      instrumentProgram(*func);

    return;
  }

  void instrumentProgram(llvm::Function &CurFunc) {
    auto &curCtx = CurFunc.getContext();
    auto *curModule = CurFunc.getParent();
    auto *insertionPoint = CurFunc.getEntryBlock().getFirstNonPHIOrDbg();

    auto *func =
        curModule->getOrInsertFunction(RuntimeCallbacksPolicy::programStart(),
                                       llvm::Type::getVoidTy(curCtx), nullptr);

    llvm::CallInst::Create(llvm::cast<llvm::Function>(func), "",
                           insertionPoint);

    return;
  }

  void instrumentLoop(llvm::Loop &CurLoop) {
    assert(CurLoop.getLoopPreheader() && "Loop does not have a preheader!");
    assert(CurLoop.getExitingBlock() &&
           "Loop does not have a single exiting block!");

    auto &curCtx = CurLoop.getHeader()->getContext();
    auto *curModule = CurLoop.getHeader()->getParent()->getParent();
    auto *startInsertionPoint =
        CurLoop.getLoopPreheader()->getFirstNonPHIOrDbg();
    auto *endInsertionPoint = CurLoop.getExitBlock()->getFirstNonPHIOrDbg();

    auto *startFunc = curModule->getOrInsertFunction(
        RuntimeCallbacksPolicy::loopStart(), llvm::Type::getVoidTy(curCtx),
        llvm::Type::getInt32Ty(curCtx), nullptr);

    auto *endFunc = curModule->getOrInsertFunction(
        RuntimeCallbacksPolicy::loopStop(), llvm::Type::getVoidTy(curCtx),
        llvm::Type::getInt32Ty(curCtx), nullptr);

    auto id = LoopInstrumentationPolicy::getInstrumentationID(CurLoop);

    llvm::SmallVector<llvm::Value *, 1> args;

    args.push_back(llvm::ConstantInt::get(
        llvm::IntegerType::get(
            curCtx, std::numeric_limits<LoopInstrumentationID_t>::digits),
        id));

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
