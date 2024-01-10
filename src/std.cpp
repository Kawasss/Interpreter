#include "std.hpp"

#define SET_EXTERN_FUNCTION(name) Interpreter::SetExternFunction<##name##>(#name);

void StandardLib::Init()
{
	SET_EXTERN_FUNCTION(WriteLine);
	SET_EXTERN_FUNCTION(GetLine);
	SET_EXTERN_FUNCTION(ToString);
	SET_EXTERN_FUNCTION(ToFloat);
	SET_EXTERN_FUNCTION(ToInt);
	SET_EXTERN_FUNCTION(ToString);
	SET_EXTERN_FUNCTION(IndexString);
	//SET_EXTERN_FUNCTION(nameof);
	//SET_EXTERN_FUNCTION(typeof);
}

WriteLine::WriteLine(Function* function) : Function(function)
{
	name = "WriteLine";
	parameters = { { "text", DATA_TYPE_VOID } };
	returnType = DATA_TYPE_VOID;
}

void WriteLine::Execute()
{
	std::cout << Interpreter::FindVariable("text")->AsString() << "\n";
	Return({});
}

ToString::ToString(Function* function) : Function(function)
{
	name = "ToString";
	parameters = { { "value", DATA_TYPE_FLOAT } };
	returnType = DATA_TYPE_STRING;
}

void ToString::Execute()
{
	Return({ Interpreter::FindVariable("value")->AsString(), DATA_TYPE_STRING_CONSTANT });
}

ToFloat::ToFloat(Function* function) : Function(function)
{
	name = "ToFloat";
	parameters = { { "text", DATA_TYPE_STRING } };
	returnType = DATA_TYPE_FLOAT;
}

void ToFloat::Execute()
{
	Return({ std::to_string(std::stof(*Interpreter::FindVariable("text"))), DATA_TYPE_FLOAT_CONSTANT });
}

ToInt::ToInt(Function* function) : Function(function)
{
	name = "ToInt";
	parameters = { { "text", DATA_TYPE_STRING } };
	returnType = DATA_TYPE_INT;
}

void ToInt::Execute()
{
	Return({ std::to_string(std::stoi(*Interpreter::FindVariable("text"))), DATA_TYPE_INT_CONSTANT });
}

nameof::nameof(Function* function) : Function(function)
{
	name = "nameof";
	parameters = { { "var", DATA_TYPE_VOID } };
	returnType = DATA_TYPE_STRING;
}

void nameof::Execute()
{
	Return({ stackFrame["var"].name, DATA_TYPE_STRING_CONSTANT });
}

typeof::typeof(Function* function) : Function(function)
{
	name = "typeof";
	parameters = { { "var", DATA_TYPE_VOID } };
	returnType = DATA_TYPE_STRING;
}

void typeof::Execute()
{
	Return({ DataTypeToString(Interpreter::FindVariable("var")->type), DATA_TYPE_STRING_CONSTANT });
}

std::string typeof::DataTypeToString(DataType type)
{
	switch (type)
	{
	case DATA_TYPE_CHAR_CONSTANT:
	case DATA_TYPE_CHAR:
		return "char";
	case DATA_TYPE_FLOAT_CONSTANT:
	case DATA_TYPE_FLOAT:
		return "float";
	case DATA_TYPE_INT_CONSTANT:
	case DATA_TYPE_INT:
		return "int";
	case DATA_TYPE_STRING_CONSTANT:
	case DATA_TYPE_STRING:
		return "string";
	}
	return "invalid_type";
}

GetLine::GetLine(Function* function) : Function(function)
{
	name = "GetLine";
	returnType = DATA_TYPE_STRING_CONSTANT;
}

void GetLine::Execute()
{
	std::string ret;
	std::cin >> ret;
	Return({ ret, DATA_TYPE_STRING_CONSTANT });
}

IndexString::IndexString(Function* function) : Function(function)
{
	name = "IndexString";
	returnType = DATA_TYPE_STRING_CONSTANT;
}

void IndexString::Execute()
{
	char ret = ((std::string)*Interpreter::FindVariable("input"))[(int)*Interpreter::FindVariable("index")];
	Return({ std::string{ ret }, DATA_TYPE_STRING_CONSTANT });
}