#ifdef PLUMB_DEBUG

#include "ui/app.hh"
#include "base/files.hh"
#include <windows.h>
#include <shellapi.h>

static int32_t argument_count = 0;
static wchar_t** arguments = nullptr;

static const wchar_t* argument ( const wchar_t* name ) {

	for ( int32_t i = 1; i + 1 < argument_count; i++ ) {

		if ( base::same_text ( arguments [ i ], name ) )
			return arguments [ i + 1 ];

	}

	return nullptr;

}

static float parse_float ( const wchar_t* text ) {

	float value = 0.0f;
	float scale = 0.0f;
	bool negative = false;

	if ( *text == L'-' ) {

		negative = true;
		text++;

	}

	for ( ; *text; text++ ) {

		if ( *text == L'.' ) {

			scale = 0.1f;

			continue;

		}

		if ( *text < L'0' || *text > L'9' )
			break;

		if ( scale > 0.0f ) {

			value += static_cast < float > ( *text - L'0' ) * scale;
			scale *= 0.1f;

		} else
			value = value * 10.0f + static_cast < float > ( *text - L'0' );

	}

	return negative ? -value : value;

}

struct script_reader {

	const wchar_t* cursor;

	bool word ( wchar_t* out, int32_t room ) {

		while ( *cursor == L' ' || *cursor == L';' || *cursor == L'\n' || *cursor == L'\r' || *cursor == L'\t' )
			cursor++;

		if ( !*cursor )
			return false;

		int32_t length = 0;

		while ( *cursor && *cursor != L' ' && *cursor != L';' && *cursor != L'\n' && *cursor != L'\r' && length < room - 1 )
			out [ length++ ] = *cursor++;

		out [ length ] = 0;

		return true;

	}

	float number ( ) {

		wchar_t token [ 64 ];

		if ( !word ( token, 64 ) )
			return 0.0f;

		return parse_float ( token );

	}

};

static uint32_t key_code ( const wchar_t* name, bool& control, bool& shift, bool& alt ) {

	control = false;
	shift = false;
	alt = false;

	while ( true ) {

		if ( name [ 0 ] == L'c' && name [ 1 ] == L't' && name [ 2 ] == L'r' && name [ 3 ] == L'l' && name [ 4 ] == L'+' ) {

			control = true;
			name += 5;

		} else if ( name [ 0 ] == L's' && name [ 1 ] == L'h' && name [ 2 ] == L'i' && name [ 3 ] == L'f' && name [ 4 ] == L't' && name [ 5 ] == L'+' ) {

			shift = true;
			name += 6;

		} else if ( name [ 0 ] == L'a' && name [ 1 ] == L'l' && name [ 2 ] == L't' && name [ 3 ] == L'+' ) {

			alt = true;
			name += 4;

		} else
			break;

	}

	const wchar_t* names [ ] = { L"left", L"right", L"up", L"down", L"escape", L"enter", L"pagedown", L"pageup", L"home", L"end", L"f1", L"f5", L"tab", L"back" };
	const uint32_t codes [ ] = { VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN, VK_ESCAPE, VK_RETURN, VK_NEXT, VK_PRIOR, VK_HOME, VK_END, VK_F1, VK_F5, VK_TAB, VK_BACK };

	for ( int32_t i = 0; i < 14; i++ ) {

		if ( base::same_text ( name, names [ i ] ) )
			return codes [ i ];

	}

	if ( name [ 0 ] && !name [ 1 ] )
		return static_cast < uint32_t > ( name [ 0 ] >= L'a' && name [ 0 ] <= L'z' ? name [ 0 ] - 32 : name [ 0 ] );

	return 0;

}

static bool capture_mode ( ) {

	const wchar_t* prefix = argument ( L"--capture" );

	if ( !prefix )
		return false;

	float width = 1440.0f;
	float height = 900.0f;
	float scale = 1.0f;
	const wchar_t* size_text = argument ( L"--size" );
	const wchar_t* scale_text = argument ( L"--scale" );
	const wchar_t* script = argument ( L"--script" );
	base::list < char > script_file;
	base::list < wchar_t > script_wide;

	if ( size_text ) {

		width = parse_float ( size_text );

		for ( const wchar_t* p = size_text; *p; p++ ) {

			if ( *p == L'x' ) {

				height = parse_float ( p + 1 );

				break;

			}

		}

	}

	if ( scale_text )
		scale = parse_float ( scale_text );

	const wchar_t* script_path = argument ( L"--script-file" );

	if ( script_path && files::read ( script_path, script_file ) ) {

		int32_t length = MultiByteToWideChar ( CP_UTF8, 0, script_file.data ( ), -1, nullptr, 0 );

		script_wide.resize ( static_cast < size_t > ( base::max ( length, 1 ) ) );
		script_wide [ 0 ] = 0;
		MultiByteToWideChar ( CP_UTF8, 0, script_file.data ( ), -1, script_wide.data ( ), length );
		script = script_wide.data ( );

	}

	render::gpu* gpu = new render::gpu ( );

	if ( !gpu->setup ( ) )
		return true;

	ui::app* app = new ui::app ( );

	app->fixed_step = true;
	app->persist = false;

	if ( !app->setup ( gpu, nullptr ) )
		return true;

	app->resize ( static_cast < int32_t > ( width * scale ), static_cast < int32_t > ( height * scale ), scale );

	double clock = 0.0;
	auto step = [ & ] ( int32_t frames ) {

		for ( int32_t i = 0; i < frames; i++ ) {

			clock += 1.0 / 60.0;
			app->frame ( clock );

		}

	};

	step ( 20 );

	script_reader reader = { script ? script : L"shot main" };
	wchar_t command [ 600 ];

	while ( reader.word ( command, 600 ) ) {

		if ( base::same_text ( command, L"wait" ) )
			step ( base::max ( static_cast < int32_t > ( reader.number ( ) * 60.0f + 0.5f ), 1 ) );
		else if ( base::same_text ( command, L"frames" ) )
			step ( static_cast < int32_t > ( reader.number ( ) ) );
		else if ( base::same_text ( command, L"open" ) ) {

			wchar_t path [ 600 ];

			reader.word ( path, 600 );

			for ( int32_t i = 0; path [ i ]; i++ ) {

				if ( path [ i ] == L'|' )
					path [ i ] = L' ';

			}

			app->open_file ( path );
			step ( 2 );

		} else if ( base::same_text ( command, L"theme" ) ) {

			wchar_t name [ 32 ];

			reader.word ( name, 32 );
			app->set_theme ( base::same_text ( name, L"light" ) );
			step ( 1 );

		} else if ( base::same_text ( command, L"move" ) ) {

			float x = reader.number ( );
			float y = reader.number ( );

			app->mouse_move ( x * scale, y * scale );
			step ( 1 );

		} else if ( base::same_text ( command, L"click" ) ) {

			float x = reader.number ( );
			float y = reader.number ( );

			app->mouse_move ( x * scale, y * scale );
			step ( 2 );
			app->mouse_button ( 0, true );
			step ( 1 );
			app->mouse_button ( 0, false );
			step ( 1 );

		} else if ( base::same_text ( command, L"down" ) ) {

			app->mouse_button ( 0, true );
			step ( 1 );

		} else if ( base::same_text ( command, L"up" ) ) {

			app->mouse_button ( 0, false );
			step ( 1 );

		} else if ( base::same_text ( command, L"leave" ) ) {

			app->mouse_leave ( );
			step ( 1 );

		} else if ( base::same_text ( command, L"wheel" ) ) {

			app->mouse_wheel ( reader.number ( ), false );
			step ( 1 );

		} else if ( base::same_text ( command, L"key" ) ) {

			wchar_t name [ 64 ];
			bool control = false;
			bool shift = false;
			bool alt = false;

			reader.word ( name, 64 );

			uint32_t code = key_code ( name, control, shift, alt );

			if ( code )
				app->key ( code, control, shift, alt );

			step ( 1 );

		} else if ( base::same_text ( command, L"type" ) ) {

			wchar_t text [ 256 ];

			reader.word ( text, 256 );

			for ( int32_t i = 0; text [ i ]; i++ ) {

				app->character ( text [ i ] == L'_' ? L' ' : text [ i ] );
				step ( 2 );

			}

		} else if ( base::same_text ( command, L"run" ) ) {

			app->run ( static_cast < ui::command > ( static_cast < int32_t > ( reader.number ( ) ) ) );
			step ( 1 );

		} else if ( base::same_text ( command, L"size" ) ) {

			width = reader.number ( );
			height = reader.number ( );
			app->resize ( static_cast < int32_t > ( width * scale ), static_cast < int32_t > ( height * scale ), scale );
			step ( 2 );

		} else if ( base::same_text ( command, L"shot" ) ) {

			wchar_t name [ 64 ];
			base::writer line;

			reader.word ( name, 64 );
			line.add ( prefix ).add ( L"_" ).add ( name ).add ( L".png" );
			gpu->save ( gpu->chrome, line.text ( ), app->tones [ static_cast < int32_t > ( ui::tone::BASE ) ], app->width, app->height );

		}

	}

	return true;

}

bool debug_main ( ) {

	arguments = CommandLineToArgvW ( GetCommandLineW ( ), &argument_count );

	if ( !arguments || argument_count < 2 )
		return false;

	return capture_mode ( );

}

#endif