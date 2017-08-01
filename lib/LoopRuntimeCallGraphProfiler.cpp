
#include "LoopRuntimeCallGraphProfiler.hpp"

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo
// using llvm::Loop

#include "llvm/IR/Dominators.h"
// using llvm::DominatorTree

#include "llvm/Support/Debug.h"
// using llvm::dbgs()

namespace icsa {
namespace LoopRuntimeProfiler {

void LoopRuntimeCallGraphProfiler::getLoops() {
  populateSCCs();

  for (const auto &e : m_LoopMap)
    for (const auto &SCCNode : e.first) {
      auto *func = SCCNode->getFunction();

      if (func && func->hasName())
        llvm::dbgs() << func->getName() << "\n";
    }

  llvm::DominatorTree DT;

  for (const auto &e : m_LoopMap)
    for (const auto &SCCNode : e.first) {
      auto *func = SCCNode->getFunction();

      if (func && !func->isDeclaration()) {
        DT.recalculate(*func);

        llvm::LoopInfo LI;
        LI.Analyze(DT);

        for (const auto &l : LI)
          llvm::dbgs() << *l << "\n";
      }
    }

  return;
}

void LoopRuntimeCallGraphProfiler::populateSCCs() {
  auto SCCIter = llvm::scc_begin(&m_CG);
  auto SCCIterEnd = llvm::scc_end(&m_CG);

  for (; SCCIter != SCCIterEnd; ++SCCIter) {
    auto &CurSCC = *SCCIter;
    std::set<llvm::Loop *> Loops;

    m_LoopMap.emplace(CurSCC, Loops);
  }

  return;
}

} // namespace LoopRuntimeProfiler end
} // namespace icsa end
