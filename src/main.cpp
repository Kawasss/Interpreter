#include <iostream>
#include <fstream>
#include <conio.h>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "Function.hpp"
#include "std.hpp"
#include "Behavior.hpp"
#include "Debug.hpp"

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
		importedFiles.push_back(file);

		if (Behavior::verbose)
			std::cout << "Successfully imported file at " << file << "\n";
	}
}

inline void InterpretString(std::string input)
{
	std::vector<Lexer::Token> tokens = Lexer::LexInput(input);
	CheckForImport(tokens);

	if (Behavior::dumpTokens)
	{
		for (Lexer::Token& token : tokens)
			std::cout << Debug::DumpToken(token) << "\n";
		return;
	}

	AbstractSyntaxTree tree = Parser::CreateAST(tokens);
	if (Behavior::verbose)
	{
		std::cout << "Processed functions:\n";
		for (Function* fn : tree.functions)
			std::cout << "  " << fn->GetName() << ",\n";
			
		if (tree.entryPoint != nullptr)
			std::cout << "Using " << tree.entryPoint->GetName() << " as entry point\n\n";
	}

	Interpreter::Init();
	Interpreter::SetAST(tree);

	StandardLib::Init();

	if (tree.entryPoint == nullptr && !Behavior::dumpFunctionInstructions && !Behavior::dumpStackFrame)
		throw std::runtime_error("Failed to find the entry point, create a function named \"main()\" or define a custom entry point with the command argument -entry_point");

	if (!Behavior::dumpFunctionInstructions && !Behavior::dumpStackFrame) // dumping instructions means no code gets executed
		tree.entryPoint->ExecuteBody();
}

int main(int argsc, const char** argsv)
{
	Behavior::ProcessCommandArguments(argsc, argsv);
	try 
	{
		if (Behavior::input == "")
			throw std::runtime_error("No input file given, use the -input command argument to give the input file");
		std::vector<char> code = ReadFile(Behavior::input);
		std::string strCode = code.data();

		InterpretString(strCode);
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}

	#ifdef _WIN32 // afaik this only works on windows?
	std::cout << "\nExecution ended, press any key to exit..." << std::endl;
	_getch();
	#endif

	return 0;
}