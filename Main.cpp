﻿#include <iostream>
#include "PePatch.hpp"

// https://github.com/brofield/simpleopt
// MIT license
#include "utils/SimpleOpt.h"

enum { OPT_HELP, OPT_PATH, OPT_SECT };

CSimpleOpt::SOption g_rgOptions[] = {
	{ OPT_PATH, "-p",        SO_REQ_SEP },
	{ OPT_SECT, "-s",        SO_REQ_SEP },
	{ OPT_SECT, "--section", SO_REQ_SEP },
	{ OPT_HELP, "-h",        SO_NONE },
	{ OPT_HELP, "--help",    SO_NONE },
	SO_END_OF_OPTIONS
};

void PrintUsage() {
    std::cout << "Usage: pe_ep_intercept.exe "
            "[-p PATH] [-s SECTION_NAME] "
            "[--section SECTION_NAME] [-h] [--help]" << std::endl;
}

int main(int argc, char *argv[]) {
    std::cout << "pe_ep_intercept" << std::endl;

	if (argc < 2) {
		PrintUsage();
		return 0;
	}

	CSimpleOpt args(argc, argv, g_rgOptions, SO_O_EXACT);

	std::string new_section_name;
    std::string target_file;
    _ESOError esoState = SO_SUCCESS;

	while (args.Next() && esoState == SO_SUCCESS) {
		esoState = args.LastError();
		if (esoState == SO_SUCCESS) {
			switch (args.OptionId()) {
			case OPT_HELP:
				PrintUsage();
				return 0;
			case OPT_PATH:
				target_file = args.OptionArg();
				break;
			case OPT_SECT:
				new_section_name = args.OptionArg();
				break;
            default:
				break;
			}
		} else if (esoState == SO_OPT_INVALID) {
            std::cout << "Error: unknown option was given: " << args.OptionText() << std::endl;
		} else if (esoState == SO_ARG_MISSING) {
            std::cout << "Error: missing argument was given for: " << args.OptionText() << std::endl;
		}
	}

	if (esoState != SO_SUCCESS) {
		PrintUsage();
		return 1;
	}

    if (target_file.empty()) {
        std::cout << "Error: path cannot be empty." << std::endl;
		PrintUsage();
		return 1;
    }

	try {
		PePatch patcher(target_file);
        std::string instructions = patcher.CreateEntryPointSubroutine(100);
        std::vector<char> machine_code = patcher.Assemble(instructions);

        if(machine_code.size() > 0xFFFFFF) {
            return 1;
        }

        patcher.AddSection(".code", static_cast<uint32_t>(machine_code.size()));
        patcher.SaveFile("a2.exe", machine_code);
	} catch (std::runtime_error &err) {
        std::cout << err.what() << std::endl;
		return 1;
	}

	return 0;
}