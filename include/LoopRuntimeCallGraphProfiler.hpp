
#ifndef LOOPRUNTIMECALLGRAPHPROFILER_HPP
#define LOOPRUNTIMECALLGRAPHPROFILER_HPP

#include "llvm/Analysis/CallGraph.h"
// using llvm::CallGraph

#include "llvm/ADT/SCCIterator.h"
// using llvm::scc_iterator
// using llvm::scc_begin
// using llvm::scc_end

#include <map>
// using std::map

#include <set>
// using std::set

namespace llvm {
class LoopInfo;
class Loop;
} // namespace llvm end

namespace icsa {
namespace LoopRuntimeProfiler {

class LoopRuntimeCallGraphProfiler {
  using SCCTy = llvm::scc_iterator<llvm::CallGraph *>::value_type;

public:
  using LoopMapTy = std::map<SCCTy, std::set<llvm::Loop *>>;

  LoopRuntimeCallGraphProfiler(llvm::CallGraph &CG) : m_CG(CG) {}

  void getLoops();

private:
  void populateSCCs();

  llvm::CallGraph &m_CG;
  LoopMapTy m_LoopMap;
};

} // namespace LoopRuntimeProfiler end
} // namespace icsa end

#endif // LOOPRUNTIMECALLGRAPHPROFILER_HPP
