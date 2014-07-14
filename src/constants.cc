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


#define CONSTANT_EXPR_LIST(V) \
  V(AlignOf, ARGS_1, 1, Type, _, _, _) \
  V(SizeOf, ARGS_1, 1, Type, _, _, _) \
  V(FNeg, ARGS_1, 1, Constant, _, _, _) \
  V(Not, ARGS_1, 1, Constant, _, _, _) \
  V(NSWNeg, ARGS_1, 1, Constant, _, _, _) \
  V(NUWNeg, ARGS_1, 1, Constant, _, _, _) \
  V(FAdd, ARGS_2, 2, Constant, Constant, _, _) \
  V(FSub, ARGS_2, 2, Constant, Constant, _, _) \
  V(FMul, ARGS_2, 2, Constant, Constant, _, _) \
  V(FDiv, ARGS_2, 2, Constant, Constant, _, _) \
  V(URem, ARGS_2, 2, Constant, Constant, _, _) \
  V(SRem, ARGS_2, 2, Constant, Constant, _, _) \
  V(FRem, ARGS_2, 2, Constant, Constant, _, _) \
  V(And, ARGS_2, 2, Constant, Constant, _, _)  \
  V(Or, ARGS_2, 2, Constant, Constant, _, _)   \
  V(Xor, ARGS_2, 2, Constant, Constant, _, _)  \
  V(NSWAdd, ARGS_2, 2, Constant, Constant, _, _) \
  V(NUWAdd, ARGS_2, 2, Constant, Constant, _, _) \
  V(NSWSub, ARGS_2, 2, Constant, Constant, _, _) \
  V(NUWSub, ARGS_2, 2, Constant, Constant, _, _) \
  V(NSWMul, ARGS_2, 2, Constant, Constant, _, _) \
  V(NUWMul, ARGS_2, 2, Constant, Constant, _, _) \
  V(NSWShl, ARGS_2, 2, Constant, Constant, _, _) \
  V(NUWShl, ARGS_2, 2, Constant, Constant, _, _) \
  V(ExactSDiv, ARGS_2, 2, Constant, Constant, _, _) \
  V(ExactUDiv, ARGS_2, 2, Constant, Constant, _, _) \
  V(ExactAShr, ARGS_2, 2, Constant, Constant, _, _) \
  V(ExactLShr, ARGS_2, 2, Constant, Constant, _, _) \
  V(ExtractElement, ARGS_2, 2, Constant, Constant, _, _) \
  V(Trunc, ARGS_2, 2, Constant, Type, _, _) \
  V(SExt, ARGS_2, 2, Constant, Type, _, _) \
  V(ZExt, ARGS_2, 2, Constant, Type, _, _) \
  V(FPTrunc, ARGS_2, 2, Constant, Type, _, _) \
  V(FPExtend, ARGS_2, 2, Constant, Type, _, _) \
  V(UIToFP, ARGS_2, 2, Constant, Type, _, _) \
  V(SIToFP, ARGS_2, 2, Constant, Type, _, _) \
  V(FPToUI, ARGS_2, 2, Constant, Type, _, _) \
  V(FPToSI, ARGS_2, 2, Constant, Type, _, _) \
  V(PtrToInt, ARGS_2, 2, Constant, Type, _, _) \
  V(IntToPtr, ARGS_2, 2, Constant, Type, _, _) \
  V(BitCast, ARGS_2, 2, Constant, Type, _, _) \
  V(AddrSpaceCast, ARGS_2, 2, Constant, Type, _, _) \
  V(ZExtOrBitCast, ARGS_2, 2, Constant, Type, _, _) \
  V(SExtOrBitCast, ARGS_2, 2, Constant, Type, _, _) \
  V(TruncOrBitCast, ARGS_2, 2, Constant, Type, _, _) \
  V(PointerCast, ARGS_2, 2, Constant, Type, _, _) \
  V(PointerBitCastOrAddrSpaceCast, ARGS_2, 2, Constant, Type, _, _) \
  V(FPCast, ARGS_2, 2, Constant, Type, _, _) \
  V(OffsetOf, ARGS_2, 2, Type, Constant, _, _) \
  V(UDiv, ARGS_3, 2, Constant, Constant, bool, _) \
  V(SDiv, ARGS_3, 2, Constant, Constant, bool, _) \
  V(LShr, ARGS_3, 2, Constant, Constant, bool, _) \
  V(AShr, ARGS_3, 2, Constant, Constant, bool, _) \
  V(Neg, ARGS_3, 1, Constant, bool, bool, _) \
  V(Select, ARGS_3, 3, Constant, Constant, Constant, _) \
  V(IntegerCast, ARGS_3, 2, Constant, Type, bool, _) \
  V(InsertElement, ARGS_3, 3, Constant, Constant, Constant, _) \
  V(ShuffleVector, ARGS_3, 3, Constant, Constant, Constant, _) \
  V(GetElementPtr, ARGS_3, 2, Constant, ArrayRef, bool, _) \
  V(InBoundsGetElementPtr, ARGS_2, 2, Constant, ArrayRef, _, _) \
  V(Add, ARGS_4, 2, Constant, Constant, bool, bool) \
  V(Sub, ARGS_4, 2, Constant, Constant, bool, bool) \
  V(Mul, ARGS_4, 2, Constant, Constant, bool, bool) \
  V(Shl, ARGS_4, 2, Constant, Constant, bool, bool) \
//  V(ExtractValue (Constant *Agg, ArrayRef< unsigned > Idxs)
//  V(InsertValue (Constant *Agg, Constant *Val, ArrayRef< unsigned > Idxs)

#define ARG_Constant(V, idx) UNWRAP_ARG(pConstant, V, idx)
#define ARG_Type(V, idx) UNWRAP_ARG(pType, V, idx)
#define ARG_bool(V, idx) BOOL_ARG(V, idx)
#define ARG_ArrayRef(V, idx) ARRAY_UNWRAP_ARG(pValue, llvm::Value, V, idx)
#define ARG__(V, idx)
#define ARGS_1 arg0
#define ARGS_2 arg0, arg1
#define ARGS_3 arg0, arg1, arg2
#define ARGS_4 arg0, arg1, arg2, arg3

#define DECLARE_CONSTANT_EXPR4(Name, NARGS, PARAMS, ARG0, ARG1, ARG2, ARG3)  \
static Handle<Value> get##Name(const Arguments& args){                       \
    ENTER_METHOD(pConstantExpr, PARAMS);                                     \
    ARG_##ARG0(arg0, 0);                                                     \
    ARG_##ARG1(arg1, 1);                                                     \
    ARG_##ARG2(arg2, 2);                                                     \
    ARG_##ARG3(arg3, 3);                                                     \
    auto ret = llvm::ConstantExpr::get##Name(NARGS);                         \
    return scope.Close(pConstant.create(ret));                               \
}
CONSTANT_EXPR_LIST(DECLARE_CONSTANT_EXPR4)
#undef DECLARE_CONSTANT_EXPR4

#undef ARG_ConstantExpr
#undef ARG_Type
#undef ARG_bool
#undef ARG_ArrayRef
#undef ARG__
#undef ARGS_1
#undef ARGS_2
#undef ARGS_3
#undef ARGS_4


static void initConstantExpr(Handle<Object> target){
    pConstantExpr.init(&valueConstructor);
    pConstantExpr.inherit(pConstant);
#define ADD_STATIC_METHOD(name, a, b, c, d, e, f) \
    pConstantExpr.addStaticMethod("get" #name, &get##name);
    CONSTANT_EXPR_LIST(ADD_STATIC_METHOD)
#undef ADD_STATIC_METHOD
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
