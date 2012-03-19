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

#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <libscc/context.h>
#include <libscc/intermediate.h>
#include <libscc/treenode.h>
#include <libscc/blocks.h>

#include <sharemind/libas/assemble.h>
#include <sharemind/libas/linker.h>
#include <sharemind/libas/tokenizer.h>

#include "Compiler.h"

using namespace std;
using namespace SecreCC;

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

namespace /* anonymous */ {

/*
 * RAII to remove a temporary path.
 */
class ScopedRemovePath : boost::noncopyable {
public: /* Methods: */
    ScopedRemovePath (const fs::path& path)
        : m_path (path) { }

    ~ScopedRemovePath () {
        boost::system::error_code ec;
        fs::remove (m_path, ec);
    }

private: /* Fields: */
    const fs::path& m_path;
};

/*
 * RAII for SharemindAssemblerTokens.
 */
class ScopedAsmTokens : boost::noncopyable {
public:
    explicit ScopedAsmTokens (SharemindAssemblerTokens* ts)
        : m_ts (ts) { }

    ~ScopedAsmTokens () {
        SharemindAssemblerTokens_free (m_ts);
    }

    SharemindAssemblerTokens* get () const { return m_ts; }
private:
    SharemindAssemblerTokens* const m_ts;
};

/*
 * RAII for SharemindAssemblerLinkingUnits.
 */
class ScopedAsmLinkingUnits : public SharemindAssemblerLinkingUnits, boost::noncopyable {
public:
    ScopedAsmLinkingUnits () {
        SharemindAssemblerLinkingUnits_init (this);
    }

    ~ScopedAsmLinkingUnits () {
        SharemindAssemblerLinkingUnits_destroy_with(this,
            &SharemindAssemblerLinkingUnit_destroy);
    }
};

/*
 * RAII to force C style free. auto_ptr uses delete which is incorrect.
 */
class ScopedFree : boost::noncopyable {
public:
    explicit ScopedFree (void* p)
        : m_ptr (p) { }
    ~ScopedFree () { free (m_ptr); }
    void* get () const { return m_ptr; }
private:
    void* const m_ptr;
};


/*
 * Collection of program options for a nice overview.
 */
struct ProgramOptions {
    bool                     showHelp;
    bool                     verbose;
    bool                     assembleOnly;
    boost::optional<string>  output; // nothing if cout
    boost::optional<string>  input; // nothing if cin
    vector<string>           includes;

    ProgramOptions ()
        : showHelp (false)
        , verbose (false)
        , assembleOnly (false)
    { }
};

/*
 * Parse program options. Returns false on failure.
 */
bool readProgramOptions (int argc, char *argv[], ProgramOptions* opts) {

     po::options_description desc ("Available options");
     desc.add_options ()
             ("help,h", "Display this help message.")
             ("verbose,v", "Enable verbose output.")
             ("include,I", po::value<vector<string> >(), "Directory for module search path.")
             ("assemble,S", "Output assembly.")
             ("output,o", po::value<string>(), "Output file.")
             ("input", po::value<string>(), "Input file.")
             ;
     po::positional_options_description p;
     p.add("input", -1);
     po::variables_map vm;

     try {
         po::store (po::command_line_parser (argc, argv).
            options (desc).positional (p).run (), vm);
         po::notify (vm);
     }
     catch (const std::exception& e) {
         cerr << e.what () << endl;
         cerr << desc << endl;
         return false;
     }
     catch (...) {
         cerr << desc << endl;
         return false;
     }

     if (vm.count ("help")) {
        opts->showHelp = true;
        cout << desc << endl;
     }

     opts->verbose = vm.count ("verbose");
     opts->assembleOnly = vm.count ("assemble");

     if (vm.count ("output")) {
         opts->output = vm["output"].as<string>();
     }

     if (vm.count ("input")) {
         opts->input = vm["input"].as<string>();
     }

     if (vm.count ("include")) {
         opts->includes = vm["include"].as<vector<string> >();
     }

    return true;
}

/*
 * Parse a secrec program, returns 0 on failure.
 */
auto_ptr<SecreC::TreeNodeModule> parseProgram (const ProgramOptions& opts, int* errorCode) {
    SecreC::TreeNodeModule* ast = 0;

    if (opts.input) {
        const std::string& fname = opts.input.get ();
        FILE* f = fopen(fname.c_str (), "r");
        if (f != NULL) {
            if (opts.verbose) {
                cerr << "Parsing file: \"" << fname << "\"... ";
                cerr << flush;
            }

            *errorCode = sccparse_file (f, &ast);
            fclose (f);
        } else {
            cerr << "Unable to open file \"" << fname << "\" for parsing." << endl;
        }
    }
    else {
        SecreC::TreeNodeModule* ast = 0;
        *errorCode = sccparse (&ast);
    }

    return auto_ptr<SecreC::TreeNodeModule> (ast);
}

/*
 * Compile the actual bytecode executable.
 */
bool compileExecutable (ostream& os, const VMLinkingUnit& vmlu) {
    fs::path p = fs::temp_directory_path () / fs::unique_path ();

    ScopedRemovePath scopedRemove (p);

    {
        io::stream<io::file_sink > fout (p.string ());

        if (! fout.is_open ()) {
            cerr << "Failed to open a temporary file \"" << p
                 << "\" for buffering!" << endl;
            return false;
        }

        fout << vmlu << flush;

        if (fout.bad ()) {
            cerr << "Writing to a temporary file \"" << p << "\" failed!" << std::endl;
            return false;
        }
    }

    ScopedAsmLinkingUnits lus;

    {
        io::stream<io::mapped_file_source > fin (p.string ());
        if (! fin.is_open ()) {
            cerr << "Failed to mmap a temporary file \"" << p
                 << "\" for reading!" << endl;
            return false;
        }

        size_t sl = 0u;
        size_t sc = 0u;
        ScopedAsmTokens ts (sharemind_assembler_tokenize (fin->data (), fin->size (), &sl, &sc));
        if (ts.get () == 0) {
            cerr << "ICE: Tokenization failed at (" << sl << "," << sc << ")" << endl;
            return false;
        }

        const SharemindAssemblerToken* errorToken = 0;
        char* errorString = 0;
        SharemindAssemblerError r = sharemind_assembler_assemble (ts.get (), &lus, &errorToken, &errorString);
        if (r != SHAREMIND_ASSEMBLE_OK) {
            const char* smasErrorStr = SharemindAssemblerError_toString (r);
            assert (smasErrorStr);

            cerr << "ICE: Assembling error: ";
            if (errorToken) {
                cerr << '(' << errorToken->start_line << ", "
                            << errorToken->start_column << ", "
                            << SharemindAssemblerTokenType_toString(errorToken->type) + 11
                     << ')';
            }

            cerr << smasErrorStr;
            if (errorString) cerr <<  ": " << errorString;
            cerr << endl;
            free (errorString);
            return false;
        }
    }

    size_t outputLength = 0;
    ScopedFree ptr (sharemind_assembler_link (0x0, &lus, &outputLength, 0));
    os.write (static_cast<const char*>(ptr.get ()), outputLength);
    if (os.bad ()) {
        cerr << "Writing bytecode to output failed." << endl;
        return false;
    }

    return true;
}

} // anonymous namespace


int main (int argc, char *argv[]) {
    ProgramOptions opts;

    if (! readProgramOptions (argc, argv, &opts)) {
        return EXIT_FAILURE;
    }

    if (opts.showHelp) {
        return EXIT_SUCCESS;
    }

    /* Get output stream: */
    ostream os (cout.rdbuf ());
    io::stream_buffer<io::file_sink> fileBuf;
    if (opts.output) {
        ios_base::openmode mode;
        if (! opts.assembleOnly) {
            mode |= ios_base::binary;
        }

        fileBuf.open (opts.output.get (), mode);
        if (! fileBuf.is_open ()) {
            cerr << "Failed to open the output file \"" << opts.output.get () << "\"." << endl;
            return EXIT_FAILURE;
        }

        os.rdbuf (&fileBuf);
    }

    /* Parse the program: */
    int parseErrorCode = 0;
    std::auto_ptr<SecreC::TreeNodeModule> parseTree = parseProgram (opts, &parseErrorCode);
    if (parseErrorCode != 0) {
        cerr << "Parsing failed! Error code " << parseErrorCode << "." << endl;
        return EXIT_FAILURE;
    }
    
    /* Translate to intermediate code: */
    SecreC::Context context;
    SecreC::ICode icode;

    /* Collect possible include files: */
    BOOST_FOREACH (const string& name, opts.includes) {
        if (! icode.modules ().addSearchPath (name) && opts.verbose) {
            cerr << "Invalid search path \"" << name << "\"." << endl;
        }
    }

    if (icode.init (context, parseTree.get ()) != SecreC::ICode::OK) {
        cerr << "Error generating valid intermediate code." << endl;
        cerr << icode.compileLog () << endl;
        return EXIT_FAILURE;
    }

    /* Compile: */
    VMLinkingUnit vmlu;
    Compiler compiler (icode);
    compiler.run (vmlu);

    if (opts.assembleOnly) {
        os << vmlu << endl;
    }
    else {
        if (! compileExecutable (os, vmlu)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
