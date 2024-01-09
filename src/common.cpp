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