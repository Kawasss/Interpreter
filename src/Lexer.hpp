#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "common.hpp"

constexpr auto EMPTY_STRING = "";

enum LexicalToken
{
	LEXER_TOKEN_INVALID,
	LEXER_TOKEN_IDENTIFIER,
	LEXER_TOKEN_SEPERATOR,
	LEXER_TOKEN_LITERAL,
	LEXER_TOKEN_KEYWORD,
	LEXER_TOKEN_OPERATOR,
	LEXER_TOKEN_DATATYPE,
	LEXER_TOKEN_WHITESPACE
};

enum Lexeme
{
	LEXEME_INVALID,
	LEXEME_DATATYPE_CHAR = DATA_TYPE_CHAR,
	LEXEME_DATATYPE_FLOAT = DATA_TYPE_FLOAT,
	LEXEME_DATATYPE_INT = DATA_TYPE_INT,
	LEXEME_DATATYPE_STRING = DATA_TYPE_STRING,
	LEXEME_DATATYPE_VOID = DATA_TYPE_VOID,
	LEXEME_IDENTIFIER,
	LEXEME_RETURN,
	LEXEME_EXTERN,
	LEXEME_IF,
	LEXEME_WHILE,
	LEXEME_FOR,
	LEXEME_IMPORT,
	LEXEME_ENDLINE,
	LEXEME_NEWLINE,
	LEXEME_LITERAL_CHAR,
	LEXEME_LITERAL_FLOAT,
	LEXEME_LITERAL_INT,
	LEXEME_LITERAL_STRING,
	LEXEME_USERTYPE,
	LEXEME_STRUCT,
	LEXEME_DOT,
	LEXEME_COMMA,
	LEXEME_EQUALS,
	LEXEME_PLUSEQUALS,
	LEXEME_MINUSEQUALS,
	LEXEME_MULTIPLYEQUALS,
	LEXEME_DIVIDEEQUALS,
	LEXEME_ISNOT,
	LEXEME_ISNOT_SINGLE, // !identifier
	LEXEME_IS,
	LEXEME_GREATER,
	LEXEME_IS_OR_GREATER,
	LEXEME_LESS,
	LEXEME_IS_OR_LESS,
	LEXEME_PLUS,
	LEXEME_MINUS,
	LEXEME_MULTIPLY,
	LEXEME_DIVIDE,
	LEXEME_WHITESPACE,
	LEXEME_OPEN_CBRACKET,  // {
	LEXEME_CLOSE_CBRACKET, // }
	LEXEME_OPEN_SBRACKET,  // [
	LEXEME_CLOSE_SBRACKET, // ]
	LEXEME_OPEN_PARENTHESIS,
	LEXEME_CLOSE_PARENTHESIS,
};

class Lexer
{
public:
	struct Token
	{
		std::string content = EMPTY_STRING;
		LexicalToken token = LEXER_TOKEN_INVALID;
		Lexeme lexeme = LEXEME_INVALID;
	};

	static std::vector<Token> LexInput(std::string input);

private:
	static LexicalToken GetLexicalToken(std::string item);
	static Lexeme GetLexeme(LexicalToken token, std::string item);
	static Token CreateToken(std::string content);

	static bool IsSeperator(char item);
	static bool IsOperator(char item);
	static bool IsDigitLiteral(std::string item);
	static bool IsStringLiteral(std::string item);
	static bool IsCharLiteral(std::string item);
	static bool IsFloatLiteral(std::string item);

	static std::unordered_map<std::string, LexicalToken> stringToLexicalToken;
	static std::unordered_map<std::string, Lexeme>       stringToLexeme;
	static int lineNumber;
};