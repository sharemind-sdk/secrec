#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>

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
#include <libscc/analysis/LiveVariables.h>
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
    bool m_printIR;
    bool m_eval;
    bool m_stdin;
    bool m_stdout;

    string m_output;
    string m_input;
    set<string > m_includes;
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

    ~Configuration () { }

    void read (const po::variables_map& vm) {
        m_verbose = vm.count ("verbose");
        m_help = vm.count ("help");
        m_eval = vm.count ("eval");
        m_printST = vm.count ("print-st");
        m_printAST = vm.count ("print-ast");
        m_printCFG = vm.count ("print-cfg");
        m_printIR = vm.count ("print-ir");

        if (vm.count ("output")) {
            m_stdout = false;
            m_output = vm["output"].as<string>();
        }

        if (vm.count ("input")) {
            m_stdin = false;
            m_input = vm["input"].as<string>();
        }

        if (vm.count ("include")) {
            const vector<string >& v = vm["include"].as<vector<string > >();
            m_includes.insert (v.begin (), v.end ());
        }

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

    if (name == "lv") {
        return new SecreC::LiveVariables ();
    }

    if (name == "dom") {
        return new SecreC::Dominators ();
    }

    return 0;
}

int run (const Configuration& cfg) {
    std::auto_ptr<SecreC::TreeNodeModule> parseTree;
    std::ostream out (cout.rdbuf ());
    io::stream_buffer<io::file_sink > fileBuf;

    if (! cfg.m_stdout) {
        fileBuf.open (cfg.m_output);
        if (! fileBuf.is_open ()) {
            std::cerr << "Unable to open \"" << cfg.m_output << "\" for output.";
            return EXIT_FAILURE;
        }

        out.rdbuf (&fileBuf);
    }

    int exitCode = 0;
    if (cfg.m_stdin) {
        SecreC::TreeNodeModule* tmpTree = 0;
        exitCode = sccparse(&tmpTree);
        parseTree.reset (tmpTree);
    } else {
        FILE *f = fopen (cfg.m_input.c_str (), "r");
        if (f != NULL) {
            if (cfg.m_verbose) {
                cerr << "Parsing file: \"" << cfg.m_input << "\"... ";
                cerr << flush;
            }

            SecreC::TreeNodeModule* tmpTree = 0;
            exitCode = sccparse_file(f, &tmpTree);
            parseTree.reset (tmpTree);
            fclose(f);

            if (cfg.m_verbose) {
              cerr << "DONE!" << endl;
            }
        } else {
            cerr << "Unable to open file: \"" << cfg.m_input << '\"' << endl;
            return EXIT_FAILURE;
        }
    }

    if (exitCode != 0) {
        cerr << "Parsing input file failed." << endl;
        return EXIT_FAILURE;
    }

    out << flush;
    cerr << flush;

    assert (parseTree.get () != 0);
    if (cfg.m_printAST) {
        out << parseTree->toString() << endl;
        return EXIT_SUCCESS;
    }

    SecreC::Context context;
    SecreC::ICode icode;

    BOOST_FOREACH (const std::string& path, cfg.m_includes) {
        icode.modules ().addSearchPath (path);
    }

    icode.init (context, parseTree.get ());

    if (icode.status() == SecreC::ICode::OK) {
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
            out << pr.toString() << endl;
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
                if (a != 0) {
                    analysis.push_back (a);
                    runner.addAnalysis (a);
                }
            }

            runner.run (pr);
            out << runner.toString (pr) << endl;
        }

        if (cfg.m_eval) {
            SecreC::VirtualMachine eval;
            return eval.run (pr);
        }
    } else {
        cerr << "Error generating valid intermediate code." << endl
             << icode.compileLog();
        return EXIT_FAILURE;
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
            ("eval,e", "Evaluate the program")
            ("print-ast", "Print the abstract syntax tree")
            ("print-st",  "Print the symbol table")
            ("print-cfg", "Print the control flow graph")
            ("print-ir",  "Print the intermediate represetnation")
            ("analysis,a", po::value<vector<string > >(),
             "Run specified analysis. Options are:\n"
             "\t\"rd\"  -- reaching definitions\n"
             "\t\"rj\"  -- reaching jumps\n"
             "\t\"rdc\" -- reaching declassify\n"
             "\t\"lv\"  -- live variables\n"
             "\t\"dom\" -- dominators\n");
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
