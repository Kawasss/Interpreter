#pragma once
#include <string>
#include <unordered_set>
#include <type_traits>
#include <stdexcept>

typedef unsigned char byte;

enum InstructionType
{
	INSTRUCTION_TYPE_INVALID,
	INSTRUCTION_TYPE_ADD,
	INSTRUCTION_TYPE_SUBTRACT,
	INSTRUCTION_TYPE_MULTIPLY,
	INSTRUCTION_TYPE_DIVIDE,
	INSTRUCTION_TYPE_ASSIGN,
	INSTRUCTION_TYPE_ASSIGN_CONSTANT,
	INSTRUCTION_TYPE_DECLARE,
	INSTRUCTION_TYPE_CALL,
	INSTRUCTION_TYPE_RETURN,
	INSTRUCTION_TYPE_PUSH, // push unto buffer
	INSTRUCTION_TYPE_PULL, // pull from buffer
	INSTRUCTION_TYPE_EQUAL, // boolean instructions expect the instruction to execute if the comparison is true in the pNext of the current instruction
	INSTRUCTION_TYPE_NOT_EQUAL,
	INSTRUCTION_TYPE_GREATER,
	INSTRUCTION_TYPE_LESS,
	INSTRUCTION_TYPE_EQUAL_OR_GREATER,
	INSTRUCTION_TYPE_EQUAL_OR_LESS,
	INSTRUCTION_TYPE_JUMP, // jump is relative to the current instruction index
	INSTRUCTION_TYPE_PUSH_SCOPE,
	INSTRUCTION_TYPE_POP_SCOPE,
	INSTRUCTION_TYPE_INDEX
};
inline extern std::string InstructionTypeToString(InstructionType type);

enum DataType
{
	DATA_TYPE_INVALID,
	DATA_TYPE_FLOAT,
	DATA_TYPE_CHAR,
	DATA_TYPE_INT,
	DATA_TYPE_STRING,
	DATA_TYPE_VOID,
	DATA_TYPE_USERTYPE,
	DATA_TYPE_FLOAT_CONSTANT,
	DATA_TYPE_CHAR_CONSTANT,
	DATA_TYPE_INT_CONSTANT,
	DATA_TYPE_STRING_CONSTANT,
	DATA_TYPE_POINTER,
};
inline extern std::string DataTypeToInternalTypeString(DataType type);
inline extern std::string DataTypeToString(DataType type);
inline extern bool DataTypeIsFloat(DataType type);
inline extern bool DataTypeIsChar(DataType type);
inline extern bool DataTypeIsInt(DataType type);
inline extern bool DataTypeIsString(DataType type);

inline extern size_t Sizeof(DataType dataType);

template<typename T> inline DataType GetDataTypeTemplate()
{
	if (std::is_same_v<T, float>)
		return DATA_TYPE_FLOAT;
	if (std::is_same_v<T, char>)
		return DATA_TYPE_CHAR;
	if (std::is_same_v<T, int>)
		return DATA_TYPE_INT;
	if (std::is_same_v<T, std::string>)
		return DATA_TYPE_STRING;
	return DATA_TYPE_INVALID;
}

struct VariableInfo
{
	std::string name = "";
	DataType dataType = DATA_TYPE_INVALID;
	uint32_t size = 0;
	std::string literalValue;
};

struct Variable
{
	Variable() = default;
	Variable(const Variable& rvalue) noexcept;
	Variable(const VariableInfo& rvalue);
	Variable(std::string value, std::string name);
	Variable(float rvalue);
	Variable(char rvalue);
	Variable(int rvalue);
	Variable(std::string rvalue);

	Variable& operator+=(const Variable& rvalue);
	Variable& operator-=(const Variable& rvalue);
	Variable& operator*=(const Variable& rvalue);
	Variable& operator/=(const Variable& rvalue);

	std::string operator[](size_t index);

	extern friend Variable operator+(Variable lvalue, const Variable& rvalue);
	extern friend Variable operator-(Variable lvalue, const Variable& rvalue);
	extern friend Variable operator/(Variable lvalue, const Variable& rvalue);
	extern friend Variable operator*(Variable lvalue, const Variable& rvalue);

	extern friend bool operator<(const Variable& lvalue, const Variable& rvalue);
	extern friend bool operator>(const Variable& lvalue, const Variable& rvalue);
	extern friend bool operator<=(const Variable& lvalue, const Variable& rvalue);
	extern friend bool operator>=(const Variable& lvalue, const Variable& rvalue);

	extern friend bool operator==(const Variable& lvalue, const Variable& rvalue);
	extern friend bool operator!=(const Variable& lvalue, const Variable& rvalue);

	operator float() const;
	operator int() const;
	operator char() const;
	operator std::string() const;

	Variable& operator=(float rvalue);
	Variable& operator=(char rvalue);
	Variable& operator=(int rvalue);
	Variable& operator=(std::string rvalue);

	std::string name = "";
	DataType type = DATA_TYPE_INVALID;
	DataType baseType = DATA_TYPE_INVALID; // only applies if the variable is a pointer

	std::string AsString();
	DataType GetDataType();
	void SetDataType(DataType type);

private:
	template<typename T> T GetDataAs() const
	{
		if (type != GetDataTypeTemplate<T>() || size != sizeof(T))
			throw std::runtime_error("Invalid cast: types do not match");
		return *(T*)data.data();
	}
	void Create(VariableInfo info);

	size_t size = 0;
	std::vector<uint8_t> data;
};

struct Instruction
{
	InstructionType type = INSTRUCTION_TYPE_INVALID;
	VariableInfo operand1{};
	VariableInfo operand2{};
	void* pNext          = nullptr;
};

inline bool IsInstructionSelfAssigning(Instruction& instruction)
{
	return (instruction.type == INSTRUCTION_TYPE_ASSIGN) && (instruction.operand1.name == instruction.operand2.name);
}