#pragma once
#include <iostream>
#include "Interpreter.hpp"
#include "Function.hpp"

#define DECLARE_FUNCTION(name)  \
class name : public Function    \
{                               \
public:                         \
	name##(Function* function); \
	void Execute() override;    \
};                              \

DECLARE_FUNCTION(WriteLine);
DECLARE_FUNCTION(ToString);
DECLARE_FUNCTION(IntToString);
DECLARE_FUNCTION(nameof);
DECLARE_FUNCTION(typeof);
DECLARE_FUNCTION(GetLine);
DECLARE_FUNCTION(ToFloat);
DECLARE_FUNCTION(ToInt);
DECLARE_FUNCTION(IndexString);

namespace StandardLib
{
	inline extern void Init();
}