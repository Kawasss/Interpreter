#include "common.hpp"

Variable::Variable(std::string name, DataType type)
{
	this->name = name;
	this->type = type;
	size = Sizeof(type);
}

Variable::Variable(const Variable& rvalue) noexcept
{
	type = rvalue.type;
	size = rvalue.size;
	data = rvalue.data;
	str = rvalue.str;
}

Variable::Variable(std::string value, std::string name)
{
	type = name == "" ? DATA_TYPE_STRING_CONSTANT : DATA_TYPE_STRING;
	size = sizeof(value);
	str = value;
	this->name = name;
}

Variable& Variable::operator+=(const Variable& rvalue)
{
	if ((type == DATA_TYPE_STRING || type == DATA_TYPE_STRING_CONSTANT) && (rvalue.type != DATA_TYPE_STRING && rvalue.type != DATA_TYPE_STRING_CONSTANT))
		throw std::runtime_error("Cannot add a non-string value to a string");
	if (type == DATA_TYPE_STRING || type == DATA_TYPE_STRING_CONSTANT)
		str += rvalue.str;
	else
		data += rvalue.data;
	return *this;
}

Variable operator+(Variable lvalue, const Variable& rvalue)
{
	lvalue += rvalue;
	return lvalue;
}

Variable& Variable::operator-=(const Variable& rvalue)
{
	data -= rvalue.data;
	return *this;
}

Variable operator-(Variable lvalue, const Variable& rvalue)
{
	lvalue -= rvalue;
	return lvalue;
}

Variable& Variable::operator/=(const Variable& rvalue)
{
	data /= rvalue.data;
	return *this;
}

Variable operator/(Variable lvalue, const Variable& rvalue)
{
	lvalue /= rvalue;
	return lvalue;
}

Variable& Variable::operator*=(const Variable& rvalue)
{
	data *= rvalue.data;
	return *this;
}

Variable operator*(Variable lvalue, const Variable& rvalue)
{
	lvalue *= rvalue;
	return lvalue;
}

std::string Variable::operator[](size_t index)
{
	if (type != DATA_TYPE_STRING && type != DATA_TYPE_STRING_CONSTANT)
		throw std::runtime_error("Cannot index this type: only strings can be indexed");
	return std::string{ str[index] };
}

bool operator<(const Variable& lvalue, const Variable& rvalue)
{
	return lvalue.data < rvalue.data;
}

bool operator>(const Variable& lvalue, const Variable& rvalue)
{
	return rvalue < lvalue;
}

bool operator<=(const Variable& lvalue, const Variable& rvalue)
{
	return !(lvalue > rvalue);
}

bool operator>=(const Variable& lvalue, const Variable& rvalue)
{
	return !(lvalue < rvalue);
}


bool operator==(const Variable& lvalue, const Variable& rvalue)
{
	if (lvalue.type == DATA_TYPE_STRING || lvalue.type == DATA_TYPE_STRING_CONSTANT)
		return lvalue.str == rvalue.str;
	else
		return lvalue.data == rvalue.data;
}

bool operator!=(const Variable& lvalue, const Variable& rvalue)
{
	return !(lvalue == rvalue);
}

Variable::operator char() const
{
	return (char)(int)data;
}

Variable::operator float() const
{
	return data;
}

Variable::operator int() const
{
	return (int)data;
}

Variable::operator std::string() const
{
	return str;
}

std::string Variable::AsString()
{
	switch (type)
	{
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return std::string{ str };
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return std::to_string((float)data);
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return std::to_string((int)data);
	case DATA_TYPE_STRING:
	case DATA_TYPE_STRING_CONSTANT:
		return str;
	}
	return "Cannot convert variable to string";
}

DataType Variable::GetDataType()
{
	return type;
}

bool DataTypeIsFloat(DataType type)
{
	return type == DATA_TYPE_FLOAT || type == DATA_TYPE_FLOAT_CONSTANT;
}

bool DataTypeIsChar(DataType type)
{
	return type == DATA_TYPE_CHAR || type == DATA_TYPE_CHAR_CONSTANT;
}

bool DataTypeIsInt(DataType type)
{
	return type == DATA_TYPE_INT || type == DATA_TYPE_INT_CONSTANT;
}

bool DataTypeIsString(DataType type)
{
	return type == DATA_TYPE_STRING || type == DATA_TYPE_STRING_CONSTANT;
}

size_t Sizeof(DataType dataType)
{
	switch (dataType)
	{
	case DATA_TYPE_FLOAT: return sizeof(float);
	case DATA_TYPE_CHAR: return sizeof(char);
	case DATA_TYPE_INT: return sizeof(int);
	}
	return 0;
}

std::string InstructionTypeToString(InstructionType type)
{
	switch (type)
	{
	case INSTRUCTION_TYPE_INVALID:           return "INSTRUCTION_TYPE_INVALID";
	case INSTRUCTION_TYPE_ADD:               return "INSTRUCTION_TYPE_ADD";
	case INSTRUCTION_TYPE_SUBTRACT:          return "INSTRUCTION_TYPE_SUBTRACT";
	case INSTRUCTION_TYPE_MULTIPLY:          return "INSTRUCTION_TYPE_MULTIPLY";
	case INSTRUCTION_TYPE_DIVIDE:            return "INSTRUCTION_TYPE_DIVIDE";
	case INSTRUCTION_TYPE_ASSIGN:            return "INSTRUCTION_TYPE_ASSIGN";
	case INSTRUCTION_TYPE_ASSIGN_CONSTANT:   return "INSTRUCTION_TYPE_ASSIGN_CONSTANT";
	case INSTRUCTION_TYPE_DECLARE:           return "INSTRUCTION_TYPE_DECLARE";
	case INSTRUCTION_TYPE_CALL:              return "INSTRUCTION_TYPE_CALL";
	case INSTRUCTION_TYPE_RETURN:            return "INSTRUCTION_TYPE_RETURN";
	case INSTRUCTION_TYPE_PUSH:              return "INSTRUCTION_TYPE_PUSH";
	case INSTRUCTION_TYPE_PULL:              return "INSTRUCTION_TYPE_PULL";
	case INSTRUCTION_TYPE_EQUAL:             return "INSTRUCTION_TYPE_EQUAL";
	case INSTRUCTION_TYPE_NOT_EQUAL:         return "INSTRUCTION_TYPE_NOT_EQUAL";
	case INSTRUCTION_TYPE_GREATER:           return "INSTRUCTION_TYPE_GREATER";
	case INSTRUCTION_TYPE_LESS:              return "INSTRUCTION_TYPE_LESS";
	case INSTRUCTION_TYPE_EQUAL_OR_GREATER:  return "INSTRUCTION_TYPE_EQUAL_OR_GREATER";
	case INSTRUCTION_TYPE_EQUAL_OR_LESS:     return "INSTRUCTION_TYPE_EQUAL_OR_LESS";
	case INSTRUCTION_TYPE_JUMP:              return "INSTRUCTION_TYPE_JUMP";
	case INSTRUCTION_TYPE_PUSH_SCOPE:        return "INSTRUCTION_TYPE_PUSH_SCOPE";
	case INSTRUCTION_TYPE_POP_SCOPE:         return "INSTRUCTION_TYPE_POP_SCOPE";
	case INSTRUCTION_TYPE_INDEX:             return "INSTRUCTION_TYPE_INDEX";
	}
	return "";
}

std::string DataTypeToString(DataType type)
{
	switch (type)
	{
	case DATA_TYPE_INVALID:         return "DATA_TYPE_INVALID";
	case DATA_TYPE_FLOAT:           return "DATA_TYPE_FLOAT";
	case DATA_TYPE_CHAR:            return "DATA_TYPE_CHAR";
	case DATA_TYPE_INT:             return "DATA_TYPE_INT";
	case DATA_TYPE_STRING:          return "DATA_TYPE_STRING";
	case DATA_TYPE_VOID:            return "DATA_TYPE_VOID";
	case DATA_TYPE_USERTYPE:        return "DATA_TYPE_USERTYPE";
	case DATA_TYPE_FLOAT_CONSTANT:  return "DATA_TYPE_FLOAT_CONSTANT";
	case DATA_TYPE_CHAR_CONSTANT:   return "DATA_TYPE_CHAR_CONSTANT";
	case DATA_TYPE_INT_CONSTANT:    return "DATA_TYPE_INT_CONSTANT";
	case DATA_TYPE_STRING_CONSTANT: return "DATA_TYPE_STRING_CONSTANT";
	}
	return "";
}