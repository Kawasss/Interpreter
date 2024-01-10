#pragma once
#include "common.hpp"
#include "StackFrame.hpp"
#include "Stack.hpp"
#include <unordered_map>

class Function;
struct AbstractSyntaxTree;

const VariableInfo leftBoolValue =       { "%lbv", DATA_TYPE_INT  };
const VariableInfo rightBoolValue =      { "%rbv", DATA_TYPE_INT  };
const VariableInfo floatStorageVar =     { "%fsv", DATA_TYPE_VOID };
const VariableInfo floatCalculationVar = { "%fcv", DATA_TYPE_VOID };
const VariableInfo floatReturnVar =      { "%frv", DATA_TYPE_VOID };
const VariableInfo bufferParametersVar = { "%bpv", DATA_TYPE_VOID };

enum InterpreterFlags
{
	OBJECT_FLAG_VERBOSE = 1 << 0,
};
InterpreterFlags operator|(const InterpreterFlags lvalue, const InterpreterFlags rvalue);
InterpreterFlags operator&(const InterpreterFlags lvalue, const InterpreterFlags rvalue);

class Interpreter
{
public:
	static void Init();
	static void SetAST(AbstractSyntaxTree& ast);

	static bool ExecuteInstructions(const std::vector<Instruction> instructions); // copying isnt the best move
	static Variable* FindVariable(std::string name);
	static Variable* FindVariable(VariableInfo& info);
	static Variable  GetValue(VariableInfo& info);
	static void DeclareVariable(const VariableInfo& info);
	static void DeclareBuffer(std::string name);

	static void CopyLocalVariableToStackFrame(std::string sourceName, std::string newName, StackFrame* destination);

	template<typename T> static void SetExternFunction(std::string name)
	{
		Function* oldFunc = functions[name];
		Function* newFunc = new T(oldFunc);
		functions[name] = newFunc;
		//delete oldFunc; // cant delete cause assert fails?
	}

	template<typename T> static T FindVariable(VariableInfo& info)
	{
		return (T)*FindVariable(info.name);
	}

private:
	static void DeclareCacheVariable(const VariableInfo& info);

	static Scope cacheVariables;

	static std::unordered_map<std::string, Function*> functions;
	static std::unordered_map<std::string, std::vector<Variable>> buffers;
	static Stack stack;
};