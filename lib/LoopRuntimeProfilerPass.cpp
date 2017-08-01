//
//
//

#include "Config.hpp"

#include "Utils.hpp"

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
#include "AnnotateLoops.hpp"
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

#include "LoopRuntimeProfilerPass.hpp"

#include "LoopRuntimeProfiler.hpp"

#include "Instrumenter.hpp"

#include "llvm/Pass.h"
// using llvm::RegisterPass

#include "llvm/IR/Type.h"
// using llvm::Type

#include "llvm/IR/DerivedTypes.h"
// using llvm::IntegerType

#include "llvm/IR/Instruction.h"
// using llvm::Instruction

#include "llvm/IR/Function.h"
// using llvm::Function

#include "llvm/IR/Module.h"
// using llvm::Module

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass
// using llvm::LoopInfo

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

#include "llvm/Transforms/Scalar.h"
// using char llvm::LoopInfoSimplifyID

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/Support/CommandLine.h"
// using llvm::cl::opt
// using llvm::cl::list
// using llvm::cl::desc
// using llvm::cl::value_desc
// using llvm::cl::location
// using llvm::cl::ZeroOrMore

#include "llvm/Support/raw_ostream.h"
// using llvm::raw_ostream

#include "llvm/Support/Debug.h"
// using DEBUG macro
// using llvm::dbgs

#include "llvm/IR/Verifier.h"
// using llvm::verifyFunction

#include <set>
// using std::set

#include <algorithm>
// using std::for_each

#include <string>
// using std::string
// using std::stoul

#include <fstream>
// using std::ifstream

#include <limits>
// using std::numeric_limits

#include <cassert>
// using assert

#define DEBUG_TYPE "loop_runtime_profiler"

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

enum LRPOpts {
  module,
  callgraph,
};

static llvm::cl::opt<LRPOpts> OperationMode(
    "lrp-mode", llvm::cl::desc("operation mode"),
    llvm::cl::values(clEnumVal(module, "module mode"),
                     clEnumVal(callgraph, "call graph mode"), nullptr));

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

  bool hasModuleChanged = false;
  bool useLoopIDWhitelist = !LoopIDWhiteListFilename.empty();
  llvm::SmallVector<llvm::Loop *, 16> workList;
  std::set<unsigned> loopIDs;

  if (OperationMode == LRPOpts::callgraph) {
    return false;
  }

  if (useLoopIDWhitelist) {
    std::ifstream loopIDWhiteListFile{LoopIDWhiteListFilename};

    if (loopIDWhiteListFile.is_open()) {
      std::string loopID;

      while (loopIDWhiteListFile >> loopID) {
        if (loopID.size() > 0 && loopID[0] != '#')
          loopIDs.insert(std::stoul(loopID));
      }

      loopIDWhiteListFile.close();
    } else
      llvm::errs() << "could not open file: \'" << LoopIDWhiteListFilename
                   << "\'\n";
  }

  for (const auto &e : LoopIDWhiteList)
    loopIDs.insert(e);

  LoopRuntimeProfiler::Instrumenter<
      LoopRuntimeProfiler::DefaultRuntimeCallbacksPolicy> instrumenter;

  instrumenter.instrumentProgram(CurMod);

  for (auto &CurFunc : CurMod) {
    if (CurFunc.isDeclaration())
      continue;

    auto &LI = getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc).getLoopInfo();

    workList.clear();

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
    AnnotateLoops al;

    auto loopsFilter = [&](llvm::Loop *e) {
      if (al.hasAnnotatedId(*e)) {
        auto id = al.getAnnotatedId(*e);
        if (loopIDs.count(id))
          workList.push_back(e);
      }
    };
#else
    auto loopsFilter = [&](llvm::Loop *e) { workList.push_back(e); };
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

    std::for_each(LI.begin(), LI.end(), loopsFilter);

    for (auto i = 0; i < workList.size(); ++i)
      for (auto &e : workList[i]->getSubLoops())
        workList.push_back(e);

    workList.erase(
        std::remove_if(workList.begin(), workList.end(), [](const auto *e) {
          auto d = e->getLoopDepth();
          return d < LoopDepthLB || d > LoopDepthUB;
        }), workList.end());

    std::reverse(workList.begin(), workList.end());

    for (auto *e : workList) {
#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
      auto *id = llvm::ConstantInt::get(
          llvm::IntegerType::get(
              CurMod.getContext(),
              std::numeric_limits<AnnotateLoops::LoopID_t>::digits),
          al.getAnnotatedId(*e));
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
      instrumenter.instrumentLoop(*e, id);
    }

    llvm::errs() << "instrumented " << workList.size() << " loops in function: "
                 << (CurFunc.hasName() ? CurFunc.getName() : "") << "\n";
  }

  return hasModuleChanged;
}

void LoopRuntimeProfilerPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addPreservedID(llvm::LoopSimplifyID);
  AU.addRequiredTransitiveID(llvm::LoopSimplifyID);
  AU.addRequiredTransitive<llvm::LoopInfoWrapperPass>();
  AU.addPreserved<llvm::LoopInfoWrapperPass>();

  return;
}
} // namespace icsa end
