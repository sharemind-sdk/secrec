/*
 * This file is a part of the Sharemind framework.
 * Copyright (C) Cybernetica AS
 *
 * All rights are reserved. Reproduction in whole or part is prohibited
 * without the written consent of the copyright owner. The usage of this
 * code is subject to the appropriate license agreement.
 */

#include <algorithm>
#include <cassert>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <libscc/intermediate.h>
#include <libscc/treenode.h>
#include <libscc/blocks.h>

#include "args.h"
#include "Compiler.h"

using namespace std;
using namespace SecreCC;

int main(int argc, char *argv[]) {

    /* Parse command line arguments: */
    const char* output_fname = 0;
    const char* input_fname = 0;
    while (true) {
        static struct option options[] = 
            { {"verbose", no_argument,       0, 'v'}
            , {"help",    no_argument,       0, 'h'}
            , {"output",  required_argument, 0, 'o'}
            , { "optimize", no_argument,     0, 'O'}
            , {0, 0, 0, 0}
            };

        int option_index = 0;
        const int c = getopt_long (argc, argv, "vhOo:", options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:   /* intentionally empty*/   break;
            case 'v': flags[Flag::Verbose] = 1;  break;
            case 'h': flags[Flag::Help] = 1;     break;
            case 'O': flags[Flag::Optimize] = 1; break;
            case 'o':
                if (flags[Flag::Output] != 0) {
                    help ();
                    return EXIT_FAILURE;
                }

                flags[Flag::Output] = 1;
                output_fname = optarg;
                break;
            default: 
                help ();
                return EXIT_FAILURE;
        }
    }

    if (flags[Flag::Help]) {
        help ();
        return EXIT_SUCCESS;
    }

    if (optind < argc) {
        input_fname = argv[optind ++];
    }

    /* Get input stream: */
    ostream* os = &cout;
    ofstream fout;
    if (output_fname != 0) {
        fout.open (output_fname);
        os = &fout;
    }

    /* Parse the program: */
    SecreC::TreeNodeProgram* parseTree = 0;
    int parseResult = 0;
    if (input_fname == 0) {
        parseResult = sccparse (&parseTree);
    }
    else {
        FILE *f = fopen(input_fname, "r");
        if (f != NULL) {
            if (flags[Flag::Verbose]) {
                cerr << "Parsing file: \"" << input_fname << "\"... ";
                cerr << flush;
            }

            parseResult = sccparse_file (f, &parseTree);
            fclose (f);

            if (flags[Flag::Verbose]) {
              cerr << "DONE!" << endl;
            }
        } else {
            cerr << "Unable to open file: \"" << input_fname << "\"" << endl;
            return EXIT_FAILURE;
        }
    }

    if (parseResult != 0) {
        return parseResult;
    }
    
    /* Translate to intermediate code: */
    SecreC::ICode icode;
    if (icode.init (parseTree) != SecreC::ICode::OK) {
        ::operator << (cerr << "Error generating valid intermediate code." << endl
                      , icode.compileLog ());
        return EXIT_FAILURE;
    }

    /* Compiler to assembly: */
    Compiler compiler (icode);
    compiler.run ();
    *os << compiler.target () << endl;
    delete parseTree;
    return EXIT_SUCCESS;
}
