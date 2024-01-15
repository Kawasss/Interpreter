#pragma once
#include <unordered_set>
#include <unordered_map>
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

struct StructInfo
{
	std::string name;
	size_t size;
	std::vector<VariableInfo> variables;
};

class Parser
{
public:
	static AbstractSyntaxTree CreateAST(std::vector<Lexer::Token>& tokens);
	static std::vector<FunctionInfo> GetAllFunctionInfos(std::vector<Lexer::Token>& tokens);
	static void ParseTokens(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed);

	static std::vector<Instruction> GetInstructionsFromScopes(std::vector<std::vector<Lexer::Token>>& tokens);

	static void GetInstructionsFromRValueRecursive(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& destination, const VariableInfo& varToWriteTo);

	static VariableInfo GetAssignVariableInfo(std::vector<Lexer::Token>& lvalue);
	static FunctionInfo GetFunctionInfoFromTokens(std::vector<Lexer::Token>& tokens);
	static bool IsFunctionDeclaration(std::vector<Lexer::Token>& tokens);
	static bool DoesFunctionExist(std::string name);

private:
	static void ParseScope(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed);
	static FunctionBody GetAllScopesFromBody(std::vector<Lexer::Token>& tokens);

	static size_t ParseScopeDeclaration(const std::vector<Lexer::Token>& tokens, std::vector<Instruction>& ret, size_t offset); // returns the index of where it left of
	static size_t ParseScopeOperator(std::vector<Lexer::Token>& tokens,          std::vector<Instruction>& ret, size_t offset);
	static size_t ParseScopeIdentifier(const std::vector<Lexer::Token>& tokens,  std::vector<Instruction>& ret, size_t offset);

	static void GetFunctionPushInstructions(const std::vector<Lexer::Token>& tokens, size_t& index, std::vector<Instruction>& ret);
	static void GetConditionInstructions(std::vector<Lexer::Token>& tokens, size_t index, std::vector<Instruction>& ret);

	static void ProcessKeyword(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessIfStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessWhileStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessForStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret);
	static void ProcessReturnStatement(std::vector<Lexer::Token>& tokens, size_t& i, std::vector<Instruction>& ret);
	static void ProcessLocationOfOperator(const std::vector<Lexer::Token>& tokens, size_t& i, VariableInfo& info);
	static void ProcessDereferenceOperator(const std::vector<Lexer::Token>& tokens, size_t& i, Instruction& instruction);

	static void GetInstructionsForLexemeEqualsOperator(const Lexer::Token& op, const VariableInfo& varToWriteTo, std::vector<Instruction>& instructions);

	static void CheckOpenCloseIntegrityPremature(const std::vector<Lexer::Token>& tokens);
	static void CheckOperationIntegrity(const Lexeme op, const VariableInfo& lvalue, const VariableInfo& rvalue, size_t line);
	static void CheckInstructionIntegrity(const Instruction& instruction, size_t index);
	static Lexeme InstructionTypeToLexemeOperator(InstructionType type);

	static void ReplaceTokensForSpecialOperator(size_t index, std::vector<Lexer::Token>& tokens);

	static StackFrame simulationStackFrame;
	static std::unordered_map<std::string, FunctionInfo> functionInfos;
	static std::unordered_set<std::string> calledFunctions;
};