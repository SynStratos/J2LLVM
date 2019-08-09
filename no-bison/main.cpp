#include"libraries.cpp"

static void HandleClass()
{
	if (auto ClassNode = ParseClass())
	{
		ClassNode->codegen();
	}else{
		// Skip token for error recovery.
		next_token();
	}
}

static void Drive()
{
	bool flag_class = false;
	while(true)
	{	
		switch(cur_token){
		case EOF_T		: return;
		case CLASS_T	: { if(!flag_class){
								HandleClass(); 
								flag_class=true; 
								next_token(); 
								break;
						}else LogError("only one definition class expected.");	}
		default			: LogError("expected class declaration.");
		}
	}
}

////////////////////////////////////////////////////////
//------------------------MAIN------------------------//
////////////////////////////////////////////////////////
int main()
{
	init_precedence();
	MyModule = new Module("",MyContext);
	next_token();
	
	Drive();
	MyModule->print(errs(), nullptr);
	return 0;
}