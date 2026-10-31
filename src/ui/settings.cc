#include "ui/app.hh"
#include "ui/icons.hh"
#include "base/files.hh"
#include <windows.h>

namespace ui {

	void app::remember_recent ( const wchar_t* path ) {

		for ( int32_t i = 0; i < recent.count ( ); i++ ) {

			if ( base::same_text ( recent [ static_cast < size_t > ( i ) ].get ( ), path ) ) {

				recent.remove_at ( static_cast < size_t > ( i ) );

				break;

			}

		}

		base::list < base::text > next;

		next.push ( base::text ( path ) );

		for ( const base::text& item : recent ) {

			if ( next.count ( ) < 8 )
				next.push ( item );

		}

		recent = base::move ( next );

		if ( persist )
			save_settings ( );

	}

	void app::forget_recent ( int32_t index ) {

		if ( index < 0 || index >= recent.count ( ) )
			return;

		recent.remove_at ( static_cast < size_t > ( index ) );

		if ( persist )
			save_settings ( );

	}

	static void project_path ( const loader::binary& file, wchar_t* out, int32_t room ) {

		uint64_t hash = 0xCBF29CE484222325ull;

		for ( const wchar_t* at = file.path.get ( ); *at; at++ ) {

			wchar_t c = *at >= L'A' && *at <= L'Z' ? static_cast < wchar_t > ( *at + 32 ) : *at;

			hash ^= static_cast < uint64_t > ( c );
			hash *= 0x100000001B3ull;

		}

		hash ^= file.size;
		hash *= 0x100000001B3ull;

		base::writer target;

		target.add ( files::folder ( ) ).add ( L"\\project_" ).hex ( hash, 16 ).add ( L".txt" );

		for ( int32_t i = 0; i <= target.size ( ) && i < room; i++ )
			out [ i ] = target.text ( ) [ i ];

	}

	void app::save_project ( ) {

		if ( !persist || !file.loaded ( ) )
			return;

		base::list < char > out;
		wchar_t path [ 600 ];
		char utf8 [ 1200 ];

		auto write_list = [ & ] ( const char* key, const base::list < user_name >& list ) {

			for ( const user_name& item : list ) {

				base::writer line;

				line.hex ( item.address ).add ( L" " ).add ( item.text.get ( ) );

				for ( const char* at = key; *at; at++ )
					out.push ( *at );

				out.push ( ' ' );

				int32_t length = WideCharToMultiByte ( CP_UTF8, 0, line.text ( ), line.size ( ), utf8, 1199, nullptr, nullptr );

				for ( int32_t i = 0; i < length; i++ )
					out.push ( utf8 [ i ] == '\n' ? ' ' : utf8 [ i ] );

				out.push ( '\n' );

			}

		};

		write_list ( "name", renamed );
		write_list ( "comment", comments );
		write_list ( "bookmark", bookmarks );
		write_list ( "number", number_forms );
		project_path ( file, path, 600 );

		if ( out.empty ( ) ) {

			DeleteFileW ( path );

			return;

		}

		files::write_later ( path, out.data ( ), out.count ( ) );

	}

	void app::load_project ( ) {

		renamed.clear ( );
		comments.clear ( );
		bookmarks.clear ( );
		number_forms.clear ( );

		if ( !file.loaded ( ) )
			return;

		wchar_t path [ 600 ];
		base::list < char > data;

		project_path ( file, path, 600 );

		if ( !files::read ( path, data ) )
			return;

		const char* cursor = data.data ( );

		while ( *cursor ) {

			const char* line = cursor;

			while ( *cursor && *cursor != '\n' )
				cursor++;

			int32_t length = static_cast < int32_t > ( cursor - line );

			if ( *cursor )
				cursor++;

			wchar_t wide [ 1200 ];
			int32_t made = MultiByteToWideChar ( CP_UTF8, 0, line, length, wide, 1199 );

			wide [ made > 0 ? made : 0 ] = 0;

			wchar_t* at = wide;
			wchar_t* key = at;

			while ( *at && *at != L' ' )
				at++;

			if ( !*at )
				continue;

			*at++ = 0;

			uint64_t address = 0;

			while ( *at && *at != L' ' ) {

				wchar_t c = *at++;

				address = ( address << 4 ) | static_cast < uint64_t > ( c >= L'a' ? c - L'a' + 10 : ( c >= L'A' ? c - L'A' + 10 : c - L'0' ) );

			}

			if ( *at == L' ' )
				at++;

			while ( made > 0 && ( wide [ made - 1 ] == L'\r' ) )
				wide [ --made ] = 0;

			if ( base::same_text ( key, L"name" ) )
				set_user_text ( renamed, address, at );
			else if ( base::same_text ( key, L"comment" ) )
				set_user_text ( comments, address, at );
			else if ( base::same_text ( key, L"bookmark" ) )
				set_user_text ( bookmarks, address, at );
			else if ( base::same_text ( key, L"number" ) )
				set_user_text ( number_forms, address, at );

		}

	}

	static void put_line ( base::list < char >& out, const char* key, const wchar_t* value ) {

		for ( const char* at = key; *at; at++ )
			out.push ( *at );

		out.push ( '=' );

		char utf8 [ 1024 ];
		int32_t length = WideCharToMultiByte ( CP_UTF8, 0, value, -1, utf8, 1023, nullptr, nullptr );

		for ( int32_t i = 0; i < length - 1; i++ )
			out.push ( utf8 [ i ] );

		out.push ( '\n' );

	}

	void app::save_settings ( ) {

		base::list < char > out;
		base::writer value;
		wchar_t path [ 600 ];

		put_line ( out, "theme", light ? L"light" : L"dark" );
		value.number ( static_cast < int64_t > ( browser_width ) );
		put_line ( out, "browser", value.text ( ) );
		value.clear ( );
		value.number ( static_cast < int64_t > ( inspector_width ) );
		put_line ( out, "inspector", value.text ( ) );
		value.clear ( );
		value.number ( static_cast < int64_t > ( output_height ) );
		put_line ( out, "output", value.text ( ) );
		value.clear ( );
		value.number ( ( browser_open ? 1 : 0 ) | ( inspector_open ? 2 : 0 ) | ( output_open ? 4 : 0 ) );
		put_line ( out, "panels", value.text ( ) );

		if ( placement_valid ) {

			value.clear ( );

			for ( int32_t i = 0; i < 4; i++ ) {

				if ( i )
					value.add ( L',' );

				value.number ( placement [ i ] );

			}

			put_line ( out, "window", value.text ( ) );
			put_line ( out, "maximized", placement_maximized ? L"1" : L"0" );

		}

		for ( const base::text& item : recent )
			put_line ( out, "recent", item.get ( ) );

		base::writer target;

		target.add ( files::folder ( ) ).add ( L"\\settings.ini" );

		for ( int32_t i = 0; i <= target.size ( ); i++ )
			path [ i ] = target.text ( ) [ i ];

		files::write_later ( path, out.data ( ), out.count ( ) );

	}

	static int64_t read_integer ( const wchar_t* text ) {

		int64_t value = 0;
		bool negative = *text == L'-';

		if ( negative )
			text++;

		while ( *text >= L'0' && *text <= L'9' )
			value = value * 10 + ( *text++ - L'0' );

		return negative ? -value : value;

	}

	void app::load_settings ( ) {

		base::list < char > data;
		base::writer target;

		target.add ( files::folder ( ) ).add ( L"\\settings.ini" );

		if ( !files::read ( target.text ( ), data ) )
			return;

		const char* cursor = data.data ( );

		while ( *cursor ) {

			const char* name = cursor;

			while ( *cursor && *cursor != '=' && *cursor != '\n' )
				cursor++;

			if ( *cursor != '=' ) {

				while ( *cursor && *cursor != '\n' )
					cursor++;

				if ( *cursor )
					cursor++;

				continue;

			}

			int32_t name_length = static_cast < int32_t > ( cursor - name );
			const char* start = ++cursor;

			while ( *cursor && *cursor != '\n' && *cursor != '\r' )
				cursor++;

			wchar_t value [ 1024 ];
			int32_t made = MultiByteToWideChar ( CP_UTF8, 0, start, static_cast < int32_t > ( cursor - start ), value, 1023 );

			value [ made > 0 ? made : 0 ] = 0;

			while ( *cursor == '\r' || *cursor == '\n' )
				cursor++;

			auto named = [ & ] ( const char* key ) {

				int32_t i = 0;

				for ( ; key [ i ]; i++ ) {

					if ( i >= name_length || key [ i ] != name [ i ] )
						return false;

				}

				return i == name_length;

			};

			if ( named ( "theme" ) )
				light = base::same_text ( value, L"light" );
			else if ( named ( "browser" ) )
				browser_width = base::clamp ( static_cast < float > ( read_integer ( value ) ), 200.0f, 600.0f );
			else if ( named ( "inspector" ) )
				inspector_width = base::clamp ( static_cast < float > ( read_integer ( value ) ), 220.0f, 600.0f );
			else if ( named ( "output" ) )
				output_height = base::clamp ( static_cast < float > ( read_integer ( value ) ), 90.0f, 600.0f );
			else if ( named ( "panels" ) ) {

				int64_t flags = read_integer ( value );

				browser_open = ( flags & 1 ) != 0;
				inspector_open = ( flags & 2 ) != 0;
				output_open = ( flags & 4 ) != 0;

			} else if ( named ( "window" ) ) {

				const wchar_t* at = value;

				for ( int32_t i = 0; i < 4; i++ ) {

					placement [ i ] = static_cast < int32_t > ( read_integer ( at ) );

					while ( *at && *at != L',' )
						at++;

					if ( *at == L',' )
						at++;

				}

				placement_valid = placement [ 2 ] > 300 && placement [ 3 ] > 200;

			} else if ( named ( "maximized" ) )
				placement_maximized = value [ 0 ] == L'1';
			else if ( named ( "recent" ) && value [ 0 ] && recent.count ( ) < 8 )
				recent.push ( base::text ( value ) );

		}

		theme_mix = light ? 1.0f : 0.0f;
		browser_show = browser_open ? 1.0f : 0.0f;
		inspector_show = inspector_open ? 1.0f : 0.0f;
		output_show = output_open ? 1.0f : 0.0f;
		update_theme ( );

	}

}