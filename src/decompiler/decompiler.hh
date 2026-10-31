#pragma once

#include "base/base.hh"
#include "loader/binary.hh"
#include "analysis/database.hh"

namespace decompiler {

	enum class syntax : uint8_t {

		PLAIN,
		KEYWORD,
		CONTROL,
		TYPE,
		FUNCTION,
		LOCAL,
		PARAMETER,
		GLOBAL,
		NUMBER,
		STRING,
		COMMENT,
		OPERATOR,
		LABEL,
		IMPORT,

	};

	struct span {

		int32_t start = 0;
		int32_t length = 0;
		syntax kind = syntax::PLAIN;
		uint64_t target = 0;

	};

	struct line {

		int32_t text = 0;
		int32_t length = 0;
		int32_t first_span = 0;
		int32_t span_count = 0;
		int32_t indent = 0;
		uint64_t address = 0;

	};

	struct result {

		uint64_t function = 0;
		base::list < wchar_t > text;
		base::list < span > spans;
		base::list < line > lines;
		int32_t instructions = 0;
		int32_t blocks = 0;
		int32_t gotos = 0;
		int32_t locals = 0;
		double seconds = 0.0;
		uint32_t revision = 0;
		bool ok = false;

	};

	struct namer {

		virtual bool name ( uint64_t address, bool code, wchar_t* out, int32_t room ) = 0;

		virtual bool literal ( uint64_t address, wchar_t* out, int32_t room ) = 0;

		virtual const wchar_t* local_name ( uint64_t function, int32_t slot ) = 0;

	};

	class cache {

	public:

		cache ( ) = default;

		cache ( const cache& ) = delete;

		cache& operator= ( const cache& ) = delete;

		~cache ( );

		result* find ( uint64_t function, uint32_t revision );

		result* make ( uint64_t function );

		void clear ( );

	private:

		base::list < result* > entries;

	};

	bool decompile ( const analysis::database& db, uint64_t function, namer& names, result& out );

}