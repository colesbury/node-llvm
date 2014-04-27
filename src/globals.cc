#include "node-llvm.h"

static Handle<Value> setInitializer(const Arguments& args){
    ENTER_METHOD(pGlobalVariable, 1);
    UNWRAP_ARG(pConstant, InitVal, 0);
    // TODO: check that InitVal type matches global variable type
    self->setInitializer(InitVal);
    return scope.Close(Undefined());
}

static void initGlobalVariable(Handle<Object> target){
    pGlobalVariable.init(&valueConstructor);
    pGlobalVariable.inherit(pConstant);
    pGlobalVariable.addMethod("setInitializer", &setInitializer);
}

Proto<llvm::GlobalVariable> pGlobalVariable("GlobalVariable", &initGlobalVariable);
