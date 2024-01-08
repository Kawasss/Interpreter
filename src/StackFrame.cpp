#include "StackFrame.hpp"

StackFrame::StackFrame()
{
	IncrementScope();
}

void StackFrame::Allocate(const VariableInfo& info)
{
	scopes.back().insert({ info.name, { info.name, info.dataType } });
}

void StackFrame::IncrementScope()
{
	scopes.push_back({});
}

void StackFrame::DecrementScope()
{
	scopes.back().clear();
	scopes.pop_back();
}

Variable& StackFrame::operator[](std::string index)
{
	for (Scope& scope : scopes)
		if (scope.count(index) > 0)
			return scope[index];
	return scopes.back()[index];
}

Variable& StackFrame::GetVariable(std::string var)
{
	for (Scope& scope : scopes)
		if (scope.count(var) > 0)
			return scope[var];
	throw std::runtime_error("Cannot find variable \"" + var + "\" in the current stack frame");
}

bool StackFrame::Has(std::string var)
{
	for (Scope& scope : scopes)
		if (scope.count(var) > 0)
			return true;
	return false;
}

void StackFrame::Clear()
{
	scopes.clear();
	IncrementScope(); // always keep one scope alive
}