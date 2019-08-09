///////////////////////////////////////////////////////////
//------------------------CODEGEN------------------------//
///////////////////////////////////////////////////////////
static LLVMContext MyContext;
static IRBuilder<> Builder(MyContext);
static Module* MyModule;
//mappa contenente le variabili istanziate all'interno di ogni funzione, la svuoto al termine di questa
static map<string, Value*> innerVariables;

static PointerType* pointer1; //puntatore alla struttura, utilizzato per avere la reference alla classe di riferimento all'interno dei metodi
static LoadInst* fp1; //load del puntatore classe all'interno della funzione attuale
static map<string,ConstantInt*> gvar; //variabili dichiarate nella classe, mappa costituita dal nome della variabile e dal rispettivo offset
static StructType* structty; //tipo di struttura, in questo caso il nome della classe


//basic block con le istruzioni finali per il valore di return, presente di default al termine di ogni funzione
static BasicBlock* returnBB;
//booleano per capire se ho già trovato stmt di "return" all'interno del ramo della funzione, non leggo istruzioni successive
static bool return_flag=false;
//variabile alloca in cui salvare il valore di return di ogni funzione
static AllocaInst* return_ptr;

//vettore di stringhe per tenere traccia delle variabili di tipo string istanziate, non potendo utilizzare semplicemente getType() come negli altri casi per effettuare confronti
//l'impedimento è costituito dalla lunghezza variabile del dato
static vector<string> stringhe;

//tipo della funzione istanziata per poter controllare la corrispondenza con il tipo del return stmt. svuotata chiudendo la funzione.
static string ftype_temp;
//funzione per i passaggi di default per la creazione dell'istanza della classe all'interno della funzione main()
void CreateClassRef();
//funzione per generare la variabile globale corrispondente ad una stringa
Value* StringGen(string str_value);
//creazione e istanza delle funzioni memcpy e memset
Function* createMemCpy();
Function* createMemSet();
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
Value* ClassNode::codegen()
{
	structty = MyModule->getTypeByName(name);
	if(!structty) structty = StructType::create(MyContext, name);
	
	vector<Type*> struct_fields;
	
	//memorizzo i tipi delle variabili definite all'interno della classe	
	for (unsigned it=0; it<variables.size(); it++)
	{
		if(gvar[variables[it]->name]) LogError("already defined variable.");
		if(variables[it]->type!="string"){
			struct_fields.push_back(typeOf(variables[it]->type));
			ConstantInt* ci = ConstantInt::get(MyContext, APInt(32, StringRef(to_string(it)), 10));
			gvar[variables[it]->name] = ci;
		}else{
			LogError("impossible variable definition.");
		}

	}
	
	if(structty->isOpaque()) structty->setBody(struct_fields, false);
	
	pointer1 = PointerType::get(structty, 0);
	
	for(unsigned i=0, e=content.size(); i!=e; i++) content[i]->codegen();
	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
Value* BoolNode::codegen()
{
	if(value=="true") return ConstantInt::get(Type::getInt1Ty(MyContext), 1);
	if(value=="false") return ConstantInt::get(Type::getInt1Ty(MyContext), 0);
}
////////////////////////////////////////////////////////////////////////////////////////
Value* CharNode::codegen()
{
	return ConstantInt::get(Type::getInt8Ty(MyContext), value);
}
////////////////////////////////////////////////////////////////////////////////////////
Value* IntegerNode::codegen()
{
	return ConstantInt::get(Type::getInt32Ty(MyContext), value);
} 
////////////////////////////////////////////////////////////////////////////////////////
Value* DoubleNode::codegen()
{	
	return ConstantFP::get(Type::getDoubleTy(MyContext), value);
}
////////////////////////////////////////////////////////////////////////////////////////
Value* StringNode::codegen()
{	
	Value* g_string = StringGen(value);

	Function* memcpy_t = createMemCpy();

	ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(MyContext, 8), value.length()+1);
	AllocaInst* allocatemp = Builder.CreateAlloca(ArrayTy_0, nullptr, "string_v");
	CastInst* ptr_b = new BitCastInst(allocatemp,PointerType::get(IntegerType::get(MyContext, 8), 0), "string_bitcast", Builder.GetInsertBlock());	

	vector<Value*> mem_par;
	mem_par.push_back(ptr_b);
	mem_par.push_back(g_string);
	mem_par.push_back(ConstantInt::get(MyContext, APInt(64, StringRef(to_string(value.length()+1)), 10)));
	mem_par.push_back(ConstantInt::get(MyContext, APInt(32, StringRef("1"), 10)));
	mem_par.push_back(ConstantInt::get(MyContext, APInt(1, StringRef("0"), 10)));

	Builder.CreateCall(memcpy_t,mem_par,"");
	return allocatemp;
}
////////////////////////////////////////////////////////////////////////////////////////
Value* StringGen(string str_value)
{
	//istanzio una variabile globale in cui saranno salvati i caratteri assegnati alla stringa
	ArrayType* arrayt = ArrayType::get(IntegerType::get(MyContext, 8), str_value.length()+1);
	GlobalVariable* gvar_array_str = new GlobalVariable(*MyModule, arrayt, true, GlobalValue::PrivateLinkage, 0, "str");
	
	Constant* string_co = ConstantDataArray::getString(MyContext, str_value, true);
	gvar_array_str->setInitializer(string_co);
	ConstantInt* const_int = ConstantInt::get(MyContext, APInt(32, StringRef("0"), 10));
	vector<Constant*> indices;
	indices.push_back(const_int);
	indices.push_back(const_int);
	
	return ConstantExpr::getGetElementPtr(arrayt, gvar_array_str, indices);
}
////////////////////////////////////////////////////////////////////////////////////////
Value* IdentifierNode::codegen()
{
	Value* v;
	v = innerVariables[name];
	if(!v)
	{
		//se la variabile non è definita all'interno della funzione, la cerco in quelle definite all'interno della classe
		ConstantInt* beg = gvar.begin()->second;
		ConstantInt* end = gvar[name];
		if(!end) LogError("unknown variable name.");
		GetElementPtrInst* ptr = GetElementPtrInst::Create(structty, fp1, {beg, end}, "", Builder.GetInsertBlock());
		return Builder.CreateLoad(ptr, name);
	}
	
	if(!v) LogError("unknown variable name.");
	return Builder.CreateLoad(v, name);
}
////////////////////////////////////////////////////////////////////////////////////////
Value* VariableDec::codegen()
{	
	Value* v;
	if(innerVariables[name]) LogError("already defined variable.");
	if(type=="string"){
		if(value) {
			innerVariables[name] = value->codegen();
			stringhe.push_back(name);
			return innerVariables[name];	
		}else{
			//nel caso in cui venga istanziata una variabile string senza un valore
			//salvo un'alloca con valore nullo da istanziare all'interno del vettore
			innerVariables[name] = Builder.CreateAlloca(typeOf(type), nullptr, "null_string");
			stringhe.push_back(name);
			return innerVariables[name];
		}
	}else{
		v = Builder.CreateAlloca(typeOf(type), nullptr, name);
		innerVariables[name] = v;
		if(!value) {
			return v;
		}else{
			Value* vv=value->codegen();
			if (PointerType::get(vv->getType(), 0)==v->getType()) return Builder.CreateStore(vv, v);
			else LogError("wrong type defined.");
		}	
	}
}
////////////////////////////////////////////////////////////////////////////////////////
Function* FunctionDec::codegen()
{	
	//controllo che la funzione non sia stata già dichiarata
	if(MyModule->getFunction(name)) LogError("function already defined.");

	//memorizzo il tipo dichiarato per la funzione per poter eseguire il controllo al momento dello stmt di return
	ftype_temp = type;

	vector<Type*> arguments;
	vector<DeclarationNode*>::const_iterator it;
	if(name!="main"){
		//se la funzione è diversa da main() inserisco tra gli argomenti quelli dichiarati e la reference alla classe
		arguments.push_back(pointer1);
		for (it = args.begin(); it != args.end(); it++) {
			arguments.push_back(typeOf((**it).type));
		}
	}
	
	FunctionType* ftype = FunctionType::get(typeOf(type), arguments, false);
	Function* function_x = Function::Create(ftype, Function::ExternalLinkage, name, MyModule);
	BasicBlock* BB = BasicBlock::Create(MyContext, "entry", function_x, 0);
	returnBB = BasicBlock::Create(MyContext, "return_entry", function_x); //inserisco il blocco per il return
	
	Builder.SetInsertPoint(BB);

	//alloca in cui salvare i valori su cui fare return alla fine (globale per averlo in codegen di returnstmt) 
	if(type!="void") return_ptr = Builder.CreateAlloca(typeOf(type), nullptr, ""); 

	if(name!="main"){
		//per funzioni diverse da main() inserisco i nomi degli argomenti passati, compresa la struct
		Function::arg_iterator argsValues = function_x->arg_begin();
	    Value* argumentValue;
		argumentValue = &*argsValues++;
		argumentValue->setName("this");
		Value* ptr_this = argumentValue;

		//instance alloca della struct
		AllocaInst* fp =  Builder.CreateAlloca(pointer1, nullptr, ""); 
		Builder.CreateStore(ptr_this, fp);

		//instance di load della variabile relativa alla struct
		fp1 = new LoadInst(fp, "", false, Builder.GetInsertBlock());
	
		for (it = args.begin(); it != args.end(); it++)
		{
			argumentValue = &*argsValues++;
			argumentValue->setName((*it)->name);
			
			//necessario precaricare gli argomenti all'interno della funzione, in modo analogo a quanto fatto per la classe
			AllocaInst* ainst = Builder.CreateAlloca(typeOf((*it)->type), nullptr, (*it)->name);
	  		Builder.CreateStore(argumentValue, ainst);
	  		innerVariables[(*it)->name]=ainst;
		}
	}else{ 
		//in caso la funzione sia main() utilizzo l'apposita funzione per istanziare al suo interno la classe
		if(type!="int" && type!="void") LogError("wrong function type.");
		CreateClassRef();
	}
	//////////////////////////////////////////////////////////////////

	for(unsigned i=0; i!=body.size(); i++) {
		if(!return_flag) body[i]->codegen();
	}
	
	//al termine delle istruzioni all'interno della funzione

	BranchInst::Create(returnBB, Builder.GetInsertBlock());
	Builder.SetInsertPoint(returnBB);
	if(type!="void"){
		LoadInst* ret_load = Builder.CreateLoad(return_ptr, "return_v"); //load del valore ret0 in cui ho salvato i return
		Builder.CreateRet(ret_load); //creo return finale della funzione
	}else{
		Builder.CreateRet(nullptr);
	}

	verifyFunction(*function_x);
	innerVariables.clear();
	stringhe.clear();
	ftype_temp = "";
	return_flag = false;
	return function_x;
}
////////////////////////////////////////////////////////////////////////////////////////
//operazioni da effettuare all'interno del main per creare l'istanza della classe (come se il metodo fosse esterno)
void CreateClassRef()
{
	Function* f_Znwm = MyModule->getFunction("_Znwm");
	if(!f_Znwm)
	{
		PointerType* pt = PointerType::get(IntegerType::get(MyContext, 8), 0);
		vector<Type*> pt_a;
		pt_a.push_back(IntegerType::get(MyContext,64));
		FunctionType* ft = FunctionType::get(pt, pt_a, false);
		f_Znwm = Function::Create(ft, GlobalValue::ExternalLinkage, "_Znwm", MyModule);
		f_Znwm->setCallingConv(CallingConv::C);
	}

	Function* func_llvm_memset_p0i8_i64 = createMemSet();

	AllocaInst* class_alloca = Builder.CreateAlloca(pointer1, nullptr, "class_all");
	CallInst* call_Znwm = CallInst::Create(f_Znwm, ConstantInt::get(MyContext, APInt(64, StringRef("4"), 10)), "call_Znwm", Builder.GetInsertBlock());
	CastInst* bitcast1 = new BitCastInst(call_Znwm, pointer1, "", Builder.GetInsertBlock());
	CastInst* bitcast2 = new BitCastInst(bitcast1, PointerType::get(IntegerType::get(MyContext, 8), 0), "", Builder.GetInsertBlock());
	vector<Value*> memset_par;
	memset_par.push_back(bitcast2);
	memset_par.push_back(ConstantInt::get(MyContext, APInt(8, StringRef("0"), 10)));
	memset_par.push_back(ConstantInt::get(MyContext, APInt(64, StringRef("4"), 10)));
	memset_par.push_back(ConstantInt::get(MyContext, APInt(32, StringRef("4"), 10)));
	memset_par.push_back(ConstantInt::get(MyContext, APInt(1, StringRef("0"), 10)));
	
	Builder.CreateCall(func_llvm_memset_p0i8_i64, memset_par, "");
	Builder.CreateStore(bitcast1, class_alloca);
	
	fp1 = Builder.CreateLoad(class_alloca,"class_load");
}
////////////////////////////////////////////////////////////////////////////////////////
Value* FunctionCall::codegen()
{
	Function* function = MyModule->getFunction(name);
	if(!function) LogError("unknown function referenced.");

	if(function->arg_size() != args.size()+1) LogError("incorrect arguments passed.");
	
	vector<Value*> arguments;
	
	//Agli argomenti della funzione passa sicuramente il puntatore alla classe
	arguments.push_back(fp1);
	//Poi inserisco tutti gli altri argomenti
	Argument* begin_arg = function->arg_begin()+1; //Offset di "+1" perchè il primo parametro che passo alla funzione è il riferimento alla classe
	for( unsigned i=0, e=args.size(); i!=e; i++)
	{
		//Genero codegen del parametro che passo
		Value* temp_arg = args[i]->codegen();
		//Confronto il tipo della variabile che ho passato con il tipo della variabile nella definizione di funzione
		if(begin_arg->getType() != temp_arg->getType())
		{
			//I due tipi non coincidono
			LogError("wrong type of arguments passed.");
		}

		//I due tipi coincidono -> OK
		arguments.push_back(temp_arg);
		begin_arg++;
	}
	return Builder.CreateCall(function, arguments, "");
}
////////////////////////////////////////////////////////////////////////////////////////
Value* ReturnNode::codegen()
{
	Value* v_temp = value->codegen();
	if(ftype_temp=="void") LogError("unexpected return statement in void function.");
	if(v_temp->getType() != typeOf(ftype_temp)) LogError("wrong return type.");
	return_flag = true;
	return Builder.CreateStore(v_temp, return_ptr);	
}
////////////////////////////////////////////////////////////////////////////////////////
Value* BinOperator::codegen()
{
	Value *L = lhs->codegen();
	Value *R = rhs->codegen();

	if (!L || !R)	LogError("missing operand.");

	//controllo che entrambi gli elementi di un operazione binaria siano dello stesso tipo
	//ogni tipo differente di variabile avrà gli operatori corrispondenti
	if( L->getType() == Type::getInt1Ty(MyContext) && R->getType() == Type::getInt1Ty(MyContext))
	{
		switch(op){
			case AND_T	: return Builder.CreateAnd(L, R, "");
			case OR_T	: return Builder.CreateOr(L, R, "");

			default		: LogError("unavalaible expression.");
		}
	}

	if( L->getType() == Type::getDoubleTy(MyContext) && R->getType() == Type::getDoubleTy(MyContext))
	{
		switch(op){
			case '+'	: return Builder.CreateFAdd(L, R, "addtmp");
			case '-'	: return Builder.CreateFSub(L, R, "subtmp");
			case '*'	: return Builder.CreateFMul(L, R, "multmp");
			case '/'	: return Builder.CreateFDiv(L, R, "divtmp");
			
			case '<'	: L = Builder.CreateFCmpOLT(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case '>'	: L = Builder.CreateFCmpOGT(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case LE_T	: L = Builder.CreateFCmpOLE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case GE_T 	: L = Builder.CreateFCmpOGE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case EQUAL_T	: L = Builder.CreateFCmpOEQ(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case NOTEQUAL_T	: L = Builder.CreateFCmpONE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			
			default		: LogError("unavalaible expression.");
		}	
	}

	if( L->getType() == Type::getInt32Ty(MyContext) && R->getType() == Type::getInt32Ty(MyContext))
	{

		switch(op){
			case '+'	: return Builder.CreateAdd(L, R, "addtmp");
			case '-'	: return Builder.CreateSub(L, R, "subtmp");
			case '*'	: return Builder.CreateMul(L, R, "multmp");
			case '/'	: return Builder.CreateUDiv(L, R, "divtmp");
			
			case '<'	: L = Builder.CreateICmpULT(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case '>'	: L = Builder.CreateICmpUGT(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case LE_T	: L = Builder.CreateICmpULE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case GE_T 	: L = Builder.CreateICmpUGE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case EQUAL_T	: L = Builder.CreateICmpEQ(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
			case NOTEQUAL_T	: L = Builder.CreateICmpNE(L, R, "cmptmp"); return Builder.CreateZExt(L, Type::getInt1Ty(MyContext), "booltmp");
		
			default		: LogError("unavalaible expression.");
		}
	}

	LogError("unrecognised type of operand.");
}
////////////////////////////////////////////////////////////////////////////////////////
Value* Assignment::codegen()
{	
	Value* val = value->codegen();
	if(!val) LogError("missing right operand.");
	Value* var = innerVariables[name];
	if(!var){
		//nel caso in cui non trovo la variabile tra quelle dichiarate all'interno del metodo cerco in quelle "globali"
		ConstantInt* beg = gvar.begin()->second;
		ConstantInt* end = gvar[name];
		if(!end) LogError("undefined variable.");
		
		GetElementPtrInst* ptr = GetElementPtrInst::Create(structty, fp1, {beg, end}, "", Builder.GetInsertBlock());
		if (PointerType::get(val->getType(), 0)==ptr->getType()) return Builder.CreateStore(val, ptr);
		else LogError("wrong type defined.");
	}
	if (find(stringhe.begin(), stringhe.end(), name) != stringhe.end() )	return innerVariables[name]=val;
	
	if (PointerType::get(val->getType(), 0)==var->getType())	return Builder.CreateStore(val, var);
	else LogError("wrong type defined.");
}
////////////////////////////////////////////////////////////////////////////////////////
Value* IfStmt::codegen()
{
	Value* Condition = cond->codegen();
	//verifico che la condizione passata allo stmt di IfElse sia di uno dei tipi previsti
	if(Condition->getType()!=Type::getInt32Ty(MyContext) 
		&& Condition->getType()!=Type::getInt1Ty(MyContext) 
		&& Condition->getType()!=Type::getInt8Ty(MyContext) 
		&& Condition->getType()!=Type::getDoubleTy(MyContext)) LogError("illegal type of condition.");
	Condition = Builder.CreateICmpNE(Condition, Builder.getInt1(0), "ifcond");
	
	Function* TheFunc = Builder.GetInsertBlock()-> getParent();
	
	//creo i blocchi per la diramazione dello stmt e quello in cui convergere in caso di condizione true o false
	BasicBlock* ifBB = BasicBlock::Create(MyContext, "if", TheFunc);
	BasicBlock* elseBB = BasicBlock::Create(MyContext, "else", TheFunc);
	BasicBlock* mergeBB = BasicBlock::Create(MyContext, "merge", TheFunc);
	
	BranchInst::Create(ifBB, elseBB, Condition, Builder.GetInsertBlock());
	
	Builder.SetInsertPoint(ifBB);
	for(unsigned i=0; i!=if_exec.size(); i++) if_exec[i]->codegen();
	//verifico di non aver incontrato alcun stmt di return, in tal caso non genero gli stmt successivi e procedo fino al termine del blocco
	if(return_flag){
		BranchInst::Create(returnBB, Builder.GetInsertBlock());
	}else{
		BranchInst::Create(mergeBB, Builder.GetInsertBlock());
	}
	return_flag=false;
	
	Builder.SetInsertPoint(elseBB);
	for(unsigned i=0; i!=else_exec.size(); i++) else_exec[i]->codegen();
	//verifico di non aver incontrato alcun stmt di return, in tal caso non genero gli stmt successivi e procedo fino al termine del blocco
	if(return_flag){
		BranchInst::Create(returnBB, Builder.GetInsertBlock());
	}else{
		BranchInst::Create(mergeBB, Builder.GetInsertBlock());
	}
	return_flag=false;
	
	Builder.SetInsertPoint(mergeBB);
	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
Value* WhileStmt::codegen()
{
	Function* TheFunc = Builder.GetInsertBlock()-> getParent();
	
	BasicBlock* condBB = BasicBlock::Create(MyContext, "cond", TheFunc);
	BranchInst::Create(condBB, Builder.GetInsertBlock());
	Builder.SetInsertPoint(condBB);
	
	Value* Condition = cond->codegen();
	//verifico che la condizione passata allo stmt di IfElse sia di uno dei tipi previsti
	if(Condition->getType()!=Type::getInt32Ty(MyContext) 
		&& Condition->getType()!=Type::getInt1Ty(MyContext) 
		&& Condition->getType()!=Type::getInt8Ty(MyContext) 
		&& Condition->getType()!=Type::getDoubleTy(MyContext)) LogError("illegal type of condition.");

	Condition = Builder.CreateICmpNE(Condition, Builder.getInt1(0), "whilecond");
	
	//creo i blocchi per la diramazione dello stmt e quello in cui convergere in caso di condizione true o false
	BasicBlock* whileBB = BasicBlock::Create(MyContext, "while", TheFunc);
	BasicBlock* mergeBB = BasicBlock::Create(MyContext, "merge", TheFunc);
	
	BranchInst::Create(whileBB, mergeBB, Condition, Builder.GetInsertBlock());
	
	Builder.SetInsertPoint(whileBB);
	for(unsigned i=0; i!=while_exec.size(); i++) while_exec[i]->codegen();
	//verifico di non aver incontrato alcun stmt di return, in tal caso non genero gli stmt successivi e procedo fino al termine del blocco
	if(return_flag){
		BranchInst::Create(returnBB, Builder.GetInsertBlock());
	}else{

		BranchInst::Create(condBB, Builder.GetInsertBlock());
	}
	return_flag=false;
	
	Builder.SetInsertPoint(mergeBB);
	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////
Value* PrintStmt::codegen()
{
	Value* call=nullptr;
	Value* str_call=nullptr;
	Value* id_call=nullptr;

	if(!id_v && !string_v && !all_v) LogError("illegal argument type for print function.");

	//effettuo call alla funzione printf
	vector<Type*> FuncTy_5_args;
 	FuncTy_5_args.push_back(PointerType::get(IntegerType::get(MyContext, 8), 0));
 	FunctionType* FuncTy_5 = FunctionType::get(IntegerType::get(MyContext, 32),FuncTy_5_args,true);
	Function* f_printf = MyModule->getFunction("printf");
	if(!f_printf){
		f_printf = Function::Create(FuncTy_5, GlobalValue::ExternalLinkage, "printf", MyModule);
		f_printf->setCallingConv(CallingConv::C);
		}
	////////////////////////////////////////////////////////////////////////////	
	vector<Value*> call_args;
	//nel caso il parametro passato sia un int/double/char System.out.println(2)
	if(all_v){
		call = all_v->codegen();
		if(call->getType() == Type::getInt32Ty(MyContext))
		{
			str_call = StringGen("%d");
			call_args.push_back(str_call);
		}

		if(call->getType() == Type::getDoubleTy(MyContext))
		{
			str_call = StringGen("%f");
			call_args.push_back(str_call);
		}

		if(call->getType() == Type::getInt8Ty(MyContext))
		{
			str_call = StringGen("%c");
			call_args.push_back(str_call);
		}
		call_args.push_back(call);
	}
	//nel caso il parametro passato sia una stringa System.out.println("string")
	if(string_v)
	{
		call_args.push_back(StringGen(string_v->value));
	}			
	//nel caso il parametro passato sia una variabile precedentemente dichiarata System.out.println(var)
	if(id_v)
	{
		id_call = innerVariables[id_v->name];
		if(!id_call){
			ConstantInt* beg = gvar.begin()->second;
			ConstantInt* end = gvar[id_v->name];
			if(!end) LogError("undefined variable.");
			
			GetElementPtrInst* ptr = GetElementPtrInst::Create(structty, fp1, {beg, end}, "", Builder.GetInsertBlock());
			id_call=ptr;
		}
		if(id_call->getType() == PointerType::get(IntegerType::get(MyContext, 8), 0)){
			//nel caso in cui la variabile passata sia di tipo char
			str_call = StringGen("%c");
			id_call = Builder.CreateLoad(id_call,"");
		}else if(id_call->getType() == PointerType::get(Type::getDoubleTy(MyContext), 0)){
			//nel caso in cui la variabile passata sia di tipo double
			str_call = StringGen("%f");
			id_call = Builder.CreateLoad(id_call,"");
		}else if(id_call->getType() == PointerType::get(IntegerType::get(MyContext, 32), 0)){
			//nel caso in cui la variabile passata sia di tipo int
			str_call = StringGen("%d");
			id_call = Builder.CreateLoad(id_call,"");
		}else if (find(stringhe.begin(), stringhe.end(), id_v->name) != stringhe.end()){
			//nel caso in cui la variabile passata sia di tipo string
			str_call = StringGen("%s");
		}else{
			//nel caso in cui la variabile passata sia di un tipo non previsto
			LogError("illegal argument type for print function.");
		}
	
		call_args.push_back(str_call);
		call_args.push_back(id_call);
	}

	CallInst* print_inst = CallInst::Create(f_printf, call_args, "print_inst", Builder.GetInsertBlock());
	return print_inst;
}
////////////////////////////////////////////////////////////////////////////////////////
Value* InputNode::codegen()
{
	if (find(scanner_variables.begin(), scanner_variables.end(), identifier) != scanner_variables.end()){
		Value* str_call = StringGen("%s"); //Crea LLVMIR per la variabile globale stringa "%s\00"

		ArrayType* ArrayTy_0 = ArrayType::get(IntegerType::get(MyContext, 8),64);
		AllocaInst* allocatemp = Builder.CreateAlloca(ArrayTy_0, nullptr, "str_v");
		CastInst* ptr_b = new BitCastInst(allocatemp,PointerType::get(IntegerType::get(MyContext, 8), 0), "str_bitcast", Builder.GetInsertBlock());	

		vector<Type*> FuncTy_5_args;
	 	FuncTy_5_args.push_back(PointerType::get(IntegerType::get(MyContext, 8), 0));
	 	FunctionType* FuncTy_5 = FunctionType::get(IntegerType::get(MyContext, 32),FuncTy_5_args,true);

	 	//effettuo call alla funzione scanf
		Function* f_scanf = MyModule->getFunction("scanf");
		if(!f_scanf){
			f_scanf = Function::Create(FuncTy_5, GlobalValue::ExternalLinkage, "scanf", MyModule);
			}
		vector<Value*> call_args;
		call_args.push_back(str_call);
		call_args.push_back(ptr_b);

		Builder.CreateCall(f_scanf, call_args, "scanf_inst");
		return allocatemp;
	}else{
		LogError("no Scanner previously defined.");
	}
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//---------HELPER FUNCTION FOR LLVM CODEGEN-----------//
////////////////////////////////////////////////////////
Function* createMemCpy()
{
	Function* func_llvm_memcpy_p0i8_p0i8_i64 = MyModule->getFunction("llvm.memcpy.p0i8.p0i8.i64");
 	if (!func_llvm_memcpy_p0i8_p0i8_i64) 
 	{
 		vector<Type*>ty_args;
		ty_args.push_back(PointerType::get(IntegerType::get(MyContext, 8), 0));
		ty_args.push_back(PointerType::get(IntegerType::get(MyContext, 8), 0));
		ty_args.push_back(IntegerType::get(MyContext, 64));
		ty_args.push_back(IntegerType::get(MyContext, 32));
		ty_args.push_back(IntegerType::get(MyContext, 1));
		FunctionType* f_ty = FunctionType::get(Type::getVoidTy(MyContext),ty_args,false);
 		func_llvm_memcpy_p0i8_p0i8_i64 = Function::Create(f_ty, GlobalValue::ExternalLinkage,"llvm.memcpy.p0i8.p0i8.i64", MyModule);
 	}
	return func_llvm_memcpy_p0i8_p0i8_i64;
}
////////////////////////////////////////////////////////////////////////////////////////
Function* createMemSet()
{
	Function* func_llvm_memset_p0i8_i64 = MyModule->getFunction("llvm.memset.p0i8.i64");
	if(!func_llvm_memset_p0i8_i64)
	{
		vector<Type*>memset_args;
		memset_args.push_back(PointerType::get(IntegerType::get(MyContext, 8), 0));
		memset_args.push_back(IntegerType::get(MyContext, 8));
		memset_args.push_back(IntegerType::get(MyContext, 64));
		memset_args.push_back(IntegerType::get(MyContext, 32));
		memset_args.push_back(IntegerType::get(MyContext, 1));
		FunctionType* memset_ft = FunctionType::get(Type::getVoidTy(MyContext), memset_args,false);

		func_llvm_memset_p0i8_i64 = Function::Create(memset_ft, GlobalValue::ExternalLinkage, "llvm.memset.p0i8.i64", MyModule);
		func_llvm_memset_p0i8_i64->setCallingConv(CallingConv::C);
	}
	return func_llvm_memset_p0i8_i64;
}
////////////////////////////////////////////////////////////////////////////////////////