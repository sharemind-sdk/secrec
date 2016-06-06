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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <locale>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <libscc/Context.h>
#include <libscc/Intermediate.h>
#include <libscc/TreeNode.h>
#include <libscc/Blocks.h>
#include <libscc/StringTable.h>

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
        SharemindAssemblerLinkingUnits_destroy(this);
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
    bool                     optimize;
    bool                     syntaxOnly;
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
bool readProgramOptions(int argc, char * argv[], ProgramOptions & opts) {

    po::options_description desc ("Available options");
    desc.add_options ()
            ("help,h", "Display this help message.")
            ("verbose,v", "Enable verbose output.")
            ("include,I", po::value<vector<string> >(), "Directory for module search path.")
            ("assemble,S", "Output assembly.")
            ("output,o", po::value<string>(), "Output file.")
            ("input", po::value<string>(), "Input file.")
            ("no-stdlib", "Do not look for standard library imports.")
            ("optimize,O", "Optimize the generated code.")
            ("syntax-only", "Parse and type check only. Do not generate code.")
            ;
    po::positional_options_description p;
    p.add("input", -1);
    po::variables_map vm;

    try {

        po::store(po::command_line_parser(argc, argv)
                      .options(desc)
                      .positional(p)
                      .style(po::command_line_style::default_style ^ po::command_line_style::allow_guessing)
                      .run(),
                  vm);
        po::notify(vm);

        if (vm.count("help")) {
           opts.showHelp = true;
           cout << desc << endl;
           return true;
        }

        opts.verbose = vm.count("verbose");
        opts.assembleOnly = vm.count("assemble");
        opts.optimize = vm.count ("optimize");
        opts.syntaxOnly = vm.count("syntax-only");

        if (vm.count("output"))
            opts.output = vm["output"].as<string>();

        if (vm.count("input"))
            opts.input = vm["input"].as<string>();

        if (vm.count("include"))
            opts.includes = vm["include"].as<vector<string> >();


#ifndef SHAREMIND_STDLIB_PATH
#error "SHAREMIND_STDLIB_PATH not defined"
#else
        if (!vm.count("no-stdlib"))
            opts.includes.push_back (SHAREMIND_STDLIB_PATH);
#endif
        return true;
    }
    catch (const std::exception & e) {
        cerr << e.what() << endl;
        cerr << desc << endl;
        return false;
    }
}

/*
 * Lazy output.
 */
class Output {
    Output(const Output&);
    void operator= (const Output&);
public: /* Methods: */
    Output (const ProgramOptions& opts)
        : m_os (cout.rdbuf ())
        , m_fileBuf()
        , m_opts (opts)
        , m_fileOpened (false)
    { }

    std::ostream& getStream () {
        if (!m_fileOpened && m_opts.output) {
            ios_base::openmode mode = ios_base::out;
            m_fileOpened = true;
            if (! m_opts.assembleOnly) {
                mode |= ios_base::binary;
            }

            m_fileBuf.open (m_opts.output.get (), mode);
            if (! m_fileBuf.is_open ()) {
                cerr << "Failed to open output file \""
                     << m_opts.output.get () << "\"." << endl;
                m_os.setstate(ios::failbit);
                return m_os;
            }

            m_os.rdbuf (&m_fileBuf);
        }

        return m_os;
    }

private: /* Fields: */
    std::ostream m_os;
    io::stream_buffer<io::file_sink> m_fileBuf;
    const ProgramOptions& m_opts;
    bool m_fileOpened;
};

/*
 * Compile the actual bytecode executable.
 */
bool compileExecutable (Output& output, const VMLinkingUnit& vmlu) {
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
            cerr << "Writing to a temporary file \"" << p << "\" failed!" << endl;
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
        if (ts.get () == NULL) {
            cerr << "ICE: Tokenization failed at (" << sl << "," << sc << ")" << endl;
            return false;
        }

        const SharemindAssemblerToken* errorToken = NULL;
        char* errorString = NULL;
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
    std::ostream& os = output.getStream ();
    os.write (static_cast<const char*>(ptr.get ()), outputLength);
    if (os.bad ()) {
        cerr << "Writing bytecode to output failed." << endl;
        return false;
    }

    return true;
}

} // anonymous namespace


int main (int argc, char *argv[]) {
    try {
        std::locale("");
    } catch (std::exception const & e) {
        std::cerr << "Invalid default locale from environment: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Invalid default locale from environment!" << std::endl;
        return EXIT_FAILURE;
    }
    ProgramOptions opts;

    if (!readProgramOptions(argc, argv, opts))
        return EXIT_FAILURE;

    if (opts.showHelp)
        return EXIT_SUCCESS;

    Output output (opts);
    SecreC::ICode icode;

    /* Parse the program: */
    SecreC::TreeNodeModule * parseTree = icode.parseMain (opts.input);
    if (icode.status () != SecreC::ICode::OK) {
        std::cerr << icode.compileLog ();
        return EXIT_FAILURE;
    }

    /* Collect possible include files: */
    for (const string& name : opts.includes) {
        if (! icode.modules ().addSearchPath (name) && opts.verbose) {
            cerr << "Invalid search path \"" << name << "\"." << endl;
        }
    }

    /* TODO: We should split type checking and compilation entirely. */
    /* Translate to intermediate code: */
    icode.compile (parseTree);
    if (icode.status () != SecreC::ICode::OK) {
        cerr << "Error generating valid intermediate code." << endl;
        cerr << icode.compileLog () << endl;
        return EXIT_FAILURE;
    }

    if (opts.syntaxOnly)
        return EXIT_SUCCESS;

    /* Compile: */
    VMLinkingUnit vmlu;
    Compiler compiler (icode, opts.optimize);
    compiler.run (vmlu);

    /* Output: */
    if (opts.assembleOnly) {
        output.getStream() << vmlu << endl;
    }
    else {
        if (! compileExecutable (output, vmlu)) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
