#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>

using namespace llvm;

int main(int argc, char **argv) {
    if (argc < 2) {
        errs() << "Usage: " << argv[0] << " input.ll [output.ll]\n";
        return 1;
    }

    LLVMContext Context;
    SMDiagnostic Err;
    std::unique_ptr<Module> M = parseIRFile(argv[1], Err, Context);
    if (!M) {
        Err.print(argv[0], errs());
        return 1;
    }

    // 声明 printf 函数: int printf(const char*, ...)
    FunctionType *PrintfTy = FunctionType::get(
        IntegerType::getInt32Ty(Context),
        PointerType::get(Context, 0), // i8*
        true
    );
    FunctionCallee Printf = M->getOrInsertFunction("printf", PrintfTy);

    // 创建全局格式字符串常量: "Call function: %s\n"
    std::string FmtStr = "Call function: %s\n";
    Constant *FmtStrConst = ConstantDataArray::getString(Context, FmtStr, true);
    GlobalVariable *FmtStrVar = new GlobalVariable(
        *M, FmtStrConst->getType(), true,
        GlobalValue::PrivateLinkage, FmtStrConst, ".fmt.call"
    );
    // 转换为 i8*
    Value *FmtPtr = ConstantExpr::getPointerCast(FmtStrVar, PointerType::get(Context, 0));

    // 遍历所有函数的所有基本块，找到所有 CallBase 指令
    for (Function &F : *M) {
        if (F.isDeclaration()) continue; // 跳过外部函数
        for (BasicBlock &BB : F) {
            // 收集需要插桩的调用指令（避免在遍历中修改容器）
            std::vector<CallBase *> CallsToInstrument;
            for (Instruction &I : BB) {
                if (auto *Call = dyn_cast<CallBase>(&I)) {
                    CallsToInstrument.push_back(Call);
                }
            }
            for (CallBase *Call : CallsToInstrument) {
                // 获取被调用函数的名字（尽可能）
                std::string FuncName;
                if (Function *Callee = Call->getCalledFunction()) {
                    FuncName = Callee->getName().str();
                } else if (Value *CalleeVal = Call->getCalledOperand()) {
                    if (CalleeVal->hasName())
                        FuncName = CalleeVal->getName().str();
                    else
                        FuncName = "indirect";
                } else {
                    FuncName = "unknown";
                }

                // 为每个函数名创建全局字符串常量
                Constant *NameConst = ConstantDataArray::getString(Context, FuncName, true);
                GlobalVariable *NameVar = new GlobalVariable(
                    *M, NameConst->getType(), true,
                    GlobalValue::PrivateLinkage, NameConst, ".caller." + FuncName
                );
                Value *NamePtr = ConstantExpr::getPointerCast(NameVar, PointerType::get(Context, 0));

                // 在调用前插入 printf
                IRBuilder<> Builder(Call);
                Builder.CreateCall(Printf, {FmtPtr, NamePtr});
            }
        }
    }

    // 输出修改后的 IR
    if (argc >= 3) {
        std::error_code EC;
        raw_fd_ostream Out(argv[2], EC);
        if (EC) {
            errs() << "Cannot open output file: " << EC.message() << "\n";
            return 1;
        }
        M->print(Out, nullptr);
    } else {
        M->print(outs(), nullptr);
    }

    return 0;
}