#pragma once
#include <unordered_map>
#include <vector>
#include "common.hpp"

typedef std::unordered_map<std::string, Variable> Scope;

class StackFrame
{
public:
	StackFrame();

	void Allocate(const VariableInfo& info);
	void IncrementScope();
	void DecrementScope();
	void Clear();
	
	bool Has(std::string var);

	Variable& operator[](std::string index);
	Variable& GetVariable(std::string var);

private:
	std::vector<Scope> scopes;
};