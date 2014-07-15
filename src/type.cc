#include "node-llvm.h"

static std::unordered_map<llvm::Type*, Persistent<Value>> types;

static Handle<Value> typeConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR_POINTER(pType, 2);
	auto it = types.find(self);
	if (it != types.end()) {
		return scope.Close(it->second);
	} else {
		types[self] = Persistent<Value>::New(args.This());
	}
	setConst(args.This(), "context", args[1]);
	return scope.Close(args.This());
}

static Handle<Value> getPointerTo(const Arguments& args){
	ENTER_METHOD(pType, 0);
	// TODO: AddrSpace parameter
	auto ctx = args.This()->Get(String::NewSymbol("context"));
	return scope.Close(pType.create(self->getPointerTo(), ctx));
}

static Handle<Value> dump(const Arguments& args){
	ENTER_METHOD(pType, 0);
	std::string s;
	llvm::raw_string_ostream stream(s);
	self->print(stream);
	return scope.Close(String::New(stream.str().c_str()));
}

static Handle<Value> intConst(const Arguments& args){
	ENTER_METHOD(pIntegerType, 1);
	llvm::Constant* v;
	if (args[0]->IsString()){
		STRING_ARG(val, 0);
		int radix = 10;
		if (args.Length() > 1){
			INT_ARG(_radix, 1); radix = _radix;
		}
		v = llvm::ConstantInt::get(self, val, radix);
	}else{
		INT_ARG(val, 0);
		v = llvm::ConstantInt::getSigned(self, val);
	}

	// auto ctx = args.This()->Get(String::NewSymbol("context"));
	return scope.Close(pConstant.create(v));
}

static Handle<Value> fpConst(const Arguments& args){
	ENTER_METHOD(pFPType, 1);
	llvm::Constant* v;
	if (args[0]->IsString()){
		STRING_ARG(val, 0);
		v = llvm::ConstantFP::get(self, val);
	}else{
		DOUBLE_ARG(val, 0);
		v = llvm::ConstantFP::get(self, val);
	}
	// auto ctx = args.This()->Get(String::NewSymbol("context"));
	return scope.Close(pConstant.create(v));
}

static Handle<Value> getArrayType(const Arguments& args){
	ENTER_METHOD(pArrayType, 2);
	UNWRAP_ARG(pType, elementType, 0);
	INT_ARG(numElements, 1);
	auto type = llvm::ArrayType::get(elementType, numElements);
	return scope.Close(pArrayType.create(type));
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

static Handle<Value> getFunctionType(const Arguments& args){
	ENTER_METHOD(pFunctionType, 2);
	UNWRAP_ARG(pType, result, 0);
	ARRAY_UNWRAP_ARG(pType, llvm::Type, params, 1);
	BOOL_ARG(isVarArg, 2);
	auto type = llvm::FunctionType::get(result, params, isVarArg);
	return scope.Close(pFunctionType.create(type));
}

static void init(Handle<Object> target){
	pType.init(&typeConstructor);
	pType.addMethod("_getPointerTo", &getPointerTo);
	pType.addMethod("dump", &dump);

	pIntegerType.init(&typeConstructor);
	pIntegerType.inherit(pType);
	pIntegerType.addMethod("const", &intConst);

	pArrayType.init(&typeConstructor);
	pArrayType.inherit(pType);
	pArrayType.addStaticMethod("get", &getArrayType);

	pFPType.init(&typeConstructor);
	pFPType.inherit(pType);
	pFPType.addMethod("const", &fpConst);

	pFunctionType.init(&typeConstructor);
	pFunctionType.inherit(pType);
    pFunctionType.addMethod("getNumParams", &getNumParams);
    pFunctionType.addMethod("getParamType", &getParamType);
    pFunctionType.addStaticMethod("get", &getFunctionType);
}

Proto<llvm::Type> pType("Type", &init);
Proto<llvm::IntegerType> pIntegerType("IntegerType");
Proto<llvm::ArrayType> pArrayType("ArrayType");
// Does not exist in LLVM, but we make use it for .const
Proto<llvm::Type> pFPType("FPType");
Proto<llvm::FunctionType> pFunctionType("FunctionType");
