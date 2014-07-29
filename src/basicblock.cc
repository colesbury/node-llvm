#include "node-llvm.h"
#include "llvm/Support/CFG.h"

using llvm::BasicBlock;

static Handle<Value> BBConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(1);

	llvm::BasicBlock* bb;
	// It can accept a pointer as 2nd argument from Function constructor
	if (args.Length() > 1 && args[1]->IsExternal()){
		bb = static_cast<llvm::BasicBlock*>(Local<External>::Cast(args[1])->Value());
	}else{
		UNWRAP_ARG(pContext, ctx, 0);
		STRING_ARG(name, 1);
		bb = llvm::BasicBlock::Create(*ctx, name);
	}

	pBasicBlock.wrap(args.This(), bb);
	// setConst(args.This(), "context", args[0]);
	return scope.Close(args.This());
}

static Handle<Value> getTerminator(const Arguments& args){
    ENTER_METHOD(pBasicBlock, 0);
    return scope.Close(pValue.create(self->getTerminator()));
}

static Handle<Value> getInstructions(const Arguments& args){
    ENTER_METHOD(pBasicBlock, 0);

	llvm::BasicBlock::InstListType& list = self->getInstList();
	Handle<Array> array = Array::New(list.size());
	unsigned idx = 0;
	for (auto & I : *self) {
		array->Set(idx, pValue.create(&I, args.This()));
		++idx;
	}

    return scope.Close(array);
}

static Handle<Value> getPredecessors(Local<String> property, const AccessorInfo &info){
	ENTER_ACCESSOR(pBasicBlock);

	unsigned idx = 0;
	Handle<Array> array = Array::New(0);
	for (llvm::pred_iterator PI = llvm::pred_begin(self), E = llvm::pred_end(self); PI != E; ++PI) {
		BasicBlock *Pred = *PI;
		array->Set(idx++, pBasicBlock.create(Undefined(), External::New(Pred)));
	}

	return scope.Close(array);
}

static Handle<Value> getSuccessors(Local<String> property, const AccessorInfo &info){
	ENTER_ACCESSOR(pBasicBlock);

	unsigned idx = 0;
	Handle<Array> array = Array::New(0);
	for (llvm::succ_iterator PI = llvm::succ_begin(self), E = llvm::succ_end(self); PI != E; ++PI) {
		BasicBlock *Pred = *PI;
		array->Set(idx++, pBasicBlock.create(Undefined(), External::New(Pred)));
	}

	return scope.Close(array);
}

static void init(Handle<Object> target){
	pBasicBlock.init(&BBConstructor);
	pBasicBlock.inherit(pValue);
    pBasicBlock.addMethod("getTerminator", &getTerminator);
    pBasicBlock.addMethod("getInstructions", &getInstructions);
    pBasicBlock.addAccessor("predecessors", &getPredecessors);
    pBasicBlock.addAccessor("successors", &getSuccessors);

	// getTerminator
}

Proto<llvm::BasicBlock> pBasicBlock("BasicBlock", &init);
