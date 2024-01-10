#pragma once
#include <unordered_set>
#include "common.hpp"
#include "Lexer.hpp"
#include "Function.hpp"
#include "StackFrame.hpp"

typedef std::vector<std::vector<Lexer::Token>> FunctionBody;

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
	static void ParseTokens(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed);

	static std::vector<Instruction> GetInstructionsFromScopes(std::vector<std::vector<Lexer::Token>>& tokens);
	static std::vector<Instruction> GetInstructionsFromBody(FunctionBody& body);

	static void GetInstructionsFromRValue(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& destination, const VariableInfo& varToWriteTo);

	static VariableInfo GetAssignVariableInfo(std::vector<Lexer::Token>& lvalue);
	static FunctionInfo GetFunctionInfoFromTokens(std::vector<Lexer::Token>& tokens);
	static bool IsFunctionDeclaration(std::vector<Lexer::Token>& tokens);

private:
	static std::vector<std::vector<Lexer::Token>> DivideByEndLine(std::vector<Lexer::Token>& tokens);
	static FunctionBody GetAllScopesFromBody(std::vector<Lexer::Token>& tokens);

	static std::vector<Instruction> GetDefinitionInstructions(VariableInfo& info, VariableInfo& startValue);
	static std::vector<Instruction> GetCallFunctionInstructions(FunctionInfo& info, std::vector<VariableInfo>& parameterVars);
	static void GetFunctionPushInstructions(std::vector<Lexer::Token>& tokens, size_t& index, std::vector<Instruction>& ret);
	static void GetConditionInstructions(std::vector<Lexer::Token>& tokens, size_t index, std::vector<Instruction>& ret);

	static void ProcessIfStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessWhileStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessForStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);

	static void GetInstructionsForLexemeEqualsOperator(Lexeme op, const VariableInfo& varToWriteTo, std::vector<Instruction>& instructions);

	static void CheckOpenCloseIntegrityPremature(const std::vector<Lexer::Token>& tokens);
	static void CheckOperationIntegrity(const Lexeme op, const VariableInfo& lvalue, const VariableInfo& rvalue);
	static void CheckInstructionIntegrity(const Instruction& instruction);
	static Lexeme InstructionTypeToLexemeOperator(InstructionType type);

	static StackFrame simulationStackFrame;
	static std::unordered_map<std::string, FunctionInfo> functionInfos;
};