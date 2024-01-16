#include <iostream>
#include <stdexcept>
#include "Interpreter.hpp"
#include "Parser.hpp"
#include "Debug.hpp"
#include "Behavior.hpp"
#include "common.hpp"

Scope Interpreter::cacheVariables;

std::unordered_map<std::string, Function*> Interpreter::functions;
std::unordered_map<std::string, std::vector<Variable>> Interpreter::buffers;
Stack Interpreter::stack;

void Interpreter::Init()
{
	DeclareCacheVariable(floatCalculationVar);
	DeclareCacheVariable(floatStorageVar);
	DeclareCacheVariable(floatReturnVar);
	DeclareCacheVariable(leftBoolValue);
	DeclareCacheVariable(rightBoolValue);
	DeclareBuffer(bufferParametersVar.name);
}

void Interpreter::SetAST(AbstractSyntaxTree& ast)
{
	for (Function* fnPtr : ast.functions)
	{
		functions[fnPtr->GetName()] = fnPtr;
	}
}

bool Interpreter::ExecuteInstructions(std::vector<Instruction> instructions)
{
	for (size_t instructionPointer = 0; instructionPointer < instructions.size(); instructionPointer++)
	{
		Instruction& instruction = instructions[instructionPointer];
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
			if (GetValue(instruction.operand1) == GetValue(instruction.operand2)) // the second instruction only gets executed if the comparison is false
				instructionPointer++;
			break;
		case INSTRUCTION_TYPE_NOT_EQUAL:
			if (GetValue(instruction.operand1) != GetValue(instruction.operand2))
				instructionPointer++;
			break;
		case INSTRUCTION_TYPE_GREATER:
			if (GetValue(instruction.operand1) > GetValue(instruction.operand2))
				instructionPointer++;
			break;
		case INSTRUCTION_TYPE_LESS:
			if (GetValue(instruction.operand1) < GetValue(instruction.operand2))
				instructionPointer++;
			break;
		case INSTRUCTION_TYPE_EQUAL_OR_GREATER:
			if (GetValue(instruction.operand1) >= GetValue(instruction.operand2))
				instructionPointer++;
			break;
		case INSTRUCTION_TYPE_EQUAL_OR_LESS:
			if (GetValue(instruction.operand1) <= GetValue(instruction.operand2))
				instructionPointer++;
			break;

		case INSTRUCTION_TYPE_ASSIGN:
		{
			Variable var = GetValue(instruction.operand2);
			var.name = instruction.operand1.name; // weird??
			*FindVariable(instruction.operand1) = var;// GetValue(instruction.operand2);
			if (cacheVariables.count(instruction.operand2.name) > 0) // if a cache variable is read from (done being used) it gets reset
				cacheVariables[instruction.operand2.name] = Variable(VariableInfo{ instruction.operand2.name, DATA_TYPE_VOID, 40 });
			if (cacheVariables.count(instruction.operand1.name) > 0) // cache variables are always void
				cacheVariables[instruction.operand1.name].SetDataType(DATA_TYPE_VOID);
			break;
		}

		case INSTRUCTION_TYPE_DECLARE:
			DeclareVariable(instruction.operand1);
			break;

		case INSTRUCTION_TYPE_PUSH: // push a variable at the back of a buffer
			buffers[instruction.operand1.name].push_back(GetValue(instruction.operand2));
			buffers[instruction.operand1.name].back().name = instruction.operand2.name;
			break;
		case INSTRUCTION_TYPE_PULL: // pull the oldest value from a buffer
			if (buffers[instruction.operand1.name].empty())
				throw std::runtime_error("Cannot pull from buffer " + instruction.operand1.name + ": it is empty");
			*FindVariable(instruction.operand2) = buffers[instruction.operand1.name][0];
			buffers[instruction.operand1.name].erase(buffers[instruction.operand1.name].begin());
			break;

		case INSTRUCTION_TYPE_CALL:
			stack.CreateNewStackFrame(); // add a new, empty stack
			if (functions.count(instruction.operand1.name) <= 0)
				throw std::runtime_error("Cannot find function " + instruction.operand1.name);
			functions[instruction.operand1.name]->ExecuteBody();
			break;
		case INSTRUCTION_TYPE_RETURN:
			stack.GotoEnclosingStackFrame(); // remove the stack of the finished function
			break;

		case INSTRUCTION_TYPE_JUMP:
			instructionPointer += (size_t)std::stoi(instruction.operand1.name) - 1;
			break;

		case INSTRUCTION_TYPE_PUSH_SCOPE:
			stack.Last().IncrementScope();
			break;
		case INSTRUCTION_TYPE_POP_SCOPE:
			stack.Last().DecrementScope();
			break;

		case INSTRUCTION_TYPE_DEREFERENCE: // with deference the first operand is the pointer, the second is the variable to copy to
		{
			Variable* location = (Variable*)(uint64_t)GetValue(instruction.operand1);
			*FindVariable(instruction.operand2) = *location;
			break;
		}
		case INSTRUCTION_TYPE_ASSIGN_LOCATION:
			*FindVariable(instruction.operand1) = (uint64_t)FindVariable(instruction.operand2);
			break;
		
		case INSTRUCTION_TYPE_INVALID:
			throw std::runtime_error("Recieved invalid instruction");
		}
	}
	return true;
}

inline bool IsConstant(DataType type)
{
	return type == DATA_TYPE_CHAR_CONSTANT || type == DATA_TYPE_FLOAT_CONSTANT || type == DATA_TYPE_INT_CONSTANT || type == DATA_TYPE_STRING_CONSTANT;
}

inline Variable GetConstantTypeAsVariable(VariableInfo& info)
{
	switch (info.dataType)
	{
	case DATA_TYPE_STRING_CONSTANT:
	case DATA_TYPE_CHAR_CONSTANT:  return Variable(info.name);
	case DATA_TYPE_FLOAT_CONSTANT: return std::stof(info.name);
	case DATA_TYPE_INT_CONSTANT:   return std::stoi(info.name);
	}
	return 0;
}

Variable Interpreter::GetValue(VariableInfo& info)
{
	if (!info.literalValue.empty())
	{
		switch (info.dataType)
		{
		case DATA_TYPE_STRING:
			return info.literalValue;
		case DATA_TYPE_FLOAT:
			return std::stof(info.literalValue);
		case DATA_TYPE_INT:
			return std::stoi(info.literalValue);
		case DATA_TYPE_CHAR:
			return info.literalValue[0];
		}
	}
	return *FindVariable(info);
}

Variable* Interpreter::FindVariable(std::string name)
{
	if (cacheVariables.count(name) > 0)
		return &cacheVariables[name];

	if (stack.Last().Has(name))
	{
		if (Behavior::treatVoidAsError && stack.Last().GetVariable(name).type == DATA_TYPE_VOID)
			throw std::runtime_error("Runtime error: invalid type used (type == DATA_TYPE_VOID && treatVoidAsError)\nremove the -treat_void_as_error argument to remove this error");
		return &stack.Last().GetVariable(name);
	}
		
	throw std::runtime_error("Failed to find variable " + name);
}

void Interpreter::DeclareVariable(const VariableInfo& info)
{
	stack.Last().Allocate(info);
	//std::cout << "Created new variable \"" << info.name << "\"\n";
}

Variable* Interpreter::FindVariable(VariableInfo& info)
{
	return FindVariable(info.name);
}

void Interpreter::CopyLocalVariableToStackFrame(std::string sourceName, std::string newName, StackFrame* destination)
{
	destination->Allocate({ newName, DATA_TYPE_VOID });
}

void Interpreter::DeclareBuffer(std::string name)
{
	buffers[name] = {};
}

void Interpreter::DeclareCacheVariable(const VariableInfo& info)
{
	cacheVariables[info.name] = { info };
}