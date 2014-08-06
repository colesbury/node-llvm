#include "node-llvm.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/SmallPtrSet.h"
#include <algorithm>

using llvm::AnalysisUsage;
using llvm::FunctionPass;
using llvm::RegisterPass;
using llvm::PassRegistry;
using llvm::PassInfo;
using llvm::DenseMap;
using llvm::ValueMap;
using llvm::SmallPtrSet;
using llvm::PHINode;
using llvm::BasicBlock;
using llvm::Instruction;
using llvm::CallInst;

class LivenessPass : public FunctionPass {
  public:
	static char ID;
	LivenessPass(const std::vector<llvm::Function*>& AllocatingFunctions) : FunctionPass(ID), AllocatingFunctions_(AllocatingFunctions) {
	}
	bool doInitialization(llvm::Module &)  override;
	bool runOnFunction(llvm::Function &F) override;
	bool isAllocating(llvm::Function& F);
	bool isAllocatingCall(llvm::CallInst& I);
  private:
  	std::vector<llvm::Function*> AllocatingFunctions_;
  	llvm::Type* objectPtrType_;
};

char LivenessPass::ID = 0;

struct LiveInfo {
  std::vector<Instruction*> LiveIn;
  std::vector<Instruction*> LiveOut;

  void sort() {
  	std::sort(LiveIn.begin(), LiveIn.end());
  	std::sort(LiveOut.begin(), LiveOut.end());
  }

  bool isLiveIn(Instruction* I) {
  	return std::binary_search(LiveIn.begin(), LiveIn.end(), I);
  }

  bool isLiveOut(Instruction* I) {
  	return std::binary_search(LiveOut.begin(), LiveOut.end(), I);
  }
};

class FunctionLiveness {
 public:
  FunctionLiveness(llvm::Function &F, std::unique_ptr<LiveInfo[]>& info, DenseMap<BasicBlock*, LiveInfo*>& b) : F_(&F), info_(info), blockMap(b) {}

  void Run();
  void Up_and_Mark(BasicBlock* B, Instruction* v, LiveInfo& info);

  llvm::Function* F_;
  std::unique_ptr<LiveInfo[]>& info_;
  DenseMap<BasicBlock*, LiveInfo*>& blockMap;
};


bool LivenessPass::doInitialization(llvm::Module &M)
{
	llvm::Type* objectType = M.getTypeByName("class.russ::Object");
	objectPtrType_ = objectType->getPointerTo();
	std::sort(AllocatingFunctions_.begin(), AllocatingFunctions_.end());
	return false;
}

bool LivenessPass::isAllocating(llvm::Function& F)
{
	return std::binary_search(AllocatingFunctions_.begin(), AllocatingFunctions_.end(), &F);
}

bool LivenessPass::isAllocatingCall(llvm::CallInst& I)
{
	if (auto Fn = I.getCalledFunction()) {
		return isAllocating(*Fn);
	}
	return false;
}

typedef SmallPtrSet<Instruction*, 16> InstSet;

static void findBeforeAndAfter(BasicBlock& BB, Instruction* Needle, InstSet& before, InstSet& usedAfter)
{
	bool isBefore = true;
	for (auto &I : BB) {
		if (&I == Needle) {
			isBefore = false;
			continue;
		}

		if (isBefore) {
			before.insert(&I);
		} else {
			for (unsigned OpNum = 0, NumOps = I.getNumOperands(); OpNum != NumOps; ++OpNum) {
				llvm::Value* Op = I.getOperand(OpNum);
				if (auto OpInst = llvm::dyn_cast<Instruction>(Op)) {
					usedAfter.insert(OpInst);
				}
			}
		}
	}
}

static void findLiveAcross(BasicBlock& BB, LiveInfo& info, Instruction* Needle, InstSet& before, InstSet& usedAfter, InstSet& Live)
{
	for (Instruction* Inst : info.LiveIn) {
		if (info.isLiveOut(Inst) || usedAfter.count(Inst)) {
			Live.insert(Inst);
		}
	}
	for (auto& Inst : BB) {
		if (&Inst == Needle) break;
		if (info.isLiveOut(&Inst) || usedAfter.count(&Inst)) {
			Live.insert(&Inst);
		}
	}
}

bool LivenessPass::runOnFunction(llvm::Function &F)
{
	if (!isAllocating(F)) {
		return false;
	}
	printf("Running on %s\n", F.getName().str().c_str());

	std::unique_ptr<LiveInfo[]> info;
	DenseMap<BasicBlock*, LiveInfo*> blockMap;

	FunctionLiveness liveness(F, info, blockMap);
	liveness.Run();

	InstSet* live = new InstSet();
	for (auto &BB : F) {
		for (auto &I : BB) {
			if (auto Call = llvm::dyn_cast<CallInst>(&I)) {
				if (isAllocatingCall(*Call)) {
					InstSet before;
					InstSet after;

					findBeforeAndAfter(BB, Call, before, after);

					LiveInfo& info = *blockMap[&BB];

					findLiveAcross(BB, info, Call, before, after, *live);

					Call->getType()->dump();
					printf("\n");
				}
			}
		}
	}

	for (Instruction* instr : *live) {
		if (instr->getType() == objectPtrType_) {
			// instr->dump();
		}
	}

	return false;
}

void FunctionLiveness::Run()
{
	info_.reset(new LiveInfo[F_->getBasicBlockList().size()]);
	{
		unsigned idx = 0;
		for (auto &BB : *F_) {
			blockMap[&BB] = &info_[idx++];
		}
	}

	for (auto &BB : *F_) {
		for (auto &V : BB) {
			for (auto &use : V.uses()) {
				if (auto PN = llvm::dyn_cast<PHINode>(use.getUser())) {
					BasicBlock* B = PN->getIncomingBlock(use);
					LiveInfo& info = *blockMap.lookup(B);

					auto& LiveOut = info.LiveOut;
					if (LiveOut.empty() || LiveOut.back() != &V) LiveOut.push_back(&V);

					Up_and_Mark(B, &V, info);
				} else if (auto I = llvm::dyn_cast<Instruction>(use.getUser())) {
					BasicBlock* B = I->getParent();
					LiveInfo& info = *blockMap.lookup(B);

					Up_and_Mark(B, &V, info);
				}
			}
		}
	}

	for (unsigned I = 0, E = F_->getBasicBlockList().size(); I != E; ++I) {
		info_[I].sort();
	}
}

void FunctionLiveness::Up_and_Mark(BasicBlock* B, llvm::Instruction* v, LiveInfo& info)
{
	if (v->getParent() == B) {
		if (llvm::isa<PHINode>(v))
			if (info.LiveIn.empty() || info.LiveIn.back() != v)
				info.LiveIn.push_back(v);
		return;
	}

	if (!info.LiveIn.empty() && info.LiveIn.back() == v) return;
	info.LiveIn.push_back(v);

	for (auto PI = llvm::pred_begin(B), E = llvm::pred_end(B); PI != E; ++PI) {
		LiveInfo& PredInfo = *blockMap.lookup(*PI);
		if (PredInfo.LiveOut.empty() || PredInfo.LiveOut.back() != v) PredInfo.LiveOut.push_back(v);
		Up_and_Mark(*PI, v, PredInfo);
	}
}

llvm::Pass* createLivenessPass(const std::vector<llvm::Function*>& fns);
llvm::Pass* createLivenessPass(const std::vector<llvm::Function*>& fns)
{
	return new LivenessPass(fns);
}
