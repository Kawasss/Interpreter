#include <iostream>
#include "StackFrame.hpp"
#include "Behavior.hpp"
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
	if (Behavior::dumpStackFrame && firstPop)
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
	if (Behavior::dumpStackFrame && firstPop)
	{
		std::cout << Debug::DumpStackFrame(this) << "\n";
		firstPop = false;
	}
	firstPop = true;
}

MemoryLocation StackFrame::EncodeVariableIntoLocation(std::string variable)
{
	if (!Has(variable))
		throw std::runtime_error("Memory error: cannot encode variable, because it does not exist in the current stack frame");

	uint16_t scopeIndex = 0, locationIndex = 0;
	for (int i = 0; i < scopes.size(); i++)
		if (scopes[i].count(variable) > 0)
		{
			scopeIndex = i;
			locationIndex = (uint16_t)std::distance(scopes[i].begin(), scopes[i].find(variable));
		}
	MemoryLocation ret = 0;
	memcpy(&ret, &scopeIndex, sizeof(scopeIndex));
	memcpy((unsigned char*)&ret + 2, &locationIndex, sizeof(locationIndex));
	return ret;
}

Variable& StackFrame::GetVariableAtMemoryLocation(MemoryLocation location)
{
	uint16_t stackIndex, scopeIndex;
	DecodeMemoryLocation(location, stackIndex, scopeIndex);
	Scope::iterator iterator = scopes[stackIndex].begin();

	std::advance(iterator, scopeIndex);
	return iterator->second;
}

void StackFrame::DecodeMemoryLocation(MemoryLocation location, uint16_t& stackIndex, uint16_t& scopeIndex)
{
	memcpy(&stackIndex, &location, sizeof(stackIndex));
	memcpy(&scopeIndex, (unsigned char*)&location + 2, sizeof(scopeIndex));
}