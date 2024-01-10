#include <iostream>
#include <stdexcept>
#include "Function.hpp"
#include "Interpreter.hpp"
#include "Debug.hpp"

Function::Function(Function* function)
{
	this->name = function->name;
	this->parameters = function->parameters;
	this->returnType = function->returnType;
	this->instructions = function->instructions;
}

Function::Function(FunctionInfo& info)
{
	this->name = info.name;
	this->parameters = info.parameters;
	this->returnType = info.returnType;
	this->instructions = info.instructions;
	CreateParameters();
	if (!instructions.empty())
	{
		std::cout << "Function \"" << name << "\" instruction dump:\n";
		std::cout << Debug::DumpInstructionsData(instructions) << "\n";
	}
}

void Function::CreateParameters()
{
	for (int i = parameters.size() - 1; i >= 0; i--)
	{
		Instruction declInst{};
		declInst.type = INSTRUCTION_TYPE_DECLARE;
		declInst.operand1 = parameters[i];

		Instruction pullInst{};
		pullInst.type = INSTRUCTION_TYPE_PULL;
		pullInst.operand1 = bufferParametersVar;
		pullInst.operand2 = parameters[i];

		instructions.insert(instructions.begin(), { declInst, pullInst });
	}
}

void Function::ExecuteBody()
{
	if (!Interpreter::ExecuteInstructions(instructions))
		throw std::runtime_error("Failed to execute an instruction");
	Execute();
}

void Function::Return(VariableInfo info)
{
	static Instruction returnInst{ INSTRUCTION_TYPE_RETURN };
	if (returnType == DATA_TYPE_VOID)
	{
		Interpreter::ExecuteInstructions({ returnInst });
		return;
	}

	Instruction assignInst{};
	assignInst.type = INSTRUCTION_TYPE_ASSIGN;
	assignInst.operand1 = floatReturnVar;
	assignInst.operand2 = info;

	Interpreter::ExecuteInstructions({ assignInst, returnInst });
}

std::string Function::GetName()
{
	return name;
}