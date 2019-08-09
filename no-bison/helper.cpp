////////////////////////////////////////////////////////
//------------------------HELP------------------------//
////////////////////////////////////////////////////////
static void LogError(string str)
{
	fprintf(stderr, "line %d-error: %s", right_token[right_token.size()-2],str.c_str());
	exit(0);
}
////////////////////////////////////////////////////////////////////////////////////////
static void init_precedence()
{	
	OpPrecedence['='] = 1;
	OpPrecedence[OR_T] = 2;
	OpPrecedence[AND_T] = 3;
	OpPrecedence[EQUAL_T] = 4;
	OpPrecedence[NOTEQUAL_T] = 4;
	OpPrecedence[GE_T] = 5;
	OpPrecedence[LE_T] = 5;
	OpPrecedence['<'] = 5;
	OpPrecedence['>'] = 5;

	OpPrecedence['-'] = 6;
	OpPrecedence['+'] = 6;
	OpPrecedence['/'] = 7;
	OpPrecedence['*'] = 7;
}
////////////////////////////////////////////////////////////////////////////////////////
static Type* typeOf(string type) 
{
	if (type.compare("int") == 0) {
		return Type::getInt32Ty(MyContext);
	}
	else if (type.compare("double") == 0) {
		return Type::getDoubleTy(MyContext);
	}
	else if (type.compare("char") == 0) {
		return Type::getInt8Ty(MyContext);
	}
	else if (type.compare("boolean") == 0) {
		return Type::getInt1Ty(MyContext);
	}
	else if (type.compare("string") == 0) {
		return PointerType::get(IntegerType::get(MyContext, 8), 0);
	}
	return Type::getVoidTy(MyContext);
}