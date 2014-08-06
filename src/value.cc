#include "node-llvm.h"

Handle<Value> valueConstructor(const Arguments& args){
	ENTER_CONSTRUCTOR_POINTER(pValue, 2);
	setConst(args.This(), "parent", args[1]);
	return scope.Close(args.This());
}

Handle<Value> getValueName(Local<String> property, const AccessorInfo &info){
	ENTER_ACCESSOR(pValue);
	return scope.Close(String::New(self->getName().str().c_str()));
}

void setValueName(Local<String> property, Local<Value> value, const AccessorInfo &info){
	ENTER_ACCESSOR(pValue);
	if (value->IsString())
		self->setName(*String::Utf8Value(value->ToString()));
}

static Handle<Value> dump(const Arguments& args){
	ENTER_METHOD(pValue, 0);
	std::string s;
	llvm::raw_string_ostream stream(s);
	self->print(stream);
	return scope.Close(String::New(stream.str().c_str()));
}

static Handle<Value> getType(const Arguments& args){
	ENTER_METHOD(pValue, 0);
	auto type = self->getType();
	return scope.Close(pType.create(type));
}

static Handle<Value> getUsers(const Arguments& args){
	ENTER_METHOD(pValue, 0);

	Handle<Array> array = Array::New(self->getNumUses());
	unsigned idx = 0;
	for (auto it = self->use_begin(), end = self->use_end(); it != end; ++it) {
		array->Set(idx, pValue.create(*it, Undefined()));
		++idx;
	}

	return scope.Close(array);
}

static void init(Handle<Object> target){
	pValue.init(&valueConstructor);

	pValue.addMethod("dump", &dump);
	pValue.addMethod("getType", &getType);
	pValue.addMethod("users", &getUsers);

	pValue.addAccessor("name", &getValueName, &setValueName);
}

Proto<llvm::Value> pValue("Value", &init);
