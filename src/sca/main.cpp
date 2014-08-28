#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>

#include <boost/optional/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/device/file.hpp>

#include <libscc/context.h>
#include <libscc/blocks.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/intermediate.h>
#include <libscc/parser.h>
#include <libscc/treenode.h>
#include <libscc/virtual_machine.h>
#include <libscc/analysis/ReachingDeclassify.h>
#include <libscc/analysis/ReachingDefinitions.h>
#include <libscc/analysis/ReachingJumps.h>
#include <libscc/analysis/ReachableReleases.h>
#include <libscc/analysis/LiveVariables.h>
#include <libscc/analysis/LiveMemory.h>
#include <libscc/analysis/Dominators.h>


using namespace std;
namespace po = boost::program_options;
namespace io = boost::iostreams;

struct Configuration {
    bool m_verbose;
    bool m_help;
    bool m_printST;
    bool m_printAST;
    bool m_printCFG;
    bool m_printDom;
    bool m_printIR;
    bool m_eval;
    bool m_stdin;
    bool m_stdout;

    string m_output;
    string m_input;
    vector<string > m_includes;
    set<string > m_analysis;

    Configuration ()
        : m_verbose (false)
        , m_help (false)
        , m_printST (false)
        , m_printAST (false)
        , m_printCFG (false)
        , m_printIR (false)
        , m_eval (false)
        , m_stdin (true)
        , m_stdout (true)
    { }

    void read (const po::variables_map& vm) {
        m_verbose = vm.count ("verbose");
        m_help = vm.count ("help");
        m_eval = vm.count ("eval");
        m_printST = vm.count ("print-st");
        m_printAST = vm.count ("print-ast");
        m_printCFG = vm.count ("print-cfg");
        m_printIR = vm.count ("print-ir");
        m_printDom = vm.count ("print-dom");

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
#else
        if (!vm.count("no-stdlib"))
            m_includes.push_back (SHAREMIND_STDLIB_PATH);
#endif

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

    if (name == "rr") {
        return new SecreC::ReachableReleases ();
    }

    if (name == "lv") {
        return new SecreC::LiveVariables ();
    }

    if (name == "lm") {
        return new SecreC::LiveMemory ();
    }

    return 0;
}

int run (const Configuration& cfg) {
    SecreC::TreeNodeModule* parseTree = NULL;
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

    assert (parseTree != NULL);
    if (cfg.m_printAST) {
        parseTree->print(out);
        out << endl;
        return EXIT_SUCCESS;
    }

    BOOST_FOREACH (const std::string& path, cfg.m_includes) {
        if (! icode.modules ().addSearchPath (path) && cfg.m_verbose) {
            cerr << "Invalid search path \"" << path << "\"." << endl;
        }
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
        BOOST_FOREACH (const std::string& name, cfg.m_analysis) {
            SecreC::DataFlowAnalysis* a = getAnalysisByName (name);
            if (a != NULL) {
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

int main(int argc, char *argv[]) {
    po::options_description desc ("Available options");
    desc.add_options ()
            ("help,h",    "Display this help message")
            ("verbose,v", "Enable verbose output")
            ("output,o",   po::value<string>(), "Output file")
            ("input",      po::value<string>(), "Input file")
            ("include,I",  po::value<vector<string > >(),
             "Directory for module search path.")
            ("no-stdlib", "Do not look for standard library imports.")
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
             "\t\"rr\"  -- reachable releases\n"
             "\t\"lm\"  -- live memory\n"
             "\t\"lv\"  -- live variables\n"
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
