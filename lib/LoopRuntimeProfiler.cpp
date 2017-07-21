//
//
//

#include "LoopRuntimeProfiler.hpp"

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/Value.h"
// using llvm::Value

#include "llvm/IR/Instructions.h"
// using llvm::CallInst

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/IR/Function.h"

#include "llvm/IR/BasicBlock.h"
// using llvm::BasicBlock

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/Casting.h"
// using llvm::cast

#include <string>
// using std::string

namespace icsa {
namespace LoopRuntimeProfiler {

std::string ProfileProgramStartFuncName = "lrp_program_start";
std::string ProfileProgramStopFuncName = "lrp_program_stop";

void instrumentProgramStart(const std::string &FuncName, llvm::BasicBlock *BB) {
  auto &curCtx = BB->getContext();
  auto *curModule = BB->getParent()->getParent();
  auto *insertionPoint = BB->getFirstNonPHIOrDbg();

  auto *func =
      curModule->getOrInsertFunction(FuncName, llvm::Type::getVoidTy(curCtx),
                                     llvm::Type::getVoidTy(curCtx), nullptr);

  llvm::SmallVector<llvm::Value *, 1> funcArgs;

  llvm::CallInst::Create(llvm::cast<llvm::Function>(func), funcArgs,
                         FuncName + "call", insertionPoint);

  return;
}

} // namespace LoopRuntimeProfiler
} // namespace icsa end
