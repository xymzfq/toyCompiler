#include <stack>
#include <vector>
#include <typeinfo>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Bitcode/BitstreamReader.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/raw_ostream.h>

#include <json/json.h>

using namespace llvm;

using std::pair;
using std::string;

static std::map<std::string, llvm::StructType *> structType_map;
static std::map<std::string,std::vector<pair<std::string,std::string > > > structMembers_map;
		
class NBlock;

static LLVMContext MyContext;

class CodeGenBlock {
public:
    BasicBlock *block;
    Value *returnValue;
    std::map<std::string, Value*> locals;
};

class CodeGenContext {
    std::stack<CodeGenBlock *> blocks;
    Function *mainFunction;

public:

    Module *module;
    CodeGenContext() { module = new Module("main", MyContext); }
    
    void generateCode(NBlock& root);
    GenericValue runCode();
    std::map<std::string, Value*>& locals() { return blocks.top()->locals; }
    BasicBlock *currentBlock() { return blocks.top()->block; }
    void pushBlock(BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->returnValue = NULL; blocks.top()->block = block; }
    void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
    void setCurrentReturnValue(Value *value) { blocks.top()->returnValue = value; }
    Value* getCurrentReturnValue() { return blocks.top()->returnValue; }
    
    Value* getSymbolValue(string name) 
    {
        std::stack<CodeGenBlock *> tmp;
        while(!blocks.empty())
        {
        	auto nBlock = blocks.top(); blocks.pop();
        	tmp.push(nBlock);
					if((*nBlock).locals.find(name) != (*nBlock).locals.end())
					{
						while(!tmp.empty())
						{
							blocks.push(tmp.top());
							tmp.pop();
						}
						std::cout<<"find here! !   "<<(*nBlock).locals[name]<<std::endl;
					  return (*nBlock).locals[name];
					}
        }
        while(!tmp.empty())
				{
					blocks.push(tmp.top());
					tmp.pop();
				}
        return nullptr;
    }
    
    void addStructType(std::string name,llvm::StructType *type) 
    { 
    	structType_map[name] = type;
    	structMembers_map[name] = std::vector<pair<std::string,std::string > >(); 
    }
    void addStructMember(string structName, string memType, string memName)
    {
    	if( structType_map.find(structName) == structType_map.end() )
        printf("Unknown struct name\n");

    	structMembers_map[structName].push_back(std::make_pair(memType, memName));
		}
		long getStructMemberIndex(string structName, string memberName) {
    	if( structType_map.find(structName) == structType_map.end() ){
       	 printf("Unknown struct name\n");
       	 return 0;
  	  }
   	 auto& members = structMembers_map[structName];
   	 
   	 for(auto it=members.begin(); it!=members.end(); it++)
   	     if( it->second == memberName )
     	       return std::distance(members.begin(), it);

    	printf("Unknown struct member\n");

    	return 0;
		}
};
