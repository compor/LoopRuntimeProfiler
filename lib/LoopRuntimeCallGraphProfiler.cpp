
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

LoopRuntimeCallGraphProfiler::LoopRuntimeCallGraphProfiler(llvm::CallGraph &CG)
    : m_CG(CG) {
  populateSCCs();
  populateLoopInfos();

  for (const auto &e : m_LoopMap)
    for (const auto *l : e.second)
      l->print(llvm::dbgs());

  return;
}

void LoopRuntimeCallGraphProfiler::populateSCCs() {
  auto SCCIter = llvm::scc_begin(&m_CG);
  auto SCCIterEnd = llvm::scc_end(&m_CG);

  for (; SCCIter != SCCIterEnd; ++SCCIter)
    m_SCCs.emplace_back(*SCCIter);

  return;
}

void LoopRuntimeCallGraphProfiler::populateLoopInfos() {
  for (auto &SCC : m_SCCs) {
    m_LoopInfoMap.emplace(&SCC, std::set<llvm::LoopInfo *>());
    m_LoopMap.emplace(&SCC, std::set<llvm::Loop *>());

    for (auto &SCCNode : SCC) {
      auto *CurFunc = SCCNode->getFunction();

      if (CurFunc && !CurFunc->isDeclaration()) {
        llvm::DominatorTree DT;
        DT.recalculate(*CurFunc);

        m_LoopInfos.emplace_back(llvm::LoopInfo());
        m_LoopInfos.back().Analyze(DT);
      }

      auto &loops = m_LoopMap.find(&SCC)->second;
      for (auto *CurLoop : m_LoopInfos.back())
        loops.insert(CurLoop);

      auto &loopInfos = m_LoopInfoMap.find(&SCC)->second;
      loopInfos.insert(&m_LoopInfos.back());
    }
  }

  return;
}

} // namespace LoopRuntimeProfiler end
} // namespace icsa end
