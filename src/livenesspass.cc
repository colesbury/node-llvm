#include "node-llvm.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CFG.h"
#include "llvm/ADT/SmallPtrSet.h"
#include <algorithm>

using llvm::AnalysisUsage;
using llvm::FunctionPass;
using llvm::RegisterPass;
using llvm::PassRegistry;
using llvm::PassInfo;
using llvm::DenseMap;
using llvm::SmallPtrSet;
using llvm::PHINode;
using llvm::BasicBlock;
using llvm::Instruction;

class LivenessPass : public FunctionPass {
  public:
	static char ID;
	LivenessPass(const std::vector<llvm::Function*>& AllocatingFunctions) : FunctionPass(ID), AllocatingFunctions_(AllocatingFunctions) {
	}
	bool doInitialization(llvm::Module &)  override;
	bool runOnFunction(llvm::Function &F) override;
	bool isAllocating(llvm::Function& F);
  private:
  	std::vector<llvm::Function*> AllocatingFunctions_;
};

char LivenessPass::ID = 0;

struct LiveInfo {
  std::vector<Instruction*> LiveIn;
  std::vector<Instruction*> LiveOut;
  SmallPtrSet<Instruction*, 8> PhiUses;
};

class FunctionLiveness {
 public:
  FunctionLiveness(llvm::Function &F) : F_(&F) {}

  void Run();
  void Up_and_Mark(BasicBlock* B, Instruction* v, LiveInfo& info);

  DenseMap<BasicBlock*, LiveInfo*> blockMap;
  llvm::Function* F_;
};


bool LivenessPass::doInitialization(llvm::Module &)
{
	std::sort(AllocatingFunctions_.begin(), AllocatingFunctions_.end());
	return false;
}

bool LivenessPass::isAllocating(llvm::Function& F)
{
	return std::binary_search(AllocatingFunctions_.begin(), AllocatingFunctions_.end(), &F);
}

bool LivenessPass::runOnFunction(llvm::Function &F)
{
	if (!isAllocating(F)) {
		return false;
	}
	printf("Running on %s\n", F.getName().str().c_str());
	F.dump();

	FunctionLiveness liveness(F);
	liveness.Run();

	return false;
}

void FunctionLiveness::Run()
{
	std::unique_ptr<LiveInfo[]> infos(new LiveInfo[F_->getBasicBlockList().size()]);
	{
		unsigned idx = 0;
		for (auto &BB : *F_) {
			blockMap[&BB] = &infos[idx++];
		}
	}

	for (auto &BB : *F_) {
		Instruction* NonPHI = BB.getFirstNonPHI();
		for (auto& I : BB) {
			if (&I == NonPHI) break;
			auto PN = llvm::cast<PHINode>(&I);
			for (unsigned i = 0, e = PN->getNumIncomingValues(); i != e; ++i) {
				BasicBlock* Pred = PN->getIncomingBlock(i);
				if (auto IV = llvm::dyn_cast<Instruction>(PN->getIncomingValue(i))) {
					blockMap[Pred]->PhiUses.insert(IV);
				}
			}
		}
	}

	for (auto &BB : *F_) {
		for (auto &V : BB) {
			for (auto it = V.use_begin(), end = V.use_end(); it != end; ++it) {
				//Check if in PHI-uses
				if (auto I = llvm::dyn_cast<Instruction>(*it)) {
					BasicBlock* B = I->getParent();
					LiveInfo& info = *blockMap.lookup(B);

					if (info.PhiUses.count(&V)) {
						auto& LiveOut = info.LiveOut;
						if (std::find(LiveOut.begin(), LiveOut.end(), &V) != LiveOut.end()) {
							LiveOut.push_back(&V);
						}
					}

					Up_and_Mark(B, &V, info);
				}
			}
		}
	}

	for (auto &BB : *F_) {
		printf("Live In:\n");
		for (Instruction* I : blockMap[&BB]->LiveIn) {
			I->dump();
		}
		printf("Live Out:\n");
		for (Instruction* I : blockMap[&BB]->LiveOut) {
			I->dump();
		}
	}
}

void FunctionLiveness::Up_and_Mark(BasicBlock* B, llvm::Instruction* v, LiveInfo& info)
{
	if (v->getParent() == B && !llvm::isa<PHINode>(v)) return;
	if (!info.LiveIn.empty() && info.LiveIn.back() == v) return;
	info.LiveIn.push_back(v);
	if (llvm::isa<PHINode>(v)) return;
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
