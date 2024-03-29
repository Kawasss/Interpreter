#include "Lexer.hpp"

std::unordered_map<std::string, LexicalToken> Lexer::stringToLexicalToken =
{
	{ "=",      LEXER_TOKEN_OPERATOR  }, { "-",      LEXER_TOKEN_OPERATOR  }, { "+",       LEXER_TOKEN_OPERATOR  }, { "/",      LEXER_TOKEN_OPERATOR  }, { "*",    LEXER_TOKEN_OPERATOR  },
	{ "-=",     LEXER_TOKEN_OPERATOR  }, { "+=",     LEXER_TOKEN_OPERATOR  }, { "/=",      LEXER_TOKEN_OPERATOR  }, { "*=",     LEXER_TOKEN_OPERATOR  }, { "==",   LEXER_TOKEN_OPERATOR  },
	{ " ",      LEXER_TOKEN_SEPERATOR }, { ";",      LEXER_TOKEN_SEPERATOR }, { ".",       LEXER_TOKEN_SEPERATOR }, { ",",      LEXER_TOKEN_SEPERATOR }, { "\n",   LEXER_TOKEN_SEPERATOR },
	{ "{",      LEXER_TOKEN_SEPERATOR }, { "}",      LEXER_TOKEN_SEPERATOR }, { "(",       LEXER_TOKEN_SEPERATOR }, { ")",      LEXER_TOKEN_SEPERATOR }, { "!=",   LEXER_TOKEN_OPERATOR  },
	{ "float",  LEXER_TOKEN_DATATYPE  }, { "int",    LEXER_TOKEN_DATATYPE  }, { "char",    LEXER_TOKEN_DATATYPE  }, { "struct", LEXER_TOKEN_DATATYPE  }, { "void", LEXER_TOKEN_DATATYPE  },
	{ "return", LEXER_TOKEN_KEYWORD   }, { "extern", LEXER_TOKEN_KEYWORD   }, { "string",  LEXER_TOKEN_DATATYPE  }, { "import", LEXER_TOKEN_KEYWORD   }, { "if",   LEXER_TOKEN_KEYWORD   },
	{ "while",  LEXER_TOKEN_KEYWORD   }, { ">",      LEXER_TOKEN_OPERATOR  }, { "<",       LEXER_TOKEN_OPERATOR  }, { ">=",     LEXER_TOKEN_OPERATOR  }, { "<=",   LEXER_TOKEN_OPERATOR  },
	{ "for",    LEXER_TOKEN_KEYWORD   }, { "[",      LEXER_TOKEN_OPERATOR  }, { "]",       LEXER_TOKEN_OPERATOR  }, { "++",     LEXER_TOKEN_OPERATOR  }, { "--",   LEXER_TOKEN_OPERATOR  },
	{ "&",      LEXER_TOKEN_OPERATOR  }, { "uint64", LEXER_TOKEN_DATATYPE  }, { "typedef", LEXER_TOKEN_KEYWORD   }, { "memory", LEXER_TOKEN_DATATYPE  }
};

std::unordered_map<std::string, Lexeme> Lexer::stringToLexeme =
{
	{ "-",      LEXEME_MINUS           }, { "+",      LEXEME_PLUS           }, { "*",      LEXEME_MULTIPLY        }, { "/",      LEXEME_DIVIDE           }, { "=",      LEXEME_EQUALS            },
	{ "-=",     LEXEME_MINUSEQUALS     }, { "+=",     LEXEME_PLUSEQUALS     }, { "*=",     LEXEME_MULTIPLYEQUALS  }, { "/=",     LEXEME_DIVIDEEQUALS     }, { "==",     LEXEME_IS                },
	{ " ",      LEXEME_WHITESPACE      }, { "",       LEXEME_WHITESPACE     }, { ";",      LEXEME_ENDLINE         }, { "(",      LEXEME_OPEN_PARENTHESIS }, { ")",      LEXEME_CLOSE_PARENTHESIS },
	{ "{",      LEXEME_OPEN_CBRACKET   }, { "}",      LEXEME_CLOSE_CBRACKET }, { "[",      LEXEME_OPEN_SBRACKET   }, { "]",      LEXEME_CLOSE_SBRACKET   }, { "!=",     LEXEME_ISNOT             },
	{ "float",  LEXEME_DATATYPE_FLOAT  }, { "int",    LEXEME_DATATYPE_INT   }, { "char",   LEXEME_DATATYPE_CHAR   }, { "struct", LEXEME_STRUCT           }, { "void",   LEXEME_DATATYPE_VOID     },
	{ ".",      LEXEME_DOT             }, { ",",      LEXEME_COMMA          }, { "\n",     LEXEME_NEWLINE         }, { "return", LEXEME_RETURN           }, { "extern", LEXEME_EXTERN            },
	{ "string", LEXEME_DATATYPE_STRING }, { "import", LEXEME_IMPORT         }, { "if",     LEXEME_IF              }, { "while",  LEXEME_WHILE            }, { ">",      LEXEME_GREATER           }, 
	{ "<",      LEXEME_LESS            }, { ">=",     LEXEME_IS_OR_GREATER  }, { "<=",     LEXEME_IS_OR_LESS      }, { "for",    LEXEME_FOR              }, { "++",     LEXEME_PLUSPLUS          },
	{ "--",     LEXEME_MINUSMINUS      }, { "&",      LEXEME_AMPERSAND      }, { "uint64", LEXEME_DATATYPE_UINT64 }, { "typdef", LEXEME_TYPEDEF          }, { "memory", LEXEME_DATATYPE_UINT64   }
};

int Lexer::lineNumber = 1;

std::vector<Lexer::Token> Lexer::LexInput(std::string input)
{
	std::vector<Token> ret;
	std::string processingString = EMPTY_STRING;
	bool isInStringLiteral = false, isInCommentary = false;
	for (int i = 0; i < input.size(); i++)
	{
		if (input[i] == '\n')
			lineNumber++;
		if (lineNumber)
			lineNumber = lineNumber;
		isInCommentary = isInCommentary ? input[i] == '\n' ? false : isInCommentary : input[i] == '#';
		if (isInCommentary)
			continue;

		isInStringLiteral = isInStringLiteral ? input[i] != '\"' : input[i] == '\"';
		if ((!IsSeperator(input[i]) && !IsOperator(input[i])) || isInStringLiteral)
		{
			processingString += input[i];
			continue;
		}

		if (IsOperator(input[i]) && i != input.size() - 1)
		{
			if (!processingString.empty())
				ret.push_back(CreateToken(processingString));
			bool isLargeOperator = IsOperator(input[(size_t)i + 1]);
			if (isLargeOperator)
			{
				
				std::string fullOperator = std::string{ input[i] } + std::string{ input[(size_t)i + 1] };
				ret.push_back(CreateToken(fullOperator));
				i++;
			}
			else
				ret.push_back(CreateToken(std::string{ input[i] }));
			processingString = EMPTY_STRING;
			continue;
		}

		if (!processingString.empty())
			ret.push_back(CreateToken(processingString));
		if (input[i] != ' ' && input[i] != '\n' && input[i] != '\r' && input[i] != '\t') // whitespace is used as a seperator for cases like "float var" but whitespace doesnt need to be processed, the other seperators do need to be processed
			ret.push_back(CreateToken(std::string{ input[i] }));
		processingString = EMPTY_STRING;
	}
	return ret;
}

LexicalToken Lexer::GetLexicalToken(std::string item)
{
	if (item.empty())
		return LEXER_TOKEN_INVALID;
	if (IsDigitLiteral(item) || IsStringLiteral(item))
		return LEXER_TOKEN_LITERAL;

	return stringToLexicalToken.count(item) > 0 ? stringToLexicalToken[item] : LEXER_TOKEN_IDENTIFIER;
}

Lexeme Lexer::GetLexeme(LexicalToken token, std::string item)
{
	switch (token)
	{
	case LEXER_TOKEN_IDENTIFIER:
		return LEXEME_IDENTIFIER;

	case LEXER_TOKEN_KEYWORD:
	case LEXER_TOKEN_SEPERATOR:
	case LEXER_TOKEN_OPERATOR:
	case LEXER_TOKEN_DATATYPE:
		return stringToLexeme[item];

	case LEXER_TOKEN_LITERAL:
		if (IsCharLiteral(item))
			return LEXEME_LITERAL_CHAR;
		else if (IsStringLiteral(item))
			return LEXEME_LITERAL_STRING;
		else
			return IsFloatLiteral(item) ? LEXEME_LITERAL_FLOAT : LEXEME_LITERAL_INT;

	case LEXER_TOKEN_WHITESPACE:
		return LEXEME_WHITESPACE;
	}
	return LEXEME_INVALID;
}

Lexer::Token Lexer::CreateToken(std::string content)
{
	Token ret{};
	ret.content = content;
	ret.token = GetLexicalToken(content);
	ret.lexeme = GetLexeme(ret.token, content);
	ret.line = lineNumber;

	if (ret.lexeme == LEXEME_LITERAL_STRING) // remove the "'s of a string literal
	{
		ret.content.erase(ret.content.begin());
		ret.content.erase(ret.content.end() - 1);
	}

	return ret;
}

bool Lexer::IsSeperator(char item)
{
	switch (item)
	{
	case ' ':
	//case '.':
	case ',':
	case ';':
	case '(':
	case ')':
	case '{':
	case '}':
	case '\n':
	case '\t':
	case '\r':
		return true;
	}
	return false;
}

bool Lexer::IsOperator(char item)
{
	switch (item)
	{
	case '-':
	case '+':
	case '/':
	case '*':
	case '=':
	case '%':
	case '!':
	case '[':
	case ']':
	case '&':
		return true;
	}
	return false;
}

bool Lexer::IsDigitLiteral(std::string item)
{
	for (int i = 0; i < item.size(); i++)                                  // check if there are any non-digit chars in the string, if there arent its a number literal
	{                                                                      // i do not know if this counts a '.' or ',' is a digit, so this might not catch a float or double but that should be caught later on anyways
		if (i == 0 && item[0] == '-' && item.size() > 1 || item[i] == '.') // a negative number starts with a '-' which is not a digit, so skip the digit check to pretend that the '-' never existed
			continue;
		if (!std::isdigit((unsigned char)item[i]))
			return false;
	}
	return true;
}

bool Lexer::IsStringLiteral(std::string item)
{
	return item[0] == '\"' && item.back() == '\"';         // any string literal starts and ends with a '"', this does not however check if there are any syntaxical errors
}

bool Lexer::IsCharLiteral(std::string item)
{
	return item[0] == '\'' && item.back() == '\'';
}

bool Lexer::IsFloatLiteral(std::string item)
{
	return item.find('.') != std::string::npos;
}

std::string LexicalTokenToString(LexicalToken token)
{
	switch (token)
	{
	case LEXER_TOKEN_INVALID:    return "LEXER_TOKEN_INVALID";
	case LEXER_TOKEN_IDENTIFIER: return "LEXER_TOKEN_IDENTIFIER";
	case LEXER_TOKEN_SEPERATOR:  return "LEXER_TOKEN_SEPERATOR";
	case LEXER_TOKEN_LITERAL:    return "LEXER_TOKEN_LITERAL";
	case LEXER_TOKEN_KEYWORD:    return "LEXER_TOKEN_KEYWORD";
	case LEXER_TOKEN_OPERATOR:   return "LEXER_TOKEN_OPERATOR";
	case LEXER_TOKEN_DATATYPE:   return "LEXER_TOKEN_DATATYPE";
	case LEXER_TOKEN_WHITESPACE: return "LEXER_TOKEN_WHITESPACE";	
	}
	return "";
}

std::string LexemeToString(Lexeme value)
{
	switch (value)
	{
	case LEXEME_INVALID:           return "LEXEME_INVALID";
	case LEXEME_DATATYPE_CHAR:     return "LEXEME_DATATYPE_CHAR";
	case LEXEME_DATATYPE_FLOAT:    return "LEXEME_DATATYPE_FLOAT";
	case LEXEME_DATATYPE_INT:      return "LEXEME_DATATYPE_INT";
	case LEXEME_DATATYPE_STRING:   return "LEXEME_DATATYPE_STRING";
	case LEXEME_DATATYPE_VOID:     return "LEXEME_DATATYPE_VOID";
	case LEXEME_IDENTIFIER:        return "LEXEME_IDENTIFIER";
	case LEXEME_RETURN:            return "LEXEME_RETURN";
	case LEXEME_EXTERN:            return "LEXEME_EXTERN";
	case LEXEME_IF:                return "LEXEME_IF";
	case LEXEME_WHILE:             return "LEXEME_WHILE";
	case LEXEME_FOR:               return "LEXEME_FOR";
	case LEXEME_IMPORT:            return "LEXEME_IMPORT";
	case LEXEME_ENDLINE:           return "LEXEME_ENDLINE";
	case LEXEME_NEWLINE:           return "LEXEME_NEWLINE";
	case LEXEME_LITERAL_CHAR:      return "LEXEME_LITERAL_CHAR";
	case LEXEME_LITERAL_FLOAT:     return "LEXEME_LITERAL_FLOAT";
	case LEXEME_LITERAL_INT:       return "LEXEME_LITERAL_INT";
	case LEXEME_LITERAL_STRING:    return "LEXEME_LITERAL_STRING";
	case LEXEME_USERTYPE:          return "LEXEME_USERTYPE";
	case LEXEME_STRUCT:            return "LEXEME_STRUCT";
	case LEXEME_DOT:               return "LEXEME_DOT";
	case LEXEME_COMMA:             return "LEXEME_COMMA";
	case LEXEME_EQUALS:            return "LEXEME_EQUALS";
	case LEXEME_PLUSEQUALS:        return "LEXEME_PLUSEQUALS";
	case LEXEME_MINUSEQUALS:       return "LEXEME_MINUSEQUALS";
	case LEXEME_MULTIPLYEQUALS:    return "LEXEME_MULTIPLYEQUALS";
	case LEXEME_DIVIDEEQUALS:      return "LEXEME_DIVIDEEQUALS";
	case LEXEME_ISNOT:             return "LEXEME_ISNOT";
	case LEXEME_ISNOT_SINGLE:      return "LEXEME_ISNOT_SINGLE";
	case LEXEME_IS:                return "LEXEME_IS";
	case LEXEME_GREATER:           return "LEXEME_GREATER";
	case LEXEME_IS_OR_GREATER:     return "LEXEME_IS_OR_GREATER";
	case LEXEME_LESS:              return "LEXEME_LESS";
	case LEXEME_IS_OR_LESS:        return "LEXEME_IS_OR_LESS";
	case LEXEME_PLUS:              return "LEXEME_PLUS";
	case LEXEME_MINUS:             return "LEXEME_MINUS";
	case LEXEME_MULTIPLY:          return "LEXEME_MULTIPLY";
	case LEXEME_DIVIDE:            return "LEXEME_DIVIDE";
	case LEXEME_PLUSPLUS:          return "LEXEME_PLUSPLUS";
	case LEXEME_MINUSMINUS:        return "LEXEME_MINUSMINUS";
	case LEXEME_WHITESPACE:        return "LEXEME_WHITESPACE";
	case LEXEME_OPEN_CBRACKET:     return "LEXEME_OPEN_CBRACKET";
	case LEXEME_CLOSE_CBRACKET:    return "LEXEME_CLOSE_CBRACKET";
	case LEXEME_OPEN_SBRACKET:     return "LEXEME_OPEN_SBRACKET";
	case LEXEME_CLOSE_SBRACKET:    return "LEXEME_CLOSE_SBRACKET";
	case LEXEME_OPEN_PARENTHESIS:  return "LEXEME_OPEN_PARENTHESIS";
	case LEXEME_CLOSE_PARENTHESIS: return "LEXEME_CLOSE_PARENTHESIS";
	case LEXEME_AMPERSAND:         return "LEXEME_AMPERSAND";
	}
	return "";
}