#include <iostream>
#include <stdexcept>
#include "Function.hpp"
#include "Interpreter.hpp"

Function::Function(FunctionInfo& info)
{
	this->name = info.name;
	this->parameters = info.parameters;
	this->returnType = info.returnType;
	this->instructions = info.instructions;
	//std::cout << "Created new function " << name << "\n";
}

void Function::CreateParameters()
{
	for (int i = 0; i < parameters.size(); i++)
	{
		Instruction declInst{};
		declInst.type = INSTRUCTION_TYPE_DECLARE;
		declInst.operand1 = parameters[i];

		Instruction pullInst{};
		pullInst.type = INSTRUCTION_TYPE_PULL;
		pullInst.operand1 = bufferParametersVar;
		pullInst.operand2 = parameters[i];

		Interpreter::ExecuteInstruction(declInst);
		Interpreter::ExecuteInstruction(pullInst);
	}
}

void Function::ExecuteBody()
{
	Interpreter::SetLocalScope(&stackFrame);
	CreateParameters();
	
	Execute();

	stackFrame.clear();
	Interpreter::GotoLowerScope();
}

void Function::Execute()
{
	for (int i = 0; i < instructions.size(); i++)
	{
		if (!Interpreter::ExecuteInstruction(instructions[i]))
			throw std::runtime_error("Failed to execute an instruction");
	}
}

void Function::Return(VariableInfo info)
{
	Instruction assignInst{};
	assignInst.type = INSTRUCTION_TYPE_ASSIGN;
	assignInst.operand1 = floatReturnVar;
	assignInst.operand2 = info;

	Interpreter::ExecuteInstruction(assignInst);
}

std::string Function::GetName()
{
	return name;
}