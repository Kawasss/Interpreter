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
	size_t Size() const;
	
	bool Has(std::string var);

	const Scope& At(size_t index) const;
	Variable& operator[](std::string index);
	Variable& GetVariable(std::string var);

private:
	std::vector<Scope> scopes;
	bool firstPop = true; // for debug
};