#include "node-llvm.h"

static Handle<Value> getNullValue(const Arguments& args){
    ENTER_METHOD(pConstant, 1);
    UNWRAP_ARG(pType, type, 0);
    return scope.Close(pConstant.create(llvm::Constant::getNullValue(type)));
}

static void initConstant(Handle<Object> target){
    pConstant.init(&valueConstructor);
    pConstant.inherit(pValue);
    pConstant.addStaticMethod("getNullValue", &getNullValue);
}

Proto<llvm::Constant> pConstant("Constant", &initConstant);


static Handle<Value> getGetElementPtr(const Arguments& args){
    ENTER_METHOD(pConstantExpr, 2);
    UNWRAP_ARG(pConstant, C, 0);
    BOOL_ARG(InBounds, 2);

    llvm::Constant* ret;
    if (args[1]->IsArray()) {
        ARRAY_UNWRAP_ARG(pValue, llvm::Value, IdxList, 1);
        ret = llvm::ConstantExpr::getGetElementPtr(C, IdxList, InBounds);
    } else {
        UNWRAP_ARG(pConstant, Idx, 1);
        ret = llvm::ConstantExpr::getGetElementPtr(C, Idx, InBounds);
    }

    return scope.Close(pConstant.create(ret));
}

static Handle<Value> getAdd(const Arguments& args){
    ENTER_METHOD(pConstantExpr, 2);
    UNWRAP_ARG(pConstant, C1, 0);
    UNWRAP_ARG(pConstant, C2, 1);
    BOOL_ARG(HasNUW, 2);
    BOOL_ARG(HasNSW, 3);

    llvm::Constant* ret;
    ret = llvm::ConstantExpr::getAdd(C1, C2, HasNUW, HasNSW);

    return scope.Close(pConstant.create(ret));
}

static Handle<Value> getBitCast(const Arguments& args){
    ENTER_METHOD(pConstantExpr, 2);
    UNWRAP_ARG(pConstant, C, 0);
    UNWRAP_ARG(pType, Ty, 1);

    llvm::Constant* ret = llvm::ConstantExpr::getBitCast(C, Ty);

    return scope.Close(pConstant.create(ret));
}

static Handle<Value> getPointerCast(const Arguments& args){
    ENTER_METHOD(pConstantExpr, 2);
    UNWRAP_ARG(pConstant, C, 0);
    UNWRAP_ARG(pType, Ty, 1);

    llvm::Constant* ret = llvm::ConstantExpr::getPointerCast(C, Ty);

    return scope.Close(pConstant.create(ret));
}

static Handle<Value> getIntToPtr(const Arguments& args){
    ENTER_METHOD(pConstantExpr, 2);
    UNWRAP_ARG(pConstant, C, 0);
    UNWRAP_ARG(pType, Ty, 1);

    llvm::Constant* ret = llvm::ConstantExpr::getIntToPtr(C, Ty);

    return scope.Close(pConstant.create(ret));
}

static void initConstantExpr(Handle<Object> target){
    pConstantExpr.init(&valueConstructor);
    pConstantExpr.inherit(pConstant);
    pConstantExpr.addStaticMethod("getGetElementPtr", &getGetElementPtr);
    pConstantExpr.addStaticMethod("getAdd", &getAdd);
    pConstantExpr.addStaticMethod("getBitCast", &getBitCast);
    pConstantExpr.addStaticMethod("getPointerCast", &getPointerCast);
    pConstantExpr.addStaticMethod("getIntToPtr", &getIntToPtr);
}

Proto<llvm::ConstantExpr> pConstantExpr("ConstantExpr", &initConstantExpr);


static Handle<Value> getConstantDataArray(const Arguments& args){
    ENTER_METHOD(pConstantDataArray, 2);
    UNWRAP_ARG(pContext, ctx, 0);
    return Undefined();
}

static Handle<Value> getString(const Arguments& args){
    ENTER_METHOD(pConstantDataArray, 2);
    UNWRAP_ARG(pContext, ctx, 0);
    STRING_ARG(value, 1);
    BOOL_ARG(AddNull, 2);
    if (args.Length() <= 2) {
        // default is true
        AddNull = true;
    }
    auto data = llvm::ConstantDataArray::getString(*ctx, value, AddNull);
    return scope.Close(pConstant.create(data, args[0]));
}

static void initConstantDataArray(Handle<Object> target){
    pConstantDataArray.init(&valueConstructor);
    pConstantDataArray.inherit(pConstant);
    pConstantDataArray.addStaticMethod("get", &getConstantDataArray);
    pConstantDataArray.addStaticMethod("getString", &getString);
}

Proto<llvm::ConstantDataArray> pConstantDataArray("ConstantDataArray", &initConstantDataArray);
