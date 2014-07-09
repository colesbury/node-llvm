#include "node-llvm.h"

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

static void init(Handle<Object> target){
	pBasicBlock.init(&BBConstructor);
	pBasicBlock.inherit(pValue);
    pBasicBlock.addMethod("getTerminator", &getTerminator);

	// getTerminator
}

Proto<llvm::BasicBlock> pBasicBlock("BasicBlock", &init);
