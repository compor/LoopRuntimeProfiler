
#ifndef LOOPRUNTIMECALLGRAPHPROFILER_HPP
#define LOOPRUNTIMECALLGRAPHPROFILER_HPP

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/ADT/SCCIterator.h"
// using llvm::scc_iterator

#include "llvm/Support/Debug.h"
// using llvm::dbgs()

#include <map>
// using std::map

#include <set>

namespace icsa {
namespace LoopRuntimeProfiler {

class LoopRuntimeCallGraphProfiler {
  using SCCTy = llvm::scc_iterator<llvm::CallGraph *>::value_type;

public:
  using LoopMapTy = std::map<SCCTy, std::set<llvm::Loop *>>;

  LoopRuntimeCallGraphProfiler(llvm::CallGraph &CG) : m_CG(CG) {}

  void getLoops() {
    populateSCCs();

    for (const auto &e : m_LoopMap)
      for (const auto &SCCNode : e.first) {
        auto *func = SCCNode->getFunction();

        if (func && func->hasName())
          llvm::dbgs() << func->getName() << "\n";
      }

    return;
  }

private:
  void populateSCCs() {
    auto SCCIter = llvm::scc_begin(&m_CG);
    auto SCCIterEnd = llvm::scc_end(&m_CG);

    for (; SCCIter != SCCIterEnd; ++SCCIter) {
      auto &CurSCC = *SCCIter;
      std::set<llvm::Loop *> Loops;

      m_LoopMap.emplace(CurSCC, Loops);
    }

    return;
  }

  llvm::CallGraph &m_CG;
  LoopMapTy m_LoopMap;
};

} // namespace LoopRuntimeProfiler end
} // namespace icsa end

#endif // LOOPRUNTIMECALLGRAPHPROFILER_HPP
