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
#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <libscc/context.h>
#include <libscc/intermediate.h>
#include <libscc/treenode.h>
#include <libscc/blocks.h>

#include "Compiler.h"

using namespace std;
using namespace SecreCC;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {

    po::options_description desc ("Available options");
    desc.add_options ()
            ("help,h", "Display this help message")
            ("verbose,v", "Enable verbose output")
            ("include,I", po::value<vector<string > >(), "Directory for module search path.")
            ("output,o", po::value<string>(), "Output file")
            ("input", po::value<string>(), "Input file")
            ;
    po::positional_options_description p;
    p.add("input", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
        options (desc).positional (p).run (), vm);
    po::notify(vm);

    bool verbose = false;
    if (vm.count ("verbose")) {
        verbose = true;
    }

    if (vm.count ("help")) {
        cout << desc << "\n";
        return EXIT_FAILURE;
    }

    /* Get input stream: */
    ostream* os = &cout;
    ofstream fout;
    if (vm.count ("output")) {
        fout.open (vm["output"].as<string>().c_str ());
        os = &fout;
    }

    /* Parse the program: */
    SecreC::TreeNodeModule* parseTree = 0;
    int parseResult = 0;
    if (vm.count ("input")) {
        const std::string fname = vm["input"].as<string>();
        FILE* f = fopen(fname.c_str (), "r");
        if (f != NULL) {
            if (verbose) {
                cerr << "Parsing file: \"" << fname << "\"... ";
                cerr << flush;
            }

            parseResult = sccparse_file (f, &parseTree);
            fclose (f);
        } else {
            cerr << "Unable to open file: \"" << fname << "\"" << endl;
            return EXIT_FAILURE;
        }
    }
    else {
        parseResult = sccparse (&parseTree);
    }

    if (parseResult != 0) {
        return parseResult;
    }
    
    /* Translate to intermediate code: */
    SecreC::Context context;
    SecreC::ICode icode;

    /* Collect possible include files: */
    if (vm.count ("include")) {
        BOOST_FOREACH (const string& name, vm["include"].as<vector<string > >()) {
            icode.modules ().addSearchPath (name);
        }
    }

    if (icode.init (context, parseTree) != SecreC::ICode::OK) {
        ::operator << (cerr << "Error generating valid intermediate code." << endl
                      , icode.compileLog ());
        return EXIT_FAILURE;
    }

    /* Compiler to assembly: */
    VMLinkingUnit vmlu;
    Compiler compiler (icode);
    compiler.run (vmlu);
    *os << vmlu << endl;
    delete parseTree;
    return EXIT_SUCCESS;
}
