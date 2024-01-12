#pragma once
#include <string>
#include <unordered_map>
#include <iostream>

enum Args
{
	ARG_NONE,
	ARG_VERBOSE,
	ARG_ENTRY_POINT,
	ARG_DUMP_INSTRUCTIONS,
	ARG_DUMP_STACK_FRAMES,
	ARG_TREAT_VOID_AS_ERROR,
	ARG_DISABLE_IMPLICIT_CONVERSION,
	ARG_INPUT,
	ARG_REMOVE_UNUSED_SYMBOLS,
	ARG_OPTIMIZE_INSTRUCTIONS,
	ARG_PARSE_MULTITHREADED, // reserved
};

namespace Behavior
{
	inline bool verbose = false;
	inline bool dumpFunctionInstructions = false;
	inline bool dumpStackFrame = false;
	inline bool treatVoidAsError = false;
	inline bool disableImplicitConversion = false;
	inline bool removeUnusedSymbols = false;
	inline bool optimizeInstructions = false;

	inline std::string input = "";
	inline std::string entryPoint = "main";

	inline void ProcessCommandArguments(int argc, const char** argv)
	{
		static std::unordered_map<std::string, Args> stringToArgs =
		{
			{ "-verbose", ARG_VERBOSE }, { "-entry_point",       ARG_ENTRY_POINT       }, { "-dump_instructions", ARG_DUMP_INSTRUCTIONS     },
			{ "-input",   ARG_INPUT   }, { "-dump_stack_frames", ARG_DUMP_STACK_FRAMES }, { "-treat_void_as_error", ARG_TREAT_VOID_AS_ERROR },
			{ "-remove_unused_symbols", ARG_REMOVE_UNUSED_SYMBOLS }, {"-disable_implicit_conversion", ARG_DISABLE_IMPLICIT_CONVERSION},
			{ "-optimize_instructions", ARG_OPTIMIZE_INSTRUCTIONS }, { "-parse_multithreaded", ARG_PARSE_MULTITHREADED },
		};
		for (int i = 0; i < argc; i++)
		{
			std::string arg = argv[i];
			if (stringToArgs.count(arg) == 0)
				continue;

			switch (stringToArgs[arg])
			{
			case ARG_VERBOSE:
				verbose = true;
				break;
			case ARG_ENTRY_POINT:
				entryPoint = argv[i + 1];
				i++;
				break;
			case ARG_DUMP_INSTRUCTIONS:
				dumpFunctionInstructions = true;
				break;
			case ARG_INPUT:
				input = argv[i + 1];
				i++;
				break;
			case ARG_DUMP_STACK_FRAMES:
				dumpStackFrame = true;
				break;
			case ARG_TREAT_VOID_AS_ERROR:
				treatVoidAsError = true;
				break;
			case ARG_DISABLE_IMPLICIT_CONVERSION:
				disableImplicitConversion = true;
				break;
			case ARG_REMOVE_UNUSED_SYMBOLS:
				removeUnusedSymbols = true;
				break;
			case ARG_OPTIMIZE_INSTRUCTIONS:
				optimizeInstructions = true;
				break;
			}
		}
		if (!verbose)
			return;

		std::cout << "Recieved " << argc << " arguments:\n";
		for (int i = 0; i < argc; i++)
		{
			if (argv[i][0] == '-')
				std::cout << "\n" << "  " << argv[i] << " ";
			else
				std::cout << argv[i];
		}
		std::cout << "\n\n";
	}
}