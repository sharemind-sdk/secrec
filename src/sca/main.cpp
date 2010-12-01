#include <cassert>
#include <iostream>
#include <cstring>
#include <getopt.h>

#include <libscc/blocks.h>
#include <libscc/intermediate.h>
#include <libscc/parser.h>
#include <libscc/dataflowanalysis.h>
#include <libscc/treenode.h>
#include <libscc/virtual_machine.h>

using namespace std;

void help (void) {
  cout <<
  "Usage: sca [options] [file]\n"
  "Options:\n"
  "  -h, --help           this help\n"
  "  -v, --verbose        some extra information\n"
  "      --print_ast      print abstract syntax tree\n"
  "  -e, --eval           evaluate the code\n"
  "  -a, --analysis       select analysis that you wish to enable\n"
  "                       Possible comma separated values are:\n"
  "                       \"rd\" for reaching definitions\n"
  "                       \"rj\" for reaching jumps\n"
  "                       \"rdc\" for reaching declassify\n"
  << endl;
}

int main(int argc, char *argv[]) {
    char* filename = 0;
    int verbose_flag = 0;
    int eval_flag = 0;
    int help_flag = 0;
    int run_analysis = 0;
    int print_ast = 0;
    int rd_flag = 0, rj_flag = 0, rdc_flag = 0;
    SecreC::TreeNodeProgram *parseTree = 0;

    int parseResult;

    while (1) {
      static struct option options[] = {
        {"verbose",      no_argument,       0,                  'v'},
        {"help",         no_argument,       0,                  'h'},
        {"print_ast",    no_argument,       &print_ast,          1 },
        {"eval",         no_argument,       0,                  'e'},
        {"analysis",     optional_argument, 0,                  'a'},
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
        case 'v':
          verbose_flag = 1;
          break;
        case 'e':
          eval_flag = 1;
          break;
        case 'h':
          help_flag = 1;
          break;
        case 'a':
          rd_flag = rj_flag = rdc_flag = 0;
          run_analysis = 0;
          while (1) {
            char const *token = strtok (str, ",");
            if (token == 0) break;
            if (strcmp (token, "rd") == 0)  { run_analysis = rd_flag = 1; }
            if (strcmp (token, "rj") == 0)  { run_analysis = rj_flag = 1; }
            if (strcmp (token, "rdc") == 0) { run_analysis = rdc_flag = 1; }
            str = NULL;
          }

          break;
        default:
          help ();
          return 1;
      }
    }

    if (help_flag) {
      help ();
      return 0;
    }

    if (optind < argc) {
      filename = argv[optind ++];
    }

    if (filename == 0) {
        parseResult = sccparse(&parseTree);
    } else {
        FILE *f = fopen(filename, "r");
        if (f != NULL) {
            if (verbose_flag) {
              cerr << "Parsing file: " << filename << endl;
            }

            parseResult = sccparse_file(f, &parseTree);
            fclose(f);
        } else {
            cerr << "Unable to open file: " << filename << endl;
            return 1;
        }
    }

    fflush(stdout);
    fflush(stderr);

    if (parseResult == 0) {
        assert(parseTree != 0);
        if (print_ast) {
          cout << parseTree->toString() << endl << endl;
        }

        SecreC::ICode icode;
        icode.init(parseTree);

        if (icode.status() == SecreC::ICode::OK) {
            if (verbose_flag) {
              cerr << "Valid intermediate code generated." << endl
                   << icode.compileLog();
            }

            // Print basic blocks:
            SecreC::Blocks bs;
            bs.init(icode.code());

            if (verbose_flag) {
                cerr << bs.toString() << endl;
            }

            if (eval_flag) {
                SecreC::VirtualMachine eval;
                eval.run(icode.code());
                if (verbose_flag) {
                    cerr << eval.toString();
                }
            }

            // Run data flow analysis and print the results:
            if (run_analysis) {
              SecreC::DataFlowAnalysisRunner runner;
              SecreC::ReachingDefinitions rd;
              SecreC::ReachingJumps rj;
              SecreC::ReachingDeclassify rdc;
              if (rd_flag)  runner.addAnalysis(&rd);
              if (rj_flag)  runner.addAnalysis(&rj);
              if (rdc_flag) runner.addAnalysis(&rdc);
              runner.run(bs);
              if (rd_flag)  cout << rd.toString(bs) << endl;
              if (rj_flag)  cout << rj.toString(bs) << endl;
              if (rdc_flag) cout << rdc.toString()  << endl;
            }
        } else {
            cerr << "Error generating valid intermediate code." << endl
                 << icode.compileLog();
        }
    }
    delete parseTree;

    return parseResult;
}
