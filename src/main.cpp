#include <iostream>
#include <fstream>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "Function.hpp"
#include "std.hpp"

// do something with args, like -verbose and -entry_point string, then store them with flags in an enum and pass that to the interpreter

inline std::vector<char> ReadFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize + 1);
	buffer.back() = '\0';

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

inline void CheckForImport(std::vector<Lexer::Token>& tokens)
{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].lexeme != LEXEME_IMPORT)
			continue;
		
		std::string file = tokens[i + 1].content;
		std::vector<char> importCode = ReadFile(file);
		std::string code = importCode.data();
		std::vector<Lexer::Token> importTokens = Lexer::LexInput(code);

		tokens.erase(tokens.begin() + i);
		tokens.erase(tokens.begin() + i); // remove the import statement to prevent an endless loop
		tokens.insert(tokens.begin() + i, importTokens.begin(), importTokens.end());
		i = 0;
	}
}

inline void InterpretString(std::string input)
{
	std::vector<Lexer::Token> tokens = Lexer::LexInput(input);
	CheckForImport(tokens);
	AbstractSyntaxTree tree = Parser::CreateAST(tokens);
	
	Interpreter::Init();
	Interpreter::SetAST(tree);

	StandardLib::Init();

	tree.entryPoint->ExecuteBody();
}

int main(int argsc, const char** argsv)
{
	try 
	{
		std::vector<char> code = ReadFile("script/test.script");
		std::string strCode = code.data();

		InterpretString(strCode);
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	int waiting;
	std::cin >> waiting;

	return 0;
}