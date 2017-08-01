//
//
//

#ifndef LOOPRUNTIMECALLGRAPHPROFILER_HPP
#define LOOPRUNTIMECALLGRAPHPROFILER_HPP

#include "Config.hpp"

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
  using SCCTy = llvm::scc_iterator<const llvm::CallGraph *>::value_type;
  using LoopInfoMapTy =
      std::map<const SCCTy *, std::set<const llvm::LoopInfo *>>;
  using LoopMapTy = std::map<const SCCTy *, std::set<const llvm::Loop *>>;
  using iterator = LoopMapTy::iterator;
  using const_iterator = LoopMapTy::const_iterator;

  void populateSCCs();
  void populateLoopInfos();

  const llvm::CallGraph &m_CG;
  std::vector<SCCTy> m_SCCs;
  std::vector<llvm::LoopInfo> m_LoopInfos;
  LoopInfoMapTy m_LoopInfoMap;
  LoopMapTy m_LoopMap;

public:
  LoopRuntimeCallGraphProfiler(const llvm::CallGraph &CG);
  LoopRuntimeCallGraphProfiler(const LoopRuntimeCallGraphProfiler &) = delete;
  LoopRuntimeCallGraphProfiler(const LoopRuntimeCallGraphProfiler &&) = delete;

  inline iterator begin() { return m_LoopMap.begin(); }
  inline iterator end() { return m_LoopMap.end(); }
  inline const_iterator begin() const { return m_LoopMap.begin(); }
  inline const_iterator end() const { return m_LoopMap.end(); }
};

} // namespace LoopRuntimeProfiler end
} // namespace icsa end

#endif // LOOPRUNTIMECALLGRAPHPROFILER_HPP
