#include <iostream>
#include <fstream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
	yyparse();
	cout << programBlock << endl;
	auto root = programBlock->jsonGen();
    // see http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	//createCoreFunctions(context);
	context.generateCode(*programBlock);
	context.runCode();
	string jsonFile = "/home/lzt/all_them/vis/A_tree.json";
    std::ofstream astJson(jsonFile);
    if( astJson.is_open() ){
        astJson << root;
        astJson.close();
        cout << "json write to " << jsonFile << endl;
    }
	return 0;
}
