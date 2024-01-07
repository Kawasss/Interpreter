#pragma once
#include "common.hpp"
#include <unordered_map>

typedef std::unordered_map<std::string, Variable> StackFrame;

struct FunctionInfo
{
	std::string name;
	std::vector<VariableInfo> parameters;
	std::vector<Instruction> instructions;
	DataType returnType;
};

class Function
{
public:
	Function() = default;
	Function(FunctionInfo& info);
	~Function() {}

	void ExecuteBody();

	std::string GetName();

protected:
	virtual void Execute();
	void Return(VariableInfo info);
	std::string name = "";
	std::vector<VariableInfo> parameters;
	DataType returnType = DATA_TYPE_INVALID;
	StackFrame stackFrame{};

private:
	void CreateParameters();

	std::vector<Instruction> instructions;
};