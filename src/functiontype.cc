#include "node-llvm.h"

static Handle<Value> functionTypeConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR_POINTER(pFunctionType, 2);
	setConst(args.This(), "context", args[1]);
	return scope.Close(args.This());
}

static Handle<Value> getNumParams(const Arguments& args){
	ENTER_METHOD(pFunctionType, 0);
	return scope.Close(Integer::New(self->getNumParams()));
}

static Handle<Value> getParamType(const Arguments& args){
	ENTER_METHOD(pFunctionType, 1);
	INT_ARG(idx, 0);
	llvm::Type* type = self->getParamType(idx);
	return scope.Close(pType.create(type));
}

static void init(Handle<Object> target){
	pFunctionType.init(&functionTypeConstructor);
	pFunctionType.inherit(pType);
    pFunctionType.addMethod("getNumParams", &getNumParams);
    pFunctionType.addMethod("getParamType", &getParamType);

}

Proto<llvm::FunctionType> pFunctionType("FunctionType", &init);
