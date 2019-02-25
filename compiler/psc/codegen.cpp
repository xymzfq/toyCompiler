#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include <string>
using namespace std;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static Function* mainFunc;

static vector< pair <llvm::Type *,string  > > pscVec;

int curLang ;

/* Compile the AST into a module */

void CodeGenContext::generateCode(NBlock& root)
{
	std::cout << "Generating code...\n";

	/* Create the top level interpreter function to call as entry */
	BasicBlock* block = BasicBlock::Create(TheContext, "entry");
	curLang = 2;
	if(curLang == 2)
	{
		vector<Type*> argTypes;
		argTypes.push_back(Type::getInt32Ty(MyContext));
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(MyContext), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, "printNum", this->module);
	}
	pushBlock(block);
	root.codeGen(*this); 
	popBlock();
	/* emit bytecode for the toplevel block */
	/* Print the bytecode in a human-readable format
	   to see if our program compiled properly
	 */
	std::cout << "Code is generated.\n";
	//module->dump();
	
	legacy::PassManager pm;
	pm.add(createPrintModulePass(outs()));
	pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunc, noargs);
	std::cout << "Code was run.\n";
	return v;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const NIdentifier& type)
{
	std::string typeStr = type.name;
	std::cout<<"look here\n"<<typeStr<<std::endl;
	if (type.name.compare("int") == 0) {
		return Type::getInt64Ty(MyContext);
	}
	else if (type.name.compare("string") == 0) {
		return Type::getInt8PtrTy(MyContext);
	}
	if(structType_map.find(typeStr) != structType_map.end())
	{
		printf("\n\n\nritht\n\n");
		return structType_map[typeStr];
	}
	return Type::getVoidTy(MyContext);
}


/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
	std::cout << "Creating integer: " << value << endl;
	return ConstantInt::get(Type::getInt64Ty(MyContext), value, true);
}


Value* NIdentifier::codeGen(CodeGenContext& context)
{
	std::cout << "Creating identifier reference: " << name << endl;
	Value* tmpValue = context.getSymbolValue(name);
	if(tmpValue == nullptr)
	{
		std::cerr << "undeclared variable " << name << endl;
		return NULL;
	}
	return Builder.CreateLoad(tmpValue, false, "");

}

Value* NMethodCall::codeGen(CodeGenContext& context)
{

	//std::cout << "Creating method call: " << issd.name << endl;
	Function * calleeF = context.module->getFunction(this->id.name);
    if( !calleeF ){
        printf("Function name not found\n");
    }
    if( calleeF->arg_size() != this->arguments.size() ){
        printf("Function arguments size not match, calleeF=\n");
    }
    std::vector<Value*> argsv;
    for(auto it=this->arguments.begin(); it!=this->arguments.end(); it++){
        argsv.push_back((*it)->codeGen(context));
        if( !argsv.back() ){        // if any argument codegen fail
            return nullptr;
        }
    }
    std::cout << "\n\n success Creating method call: " << id.name << endl;
    return Builder.CreateCall(calleeF, argsv, "calltmp");

}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
	cout << "Generating binary operator" << endl;
	Value* L = this->lhs.codeGen(context);
    Value* R = this->rhs.codeGen(context);

    switch (this->op){
        case TPLUS:
            return Builder.CreateAdd(L, R, "addtmp");
        case TMINUS:
            return Builder.CreateSub(L, R, "subtmp");
        case TMUL:
            return Builder.CreateMul(L, R, "multmp");
        case TDIV:
            return Builder.CreateSDiv(L, R, "divtmp");
        case TCLT:
            return Builder.CreateICmpULT(L, R, "cmptmp");
        case TCLE:
            return Builder.CreateICmpSLE(L, R, "cmptmp");
        case TCGE:
            return Builder.CreateICmpSGE(L, R, "cmptmp");
        case TCGT:
            return Builder.CreateICmpSGT(L, R, "cmptmp");
        case TCEQ:
            return Builder.CreateICmpEQ(L, R, "cmptmp");
        case TCNE:
            return Builder.CreateICmpNE(L, R, "cmptmp");

    }

	return NULL;
}

Value* NAssignment::codeGen(CodeGenContext& context)
{
	std::cout << "Creating assignment for " << lhs.name << endl;

	Value* dst = context.getSymbolValue(lhs.name);
	if(dst == nullptr)
	{
		std::cerr << "undeclared variable " << lhs.name << endl;
		return NULL;
	}

	Builder.CreateStore(rhs.codeGen(context), dst);
	return dst;

}
Value* NBlock::codeGen(CodeGenContext& context)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Generating code for " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
	}
	std::cout << "Creating block" << endl;
	return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Generating return code for " << typeid(expression).name() << endl;
	Value *returnValue = expression.codeGen(context);
	context.setCurrentReturnValue(returnValue);
	return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
	std::cout << "Creating variable declaration asd" << type.name << " " << id.name << endl;
	if(curLang == 2)
	{
		pscVec.push_back({typeOf(type),id.name});
		return nullptr;
	}
	Value * alloc = Builder.CreateAlloca(typeOf(type));
	context.locals()[id.name] = alloc;
	std::cout<<id.name<<"'s value = "<<alloc<<endl;
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(context);
	}
	return alloc;
}

Value* NExternDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        argTypes.push_back(typeOf((**it).type));
    }
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
    Function *function = Function::Create(ftype, GlobalValue::ExternalLinkage, id.name.c_str(), context.module);
    return function;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
	
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back(typeOf((**it).type));
	}
	
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	cout<<"plc1"<<endl;
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	cout<<"plc2"<<endl;
	
	cout<<"kankan "<<id.name.c_str()<<"\n\n";
	BasicBlock *bblock = BasicBlock::Create(MyContext, "entry", function, 0);
	string tmp = "main";
	if(id.name.c_str()==tmp) 
	{
		mainFunc = function;
	}
	context.pushBlock(bblock);	
	Builder.SetInsertPoint(bblock);
	cout<<"p1"<<endl;
	Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);

		argumentValue = &*argsValues++;
		argumentValue->setName((*it)->id.name.c_str());
		StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
	}
	if(curLang == 2)
	{
		for(auto tmpPair : pscVec)
		{
			llvm::Type* tmpType = tmpPair.first;
			string tmpName = tmpPair.second;
			Value * alloc = Builder.CreateAlloca(tmpType);
			context.locals()[tmpName] = alloc;
			std::cout<<tmpName<<"s value = "<<alloc<<endl;
		}
	}
	block.codeGen(context);
	if( context.getCurrentReturnValue() ){
            Builder.CreateRet(context.getCurrentReturnValue());
        } else{
            Builder.CreateRet(0);
        }

	context.popBlock();
	std::cout << "Creating function: " << id.name << endl;
	return function;
}




