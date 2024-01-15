#include "std.hpp"
#include "Interpreter.hpp"
#include "Parser.hpp"

#define SET_EXTERN_FUNCTION(name) if (Parser::DoesFunctionExist(#name)) Interpreter::SetExternFunction<##name##>(#name);

void StandardLib::Init()
{
	for (std::string file : importedFiles)
	{
		if (file == "std/io.script")
		{
			SET_EXTERN_FUNCTION(WriteLine);
			SET_EXTERN_FUNCTION(GetLine);
		}
		if (file == "std/types.script")
		{
			SET_EXTERN_FUNCTION(ToString);
			SET_EXTERN_FUNCTION(ToFloat);
			SET_EXTERN_FUNCTION(ToInt);
			SET_EXTERN_FUNCTION(ToString);
			SET_EXTERN_FUNCTION(IntToString);
		}
		if (file == "std/string.script")
		{
			SET_EXTERN_FUNCTION(IndexString);
		}
		if (file == "std/reflection.script")
		{
			SET_EXTERN_FUNCTION(nameof);
			SET_EXTERN_FUNCTION(typeof);
		}
	}
}

WriteLine::WriteLine(Function* function) : Function(function)
{
	name = "WriteLine";
	parameters = { { "text", DATA_TYPE_STRING } };
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
	Return({ {}, DATA_TYPE_STRING, sizeof(std::string), Interpreter::FindVariable("value")->AsString() });
}

IntToString::IntToString(Function* function) : Function(function)
{
	name = "IntToString";
	parameters = { { "value", DATA_TYPE_INT } };
	returnType = DATA_TYPE_STRING;
}

void IntToString::Execute()
{
	Variable* var = Interpreter::FindVariable("value");
	var->SetDataType(DATA_TYPE_INT);
	Return({ {}, DATA_TYPE_STRING, sizeof(std::string), var->AsString() });
}

ToFloat::ToFloat(Function* function) : Function(function)
{
	name = "ToFloat";
	parameters = { { "text", DATA_TYPE_STRING } };
	returnType = DATA_TYPE_FLOAT;
}

void ToFloat::Execute()
{
	Return({ {}, DATA_TYPE_FLOAT, sizeof(float), std::to_string(std::stof(*Interpreter::FindVariable("text"))) });
}

ToInt::ToInt(Function* function) : Function(function)
{
	name = "ToInt";
	parameters = { { "text", DATA_TYPE_STRING } };
	returnType = DATA_TYPE_INT;
}

void ToInt::Execute()
{
	std::string str = (std::string)*Interpreter::FindVariable("text");
	int var = std::stoi(str);
	std::string ret = std::to_string(var);
	Return({ {}, DATA_TYPE_INT, sizeof(int), ret });
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
	Return({ DataTypeToInternalTypeString(Interpreter::FindVariable("var")->type), DATA_TYPE_STRING_CONSTANT });
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
	Return({ {}, DATA_TYPE_STRING, sizeof(std::string), ret});
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