#define __WASMDEC_VERSION "1.0.0"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <iterator>

#include "Decompiler.h"
#include "asm2wasm.h"

bool readFile(vector<char>* data, string path) {
	ifstream file(path);
	if (!file.eof() && !file.fail()) {
	    file.seekg(0, ios_base::end);
	    streampos fileSize = file.tellg();
	    data->resize(fileSize);
	    file.seekg(0, ios_base::beg);
	    file.read(&(data->operator[](0)), fileSize);
		return true;
	} else {
		return false;
	}
}
bool writeFile(string path, string data) {
	ofstream file(path);
	if (!file.eof() && !file.fail()) {
		file << data;
		file.close();
		return true;
	} else {
		return false;
	}
}
string getFileExt(string fname) {
	string::size_type idx = fname.rfind('.');
	if(idx != string::npos) {
	    string extension = fname.substr(idx + 1);
	    return extension;
	} else {
	    return "";
	}
}
int usage() {
	cerr << "Usage:" << endl
	<< "wasmdec {arguments}, where arguments is one of:" << endl
	<< "\t -o / --out <outfile> : Path to output file (Required)" << endl
	<< "\t -i / --in <infile> : Path to input file (Required)" << endl
	<< "\t -d / --debug : Enable debug output" << endl
	<< "\t -e / --extra : Emit extra data into outfile" << endl
	<< "\t -m / --memdump : Dump the input file's memory to disk" << endl
	<< "\t -h / --help : Print usage information" << endl
	<< "\t -v / --version : Print the program's version" << endl;
	return 1;
}
int usage(int optionalRetVal) {
	usage();
	return optionalRetVal;
}
int main(int argc, const char** argv) {
	string infile, outfile;
	bool enableDebugging = false,
		enableExtra = false,
		enableMemdump = false;
	if (argc == 2) {
		string argv1 = string(argv[1]);
		if (argv1 == "-h" || argv1 == "--help") return usage(0);
		if (argv1 == "-v" || argv1 == "--version") {
			cerr << "wasmdec v" << __WASMDEC_VERSION << endl;
			return 0;
		}
	}
	if (argc < 2) return usage();
	for (int i = 0; i < argc; ++i) {
		string sarg = string(argv[i]);
		if (sarg == "-d" || sarg == "--debug") {
			enableDebugging = true;
		} else if (sarg == "-e" || sarg == "--extra") {
			enableExtra = true;
		} else if (sarg == "-o" || sarg == "--out") {
			int outnameIndex = i + 1;
			if (outnameIndex >= argc) {
				return usage(); // Invalid arguments
			}
			// Set output file
			outfile = string(argv[outnameIndex]);
		} else if (sarg == "-i" || sarg == "--in") {
			int innameIndex = i + 1;
			if (innameIndex >= argc) {
				return usage(); // Invalid arguments
			}
			// Set input file
			infile = string(argv[innameIndex]);
		} else if (sarg == "-m" || sarg == "--memdump") {
			enableMemdump = true;
		}
	}
	if (!outfile.length()) {
		cerr << "Missing output file!";
		if (enableMemdump) {
			cerr << " You must provide an output file when using memdump!" << endl;
		} else {
			cerr << endl;
		}
		return usage();
	}
	if (!infile.length()) {
		cerr << "Missing input file!" << endl;
		return usage();
	}
	vector<char> vfile = vector<char>();
	bool rsuccess = readFile(&vfile, infile);
	if (!rsuccess) {
		cerr << "wasmdec: Failed to read file '" << infile << "'" << endl;
		return 1;
	}
	string ext = getFileExt(infile);
	DisasmMode mode = DisasmMode::None;
	// Convert file extension to disassembler mode
	if (ext == "wasm") {
		mode = DisasmMode::Wasm;
	} else if (ext == "wast") {
		mode = DisasmMode::Wast;
	} else if (ext == "js") {
		mode = DisasmMode::AsmJs;
	} else {
		cerr << "wasmdec: Invalid input file extension, aborting." << endl;
		return 1;
	}
	DisasmConfig config(enableDebugging, enableExtra, mode);
	Decompiler decomp(config, &vfile);
	if (decomp.failed()) {
		cerr << "wasmdec: Code generation failed, aborting." << endl;
		return 1;
	}
	
	if (enableMemdump) {
		// If memdump is enabled, ONLY dump the binary's memory and exit
		vector<char>* rawmem = decomp.dumpMemory();
		if (!rawmem) {
			cerr << "wasmdec: FATAL: failed to dump memory" << endl;
			return usage();
		}
		vector<char>* rawtable = decomp.dumpTable();
		if (!rawtable) {
			cerr << "wasmdec: FATAL: failed to dump table" << endl;
			return usage();
		}
		string outMemFile = outfile + ".mem.bin";
		string outTableFile = outfile + ".table.bin";
		bool memWriteSuccess = writeFile(outMemFile, string(rawmem->begin(), rawmem->end()));
		bool tableWriteSuccess = writeFile(outTableFile, string(rawtable->begin(), rawtable->end()));
		if (!memWriteSuccess || !tableWriteSuccess) {
			cerr << "Failed to write one or more memory dump files, aborting." << endl;
			return 1;
		}
	} else {
		// Otherwise we can normally decompile the binary
		decomp.gen();
		auto res = decomp.getEmittedCode();
		bool wsuccess = writeFile(outfile, res);
		if (!wsuccess) {
			cerr << "wasmdec: Failed to write file '" << outfile << "'" << endl;
			return 1;
		}
	}
	return 0;
}