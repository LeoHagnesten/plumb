#include "ui/app.hh"
#include "ui/icons.hh"

namespace ui {

	void app::open_prompt ( command action, uint64_t address ) {

		prompt_open = true;
		prompt_action = action;
		prompt_address = address;
		prompt_field = field ( );

		const wchar_t* fill = nullptr;
		wchar_t name [ 200 ];

		if ( action == command::RENAME ) {

			if ( symbol_name ( address, true, name, 200 ) )
				fill = name;

		} else if ( action == command::COMMENT )
			fill = user_text ( comments, address );
		else if ( action == command::FIND_SIGNATURE && !search_query.empty ( ) )
			fill = search_query.get ( );

		if ( fill ) {

			int32_t i = 0;

			for ( ; fill [ i ] && i < 250; i++ )
				prompt_field.text [ i ] = fill [ i ];

			prompt_field.text [ i ] = 0;
			prompt_field.length = i;

		}

		focus_field ( make_id ( "prompt" ), prompt_field, true );

	}

	void app::apply_prompt ( ) {

		wchar_t text [ 256 ];
		int32_t length = 0;

		for ( int32_t i = 0; i < prompt_field.length; i++ ) {

			wchar_t c = prompt_field.text [ i ];

			if ( prompt_action == command::RENAME ) {

				bool word = ( c >= L'a' && c <= L'z' ) || ( c >= L'A' && c <= L'Z' ) || ( c >= L'0' && c <= L'9' ) || c == L'_' || c == L':' || c == L'@' || c == L'?' || c == L'$' || c == L'~';

				if ( !word )
					c = L'_';

			}

			text [ length++ ] = c;

		}

		text [ length ] = 0;
		prompt_open = false;
		focus_id = 0;
		focused_field = nullptr;

		switch ( prompt_action ) {

			case command::RENAME:

				if ( length && text [ 0 ] >= L'0' && text [ 0 ] <= L'9' ) {

					log ( log_kind::WARN, L"Names can't start with a digit." );

					return;

				}

				set_user_text ( renamed, prompt_address, text );
				refresh_filter ( panel::FUNCTIONS );
				save_project ( );
				log ( log_kind::INFO, length ? base::writer ( ).add ( L"Renamed to " ).add ( text ).text ( ) : L"Name reset." );
				break;

			case command::COMMENT:
				set_user_text ( comments, prompt_address, text );
				save_project ( );
				break;

			case command::FIND_SIGNATURE:
				find_signature ( text );
				break;

			default:
				break;

		}

		pending = base::max ( pending, 3 );

	}

	void app::make_signature ( uint64_t address ) {

		if ( !db || !analysis_applied )
			return;

		double started = base::seconds ( );

		signature_ok = analysis::make_signature ( *db, address, signature_result );
		signature_seconds = base::seconds ( ) - started;
		signature_open = true;
		signature_copied = -1;

		base::list < wchar_t > text;

		analysis::format_pattern ( signature_result.bytes, analysis::pattern_style::IDA, text );

		if ( signature_ok )
			log ( log_kind::GOOD, base::writer ( ).add ( signature_result.via_reference ? L"Signature through a caller: " : L"Signature: " ).add ( text.data ( ) ).text ( ) );
		else
			log ( log_kind::WARN, base::writer ( ).add ( L"No unique signature here, best has " ).number ( signature_result.matches ).add ( L" matches." ).text ( ) );

	}

	void app::find_signature ( const wchar_t* text ) {

		if ( !file.loaded ( ) )
			return;

		analysis::pattern find;

		if ( !analysis::parse_pattern ( text, find ) ) {

			log ( log_kind::FAIL, base::writer ( ).add ( L"Can't read \"" ).add ( text ).add ( L"\" as a pattern. Try 48 8B 05 ? ? ? ? or \"text\"." ).text ( ) );

			return;

		}

		double started = base::seconds ( );
		base::list < uint64_t > hits;
		const int32_t limit = 20000;

		analysis::scan ( file, find, false, hits, limit );

		bool plain = text [ 0 ] == L'"';

		for ( const wchar_t* at = text; *at && plain; at++ ) {

			if ( at [ 0 ] == L'\\' && at [ 1 ] == L'x' )
				plain = false;

		}

		if ( plain ) {

			analysis::pattern wide;
			base::list < uint64_t > more;

			for ( int32_t i = 0; i < find.bytes.count ( ); i++ ) {

				wide.bytes.push ( find.bytes [ static_cast < size_t > ( i ) ] );
				wide.mask.push ( 1 );
				wide.bytes.push ( 0 );
				wide.mask.push ( 1 );

			}

			analysis::scan ( file, wide, false, more, limit );

			for ( uint64_t value : more )
				hits.push ( value );

		}

		search_hits.clear ( );

		for ( uint64_t value : hits ) {

			search_hit& item = search_hits.emplace ( );

			item.address = value;
			item.length = static_cast < uint32_t > ( find.bytes.count ( ) );

		}

		search_seconds = base::seconds ( ) - started;
		search_truncated = hits.count ( ) >= limit;
		search_query.set ( text );
		browsing = panel::SEARCH;
		browser_open = true;
		browser_scroll [ static_cast < int32_t > ( panel::SEARCH ) ] = scroller ( );
		refresh_filter ( panel::SEARCH );

		base::writer line;

		line.grouped ( static_cast < uint64_t > ( search_hits.count ( ) ) ).add ( search_hits.count ( ) == 1 ? L" match" : L" matches" ).add ( search_truncated ? L" (stopped at the limit)" : L"" ).add ( L" in " ).fixed ( search_seconds * 1000.0, 1 ).add ( L" ms" );
		log ( search_hits.empty ( ) ? log_kind::WARN : log_kind::GOOD, line.text ( ) );

		if ( search_hits.count ( ) == 1 )
			go ( search_hits [ 0 ].address, true );

	}

}