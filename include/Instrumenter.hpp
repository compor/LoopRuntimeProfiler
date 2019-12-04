//
//
//

#ifndef INSTRUMENTER_HPP
#define INSTRUMENTER_HPP

#include "Config.hpp"

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/DerivedTypes.h"
// using llvm::FunctionType

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

#include "llvm/IR/CFG.h"
// using llvm::successors

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Casting.h"
// using llvm::cast

#include <utility>
// std::forward
// using std::pair
// using std::make_pair

#include <tuple>
// using std::tuple
// using std::make_tuple

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

struct DefaultRuntimeCallbacksPolicy {
  DefaultRuntimeCallbacksPolicy()
      : m_PrgStartFnName("lrp_program_start"),
        m_PrgStopFnName("lrp_program_stop"),
        m_LoopStartFnName("lrp_loop_start"), m_LoopStopFnName("lrp_loop_stop"),
        m_LoopHeaderFnName("lrp_loop_header"),
        m_LoopBodyFnName("lrp_loop_body"), m_LoopLatchFnName("lrp_loop_latch"),
        m_PrgEntryFnName("main") {}

  constexpr const std::string &programStart() const { return m_PrgStartFnName; }
  constexpr const std::string &programStop() const { return m_PrgStopFnName; }
  constexpr const std::string &loopStart() const { return m_LoopStartFnName; }
  constexpr const std::string &loopStop() const { return m_LoopStopFnName; }
  constexpr const std::string &loopHeader() const { return m_LoopHeaderFnName; }
  constexpr const std::string &loopBody() const { return m_LoopBodyFnName; }
  constexpr const std::string &loopLatch() const { return m_LoopLatchFnName; }
  constexpr const std::string &programEntry() const { return m_PrgEntryFnName; }

private:
  const std::string m_PrgStartFnName;
  const std::string m_PrgStopFnName;
  const std::string m_LoopStartFnName;
  const std::string m_LoopStopFnName;
  const std::string m_LoopHeaderFnName;
  const std::string m_LoopBodyFnName;
  const std::string m_LoopLatchFnName;
  const std::string m_PrgEntryFnName;
};

template <typename RuntimeCallbacksPolicy = DefaultRuntimeCallbacksPolicy>
class Instrumenter : private RuntimeCallbacksPolicy {
private:
  const std::string m_InitFnName;

  decltype(auto) insertVarargFunction(const std::string &name,
                                      llvm::Module *CurMod) {
    auto *funcType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(CurMod->getContext()), true);

    return CurMod->getOrInsertFunction(name, funcType);
  }

  llvm::CallInst *instrumentInit(llvm::BasicBlock &BB) {
    static bool hasBeenCalled = false;

    if (hasBeenCalled)
      return nullptr;

    hasBeenCalled = true;

    auto *func =
        insertVarargFunction(m_InitFnName, BB.getParent()->getParent());

    auto *insertBefore = BB.getFirstNonPHIOrDbg();
    return llvm::CallInst::Create(llvm::cast<llvm::Function>(func), "",
                                  insertBefore);
  }

public:
  Instrumenter() : m_InitFnName("lrp_init") {}

  template <typename... Ts>
  llvm::CallInst *instrumentProgram(llvm::Module &CurMod, Ts... args) {
    auto *func = CurMod.getFunction(RuntimeCallbacksPolicy::programEntry());

    assert(func && "Program entry function was not found!");

    return !func->isDeclaration()
               ? instrumentProgram(*func, std::forward<Ts>(args)...)
               : nullptr;
  }

  template <typename... Ts>
  llvm::CallInst *instrumentProgram(llvm::Function &CurFunc, Ts... args) {
    static bool hasBeenCalled = false;

    if (hasBeenCalled)
      return nullptr;

    hasBeenCalled = true;

    auto *func = insertVarargFunction(RuntimeCallbacksPolicy::programStart(),
                                      CurFunc.getParent());

    constexpr const int size = sizeof...(args);
    llvm::SmallVector<llvm::Value *, size> fargs{args...};

    auto *insertBefore = CurFunc.getEntryBlock().getFirstNonPHIOrDbg();
    auto *call = llvm::CallInst::Create(llvm::cast<llvm::Function>(func), fargs,
                                        "", insertBefore);

    auto *insertPoint = instrumentInit(CurFunc.getEntryBlock());
    assert(insertPoint && "Call to runtime init has already been added!");

    return call;
  }

  template <typename... Ts>
  decltype(auto) instrumentLoop(llvm::Loop &CurLoop, Ts... args) {
    auto *curPhdr = CurLoop.getLoopPreheader();
    assert(curPhdr && "Loop does not have a preheader!");
    auto *curHdr = CurLoop.getHeader();

    auto &curCtx = curHdr->getContext();
    auto *curModule = curHdr->getParent()->getParent();

    auto *startFunc =
        insertVarargFunction(RuntimeCallbacksPolicy::loopStart(), curModule);
    auto *endFunc =
        insertVarargFunction(RuntimeCallbacksPolicy::loopStop(), curModule);

    constexpr const int size = sizeof...(args);
    llvm::SmallVector<llvm::Value *, size> fargs{args...};

    auto *startInsertionPoint = curPhdr->getFirstNonPHIOrDbg();
    auto *call1 = llvm::CallInst::Create(llvm::cast<llvm::Function>(startFunc),
                                         fargs, "", startInsertionPoint);
    std::vector<llvm::CallInst *> calls(1, call1);

    llvm::SmallVector<llvm::BasicBlock *, 5> exits;
    CurLoop.getExitBlocks(exits);

    for (auto &e : exits) {
      auto *call2 = llvm::CallInst::Create(llvm::cast<llvm::Function>(endFunc),
                                           fargs, "", e->getFirstNonPHIOrDbg());
      calls.push_back(call2);
    }

    return std::make_tuple(calls);
  }

  template <typename... Ts>
  decltype(auto) instrumentLoopHeader(llvm::Loop &CurLoop, Ts... args) {
    auto *curHdr = CurLoop.getHeader();
    auto &curCtx = curHdr->getContext();
    auto *curModule = curHdr->getParent()->getParent();

    auto *headerFunc =
        insertVarargFunction(RuntimeCallbacksPolicy::loopHeader(), curModule);

    constexpr const int size = sizeof...(args);
    llvm::SmallVector<llvm::Value *, size> fargs{args...};

    auto *startInsertionPoint = curHdr->getFirstNonPHIOrDbgOrLifetime();
    auto *call1 = llvm::CallInst::Create(llvm::cast<llvm::Function>(headerFunc),
                                         fargs, "", startInsertionPoint);
    std::vector<llvm::CallInst *> calls(1, call1);
    return std::make_tuple(calls);
  }

  template <typename... Ts>
  decltype(auto) instrumentLoopBody(llvm::Loop &CurLoop, Ts... args) {
    auto *curHdr = CurLoop.getHeader();
    auto &curCtx = curHdr->getContext();
    auto *curModule = curHdr->getParent()->getParent();

    auto *bodyFunc =
        insertVarargFunction(RuntimeCallbacksPolicy::loopBody(), curModule);

    constexpr const int size = sizeof...(args);
    llvm::SmallVector<llvm::Value *, size> fargs{args...};

    auto *startInsertionPoint = curHdr->getFirstNonPHIOrDbgOrLifetime();
    auto *call1 = llvm::CallInst::Create(llvm::cast<llvm::Function>(bodyFunc),
                                         fargs, "", startInsertionPoint);

    std::vector<llvm::CallInst *> calls;

    for (auto *e : llvm::successors(curHdr)) {
      auto *call = llvm::CallInst::Create(llvm::cast<llvm::Function>(bodyFunc),
                                          fargs, "", e->getFirstNonPHIOrDbg());
      calls.push_back(call);
    }

    return std::make_tuple(calls);
  }

  template <typename... Ts>
  decltype(auto) instrumentLoopLatches(llvm::Loop &CurLoop, Ts... args) {
    auto *curHdr = CurLoop.getHeader();
    auto &curCtx = curHdr->getContext();
    auto *curModule = curHdr->getParent()->getParent();

    auto *latchFunc =
        insertVarargFunction(RuntimeCallbacksPolicy::loopLatch(), curModule);

    constexpr const int size = sizeof...(args);
    llvm::SmallVector<llvm::Value *, size> fargs{args...};

    llvm::SmallVector<llvm::BasicBlock *, 5> latches;
    CurLoop.getLoopLatches(latches);
    std::vector<llvm::CallInst *> calls;

    for (auto &e : latches) {
      auto *call = llvm::CallInst::Create(llvm::cast<llvm::Function>(latchFunc),
                                          fargs, "", e->getFirstNonPHIOrDbg());
      calls.push_back(call);
    }

    return std::make_tuple(calls);
  }
};

} // namespace LoopRuntimeProfiler
} // namespace icsa

#endif // INSTRUMENTER_HPP
