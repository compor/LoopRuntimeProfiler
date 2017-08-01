
#ifndef LOOPRUNTIMECALLGRAPHPROFILER_HPP
#define LOOPRUNTIMECALLGRAPHPROFILER_HPP

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/Analysis/LoopInfo.h"
// using llvm::LoopInfo

#include "llvm/ADT/SmallPtrSet.h"
// using llvm::SmallPtrSetImpl

#include "llvm/ADT/SCCIterator.h"
// using llvm::scc_iterator

#include "llvm/Support/Debug.h"
// using llvm::dbgs()

#include <map>
// using std::map

namespace icsa {
namespace LoopRuntimeProfiler {

class LoopRuntimeCallGraphProfiler {
  using SCCTy = llvm::scc_iterator<llvm::CallGraph *>::value_type;

public:
  using LoopMapTy = std::map<SCCTy, llvm::SmallPtrSetImpl<llvm::Loop>>;

  LoopRuntimeCallGraphProfiler(llvm::CallGraph &CG) : m_CG(CG) {}

  // void getLoops(LoopMapTy &LoopMap) {
  void getLoops() {
    auto SCCIter = llvm::scc_begin(&m_CG);
    auto SCCIterEnd = llvm::scc_end(&m_CG);

    for (; SCCIter != SCCIterEnd; ++SCCIter) {
      auto &CurSCC = *SCCIter;

      for (auto &SCCNode : CurSCC) {
        auto *func = SCCNode->getFunction();

        if (func && func->hasName())
          llvm::dbgs() << func->getName() << "\n";
      }
      llvm::dbgs() << "---\n";
    }

    return;
  }

private:
  llvm::CallGraph &m_CG;
};

} // namespace LoopRuntimeProfiler end
} // namespace icsa end

#endif // LOOPRUNTIMECALLGRAPHPROFILER_HPP
