//
//
//

#include "LoopRuntimeProfiler.hpp"

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

#include <string>
// using std::string

#include <cassert>
// using assert

namespace icsa {
namespace LoopRuntimeProfiler {

std::string ProfilerProgramEntryFuncName = "main";
std::string ProfilerProgramStartFuncName = "lrp_program_start";
std::string ProfilerProgramStopFuncName = "lrp_program_stop";

int IdBits = std::numeric_limits<std::uint32_t>::digits;

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock &BB) {
  auto &curCtx = BB.getContext();
  auto *curModule = BB.getParent()->getParent();
  auto *insertionPoint = BB.getFirstNonPHIOrDbg();

  auto *func = curModule->getOrInsertFunction(
      FuncName, llvm::Type::getVoidTy(curCtx), nullptr);

  llvm::CallInst::Create(llvm::cast<llvm::Function>(func), "", insertionPoint);

  return;
}

void instrumentLoop(const std::string &StartFuncName,
                    const std::string &EndFuncName, llvm::Loop &CurLoop) {
  assert(CurLoop.getLoopPreheader() && "Loop does not have a preheader!");
  assert(CurLoop.getExitingBlock() &&
         "Loop does not have a single exiting block!");

  auto &curCtx = CurLoop.getHeader()->getContext();
  auto *curModule = CurLoop.getHeader()->getParent()->getParent();
  auto *startInsertionPoint = CurLoop.getLoopPreheader()->getFirstNonPHIOrDbg();
  auto *endInsertionPoint = CurLoop.getExitBlock()->getFirstNonPHIOrDbg();

  auto *startFunc = curModule->getOrInsertFunction(
      StartFuncName, llvm::Type::getVoidTy(curCtx),
      llvm::Type::getInt32Ty(curCtx), nullptr);

  auto *endFunc =
      curModule->getOrInsertFunction(EndFuncName, llvm::Type::getVoidTy(curCtx),
                                     llvm::Type::getInt32Ty(curCtx), nullptr);

  llvm::SmallVector<llvm::Value *, 1> args;
  args.push_back(llvm::ConstantInt::get(llvm::IntegerType::get(curCtx, 32), 0));

  llvm::CallInst::Create(llvm::cast<llvm::Function>(startFunc), "",
                         startInsertionPoint);

  llvm::CallInst::Create(llvm::cast<llvm::Function>(endFunc), "",
                         endInsertionPoint);
  return;
}

} // namespace LoopRuntimeProfiler
} // namespace icsa end
