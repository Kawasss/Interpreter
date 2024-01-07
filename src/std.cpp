#include "std.hpp"

void StandardLib::Init()
{
	Interpreter::SetExternFunction(new WriteLine());
	Interpreter::SetExternFunction(new ToString());
	Interpreter::SetExternFunction(new ToFloat());
	Interpreter::SetExternFunction(new ToInt());
	Interpreter::SetExternFunction(new nameof());
	Interpreter::SetExternFunction(new typeof());
	Interpreter::SetExternFunction(new GetLine());
}

WriteLine::WriteLine()
{
	name = "WriteLine";
	parameters = { { "text", DATA_TYPE_FLOAT } };
}

void WriteLine::Execute()
{
	std::cout << (std::string)stackFrame["text"] << "\n";
}

ToString::ToString()
{
	name = "ToString";
	parameters = { { "value", DATA_TYPE_FLOAT } };
}

void ToString::Execute()
{
	Return({ std::to_string((float)stackFrame["value"]), DATA_TYPE_STRING_CONSTANT });
}

ToFloat::ToFloat()
{
	name = "ToFloat";
	parameters = { { "text", DATA_TYPE_STRING } };
}

void ToFloat::Execute()
{
	Return({ std::to_string(std::stof(stackFrame["text"])), DATA_TYPE_FLOAT_CONSTANT });
}

ToInt::ToInt()
{
	name = "ToInt";
	parameters = { { "text", DATA_TYPE_STRING } };
}

void ToInt::Execute()
{
	Return({ std::to_string(std::stoi(stackFrame["text"])), DATA_TYPE_INT_CONSTANT });
}

nameof::nameof()
{
	name = "nameof";
	parameters = { { "var", DATA_TYPE_VOID } };
}

void nameof::Execute()
{
	Return({ stackFrame["var"].name, DATA_TYPE_STRING_CONSTANT });
}

typeof::typeof()
{
	name = "typeof";
	parameters = { { "var", DATA_TYPE_VOID } };
}

void typeof::Execute()
{
	Return({ DataTypeToString(stackFrame["var"].type), DATA_TYPE_STRING_CONSTANT });
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

GetLine::GetLine()
{
	name = "GetLine";
}

void GetLine::Execute()
{
	std::string ret;
	std::cin >> ret;
	Return({ ret, DATA_TYPE_STRING_CONSTANT });
}