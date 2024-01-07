#pragma once
#include "common.hpp"
#include <unordered_map>

class Function;
struct AbstractSyntaxTree;
typedef std::unordered_map<std::string, Variable> StackFrame;

const VariableInfo floatCalculationVar = { "%fcv", DATA_TYPE_FLOAT };
const VariableInfo floatReturnVar =      { "%frv", DATA_TYPE_FLOAT };
const VariableInfo bufferParametersVar = { "%bpv", DATA_TYPE_VOID  };

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
	static void SetExternFunction(Function* function);

	static bool ExecuteInstruction(Instruction& instruction);
	template<typename T, typename Y> bool ExecuteInstruction(InstructionType type, Variable* operand1, Variable* operand2, void* pNext = nullptr);
	static Variable* FindVariable(std::string name);
	static Variable* FindVariable(VariableInfo& info);
	static Variable  GetValue(VariableInfo& info);
	static void DeclareVariable(const VariableInfo& info, StackFrame* stackFrame);
	static void DeclareBuffer(std::string name);
	static void SetLocalScope(StackFrame* pStackFrame);
	static void GotoEnclosingScope();
	static void GotoLowerScope();
	static void CopyLocalVariableToStackFrame(std::string sourceName, std::string newName, StackFrame* destination);

	template<typename T> static T FindVariable(VariableInfo& info)
	{
		return (T)*FindVariable(info.name);
	}

private:
	static StackFrame cacheVariables;
	static StackFrame* localScope;
	static StackFrame enclosingScope;
	static StackFrame globalScope;

	static std::unordered_map<std::string, Function*> functions;
	static std::unordered_map<std::string, std::vector<Variable>> buffers;
	static std::vector<StackFrame*> stack;
};