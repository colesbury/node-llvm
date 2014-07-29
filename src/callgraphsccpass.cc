#include "node-llvm.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/InitializePasses.h"

using llvm::AnalysisUsage;
using llvm::CallGraphSCCPass;
using llvm::CallGraphSCC;
using llvm::CallGraph;
using llvm::CallGraphNode;
using llvm::RegisterPass;
using llvm::PassRegistry;
using llvm::PassInfo;

extern Proto<llvm::CallGraphSCC>  pCallGraphSCC;
extern Proto<llvm::CallGraphNode>  pCallGraphNode;

class JSCallGraphSCCPass : public CallGraphSCCPass {
  public:
	static char ID;
	JSCallGraphSCCPass(Persistent<Object> handle) : CallGraphSCCPass(ID), object_(handle) {
		llvm::initializeCallGraphPass(*PassRegistry::getPassRegistry());
	}
	bool doInitialization(CallGraph &CG) override;
	bool runOnSCC(CallGraphSCC &SCC) override;
  private:
	Persistent<Object> object_;
};

char JSCallGraphSCCPass::ID = 0;
bool JSCallGraphSCCPass::doInitialization(CallGraph &CG)
{
	return true;
}

bool JSCallGraphSCCPass::runOnSCC(CallGraphSCC &SCC)
{
	HandleScope handle_scope;

	Handle<Value> scc = pCallGraphSCC.create(&SCC);
	Handle<Value> run = object_->Get(String::New("run"));
	if (run->IsObject() && Handle<Object>::Cast(run)->IsCallable()) {
		auto fn = Handle<Object>::Cast(run);
		fn->CallAsFunction(object_, 1, &scc);
	}

	return false;
}

static Handle<Value> callGraphSCCPassConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(0);

	auto pass = new JSCallGraphSCCPass(Persistent<Object>::New(args.This()));
	pCallGraphSCCPass.wrap(args.This(), pass);

	return scope.Close(args.This());
}

static Handle<Value> callGraphSCCConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR_POINTER(pCallGraphSCC, 2);
	return scope.Close(args.This());
}

static Handle<Value> getNodes(const Arguments& args){
	ENTER_METHOD(pCallGraphSCC, 0);

	Handle<Array> array = Array::New(self->size());
	unsigned idx = 0;
	for (CallGraphNode* node : *self) {
		array->Set(idx, pCallGraphNode.create(node));
		++idx;
	}

	return scope.Close(array);
}

static Handle<Value> callGraphNodeConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR_POINTER(pCallGraphNode, 2);
	return scope.Close(args.This());
}

static Handle<Value> getFunction(const Arguments& args){
	ENTER_METHOD(pCallGraphNode, 0);
	auto fn = self->getFunction();
	if (!fn) {
		return scope.Close(Undefined());
	}
	auto module = fn->getParent();
	return scope.Close(pFunction.create(fn, pModule.create(module)));
}

static Handle<Value> getCalledNodes(const Arguments& args){
	ENTER_METHOD(pCallGraphNode, 0);
	Handle<Array> array = Array::New(self->size());
	for (unsigned idx = 0; idx < self->size(); ++idx) {
		CallGraphNode* node = (*self)[idx];
		array->Set(idx, pCallGraphNode.create(node));
	}

	return scope.Close(array);
}

static void init(Handle<Object> target){
	pCallGraphSCCPass.init(&callGraphSCCPassConstructor);

	pCallGraphSCC.init(&callGraphSCCConstructor);
	pCallGraphSCC.addMethod("getNodes", &getNodes);

	pCallGraphNode.init(&callGraphNodeConstructor);
	pCallGraphNode.addMethod("getFunction", &getFunction);
	pCallGraphNode.addMethod("getCalledNodes", &getCalledNodes);
}

Proto<llvm::CallGraphSCCPass> pCallGraphSCCPass("CallGraphSCCPass", init);
Proto<llvm::CallGraphSCC>     pCallGraphSCC("CallGraphSCC");
Proto<llvm::CallGraphNode>    pCallGraphNode("CallGraphNode");

// using namespace llvm;
// INITIALIZE_PASS_BEGIN(JSCallGraphSCCPass, "js-call-graph",
// 				"Sam's Call Graph Analysis", false, false)
// INITIALIZE_PASS_DEPENDENCY(CallGraph)
// INITIALIZE_PASS_END(JSCallGraphSCCPass, "js-call-graph",
// 				"Sam's Call Graph Analysis", false, false)
