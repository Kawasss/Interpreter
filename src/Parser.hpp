#pragma once
#include <unordered_set>
#include <optional>
#include "common.hpp"
#include "Lexer.hpp"
#include "Function.hpp"

struct AbstractSyntaxTree
{
	std::unordered_set<Function*> functions;
	Function* entryPoint = nullptr;
};

class Parser
{
public:
	static AbstractSyntaxTree CreateAST(std::vector<Lexer::Token>& tokens);
	static std::vector<FunctionInfo> GetAllFunctionInfos(std::vector<Lexer::Token>& tokens);
	static void ParseLine(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& ret);
	static std::vector<Instruction> GetInstructions(std::vector<Lexer::Token>& tokens);
	static void GetInstructionsFromRValue(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& destination, const VariableInfo& varToWriteTo);

	static VariableInfo GetAssignVariableInfo(std::vector<Lexer::Token>& lvalue);
	static FunctionInfo GetFunctionInfoFromTokens(std::vector<Lexer::Token>& tokens);
	static bool IsFunctionDeclaration(std::vector<Lexer::Token>& tokens);

private:
	static std::vector<std::vector<Lexer::Token>> DivideByEndLine(std::vector<Lexer::Token>& tokens);
	static std::vector<Instruction> GetDefinitionInstructions(VariableInfo& info, VariableInfo& startValue);
	static std::vector<Instruction> GetCallFunctionInstructions(FunctionInfo& info, std::vector<VariableInfo>& parameterVars);
	static void GetFunctionPushInstructions(std::vector<Lexer::Token>& tokens, size_t& index, std::vector<Instruction>& ret);

	static std::unordered_map<Lexeme, InstructionType> lexemeToInstructionType;
	static std::unordered_map<std::string, FunctionInfo> functionInfos;
};