#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <typeinfo>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/ADT/SmallVector.h"

using namespace std;
using namespace llvm;

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

//funzione per la gestione a vista degli errori
static void LogError(string str, int line)
{
	fprintf(stderr, "line: %d - error: %s \n",line, str.c_str());
	exit(0);
}

//funzione che ritorna il tipo di Value dalla stringa passata
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
