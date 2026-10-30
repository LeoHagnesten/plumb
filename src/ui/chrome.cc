#include "ui/app.hh"
#include "ui/icons.hh"
#include <windows.h>

namespace ui {

	struct menu_spec {

		const wchar_t* title;
		command items [ 12 ];
		int32_t count;

	};

	static const menu_spec menus [ ] = {
		{ L"File", { command::OPEN, command::CLOSE, command::COUNT, command::EXIT }, 4 },
		{ L"Edit", { command::COPY, command::FIND, command::COUNT, command::PALETTE }, 4 },
		{ L"Jump", { command::GO_ADDRESS, command::GO_ENTRY, command::COUNT, command::BACK, command::FORWARD, command::COUNT, command::NEXT_SECTION, command::PREVIOUS_SECTION }, 8 },
		{ L"View", { command::VIEW_LISTING, command::VIEW_HEX, command::VIEW_HEADERS, command::VIEW_GRAPH, command::VIEW_PSEUDOCODE, command::COUNT, command::THEME }, 7 },
		{ L"Window", { command::SHOW_FUNCTIONS, command::SHOW_IMPORTS, command::SHOW_EXPORTS, command::SHOW_STRINGS, command::SHOW_SECTIONS, command::COUNT, command::TOGGLE_BROWSER, command::TOGGLE_INSPECTOR, command::TOGGLE_OUTPUT }, 9 },
		{ L"Analysis", { command::ANALYZE, command::DECODE_HERE }, 2 },
		{ L"Help", { command::SHORTCUTS, command::COUNT, command::ABOUT }, 3 }
	};

	static constexpr int32_t menu_count = 7;
	static float menu_x [ menu_count ];
	static float menu_w [ menu_count ];

	void app::draw_menu_bar ( math::rect area ) {

		render::canvas& c = canvas;
		float x = area.x;

		for ( int32_t i = 0; i < menu_count; i++ ) {

			float text_w = c.measure ( menus [ i ].title, render::font::UI );
			math::rect slot ( x, area.y + 7.0f, text_w + 20.0f, area.h - 14.0f );
			uint64_t id = make_id ( "menu_title", i );
			widget& item = state ( id );
			bool clicked = interact ( id, slot, item );

			menu_x [ i ] = slot.x;
			menu_w [ i ] = slot.w;

			if ( clicked ) {

				menu_open = menu_open == i ? -1 : i;
				menu_focus = -1;

			} else if ( menu_open >= 0 && menu_open != i && hot == id )
				menu_open = i;

			float lit = base::max ( item.hover, menu_open == i ? 1.0f : 0.0f );

			c.fill ( slot, 4.0f, ink ( tone::HOVER ).fade ( lit * 1.4f ) );
			c.text ( menus [ i ].title, render::font::UI, math::vec2 ( slot.x + 10.0f, slot.center ( ).y ), render::mix ( ink ( tone::INK_SOFT ), ink ( tone::INK ), lit ) );
			x = slot.right ( ) + 1.0f;

		}

	}

	void app::draw_title ( ) {

		render::canvas& c = canvas;
		math::rect area = title_area;

		c.fill ( area, 0.0f, ink ( tone::TITLE ) );
		c.fill ( math::rect ( 0.0f, area.bottom ( ) - 1.0f, w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		draw_brand ( math::vec2 ( 22.0f, area.center ( ).y ), 20.0f );
		c.text ( L"Plumb", render::font::BRAND, math::vec2 ( 38.0f, area.center ( ).y ), ink ( tone::INK ) );
		draw_menu_bar ( math::rect ( 92.0f, area.y, 520.0f, area.h ) );

		float buttons = 46.0f * 3.0f;
		float right_edge = w - buttons;

		if ( file.loaded ( ) ) {

			base::writer detail;

			detail.add ( file.format_name ( ) ).add ( L"  \x00B7  " ).add ( file.machine_name ( ) );

			float name_w = c.measure ( file.name.get ( ), render::font::UI_STRONG );
			float detail_w = c.measure ( detail.text ( ), render::font::SMALL );
			float total = name_w + 14.0f + detail_w;
			float x = base::max ( w * 0.5f - total * 0.5f, 640.0f );

			if ( x + total < right_edge - 12.0f ) {

				c.text ( file.name.get ( ), render::font::UI_STRONG, math::vec2 ( x, area.center ( ).y ), ink ( tone::INK ) );
				c.text ( detail.text ( ), render::font::SMALL, math::vec2 ( x + name_w + 14.0f, area.center ( ).y ), ink ( tone::INK_FAINT ) );

			}

		}

		const uint32_t glyphs [ 3 ] = { icons::MINIMIZE, maximized ? icons::RESTORE : icons::MAXIMIZE, icons::CLOSE };
		const wchar_t* labels [ 3 ] = { L"Minimize", maximized ? L"Restore" : L"Maximize", L"Close" };

		for ( int32_t i = 0; i < 3; i++ ) {

			math::rect slot ( right_edge + 46.0f * static_cast < float > ( i ), 0.0f, 46.0f, area.h - 1.0f );
			uint64_t id = make_id ( "window_button", i );
			widget& item = state ( id );
			bool clicked = interact ( id, slot, item, i == 1 ? hit::MAXIMIZE : hit::CLIENT );
			render::color wash = i == 2 ? render::rgb ( 0xC42B1C ) : ink ( tone::HOVER );
			float lit = i == 2 ? item.hover : item.hover * 1.4f;

			c.fill ( slot, 0.0f, wash.fade ( lit ) );
			c.icon ( glyphs [ i ], slot.center ( ), 10.0f, i == 2 ? render::mix ( ink ( tone::INK_SOFT ), render::rgb ( 0xFFFFFF ), item.hover ) : ink ( tone::INK_SOFT ) );
			tip ( id, labels [ i ], slot );

			if ( clicked )
				pending_request = i == 0 ? request::MINIMIZE : ( i == 1 ? request::MAXIMIZE : request::CLOSE );

		}

	}

	void app::draw_menu ( ) {

		spring ( menu_show, menu_velocity, menu_open >= 0 ? 1.0f : 0.0f, 900.0f, 60.0f );

		if ( menu_open < 0 ) {

			menu_show = 0.0f;
			menu_velocity = 0.0f;

			return;

		}

		render::canvas& c = canvas;
		const menu_spec& spec = menus [ menu_open ];
		float row_h = 28.0f;
		float width_menu = 290.0f;
		float height_menu = 8.0f;

		for ( int32_t i = 0; i < spec.count; i++ )
			height_menu += spec.items [ i ] == command::COUNT ? 9.0f : row_h;

		math::rect area ( menu_x [ menu_open ], title_area.bottom ( ) - 3.0f, width_menu, height_menu );
		uint64_t blocker = make_id ( "menu_blocker" );
		widget& block = state ( blocker );

		if ( interact ( blocker, math::rect ( 0.0f, title_area.bottom ( ), w, h - title_area.bottom ( ) ), block ) )
			menu_open = -1;

		float shown = math::saturate ( menu_show );

		c.push ( math::vec2 ( area.x, area.y ), 1.0f, shown, math::vec2 ( 0.0f, -4.0f * ( 1.0f - shown ) ) );
		c.shadow ( area, 6.0f, 18.0f, ink ( tone::SHADOW ).fade ( 0.55f ), math::vec2 ( 0.0f, 6.0f ) );
		c.outline ( area, 6.0f, ink ( tone::RAISED ), 1.0f, ink ( tone::LINE_STRONG ) );

		uint64_t surface = make_id ( "menu_surface" );
		widget& surface_state = state ( surface );

		interact ( surface, area, surface_state );

		float y = area.y + 4.0f;
		bool ran = false;

		for ( int32_t i = 0; i < spec.count; i++ ) {

			command action = spec.items [ i ];

			if ( action == command::COUNT ) {

				c.fill ( math::rect ( area.x + 10.0f, y + 4.0f, area.w - 20.0f, 1.0f ), 0.0f, ink ( tone::LINE ) );
				y += 9.0f;

				continue;

			}

			math::rect row ( area.x + 4.0f, y, area.w - 8.0f, row_h );
			uint64_t id = make_id ( "menu_item", static_cast < int64_t > ( action ) );
			widget& item = state ( id );
			bool usable = enabled ( action ) || action == command::ANALYZE || action == command::DECODE_HERE;
			bool clicked = interact ( id, row, item );
			render::color text = enabled ( action ) ? ink ( tone::INK ) : ink ( tone::INK_FAINT );

			c.fill ( row, 4.0f, ink ( tone::SELECT ).fade ( item.hover * ( usable ? 1.0f : 0.4f ) ) );

			if ( checked ( action ) )
				c.icon ( icons::CHECK, math::vec2 ( row.x + 14.0f, row.center ( ).y ), 11.0f, ink ( tone::ACCENT ) );

			c.text ( command_label ( action ), render::font::UI, math::vec2 ( row.x + 30.0f, row.center ( ).y ), text );
			c.text ( command_keys ( action ), render::font::SMALL, math::vec2 ( row.right ( ) - 10.0f, row.center ( ).y ), ink ( tone::INK_FAINT ), render::align::RIGHT );

			if ( clicked && !ran ) {

				ran = true;
				menu_open = -1;
				run ( action );

			}

			y += row_h;

		}

		c.pop ( );

	}

	static render::color band_color ( const app& owner, image::kind type, const render::color* tones ) {

		switch ( type ) {

			case image::kind::CODE:
				return tones [ static_cast < int32_t > ( tone::CODE_BAND ) ];

			case image::kind::CONSTANT:
				return tones [ static_cast < int32_t > ( tone::CONST_BAND ) ];

			case image::kind::EMPTY:
				return tones [ static_cast < int32_t > ( tone::EMPTY_BAND ) ];

			case image::kind::HEADER:
				return tones [ static_cast < int32_t > ( tone::HEADER_BAND ) ];

			default:
				return tones [ static_cast < int32_t > ( tone::DATA_BAND ) ];

		}

	}

	void app::draw_band ( ) {

		if ( band_area.h < 1.0f )
			return;

		render::canvas& c = canvas;
		math::rect area ( band_area.x + 8.0f, band_area.y + 4.0f, band_area.w - 16.0f, band_area.h - 7.0f );
		uint64_t low = file.low ( );
		uint64_t high = file.high ( );
		double span = static_cast < double > ( base::max ( high - low, static_cast < uint64_t > ( 1 ) ) );
		uint64_t id = make_id ( "band" );
		widget& item = state ( id );

		c.fill ( band_area, 0.0f, ink ( tone::PANEL ) );
		c.fill ( math::rect ( 0.0f, band_area.bottom ( ) - 1.0f, w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		c.fill ( area, 2.0f, ink ( tone::SUNKEN ) );
		interact ( id, area, item );

		auto x_of = [ & ] ( uint64_t address ) {

			return area.x + static_cast < float > ( static_cast < double > ( address - low ) / span ) * area.w;

		};

		for ( const image::section& part : file.sections ) {

			float x0 = x_of ( part.address );
			float x1 = x_of ( part.address + base::max ( part.size, part.raw ) );

			c.fill ( math::rect ( x0, area.y, base::max ( x1 - x0 - 1.0f, 1.5f ), area.h ), 1.0f, band_color ( *this, part.type, tones ) );

		}

		int32_t ticks = 0;

		for ( const image::symbol& function : file.functions ) {

			if ( ticks++ > 3000 )
				break;

			c.fill ( math::rect ( x_of ( function.address ) - 0.5f, area.y, 1.0f, 3.0f ), 0.0f, ink ( tone::LABEL ) );

		}

		float marker = x_of ( cursor_address );

		c.fill ( math::rect ( marker - 1.0f, area.y - 3.0f, 2.0f, area.h + 6.0f ), 1.0f, ink ( tone::INK ) );

		if ( item.hover > 0.01f && over ( area ) ) {

			float t = math::saturate ( ( mouse.x - area.x ) / area.w );
			uint64_t address = low + static_cast < uint64_t > ( t * span );
			int32_t owner = file.section_at ( address );
			static wchar_t label [ 96 ];
			base::writer line;

			line.add ( owner >= 0 ? file.sections [ static_cast < size_t > ( owner ) ].name : L"" ).add ( L":" ).hex ( address, file.wide ( ) ? 16 : 8 );

			for ( int32_t i = 0; i <= line.size ( ) && i < 95; i++ )
				label [ i ] = line.text ( ) [ i ];

			c.fill ( math::rect ( mouse.x - 0.5f, area.y, 1.0f, area.h ), 0.0f, ink ( tone::INK_SOFT ).fade ( 0.7f ) );
			tip ( id, label, math::rect ( mouse.x - 1.0f, area.y, 2.0f, area.h ) );
			wanted_cursor = pointer::HAND;

			if ( active_id == id && mouse_down ) {

				uint64_t offset = 0;

				if ( owner >= 0 && ( press_event || clock - last_press > 0.03 ) ) {

					go ( address, press_event );
					pending = base::max ( pending, 2 );

				}

				( void ) offset;

			}

		}

	}

	void app::draw_status ( ) {

		render::canvas& c = canvas;
		math::rect area = status_area;
		float cy = area.center ( ).y;

		c.fill ( area, 0.0f, ink ( tone::TITLE ) );
		c.fill ( math::rect ( 0.0f, area.y, w, 1.0f ), 0.0f, ink ( tone::LINE ) );

		bool has = file.loaded ( );
		render::color dot = has ? ink ( tone::SUCCESS ) : ink ( tone::INK_FAINT );

		c.circle ( math::vec2 ( 14.0f, cy ), 3.5f, dot );

		float x = 26.0f;

		x += c.text ( has ? L"Ready" : L"No file open", render::font::SMALL, math::vec2 ( x, cy ), ink ( tone::INK_SOFT ) ) + 16.0f;

		if ( has ) {

			c.fill ( math::rect ( x - 8.0f, area.y + 6.0f, 1.0f, area.h - 12.0f ), 0.0f, ink ( tone::LINE ) );
			x += c.text ( L"Analysis idle \x00B7 decoder not wired in", render::font::SMALL, math::vec2 ( x, cy ), ink ( tone::INK_FAINT ) ) + 16.0f;

			base::writer where;
			uint64_t offset = 0;
			int32_t owner = file.section_at ( cursor_address );

			if ( owner >= 0 )
				where.add ( file.sections [ static_cast < size_t > ( owner ) ].name ).add ( L":" );

			where.hex ( cursor_address, file.wide ( ) ? 16 : 8 );

			if ( file.type != image::format::RAW )
				where.add ( L"   RVA " ).hex ( cursor_address - file.base );

			if ( file.offset_of ( cursor_address, offset ) )
				where.add ( L"   File " ).hex ( offset );

			float right = w - 12.0f;
			base::writer info;

			info.add ( file.format_name ( ) ).add ( L" \x00B7 " ).add ( file.machine_name ( ) ).add ( L" \x00B7 " ).grouped ( file.size ).add ( L" bytes" );
			right -= c.text ( info.text ( ), render::font::SMALL, math::vec2 ( right, cy ), ink ( tone::INK_FAINT ), render::align::RIGHT ) + 18.0f;
			c.fill ( math::rect ( right + 8.0f, area.y + 6.0f, 1.0f, area.h - 12.0f ), 0.0f, ink ( tone::LINE ) );
			c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( right, cy ), ink ( tone::INK_SOFT ), render::align::RIGHT );

		} else {

			c.text ( light ? L"Light theme" : L"Dark theme", render::font::SMALL, math::vec2 ( w - 12.0f, cy ), ink ( tone::INK_FAINT ), render::align::RIGHT );

		}

	}

	static int32_t fuzzy_score ( const wchar_t* text, const wchar_t* query ) {

		if ( !query [ 0 ] )
			return 1;

		int32_t score = 0;
		int32_t q = 0;
		bool boundary = true;

		for ( int32_t i = 0; text [ i ] && query [ q ]; i++ ) {

			wchar_t a = text [ i ] >= L'A' && text [ i ] <= L'Z' ? static_cast < wchar_t > ( text [ i ] + 32 ) : text [ i ];
			wchar_t b = query [ q ] >= L'A' && query [ q ] <= L'Z' ? static_cast < wchar_t > ( query [ q ] + 32 ) : query [ q ];

			if ( a == b ) {

				score += boundary ? 8 : 2;
				q++;

			}

			boundary = text [ i ] == L' ';

		}

		return query [ q ] ? 0 : score;

	}

	void app::draw_palette ( ) {

		spring ( palette_show, palette_velocity, palette_open ? 1.0f : 0.0f, 700.0f, 52.0f );

		if ( palette_show <= 0.004f && !palette_open )
			return;

		render::canvas& c = canvas;
		float shown = math::saturate ( palette_show );
		float width_box = base::min ( 600.0f, w - 40.0f );
		float x = w * 0.5f - width_box * 0.5f;
		float y = title_area.bottom ( ) + 54.0f;
		uint64_t blocker = make_id ( "palette_blocker" );
		widget& block = state ( blocker );

		if ( palette_open && interact ( blocker, math::rect ( 0.0f, 0.0f, w, h ), block ) ) {

			palette_open = false;
			focus_id = 0;
			focused_field = nullptr;

		}

		c.fill ( math::rect ( 0.0f, 0.0f, w, h ), 0.0f, ink ( tone::SHADOW ).fade ( 0.35f * shown ) );

		if ( palette_open && palette_field.revision != palette_seen ) {

			palette_seen = palette_field.revision;
			palette_items.clear ( );
			palette_pick = 0;

			if ( !palette_address ) {

				for ( int32_t i = 0; i < static_cast < int32_t > ( command::COUNT ); i++ ) {

					int32_t score = fuzzy_score ( command_label ( static_cast < command > ( i ) ), palette_field.text );

					if ( !score )
						continue;

					palette_entry entry = { static_cast < command > ( i ), score + ( enabled ( static_cast < command > ( i ) ) ? 100 : 0 ) };
					int32_t at = palette_items.count ( );

					palette_items.push ( entry );

					while ( at > 0 && palette_items [ static_cast < size_t > ( at - 1 ) ].score < entry.score ) {

						palette_items [ static_cast < size_t > ( at ) ] = palette_items [ static_cast < size_t > ( at - 1 ) ];
						at--;

					}

					palette_items [ static_cast < size_t > ( at ) ] = entry;

				}

			}

		}

		int32_t visible = base::min ( palette_items.count ( ), 9 );
		float row_h = 34.0f;
		float list_h = palette_address ? 48.0f : static_cast < float > ( visible ) * row_h + ( visible ? 10.0f : 0.0f );
		math::rect box ( x, y, width_box, 52.0f + list_h );

		palette_pick = palette_items.count ( ) ? base::clamp ( palette_pick, 0, palette_items.count ( ) - 1 ) : 0;
		c.push ( math::vec2 ( box.center ( ).x, box.y ), 0.97f + 0.03f * shown, shown, math::vec2 ( 0.0f, -8.0f * ( 1.0f - shown ) ) );
		c.shadow ( box, 8.0f, 30.0f, ink ( tone::SHADOW ), math::vec2 ( 0.0f, 12.0f ) );
		c.outline ( box, 8.0f, ink ( tone::RAISED ), 1.0f, ink ( tone::LINE_STRONG ) );

		uint64_t surface = make_id ( "palette_surface" );
		widget& surface_state = state ( surface );

		interact ( surface, box, surface_state );

		bool submitted = text_field ( make_id ( "palette" ), palette_field, math::rect ( box.x + 10.0f, box.y + 10.0f, box.w - 20.0f, 32.0f ), palette_address ? L"Address, RVA (+1A30), symbol or section" : L"Type a command", palette_address ? icons::FORWARD : icons::SEARCH, palette_address ? render::font::MONO : render::font::UI );

		c.fill ( math::rect ( box.x, box.y + 51.0f, box.w, 1.0f ), 0.0f, ink ( tone::LINE ) );

		if ( palette_address ) {

			uint64_t address = 0;
			bool valid = file.loaded ( ) && parse_address ( palette_field.text, address );
			base::writer hint;

			if ( !file.loaded ( ) )
				hint.add ( L"Open a file first." );
			else if ( !palette_field.length )
				hint.add ( L"Enter jumps.  Plain hex is a virtual address, +hex is relative to the image base." );
			else if ( valid ) {

				int32_t owner = file.section_at ( address );

				hint.add ( L"Go to " ).add ( owner >= 0 ? file.sections [ static_cast < size_t > ( owner ) ].name : L"(outside the image)" ).add ( L":" ).hex ( address, file.wide ( ) ? 16 : 8 );

			} else
				hint.add ( L"That isn't an address or a known name." );

			c.text ( hint.text ( ), render::font::SMALL, math::vec2 ( box.x + 18.0f, box.y + 52.0f + 24.0f ), valid ? ink ( tone::INK_SOFT ) : ink ( tone::INK_FAINT ) );

			if ( submitted && valid ) {

				palette_open = false;
				focus_id = 0;
				focused_field = nullptr;
				go ( address, true );

			}

		} else {

			float target = static_cast < float > ( palette_pick );

			if ( palette_pick_y < -0.5f )
				palette_pick_y = target;

			spring ( palette_pick_y, palette_pick_velocity, target, 900.0f, 60.0f );

			float list_top = box.y + 57.0f;

			if ( visible )
				c.fill ( math::rect ( box.x + 6.0f, list_top + palette_pick_y * row_h, box.w - 12.0f, row_h ), 4.0f, ink ( tone::SELECT ) );

			for ( int32_t i = 0; i < visible; i++ ) {

				const palette_entry& entry = palette_items [ static_cast < size_t > ( i ) ];
				math::rect row ( box.x + 6.0f, list_top + static_cast < float > ( i ) * row_h, box.w - 12.0f, row_h );
				uint64_t id = make_id ( "palette_row", i );
				widget& item = state ( id );
				bool usable = enabled ( entry.action );

				if ( interact ( id, row, item ) ) {

					palette_pick = i;
					submitted = true;

				}

				if ( item.hover > 0.5f && mouse_inside )
					palette_pick = i;

				c.text ( command_label ( entry.action ), render::font::UI, math::vec2 ( row.x + 14.0f, row.center ( ).y ), usable ? ink ( tone::INK ) : ink ( tone::INK_FAINT ) );

				if ( !usable )
					c.text ( L"unavailable", render::font::SMALL, math::vec2 ( row.right ( ) - 14.0f - ( command_keys ( entry.action ) [ 0 ] ? c.measure ( command_keys ( entry.action ), render::font::SMALL ) + 24.0f : 0.0f ), row.center ( ).y ), ink ( tone::INK_FAINT ), render::align::RIGHT );

				key_cap ( math::vec2 ( row.right ( ) - 10.0f, row.center ( ).y ), command_keys ( entry.action ), ink ( tone::INK_SOFT ), true );

			}

			if ( submitted && palette_items.count ( ) ) {

				command action = palette_items [ static_cast < size_t > ( palette_pick ) ].action;

				palette_open = false;
				focus_id = 0;
				focused_field = nullptr;
				run ( action );

			}

		}

		c.pop ( );

	}

	void app::draw_dialogs ( ) {

		spring ( about_show, about_velocity, about_open ? 1.0f : 0.0f, 600.0f, 48.0f );
		spring ( keys_show, keys_velocity, keys_open ? 1.0f : 0.0f, 600.0f, 48.0f );

		render::canvas& c = canvas;
		float show = math::saturate ( base::max ( about_show, keys_show ) );

		if ( show <= 0.004f )
			return;

		uint64_t blocker = make_id ( "dialog_blocker" );
		widget& block = state ( blocker );

		if ( interact ( blocker, math::rect ( 0.0f, 0.0f, w, h ), block ) ) {

			about_open = false;
			keys_open = false;

		}

		c.fill ( math::rect ( 0.0f, 0.0f, w, h ), 0.0f, ink ( tone::SHADOW ).fade ( 0.45f * show ) );

		bool about = about_show >= keys_show;
		float box_w = about ? 440.0f : 620.0f;
		float box_h = about ? 300.0f : 520.0f;
		math::rect box ( w * 0.5f - box_w * 0.5f, h * 0.5f - box_h * 0.5f, box_w, box_h );

		c.push ( box.center ( ), 0.96f + 0.04f * show, show );
		c.shadow ( box, 8.0f, 34.0f, ink ( tone::SHADOW ), math::vec2 ( 0.0f, 14.0f ) );
		c.outline ( box, 8.0f, ink ( tone::RAISED ), 1.0f, ink ( tone::LINE_STRONG ) );

		uint64_t surface = make_id ( "dialog_surface" );
		widget& surface_state = state ( surface );

		interact ( surface, box, surface_state );

		if ( icon_button ( make_id ( "dialog_close" ), math::vec2 ( box.right ( ) - 22.0f, box.y + 22.0f ), 30.0f, icons::CANCEL, 11.0f, L"Close" ) ) {

			about_open = false;
			keys_open = false;

		}

		if ( about ) {

			draw_brand ( math::vec2 ( box.x + 54.0f, box.y + 62.0f ), 50.0f );
			c.text ( L"Plumb", render::font::TITLE, math::vec2 ( box.x + 92.0f, box.y + 52.0f ), ink ( tone::INK ) );
			c.text ( L"Version 0.1 \x00B7 baseline interface", render::font::SMALL, math::vec2 ( box.x + 93.0f, box.y + 78.0f ), ink ( tone::INK_SOFT ) );
			c.paragraph ( L"A workbench for taking Windows binaries apart. Plumb maps the file, reads its PE structure, imports, exports and strings, and lays it out as a listing and a hex view. The instruction decoder plugs into the listing next.", render::font::UI, math::vec2 ( box.x + 28.0f, box.y + 116.0f ), box.w - 56.0f, ink ( tone::INK ) );
			c.fill ( math::rect ( box.x + 28.0f, box.bottom ( ) - 70.0f, box.w - 56.0f, 1.0f ), 0.0f, ink ( tone::LINE ) );
			c.text ( L"No C runtime, no dependencies. Direct3D 11 and DirectWrite.", render::font::SMALL, math::vec2 ( box.x + 28.0f, box.bottom ( ) - 46.0f ), ink ( tone::INK_FAINT ) );
			c.text ( L"Windows 10 and 11", render::font::SMALL, math::vec2 ( box.x + 28.0f, box.bottom ( ) - 28.0f ), ink ( tone::INK_FAINT ) );

		} else {

			c.text ( L"Keyboard shortcuts", render::font::HEADING, math::vec2 ( box.x + 28.0f, box.y + 30.0f ), ink ( tone::INK ) );

			const command order [ ] = { command::OPEN, command::CLOSE, command::PALETTE, command::GO_ADDRESS, command::GO_ENTRY, command::BACK, command::FORWARD, command::NEXT_SECTION, command::PREVIOUS_SECTION, command::FIND, command::COPY, command::VIEW_LISTING, command::VIEW_HEX, command::VIEW_HEADERS, command::VIEW_GRAPH, command::VIEW_PSEUDOCODE, command::SHOW_FUNCTIONS, command::SHOW_IMPORTS, command::SHOW_EXPORTS, command::SHOW_STRINGS, command::SHOW_SECTIONS, command::TOGGLE_BROWSER, command::TOGGLE_INSPECTOR, command::TOGGLE_OUTPUT, command::THEME };
			int32_t total = static_cast < int32_t > ( sizeof ( order ) / sizeof ( order [ 0 ] ) );
			int32_t half = ( total + 1 ) / 2;
			float column = ( box.w - 56.0f - 24.0f ) * 0.5f;

			for ( int32_t i = 0; i < total; i++ ) {

				float cx = box.x + 28.0f + ( i < half ? 0.0f : column + 24.0f );
				float cy = box.y + 72.0f + static_cast < float > ( i < half ? i : i - half ) * 33.0f;

				c.text ( command_label ( order [ i ] ), render::font::UI, math::vec2 ( cx, cy ), ink ( tone::INK ) );
				key_cap ( math::vec2 ( cx + column, cy ), command_keys ( order [ i ] ), ink ( tone::INK_SOFT ), true );

			}

		}

		c.pop ( );

	}

	void app::draw_welcome ( math::rect area ) {

		render::canvas& c = canvas;
		float shown = math::saturate ( welcome_show );
		float column = base::min ( 520.0f, area.w - 80.0f );
		float x = area.center ( ).x - column * 0.5f;
		float top = area.y + base::max ( area.h * 0.5f - 250.0f, 40.0f );

		c.push ( math::vec2 ( area.center ( ).x, top ), 1.0f, shown, math::vec2 ( 0.0f, 10.0f * ( 1.0f - shown ) ) );

		for ( int32_t i = 0; i < 9; i++ ) {

			float gx = area.x + 40.0f + static_cast < float > ( i ) * ( area.w - 80.0f ) / 8.0f;

			c.fill ( math::rect ( gx, area.y + 30.0f, 1.0f, area.h - 60.0f ), 0.0f, ink ( tone::LINE ).fade ( 0.35f ) );

		}

		draw_brand ( math::vec2 ( x + 28.0f, top + 34.0f ), 64.0f );
		c.text ( L"Plumb", render::font::DISPLAY, math::vec2 ( x + 74.0f, top + 22.0f ), ink ( tone::INK ) );
		c.text ( L"Take a binary apart, one byte at a time.", render::font::UI, math::vec2 ( x + 76.0f, top + 56.0f ), ink ( tone::INK_SOFT ) );

		float y = top + 104.0f;

		if ( button ( make_id ( "welcome_open" ), math::rect ( x, y, 220.0f, 38.0f ), L"Open a binary", icons::FOLDER_OPEN, true ) )
			run ( command::OPEN );

		if ( button ( make_id ( "welcome_self" ), math::rect ( x + 232.0f, y, 200.0f, 38.0f ), L"Inspect Plumb itself", icons::CHIP, false ) ) {

			wchar_t self [ 1024 ];
			DWORD length = GetModuleFileNameW ( nullptr, self, 1023 );

			if ( length )
				open_file ( self );

		}

		y += 58.0f;

		if ( !load_problem.empty ( ) ) {

			math::rect warning ( x, y, column, 40.0f );

			c.outline ( warning, 4.0f, ink ( tone::DANGER ).fade ( 0.1f ), 1.0f, ink ( tone::DANGER ).fade ( 0.5f ) );
			c.icon ( icons::FAILURE, math::vec2 ( warning.x + 18.0f, warning.center ( ).y ), 13.0f, ink ( tone::DANGER ) );
			c.text ( load_problem.get ( ), render::font::SMALL, math::vec2 ( warning.x + 36.0f, warning.center ( ).y ), ink ( tone::INK ), render::align::LEFT, column - 48.0f );
			y += 56.0f;

		}

		section_heading ( math::rect ( x, y, column, 18.0f ), L"RECENT", recent.empty ( ) ? nullptr : L"" );
		y += 26.0f;
		c.fill ( math::rect ( x, y - 4.0f, column, 1.0f ), 0.0f, ink ( tone::LINE ) );

		if ( recent.empty ( ) ) {

			c.text ( L"Nothing yet. Files you open show up here.", render::font::UI, math::vec2 ( x, y + 16.0f ), ink ( tone::INK_FAINT ) );
			y += 40.0f;

		}

		for ( int32_t i = 0; i < recent.count ( ); i++ ) {

			const wchar_t* path = recent [ static_cast < size_t > ( i ) ].get ( );
			const wchar_t* tail = path;

			for ( const wchar_t* at = path; *at; at++ ) {

				if ( *at == L'\\' || *at == L'/' )
					tail = at + 1;

			}

			math::rect row ( x - 8.0f, y, column + 16.0f, 40.0f );
			uint64_t id = make_id ( "recent", i );
			widget& item = state ( id );

			if ( interact ( id, row, item ) )
				open_file ( path );

			c.fill ( row, 4.0f, ink ( tone::HOVER ).fade ( item.hover * 1.5f ) );
			c.icon ( icons::DOCUMENT, math::vec2 ( x + 10.0f, row.center ( ).y ), 14.0f, ink ( tone::INK_FAINT ) );
			c.text ( tail, render::font::UI_STRONG, math::vec2 ( x + 30.0f, row.y + 13.0f ), ink ( tone::INK ) );
			c.text ( path, render::font::SMALL, math::vec2 ( x + 30.0f, row.y + 28.0f ), ink ( tone::INK_FAINT ), render::align::LEFT, column - 40.0f );
			y += 42.0f;

		}

		y += 18.0f;

		const command hints [ 4 ] = { command::OPEN, command::PALETTE, command::GO_ADDRESS, command::THEME };

		for ( int32_t i = 0; i < 4; i++ ) {

			float hx = x + static_cast < float > ( i % 2 ) * ( column * 0.5f + 8.0f );
			float hy = y + static_cast < float > ( i / 2 ) * 30.0f;

			key_cap ( math::vec2 ( hx, hy ), command_keys ( hints [ i ] ), ink ( tone::INK_SOFT ), false );
			c.text ( command_label ( hints [ i ] ), render::font::SMALL, math::vec2 ( hx + c.measure ( command_keys ( hints [ i ] ), render::font::SMALL ) + 22.0f, hy ), ink ( tone::INK_FAINT ) );

		}

		c.text ( L"Or drop any .exe, .dll or file onto this window.", render::font::SMALL, math::vec2 ( x, y + 76.0f ), ink ( tone::INK_FAINT ) );
		c.pop ( );

	}

}