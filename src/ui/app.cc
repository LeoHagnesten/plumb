#include "ui/app.hh"
#include "ui/icons.hh"
#include "base/files.hh"
#include <windows.h>

namespace ui {

	struct command_info {

		const wchar_t* label;
		const wchar_t* keys;

	};

	static const command_info commands [ static_cast < int32_t > ( command::COUNT ) ] = {
		{ L"Open file", L"Ctrl+O" },
		{ L"Close file", L"Ctrl+W" },
		{ L"Exit", L"Alt+F4" },
		{ L"Copy", L"Ctrl+C" },
		{ L"Go to address", L"G" },
		{ L"Go to entry point", L"Ctrl+E" },
		{ L"Back", L"Alt+Left" },
		{ L"Forward", L"Alt+Right" },
		{ L"Next section", L"Ctrl+PgDn" },
		{ L"Previous section", L"Ctrl+PgUp" },
		{ L"Find in strings", L"Ctrl+F" },
		{ L"Command palette", L"Ctrl+Shift+P" },
		{ L"Listing", L"Ctrl+1" },
		{ L"Hex", L"Ctrl+2" },
		{ L"Headers", L"Ctrl+3" },
		{ L"Graph", L"Ctrl+4" },
		{ L"Pseudocode", L"Ctrl+5" },
		{ L"Functions", L"Alt+1" },
		{ L"Imports", L"Alt+2" },
		{ L"Exports", L"Alt+3" },
		{ L"Strings", L"Alt+4" },
		{ L"Sections", L"Alt+5" },
		{ L"Toggle browser", L"Ctrl+B" },
		{ L"Toggle inspector", L"Ctrl+I" },
		{ L"Toggle output", L"Ctrl+J" },
		{ L"Switch light or dark theme", L"Ctrl+Shift+L" },
		{ L"Analyze image", L"" },
		{ L"Decode at cursor", L"C" },
		{ L"Keyboard shortcuts", L"F1" },
		{ L"About Plumb", L"" }
	};

	const wchar_t* command_label ( command action ) {

		return commands [ static_cast < int32_t > ( action ) ].label;

	}

	const wchar_t* command_keys ( command action ) {

		return commands [ static_cast < int32_t > ( action ) ].keys;

	}

	uint64_t make_id ( const char* name, int64_t index ) {

		uint64_t hash = 0xCBF29CE484222325ull;

		for ( ; *name; name++ ) {

			hash ^= static_cast < uint8_t > ( *name );
			hash *= 0x100000001B3ull;

		}

		hash ^= static_cast < uint64_t > ( index ) * 0x9E3779B97F4A7C15ull;
		hash *= 0x100000001B3ull;

		return hash ? hash : 1;

	}

	static const uint32_t dark_tones [ static_cast < int32_t > ( tone::COUNT ) ] = {
		0x17191D, 0x1B1E23, 0x24282F, 0x121417, 0x131518, 0x292E36, 0x3A404A, 0xD8DBE1, 0x9BA2AE, 0x5F6672,
		0xD4A24C, 0xD4A24C, 0x1A1408, 0x22344C, 0x4F82C4, 0xFFFFFF, 0xFFFFFF, 0x000000, 0x7C8594, 0x646C79,
		0xE2B65B, 0x7FB0E8, 0xA8C98A, 0xE09D6B, 0x8CD1C8, 0x737C6E, 0x4F7DBF, 0x8C7A52, 0x6E6A90, 0x3A3F47,
		0x5A6170, 0xE5675B, 0x7DBE7A, 0xE3B341
	};

	static const uint32_t light_tones [ static_cast < int32_t > ( tone::COUNT ) ] = {
		0xFAF9F6, 0xF1EFEA, 0xFFFFFF, 0xE9E6E0, 0xE6E3DD, 0xDCD8D0, 0xC5C0B6, 0x1E2126, 0x5B616B, 0x99A0A8,
		0xA86F1C, 0xA86F1C, 0xFFFFFF, 0xDCE6F4, 0x4A78B8, 0x000000, 0x000000, 0x000000, 0x6D7580, 0x8D939B,
		0x96620E, 0x285CA6, 0x3E7A2D, 0xB0561E, 0x1D776D, 0x7B816F, 0x6F95CC, 0xB8A57A, 0x9C97B5, 0xD6D2CA,
		0x9AA0A8, 0xC8473C, 0x3F8F3C, 0xB7860F
	};

	static const float tone_alpha [ static_cast < int32_t > ( tone::COUNT ) ] [ 2 ] = {
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 0.16f, 0.14f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.045f, 0.045f }, { 0.085f, 0.08f }, { 0.5f, 0.16f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }
	};

	render::color app::ink ( tone value ) const {

		return tones [ static_cast < int32_t > ( value ) ];

	}

	void app::update_theme ( ) {

		spring ( theme_mix, theme_velocity, light ? 1.0f : 0.0f, 210.0f, 26.0f );

		float t = math::saturate ( theme_mix );

		for ( int32_t i = 0; i < static_cast < int32_t > ( tone::COUNT ); i++ ) {

			render::color dark = render::rgb ( dark_tones [ i ], tone_alpha [ i ] [ 0 ] );
			render::color bright = render::rgb ( light_tones [ i ], tone_alpha [ i ] [ 1 ] );

			tones [ i ] = render::mix ( dark, bright, t );

		}

	}

	void app::set_theme ( bool value ) {

		light = value;
		pending = base::max ( pending, 3 );

	}

	bool app::setup ( render::gpu* device, void* window_handle ) {

		gpu = device;
		window = window_handle;
		widgets = new widget [ widget_room ];
		theme_mix = light ? 1.0f : 0.0f;
		update_theme ( );
		log ( log_kind::INFO, L"Plumb is ready. Open a binary with Ctrl+O or drop one on the window." );

		return true;

	}

	void app::rebuild_gpu ( render::gpu* device ) {

		gpu = device;
		pending = base::max ( pending, 3 );

	}

	void app::shutdown ( ) {

		if ( persist )
			save_settings ( );

		files::settle ( );
		file.close ( );

	}

	void app::resize ( int32_t width_px, int32_t height_px, float dpi_scale ) {

		width = base::max ( width_px, 1 );
		height = base::max ( height_px, 1 );
		dpi = dpi_scale;
		pending = base::max ( pending, 2 );

	}

	void app::set_maximized ( bool value ) {

		maximized = value;
		pending = base::max ( pending, 2 );

	}

	void app::set_focus ( bool value ) {

		focused = value;
		pending = base::max ( pending, 2 );

	}

	bool app::busy ( ) const {

		return animating || pending > 0;

	}

	double app::wake_after ( ) const {

		return focused_field ? 0.5 : -1.0;

	}

	request app::take_request ( ) {

		request value = pending_request;

		pending_request = request::NONE;

		return value;

	}

	pointer app::cursor ( ) const {

		return wanted_cursor;

	}

	void app::mouse_move ( float x, float y ) {

		mouse = math::vec2 ( x / dpi, y / dpi );
		mouse_inside = true;
		pending = base::max ( pending, 2 );

	}

	void app::mouse_button ( int32_t button, bool down ) {

		if ( button == 1 ) {

			if ( down )
				right_event = true;

			pending = base::max ( pending, 2 );

			return;

		}

		if ( button == 3 || button == 4 ) {

			if ( down )
				run ( button == 3 ? command::BACK : command::FORWARD );

			return;

		}

		if ( down ) {

			press_event = true;
			mouse_down = true;
			press_at = mouse;

			if ( clock - last_press < 0.4 )
				double_event = true;

			last_press = clock;

		} else {

			release_event = true;
			mouse_down = false;

		}

		pending = base::max ( pending, 2 );

	}

	void app::mouse_leave ( ) {

		mouse_inside = false;

		if ( !mouse_down )
			mouse = math::vec2 ( -1000.0f, -1000.0f );

		pending = base::max ( pending, 2 );

	}

	void app::mouse_wheel ( float steps, bool horizontal ) {

		if ( horizontal )
			wheel_side += steps;
		else
			wheel_steps += steps;

		pending = base::max ( pending, 2 );

	}

	void app::key ( uint32_t code, bool control, bool shift, bool alt ) {

		keys.push ( code );
		key_control = control;
		key_shift = shift;
		key_alt = alt;
		caret_time = clock;
		pending = base::max ( pending, 2 );

	}

	void app::character ( wchar_t value ) {

		typed.push ( value );
		caret_time = clock;
		pending = base::max ( pending, 2 );

	}

	hit app::hit_test ( float x, float y ) const {

		math::vec2 at ( x / dpi, y / dpi );

		for ( int32_t i = hits.count ( ) - 1; i >= 0; i-- ) {

			const hit_rect& item = hits [ static_cast < size_t > ( i ) ];

			if ( item.area.contains ( at ) )
				return item.kind;

		}

		return title_area.contains ( at ) ? hit::CAPTION : hit::CLIENT;

	}

	widget& app::state ( uint64_t id ) {

		uint32_t slot = static_cast < uint32_t > ( id % static_cast < uint64_t > ( widget_room ) );

		for ( int32_t probe = 0; probe < widget_room; probe++ ) {

			widget& item = widgets [ slot ];

			if ( item.id == id ) {

				item.seen = frame_index;

				return item;

			}

			if ( item.id == 0 || item.seen + 600 < frame_index ) {

				item = widget ( );
				item.id = id;
				item.seen = frame_index;

				return item;

			}

			slot = ( slot + 1 ) % static_cast < uint32_t > ( widget_room );

		}

		return widgets [ id % static_cast < uint64_t > ( widget_room ) ];

	}

	bool app::over ( math::rect area ) const {

		return mouse_inside && area.contains ( mouse );

	}

	bool app::interact ( uint64_t id, math::rect area, widget& item, hit kind ) {

		hit_rect entry = { id, area, kind };

		next_hits.push ( entry );
		approach ( item.hover, hot == id ? 1.0f : 0.0f, 22.0f );
		approach ( item.press, active_id == id && mouse_down && hot == id ? 1.0f : 0.0f, 30.0f );

		return clicked_id == id;

	}

	void app::scroll_region ( uint64_t id, math::rect area ) {

		hit_rect entry = { id, area, hit::CLIENT };

		next_scrolls.push ( entry );

	}

	float app::take_wheel ( uint64_t id ) {

		if ( scroll_hot != id || wheel_steps == 0.0f )
			return 0.0f;

		float value = wheel_steps;

		wheel_steps = 0.0f;

		return value;

	}

	void app::tip ( uint64_t id, const wchar_t* text, math::rect anchor ) {

		if ( hot != id || !text || mouse_down )
			return;

		tip_candidate = id;
		tip_candidate_text = text;
		tip_candidate_anchor = anchor;

	}

	void app::approach ( float& value, float target, float rate ) {

		float step = 1.0f - math::exp ( -rate * dt );

		value += ( target - value ) * step;

		if ( math::abs ( target - value ) < 0.002f )
			value = target;
		else
			animating = true;

	}

	void app::spring ( float& value, float& velocity, float target, float stiffness, float damping ) {

		float remaining = dt;

		while ( remaining > 0.0f ) {

			float step = base::min ( remaining, 1.0f / 240.0f );

			velocity += ( stiffness * ( target - value ) - damping * velocity ) * step;
			value += velocity * step;
			remaining -= step;

		}

		if ( math::abs ( target - value ) < 0.0008f && math::abs ( velocity ) < 0.01f ) {

			value = target;
			velocity = 0.0f;

		} else
			animating = true;

	}

	void app::glide ( scroller& item, float limit ) {

		item.limit = base::max ( limit, 0.0f );
		item.target = base::clamp ( item.target, 0.0f, item.limit );

		float gap = item.target - item.offset;

		if ( math::abs ( gap ) < 0.01f && math::abs ( item.velocity ) < 0.05f ) {

			item.offset = item.target;
			item.velocity = 0.0f;

			return;

		}

		spring ( item.offset, item.velocity, item.target, 420.0f, 42.0f );
		item.offset = base::clamp ( item.offset, -40.0f, item.limit + 40.0f );

	}

	bool app::begin_frame ( double seconds ) {

		if ( last < 0.0 )
			last = seconds;

		dt = fixed_step ? 1.0f / 60.0f : static_cast < float > ( base::clamp ( seconds - last, 0.0, 0.05 ) );
		last = seconds;
		clock = seconds;
		animating = false;
		wanted_cursor = pointer::ARROW;
		w = static_cast < float > ( width ) / dpi;
		h = static_cast < float > ( height ) / dpi;
		frame_index++;

		return true;

	}

	void app::begin_input ( ) {

		hot = 0;
		scroll_hot = 0;

		if ( mouse_inside || mouse_down ) {

			for ( int32_t i = hits.count ( ) - 1; i >= 0; i-- ) {

				if ( hits [ static_cast < size_t > ( i ) ].area.contains ( mouse ) ) {

					hot = hits [ static_cast < size_t > ( i ) ].id;

					break;

				}

			}

			for ( int32_t i = scrolls.count ( ) - 1; i >= 0; i-- ) {

				if ( scrolls [ static_cast < size_t > ( i ) ].area.contains ( mouse ) ) {

					scroll_hot = scrolls [ static_cast < size_t > ( i ) ].id;

					break;

				}

			}

		}

		clicked_id = 0;
		double_id = 0;
		right_id = 0;

		if ( press_event ) {

			active_id = hot;

			if ( double_event )
				double_id = hot;

		}

		if ( right_event )
			right_id = hot;

		if ( release_event && active_id && active_id == hot )
			clicked_id = active_id;

	}

	void app::end_input ( ) {

		if ( release_event ) {

			active_id = 0;
			dragging_split = 0;
			hex_selecting = false;

		}

		press_event = false;
		release_event = false;
		right_event = false;
		double_event = false;
		wheel_steps = 0.0f;
		wheel_side = 0.0f;
		keys.clear ( );
		typed.clear ( );
		hits = base::move ( next_hits );
		next_hits.clear ( );
		scrolls = base::move ( next_scrolls );
		next_scrolls.clear ( );

		if ( tip_candidate && tip_candidate == tip_id )
			tip_time += dt;
		else {

			tip_time = 0.0f;
			tip_id = tip_candidate;

		}

		if ( tip_candidate ) {

			tip_text = tip_candidate_text;
			tip_anchor = tip_candidate_anchor;

			if ( tip_time < 0.6f )
				animating = true;

		}

		tip_candidate = 0;

	}

	void app::log ( log_kind kind, const wchar_t* text ) {

		log_line& line = output.emplace ( );

		line.kind = kind;
		line.text.set ( text );
		line.time = base::seconds ( );

		if ( output.count ( ) > 2000 )
			output.remove_at ( 0 );

		output_scroll.target = 1e9f;
		pending = base::max ( pending, 2 );

	}

	void app::copy_text ( const wchar_t* text ) {

		if ( !text || !OpenClipboard ( static_cast < HWND > ( window ) ) )
			return;

		EmptyClipboard ( );

		size_t length = static_cast < size_t > ( base::text_length ( text ) ) + 1;
		HGLOBAL block = GlobalAlloc ( GMEM_MOVEABLE, length * sizeof ( wchar_t ) );

		if ( block ) {

			wchar_t* target = static_cast < wchar_t* > ( GlobalLock ( block ) );

			for ( size_t i = 0; i < length; i++ )
				target [ i ] = text [ i ];

			GlobalUnlock ( block );

			if ( !SetClipboardData ( CF_UNICODETEXT, block ) )
				GlobalFree ( block );

		}

		CloseClipboard ( );

	}

	static bool read_clipboard ( void* window, wchar_t* out, int32_t room ) {

		out [ 0 ] = 0;

		if ( !IsClipboardFormatAvailable ( CF_UNICODETEXT ) || !OpenClipboard ( static_cast < HWND > ( window ) ) )
			return false;

		HANDLE block = GetClipboardData ( CF_UNICODETEXT );
		bool ok = false;

		if ( block ) {

			const wchar_t* source = static_cast < const wchar_t* > ( GlobalLock ( block ) );

			if ( source ) {

				int32_t i = 0;

				for ( ; source [ i ] && i < room - 1; i++ )
					out [ i ] = source [ i ] == L'\r' || source [ i ] == L'\n' || source [ i ] == L'\t' ? L' ' : source [ i ];

				out [ i ] = 0;
				ok = true;
				GlobalUnlock ( block );

			}

		}

		CloseClipboard ( );

		return ok;

	}

	static bool word_char ( wchar_t value ) {

		return ( value >= L'0' && value <= L'9' ) || ( value >= L'a' && value <= L'z' ) || ( value >= L'A' && value <= L'Z' ) || value == L'_';

	}

	static void field_erase ( field& value, int32_t from, int32_t to ) {

		if ( from > to ) {

			int32_t held = from;

			from = to;
			to = held;

		}

		from = base::clamp ( from, 0, value.length );
		to = base::clamp ( to, 0, value.length );

		for ( int32_t i = to; i <= value.length; i++ )
			value.text [ from + i - to ] = value.text [ i ];

		value.length -= to - from;
		value.caret = from;
		value.anchor = from;
		value.revision++;

	}

	static void field_insert ( field& value, const wchar_t* text ) {

		if ( value.caret != value.anchor )
			field_erase ( value, value.caret, value.anchor );

		int32_t count = base::text_length ( text );

		count = base::min ( count, 254 - value.length );

		if ( count <= 0 )
			return;

		for ( int32_t i = value.length; i >= value.caret; i-- )
			value.text [ i + count ] = value.text [ i ];

		for ( int32_t i = 0; i < count; i++ )
			value.text [ value.caret + i ] = text [ i ];

		value.length += count;
		value.caret += count;
		value.anchor = value.caret;
		value.revision++;

	}

	void app::focus_field ( uint64_t id, field& value, bool select ) {

		focus_id = id;
		focused_field = &value;
		caret_time = clock;
		value.caret = value.length;
		value.anchor = select ? 0 : value.length;
		pending = base::max ( pending, 2 );

	}

	bool app::edit_keys ( field& value, uint32_t code ) {

		int32_t low = base::min ( value.caret, value.anchor );
		int32_t high = base::max ( value.caret, value.anchor );

		auto move_to = [ & ] ( int32_t target ) {

			value.caret = base::clamp ( target, 0, value.length );

			if ( !key_shift )
				value.anchor = value.caret;

		};

		auto word_left = [ & ] ( int32_t at ) {

			while ( at > 0 && !word_char ( value.text [ at - 1 ] ) )
				at--;

			while ( at > 0 && word_char ( value.text [ at - 1 ] ) )
				at--;

			return at;

		};

		auto word_right = [ & ] ( int32_t at ) {

			while ( at < value.length && !word_char ( value.text [ at ] ) )
				at++;

			while ( at < value.length && word_char ( value.text [ at ] ) )
				at++;

			return at;

		};

		switch ( code ) {

			case VK_LEFT:
				move_to ( !key_shift && low != high ? low : ( key_control ? word_left ( value.caret ) : value.caret - 1 ) );
				return true;

			case VK_RIGHT:
				move_to ( !key_shift && low != high ? high : ( key_control ? word_right ( value.caret ) : value.caret + 1 ) );
				return true;

			case VK_HOME:
				move_to ( 0 );
				return true;

			case VK_END:
				move_to ( value.length );
				return true;

			case VK_BACK:

				if ( low != high )
					field_erase ( value, low, high );
				else if ( value.caret > 0 )
					field_erase ( value, key_control ? word_left ( value.caret ) : value.caret - 1, value.caret );

				return true;

			case VK_DELETE:

				if ( low != high )
					field_erase ( value, low, high );
				else if ( value.caret < value.length )
					field_erase ( value, value.caret, key_control ? word_right ( value.caret ) : value.caret + 1 );

				return true;

			case 'A':

				if ( !key_control )
					return false;

				value.anchor = 0;
				value.caret = value.length;

				return true;

			case 'C':
			case 'X':

				if ( !key_control )
					return false;

				if ( low != high ) {

					wchar_t piece [ 256 ];
					int32_t count = high - low;

					for ( int32_t i = 0; i < count; i++ )
						piece [ i ] = value.text [ low + i ];

					piece [ count ] = 0;
					copy_text ( piece );

					if ( code == 'X' )
						field_erase ( value, low, high );

				}

				return true;

			case 'V': {

				if ( !key_control )
					return false;

				wchar_t clip [ 256 ];

				if ( read_clipboard ( window, clip, 256 ) )
					field_insert ( value, clip );

				return true;

			}

			case VK_RETURN:
				field_submit = true;
				return true;

			case VK_ESCAPE:

				if ( palette_open )
					return false;

				focus_id = 0;
				focused_field = nullptr;

				return true;

			default:
				return false;

		}

	}

	bool app::enabled ( command action ) const {

		bool has = file.loaded ( );

		switch ( action ) {

			case command::OPEN:
			case command::EXIT:
			case command::PALETTE:
			case command::THEME:
			case command::SHORTCUTS:
			case command::ABOUT:
				return true;

			case command::BACK:
				return has && !back_stack.empty ( );

			case command::FORWARD:
				return has && !forward_stack.empty ( );

			case command::GO_ENTRY:
				return has && file.entry != 0;

			case command::ANALYZE:
			case command::DECODE_HERE:
				return false;

			default:
				return has;

		}

	}

	bool app::checked ( command action ) const {

		switch ( action ) {

			case command::VIEW_LISTING:
				return showing == view::LISTING;

			case command::VIEW_HEX:
				return showing == view::HEX;

			case command::VIEW_HEADERS:
				return showing == view::HEADERS;

			case command::VIEW_GRAPH:
				return showing == view::GRAPH;

			case command::VIEW_PSEUDOCODE:
				return showing == view::PSEUDOCODE;

			case command::TOGGLE_BROWSER:
				return browser_open;

			case command::TOGGLE_INSPECTOR:
				return inspector_open;

			case command::TOGGLE_OUTPUT:
				return output_open;

			case command::THEME:
				return light;

			default:
				return false;

		}

	}

	void app::run ( command action ) {

		if ( !enabled ( action ) ) {

			if ( action == command::ANALYZE || action == command::DECODE_HERE )
				log ( log_kind::WARN, L"The instruction decoder isn't wired in yet, so there's nothing to analyze." );

			return;

		}

		menu_open = -1;
		pending = base::max ( pending, 3 );

		switch ( action ) {

			case command::OPEN:
				pending_request = request::OPEN;
				break;

			case command::CLOSE:
				close_file ( );
				break;

			case command::EXIT:
				pending_request = request::CLOSE;
				break;

			case command::COPY: {

				base::writer line;

				if ( showing == view::HEX ) {

					uint64_t low = base::min ( hex_cursor, hex_anchor );
					uint64_t high = base::min ( base::max ( hex_cursor, hex_anchor ), file.size ? file.size - 1 : 0 );

					for ( uint64_t i = low; i <= high && i - low < 600; i++ ) {

						if ( i > low )
							line.add ( L' ' );

						line.hex ( file.byte ( i ), 2 );

					}

				} else
					line.add ( L"0x" ).hex ( cursor_address, file.wide ( ) ? 16 : 8 );

				copy_text ( line.text ( ) );
				log ( log_kind::INFO, base::writer ( ).add ( L"Copied " ).add ( line.text ( ) ).text ( ) );

				break;

			}

			case command::GO_ADDRESS:
				palette_open = true;
				palette_address = true;
				palette_field = field ( );
				palette_seen = 0xFFFFFFFFu;
				focus_field ( make_id ( "palette" ), palette_field, true );
				break;

			case command::PALETTE:
				palette_open = true;
				palette_address = false;
				palette_field = field ( );
				palette_seen = 0xFFFFFFFFu;
				focus_field ( make_id ( "palette" ), palette_field, true );
				break;

			case command::GO_ENTRY:
				go ( file.entry, true );
				break;

			case command::BACK:

				if ( !back_stack.empty ( ) ) {

					forward_stack.push ( cursor_address );
					go ( back_stack.back ( ), false );
					back_stack.pop ( );

				}

				break;

			case command::FORWARD:

				if ( !forward_stack.empty ( ) ) {

					back_stack.push ( cursor_address );
					go ( forward_stack.back ( ), false );
					forward_stack.pop ( );

				}

				break;

			case command::NEXT_SECTION:
			case command::PREVIOUS_SECTION: {

				int32_t current = file.section_at ( cursor_address );
				int32_t target = current + ( action == command::NEXT_SECTION ? 1 : -1 );

				if ( target >= 0 && target < file.sections.count ( ) )
					go ( file.sections [ static_cast < size_t > ( target ) ].address, true );

				break;

			}

			case command::FIND:
				browsing = panel::STRINGS;
				browser_open = true;
				focus_field ( make_id ( "filter", static_cast < int64_t > ( panel::STRINGS ) ), filters [ static_cast < int32_t > ( panel::STRINGS ) ], true );
				break;

			case command::VIEW_LISTING:
			case command::VIEW_HEX:
			case command::VIEW_HEADERS:
			case command::VIEW_GRAPH:
			case command::VIEW_PSEUDOCODE: {

				view next = static_cast < view > ( static_cast < int32_t > ( action ) - static_cast < int32_t > ( command::VIEW_LISTING ) );

				if ( next == view::HEX && showing != view::HEX ) {

					uint64_t offset = 0;

					if ( file.offset_of ( cursor_address, offset ) ) {

						hex_cursor = offset;
						hex_anchor = offset;
						hex_scroll.target = static_cast < float > ( offset / 16 ) - 8.0f;
						hex_scroll.offset = hex_scroll.target;

					}

				}

				if ( next != showing )
					view_fade = 0.0f;

				showing = next;

				break;

			}

			case command::SHOW_FUNCTIONS:
			case command::SHOW_IMPORTS:
			case command::SHOW_EXPORTS:
			case command::SHOW_STRINGS:
			case command::SHOW_SECTIONS:
				browsing = static_cast < panel > ( static_cast < int32_t > ( action ) - static_cast < int32_t > ( command::SHOW_FUNCTIONS ) );
				browser_open = true;
				break;

			case command::TOGGLE_BROWSER:
				browser_open = !browser_open;
				break;

			case command::TOGGLE_INSPECTOR:
				inspector_open = !inspector_open;
				break;

			case command::TOGGLE_OUTPUT:
				output_open = !output_open;
				break;

			case command::THEME:
				set_theme ( !light );
				break;

			case command::SHORTCUTS:
				keys_open = true;
				break;

			case command::ABOUT:
				about_open = true;
				break;

			default:
				break;

		}

	}

	void app::handle_keys ( ) {

		field_submit = false;

		if ( focused_field ) {

			for ( wchar_t value : typed ) {

				if ( value < 32 || value == 127 )
					continue;

				wchar_t single [ 2 ] = { value, 0 };

				field_insert ( *focused_field, single );

			}

		}

		typed.clear ( );

		for ( uint32_t code : keys ) {

			if ( focused_field && edit_keys ( *focused_field, code ) )
				continue;

			if ( code == VK_ESCAPE ) {

				if ( menu_open >= 0 )
					menu_open = -1;
				else if ( palette_open ) {

					palette_open = false;
					focus_id = 0;
					focused_field = nullptr;

				} else if ( about_open || keys_open ) {

					about_open = false;
					keys_open = false;

				} else if ( focused_field ) {

					focus_id = 0;
					focused_field = nullptr;

				}

				continue;

			}

			if ( palette_open ) {

				if ( code == VK_DOWN )
					palette_pick++;
				else if ( code == VK_UP )
					palette_pick--;

				continue;

			}

			if ( key_control && key_shift ) {

				if ( code == 'P' )
					run ( command::PALETTE );
				else if ( code == 'L' )
					run ( command::THEME );

				continue;

			}

			if ( key_alt ) {

				if ( code == VK_LEFT )
					run ( command::BACK );
				else if ( code == VK_RIGHT )
					run ( command::FORWARD );
				else if ( code >= '1' && code <= '5' )
					run ( static_cast < command > ( static_cast < int32_t > ( command::SHOW_FUNCTIONS ) + static_cast < int32_t > ( code - '1' ) ) );

				continue;

			}

			if ( key_control ) {

				switch ( code ) {

					case 'O':
						run ( command::OPEN );
						break;

					case 'W':
						run ( command::CLOSE );
						break;

					case 'C':
						run ( command::COPY );
						break;

					case 'E':
						run ( command::GO_ENTRY );
						break;

					case 'F':
						run ( command::FIND );
						break;

					case 'G':
						run ( command::GO_ADDRESS );
						break;

					case 'P':
						run ( command::PALETTE );
						break;

					case 'B':
						run ( command::TOGGLE_BROWSER );
						break;

					case 'I':
						run ( command::TOGGLE_INSPECTOR );
						break;

					case 'J':
						run ( command::TOGGLE_OUTPUT );
						break;

					case VK_NEXT:
						run ( command::NEXT_SECTION );
						break;

					case VK_PRIOR:
						run ( command::PREVIOUS_SECTION );
						break;

					case VK_HOME:

						if ( file.loaded ( ) )
							set_cursor_row ( 0, false );

						break;

					case VK_END:

						if ( total_rows )
							set_cursor_row ( total_rows - 1, false );

						break;

					default:

						if ( code >= '1' && code <= '5' )
							run ( static_cast < command > ( static_cast < int32_t > ( command::VIEW_LISTING ) + static_cast < int32_t > ( code - '1' ) ) );

						break;

				}

				continue;

			}

			if ( focused_field )
				continue;

			switch ( code ) {

				case 'G':
					run ( command::GO_ADDRESS );
					break;

				case 'C':
					run ( command::DECODE_HERE );
					break;

				case VK_F1:
					run ( command::SHORTCUTS );
					break;

				case VK_F5:
					run ( command::VIEW_PSEUDOCODE );
					break;

				case VK_BACK:
					run ( command::BACK );
					break;

				default:
					break;

			}

			if ( !file.loaded ( ) )
				continue;

			if ( showing == view::LISTING ) {

				int64_t page = static_cast < int64_t > ( view_area.h / 20.0f ) - 2;
				int64_t move = 0;

				if ( code == VK_DOWN )
					move = 1;
				else if ( code == VK_UP )
					move = -1;
				else if ( code == VK_NEXT )
					move = page;
				else if ( code == VK_PRIOR )
					move = -page;

				if ( move ) {

					int64_t target = static_cast < int64_t > ( cursor_row ) + move;

					set_cursor_row ( static_cast < uint64_t > ( base::clamp < int64_t > ( target, 0, static_cast < int64_t > ( total_rows ) - 1 ) ), false );

				}

			} else if ( showing == view::HEX && file.size ) {

				int64_t page = ( static_cast < int64_t > ( view_area.h / 20.0f ) - 3 ) * 16;
				int64_t move = 0;

				if ( code == VK_RIGHT )
					move = 1;
				else if ( code == VK_LEFT )
					move = -1;
				else if ( code == VK_DOWN )
					move = 16;
				else if ( code == VK_UP )
					move = -16;
				else if ( code == VK_NEXT )
					move = page;
				else if ( code == VK_PRIOR )
					move = -page;

				if ( move ) {

					int64_t target = base::clamp < int64_t > ( static_cast < int64_t > ( hex_cursor ) + move, 0, static_cast < int64_t > ( file.size ) - 1 );

					hex_cursor = static_cast < uint64_t > ( target );

					if ( !key_shift )
						hex_anchor = hex_cursor;

					float row = static_cast < float > ( hex_cursor / 16 );
					float visible = base::max ( view_area.h / 20.0f - 3.0f, 1.0f );

					if ( row < hex_scroll.target )
						hex_scroll.target = row;
					else if ( row > hex_scroll.target + visible - 1.0f )
						hex_scroll.target = row - visible + 1.0f;

					uint64_t address = 0;

					if ( file.address_of ( hex_cursor, address ) )
						cursor_address = address;

				}

			}

		}

		keys.clear ( );

	}

	static int32_t lower_name ( wchar_t value ) {

		return value >= L'A' && value <= L'Z' ? value + 32 : value;

	}

	static bool contains_text ( const wchar_t* haystack, const wchar_t* needle ) {

		if ( !needle [ 0 ] )
			return true;

		for ( int32_t i = 0; haystack [ i ]; i++ ) {

			int32_t k = 0;

			while ( needle [ k ] && haystack [ i + k ] && lower_name ( haystack [ i + k ] ) == lower_name ( needle [ k ] ) )
				k++;

			if ( !needle [ k ] )
				return true;

		}

		return false;

	}

	static void string_preview ( const image::binary& source, const image::found_string& item, wchar_t* out, int32_t room ) {

		int32_t length = base::min ( static_cast < int32_t > ( item.length ), room - 1 );

		for ( int32_t i = 0; i < length; i++ )
			out [ i ] = static_cast < wchar_t > ( source.byte ( item.offset + static_cast < uint64_t > ( i ) * ( item.wide ? 2 : 1 ) ) );

		out [ length ] = 0;

	}

	void app::refresh_filter ( panel which ) {

		int32_t slot = static_cast < int32_t > ( which );
		base::list < int32_t >& list = shown [ slot ];
		const wchar_t* needle = filters [ slot ].text;
		wchar_t buffer [ 160 ];

		list.clear ( );
		filter_seen [ slot ] = filters [ slot ].revision;
		browser_pick [ slot ] = -1;

		if ( !file.loaded ( ) )
			return;

		switch ( which ) {

			case panel::FUNCTIONS:

				for ( int32_t i = 0; i < file.functions.count ( ); i++ ) {

					if ( contains_text ( file.functions [ static_cast < size_t > ( i ) ].name.get ( ), needle ) )
						list.push ( i );

				}

				break;

			case panel::IMPORTS:

				for ( int32_t m = 0; m < file.modules.count ( ); m++ ) {

					const image::module& owner = file.modules [ static_cast < size_t > ( m ) ];
					bool header = false;
					bool module_match = contains_text ( owner.name.get ( ), needle );

					for ( int32_t i = owner.first; i < owner.first + owner.count; i++ ) {

						const image::import& item = file.imports [ static_cast < size_t > ( i ) ];

						if ( !module_match && !contains_text ( item.name.get ( ), needle ) )
							continue;

						if ( !header ) {

							list.push ( -( m + 1 ) );
							header = true;

						}

						list.push ( i );

					}

					if ( !header && module_match )
						list.push ( -( m + 1 ) );

				}

				break;

			case panel::EXPORTS:

				for ( int32_t i = 0; i < file.exports.count ( ); i++ ) {

					if ( contains_text ( file.exports [ static_cast < size_t > ( i ) ].name.get ( ), needle ) )
						list.push ( i );

				}

				break;

			case panel::STRINGS:

				for ( int32_t i = 0; i < file.strings.count ( ); i++ ) {

					if ( needle [ 0 ] ) {

						string_preview ( file, file.strings [ static_cast < size_t > ( i ) ], buffer, 160 );

						if ( !contains_text ( buffer, needle ) )
							continue;

					}

					list.push ( i );

				}

				break;

			case panel::SECTIONS:

				for ( int32_t i = 0; i < file.sections.count ( ); i++ ) {

					if ( contains_text ( file.sections [ static_cast < size_t > ( i ) ].name, needle ) )
						list.push ( i );

				}

				break;

			default:
				break;

		}

	}

	struct mark {

		uint64_t address;
		uint64_t size;
		int32_t kind;
		int32_t ref;

	};

	static void sort_marks ( base::list < mark >& items ) {

		size_t count = items.size ( );

		if ( count < 2 )
			return;

		base::list < mark > scratch;

		scratch.resize ( count );

		mark* from = items.data ( );
		mark* to = scratch.data ( );

		for ( size_t width_run = 1; width_run < count; width_run *= 2 ) {

			for ( size_t low = 0; low < count; low += width_run * 2 ) {

				size_t middle = base::min ( low + width_run, count );
				size_t high = base::min ( low + width_run * 2, count );
				size_t a = low;
				size_t b = middle;
				size_t out = low;

				while ( a < middle && b < high ) {

					bool take_a = from [ a ].address < from [ b ].address || ( from [ a ].address == from [ b ].address && from [ a ].kind <= from [ b ].kind );

					to [ out++ ] = take_a ? from [ a++ ] : from [ b++ ];

				}

				while ( a < middle )
					to [ out++ ] = from [ a++ ];

				while ( b < high )
					to [ out++ ] = from [ b++ ];

			}

			mark* held = from;

			from = to;
			to = held;

		}

		if ( from != items.data ( ) ) {

			for ( size_t i = 0; i < count; i++ )
				items [ i ] = from [ i ];

		}

	}

	void app::build_listing ( ) {

		items.clear ( );
		names.clear ( );
		total_rows = 0;

		if ( !file.loaded ( ) )
			return;

		base::list < mark > marks;

		for ( const image::symbol& item : file.functions ) {

			mark entry = { item.address, 0, 0, names.count ( ) };

			names.push ( item.name );
			marks.push ( entry );

		}

		for ( const image::exported& item : file.exports ) {

			if ( !item.address || !item.forward.empty ( ) )
				continue;

			int32_t owner = file.section_at ( item.address );

			if ( owner >= 0 && file.sections [ static_cast < size_t > ( owner ) ].type == image::kind::CODE )
				continue;

			base::writer label;

			if ( item.name.empty ( ) )
				label.add ( L"Ordinal_" ).number ( item.ordinal );
			else
				label.add ( item.name.get ( ) );

			mark entry = { item.address, 0, 1, names.count ( ) };

			names.push ( base::text ( label.text ( ) ) );
			marks.push ( entry );

		}

		uint64_t pointer = file.wide ( ) ? 8 : 4;

		for ( int32_t i = 0; i < file.imports.count ( ); i++ ) {

			mark entry = { file.imports [ static_cast < size_t > ( i ) ].slot, pointer, 3, i };

			marks.push ( entry );

		}

		for ( int32_t i = 0; i < file.strings.count ( ); i++ ) {

			const image::found_string& item = file.strings [ static_cast < size_t > ( i ) ];
			uint64_t address = 0;

			if ( !file.address_of ( item.offset, address ) )
				continue;

			int32_t owner = file.section_at ( address );

			if ( owner < 0 || file.sections [ static_cast < size_t > ( owner ) ].type == image::kind::CODE )
				continue;

			uint64_t bytes = static_cast < uint64_t > ( item.length ) * ( item.wide ? 2 : 1 );
			uint64_t end = item.offset + bytes;

			if ( end < file.size && file.byte ( end ) == 0 && ( !item.wide || file.byte ( end + 1 ) == 0 ) )
				bytes += item.wide ? 2 : 1;

			mark entry = { address, bytes, 2, i };

			marks.push ( entry );

		}

		sort_marks ( marks );

		base::list < int32_t > order;

		for ( int32_t i = 0; i < file.sections.count ( ); i++ ) {

			int32_t at = order.count ( );

			order.push ( i );

			while ( at > 0 && file.sections [ static_cast < size_t > ( order [ at - 1 ] ) ].address > file.sections [ static_cast < size_t > ( i ) ].address ) {

				order [ at ] = order [ at - 1 ];
				at--;

			}

			order [ at ] = i;

		}

		uint64_t rows = 0;
		size_t next_mark = 0;

		auto push = [ & ] ( line_kind kind, int32_t section, uint64_t address, uint64_t size, uint32_t stride, int32_t ref, bool function ) {

			line_item& item = items.emplace ( );

			item.kind = kind;
			item.section = section;
			item.address = address;
			item.size = size;
			item.stride = stride;
			item.ref = ref;
			item.function = function;
			item.first_row = rows;
			item.rows = ( kind == line_kind::CODE || kind == line_kind::DATA ) ? static_cast < uint32_t > ( ( size + stride - 1 ) / stride ) : ( kind == line_kind::BANNER ? 2u : 1u );
			rows += item.rows;

		};

		auto fill = [ & ] ( int32_t section, uint64_t from, uint64_t to ) {

			const image::section& owner = file.sections [ static_cast < size_t > ( section ) ];
			uint64_t raw_end = owner.address + owner.raw;
			bool code = owner.type == image::kind::CODE;

			while ( from < to ) {

				if ( from >= raw_end ) {

					push ( line_kind::EMPTY, section, from, to - from, 1, -1, false );

					return;

				}

				uint64_t end = base::min ( to, raw_end );
				uint64_t chunk = base::min ( end - from, 0x100000ull * 8 );

				push ( code ? line_kind::CODE : line_kind::DATA, section, from, chunk, 8, -1, false );
				from += chunk;

			}

		};

		for ( int32_t index : order ) {

			const image::section& owner = file.sections [ static_cast < size_t > ( index ) ];
			uint64_t start = owner.address;
			uint64_t end = owner.address + base::max ( owner.size, owner.raw );
			uint64_t at = start;

			push ( line_kind::BANNER, index, start, end - start, 1, -1, false );

			while ( next_mark < marks.size ( ) && marks [ next_mark ].address < start )
				next_mark++;

			while ( at < end ) {

				uint64_t target = end;

				while ( next_mark < marks.size ( ) && marks [ next_mark ].address < at )
					next_mark++;

				if ( next_mark < marks.size ( ) && marks [ next_mark ].address < end )
					target = marks [ next_mark ].address;

				if ( target > at )
					fill ( index, at, target );

				at = target;

				if ( at >= end )
					break;

				bool consumed = false;

				while ( next_mark < marks.size ( ) && marks [ next_mark ].address == at ) {

					const mark& item = marks [ next_mark++ ];

					if ( item.kind <= 1 ) {

						push ( line_kind::LABEL, index, at, 0, 1, item.ref, item.kind == 0 );

						continue;

					}

					if ( consumed )
						continue;

					uint64_t span = base::min ( item.size, end - at );

					if ( item.kind == 2 ) {

						push ( line_kind::LABEL, index, at, 0, 1, -( item.ref + 2 ), false );
						push ( line_kind::STRING, index, at, span, 1, item.ref, false );

					} else
						push ( line_kind::POINTER, index, at, span, 1, item.ref, false );

					at += span;
					consumed = true;

				}

			}

		}

		total_rows = rows;

	}

	int32_t app::item_at ( uint64_t row ) const {

		int32_t low = 0;
		int32_t high = items.count ( ) - 1;

		if ( high < 0 )
			return -1;

		while ( low < high ) {

			int32_t middle = ( low + high + 1 ) / 2;

			if ( items [ static_cast < size_t > ( middle ) ].first_row <= row )
				low = middle;
			else
				high = middle - 1;

		}

		return low;

	}

	uint64_t app::address_at_row ( uint64_t row ) const {

		int32_t index = item_at ( row );

		if ( index < 0 )
			return 0;

		const line_item& item = items [ static_cast < size_t > ( index ) ];

		if ( item.rows > 1 && item.kind != line_kind::BANNER )
			return item.address + ( row - item.first_row ) * item.stride;

		return item.address;

	}

	uint64_t app::row_of ( uint64_t address ) const {

		int32_t low = 0;
		int32_t high = items.count ( ) - 1;

		if ( high < 0 || items [ 0 ].address > address )
			return 0;

		while ( low < high ) {

			int32_t middle = ( low + high + 1 ) / 2;

			if ( items [ static_cast < size_t > ( middle ) ].address <= address )
				low = middle;
			else
				high = middle - 1;

		}

		if ( items [ static_cast < size_t > ( low ) ].address == address ) {

			while ( low > 0 && items [ static_cast < size_t > ( low - 1 ) ].address == address )
				low--;

			if ( items [ static_cast < size_t > ( low ) ].kind == line_kind::BANNER && low + 1 < items.count ( ) && items [ static_cast < size_t > ( low + 1 ) ].address == address )
				low++;

			return items [ static_cast < size_t > ( low ) ].first_row;

		}

		const line_item& item = items [ static_cast < size_t > ( low ) ];

		if ( item.kind == line_kind::CODE || item.kind == line_kind::DATA )
			return item.first_row + base::min ( ( address - item.address ) / item.stride, static_cast < uint64_t > ( item.rows - 1 ) );

		return item.first_row;

	}

	void app::set_cursor_row ( uint64_t row, bool center ) {

		if ( !total_rows )
			return;

		cursor_row = base::min ( row, total_rows - 1 );
		cursor_address = address_at_row ( cursor_row );

		float visible = view_area.h > 40.0f ? view_area.h / 20.0f - 1.0f : 30.0f;
		float place = static_cast < float > ( cursor_row );

		if ( center )
			listing_scroll.target = place - visible * 0.35f;
		else if ( place < listing_scroll.target )
			listing_scroll.target = place;
		else if ( place > listing_scroll.target + visible - 1.0f )
			listing_scroll.target = place - visible + 1.0f;

		uint64_t offset = 0;

		if ( file.offset_of ( cursor_address, offset ) ) {

			hex_cursor = offset;
			hex_anchor = offset;

		}

		pending = base::max ( pending, 2 );

	}

	void app::go ( uint64_t address, bool remember ) {

		if ( !file.loaded ( ) )
			return;

		if ( remember && cursor_address != address ) {

			back_stack.push ( cursor_address );
			forward_stack.clear ( );

			if ( back_stack.count ( ) > 256 )
				back_stack.remove_at ( 0 );

		}

		if ( showing == view::GRAPH || showing == view::PSEUDOCODE || showing == view::HEADERS )
			showing = view::LISTING;

		set_cursor_row ( row_of ( address ), true );
		cursor_address = address;

		if ( showing == view::HEX ) {

			uint64_t offset = 0;

			if ( file.offset_of ( address, offset ) )
				go_offset ( offset );

		}

	}

	void app::go_offset ( uint64_t offset ) {

		hex_cursor = offset;
		hex_anchor = offset;
		hex_scroll.target = static_cast < float > ( offset / 16 ) - 6.0f;
		pending = base::max ( pending, 2 );

	}

	bool app::parse_address ( const wchar_t* text, uint64_t& result ) const {

		while ( *text == L' ' )
			text++;

		if ( !*text )
			return false;

		for ( const image::symbol& item : file.functions ) {

			if ( base::same_text ( item.name.get ( ), text ) ) {

				result = item.address;

				return true;

			}

		}

		for ( const image::exported& item : file.exports ) {

			if ( item.address && base::same_text ( item.name.get ( ), text ) ) {

				result = item.address;

				return true;

			}

		}

		for ( const image::section& item : file.sections ) {

			if ( base::same_text ( item.name, text ) ) {

				result = item.address;

				return true;

			}

		}

		bool relative = false;

		if ( *text == L'+' ) {

			relative = true;
			text++;

		}

		if ( text [ 0 ] == L'0' && ( text [ 1 ] == L'x' || text [ 1 ] == L'X' ) )
			text += 2;

		uint64_t value = 0;
		int32_t digits = 0;

		for ( ; *text; text++ ) {

			wchar_t c = *text;
			uint32_t nibble = 0;

			if ( c >= L'0' && c <= L'9' )
				nibble = static_cast < uint32_t > ( c - L'0' );
			else if ( c >= L'a' && c <= L'f' )
				nibble = static_cast < uint32_t > ( c - L'a' + 10 );
			else if ( c >= L'A' && c <= L'F' )
				nibble = static_cast < uint32_t > ( c - L'A' + 10 );
			else if ( c == L'h' || c == L'H' || c == L'`' || c == L' ' )
				continue;
			else
				return false;

			value = ( value << 4 ) | nibble;
			digits++;

		}

		if ( !digits || digits > 16 )
			return false;

		result = relative ? file.base + value : value;

		if ( !relative && file.type != image::format::RAW && value < file.base && file.section_at ( file.base + value ) >= 0 )
			result = file.base + value;

		return true;

	}

	void app::label_name ( int32_t index, wchar_t* out, int32_t room ) const {

		const image::found_string& item = file.strings [ static_cast < size_t > ( index ) ];
		int32_t length = 0;
		bool upper = true;

		out [ length++ ] = L'a';

		for ( uint32_t i = 0; i < item.length && length < room - 1 && length < 28; i++ ) {

			wchar_t c = static_cast < wchar_t > ( file.byte ( item.offset + static_cast < uint64_t > ( i ) * ( item.wide ? 2 : 1 ) ) );

			if ( ( c >= L'a' && c <= L'z' ) || ( c >= L'A' && c <= L'Z' ) || ( c >= L'0' && c <= L'9' ) ) {

				out [ length++ ] = upper && c >= L'a' && c <= L'z' ? static_cast < wchar_t > ( c - 32 ) : c;
				upper = false;

			} else
				upper = true;

		}

		if ( length == 1 ) {

			base::writer fallback;

			fallback.add ( L"str_" ).hex ( item.offset );

			for ( int32_t i = 0; i <= fallback.size ( ) && i < room; i++ )
				out [ i ] = fallback.text ( ) [ i ];

			return;

		}

		out [ length ] = 0;

	}

	bool app::open_file ( const wchar_t* path ) {

		base::writer problem;
		double started = base::seconds ( );

		close_file ( );

		if ( !file.open ( path, problem ) ) {

			load_problem.set ( problem.text ( ) );
			load_time = clock;
			log ( log_kind::FAIL, base::writer ( ).add ( L"Couldn't load " ).add ( path ).add ( L". " ).add ( problem.text ( ) ).text ( ) );

			return false;

		}

		load_problem.set ( L"" );
		build_listing ( );

		for ( int32_t i = 0; i < static_cast < int32_t > ( panel::COUNT ); i++ ) {

			filters [ i ] = field ( );
			browser_scroll [ i ] = scroller ( );
			refresh_filter ( static_cast < panel > ( i ) );

		}

		listing_scroll = scroller ( );
		hex_scroll = scroller ( );
		headers_scroll = scroller ( );
		back_stack.clear ( );
		forward_stack.clear ( );
		cursor_ready = false;
		showing = view::LISTING;
		remember_recent ( path );

		base::writer line;

		line.add ( L"Loaded " ).add ( file.name.get ( ) ).add ( L" (" ).grouped ( file.size ).add ( L" bytes) in " ).fixed ( ( base::seconds ( ) - started ) * 1000.0, 1 ).add ( L" ms" );
		log ( log_kind::GOOD, line.text ( ) );

		for ( const base::text& note : file.notes )
			log ( log_kind::INFO, note.get ( ) );

		line.clear ( );
		line.grouped ( static_cast < uint64_t > ( file.imports.count ( ) ) ).add ( L" imports from " ).number ( file.modules.count ( ) ).add ( L" modules, " ).grouped ( static_cast < uint64_t > ( file.exports.count ( ) ) ).add ( L" exports, " ).grouped ( static_cast < uint64_t > ( file.strings.count ( ) ) ).add ( L" strings, " ).grouped ( total_rows ).add ( L" listing rows" );
		log ( log_kind::INFO, line.text ( ) );

		if ( !file.debug_path.empty ( ) )
			log ( log_kind::INFO, base::writer ( ).add ( L"Debug info points to " ).add ( file.debug_path.get ( ) ).text ( ) );

		if ( file.type != image::format::RAW )
			log ( log_kind::WARN, L"Code sections are shown as raw bytes until the x86-64 decoder lands." );

		if ( file.entry )
			go ( file.entry, false );
		else
			set_cursor_row ( 0, false );

		listing_scroll.offset = listing_scroll.target;
		hex_scroll.offset = hex_scroll.target;
		pending = base::max ( pending, 4 );

		return true;

	}

	void app::close_file ( ) {

		if ( !file.loaded ( ) )
			return;

		log ( log_kind::INFO, base::writer ( ).add ( L"Closed " ).add ( file.name.get ( ) ).text ( ) );
		file.close ( );
		items.clear ( );
		names.clear ( );
		total_rows = 0;
		cursor_row = 0;
		cursor_address = 0;

		for ( int32_t i = 0; i < static_cast < int32_t > ( panel::COUNT ); i++ )
			shown [ i ].clear ( );

	}

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

	void app::run_command_line ( ) {

		wchar_t line [ 256 ];
		int32_t length = command_field.length;

		for ( int32_t i = 0; i <= length; i++ )
			line [ i ] = command_field.text [ i ];

		command_field = field ( );

		if ( !length )
			return;

		log ( log_kind::INFO, base::writer ( ).add ( L"> " ).add ( line ).text ( ) );

		wchar_t* word = line;
		wchar_t* rest = line;

		while ( *rest && *rest != L' ' )
			rest++;

		if ( *rest ) {

			*rest = 0;
			rest++;

			while ( *rest == L' ' )
				rest++;

		}

		uint64_t address = 0;

		if ( base::same_text ( word, L"help" ) )
			log ( log_kind::INFO, L"Commands: go <address|name>, entry, back, forward, section <name>, view <listing|hex|headers>, theme <light|dark>, open, close, clear" );
		else if ( base::same_text ( word, L"clear" ) )
			output.clear ( );
		else if ( base::same_text ( word, L"open" ) )
			run ( command::OPEN );
		else if ( base::same_text ( word, L"close" ) )
			run ( command::CLOSE );
		else if ( base::same_text ( word, L"entry" ) )
			run ( command::GO_ENTRY );
		else if ( base::same_text ( word, L"back" ) )
			run ( command::BACK );
		else if ( base::same_text ( word, L"forward" ) )
			run ( command::FORWARD );
		else if ( base::same_text ( word, L"theme" ) )
			set_theme ( base::same_text ( rest, L"light" ) || ( !rest [ 0 ] && !light ) );
		else if ( base::same_text ( word, L"view" ) ) {

			if ( base::same_text ( rest, L"hex" ) )
				run ( command::VIEW_HEX );
			else if ( base::same_text ( rest, L"headers" ) )
				run ( command::VIEW_HEADERS );
			else
				run ( command::VIEW_LISTING );

		} else if ( ( base::same_text ( word, L"go" ) || base::same_text ( word, L"section" ) ) && file.loaded ( ) ) {

			if ( parse_address ( rest, address ) )
				go ( address, true );
			else
				log ( log_kind::FAIL, base::writer ( ).add ( L"Nothing called \"" ).add ( rest ).add ( L"\" in this image." ).text ( ) );

		} else if ( file.loaded ( ) && parse_address ( word, address ) )
			go ( address, true );
		else
			log ( log_kind::FAIL, base::writer ( ).add ( L"Unknown command \"" ).add ( word ).add ( L"\". Type help for the list." ).text ( ) );

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

	void app::layout ( ) {

		bool has = file.loaded ( );
		float top = 38.0f;
		float bottom = h - 24.0f;

		title_area = math::rect ( 0.0f, 0.0f, w, top );
		status_area = math::rect ( 0.0f, bottom, w, 24.0f );
		approach ( welcome_show, has ? 0.0f : 1.0f, 12.0f );
		spring ( browser_show, browser_velocity, browser_open && has ? 1.0f : 0.0f, 320.0f, 34.0f );
		spring ( inspector_show, inspector_velocity, inspector_open && has ? 1.0f : 0.0f, 320.0f, 34.0f );
		spring ( output_show, output_velocity, output_open && has ? 1.0f : 0.0f, 320.0f, 34.0f );

		float file_show = 1.0f - welcome_show;
		float band = has ? 18.0f * file_show : 0.0f;

		band_area = math::rect ( 0.0f, top, w, band );
		top += band;

		float rail = 46.0f * file_show;

		browser_width = base::clamp ( browser_width, 200.0f, base::max ( w * 0.4f, 200.0f ) );
		inspector_width = base::clamp ( inspector_width, 220.0f, base::max ( w * 0.35f, 220.0f ) );
		output_height = base::clamp ( output_height, 90.0f, base::max ( ( bottom - top ) * 0.6f, 90.0f ) );

		float excess = browser_width + inspector_width + 480.0f - ( w - rail );

		browser_fit = browser_width;
		inspector_fit = inspector_width;

		if ( excess > 0.0f && inspector_open ) {

			float cut = base::min ( excess, base::max ( inspector_fit - 230.0f, 0.0f ) );

			inspector_fit -= cut;
			excess -= cut;

		}

		if ( excess > 0.0f && browser_open ) {

			float cut = base::min ( excess, base::max ( browser_fit - 210.0f, 0.0f ) );

			browser_fit -= cut;
			excess -= cut;

		}

		bool cramped = excess > 0.0f && w - rail - browser_fit * ( browser_open ? 1.0f : 0.0f ) - inspector_fit < 420.0f;
		float browser = browser_fit * math::saturate ( browser_show );
		float inspector = cramped ? 0.0f : inspector_fit * math::saturate ( inspector_show );

		rail_area = math::rect ( 0.0f, top, rail, bottom - top );
		browser_area = math::rect ( rail, top, browser, bottom - top );
		inspector_area = math::rect ( w - inspector, top, inspector, bottom - top );
		center_area = math::rect ( browser_area.right ( ), top, inspector_area.x - browser_area.right ( ), bottom - top );

		float outputs = output_height * math::saturate ( output_show );

		output_area = math::rect ( center_area.x, bottom - outputs, center_area.w, outputs );
		tabs_area = math::rect ( center_area.x, top, center_area.w, has ? 34.0f : 0.0f );
		view_area = math::rect ( center_area.x, tabs_area.bottom ( ), center_area.w, output_area.y - tabs_area.bottom ( ) );

	}

	void app::frame ( double seconds ) {

		begin_frame ( seconds );
		begin_input ( );
		handle_keys ( );
		update_theme ( );
		layout ( );
		canvas.begin ( gpu, dpi, math::vec2 ( 0.0f, 0.0f ), static_cast < float > ( clock ) );
		canvas.fill ( math::rect ( 0.0f, 0.0f, w, h ), 0.0f, ink ( tone::BASE ) );

		if ( welcome_show > 0.01f )
			draw_welcome ( math::rect ( 0.0f, title_area.bottom ( ), w, status_area.y - title_area.bottom ( ) ) );

		if ( file.loaded ( ) ) {

			canvas.push_alpha ( 1.0f - welcome_show );
			draw_band ( );
			draw_tabs ( );
			draw_view ( );
			draw_output ( );
			draw_browser ( );
			draw_inspector ( );
			draw_rail ( );
			canvas.pop ( );

		}

		draw_status ( );
		draw_title ( );
		draw_menu ( );
		draw_palette ( );
		draw_dialogs ( );
		draw_tip ( );

		if ( focused_field && clock - caret_time < 30.0 )
			pending = base::max ( pending, 1 );

		end_input ( );
		gpu->next_frame ( );
		gpu->prepare ( width, height );
		gpu->draw ( canvas, gpu->chrome, nullptr, true, ink ( tone::BASE ) );

		if ( pending > 0 )
			pending--;

	}

}