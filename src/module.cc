#include "node-llvm.h"

static Handle<Value> ModuleConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(2);
	STRING_ARG(name, 0);
	UNWRAP_ARG(pContext, context, 1);
	setConst(args.This(), "context", args[1]);

	pModule.wrap(args.This(), new llvm::Module(name, *context));

	return scope.Close(args.This());
}

Handle<Value> getModId(Local<String> property, const AccessorInfo &info){
	ENTER_ACCESSOR(pModule);
	return scope.Close(String::New(self->getModuleIdentifier().c_str()));
}

static Handle<Value> getFunction(const Arguments& args){
	ENTER_METHOD(pModule, 1);
	STRING_ARG(name, 0);
	return scope.Close(pFunction.create(self->getFunction(name), args.This()));
}

static Handle<Value> getOrInsertFunction(const Arguments& args){
	ENTER_METHOD(pModule, 2);
	STRING_ARG(name, 0);
	UNWRAP_ARG(pFunctionType, type, 1);
	auto fn = static_cast<llvm::Function*>(self->getOrInsertFunction(name, type));
	return scope.Close(pFunction.create(fn, args.This(), args[1]));
}

static Handle<Value> getOrInsertGlobal(const Arguments& args){
	ENTER_METHOD(pModule, 2);
	STRING_ARG(name, 0);
	UNWRAP_ARG(pType, type, 1);
	// TODO: should be llvm:Constant*
	auto global = static_cast<llvm::GlobalVariable*>(self->getOrInsertGlobal(name, type));
	return scope.Close(pGlobalVariable.create(global, args.This(), args[1]));
}

static Handle<Value> dump(const Arguments& args){
	ENTER_METHOD(pModule, 0);
	std::string s;
	llvm::raw_string_ostream stream(s);
	self->print(stream, NULL);
	return scope.Close(String::New(stream.str().c_str()));
}

static void init(Handle<Object> target){
	pModule.init(&ModuleConstructor);

	pModule.addAccessor("moduleIdentifier", &getModId);
//	pModule.addAccessor("dataLayout")
//	pModule.addAccessor("moduleIdentifier")

	pModule.addMethod("getOrInsertFunction", &getOrInsertFunction);
	pModule.addMethod("getFunction", &getFunction);

//	pModule.addMethod("getNamedGlobal", &getNamedGlobal)
	pModule.addMethod("getOrInsertGlobal", &getOrInsertGlobal);

//	pModule.addMethod("getGlobalList")
//	pModule.addMethod("getFunctionList")

	pModule.addMethod("dump", &dump);
}

Proto<llvm::Module> pModule("Module", &init);
