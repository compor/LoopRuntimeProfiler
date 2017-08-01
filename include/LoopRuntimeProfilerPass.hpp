//
//
//

#ifndef LOOPRUNTIMEPROFILERPASS_HPP
#define LOOPRUNTIMEPROFILERPASS_HPP

#include "llvm/Pass.h"
// using llvm::ModulePass

namespace llvm {
class Module;
} // namespace llvm end

namespace icsa {

class LoopRuntimeProfilerPass : public llvm::ModulePass {
public:
  static char ID;

  LoopRuntimeProfilerPass() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &CurMod) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
};

} // namespace icsa end

#endif // LOOPRUNTIMEPROFILERPASS_HPP
