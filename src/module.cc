#include "node-llvm.h"

static Handle<Value> ModuleConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR(1);

	llvm::Module* module;
	if (args[0]->IsExternal()) {
		module = static_cast<llvm::Module*>(External::Unwrap(args[0]));
		setConst(args.This(), "context", pContext.create(&module->getContext()));
	} else {
		CHECK_N_ARGS(2);
		STRING_ARG(name, 0);
		UNWRAP_ARG(pContext, context, 1);
		setConst(args.This(), "context", args[1]);
		module = new llvm::Module(name, *context);
	}

	pModule.wrap(args.This(), module);
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

static Handle<Value> getNamedGlobal(const Arguments& args){
	ENTER_METHOD(pModule, 1);
	STRING_ARG(name, 0);
	llvm::GlobalVariable* global = self->getNamedGlobal(name);
	return scope.Close(pGlobalVariable.create(global, args.This(), args[1]));
}

static Handle<Value> getOrInsertGlobal(const Arguments& args){
	ENTER_METHOD(pModule, 2);
	STRING_ARG(name, 0);
	UNWRAP_ARG(pType, type, 1);
	// TODO: should be llvm:Constant*
	auto global = static_cast<llvm::GlobalVariable*>(self->getOrInsertGlobal(name, type));
	return scope.Close(pGlobalVariable.create(global, args.This(), args[1]));
}

static Handle<Value> getTypeByName(const Arguments& args){
	ENTER_METHOD(pModule, 1);
	STRING_ARG(name, 0);
	auto type = self->getTypeByName(name);
	return scope.Close(pType.create(type));
}

static Handle<Value> getFunctionList(const Arguments& args){
	ENTER_METHOD(pModule, 0);

	llvm::Module::FunctionListType& list = self->getFunctionList();
	Handle<Array> array = Array::New(list.size());
	unsigned idx = 0;
    for (auto it = list.begin(), it_end = list.end(); it != it_end; ++it) {
    	llvm::Function* fun = it;
    	array->Set(idx, pFunction.create(fun, args.This()));
    	++idx;
    }

	return scope.Close(array);
}

static Handle<Value> dump(const Arguments& args){
	ENTER_METHOD(pModule, 0);
	std::string s;
	llvm::raw_string_ostream stream(s);
	self->print(stream, NULL);
	return scope.Close(String::New(stream.str().c_str()));
}

static Handle<Value> writeBitcodeToFile(const Arguments& args){
	ENTER_METHOD(pModule, 1);
	STRING_ARG(filename, 0);

	std::string Err;
	llvm::raw_fd_ostream out(filename.c_str(), Err);

	if (!Err.empty()) {

	}

	llvm::WriteBitcodeToFile(self, out);

	return scope.Close(Undefined());
}

static void init(Handle<Object> target){
	pModule.init(&ModuleConstructor);

	pModule.addAccessor("moduleIdentifier", &getModId);
//	pModule.addAccessor("dataLayout")
//	pModule.addAccessor("moduleIdentifier")

	pModule.addMethod("getOrInsertFunction", &getOrInsertFunction);
	pModule.addMethod("getFunction", &getFunction);

	pModule.addMethod("getNamedGlobal", &getNamedGlobal);
	pModule.addMethod("getOrInsertGlobal", &getOrInsertGlobal);

	pModule.addMethod("getTypeByName", &getTypeByName);

//	pModule.addMethod("getGlobalList")
	pModule.addMethod("getFunctionList", &getFunctionList);

	pModule.addMethod("dump", &dump);

	pModule.addMethod("writeBitcodeToFile", &writeBitcodeToFile);
}

Proto<llvm::Module> pModule("Module", &init);
