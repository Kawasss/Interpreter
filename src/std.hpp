#pragma once
#include <iostream>
#include "Interpreter.hpp"
#include "Function.hpp"

namespace StandardLib
{
	inline extern void Init();
}

class WriteLine : public Function
{
public:
	WriteLine(Function* function);
	void Execute() override;
};

class ToString : public Function
{
public:
	ToString(Function* function);
	void Execute() override;
};

class nameof : public Function
{
public:
	nameof(Function* function);
	void Execute() override;
};

class typeof : public Function
{
public:
	typeof(Function* function);
	void Execute() override;
};

class GetLine : public Function
{
public:
	GetLine(Function* function);
	void Execute() override;
};

class ToFloat : public Function
{
public:
	ToFloat(Function* function);
	void Execute() override;
};

class ToInt : public Function
{
public:
	ToInt(Function* function);
	void Execute() override;
};

class IndexString : public Function
{
public:
	IndexString(Function* function);
	void Execute() override;
};