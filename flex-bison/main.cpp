#include <iostream>
#include "node.h"
#include "codegen.h"


using namespace std;
using namespace llvm;

extern int yyparse();
extern Node* goalBlock;

extern Module* MyModule;

int main ()
{
	yyparse();
	
	if (auto ClassNode = goalBlock)
	{
		ClassNode->codegen();
	}else{
		exit(0);
	}

	return 0;
}
