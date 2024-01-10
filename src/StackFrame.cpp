#include <iostream>
#include "StackFrame.hpp"
#include "Interpreter.hpp"
#include "Debug.hpp"

StackFrame::StackFrame()
{
	IncrementScope();
}

void StackFrame::Allocate(const VariableInfo& info)
{
	scopes.back().insert({ info.name, Variable(VariableInfo{ info }) });
}

void StackFrame::IncrementScope()
{
	scopes.push_back({});
}

void StackFrame::DecrementScope()
{
	if (Interpreter::dumpStackFrame && firstPop)
	{
		std::cout << Debug::DumpStackFrame(this) << "\n";
		firstPop = false;
	}
	scopes.back().clear();
	scopes.pop_back();
}

size_t StackFrame::Size() const
{
	return scopes.size();
}

const Scope& StackFrame::At(size_t index) const
{
	return scopes[index];
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
	if (Interpreter::dumpStackFrame && firstPop)
	{
		std::cout << Debug::DumpStackFrame(this) << "\n";
		firstPop = false;
	}
	firstPop = true;
}