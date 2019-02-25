#include <iostream>
#include "codegen.h"
#include "node.h"
#include <fstream>


using namespace std;

extern int yyparse();
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
	cout<<"???"<<endl;
	yyparse();
	cout << programBlock << endl;

	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	//createCoreFunctions(context);
	context.generateCode(*programBlock);
	context.runCode();
	auto root = programBlock->jsonGen();
	string jsonFile = "/home/lzt/all_them/vis/A_tree.json";
    std::ofstream astJson(jsonFile);
    if( astJson.is_open() ){
        astJson << root;
        astJson.close();
        cout << "json write to " << jsonFile << endl;
    }
	return 0;
}

