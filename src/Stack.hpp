#pragma once
#include "StackFrame.hpp"

class Stack
{
public:
	Stack();
	void GotoEnclosingStackFrame();
	void CreateNewStackFrame();

	StackFrame& operator[](size_t index);
	StackFrame& Last();
	StackFrame& First();

	std::vector<StackFrame>::iterator Begin();
	std::vector<StackFrame>::iterator End();
	size_t Size() const;

private:
	std::vector<StackFrame> stack;
};