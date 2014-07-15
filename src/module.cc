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
	llvm::Constant* r = self->getOrInsertFunction(name, type);
	if (auto fn = llvm::dyn_cast<llvm::Function>(r)) {
		return scope.Close(pFunction.create(fn, args.This(), args[1]));
	}
	return scope.Close(pConstant.create(r));
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

static Handle<Value> insertGlobal(const Arguments& args) {
	ENTER_METHOD(pModule, 1);

	UNWRAP_ARG(pType, type, 0);
	STRING_ARG(name, 1);
	BOOL_ARG(isConstant, 2);
	llvm::Constant* initializer = nullptr;

	auto linkage = llvm::GlobalValue::LinkageTypes::InternalLinkage;
	if (args.Length() > 3) {
		INT_ARG(linkageInt, 3);
		linkage = static_cast<llvm::GlobalValue::LinkageTypes>(linkageInt);
	}
	if (args.Length() > 4) {
		UNWRAP_ARG(pConstant, initializer_, 4);
		initializer = initializer_;
	}

	auto global = new llvm::GlobalVariable(*self, type, isConstant, linkage, initializer, name);
	return scope.Close(pGlobalVariable.create(global, args.This()));
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

static llvm::Intrinsic::ID getIntrinsicIDByName(const std::string& name)
{
	const char* Name = name.c_str();
	unsigned Len = name.length();

	using namespace llvm;

#define GET_FUNCTION_RECOGNIZER
#include "llvm/IR/Intrinsics.gen"
#undef GET_FUNCTION_RECOGNIZER

	return llvm::Intrinsic::ID::not_intrinsic;
}

static Handle<Value> getIntrinsic(const Arguments& args){
	ENTER_METHOD(pModule, 1);

	STRING_ARG(name, 0);
	llvm::ArrayRef<llvm::Type*> tys = llvm::None;
	if (args.Length() > 1) {
		ARRAY_UNWRAP_ARG(pType, llvm::Type, tys, 1);
	}

	auto id = getIntrinsicIDByName(name);
	auto fun = llvm::Intrinsic::getDeclaration(self, id, tys);
	return scope.Close(pFunction.create(fun, args.This()));
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
	pModule.addMethod("insertGlobal", &insertGlobal);

	pModule.addMethod("getTypeByName", &getTypeByName);

//	pModule.addMethod("getGlobalList")
	pModule.addMethod("getFunctionList", &getFunctionList);

	pModule.addMethod("getIntrinsic", &getIntrinsic);

	pModule.addMethod("dump", &dump);

	pModule.addMethod("writeBitcodeToFile", &writeBitcodeToFile);
}

Proto<llvm::Module> pModule("Module", &init);
