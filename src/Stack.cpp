#include "Stack.hpp"

Stack::Stack()
{
	stack.push_back({});
}

void Stack::GotoEnclosingStackFrame()
{
	stack.pop_back();
}

void Stack::CreateNewStackFrame()
{
	stack.push_back({});
}

StackFrame& Stack::operator[](size_t index)
{
	return stack[index];
}

StackFrame& Stack::Last()
{
	return stack.back();
}

StackFrame& Stack::First()
{
	return stack[0];
}

std::vector<StackFrame>::iterator Stack::Begin()
{
	return stack.begin();
}

std::vector<StackFrame>::iterator Stack::End()
{
	return stack.end();
}

size_t Stack::Size() const
{
	return stack.size();
}