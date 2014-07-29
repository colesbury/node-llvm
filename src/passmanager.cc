#include "node-llvm.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Transforms/Scalar.h"

static Handle<Value> PMConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(0);

	auto ee = new llvm::PassManager();
	pPassManager.wrap(args.This(), ee);

	return scope.Close(args.This());
}

static Handle<Value> runModule(const Arguments& args){
	ENTER_METHOD(pPassManager, 1);
	UNWRAP_ARG(pModule, m, 0);
	bool r = self->run(*m);
	return scope.Close(Boolean::New(r));
}

static Handle<Value> add(const Arguments& args){
	ENTER_METHOD(pPassManager, 1);
	UNWRAP_ARG(pCallGraphSCCPass, pass, 0);
	self->add(pass);
	return scope.Close(Undefined());
}

llvm::Pass* createLivenessPass(const std::vector<llvm::Function*>& fns);
static Handle<Value> addLivenessPass(const Arguments& args){
	ENTER_METHOD(pPassManager, 1);
	ARRAY_UNWRAP_ARG(pFunction, llvm::Function, fns, 0);
	self->add(createLivenessPass(fns));
	return scope.Close(args.This());
}

static Handle<Value> FPMConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(1);
	UNWRAP_ARG(pModule, mod, 0);
	setConst(args.This(), "module", args[0]);

	auto ee = new llvm::FunctionPassManager(mod);
	pFunctionPassManager.wrap(args.This(), ee);

	return scope.Close(args.This());
}

static Handle<Value> runFunction(const Arguments& args){
	ENTER_METHOD(pFunctionPassManager, 1);
	UNWRAP_ARG(pFunction, fn, 0);
	bool r = self->run(*fn);
	return scope.Close(Boolean::New(r));
}

static Handle<Value> doInit(const Arguments& args){
	ENTER_METHOD(pFunctionPassManager, 0);
	self->doInitialization();
	return scope.Close(Undefined());
}

static Handle<Value> doFini(const Arguments& args){
	ENTER_METHOD(pFunctionPassManager, 0);
	self->doFinalization();
	return scope.Close(Undefined());
}

static Handle<Value> addDataLayoutPass(const Arguments& args){
	ENTER_METHOD(pFunctionPassManager, 1);
	STRING_ARG(l, 0);
	self->add(new llvm::DataLayout(l));
	return scope.Close(args.This());
}


// Function pointers (with templates) aren't generic enough here
#define PASSFN(PASSTYPE) \
static Handle<Value> add##PASSTYPE(const Arguments& args){ \
	ENTER_METHOD(pFunctionPassManager, 0); \
	self->add(llvm::create##PASSTYPE()); \
	return scope.Close(args.This()); \
}

PASSFN(BasicAliasAnalysisPass);
PASSFN(InstructionCombiningPass);
PASSFN(ReassociatePass);
PASSFN(GVNPass);

static void init(Handle<Object> target){
	pPassManager.init(&PMConstructor);
	pPassManager.addMethod("run", &runModule);
	pPassManager.addMethod("add", &add);
	pPassManager.addMethod("addLivenessPass", &addLivenessPass);

	pFunctionPassManager.init(&FPMConstructor);
	pFunctionPassManager.inherit(pPassManager);

	pFunctionPassManager.addMethod("run", &runFunction);
	pFunctionPassManager.addMethod("doInitialization", &doInit);
	pFunctionPassManager.addMethod("doFinalization", &doFini);

	pFunctionPassManager.addMethod("addTargetDataPass", &addDataLayoutPass);
	pFunctionPassManager.addMethod("addDataLayoutPass", &addDataLayoutPass); // New name in LLVM trunk
	pFunctionPassManager.addMethod("addBasicAliasAnalysisPass", &addBasicAliasAnalysisPass);
	pFunctionPassManager.addMethod("addInstructionCombiningPass", &addInstructionCombiningPass);
	pFunctionPassManager.addMethod("addReassociatePass", &addReassociatePass);
	pFunctionPassManager.addMethod("addGVNPass", &addGVNPass);

	pPassManager.addToModule(target);
	pFunctionPassManager.addToModule(target);
}

Proto<llvm::PassManager> pPassManager("PassManager", &init);
Proto<llvm::FunctionPassManager> pFunctionPassManager("FunctionPassManager");
