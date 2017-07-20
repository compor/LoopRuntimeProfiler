//
//
//

#include "Config.hpp"

#include "Utils.hpp"

#include "LoopRuntimeProfilerPass.hpp"

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/Support/Casting.h"
// using llvm::dyn_cast

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::desc
// using llvm::cl::location

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro

#include <cassert>
// using assert

#define DEBUG_TYPE "loopruntimeprofiler"

#define STRINGIFY_UTIL(x) #x
#define STRINGIFY(x) STRINGIFY_UTIL(x)

#define PRJ_CMDLINE_DESC(x) x " (version: " STRINGIFY(VERSION_STRING) ")"

// plugin registration for opt

namespace icsa {

char LoopRuntimeProfilerPass::ID = 0;
static llvm::RegisterPass<LoopRuntimeProfilerPass>
    X("loop-runtime-profiler", PRJ_CMDLINE_DESC("loop runtime profiler"), false,
      false);

// plugin registration for clang

// the solution was at the bottom of the header file
// 'llvm/Transforms/IPO/PassManagerBuilder.h'
// create a static free-floating callback that uses the legacy pass manager to
// add an instance of this pass and a static instance of the
// RegisterStandardPasses class

static void
registerLoopRuntimeProfilerPass(const llvm::PassManagerBuilder &Builder,
                                llvm::legacy::PassManagerBase &PM) {
  PM.add(new LoopRuntimeProfilerPass());

  return;
}

static llvm::RegisterStandardPasses RegisterLoopRuntimeProfilerPass(
    llvm::PassManagerBuilder::EP_EarlyAsPossible,
    registerLoopRuntimeProfilerPass);

//

static llvm::cl::opt<unsigned int>
    LoopDepthLB("lrp-loop-depth-lb",
                llvm::cl::desc("loop depth lower bound (inclusive)"),
                llvm::cl::init(1u));

static llvm::cl::opt<unsigned int>
    LoopDepthUB("lrp-loop-depth-ub",
                llvm::cl::desc("loop depth upper bound (inclusive)"),
                llvm::cl::init(std::numeric_limits<unsigned>::max()));

static llvm::cl::list<unsigned int>
    LoopIDWhiteList("lrp-loop-id",
                    llvm::cl::desc("Specify loop ids to whitelist"),
                    llvm::cl::value_desc("loop id"), llvm::cl::ZeroOrMore);

static llvm::cl::opt<std::string>
    LoopIDWhiteListFilename("lrp-loop-id-whitelist",
                            llvm::cl::desc("loop id whitelist filename"));

#if LOOPRUNTIMEPROFILERPASS_DEBUG
bool passDebugFlag = false;
static llvm::cl::opt<bool, true>
    Debug("lrp-debug", llvm::cl::desc("debug loop runtime profiler pass"),
          llvm::cl::location(passDebugFlag));
#endif // LOOPRUNTIMEPROFILERPASS_DEBUG

namespace {

void checkCmdLineOptions(void) {
  assert(LoopDepthLB && LoopDepthUB && "Loop depth bounds cannot be zero!");

  assert(LoopDepthLB <= LoopDepthUB &&
         "Loop depth lower bound cannot be greater that upper!");

  return;
}

} // namespace anonymous end

//

bool LoopRuntimeProfilerPass::runOnModule(llvm::Module &CurMod) {
  checkCmdLineOptions();

  return false;
}

} // namespace icsa end
