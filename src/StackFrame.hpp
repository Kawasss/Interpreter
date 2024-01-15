#pragma once
#include <unordered_map>
#include <map>
#include <vector>
#include "common.hpp"

typedef std::map<std::string, Variable> Scope;

// memory locations are sort of an alternative to pointers, where a memory location contains the scopes location and the variables location within that scope
// they can be encoded so that it contains both the index of the scope of the current stack frame and the index of the variable inside that scope
// this makes it easily possible to get a variable from within a stack frame
typedef uint32_t MemoryLocation; // memory location is defined so that its easier to change the type later on 

class StackFrame
{
public:
	StackFrame();

	void Allocate(const VariableInfo& info);
	void IncrementScope();
	void DecrementScope();
	void Clear();
	size_t Size() const;

	Variable& GetVariableAtMemoryLocation(MemoryLocation location);
	MemoryLocation EncodeVariableIntoLocation(std::string variable);
	static void DecodeMemoryLocation(MemoryLocation location, uint16_t& stackIndex, uint16_t& scopeIndex);
	
	bool Has(std::string var);

	const Scope& At(size_t index) const;
	Variable& operator[](std::string index);
	Variable& GetVariable(std::string var);

private:
	std::vector<Scope> scopes;
	bool firstPop = true; // for debug
};