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
#include <limits>
#include <locale>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>

#include <libscc/Blocks.h>
#include <libscc/Context.h>
#include <libscc/Intermediate.h>
#include <libscc/Location.h>
#include <libscc/StringTable.h>
#include <libscc/TreeNode.h>

#include <sharemind/libas/assemble.h>
#include <sharemind/libas/linker.h>
#include <sharemind/libas/tokenizer.h>
#include <sharemind/PotentiallyVoidTypeInfo.h>

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

using LocationPathStyle = SecreC::Location::PathStyle;

/*
 * Collection of program options for a nice overview.
 */
struct ProgramOptions {
    bool                     showHelp = false;
    bool                     verbose = false;
    bool                     assembleOnly = false;
    bool                     optimize = false;
    bool                     syntaxOnly = false;
    LocationPathStyle        runtimeErrorPathStyle = LocationPathStyle::FileName;
    boost::optional<string>  output; // nothing if cout
    boost::optional<string>  input; // nothing if cin
    vector<string>           includes;
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
            ("runtime-error-path-style", po::value<string>()->default_value("filename"),
             "Control how paths in SecreC runtime error messages are displayed. Either \"filename\" or \"fullpath\".")
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

        if (vm.count("runtime-error-path-style")) {
            auto const & style = vm["runtime-error-path-style"].as<string>();
            if (style == "filename")
                opts.runtimeErrorPathStyle = LocationPathStyle::FileName;
            else if (style == "fullpath")
                opts.runtimeErrorPathStyle = LocationPathStyle::FullPath;
            else {
                cerr << "Invalid runtime-error-path-style option \'" << style << "\'.";
                return false;
            }
        }

        if (vm.count("output"))
            opts.output = vm["output"].as<string>();

        if (vm.count("input"))
            opts.input = vm["input"].as<string>();

        if (vm.count ("include"))
            opts.includes = vm["include"].as<vector<string> >();

#ifndef SHAREMIND_STDLIB_PATH
#error "SHAREMIND_STDLIB_PATH not defined"
#endif
        if (!vm.count("no-stdlib"))
            opts.includes.push_back (SHAREMIND_STDLIB_PATH);

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

bool assemble(sharemind::Assembler::LinkingUnitsVector & lus,
              VMLinkingUnit const & vmlu)
{
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

    io::stream<io::mapped_file_source > fin (p.string ());
    if (! fin.is_open ()) {
        cerr << "Failed to mmap a temporary file \"" << p
             << "\" for reading!" << endl;
        return false;
    }

    lus = sharemind::Assembler::assemble(
              sharemind::Assembler::tokenize(fin->data(), fin->size()));
    return true;
}

/*
 * Compile the actual bytecode executable.
 */
bool compileExecutable (Output& output, const VMLinkingUnit& vmlu) {
    sharemind::Assembler::LinkingUnitsVector lus;
    if (!assemble(lus, vmlu))
        return false;

    auto const linkerResult(sharemind::Assembler::link(0x0, lus));
    auto readPtr = linkerResult.data();
    auto readSizeLeft = linkerResult.size();
    std::ostream& os = output.getStream ();
    static constexpr auto const streamSizeMax =
            std::numeric_limits<std::streamsize>::max();
    while (readSizeLeft > streamSizeMax) {
        os.write(readPtr, streamSizeMax);
        if (os.bad())
            goto err;
        readPtr = sharemind::ptrAdd(readPtr, streamSizeMax);
        readSizeLeft -= streamSizeMax;
    }
    os.write(readPtr, static_cast<std::streamsize>(readSizeLeft));
    if (os.bad())
        goto err;
    return true;

err:
    cerr << "Writing bytecode to output failed." << endl;
    return false;
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

    try {
        ProgramOptions opts;

        if (!readProgramOptions(argc, argv, opts))
            return EXIT_FAILURE;

        if (opts.showHelp)
            return EXIT_SUCCESS;

        Compiler compiler { opts.optimize };
        VMLinkingUnit vmlu;

        {
            SecreC::ICode icode;

            /* Parse the program: */
            SecreC::TreeNodeModule * parseTree = icode.parseMain (opts.input);
            if (icode.status () != SecreC::ICode::OK) {
                std::cerr << icode.compileLog ();
                return EXIT_FAILURE;
            }

            /* Collect possible include files: */
            for (const string& name : opts.includes) {
                icode.modules ().addSearchPath (name, opts.verbose);
            }

            /* TODO: We should split type checking and compilation entirely. */
            /* Translate to intermediate code: */
            icode.compile (parseTree, opts.runtimeErrorPathStyle);
            if (icode.status () != SecreC::ICode::OK) {
                cerr << "Error generating valid intermediate code." << endl;
                cerr << icode.compileLog () << endl;
                return EXIT_FAILURE;
            }

            if (opts.syntaxOnly)
                return EXIT_SUCCESS;

            /* Compile: */
            compiler.run (vmlu, icode);
        }

        /* Output: */
        Output output (opts);
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
    catch (const std::exception& e) {
        cerr << "Failed with exception:" << endl;
        cerr << e.what () << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << "Failed with unknown exception." << endl;
        return EXIT_FAILURE;
    }
}
