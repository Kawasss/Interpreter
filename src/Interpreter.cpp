#include <iostream>
#include <stdexcept>
#include "Interpreter.hpp"
#include "Parser.hpp"

StackFrame Interpreter::cacheVariables;
StackFrame* Interpreter::localScope;
StackFrame Interpreter::enclosingScope;
StackFrame Interpreter::globalScope;

std::unordered_map<std::string, Function*> Interpreter::functions;
std::unordered_map<std::string, std::vector<Variable>> Interpreter::buffers;
std::vector<StackFrame*> Interpreter::stack;

void Interpreter::Init()
{
	DeclareVariable(floatCalculationVar, &cacheVariables);
	DeclareVariable(floatReturnVar, &cacheVariables);
	DeclareBuffer(bufferParametersVar.name);
}

void Interpreter::SetAST(AbstractSyntaxTree& ast)
{
	for (Function* fnPtr : ast.functions)
	{
		functions[fnPtr->GetName()] = fnPtr;
	}
}

void Interpreter::SetExternFunction(Function* function)
{
	functions[function->GetName()] = function;
}

bool Interpreter::ExecuteInstruction(Instruction& instruction)
{
	switch (instruction.type)
	{
	case INSTRUCTION_TYPE_ADD:
		*FindVariable(instruction.operand1) = *FindVariable(instruction.operand1) + GetValue(instruction.operand2);
		break;
	case INSTRUCTION_TYPE_SUBTRACT:
		*FindVariable(instruction.operand1) = *FindVariable(instruction.operand1) - GetValue(instruction.operand2);
		break;
	case INSTRUCTION_TYPE_DIVIDE:
		*FindVariable(instruction.operand1) = *FindVariable(instruction.operand1) / GetValue(instruction.operand2);
		break;
	case INSTRUCTION_TYPE_MULTIPLY:
		*FindVariable(instruction.operand1) = *FindVariable(instruction.operand1) * GetValue(instruction.operand2);
		break;
	case INSTRUCTION_TYPE_EQUAL:
		if (*FindVariable(instruction.operand1) == GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;
	case INSTRUCTION_TYPE_NOT_EQUAL:
		if (*FindVariable(instruction.operand1) != GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;
	case INSTRUCTION_TYPE_GREATER:
		if (*FindVariable(instruction.operand1) > GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;
	case INSTRUCTION_TYPE_LESS:
		if (*FindVariable(instruction.operand1) < GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;
	case INSTRUCTION_TYPE_EQUAL_OR_GREATER:
		if (*FindVariable(instruction.operand1) >= GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;
	case INSTRUCTION_TYPE_EQUAL_OR_LESS:
		if (*FindVariable(instruction.operand1) <= GetValue(instruction.operand2))
			ExecuteInstruction(*(Instruction*)instruction.pNext);
		break;

	case INSTRUCTION_TYPE_ASSIGN:
	{
		Variable var = GetValue(instruction.operand2);
		*FindVariable(instruction.operand1) = var;// GetValue(instruction.operand2);
		if (cacheVariables.count(instruction.operand2.name) > 0) // if a cache variable is read from (done being used) it gets reset
			cacheVariables[instruction.operand2.name] = 0;
		break;
	}

	case INSTRUCTION_TYPE_DECLARE:
		DeclareVariable(instruction.operand1, localScope);
		break;

	case INSTRUCTION_TYPE_PUSH: // push a variable at the back of a buffer
		buffers[instruction.operand1.name].push_back(GetValue(instruction.operand2));
		buffers[instruction.operand1.name].back().name = instruction.operand2.name;
		break;
	case INSTRUCTION_TYPE_PULL: // pull the oldest value from a buffer
		*FindVariable(instruction.operand2) = buffers[instruction.operand1.name][0];
		buffers[instruction.operand1.name].erase(buffers[instruction.operand1.name].begin());
		break;

	case INSTRUCTION_TYPE_CALL:
		functions[instruction.operand1.name]->ExecuteBody();
		break;
	}
	return true;
}

inline bool IsConstant(DataType type)
{
	return type == DATA_TYPE_CHAR_CONSTANT || type == DATA_TYPE_FLOAT_CONSTANT || type == DATA_TYPE_INT_CONSTANT || type == DATA_TYPE_STRING_CONSTANT;
}

Variable Interpreter::GetValue(VariableInfo& info)
{
	if (info.dataType == DATA_TYPE_STRING_CONSTANT)
		return info.name;
	return IsConstant(info.dataType) ? (Variable)std::stof(info.name) : *FindVariable(info);
}

Variable* Interpreter::FindVariable(std::string name)
{
	if (cacheVariables.count(name) > 0)
		return &cacheVariables[name];
	if (localScope->count(name) > 0)
		return &localScope->at(name);
	if (enclosingScope.count(name) > 0)
		return &enclosingScope[name];
	if (globalScope.count(name) > 0)
		return &globalScope[name];
	throw std::runtime_error("Failed to find variable " + name);
}

void Interpreter::DeclareVariable(const VariableInfo& info, StackFrame* stackFrame)
{
	stackFrame->insert({ info.name, { info.name, info.dataType } });
	//std::cout << "Created new variable \"" << info.name << "\"\n";
}

Variable* Interpreter::FindVariable(VariableInfo& info)
{
	return FindVariable(info.name);
}

void Interpreter::SetLocalScope(StackFrame* pStackFrame)
{
	if (pStackFrame->count(floatCalculationVar.name) <= 0) // every stack frame has its own float calc var
		pStackFrame->insert({ floatCalculationVar.name, { floatCalculationVar.name, floatCalculationVar.dataType } });
	stack.push_back(pStackFrame);
	localScope = stack.back();
}

void Interpreter::GotoEnclosingScope()
{
	localScope = &enclosingScope;
}

void Interpreter::GotoLowerScope()
{
	if (stack.size() == 1)
		return;

	stack.pop_back();
	localScope = stack.back();
}

void Interpreter::CopyLocalVariableToStackFrame(std::string sourceName, std::string newName, StackFrame* destination)
{
	destination->insert({ newName, localScope->at(sourceName) });
}

void Interpreter::DeclareBuffer(std::string name)
{
	buffers[name] = {};
}