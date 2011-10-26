#include <cassert>
#include <cstring>
#include <getopt.h>
#include <iostream>

#include <libscc/blocks.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/intermediate.h>
#include <libscc/parser.h>
#include <libscc/treenode.h>
#include <libscc/virtual_machine.h>

using namespace std;

void help (void) {
  cout <<
  "Usage: sca [options] [file]\n"
  "Options:\n"
  "  -h, --help           this help\n"
  "  -v, --verbose        some extra information\n"
  "      --print-ast      print abstract syntax tree\n"
  "      --print-st       print symbol tabel\n"
  "      --cfg-dotty      output CFG as dotty graph\n"
  "  -e, --eval           evaluate the code\n"
  "  -a, --analysis       select analysis that you wish to enable\n"
  "                       Possible comma separated values are:\n"
  "                       \"rd\"  reaching definitions\n"
  "                       \"rj\"  reaching jumps\n"
  "                       \"rdc\" reaching declassify\n"
  "                       \"lv\"  live variables\n"
  << endl;
}

// top level commands
namespace Flag {
enum Name {
    Verbose = 0,
    Help,
    PrintAst,
    PrintST,
    CFGDotty,
    Eval,
    Analysis,
    Count
};
}

// arguments to --analysis command
enum AnalysisType {
    ReachingDefs       = 0x01,
    ReachingJumps      = 0x02,
    ReachingDeclassify = 0x04,
    ConstantFolding    = 0x08,
    LiveVariables      = 0x10
};

static int flags[Flag::Count];

static
int run (const char* filename) {
    SecreC::TreeNodeProgram *parseTree = 0;
    int parseResult = 0;

    if (filename == 0) {
        parseResult = sccparse(&parseTree);
    } else {
        FILE *f = fopen(filename, "r");
        if (f != NULL) {
            if (flags[Flag::Verbose]) {
                cerr << "Parsing file: \"" << filename << "\"... ";
                cerr << flush;
            }

            parseResult = sccparse_file(f, &parseTree);
            fclose(f);

            if (flags[Flag::Verbose]) {
              cerr << "DONE!" << endl;
            }
        } else {
            cerr << "Unable to open file: " << filename << endl;
            return 1;
        }
    }

    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        if (flags[Flag::PrintAst]) {
          cout << parseTree->toString() << endl << endl;
        }

        SecreC::ICode icode;
        icode.init (parseTree);

        if (icode.status() == SecreC::ICode::OK) {
            SecreC::Program& pr = icode.program ();

            if (flags[Flag::Verbose]) {
              cerr << "Valid intermediate code generated." << endl
                   << icode.compileLog();
            }

            if (flags[Flag::PrintST]) {
                cerr << icode.symbols () << endl;
            }

            if (flags[Flag::Verbose]) {
                cerr << pr.toString() << endl;
            }

            if (flags[Flag::CFGDotty]) {
                pr.toDotty (cout);
                cout << std::flush;
            }

            if (flags[Flag::Eval]) {
                SecreC::VirtualMachine eval;
                eval.run (pr);
                if (flags[Flag::Verbose]) {
                    cerr << eval.toString();
                }
            }

            // Run data flow analysis and print the results:
            if (flags[Flag::Analysis] > 0) {
                /// \todo need better solution than this if any more analysis are added
                SecreC::DataFlowAnalysisRunner runner;
                SecreC::ReachingDefinitions rd;
                SecreC::ReachingJumps rj;
                SecreC::ReachingDeclassify rdc;
                SecreC::LiveVariables lv;

                if (flags[Flag::Analysis] & ReachingDefs)       runner.addAnalysis(&rd);
                if (flags[Flag::Analysis] & ReachingJumps)      runner.addAnalysis(&rj);
                if (flags[Flag::Analysis] & ReachingDeclassify) runner.addAnalysis(&rdc);
                if (flags[Flag::Analysis] & LiveVariables)      runner.addAnalysis(&lv);

                runner.run(pr);

                if (flags[Flag::Analysis] & ReachingDefs)       cout << rd.toString(pr)  << endl;
                if (flags[Flag::Analysis] & ReachingJumps)      cout << rj.toString(pr)  << endl;
                if (flags[Flag::Analysis] & ReachingDeclassify) cout << rdc.toString(pr) << endl;
                if (flags[Flag::Analysis] & LiveVariables)      cout << lv.toString(pr)  << endl;
            }
        } else {
            cerr << "Error generating valid intermediate code." << endl
                 << icode.compileLog();
        }
    }

    delete parseTree;
    return parseResult;
}

int main(int argc, char *argv[]) {
    char* filename = 0;

    while (1) {
      static struct option options[] = {
        {"verbose",      no_argument,       0,                      'v'},
        {"help",         no_argument,       0,                      'h'},
        {"print-ast",    no_argument,       &flags[Flag::PrintAst],  1 },
        {"print-st",     no_argument,       &flags[Flag::PrintST],   1 },
        {"cfg-dotty",    no_argument,       &flags[Flag::CFGDotty],  1 },
        {"eval",         no_argument,       0,                      'e'},
        {"analysis",     optional_argument, 0,                      'a'},
        {0, 0, 0, 0}
      };

      int option_index = 0;
      int c = getopt_long (argc, argv, "vhea::", options, &option_index);
      char *str = optarg;

      if (c == -1) {
        break;
      }

      switch (c) {
        case 0:  /* intentionally empty */ break;
        case 'v': flags[Flag::Verbose] = 1; break;
        case 'e': flags[Flag::Eval] = 1;    break;
        case 'h': flags[Flag::Help] = 1;    break;
        case 'a':
          while (1) {
            const char* token = strtok (str, ",");
            if (token == 0) break;
            if (strcmp (token, "rd") == 0)  flags[Flag::Analysis] |= ReachingDefs;
            if (strcmp (token, "rj") == 0)  flags[Flag::Analysis] |= ReachingJumps;
            if (strcmp (token, "rdc") == 0) flags[Flag::Analysis] |= ReachingDeclassify;
            if (strcmp (token, "lv") == 0)  flags[Flag::Analysis] |= LiveVariables;
            str = NULL;
          }

          break;
        default:
          help ();
          return 1;
      }
    }

    if (flags[Flag::Help]) {
      help ();
      return 0;
    }

    if (optind < argc) {
      filename = argv[optind ++];
    }

    return run (filename);
}
