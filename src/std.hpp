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
	WriteLine();
	void Execute() override;
};

class ToString : public Function
{
public:
	ToString();
	void Execute() override;
};

class nameof : public Function
{
public:
	nameof();
	void Execute() override;
};

class typeof : public Function
{
public:
	typeof();
	void Execute() override;

private:
	std::string DataTypeToString(DataType type);
};

class GetLine : public Function
{
public:
	GetLine();
	void Execute() override;
};

class ToFloat : public Function
{
public:
	ToFloat();
	void Execute() override;
};

class ToInt : public Function
{
public:
	ToInt();
	void Execute() override;
};