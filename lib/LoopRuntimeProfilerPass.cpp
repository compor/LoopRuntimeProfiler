//
//
//

#include "Config.hpp"

#include "Utils.hpp"

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
#include "AnnotateValues/AnnotateLoops.hpp"
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

#include "LoopRuntimeProfilerPass.hpp"

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

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfoWrapperPass
// using llvm::LoopInfo

#include "llvm/IR/LegacyPassManager.h"
// using llvm::PassManagerBase

#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// using llvm::PassManagerBuilder
// using llvm::RegisterStandardPasses

//#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
// using char llvm::LoopSimplifyID

#include "llvm/ADT/SmallVector.h"
// using llvm::SmallVector

#include "llvm/ADT/SCCIterator.h"
// using llvm::scc_iterator
// using llvm::scc_begin
// using llvm::scc_end

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

#include <set>
// using std::set

#include <algorithm>
// using std::for_each

#include <string>
// using std::string
// using std::stoul

#include <fstream>
// using std::ifstream

#include <cstdint>
// using std::uint32_t

#include <limits>
// using std::numeric_limits

#include <cassert>
// using assert

#define DEBUG_TYPE "loop-runtime-profiler"

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

static unsigned long int NumLoopsInstrumented = 0;

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
std::map<AnnotateLoops::LoopIDTy, unsigned int> LoopsToSCCs;
std::map<AnnotateLoops::LoopIDTy, std::string> LoopsToFuncNames;
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

enum struct LRPOpts {
  module,
  cgscc,
};

static llvm::cl::opt<LRPOpts> OperationMode(
    "lrp-mode", llvm::cl::desc("operation mode"), llvm::cl::Required,
    llvm::cl::values(clEnumValN(LRPOpts::module, "module", "module mode"),
                     clEnumValN(LRPOpts::cgscc, "cgscc",
                                "call graph scc mode")));

static llvm::cl::opt<bool>
    LoopHeaderInstrument("lrp-header", llvm::cl::desc("instrument loop header"),
                         llvm::cl::init(false));

static llvm::cl::opt<unsigned int>
    LoopDepthLB("lrp-loop-depth-lb",
                llvm::cl::desc("loop depth lower bound (inclusive)"),
                llvm::cl::init(1u));

static llvm::cl::opt<unsigned int>
    LoopDepthUB("lrp-loop-depth-ub",
                llvm::cl::desc("loop depth upper bound (inclusive)"),
                llvm::cl::init(std::numeric_limits<unsigned int>::max()));

static llvm::cl::opt<unsigned int> SCCStartId("lrp-scc-start-id",
                                              llvm::cl::desc("SCC start id"),
                                              llvm::cl::init(1));

static llvm::cl::opt<unsigned int>
    SCCIdInterval("lrp-scc-id-interval", llvm::cl::desc("SCC id interval"),
                  llvm::cl::init(1));

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
static llvm::cl::list<unsigned int>
    LoopIDWhiteList("lrp-loop-id",
                    llvm::cl::desc("Specify loop ids to whitelist"),
                    llvm::cl::value_desc("loop id"), llvm::cl::ZeroOrMore);

static llvm::cl::opt<std::string>
    LoopIDWhiteListFilename("lrp-loop-id-whitelist",
                            llvm::cl::desc("loop id whitelist filename"));
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

static llvm::cl::opt<std::string>
    ReportFilenamePrefix("lrp-report",
                         llvm::cl::desc("report filename porefix"));

#if LOOPRUNTIMEPROFILER_DEBUG
bool passDebugFlag = false;
static llvm::cl::opt<bool, true>
    Debug("lrp-debug", llvm::cl::desc("debug loop runtime profiler pass"),
          llvm::cl::location(passDebugFlag));
#endif // LOOPRUNTIMEPROFILER_DEBUG

namespace {

void checkCmdLineOptions(void) {
  assert(LoopDepthLB && LoopDepthUB && "Loop depth bounds cannot be zero!");

  assert(LoopDepthLB <= LoopDepthUB &&
         "Loop depth lower bound cannot be greater that upper!");

  if (LRPOpts::module == OperationMode)
    assert(!SCCIdInterval.getPosition() && !SCCStartId.getPosition() &&
           "SCC options are legal only in cgscc mode!");

  return;
}

bool readIDWhilelist(const std::string &Filename,
                     std::set<unsigned int> &LoopIDs) {
  std::ifstream loopIDWhiteListFile{LoopIDWhiteListFilename};
  auto result = false;

  if (loopIDWhiteListFile.is_open()) {
    std::string loopID;

    while (loopIDWhiteListFile >> loopID)
      if (loopID.size() > 0 && loopID[0] != '#')
        LoopIDs.insert(std::stoul(loopID));

    loopIDWhiteListFile.close();
    result = true;
  } else
    llvm::errs() << "could not open file: \'" << LoopIDWhiteListFilename
                 << "\'\n";

  return result;
}

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
template <typename T>
#endif
void report(llvm::StringRef FilenamePrefix, llvm::StringRef FilenameSuffix
#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
            ,
            const std::map<AnnotateLoops::LoopIDTy, T> &Data
#endif
) {
  std::error_code err;

  auto filename = FilenamePrefix.str() + FilenameSuffix.str() + ".txt";
  llvm::raw_fd_ostream report(filename, err, llvm::sys::fs::F_Text);

  if (err)
    llvm::errs() << "could not open file: \"" << filename
                 << "\" reason: " << err.message() << "\n";
  else {
#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
    report << Data.size() << "\n";

    for (const auto &e : Data)
      report << e.first << " " << e.second << "\n";
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
  }

  report.close();

  return;
}

} // namespace

//

bool LoopRuntimeProfilerPass::runOnModule(llvm::Module &CurMod) {
  checkCmdLineOptions();

  bool hasModuleChanged = false;
  bool useLoopIDWhitelist = false;
  bool shouldReport = !ReportFilenamePrefix.empty();
  llvm::SmallVector<llvm::Loop *, 16> workList;
  llvm::LoopInfo *LI = nullptr;
  std::set<unsigned int> loopIDs;
  std::uint32_t idNum = 1;
  unsigned int NumLoopsInElementInstrumented = 0;

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
  bool useLoopIDWhitelistFilename = !LoopIDWhiteListFilename.empty();
  bool useLoopIDWhitelistOption = LoopIDWhiteList.size();
  useLoopIDWhitelist = useLoopIDWhitelistOption || useLoopIDWhitelistFilename;

  if (useLoopIDWhitelistFilename)
    readIDWhilelist(LoopIDWhiteListFilename, loopIDs);

  if (useLoopIDWhitelistOption)
    loopIDs.insert(LoopIDWhiteList.begin(), LoopIDWhiteList.end());

  AnnotateLoops al;

  auto loopsFilter = [&](const llvm::Loop *e) {
    if (al.has(*e)) {
      auto id = al.get(*e);

      if (useLoopIDWhitelist && !loopIDs.count(id))
        return;

      workList.push_back(const_cast<llvm::Loop *>(e));
    }
  };
#else
  auto loopsFilter = [&](llvm::Loop *e) { workList.push_back(e); };
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

  auto prepareLoops = [&]() {
    std::for_each(LI->begin(), LI->end(), loopsFilter);

    for (auto i = 0; i < workList.size(); ++i)
      for (auto &e : workList[i]->getSubLoops())
        workList.push_back(e);

    workList.erase(std::remove_if(workList.begin(), workList.end(),
                                  [](const auto *e) {
                                    auto d = e->getLoopDepth();
                                    return d < LoopDepthLB || d > LoopDepthUB;
                                  }),
                   workList.end());

    std::reverse(workList.begin(), workList.end());

    return;
  };

  //

  LoopRuntimeProfiler::Instrumenter<
      LoopRuntimeProfiler::DefaultRuntimeCallbacksPolicy>
      instrumenter;

  instrumenter.instrumentProgram(CurMod);
  hasModuleChanged = true;

  if (LRPOpts::cgscc == OperationMode) {
    idNum = SCCStartId;
    auto &CG = getAnalysis<llvm::CallGraphWrapperPass>().getCallGraph();
    auto SCCIter = llvm::scc_begin(&CG);
    auto SCCIterEnd = llvm::scc_end(&CG);

    for (; SCCIter != SCCIterEnd; ++SCCIter) {
      NumLoopsInElementInstrumented = 0;
      std::string curFuncName;

      for (const auto &SCCNode : *SCCIter) {
        if (!SCCNode->getFunction() || SCCNode->getFunction()->isDeclaration())
          continue;

        auto &CurFunc = *SCCNode->getFunction();
        curFuncName = CurFunc.getName().str();
        LI = &getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc).getLoopInfo();
        workList.clear();
        prepareLoops();

        if (!workList.size())
          continue;

        DEBUG_CMD(llvm::errs()
                  << "instrumenting " << workList.size()
                  << " loops in SCC with id " << idNum
                  << " containing function " << curFuncName << "\n");

        auto *id = llvm::ConstantInt::get(
            llvm::IntegerType::get(
                CurMod.getContext(),
                std::numeric_limits<decltype(idNum)>::digits),
            idNum);

        for (auto *e : workList) {
          instrumenter.instrumentLoop(*e, id);
          if (LoopHeaderInstrument) {
            instrumenter.instrumentLoopHeader(*e, id);
          }

#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
          if (al.has(*e)) {
            auto loopID = al.get(*e);
            LoopsToSCCs.emplace(loopID, idNum);
            LoopsToFuncNames.emplace(loopID, curFuncName);
          }
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS

          NumLoopsInElementInstrumented++;
          NumLoopsInstrumented++;
          hasModuleChanged = true;
        }
      }

      // update SCC id only if there have been loops instrumented
      if (NumLoopsInElementInstrumented)
        idNum += SCCIdInterval;

      DEBUG_CMD(llvm::errs() << "Number of loops instrumented in SCC: "
                             << NumLoopsInElementInstrumented << "\n");
    }
  } else if (LRPOpts::module == OperationMode) {
    for (auto &CurFunc : CurMod) {
      if (CurFunc.isDeclaration())
        continue;

      NumLoopsInElementInstrumented = 0;
      workList.clear();
      LI = &getAnalysis<llvm::LoopInfoWrapperPass>(CurFunc).getLoopInfo();
      prepareLoops();

      if (workList.size())
        DEBUG_CMD(llvm::errs()
                  << "instrumenting " << workList.size()
                  << " loops in function " << CurFunc.getName() << "\n");

      for (auto *e : workList) {
#if LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
        if (al.has(*e)) {
          auto tmpIdNum = al.get(*e);
          LoopsToFuncNames.emplace(tmpIdNum, CurFunc.getName().str());
          auto *id = llvm::ConstantInt::get(
              llvm::IntegerType::get(
                  CurMod.getContext(),
                  std::numeric_limits<decltype(tmpIdNum)>::digits),
              tmpIdNum);

          instrumenter.instrumentLoop(*e, id);
          if (LoopHeaderInstrument) {
            instrumenter.instrumentLoopHeader(*e, id);
          }

          NumLoopsInstrumented++;
          NumLoopsInElementInstrumented++;
          hasModuleChanged = true;
        }
#else
        auto tmpIdNum = idNum++;
#endif // LOOPRUNTIMEPROFILER_USES_ANNOTATELOOPS
      }

      DEBUG_CMD(llvm::errs() << "Number of loops instrumented in function: "
                             << NumLoopsInElementInstrumented << "\n");
    }
  }

  if (shouldReport) {
    report(ReportFilenamePrefix, "-funcs", LoopsToFuncNames);

    if (LRPOpts::cgscc == OperationMode)
      report(ReportFilenamePrefix, "-sccs", LoopsToSCCs);
  }

  return hasModuleChanged;
}

void LoopRuntimeProfilerPass::getAnalysisUsage(llvm::AnalysisUsage &AU) const {
  AU.addRequiredTransitiveID(llvm::LoopSimplifyID);
  AU.addPreservedID(llvm::LoopSimplifyID);
  AU.addRequiredTransitive<llvm::LoopInfoWrapperPass>();
  AU.addPreserved<llvm::LoopInfoWrapperPass>();
  AU.addRequiredTransitive<llvm::CallGraphWrapperPass>();

  return;
}
} // namespace icsa
