#include "common.hpp"

inline bool DataTypeIsConstant(DataType type)
{
	return type == DATA_TYPE_CHAR_CONSTANT || type == DATA_TYPE_FLOAT_CONSTANT || type == DATA_TYPE_INT_CONSTANT || type == DATA_TYPE_STRING_CONSTANT;
}

void Variable::Create(VariableInfo info)
{
	this->name = info.name;
	this->type = info.dataType;
	this->size = info.size;
	data.resize(size);
	memset(data.data(), 0, size);

	switch (type) // most constant values require a different approach to copying
	{
	case DATA_TYPE_FLOAT_CONSTANT:
	{
		float resultF = std::stof(info.name);
		memcpy(data.data(), &resultF, sizeof(resultF));
		break;
	}
	case DATA_TYPE_CHAR_CONSTANT:
		memcpy(data.data(), &info.name[0], sizeof(char));
		break;
	case DATA_TYPE_INT_CONSTANT:
	{
		int resultI = std::stof(info.name);
		memcpy(data.data(), &resultI, sizeof(resultI));
		break;
	}
	default:
		if (!DataTypeIsString(type))
			break;
		memcpy(data.data(), &info.name, size);
		break;
	}
}

Variable::Variable(const Variable& rvalue) noexcept
{
	type = rvalue.type;
	size = rvalue.size;
	data = rvalue.data;
}

Variable::Variable(const VariableInfo& info)
{
	Create(info);
}

Variable::Variable(float rvalue)
{
	size = sizeof(rvalue);
	type = DATA_TYPE_FLOAT;
	data.resize(size);
	memcpy(data.data(), &rvalue, size);
}

Variable::Variable(char rvalue)
{
	size = sizeof(rvalue);
	type = DATA_TYPE_CHAR;
	data.resize(size);
	memcpy(data.data(), &rvalue, size);
}

Variable::Variable(int rvalue) 
{
	size = sizeof(rvalue);
	type = DATA_TYPE_INT;
	data.resize(size);
	memcpy(data.data(), &rvalue, size);
}

Variable::Variable(std::string rvalue)
{
	size = sizeof(rvalue);
	type = DATA_TYPE_STRING;
	data.resize(size);
	memcpy(data.data(), &rvalue, size);
}

Variable::Variable(std::string value, std::string name)
{
	type = name == "" ? DATA_TYPE_STRING_CONSTANT : DATA_TYPE_STRING;
	size = sizeof(value);
	data.resize(size);
	memcpy(data.data(), &value, size);
	this->name = name;
}

Variable& Variable::operator=(float rvalue)
{
	if (size == 0)
	{
		size = sizeof(float);
		data.resize(size);
	}
	memcpy(data.data(), &rvalue, size);
	return *this;
}

Variable& Variable::operator=(char rvalue)
{
	if (size == 0)
	{
		size = sizeof(char);
		data.resize(size);
	}
	memcpy(data.data(), &rvalue, size);
	return *this;
}

Variable& Variable::operator=(int rvalue)
{
	if (size == 0)
	{
		size = sizeof(int);
		data.resize(size);
	}
	memcpy(data.data(), &rvalue, size);
	return *this;
}

Variable& Variable::operator=(uint64_t value)
{
	size = sizeof(value);
	type = DATA_TYPE_UINT64;
	data.resize(size);
	memcpy(data.data(), &value, sizeof(value));
	return *this;
}

Variable& Variable::operator=(std::string rvalue)
{
	if (size == 0)
	{
		size = sizeof(std::string);
		data.resize(size);
	}
	memcpy(data.data(), &rvalue, size);
	return *this;
}

Variable& Variable::operator+=(const Variable& rvalue)
{
	if (type == DATA_TYPE_VOID && name[0] != '%')
		type = DATA_TYPE_INT; // assume int as the default type
	switch (rvalue.type)
	{
	case DATA_TYPE_STRING_CONSTANT:
	case DATA_TYPE_STRING:
		*(std::string*)data.data() += (std::string)rvalue;
		break;
	
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
	{
		int first = *(int*)data.data();
		int second = (int)rvalue;
		int test = first + second;
		memcpy(data.data(), &test, sizeof(test));
		test--;
	}
		break;
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		*(float*)data.data() += (float)rvalue;
		break;
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		*(char*)data.data() += (char)rvalue;
		break;
	case DATA_TYPE_UINT64:
		*(uint64_t*)data.data() += (uint64_t)rvalue;
	}
	return *this;
}

Variable operator+(Variable lvalue, const Variable& rvalue)
{
	lvalue += rvalue;
	return lvalue;
}

Variable& Variable::operator-=(const Variable& rvalue)
{
	switch (type)
	{
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		*(int*)data.data() -= rvalue.GetDataAs<int>();
		break;
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		*(float*)data.data() -= rvalue.GetDataAs<float>();
		break;
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		*(char*)data.data() -= rvalue.GetDataAs<char>();
		break;
	}
	return *this;
}

Variable operator-(Variable lvalue, const Variable& rvalue)
{
	lvalue -= rvalue;
	return lvalue;
}

Variable& Variable::operator/=(const Variable& rvalue)
{
	switch (type)
	{
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		*(int*)data.data() /= rvalue.GetDataAs<int>();
		break;
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		*(float*)data.data() /= rvalue.GetDataAs<float>();
		break;
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		*(char*)data.data() /= rvalue.GetDataAs<char>();
		break;
	}
	return *this;
}

Variable operator/(Variable lvalue, const Variable& rvalue)
{
	lvalue /= rvalue;
	return lvalue;
}

Variable& Variable::operator*=(const Variable& rvalue)
{
	switch (type)
	{
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		*(int*)data.data() *= rvalue.GetDataAs<int>();
		break;
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		*(float*)data.data() *= rvalue.GetDataAs<float>();
		break;
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		*(char*)data.data() *= rvalue.GetDataAs<char>();
		break;
	}
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
	std::string* ptr = (std::string*)data.data();
	return std::string{ ptr->at(index) };
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
	switch (lvalue.type)
	{
	case DATA_TYPE_STRING:
	case DATA_TYPE_STRING_CONSTANT:
		return *(std::string*)lvalue.data.data() == rvalue.GetDataAs<std::string>();
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return *(int*)lvalue.data.data() == rvalue.GetDataAs<int>();
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return *(float*)lvalue.data.data() == rvalue.GetDataAs<float>();
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return *(char*)lvalue.data.data() == rvalue.GetDataAs<char>();
	}
}

bool operator!=(const Variable& lvalue, const Variable& rvalue)
{
	return !(lvalue == rvalue);
}

Variable::operator char() const
{
	switch (type)
	{
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return (char)*(int*)data.data();
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return (char)*(float*)data.data();
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return *(char*)data.data();
	case DATA_TYPE_UINT64:
		return (char)*(uint64_t*)data.data();
	}
	return '\0';
}

Variable::operator float() const
{
	switch (type)
	{
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return (float)*(int*)data.data();
	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return *(float*)data.data();
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return (float)*(char*)data.data();
	case DATA_TYPE_UINT64:
		return (float)*(uint64_t*)data.data();
	}
	return 0;
}

Variable::operator int() const
{
	switch (type)
	{
	case DATA_TYPE_VOID:
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return *(int*)data.data();

	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return (int)*(float*)data.data();
		
	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return (int)*(char*)data.data();
	case DATA_TYPE_UINT64:
		return (int)*(uint64_t*)data.data();
	}
	return 0;
}

Variable::operator uint64_t() const
{
	switch (type)
	{
	case DATA_TYPE_VOID:
	case DATA_TYPE_INT:
	case DATA_TYPE_INT_CONSTANT:
		return *(int*)data.data();

	case DATA_TYPE_FLOAT:
	case DATA_TYPE_FLOAT_CONSTANT:
		return (int)*(float*)data.data();

	case DATA_TYPE_CHAR:
	case DATA_TYPE_CHAR_CONSTANT:
		return (int)*(char*)data.data();
	case DATA_TYPE_UINT64:
		return *(uint64_t*)data.data();
	}
	return 0;
}

Variable::operator std::string() const
{
	if (DataTypeIsString(type) || type == DATA_TYPE_VOID)
		return *(std::string*)data.data();
	return "";
}

std::string Variable::AsString()
{
	switch (type)
	{
	case DATA_TYPE_CHAR:
		return std::string{ (char)data[0] };
	case DATA_TYPE_FLOAT:
		return std::to_string(*(float*)data.data());
	case DATA_TYPE_INT:
		return std::to_string(*(int*)data.data());
	case DATA_TYPE_UINT64:
		return std::to_string(*(uint64_t*)data.data());
	case DATA_TYPE_VOID:
		if (data.size() < sizeof(std::string))
			break;
		[[fallthrough]];
	case DATA_TYPE_STRING:
	case DATA_TYPE_CHAR_CONSTANT:
	case DATA_TYPE_FLOAT_CONSTANT:
	case DATA_TYPE_INT_CONSTANT:
	case DATA_TYPE_STRING_CONSTANT:
		return *(std::string*)data.data();
	}
	return "Cannot convert variable to string";
}

DataType Variable::GetDataType()
{
	return type;
}

void Variable::SetDataType(DataType type)
{
	this->type = type;
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
	case DATA_TYPE_FLOAT_CONSTANT:
	case DATA_TYPE_FLOAT: return sizeof(float);
	case DATA_TYPE_CHAR_CONSTANT:
	case DATA_TYPE_CHAR: return sizeof(char);
	case DATA_TYPE_INT_CONSTANT:
	case DATA_TYPE_INT: return sizeof(int);
	case DATA_TYPE_UINT64: return sizeof(uint64_t);
	case DATA_TYPE_STRING_CONSTANT:
	case DATA_TYPE_STRING: return sizeof(std::string);
	}
	return 0;
}

bool operator==(const VariableInfo& lvalue, const VariableInfo& rvalue)
{
	if (lvalue.name == "" && rvalue.name == "")
		return lvalue.literalValue == rvalue.literalValue && lvalue.dataType == rvalue.dataType;
	return lvalue.name == rvalue.name && lvalue.dataType == rvalue.dataType;
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
	case INSTRUCTION_TYPE_DEREFERENCE:       return "INSTRUCTION_TYPE_DEREFERENCE";
	case INSTRUCTION_TYPE_ASSIGN_LOCATION:   return "INSTRUCTION_TYPE_ASSIGN_LOCATION";
	}
	return "";
}

std::string DataTypeToInternalTypeString(DataType type)
{
	switch (type)
	{
	case DATA_TYPE_FLOAT:           return "float";
	case DATA_TYPE_CHAR:            return "char";
	case DATA_TYPE_INT:             return "int";
	case DATA_TYPE_STRING:          return "string";
	case DATA_TYPE_VOID:            return "void";
	case DATA_TYPE_USERTYPE:        return "user_type";
	case DATA_TYPE_UINT64:          return "uint64";
	case DATA_TYPE_FLOAT_CONSTANT:  return "float_literal";
	case DATA_TYPE_CHAR_CONSTANT:   return "char_literal";
	case DATA_TYPE_INT_CONSTANT:    return "int_literal";
	case DATA_TYPE_STRING_CONSTANT: return "string_literal";
	}
	return "invalid_type";
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