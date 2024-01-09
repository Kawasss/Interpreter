#include <unordered_map>
#include <stdexcept>
#include "Parser.hpp"
#include "Interpreter.hpp"

size_t Parser::sizeOfNextScope = 0;

std::unordered_map<Lexeme, InstructionType> Parser::lexemeToInstructionType =
{
	{ LEXEME_PLUS, INSTRUCTION_TYPE_ADD }, { LEXEME_MINUS, INSTRUCTION_TYPE_SUBTRACT }, { LEXEME_MULTIPLY, INSTRUCTION_TYPE_MULTIPLY },
	{ LEXEME_DIVIDE, INSTRUCTION_TYPE_DIVIDE }
};

std::unordered_map<std::string, FunctionInfo> Parser::functionInfos;

inline size_t GetNextInstanceOfLexeme(Lexeme lexeme, size_t index, std::vector<Lexer::Token>& tokens)
{
	for (; index < tokens.size(); index++)
		if (tokens[index].lexeme == lexeme)
			return index;
	return 0;
}

inline size_t GetNextInstanceOfLexicalToken(LexicalToken token, size_t index, std::vector<Lexer::Token>& tokens)
{
	for (; index < tokens.size(); index++)
		if (tokens[index].token == token)
			return index;
	return 0;
}

inline size_t GetIndexOfClosingCBracket(size_t index, std::vector<Lexer::Token>& tokens)
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

inline InstructionType GetInstructionTypeFromOperator(char strOperator, bool isFollowedByEquals) // only get the first part of a potential longer operator like "+=" since a char is way easier to copy than a string
{
	switch (strOperator) // should also add the binary operators here
	{
	case '=':
		return isFollowedByEquals ? INSTRUCTION_TYPE_EQUAL : INSTRUCTION_TYPE_ASSIGN;
	case '!':
		return isFollowedByEquals ? INSTRUCTION_TYPE_NOT_EQUAL : INSTRUCTION_TYPE_NOT_EQUAL; // single not equal (ternary not needed, but is kept from maybe future instruction)
	}
}

inline DataType LexemeLiteralToDataType(Lexeme lexeme)
{
	switch (lexeme)
	{
	case LEXEME_LITERAL_CHAR:
		return DATA_TYPE_CHAR_CONSTANT;
	case LEXEME_LITERAL_FLOAT:
		return DATA_TYPE_FLOAT_CONSTANT;
	case LEXEME_LITERAL_INT:
		return DATA_TYPE_INT_CONSTANT;
	case LEXEME_LITERAL_STRING:
		return DATA_TYPE_STRING_CONSTANT;
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

			size_t cBracketIndex = GetIndexOfClosingCBracket(cParenIndex + 1, tokens);
			std::vector<Lexer::Token> bodyTokens = { tokens.begin() + cParenIndex + 2, tokens.begin() + cBracketIndex };
			std::vector<std::vector<Lexer::Token>> test = GetAllScopesFromBody(bodyTokens);
			functionInfo.instructions = GetInstructionsFromScopes(test);
			i = cBracketIndex;

			ret.push_back(functionInfo);
			functionInfos[functionInfo.name] = functionInfo;
			break;
		}
	}
	return ret;
}

inline std::vector<Lexer::Token> GetTokensInsideParentheses(std::vector<Lexer::Token>& tokens, size_t& index)
{
	int parenReferenceCount = 0, beginIndex = index;
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

void Parser::GetFunctionPushInstructions(std::vector<Lexer::Token>& tokens, size_t& index, std::vector<Instruction>& ret)
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
			GetInstructionsFromRValue(paramTokens, ret, bufferParametersVar);
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

void Parser::GetInstructionsFromRValue(std::vector<Lexer::Token>& tokens, std::vector<Instruction>& destination, const VariableInfo& varToWriteTo)
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
			GetInstructionsFromRValue(priorityTokens, destination, varToWriteTo);
			destination.back().operand1 = floatCalculationVar;
			break;
		}
		case LEXER_TOKEN_OPERATOR:
			final.type = lexemeToInstructionType[tokens[i].lexeme];
			break;

		case LEXER_TOKEN_LITERAL:
		{
			final.operand2.dataType = LexemeLiteralToDataType(tokens[i].lexeme);
			final.operand2.name = tokens[i].content;
			destination.push_back(final);
			final = { INSTRUCTION_TYPE_ASSIGN, floatCalculationVar };
			break;
		}

		case LEXER_TOKEN_IDENTIFIER:
		{
			if (functionInfos.count(tokens[i].content) <= 0)
			{
				final.operand2.dataType = DATA_TYPE_VOID; // cant get variable types from here
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
			callInst.operand1.name = functionName;
			destination.push_back(callInst);

			final.operand2 = floatReturnVar;
			destination.push_back(final);
			final = { INSTRUCTION_TYPE_ASSIGN, floatCalculationVar };

			break;
		}
		}
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
	return { lvalue.back().content, DATA_TYPE_VOID }; // this should get the proper variable for the assign instruction, but this works for now
}

std::vector<Instruction> Parser::GetDefinitionInstructions(VariableInfo& info, VariableInfo& startValue)
{
	std::vector<Instruction> ret;

	Instruction declareInst{};
	declareInst.type = INSTRUCTION_TYPE_DECLARE;
	declareInst.operand1 = info;
	ret.push_back(declareInst);

	if (startValue.dataType == DATA_TYPE_INVALID)
		return ret;

	Instruction assignInst;
	assignInst.type = INSTRUCTION_TYPE_ASSIGN;
	assignInst.operand1 = info;
	assignInst.operand2 = startValue;
	ret.push_back(assignInst);

	return ret;
}

std::vector<Instruction> Parser::GetCallFunctionInstructions(FunctionInfo& info, std::vector<VariableInfo>& parameterVars)
{
	if (parameterVars.size() != info.parameters.size())
		throw std::runtime_error("Incorrect amount of parameters");

	std::vector<Instruction> ret;
	for (int i = 0; i < info.parameters.size(); i++) // this does not properly define the variables in the correct stack
	{
		Instruction pushInst{};
		pushInst.type = INSTRUCTION_TYPE_PUSH;
		pushInst.operand1 = bufferParametersVar;
		pushInst.operand2 = parameterVars[i];
		ret.push_back(pushInst);
	}
	return ret;
}

void Parser::ParseTokens(FunctionBody& tokens, std::vector<Instruction>& ret, size_t& scopesTraversed)
{
	for (; scopesTraversed < tokens.size(); scopesTraversed++)
	{
		size_t scopeIndex = scopesTraversed;
		for (size_t i = 0; i < tokens[scopeIndex].size(); i++)
		{
			switch (tokens[scopeIndex][i].token)
			{
			case LEXER_TOKEN_DATATYPE:
			{
				VariableInfo declVar{};
				declVar.dataType = (DataType)tokens[scopeIndex][i].lexeme;
				declVar.name = tokens[scopeIndex][i + 1].content;

				Instruction declareInst{};
				declareInst.type = INSTRUCTION_TYPE_DECLARE;
				declareInst.operand1 = declVar;
				ret.push_back(declareInst);

				if (tokens[scopeIndex][i + 2].lexeme == LEXEME_ENDLINE) // declaring without a value has a ; at the third token
					break;

				int indexOfEndLine = GetNextInstanceOfLexeme(LEXEME_ENDLINE, i, tokens[scopeIndex]);
				std::vector<Lexer::Token> rvalueTokens = { tokens[scopeIndex].begin() + i + 3, tokens[scopeIndex].begin() + indexOfEndLine };
				GetInstructionsFromRValue(rvalueTokens, ret, declVar);
				i = indexOfEndLine;
				break;
			}

			case LEXER_TOKEN_OPERATOR: // this covers more than just the equals operators so not the best solution
			{
				if (tokens[scopeIndex][i].content.back() != '=')
					break;

				std::vector<Lexer::Token> lvalue = { tokens[scopeIndex].begin(), tokens[scopeIndex].begin() + i };
				std::vector<Lexer::Token> rvalue = { tokens[scopeIndex].begin() + i + 1, tokens[scopeIndex].end() };

				VariableInfo assignVar = GetAssignVariableInfo(lvalue);
				GetInstructionsFromRValue(rvalue, ret, assignVar);
				i = GetNextInstanceOfLexeme(LEXEME_ENDLINE, i, tokens[scopeIndex]);
				break;
			}
			case LEXER_TOKEN_KEYWORD:
			{
				if (tokens[scopeIndex][i].lexeme == LEXEME_IF) // seperate into function if it works
				{
					int conditionStart = i;
					int conditionMid = GetNextInstanceOfLexicalToken(LEXER_TOKEN_OPERATOR, i, tokens[scopeIndex]);
					int conditionEnd = GetNextInstanceOfLexeme(LEXEME_CLOSE_PARENTHESIS, i, tokens[scopeIndex]);

					Instruction compareInst{};
					compareInst.type = GetInstructionTypeFromOperator(tokens[scopeIndex][conditionMid].content[0], tokens[scopeIndex][conditionMid].content[1] == '=');
					compareInst.operand1 = leftBoolValue;
					compareInst.operand2 = rightBoolValue;

					std::vector<Lexer::Token> lvalue = { tokens[scopeIndex].begin() + i + 2, tokens[scopeIndex].begin() + conditionMid };
					std::vector<Lexer::Token> rvalue = { tokens[scopeIndex].begin() + conditionMid + 1, tokens[scopeIndex].begin() + conditionEnd };

					if (lvalue.size() > 1)
						GetInstructionsFromRValue(lvalue, ret, leftBoolValue);
					else
						compareInst.operand1 = { lvalue[0].content, LexemeLiteralToDataType(lvalue[0].lexeme) };

					if (rvalue.size() > 1)
						GetInstructionsFromRValue(rvalue, ret, rightBoolValue);
					else
						compareInst.operand2 = { rvalue[0].content, LexemeLiteralToDataType(rvalue[0].lexeme) };

					Instruction jumpInst{};
					jumpInst.type = INSTRUCTION_TYPE_JUMP;
					

					ret.push_back(compareInst);
					ret.push_back(jumpInst);
					size_t jumpInstIndex = ret.size() - 1;

					scopesTraversed++;
					ParseTokens(tokens, ret, scopesTraversed);

					ret[jumpInstIndex].operand1 = { std::to_string(ret.size() - jumpInstIndex), DATA_TYPE_INT_CONSTANT };
					break;
				}

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
				GetInstructionsFromRValue(returnValueTokens, ret, floatReturnVar);
				ret.push_back(returnInst);
				break;
			}
			case LEXER_TOKEN_IDENTIFIER:
				if (functionInfos.count(tokens[scopeIndex][i].content) <= 0)
					break;

				std::string functionName = tokens[scopeIndex][i].content;
				i += 2;
				GetFunctionPushInstructions(tokens[scopeIndex], i, ret);

				Instruction callInst{};
				callInst.type = INSTRUCTION_TYPE_CALL;
				callInst.operand1.name = functionName;
				ret.push_back(callInst);
				break;
			}
		}
	}
}

std::vector<Instruction> Parser::GetInstructionsFromScopes(std::vector<std::vector<Lexer::Token>>& tokens)
{
	std::vector<Instruction> ret;
	Instruction current;

	//for (int i = 0; i < tokens.size(); i++)
	//{
	//	if (i != tokens.size() - 1)
	//	{
	//		std::vector<Instruction> holder;
	//		ParseTokens(tokens[(size_t)i + 1], holder); // this is REALLY bad and slow and should NOT stay this way, this is used to calculate amount of instruction that have to be skipped for a false condition check (for an if)
	//		sizeOfNextScope = holder.size();
	//	}
	//		
	//	ParseTokens(tokens[i], ret);
	//}
	size_t index = 0;
	ParseTokens(tokens, ret, index);
	
	return ret;
}

std::vector<Instruction> Parser::GetInstructionsFromBody(FunctionBody& body)
{
	std::vector<Instruction> ret;
	/*for (std::vector<Lexer::Token>& tokens : body)
	{
		std::vector<std::vector<Lexer::Token>> tokensPerLine = DivideByEndLine(tokens);
		for (std::vector<Lexer::Token> tokenLine : tokensPerLine)
			ParseTokens(tokenLine, ret);
	}*/
	return ret;
}

std::vector<std::vector<Lexer::Token>> Parser::DivideByEndLine(std::vector<Lexer::Token>& tokens)
{
	std::vector<std::vector<Lexer::Token>> ret;
	std::vector<Lexer::Token> current;
	for (int i = 0; i < tokens.size(); i++)
	{
		current.push_back(tokens[i]);
		if (tokens[i].lexeme != LEXEME_ENDLINE)
			continue;

		ret.push_back(current);
		current.clear();
	}
	return ret;
}

inline void GetScopesRecursive(std::vector<Lexer::Token>& tokens, int& index, FunctionBody& ret)
{
	ret.push_back({});
	int assigningIndex = ret.size() - 1;
	for (; index < tokens.size(); index++)
	{
		switch (tokens[index].lexeme)
		{
		case LEXEME_OPEN_CBRACKET:
			index++;
			GetScopesRecursive(tokens, index, ret);
			break;
		case LEXEME_CLOSE_CBRACKET:
			//index++;
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

	std::vector<FunctionInfo> infos = GetAllFunctionInfos(tokens);
	for (FunctionInfo info : infos)
	{
		Function* fnPtr = new Function(info);
		ret.functions.insert(fnPtr);
		if (info.name == "main")
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
				currentVarInfo.dataType = (DataType)tokens[i].lexeme;
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

bool Parser::IsFunctionDeclaration(std::vector<Lexer::Token>& tokens)
{
	if (tokens.size() < 4)
		return false;
	return tokens[0].token == LEXER_TOKEN_DATATYPE && tokens[1].token == LEXER_TOKEN_IDENTIFIER && tokens[2].lexeme == LEXEME_OPEN_PARENTHESIS && tokens.back().lexeme == LEXEME_CLOSE_PARENTHESIS;
}