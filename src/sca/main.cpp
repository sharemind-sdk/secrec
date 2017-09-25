/*
 * Copyright (C) 2015 Cybernetica
 *
 * Research/Commercial License Usage
 * Licensees holding a valid Research License or Commercial License
 * for the Software may use this file according to the written
 * agreement between you and Cybernetica.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 3.0 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.  Please review the following information to
 * ensure the GNU General Public License version 3.0 requirements will be
 * met: http://www.gnu.org/copyleft/gpl-3.0.html.
 *
 * For further information, please contact us at sharemind@cyber.ee.
 */

#include <cassert>
#include <cstring>
#include <iostream>
#include <locale>
#include <memory>

#include <boost/optional/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file.hpp>

#include <libscc/Blocks.h>
#include <libscc/Context.h>
#include <libscc/DataflowAnalysis.h>
#include <libscc/Intermediate.h>
#include <libscc/Optimizer.h>
#include <libscc/Parser.h>
#include <libscc/TreeNode.h>
#include <libscc/VirtualMachine.h>
#include <libscc/analysis/ConstantFolding.h>
#include <libscc/analysis/CopyPropagation.h>
#include <libscc/analysis/Dominators.h>
#include <libscc/analysis/LiveMemory.h>
#include <libscc/analysis/LiveVariables.h>
#include <libscc/analysis/ReachableDefinitions.h>
#include <libscc/analysis/ReachableReturns.h>
#include <libscc/analysis/ReachableUses.h>
#include <libscc/analysis/ReachingDeclassify.h>
#include <libscc/analysis/ReachingDefinitions.h>
#include <libscc/analysis/ReachingJumps.h>


using namespace std;
namespace po = boost::program_options;
namespace io = boost::iostreams;


namespace {

struct Configuration {
    bool m_verbose = false;
    bool m_help = false;
    bool m_printST = false;
    bool m_printAST = false;
    bool m_printCFG = false;
    bool m_printDom = false;
    bool m_printIR = false;
    bool m_eval = false;
    bool m_stdin = true;
    bool m_stdout = true;
    bool m_optimize = false;

    string m_output;
    string m_input;
    vector<string > m_includes;
    set<string > m_analysis;

    void read (const po::variables_map& vm) {
        m_verbose = vm.count ("verbose");
        m_help = vm.count ("help");
        m_eval = vm.count ("eval");
        m_printST = vm.count ("print-st");
        m_printAST = vm.count ("print-ast");
        m_printCFG = vm.count ("print-cfg");
        m_printIR = vm.count ("print-ir");
        m_printDom = vm.count ("print-dom");
        m_optimize = vm.count ("optimize");

        if (vm.count ("output")) {
            m_stdout = false;
            m_output = vm["output"].as<string>();
        }

        if (vm.count ("input")) {
            m_stdin = false;
            m_input = vm["input"].as<string>();
        }

        if (vm.count ("include")) {
            m_includes = vm["include"].as<vector<string > >();
        }

#ifndef SHAREMIND_STDLIB_PATH
#error "SHAREMIND_STDLIB_PATH not defined"
#endif
        if (!vm.count("no-stdlib"))
            m_includes.push_back (SHAREMIND_STDLIB_PATH);

        if (vm.count ("analysis")) {
            const vector<string >& v = vm["analysis"].as<vector<string > > ();
            m_analysis.insert (v.begin (), v.end ());
        }
    }
};

SecreC::DataFlowAnalysis* getAnalysisByName (const std::string& name) {
    if (name == "rd") {
        return new SecreC::ReachingDefinitions ();
    }

    if (name == "rj")  {
        return new SecreC::ReachingJumps ();
    }

    if (name == "rdc") {
        return new SecreC::ReachingDeclassify ();
    }

    if (name == "ru") {
        return new SecreC::ReachableUses ();
    }

    if (name == "rabled") {
        return new SecreC::ReachableDefinitions ();
    }

    if (name == "lv") {
        return new SecreC::LiveVariables ();
    }

    if (name == "lm") {
        return new SecreC::LiveMemory ();
    }

    if (name == "cf") {
        return new SecreC::ConstantFolding ();
    }

    if (name == "cp") {
        return new SecreC::CopyPropagation ();
    }

    if (name == "rr") {
        return new SecreC::ReachableReturns ();
    }

    return nullptr;
}

int run (const Configuration& cfg) {
    SecreC::TreeNodeModule * parseTree = nullptr;
    std::ostream out (cout.rdbuf ());
    io::stream_buffer<io::file_sink > fileBuf;
    SecreC::ICode icode;

    if (! cfg.m_stdout) {
        fileBuf.open (cfg.m_output);
        if (! fileBuf.is_open ()) {
            std::cerr << "Unable to open \"" << cfg.m_output << "\" for output.";
            return EXIT_FAILURE;
        }

        out.rdbuf (&fileBuf);
    }

    parseTree = icode.parseMain (cfg.m_stdin ? boost::optional<std::string>() : cfg.m_input);
    if (icode.status () != SecreC::ICode::OK) {
        std::cerr << icode.compileLog ();
        return EXIT_FAILURE;
    }

    out << flush;
    cerr << flush;

    assert(parseTree);
    if (cfg.m_printAST) {
        parseTree->print(out);
        out << endl;
        return EXIT_SUCCESS;
    }

    for (const std::string& path : cfg.m_includes) {
        icode.modules ().addSearchPath (path, cfg.m_verbose);
    }

    icode.compile (parseTree);
    if (icode.status () != SecreC::ICode::OK) {
        cerr << "Error generating valid intermediate code." << endl;
        cerr << icode.compileLog () << endl;
        return EXIT_FAILURE;
    }

    SecreC::Program& pr = icode.program ();

    if (cfg.m_verbose) {
        cerr << "Valid intermediate code generated." << endl
             << icode.compileLog();
    }

    if (cfg.m_optimize)
        optimizeCode (icode);

    if (cfg.m_printST) {
        out << icode.symbols () << endl;
        return EXIT_SUCCESS;
    }

    if (cfg.m_printIR) {
        out << pr << endl;
        return EXIT_SUCCESS;
    }

    if (cfg.m_printDom) {
        SecreC::Dominators dominators;
        dominators.calculate (&pr);
        dominators.dumpToDot (out);
        return EXIT_SUCCESS;
    }

    if (cfg.m_printCFG) {
        pr.toDotty (out);
        out << flush;
        return EXIT_SUCCESS;
    }

    // Run data flow analysis and print the results:
    if (! cfg.m_analysis.empty ()) {
        SecreC::DataFlowAnalysisRunner runner;
        boost::ptr_vector<SecreC::DataFlowAnalysis> analysis;
        for (const std::string& name : cfg.m_analysis) {
            if (SecreC::DataFlowAnalysis * const a = getAnalysisByName(name)) {
                analysis.push_back (a);
                runner.addAnalysis (*a);
            }
        }

        runner.run (pr);
        out << runner.toString (pr) << endl;
    }

    if (cfg.m_eval) {
        SecreC::VirtualMachine eval;
        return eval.run (pr);
    }

    return EXIT_SUCCESS;
}

} // anonymous namespace

int main(int argc, char *argv[]) {
    try {
        std::locale("");
    } catch (std::exception const & e) {
        std::cerr << "Invalid default locale from environment: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Invalid default locale from environment!" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        po::options_description desc ("Available options");
        desc.add_options ()
                ("help,h",    "Display this help message")
                ("verbose,v", "Enable verbose output")
                ("output,o",   po::value<string>(), "Output file")
                ("input",      po::value<string>(), "Input file")
                ("include,I",  po::value<vector<string > >(),
                 "Directory for module search path.")
                ("no-stdlib", "Do not look for standard library imports.")
                ("optimize,O", "Optimize the generated code.")
                ("eval,e", "Evaluate the program")
                ("print-ast", "Print the abstract syntax tree")
                ("print-st",  "Print the symbol table")
                ("print-cfg", "Print the control flow graph")
                ("print-dom", "Print the dominators tree")
                ("print-ir",  "Print the intermediate represetnation")
                ("analysis,a", po::value<vector<string > >(),
                 "Run specified analysis. Options are:\n"
                 "\t\"rd\"  -- reaching definitions\n"
                 "\t\"rj\"  -- reaching jumps\n"
                 "\t\"rdc\" -- reaching declassify\n"
                 "\t\"rabled\" -- reachable definitions\n"
                 "\t\"ru\"  -- reachable uses\n"
                 "\t\"lm\"  -- live memory\n"
                 "\t\"lv\"  -- live variables\n"
                 "\t\"cf\"  -- constant folding\n"
                 "\t\"cp\"  -- copy propagation\n"
                 "\t\"rr\"  -- reachable returns\n"
                 );
        po::positional_options_description p;
        p.add("input", -1);
        po::variables_map vm;

        try {
            p.add("input", -1);
            po::store(po::command_line_parser(argc, argv).
                      options (desc).positional (p).run (), vm);
            po::notify(vm);
        }
        catch (const std::exception& e) {
            std::cerr << e.what () << std::endl;
            std::cerr << desc << std::endl;
            return EXIT_FAILURE;
        }

        Configuration cfg;
        cfg.read (vm);
        if (cfg.m_help) {
            cout << desc << "\n";
            return EXIT_SUCCESS;
        }

        return run (cfg);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed with exception:" << std::endl;
        std::cerr << e.what () << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Failed with unknown exception." << std::endl;
        return EXIT_FAILURE;
    }
}
