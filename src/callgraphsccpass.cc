#include "node-llvm.h"
#include "llvm/Analysis/Passes.h"

using llvm::AnalysisUsage;
using llvm::CallGraphSCCPass;
using llvm::CallGraphSCC;
using llvm::CallGraph;
using llvm::RegisterPass;
using llvm::PassRegistry;
using llvm::PassInfo;

class JSCallGraphSCCPass : public CallGraphSCCPass {
  public:
    static char ID;
    JSCallGraphSCCPass() : CallGraphSCCPass(ID) {}
    JSCallGraphSCCPass(Persistent<Object> handle) : CallGraphSCCPass(ID), object_(handle) {}
	bool doInitialization(CallGraph &CG) override;
	bool runOnSCC(CallGraphSCC &SCC) override;


    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
	// const char *getPassName() const override;
  private:
  	Persistent<Object> object_;
};

static Handle<Value> contextConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(0);

	auto pass = new JSCallGraphSCCPass(Persistent<Object>::New(args.This()));
	pCallGraphSCCPass.wrap(args.This(), pass);

	return scope.Close(args.This());
}

static void init(Handle<Object> target){
	pCallGraphSCCPass.init(&contextConstructor);
}

char JSCallGraphSCCPass::ID = 0;
bool JSCallGraphSCCPass::doInitialization(CallGraph &CG)
{
	fprintf(stderr, "doInitialization: %p\n", &CG);
	return true;
}
bool JSCallGraphSCCPass::runOnSCC(CallGraphSCC &SCC)
{
	fprintf(stderr, "RUNNING ON SCC: %p\n", &SCC);
	return false;
}
// const char *JSCallGraphSCCPass::getPassName() const
// {
// 	return "JSCallGraphSCCPass";
// }

Proto<llvm::CallGraphSCCPass> pCallGraphSCCPass("CallGraphSCCPass", init);

namespace llvm {
	void initializeJSCallGraphSCCPassPass(PassRegistry &r);
}

using namespace llvm;
INITIALIZE_PASS(JSCallGraphSCCPass, "test" , "JSCallGraphSCCPass", true, true);
