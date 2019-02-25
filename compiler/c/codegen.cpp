#include "node.h"
#include "codegen.h"
#include "parser.hpp"

using namespace std;

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static Function* mainFunc;

/* Compile the AST into a module */

void CodeGenContext::generateCode(NBlock& root)
{
	std::cout << "Generating code...\n";

	/* Create the top level interpreter function to call as entry */

	root.codeGen(*this); /* emit bytecode for the toplevel block */
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
	if (context.locals().find(lhs.name) == context.locals().end()) {
		std::cerr << "undeclared variable " << lhs.name << endl;
		return NULL;
	}
	Value* dst =  context.locals()[lhs.name];
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

	block.codeGen(context);
	if( context.getCurrentReturnValue() ){
            Builder.CreateRet(context.getCurrentReturnValue());
        } else{
            cout<<"not found\n"<<endl;
        }

	context.popBlock();
	std::cout << "Creating function: " << id.name << endl;
	function->viewCFG();
	return function;
}

llvm::Value* NIfStatement::codeGen(CodeGenContext &context) {
    cout << "Generating if statement" << endl;


	Value *CondV = Cond->codeGen(context);

  if (!CondV)
    return nullptr;
	printf("%d\n",CondV);
  // Convert condition to a bool by comparing non-equal to 0.0.

  CondV = Builder.CreateIntCast(CondV, Type::getInt1Ty(TheContext), true);
  CondV = Builder.CreateICmpNE(CondV, ConstantInt::get(Type::getInt1Ty(TheContext), 0, true));
	printf("her1\n\n");
  Function *theFunction = Builder.GetInsertBlock()->getParent();
	printf("her0\n\n");
  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  BasicBlock *thenBB = BasicBlock::Create(MyContext, "then", theFunction);
  BasicBlock *elseBB = BasicBlock::Create(MyContext, "else");
  BasicBlock *mergeBB = BasicBlock::Create(MyContext, "ifcont");
	if( this->Else ){
        Builder.CreateCondBr(CondV, thenBB, elseBB);
    } else{
        Builder.CreateCondBr(CondV, thenBB, mergeBB);
    }

    Builder.SetInsertPoint(thenBB);


		context.pushBlock(thenBB);
    this->Then->codeGen(context);
    context.popBlock();



    thenBB = Builder.GetInsertBlock();

    if( thenBB->getTerminator() == nullptr ){      
        Builder.CreateBr(mergeBB);
    }

    if( this->Else ){
        theFunction->getBasicBlockList().push_back(elseBB);    
        Builder.SetInsertPoint(elseBB);            
				context.pushBlock(elseBB);


        this->Else->codeGen(context);

        context.popBlock();


        Builder.CreateBr(mergeBB);
    }

    theFunction->getBasicBlockList().push_back(mergeBB);        
    Builder.SetInsertPoint(mergeBB);        
		printf("overhere\n");
    return nullptr;
}
llvm::Value* NForStatement::codeGen(CodeGenContext &context) {

    Function* theFunction = Builder.GetInsertBlock()->getParent();

    BasicBlock *block = BasicBlock::Create(MyContext, "forloop", theFunction);
    BasicBlock *after = BasicBlock::Create(MyContext, "forcont");

    // execute the initial
    if( this->initial )
        this->initial->codeGen(context);

    Value* condValue = this->condition->codeGen(context);
    if( !condValue )
        return nullptr;

    condValue = Builder.CreateIntCast(condValue, Type::getInt1Ty(TheContext), true);
  	condValue = Builder.CreateICmpNE(condValue, ConstantInt::get(Type::getInt1Ty(TheContext), 0, true));

    // fall to the block
    Builder.CreateCondBr(condValue, block, after);

    Builder.SetInsertPoint(block);



    this->block->codeGen(context);



    // do increment
    if( this->increment ){
        this->increment->codeGen(context);
    }

    // execute the again or stop
    condValue = this->condition->codeGen(context);
    condValue = Builder.CreateIntCast(condValue, Type::getInt1Ty(TheContext), true);
  	condValue = Builder.CreateICmpNE(condValue, ConstantInt::get(Type::getInt1Ty(TheContext), 0, true));
    Builder.CreateCondBr(condValue, block, after);

    // insert the after block
    theFunction->getBasicBlockList().push_back(after);
    Builder.SetInsertPoint(after);

    return nullptr;
}
llvm::Value *NLiteral::codeGen(CodeGenContext &context) {
    return Builder.CreateGlobalString(this->value, "string");
}

llvm::Value* NStructAssignment::codeGen(CodeGenContext &context) {
    cout << "Generating struct assignment of " << this->structMember->id->name << "." << this->structMember->member->name << endl;
    auto varPtr = context.getSymbolValue(this->structMember->id->name);
    auto structPtr = Builder.CreateLoad(varPtr, "structPtr");
//    auto underlyingStruct = Builder.CreateLoad(load);
    structPtr->setAlignment(4);

    if( !structPtr->getType()->isStructTy() ){
        return cout<<"wrong here\n\n"<<endl,nullptr;
    }

    string structName = structPtr->getType()->getStructName().str();
    cout<<endl<<structName<<endl<<endl;
    long memberIndex = context.getStructMemberIndex(structName, this->structMember->member->name);

    std::vector<Value*> indices;
    auto value = this->expression->codeGen(context);
    
    indices.push_back(ConstantInt::get(Type::getInt32Ty(MyContext), 0, false));
    indices.push_back(ConstantInt::get(Type::getInt32Ty(MyContext), (uint64_t)memberIndex, false));

    auto ptr = Builder.CreateInBoundsGEP(varPtr, indices, "structMemberPtr");

    return Builder.CreateStore(value, ptr);
}

llvm::Value* NStructDeclaration::codeGen(CodeGenContext& context) {
    cout << "Generating struct declaration of " << this->name->name << endl;

    std::vector<Type*> memberTypes;

//    Builder.createstr
    auto structType = StructType::create(MyContext, this->name->name);
    context.addStructType(this->name->name, structType);

    for(auto& member: *this->members){
        context.addStructMember(this->name->name, member->type.name, member->id.name);
        memberTypes.push_back(typeOf(member->type.name));
    }

    structType->setBody(memberTypes);

    return nullptr;
}

llvm::Value *NStructMember::codeGen(CodeGenContext &context) {
    cout << "Generating struct member expression of " << this->id->name << "." << this->member->name << endl;

    auto varPtr = context.getSymbolValue(this->id->name);
    auto structPtr = Builder.CreateLoad(varPtr, "structPtr");
    structPtr->setAlignment(4);
		if( !structPtr->getType()->isStructTy() ){
        return printf("The variable is not struct"),nullptr;
    }
    string structName = structPtr->getType()->getStructName().str();
    long memberIndex = context.getStructMemberIndex(structName, this->member->name);

    std::vector<Value*> indices;
    indices.push_back(ConstantInt::get(Type::getInt32Ty(MyContext), 0, false));
    indices.push_back(ConstantInt::get(Type::getInt32Ty(MyContext), (uint64_t)memberIndex, false));
    auto ptr = Builder.CreateInBoundsGEP(varPtr, indices, "memberPtr");

    return Builder.CreateLoad(ptr);
}


