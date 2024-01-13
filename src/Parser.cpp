#include <unordered_map>
#include <stdexcept>
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "Behavior.hpp"

StackFrame Parser::simulationStackFrame;
std::unordered_map<std::string, FunctionInfo> Parser::functionInfos;
std::unordered_set<std::string> Parser::calledFunctions;

inline size_t GetNextInstanceOfLexeme(Lexeme lexeme, size_t index, const std::vector<Lexer::Token>& tokens)
{
	for (; index < tokens.size(); index++)
		if (tokens[index].lexeme == lexeme)
			return index;
	return 0;
}

inline size_t GetNextInstanceOfLexicalToken(LexicalToken token, size_t index, const std::vector<Lexer::Token>& tokens)
{
	for (; index < tokens.size(); index++)
		if (tokens[index].token == token)
			return index;
	return 0;
}

inline size_t GetIndexOfClosingCBracket(size_t index, const std::vector<Lexer::Token>& tokens)
{
	int timesReferenced = 0;
	for (; index < tokens.size(); index++) // this relies on the fact that the tokens vector starts on the opening cbracket
	{
		if (tokens[index].lexeme == LEXEME_OPEN_CBRACKET)
			timesReferenced++;
		else if (tokens[index].lexeme == LEXEME_CLOSE_CBRACKET)
			timesReferenced--;
		if (timesReferenced == 0)
			return index;
	}
	return 0;
}

inline InstructionType GetInstructionTypeFromLexemeOperator(Lexeme strOperator)
{
	switch (strOperator) // should also add the binary operators here
	{
	case LEXEME_PLUS:          return INSTRUCTION_TYPE_ADD;
	case LEXEME_MINUS:         return INSTRUCTION_TYPE_SUBTRACT;
	case LEXEME_MULTIPLY:      return INSTRUCTION_TYPE_MULTIPLY;
	case LEXEME_DIVIDE:        return INSTRUCTION_TYPE_DIVIDE;

	case LEXEME_EQUALS:        return INSTRUCTION_TYPE_ASSIGN;
	case LEXEME_IS:            return INSTRUCTION_TYPE_EQUAL;
	case LEXEME_ISNOT_SINGLE:
	case LEXEME_ISNOT:         return INSTRUCTION_TYPE_NOT_EQUAL;
	case LEXEME_GREATER:       return INSTRUCTION_TYPE_GREATER;
	case LEXEME_IS_OR_GREATER: return INSTRUCTION_TYPE_EQUAL_OR_GREATER;
	case LEXEME_LESS:          return INSTRUCTION_TYPE_LESS;
	case LEXEME_IS_OR_LESS:    return INSTRUCTION_TYPE_EQUAL_OR_LESS;
	}
	return INSTRUCTION_TYPE_INVALID;
}

inline DataType LexemeLiteralToDataType(Lexeme lexeme)
{
	switch (lexeme)
	{
	case LEXEME_LITERAL_CHAR:
		return DATA_TYPE_CHAR;
	case LEXEME_LITERAL_FLOAT:
		return DATA_TYPE_FLOAT;
	case LEXEME_LITERAL_INT:
		return DATA_TYPE_INT;
	case LEXEME_LITERAL_STRING:
		return DATA_TYPE_STRING;
	}
	return DATA_TYPE_INVALID;
}

std::vector<FunctionInfo> Parser::GetAllFunctionInfos(std::vector<Lexer::Token>& tokens)
{
	std::vector<FunctionInfo> ret;
	bool isExtern = false;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		switch (tokens[i].lexeme)
		{
		case LEXEME_EXTERN:
			isExtern = true;
			break;

		case LEXEME_DATATYPE_CHAR:
		case LEXEME_DATATYPE_FLOAT:
		case LEXEME_DATATYPE_INT:
		case LEXEME_DATATYPE_VOID:
		case LEXEME_DATATYPE_STRING:
			if (tokens[i + 2].lexeme == LEXEME_EQUALS || tokens[i + 2].lexeme == LEXEME_ENDLINE) // a var is declared with either "float var = ..." or "float var;", while a function is not
				break;

			size_t cParenIndex = GetNextInstanceOfLexeme(LEXEME_CLOSE_PARENTHESIS, i, tokens);
			std::vector<Lexer::Token> declarationTokens = { tokens.begin() + i, tokens.begin() + cParenIndex + 1 };
			FunctionInfo functionInfo = GetFunctionInfoFromTokens(declarationTokens);
			
			if (isExtern)
			{
				i = cParenIndex + 1;
				ret.push_back(functionInfo);
				functionInfos[functionInfo.name] = functionInfo;
				isExtern = false;
				break;
			}
			for (int j = 0; j < functionInfo.parameters.size(); j++)
				simulationStackFrame.Allocate(functionInfo.parameters[j]);

			size_t cBracketIndex = GetIndexOfClosingCBracket(cParenIndex + 1, tokens);
			std::vector<Lexer::Token> bodyTokens = { tokens.begin() + cParenIndex + 2, tokens.begin() + cBracketIndex };
			std::vector<std::vector<Lexer::Token>> test = GetAllScopesFromBody(bodyTokens);
			functionInfo.instructions = GetInstructionsFromScopes(test);
			simulationStackFrame.Clear();
			i = cBracketIndex;

			ret.push_back(functionInfo);
			functionInfos[functionInfo.name] = functionInfo;
			simulationStackFrame.Clear();
			break;
		}
	}
	return ret;
}

inline std::vector<Lexer::Token> GetTokensInsideParentheses(std::vector<Lexer::Token>& tokens, size_t& index)
{
	size_t parenReferenceCount = 0, beginIndex = index;
	for (; index < tokens.size(); index++)
	{
		if (tokens[index].lexeme == LEXEME_OPEN_PARENTHESIS)
			parenReferenceCount++;
		else if (tokens[index].lexeme == LEXEME_CLOSE_PARENTHESIS)
			parenReferenceCount--;
		if (parenReferenceCount == 0)
			return { tokens.begin() + beginIndex + 1, tokens.begin() + index };
	}
	return {};
}

void Parser::GetFunctionPushInstructions(const std::vector<Lexer::Token>& tokens, size_t& index, std::vector<Instruction>& ret)
{
	Instruction pushInst{};
	pushInst.type = INSTRUCTION_TYPE_PUSH;
	pushInst.operand1 = bufferParametersVar;
	int oParenReferenceCount = 0;

	std::vector<Lexer::Token> paramTokens;
	for (; index < tokens.size(); index++)
	{
		switch (tokens[index].lexeme)
		{
		case LEXEME_ENDLINE:
			return;

		case LEXEME_CLOSE_PARENTHESIS:
			if (oParenReferenceCount != 0)
				paramTokens.push_back(tokens[index]);
			oParenReferenceCount--;
			if (paramTokens.empty() || oParenReferenceCount > 0)
				break;
			[[fallthrough]];
		case LEXEME_COMMA:
		{
			GetInstructionsFromRValueRecursive(paramTokens, ret, bufferParametersVar);
			ret.back().type = INSTRUCTION_TYPE_PUSH;
			paramTokens.clear();
			break;
		}

		case LEXEME_OPEN_PARENTHESIS:
			oParenReferenceCount++;
			[[fallthrough]];
		default:
			paramTokens.push_back(tokens[index]);
			break;
		}
	}
}

void Parser::GetInstructionsFromRValueRecursive(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& destination, const VariableInfo& varToWriteTo)
{
	Instruction final{};
	final.type = INSTRUCTION_TYPE_ASSIGN;
	final.operand1 = floatCalculationVar;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		switch (tokens[i].token)
		{
		case LEXER_TOKEN_SEPERATOR:
		{
			if (tokens[i].lexeme != LEXEME_OPEN_PARENTHESIS)
				break;
			if (tokens[i].lexeme == LEXEME_ENDLINE)
			{
				final = { INSTRUCTION_TYPE_ASSIGN, varToWriteTo };
				break;
			}

			int parenReferenceCount = 0;
			std::vector<Lexer::Token> priorityTokens = GetTokensInsideParentheses(tokens, i);
			GetInstructionsFromRValueRecursive(priorityTokens, destination, varToWriteTo);
			destination.back().operand1 = floatCalculationVar;
			break;
		}
		case LEXER_TOKEN_OPERATOR:
			final.type = GetInstructionTypeFromLexemeOperator(tokens[i].lexeme);
			break;

		case LEXER_TOKEN_LITERAL:
		{
			final.operand2.dataType = LexemeLiteralToDataType(tokens[i].lexeme);
			final.operand2.literalValue = tokens[i].content;
			destination.push_back(final);
			final = { INSTRUCTION_TYPE_ASSIGN, floatCalculationVar };
			break;
		}

		case LEXER_TOKEN_IDENTIFIER:
		{
			if (functionInfos.count(tokens[i].content) <= 0)
			{
				if (!simulationStackFrame.Has(tokens[i].content))
					throw std::runtime_error("Syntax error: identifier \"" + tokens[i].content + "\" is undefined");

				final.operand2.dataType = simulationStackFrame.GetVariable(tokens[i].content).GetDataType();
				final.operand2.name = tokens[i].content;
				destination.push_back(final);
				final = { INSTRUCTION_TYPE_ASSIGN, floatCalculationVar };
				break;
			}
			// function call
			std::string functionName = tokens[i].content;
			i += 2; // skip over the '(' seperator
			GetFunctionPushInstructions(tokens, i, destination);

			Instruction callInst{};
			callInst.type = INSTRUCTION_TYPE_CALL;
			callInst.operand1 = { functionName, functionInfos[functionName].returnType };
			destination.push_back(callInst);

			final.operand2 = floatReturnVar;
			destination.push_back(final);
			final = { INSTRUCTION_TYPE_ASSIGN, floatCalculationVar };

			if (calledFunctions.count(functionName) == 0)
				calledFunctions.insert(functionName);
			break;
		}
		}
	}
	if (tokens.size() == 1)
	{
		destination.back().type = INSTRUCTION_TYPE_ASSIGN;
		destination.back().operand1 = varToWriteTo;
		return;
	}

	Instruction assignInst{};
	assignInst.type = INSTRUCTION_TYPE_ASSIGN;
	assignInst.operand1 = varToWriteTo;
	assignInst.operand2 = floatCalculationVar;
	if (!IsInstructionSelfAssigning(assignInst))
		destination.push_back(assignInst);
}

VariableInfo Parser::GetAssignVariableInfo(std::vector<Lexer::Token>& lvalue)
{
	return { lvalue.back().content, simulationStackFrame[lvalue.back().content].GetDataType() };
}

void Parser::ParseTokens(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed)
{
	for (; scopesTraversed < tokens.size(); scopesTraversed++)
	{
		ParseScope(tokens, ret, scopesTraversed);
	}
	for (size_t i = 0; i < ret.size(); i++) // lazily check all instructions (not the fastest)
		CheckInstructionIntegrity(ret[i], i);
}

void Parser::ParseScope(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed)
{
	size_t scopeIndex = scopesTraversed;
	for (size_t i = 0; i < tokens[scopeIndex].size(); i++)
	{
		switch (tokens[scopeIndex][i].token)
		{
		case LEXER_TOKEN_DATATYPE:
			i = ParseScopeDeclaration(tokens[scopeIndex], ret, i);
			break;

		case LEXER_TOKEN_OPERATOR: // this covers more than just the equals operators so not the best solution
			i = ParseScopeOperator(tokens[scopeIndex], ret, i);
			break;
		case LEXER_TOKEN_KEYWORD:
		{
			if (tokens[scopeIndex][i].lexeme == LEXEME_IF) // not so pretty if statements
				ProcessIfStatement(tokens, scopeIndex, scopesTraversed, i, ret);
			else if (tokens[scopeIndex][i].lexeme == LEXEME_WHILE)
				ProcessWhileStatement(tokens, scopeIndex, scopesTraversed, i, ret);

			else if (tokens[scopeIndex][i].lexeme == LEXEME_FOR)
				ProcessForStatement(tokens, scopeIndex, scopesTraversed, i, ret);

			if (tokens[scopeIndex][i].lexeme != LEXEME_RETURN)
				break;

			Instruction returnInst{};
			returnInst.type = INSTRUCTION_TYPE_RETURN;
			if (tokens[scopeIndex][i + 1].lexeme == LEXEME_ENDLINE) // no return value
			{
				ret.push_back(returnInst);
				break;
			}

			std::vector<Lexer::Token> returnValueTokens = { tokens[scopeIndex].begin() + i + 1, tokens[scopeIndex].begin() + GetNextInstanceOfLexeme(LEXEME_ENDLINE, i, tokens[scopeIndex]) };
			GetInstructionsFromRValueRecursive(returnValueTokens, ret, floatReturnVar);
			ret.push_back(returnInst);
			break;
		}
		case LEXER_TOKEN_IDENTIFIER:
			i = ParseScopeIdentifier(tokens[scopeIndex], ret, i);
			break;
		}
	}
	if (scopeIndex == 0 && ret.back().type != INSTRUCTION_TYPE_RETURN)
		ret.push_back({ INSTRUCTION_TYPE_RETURN });
}

size_t Parser::ParseScopeIdentifier(const std::vector<Lexer::Token>& tokens, std::vector<Instruction>& ret, size_t offset)
{
	if (functionInfos.count(tokens[offset].content) <= 0)
		return offset;

	std::string functionName = tokens[offset].content;
	offset += 2;
	GetFunctionPushInstructions(tokens, offset, ret);

	Instruction callInst{};
	callInst.type = INSTRUCTION_TYPE_CALL;
	callInst.operand1 = { functionName, functionInfos[functionName].returnType };
	ret.push_back(callInst);

	if (calledFunctions.count(functionName) == 0)
		calledFunctions.insert(functionName);
	return offset;
}

size_t Parser::ParseScopeOperator(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& ret, size_t offset)
{
	if (tokens[offset].lexeme == LEXEME_PLUSPLUS || tokens[offset].lexeme == LEXEME_MINUSMINUS)
		ReplaceTokensForSpecialOperator(offset, tokens);
	
	if (tokens[offset].content.back() != '=')
		return offset;

	size_t endIndex = GetNextInstanceOfLexeme(LEXEME_ENDLINE, offset, tokens);
	size_t otherEndIndex = GetNextInstanceOfLexeme(LEXEME_CLOSE_PARENTHESIS, offset, tokens); // this applies to conditional stuff like if's and whiles
	if (endIndex == 0 || (otherEndIndex != 0 && otherEndIndex < endIndex))
		endIndex = otherEndIndex;

	std::vector<Lexer::Token> lvalue = { tokens.begin(), tokens.begin() + offset };
	std::vector<Lexer::Token> rvalue = { tokens.begin() + offset + 1, tokens.begin() + endIndex };

	VariableInfo assignVar = GetAssignVariableInfo(lvalue);
	GetInstructionsFromRValueRecursive(rvalue, ret, floatCalculationVar);
	GetInstructionsForLexemeEqualsOperator(tokens[offset], assignVar, ret);
	return endIndex;
}

size_t Parser::ParseScopeDeclaration(const std::vector<Lexer::Token>& tokens, std::vector<Instruction>& ret, size_t offset)
{
	VariableInfo declVar{};
	declVar.dataType = (DataType)tokens[offset].lexeme;
	declVar.name = tokens[offset + 1].content;
	declVar.size = Sizeof(declVar.dataType);

	Instruction declareInst{};
	declareInst.type = INSTRUCTION_TYPE_DECLARE;
	declareInst.operand1 = declVar;
	ret.push_back(declareInst);

	simulationStackFrame.Allocate(declVar); // the functions stack frame gets simulated here in order to check for out of scope references etc. to prevent runtime errors

	if (tokens[offset + 2].lexeme == LEXEME_ENDLINE) // declaring without a value has a ; at the third token
		return offset + 2;

	size_t indexOfEndLine = GetNextInstanceOfLexeme(LEXEME_ENDLINE, offset, tokens);
	std::vector<Lexer::Token> rvalueTokens = { tokens.begin() + offset + 3, tokens.begin() + indexOfEndLine };
	GetInstructionsFromRValueRecursive(rvalueTokens, ret, declVar);
	return indexOfEndLine;
}

void Parser::ReplaceTokensForSpecialOperator(size_t index, std::vector<Lexer::Token>& tokens)
{
	switch (tokens[index].lexeme)
	{
	case LEXEME_PLUSPLUS:
		tokens[index].lexeme = LEXEME_PLUSEQUALS;
		tokens[index].content = "+=";
		break;
	case LEXEME_MINUSMINUS:
		tokens[index].lexeme = LEXEME_MINUSEQUALS;
		tokens[index].content = "-=";
		break;
	}
	Lexer::Token newToken{};
	newToken.token = LEXER_TOKEN_LITERAL;
	newToken.lexeme = LEXEME_LITERAL_INT;
	newToken.line = tokens[index].line;
	newToken.content = "1";

	tokens.insert(tokens.begin() + index + 1, newToken);
	return;
}

void Parser::GetInstructionsForLexemeEqualsOperator(const Lexer::Token& op, const VariableInfo& info, std::vector<Instruction>& instructions)
{
	VariableInfo varToReadFrom = floatCalculationVar;
	if (instructions.back().type == INSTRUCTION_TYPE_ASSIGN && instructions.back().operand1.name == varToReadFrom.name) // merge instructions that would otherwise just pass values along
	{
		varToReadFrom = instructions.back().operand2;
		instructions.pop_back();
	}
	CheckOperationIntegrity(op.lexeme, info, varToReadFrom, op.line);

	Instruction inst{};
	inst.operand1 = info;
	inst.operand2 = varToReadFrom;

	switch (op.lexeme)
	{
	case LEXEME_EQUALS:
		inst.type = INSTRUCTION_TYPE_ASSIGN;
		break;
	case LEXEME_PLUSEQUALS: // can be done better by having a map / function that takes for example PLUSEQUALS and returns PLUS
		inst.type = INSTRUCTION_TYPE_ADD;
		break;
	case LEXEME_MINUSEQUALS:
		inst.type = INSTRUCTION_TYPE_SUBTRACT;
		break;
	case LEXEME_MULTIPLYEQUALS:
		inst.type = INSTRUCTION_TYPE_MULTIPLY;
		break;
	case LEXEME_DIVIDEEQUALS:
		inst.type = INSTRUCTION_TYPE_DIVIDE;
		break;
	}
	instructions.push_back(inst);
}

void Parser::ProcessIfStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret)
{
	GetConditionInstructions(tokens[scopeIndex], i + 2, ret);

	Instruction jumpInst{};
	jumpInst.type = INSTRUCTION_TYPE_JUMP;
	ret.push_back(jumpInst);
	size_t jumpInstIndex = ret.size() - 1;

	Instruction pushScopeInst{};
	pushScopeInst.type = INSTRUCTION_TYPE_PUSH_SCOPE;
	ret.push_back(pushScopeInst);

	simulationStackFrame.IncrementScope();

	scopesTraversed++;
	ParseTokens(tokens, ret, scopesTraversed);
	scopesTraversed--;

	Instruction popScopeInst{};
	popScopeInst.type = INSTRUCTION_TYPE_POP_SCOPE;
	ret.push_back(popScopeInst);

	simulationStackFrame.DecrementScope();

	ret[jumpInstIndex].operand1 = { std::to_string(ret.size() - jumpInstIndex), DATA_TYPE_INT };
	for (; i < tokens[scopeIndex].size(); i++)
		if (tokens[scopeIndex][i].lexeme == LEXEME_CLOSE_PARENTHESIS)
			break;
	return;
}

void Parser::ProcessWhileStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret)
{
	size_t conditionIndex = ret.size();
	GetConditionInstructions(tokens[scopeIndex], i + 2, ret);

	Instruction jumpInst{};
	jumpInst.type = INSTRUCTION_TYPE_JUMP;
	size_t jumpInstIndex = conditionIndex + 1;
	ret.push_back(jumpInst);

	Instruction pushScopeInst{};
	pushScopeInst.type = INSTRUCTION_TYPE_PUSH_SCOPE;
	ret.push_back(pushScopeInst);
	simulationStackFrame.IncrementScope();

	scopesTraversed++;
	ParseTokens(tokens, ret, scopesTraversed);
	scopesTraversed--;

	Instruction popScopeInst{};
	popScopeInst.type = INSTRUCTION_TYPE_POP_SCOPE;
	ret.push_back(popScopeInst);
	simulationStackFrame.DecrementScope();

	Instruction loopBackInst{};
	loopBackInst.type = INSTRUCTION_TYPE_JUMP;
	loopBackInst.operand1 = { std::to_string((int64_t)conditionIndex - (int64_t)ret.size()), DATA_TYPE_INT };
	ret.push_back(loopBackInst);

	ret[jumpInstIndex].operand1 = { std::to_string(ret.size() - jumpInstIndex), DATA_TYPE_INT };
	for (; i < tokens[scopeIndex].size(); i++)
		if (tokens[scopeIndex][i].lexeme == LEXEME_CLOSE_PARENTHESIS)
			break;
	return;
}

void Parser::ProcessForStatement(std::vector<std::vector<Lexer::Token>>& tokens, size_t scopeIndex, size_t& scopesTraversed, size_t& i, std::vector<Instruction>& ret)
{
	size_t endOfPart1 = GetNextInstanceOfLexeme(LEXEME_ENDLINE, i, tokens[scopeIndex]);
	size_t endOfPart2 = GetNextInstanceOfLexeme(LEXEME_ENDLINE, endOfPart1 + 1, tokens[scopeIndex]);
	size_t endOfPart3 = GetNextInstanceOfLexeme(LEXEME_CLOSE_PARENTHESIS, endOfPart2 + 1, tokens[scopeIndex]);

	std::vector<Lexer::Token> part1 = { tokens[scopeIndex].begin() + i + 1, tokens[scopeIndex].begin() + endOfPart1 + 1 };
	std::vector<Lexer::Token> part2 = { tokens[scopeIndex].begin() + endOfPart1 + 1, tokens[scopeIndex].begin() + endOfPart2 + 1 };
	std::vector<Lexer::Token> part3 = { tokens[scopeIndex].begin() + endOfPart2 + 1, tokens[scopeIndex].begin() + endOfPart3 + 1 };

	Instruction pushScopeInst{};
	pushScopeInst.type = INSTRUCTION_TYPE_PUSH_SCOPE;
	ret.push_back(pushScopeInst);
	simulationStackFrame.IncrementScope();

	size_t holder = 0;
	FunctionBody forDecl = { part1 };
	ParseTokens(forDecl, ret, holder);
	ret.pop_back(); // ParseTokens adds a return instruction at the end of the return value if the scopesTraversed argument is 0, but thats not wanted here
	ret.push_back(pushScopeInst); // the for loop variable is declared in its own scope, since the code outside the for loop cant reach it but it has to stay alive in between loops
	simulationStackFrame.IncrementScope();
	holder = 0;

	size_t conditionIndex = ret.size();
	part2.back().lexeme = LEXEME_CLOSE_PARENTHESIS;
	GetConditionInstructions(part2, 0, ret);

	Instruction jumpInst{};
	jumpInst.type = INSTRUCTION_TYPE_JUMP;
	ret.push_back(jumpInst);
	size_t jumpInstIndex = conditionIndex + 1;

	scopesTraversed++;
	ParseTokens(tokens, ret, scopesTraversed);
	scopesTraversed--;

	Instruction popScopeInst{};
	popScopeInst.type = INSTRUCTION_TYPE_POP_SCOPE;

	FunctionBody endOfLoopComputations = { part3 };
	ParseTokens(endOfLoopComputations, ret, holder);
	ret.pop_back();

	Instruction loopBackInst{};
	loopBackInst.type = INSTRUCTION_TYPE_JUMP;
	loopBackInst.operand1 = { std::to_string((int64_t)conditionIndex - (int64_t)ret.size()), DATA_TYPE_INT };
	ret.push_back(loopBackInst);

	ret[jumpInstIndex].operand1 = { std::to_string(ret.size() - jumpInstIndex), DATA_TYPE_INT };

	ret.push_back(popScopeInst);
	ret.push_back(popScopeInst);
	simulationStackFrame.DecrementScope();
	simulationStackFrame.DecrementScope();

	for (; i < tokens[scopeIndex].size(); i++)
		if (tokens[scopeIndex][i].lexeme == LEXEME_CLOSE_PARENTHESIS)
			break;
	return;
}

void Parser::GetConditionInstructions(std::vector<Lexer::Token>& tokens, size_t index, std::vector<Instruction>& ret)
{
	size_t conditionMid = GetNextInstanceOfLexicalToken(LEXER_TOKEN_OPERATOR, index, tokens);
	size_t conditionEnd = GetNextInstanceOfLexeme(LEXEME_CLOSE_PARENTHESIS, index, tokens);

	Instruction compareInst{};
	compareInst.type = GetInstructionTypeFromLexemeOperator(tokens[conditionMid].lexeme);
	compareInst.operand1 = leftBoolValue;
	compareInst.operand2 = rightBoolValue;

	std::vector<Lexer::Token> lvalue = { tokens.begin() + index, tokens.begin() + conditionMid };
	std::vector<Lexer::Token> rvalue = { tokens.begin() + conditionMid + 1, tokens.begin() + conditionEnd };

	if (lvalue.size() > 1)
		GetInstructionsFromRValueRecursive(lvalue, ret, leftBoolValue);
	else if (lvalue[0].token == LEXER_TOKEN_LITERAL)
	{
		compareInst.operand1.dataType = LexemeLiteralToDataType(lvalue[0].lexeme);
		compareInst.operand1.literalValue = lvalue[0].token;
	}
	else
	{
		compareInst.operand1.name = lvalue[0].content;
		compareInst.operand1.dataType = simulationStackFrame[lvalue[0].content].GetDataType();
	}
	compareInst.operand1.size = Sizeof(compareInst.operand1.dataType);	

	if (rvalue.size() > 1)
		GetInstructionsFromRValueRecursive(rvalue, ret, rightBoolValue);
	else if (rvalue[0].token == LEXER_TOKEN_LITERAL)
	{
		compareInst.operand2.dataType = LexemeLiteralToDataType(rvalue[0].lexeme);
		compareInst.operand2.literalValue = rvalue[0].token;
	}
	else
	{
		compareInst.operand2.name = rvalue[0].content;
		compareInst.operand2.dataType = simulationStackFrame[rvalue[0].content].GetDataType();
	}
	compareInst.operand2.size = Sizeof(compareInst.operand2.dataType);
		
	ret.push_back(compareInst);
}

std::vector<Instruction> Parser::GetInstructionsFromScopes(std::vector<std::vector<Lexer::Token>>& tokens)
{
	std::vector<Instruction> ret;
	Instruction current;

	size_t index = 0;
	ParseTokens(tokens, ret, index);
	
	return ret;
}

inline void GetScopesRecursive(std::vector<Lexer::Token>& tokens, int& index, FunctionBody& ret)
{
	ret.push_back({});
	size_t assigningIndex = ret.size() - 1;
	for (; index < tokens.size(); index++)
	{
		switch (tokens[index].lexeme)
		{
		case LEXEME_OPEN_CBRACKET:
			index++;
			GetScopesRecursive(tokens, index, ret);
			break;
		case LEXEME_CLOSE_CBRACKET:
			return;
		default:
			ret[assigningIndex].push_back(tokens[index]);
			break;
		}
	}
}

FunctionBody Parser::GetAllScopesFromBody(std::vector<Lexer::Token>& tokens)
{
	FunctionBody ret = {};
	int i = 0;
	GetScopesRecursive(tokens, i, ret);

	return ret;
}

AbstractSyntaxTree Parser::CreateAST(std::vector<Lexer::Token>& tokens)
{
	AbstractSyntaxTree ret{};
	if (tokens.empty())
		return ret;
	CheckOpenCloseIntegrityPremature(tokens);

	std::vector<FunctionInfo> infos = GetAllFunctionInfos(tokens);
	for (FunctionInfo info : infos)
	{
		if (Behavior::removeUnusedSymbols && info.name != Behavior::entryPoint && calledFunctions.count(info.name) == 0)
		{
			if (Behavior::verbose)
				std::cout << "found unused function " << info.name << ", removing...\n";
			continue;
		}

		Function* fnPtr = new Function(info);
		ret.functions.insert(fnPtr);
		if (info.name == Behavior::entryPoint)
			ret.entryPoint = fnPtr;
	}
	return ret;
}

FunctionInfo Parser::GetFunctionInfoFromTokens(std::vector<Lexer::Token>& tokens)
{
	if (tokens.size() < 4)
		throw std::runtime_error("Not enough tokens"); // functions always have more than 3 tokens

	FunctionInfo ret{}; 
	VariableInfo currentVarInfo{};
	bool recordParams = false, isExtern = false;
	for (int i = 0; i < tokens.size(); i++)
	{
		switch (tokens[i].lexeme)
		{
		case LEXEME_OPEN_PARENTHESIS:
			recordParams = true;
			break;
		case LEXEME_CLOSE_PARENTHESIS:
			if (currentVarInfo.name != "")
				ret.parameters.push_back(currentVarInfo);
			recordParams = false;
			break;

		case LEXEME_DATATYPE_CHAR:
		case LEXEME_DATATYPE_FLOAT:
		case LEXEME_DATATYPE_INT:
		case LEXEME_DATATYPE_VOID:
		case LEXEME_DATATYPE_STRING:
			if (recordParams)
			{
				currentVarInfo.dataType = (DataType)tokens[i].lexeme;
				currentVarInfo.size = Sizeof(currentVarInfo.dataType);
			}
			else
				ret.returnType = (DataType)tokens[i].lexeme;
			break;

		case LEXEME_IDENTIFIER:
			if (recordParams)
				currentVarInfo.name = tokens[i].content;
			else
				ret.name = tokens[i].content;
			break;

		case LEXEME_COMMA:
			if (!recordParams)
				break;
			ret.parameters.push_back(currentVarInfo);
			currentVarInfo = {};
			break;

		case LEXEME_EXTERN:
			isExtern = true;
			break;

		case LEXEME_ENDLINE: // extern functions dont have a body declaration
			if (isExtern)
				return ret;
			break;
		}
	}
	return ret;
}

inline bool IsOperatorBinary(Lexeme op)
{
	switch (op)
	{
	case LEXEME_PLUS:
	case LEXEME_PLUSEQUALS:
	case LEXEME_MINUS:
	case LEXEME_MINUSEQUALS:
	case LEXEME_MULTIPLY:
	case LEXEME_MULTIPLYEQUALS:
	case LEXEME_DIVIDE:
	case LEXEME_DIVIDEEQUALS:
		return true;
	}
	return false;
}

void Parser::CheckOpenCloseIntegrityPremature(const std::vector<Lexer::Token>& tokens)
{
	size_t closeParenCount = 0, openParenCount = 0;
	size_t closeCBracketCount = 0, openCBracketCount = 0;
	size_t closeSBracketCount = 0, openSBracketCount = 0;

	for (size_t i = 0; i < tokens.size(); i++)
	{
		switch (tokens[i].lexeme)
		{
		case LEXEME_CLOSE_PARENTHESIS:
			closeParenCount++;
			break;
		case LEXEME_OPEN_PARENTHESIS:
			openParenCount++;
			break;
		case LEXEME_CLOSE_CBRACKET:
			closeCBracketCount++;
			break;
		case LEXEME_OPEN_CBRACKET:
			openCBracketCount++;
			break;
		case LEXEME_CLOSE_SBRACKET:
			closeSBracketCount++;
			break;
		case LEXEME_OPEN_SBRACKET:
			openSBracketCount++;
			break;
		}
	}
	if (closeParenCount != openParenCount)
		throw std::runtime_error("Syntax error: the count of '(' and ')' is not equal");
	if (closeCBracketCount != openCBracketCount)
		throw std::runtime_error("Syntax error: the count of '{' and '}' is not equal");
	if (closeSBracketCount != openSBracketCount)
		throw std::runtime_error("Syntax error: the count of '[' and ']' is not equal");
}

inline bool OneVariableIsString(const VariableInfo& lvalue, const VariableInfo& rvalue)
{
	return (DataTypeIsString(lvalue.dataType) && !DataTypeIsString(rvalue.dataType)) || (!DataTypeIsString(lvalue.dataType) && DataTypeIsString(rvalue.dataType));
}

void Parser::CheckOperationIntegrity(const Lexeme op, const VariableInfo& lvalue, const VariableInfo& rvalue, int line)
{
	if (lvalue.dataType == LEXEME_DATATYPE_VOID || rvalue.dataType == DATA_TYPE_VOID || op == LEXEME_INVALID)
		return; // voids cannot be checked

	if (Behavior::disableImplicitConversion && (lvalue.dataType != rvalue.dataType))
		throw std::runtime_error("Invalid conversion (from " + DataTypeToInternalTypeString(rvalue.dataType) + " to " + DataTypeToInternalTypeString(lvalue.dataType) + ") at line " + std::to_string(line) + ": implicit conversions are disabled (disableImplicitConversion && lvalue.dataType != rvalue.dataType)\nremove the - disable_implicit_conversion argument to remove this error");

	if (OneVariableIsString(lvalue, rvalue)) // no operator can be used if one is a string and the other is not (regardless of order)
		throw std::runtime_error("Syntax error at line " + std::to_string(line) + ": cannot use a non-string value (type: " + DataTypeToInternalTypeString(lvalue.dataType) + ") with a string");

	if (op == LEXEME_EQUALS)
		return;

	if ((op == LEXEME_PLUS || op == LEXEME_PLUSEQUALS) && OneVariableIsString(lvalue, rvalue)) // strings can only be added to each other if both vars are a string
		throw std::runtime_error("Syntax error at line " + std::to_string(line) + ": cannot add a non-string value (type: " + DataTypeToInternalTypeString(lvalue.dataType) + ") to a string");
	if (IsOperatorBinary(op) && (op != LEXEME_PLUS && op != LEXEME_PLUSEQUALS) && (DataTypeIsString(lvalue.dataType) || DataTypeIsString(rvalue.dataType))) // the only valid binary operator for strings is '+(=)'
		throw std::runtime_error("Syntax error at line " + std::to_string(line) + ": cannot use current operator, it is not valid for strings");

	if (lvalue.dataType == DATA_TYPE_USERTYPE || rvalue.dataType == DATA_TYPE_USERTYPE)
		throw std::runtime_error("Syntax error at line " + std::to_string(line) + ": operators cannot be used on user types");
}

void Parser::CheckInstructionIntegrity(const Instruction& instruction, int index)
{
	const Lexeme op = InstructionTypeToLexemeOperator(instruction.type);
	CheckOperationIntegrity(op, instruction.operand1, instruction.operand2, index);
}

bool Parser::IsFunctionDeclaration(std::vector<Lexer::Token>& tokens)
{
	if (tokens.size() < 4)
		return false;
	return tokens[0].token == LEXER_TOKEN_DATATYPE && tokens[1].token == LEXER_TOKEN_IDENTIFIER && tokens[2].lexeme == LEXEME_OPEN_PARENTHESIS && tokens.back().lexeme == LEXEME_CLOSE_PARENTHESIS;
}

bool Parser::DoesFunctionExist(std::string name)
{
	return calledFunctions.count(name) > 0;
}

Lexeme Parser::InstructionTypeToLexemeOperator(InstructionType type)
{
	switch (type)
	{
	case INSTRUCTION_TYPE_ADD:              return LEXEME_PLUS;
	case INSTRUCTION_TYPE_SUBTRACT:         return LEXEME_MINUS;
	case INSTRUCTION_TYPE_MULTIPLY:         return LEXEME_MULTIPLY;
	case INSTRUCTION_TYPE_DIVIDE:           return LEXEME_DIVIDE;
	case INSTRUCTION_TYPE_EQUAL_OR_GREATER: return LEXEME_IS_OR_GREATER;
	case INSTRUCTION_TYPE_EQUAL:            return LEXEME_IS;
	case INSTRUCTION_TYPE_NOT_EQUAL:        return LEXEME_ISNOT;
	case INSTRUCTION_TYPE_EQUAL_OR_LESS:    return LEXEME_IS_OR_LESS;
	case INSTRUCTION_TYPE_LESS:             return LEXEME_LESS;
	}
	return LEXEME_INVALID;
}