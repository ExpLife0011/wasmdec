#ifndef _CODEGEN_H
#define _CODEGEN_H

#include "wasm-s-parser.h"
#include "wasm-builder.h"
#include "wasm-printing.h"
#include "wasm-io.h"
#include "wasm-validator.h"

#include "convert/Conversion.h"
#include "Emitter.h"
#include "DisasmConfig.h"

using namespace wasmdec;
using namespace std;

namespace wasmdec {
	class Decompiler {
	public:
		wasm::Module module;
		vector<char> binary;
		Emitter emit;
		Decompiler(DisasmConfig, vector<char>*);
		void gen();
		string getEmittedCode();
		bool failed();
		vector<char>* dumpMemory();
		vector<char>* dumpTable();
		DisasmMode mode;
		
		// Allocated as needed
		wasm::WasmBinaryBuilder* parser;
		wasm::SExpressionParser* sparser;
		wasm::SExpressionWasmBuilder* sbuilder;
	protected:
		void debug(string);
		void debugf(string);
		bool parserFailed;
		bool isDebug;
		bool emitExtraData;
		vector<char> rawTable;
		vector<char> rawMemory;
	};
} // namespace wasmdec

#endif // _CODEGEN_H