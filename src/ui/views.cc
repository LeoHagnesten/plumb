#include "ui/app.hh"
#include "ui/icons.hh"

namespace ui {

	static const wchar_t* view_titles [ static_cast < int32_t > ( view::COUNT ) ] = { L"Listing", L"Hex", L"Headers", L"Graph", L"Pseudocode" };

	static void ida_number ( base::writer& out, uint64_t value ) {

		if ( value < 10 ) {

			out.number ( static_cast < int64_t > ( value ) );

			return;

		}

		base::writer digits;

		digits.hex ( value );

		if ( digits.text ( ) [ 0 ] >= L'A' )
			out.add ( L'0' );

		out.add ( digits.text ( ) ).add ( L'h' );

	}

	static wchar_t printable ( uint8_t value ) {

		return value >= 0x20 && value < 0x7F ? static_cast < wchar_t > ( value ) : L'.';

	}

	void app::draw_tabs ( ) {

		render::canvas& c = canvas;
		math::rect area = tabs_area;

		c.fill ( area, 0.0f, ink ( tone::PANEL ) );
		c.fill ( math::rect ( area.x, area.bottom ( ) - 1.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		c.push_clip ( area );

		float x = area.x + 8.0f;
		float active_x = 0.0f;
		float active_w = 0.0f;

		for ( int32_t i = 0; i < static_cast < int32_t > ( view::COUNT ); i++ ) {

			float text_w = c.measure ( view_titles [ i ], render::font::UI );
			math::rect slot ( x, area.y, text_w + 28.0f, area.h - 1.0f );
			uint64_t id = make_id ( "tab", i );
			widget& item = state ( id );
			bool selected = showing == static_cast < view > ( i );
			bool pending_view = i >= static_cast < int32_t > ( view::GRAPH );

			if ( interact ( id, slot, item ) )
				run ( static_cast < command > ( static_cast < int32_t > ( command::VIEW_LISTING ) + i ) );

			if ( selected ) {

				active_x = slot.x + 8.0f;
				active_w = slot.w - 16.0f;

			}

			c.fill ( math::rect ( slot.x + 3.0f, slot.y + 5.0f, slot.w - 6.0f, slot.h - 10.0f ), 4.0f, ink ( tone::HOVER ).fade ( item.hover * 1.2f ) );
			c.text ( view_titles [ i ], render::font::UI, math::vec2 ( slot.x + 14.0f, slot.center ( ).y ), selected ? ink ( tone::INK ) : ( pending_view ? ink ( tone::INK_FAINT ) : render::mix ( ink ( tone::INK_SOFT ), ink ( tone::INK ), item.hover ) ) );
			x = slot.right ( );

		}

		if ( !tab_ready ) {

			tab_x = active_x;
			tab_w = active_w;
			tab_ready = true;

		}

		spring ( tab_x, tab_vx, active_x, 520.0f, 42.0f );
		spring ( tab_w, tab_vw, active_w, 520.0f, 42.0f );
		c.fill ( math::rect ( tab_x, area.bottom ( ) - 2.5f, tab_w, 2.0f ), 1.0f, ink ( tone::ACCENT ) );

		float right = area.right ( ) - 10.0f;
		base::writer where;
		int32_t owner = file.section_at ( cursor_address );

		if ( owner >= 0 )
			where.add ( file.sections [ static_cast < size_t > ( owner ) ].name ).add ( L":" );

		where.hex ( cursor_address, file.wide ( ) ? 16 : 8 );

		if ( right - x < 104.0f ) {

			c.pop_clip ( );

			return;

		}

		if ( icon_button ( make_id ( "tab_go" ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::SEARCH, 13.0f, L"Go to address (G)" ) )
			run ( command::GO_ADDRESS );

		right -= 32.0f;

		if ( icon_button ( make_id ( "tab_forward" ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::FORWARD, 13.0f, L"Forward (Alt+Right)" ) )
			run ( command::FORWARD );

		right -= 30.0f;

		if ( icon_button ( make_id ( "tab_back" ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::BACK, 13.0f, L"Back (Alt+Left)" ) )
			run ( command::BACK );

		right -= 40.0f;

		if ( right - x > 160.0f )
			c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( right, area.center ( ).y ), ink ( tone::INK_SOFT ), render::align::RIGHT );

		c.pop_clip ( );

	}

	void app::draw_view ( ) {

		if ( view_area.h < 4.0f || view_area.w < 4.0f )
			return;

		approach ( view_fade, 1.0f, 14.0f );

		render::canvas& c = canvas;
		float fade = math::saturate ( view_fade );

		c.push ( math::vec2 ( view_area.x, view_area.y ), 1.0f, 0.4f + 0.6f * fade, math::vec2 ( 0.0f, 4.0f * ( 1.0f - fade ) ) );

		switch ( showing ) {

			case view::HEX:
				draw_hex ( view_area );
				break;

			case view::HEADERS:
				draw_headers ( view_area );
				break;

			case view::GRAPH:
				draw_pending ( view_area, L"Control-flow graph", L"Basic blocks and branches appear here once the x86-64 decoder can walk the code. Jump to a function and switch views with Ctrl+4.", icons::BRANCH );
				break;

			case view::PSEUDOCODE:
				draw_pending ( view_area, L"Pseudocode", L"Lifted, readable code for the function under the cursor. This view sits on top of the decoder and the graph, so it comes after both.", icons::CODE );
				break;

			default:
				draw_listing ( view_area );
				break;

		}

		c.pop ( );

	}

	void app::draw_pending ( math::rect area, const wchar_t* title, const wchar_t* detail, uint32_t glyph ) {

		render::canvas& c = canvas;
		float column = base::min ( 420.0f, area.w - 60.0f );
		float x = area.center ( ).x - column * 0.5f;
		float y = area.y + area.h * 0.36f;

		c.push_clip ( area );

		for ( int32_t i = 0; i < 3; i++ ) {

			math::rect node ( area.center ( ).x - 70.0f + static_cast < float > ( i % 2 ) * 90.0f, y - 150.0f + static_cast < float > ( i ) * 34.0f, 70.0f, 22.0f );

			c.outline ( node, 3.0f, ink ( tone::PANEL ), 1.0f, ink ( tone::LINE_STRONG ) );
			c.fill ( math::rect ( node.x + 8.0f, node.center ( ).y - 1.0f, node.w * ( 0.4f + 0.15f * static_cast < float > ( i ) ), 2.0f ), 1.0f, ink ( tone::LINE_STRONG ) );

		}

		c.icon ( glyph, math::vec2 ( x + 14.0f, y + 2.0f ), 18.0f, ink ( tone::ACCENT ) );
		c.text ( title, render::font::HEADING, math::vec2 ( x + 36.0f, y + 2.0f ), ink ( tone::INK ) );
		c.paragraph ( detail, render::font::UI, math::vec2 ( x, y + 22.0f ), column, ink ( tone::INK_SOFT ) );
		c.pop_clip ( );

	}

	void app::draw_listing ( math::rect area ) {

		render::canvas& c = canvas;
		float row_h = 20.0f;
		float adv = c.measure ( L"0000000000", render::font::MONO ) * 0.1f;
		uint64_t id = make_id ( "listing" );
		widget& surface = state ( id );
		float visible = area.h / row_h;

		c.fill ( area, 0.0f, ink ( tone::BASE ) );
		scroll_region ( id, area );
		listing_scroll.target -= take_wheel ( id ) * 3.0f;
		glide ( listing_scroll, static_cast < float > ( total_rows ) - visible + 3.0f );
		interact ( id, area, surface );

		float offset = base::max ( listing_scroll.offset, 0.0f );
		uint64_t first = static_cast < uint64_t > ( offset );
		float shift = ( offset - static_cast < float > ( first ) ) * row_h;
		int32_t count = static_cast < int32_t > ( visible ) + 2;

		if ( active_id == id && press_event && over ( area ) ) {

			uint64_t row = first + static_cast < uint64_t > ( base::max ( ( mouse.y - area.y + shift ) / row_h, 0.0f ) );

			if ( row < total_rows )
				set_cursor_row ( row, false );

		}

		if ( !cursor_ready ) {

			cursor_y = static_cast < float > ( cursor_row );
			cursor_ready = true;

		}

		float row_target = static_cast < float > ( cursor_row );

		if ( math::abs ( row_target - cursor_y ) > 40.0f )
			cursor_y = row_target;

		spring ( cursor_y, cursor_velocity, row_target, 900.0f, 60.0f );

		float x0 = area.x + 14.0f;
		bool show_bytes = area.w >= adv * 118.0f;
		float x_bytes = x0 + adv * 25.0f;
		float x_mnem = show_bytes ? x_bytes + adv * 25.0f : x_bytes;
		float x_ops = x_mnem + adv * 7.0f;
		float x_note = base::max ( base::min ( x_ops + adv * 50.0f, area.right ( ) - adv * 24.0f ), x_ops + adv * 14.0f );
		float ops_room = x_note - x_ops - adv * 2.0f;
		uint32_t digits = file.wide ( ) ? 16 : 8;
		int32_t hover_row = over ( area ) ? static_cast < int32_t > ( ( mouse.y - area.y + shift ) / row_h ) : -1;

		c.push_clip ( area );

		for ( int32_t pass = 0; pass < 2; pass++ ) {

			if ( pass == 1 ) {

				float cy = area.y + ( cursor_y - offset ) * row_h;

				if ( cy > area.y - row_h && cy < area.bottom ( ) ) {

					c.fill ( math::rect ( area.x, cy, area.w, row_h ), 0.0f, ink ( tone::SELECT ) );
					c.fill ( math::rect ( area.x, cy, 2.5f, row_h ), 0.0f, ink ( tone::SELECT_LINE ) );

				}

			}

			for ( int32_t k = 0; k < count; k++ ) {

				uint64_t row = first + static_cast < uint64_t > ( k );

				if ( row >= total_rows )
					break;

				int32_t index = item_at ( row );

				if ( index < 0 )
					break;

				const line_item& item = items [ static_cast < size_t > ( index ) ];
				float y = area.y + static_cast < float > ( k ) * row_h - shift;
				float cy = y + row_h * 0.5f;
				uint32_t sub = static_cast < uint32_t > ( row - item.first_row );
				const image::section& owner = file.sections [ static_cast < size_t > ( item.section ) ];

				if ( pass == 0 ) {

					if ( item.kind == line_kind::BANNER && sub == 0 ) {

						c.fill ( math::rect ( area.x, y + 1.0f, area.w, row_h - 2.0f ), 0.0f, ink ( tone::PANEL ) );
						c.fill ( math::rect ( area.x, y + 1.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );
						c.fill ( math::rect ( area.x, y + row_h - 2.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );

					} else if ( item.kind == line_kind::LABEL && item.function )
						c.fill ( math::rect ( x0, y + 1.0f, area.w - 28.0f, 1.0f ), 0.0f, ink ( tone::LINE_STRONG ) );

					if ( k == hover_row && item.kind != line_kind::BANNER && row != cursor_row )
						c.fill ( math::rect ( area.x, y, area.w, row_h ), 0.0f, ink ( tone::HOVER ) );

					continue;

				}

				if ( item.kind == line_kind::BLANK || ( item.kind == line_kind::BANNER && sub > 0 ) )
					continue;

				uint64_t address = item.address + ( item.kind == line_kind::CODE || item.kind == line_kind::DATA ? static_cast < uint64_t > ( sub ) * item.stride : 0 );
				base::writer text;

				if ( item.kind == line_kind::BANNER ) {

					base::writer detail;
					uint64_t span = base::max ( owner.size, owner.raw );

					detail.hex ( owner.address, digits ).add ( L" \x2013 " ).hex ( owner.address + span, digits ).add ( L"   \x00B7   " ).add ( image::kind_name ( owner.type ) ).add ( L"   \x00B7   " ).add ( owner.read ? L"R" : L"-" ).add ( owner.write ? L"W" : L"-" ).add ( owner.execute ? L"X" : L"-" ).add ( L"   \x00B7   " ).grouped ( span ).add ( L" bytes" );
					c.text ( L"SECTION", render::font::TINY, math::vec2 ( x0, cy ), ink ( tone::INK_FAINT ) );
					c.text ( owner.name, render::font::MONO_STRONG, math::vec2 ( x0 + 62.0f, cy ), ink ( tone::LABEL ) );
					c.text ( detail.text ( ), render::font::MONO_SMALL, math::vec2 ( x0 + 62.0f + adv * 9.0f, cy ), ink ( tone::INK_SOFT ) );

					continue;

				}

				text.add ( owner.name ).add ( L':' ).hex ( address, static_cast < int32_t > ( digits ) );
				c.text ( text.text ( ), render::font::MONO, math::vec2 ( x0, cy ), ink ( tone::ADDRESS ) );

				uint64_t offset_at = 0;
				bool mapped = file.offset_of ( address, offset_at );
				uint64_t end = item.address + item.size;
				uint32_t take = static_cast < uint32_t > ( base::min ( static_cast < uint64_t > ( item.stride ), end > address ? end - address : 0 ) );

				switch ( item.kind ) {

					case line_kind::LABEL: {

						base::writer label;
						const wchar_t* note = nullptr;

						if ( item.ref >= 0 ) {

							label.add ( names [ static_cast < size_t > ( item.ref ) ].get ( ) );

							if ( item.ref < file.functions.count ( ) ) {

								image::symbol_kind kind = file.functions [ static_cast < size_t > ( item.ref ) ].type;

								note = kind == image::symbol_kind::ENTRY ? L"; entry point" : ( kind == image::symbol_kind::TLS ? L"; TLS callback" : L"; exported function" );

							} else
								note = L"; exported data";

						} else {

							wchar_t generated [ 40 ];

							label_name ( -item.ref - 2, generated, 40 );
							label.add ( generated );

						}

						label.add ( L':' );

						float label_w = c.text ( label.text ( ), render::font::MONO_STRONG, math::vec2 ( x_bytes, cy ), ink ( tone::LABEL ) );

						if ( note )
							c.text ( note, render::font::MONO, math::vec2 ( base::max ( x_note, x_bytes + label_w + adv * 2.0f ), cy ), ink ( tone::COMMENT ) );

						break;

					}

					case line_kind::CODE:
					case line_kind::DATA: {

						base::writer bytes;
						base::writer operands;
						wchar_t ascii [ 20 ];

						for ( uint32_t b = 0; b < take && mapped; b++ ) {

							uint8_t value = file.byte ( offset_at + b );

							if ( b )
								bytes.add ( L' ' );

							bytes.hex ( value, 2 );

							if ( b )
								operands.add ( L", " );

							ida_number ( operands, value );
							ascii [ b ] = printable ( value );

						}

						ascii [ mapped ? take : 0 ] = 0;

						if ( item.kind == line_kind::CODE ) {

							if ( show_bytes )
								c.text ( bytes.text ( ), render::font::MONO, math::vec2 ( x_bytes, cy ), ink ( tone::BYTES ) );

							c.text ( L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::INK_FAINT ) );

							if ( !show_bytes )
								c.text ( operands.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::INK_FAINT ), render::align::LEFT, ops_room );

						} else {

							base::writer note;

							note.add ( L"; " ).add ( ascii );

							if ( show_bytes )
								c.text ( bytes.text ( ), render::font::MONO, math::vec2 ( x_bytes, cy ), ink ( tone::BYTES ) );

							c.text ( L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
							c.text ( operands.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::NUMBER ), render::align::LEFT, ops_room );
							c.text ( note.text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						}

						break;

					}

					case line_kind::STRING: {

						const image::found_string& found = file.strings [ static_cast < size_t > ( item.ref ) ];
						base::writer value;
						int32_t limit = base::max ( static_cast < int32_t > ( ( area.right ( ) - x_ops ) / adv ) - 6, 8 );
						int32_t shown_chars = base::min ( static_cast < int32_t > ( found.length ), base::min ( limit, 300 ) );

						value.add ( L'"' );

						for ( int32_t i = 0; i < shown_chars; i++ ) {

							wchar_t ch = static_cast < wchar_t > ( file.byte ( found.offset + static_cast < uint64_t > ( i ) * ( found.wide ? 2 : 1 ) ) );

							if ( ch == L'\t' )
								value.add ( L"\\t" );
							else
								value.add ( ch );

						}

						if ( shown_chars < static_cast < int32_t > ( found.length ) )
							value.add ( L'\x2026' );

						value.add ( L"\",0" );
						c.text ( found.wide ? L"du" : L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::STRING ), render::align::LEFT, area.right ( ) - x_ops - 12.0f );

						break;

					}

					case line_kind::POINTER: {

						const image::import& entry = file.imports [ static_cast < size_t > ( item.ref ) ];
						const image::module& from = file.modules [ static_cast < size_t > ( entry.owner ) ];
						base::writer name;
						base::writer note;

						if ( entry.by_ordinal || entry.name.empty ( ) )
							name.add ( L"Ordinal_" ).number ( entry.ordinal );
						else
							name.add ( entry.name.get ( ) );

						name.add ( file.wide ( ) ? L":qword" : L":dword" );
						note.add ( L"; " ).add ( from.name.get ( ) );
						c.text ( L"extrn", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( name.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::NAME ) );
						c.text ( note.text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						break;

					}

					case line_kind::EMPTY: {

						base::writer value;

						ida_number ( value, item.size );
						value.add ( L" dup(?)" );
						c.text ( L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::NUMBER ) );
						c.text ( L"; uninitialized", render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						break;

					}

					default:
						break;

				}

			}

		}

		c.pop_clip ( );
		draw_scrollbar ( make_id ( "listing_bar" ), area, listing_scroll, static_cast < float > ( total_rows ) + 3.0f, visible );

	}

	void app::draw_hex ( math::rect area ) {

		render::canvas& c = canvas;
		float row_h = 20.0f;
		float adv = c.measure ( L"0000000000", render::font::MONO ) * 0.1f;
		uint64_t id = make_id ( "hex" );
		widget& surface = state ( id );
		uint64_t rows = ( file.size + 15 ) / 16;
		math::rect header ( area.x, area.y, area.w, 26.0f );
		math::rect body ( area.x, area.y + 26.0f, area.w, area.h - 26.0f );
		float visible = body.h / row_h;

		c.fill ( area, 0.0f, ink ( tone::BASE ) );
		scroll_region ( id, body );
		hex_scroll.target -= take_wheel ( id ) * 3.0f;
		glide ( hex_scroll, static_cast < float > ( rows ) - visible + 2.0f );
		interact ( id, body, surface );

		float x_off = area.x + 14.0f;
		float x_hex = x_off + adv * 12.0f;
		float x_ascii = x_hex + adv * 50.0f;
		auto byte_x = [ & ] ( int32_t column ) {

			return x_hex + static_cast < float > ( column ) * adv * 3.0f + ( column >= 8 ? adv : 0.0f );

		};

		float offset = base::max ( hex_scroll.offset, 0.0f );
		uint64_t first = static_cast < uint64_t > ( offset );
		float shift = ( offset - static_cast < float > ( first ) ) * row_h;

		if ( active_id == id && mouse_down && file.size ) {

			float local_y = mouse.y - body.y + shift;
			uint64_t row = first + static_cast < uint64_t > ( base::max ( local_y / row_h, 0.0f ) );
			int32_t column = -1;

			for ( int32_t i = 0; i < 16; i++ ) {

				if ( mouse.x >= byte_x ( i ) - adv * 0.5f && mouse.x < byte_x ( i ) + adv * 2.5f )
					column = i;

				if ( mouse.x >= x_ascii + static_cast < float > ( i ) * adv && mouse.x < x_ascii + static_cast < float > ( i + 1 ) * adv )
					column = i;

			}

			if ( column >= 0 ) {

				uint64_t target = base::min ( row * 16 + static_cast < uint64_t > ( column ), file.size - 1 );

				if ( press_event ) {

					hex_cursor = target;

					if ( !key_shift )
						hex_anchor = target;

					hex_selecting = true;

				} else if ( hex_selecting )
					hex_cursor = target;

				uint64_t address = 0;

				if ( file.address_of ( hex_cursor, address ) )
					cursor_address = address;

			}

		}

		c.fill ( header, 0.0f, ink ( tone::PANEL ) );
		c.fill ( math::rect ( header.x, header.bottom ( ) - 1.0f, header.w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		c.text ( L"Offset", render::font::SMALL_STRONG, math::vec2 ( x_off, header.center ( ).y ), ink ( tone::INK_SOFT ) );

		for ( int32_t i = 0; i < 16; i++ ) {

			base::writer column;

			column.hex ( static_cast < uint64_t > ( i ), 2 );
			c.text ( column.text ( ), render::font::MONO_SMALL, math::vec2 ( byte_x ( i ), header.center ( ).y ), ink ( tone::INK_FAINT ) );

		}

		c.text ( L"ASCII", render::font::SMALL_STRONG, math::vec2 ( x_ascii, header.center ( ).y ), ink ( tone::INK_SOFT ) );

		uint64_t low = base::min ( hex_cursor, hex_anchor );
		uint64_t high = base::max ( hex_cursor, hex_anchor );
		int32_t count = static_cast < int32_t > ( visible ) + 2;

		c.push_clip ( body );

		for ( int32_t pass = 0; pass < 2; pass++ ) {

			for ( int32_t k = 0; k < count; k++ ) {

				uint64_t row = first + static_cast < uint64_t > ( k );

				if ( row >= rows )
					break;

				float y = body.y + static_cast < float > ( k ) * row_h - shift;
				float cy = y + row_h * 0.5f;
				uint64_t start = row * 16;

				if ( pass == 0 ) {

					for ( int32_t i = 0; i < 16 && start + static_cast < uint64_t > ( i ) < file.size; i++ ) {

						uint64_t at = start + static_cast < uint64_t > ( i );

						if ( at < low || at > high )
							continue;

						bool edge = at == hex_cursor;
						render::color tint = edge ? ink ( tone::SELECT_LINE ).fade ( 0.55f ) : ink ( tone::SELECT );

						c.fill ( math::rect ( byte_x ( i ) - adv * 0.5f, y + 1.0f, adv * 3.0f, row_h - 2.0f ), 2.0f, tint );
						c.fill ( math::rect ( x_ascii + static_cast < float > ( i ) * adv, y + 1.0f, adv, row_h - 2.0f ), 1.0f, tint );

					}

					for ( const image::section& part : file.sections ) {

						if ( part.raw && part.offset >= start && part.offset < start + 16 && part.type != image::kind::HEADER ) {

							int32_t column = static_cast < int32_t > ( part.offset - start );

							c.fill ( math::rect ( byte_x ( column ) - adv * 0.5f, y, 1.5f, row_h ), 0.0f, ink ( tone::ACCENT ) );

						}

					}

					continue;

				}

				base::writer position;
				wchar_t ascii [ 17 ];

				position.hex ( start, file.size > 0xFFFFFFFFull ? 12 : 8 );
				c.text ( position.text ( ), render::font::MONO, math::vec2 ( x_off, cy ), ink ( tone::ADDRESS ) );

				int32_t filled = 0;

				for ( int32_t i = 0; i < 16 && start + static_cast < uint64_t > ( i ) < file.size; i++ ) {

					uint8_t value = file.byte ( start + static_cast < uint64_t > ( i ) );
					base::writer pair;
					render::color tint = value == 0 ? ink ( tone::INK_FAINT ) : ( value >= 0x20 && value < 0x7F ? ink ( tone::INK ) : ( value == 0xFF ? ink ( tone::NUMBER ) : ink ( tone::INK_SOFT ) ) );

					pair.hex ( value, 2 );
					c.text ( pair.text ( ), render::font::MONO, math::vec2 ( byte_x ( i ), cy ), tint );
					ascii [ i ] = printable ( value );
					filled = i + 1;

				}

				ascii [ filled ] = 0;
				c.text ( ascii, render::font::MONO, math::vec2 ( x_ascii, cy ), ink ( tone::INK_SOFT ) );

				for ( const image::section& part : file.sections ) {

					if ( part.raw && part.offset >= start && part.offset < start + 16 && part.type != image::kind::HEADER )
						c.text ( part.name, render::font::SMALL_STRONG, math::vec2 ( area.right ( ) - 22.0f, cy ), ink ( tone::ACCENT ), render::align::RIGHT );

				}

			}

		}

		c.pop_clip ( );
		draw_scrollbar ( make_id ( "hex_bar" ), body, hex_scroll, static_cast < float > ( rows ) + 2.0f, visible );

	}

	struct header_row {

		int32_t kind = 1;
		const wchar_t* name = L"";
		uint64_t offset = 0;
		uint32_t size = 0;
		wchar_t value [ 48 ] = { };
		wchar_t meaning [ 160 ] = { };

	};

	static void copy_to ( wchar_t* target, int32_t room, const wchar_t* source ) {

		int32_t i = 0;

		for ( ; source && source [ i ] && i < room - 1; i++ )
			target [ i ] = source [ i ];

		target [ i ] = 0;

	}

	static void describe_time ( uint32_t stamp, base::writer& out ) {

		int64_t days = static_cast < int64_t > ( stamp ) / 86400;
		int64_t rest = static_cast < int64_t > ( stamp ) % 86400;
		int64_t z = days + 719468;
		int64_t era = z / 146097;
		int64_t doe = z - era * 146097;
		int64_t yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
		int64_t year = yoe + era * 400;
		int64_t doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
		int64_t mp = ( 5 * doy + 2 ) / 153;
		int64_t day = doy - ( 153 * mp + 2 ) / 5 + 1;
		int64_t month = mp < 10 ? mp + 3 : mp - 9;

		if ( month <= 2 )
			year++;

		out.number ( year ).add ( L'-' );

		if ( month < 10 )
			out.add ( L'0' );

		out.number ( month ).add ( L'-' );

		if ( day < 10 )
			out.add ( L'0' );

		out.number ( day ).add ( L' ' );

		if ( rest / 3600 < 10 )
			out.add ( L'0' );

		out.number ( rest / 3600 ).add ( L':' );

		if ( rest / 60 % 60 < 10 )
			out.add ( L'0' );

		out.number ( rest / 60 % 60 ).add ( L" UTC" );

	}

	static void describe_flags ( uint32_t value, const uint32_t* bits, const wchar_t* const* labels, int32_t count, base::writer& out ) {

		bool any = false;

		for ( int32_t i = 0; i < count; i++ ) {

			if ( !( value & bits [ i ] ) )
				continue;

			if ( any )
				out.add ( L", " );

			out.add ( labels [ i ] );
			any = true;

		}

		if ( !any )
			out.add ( L"none" );

	}

	void app::draw_headers ( math::rect area ) {

		render::canvas& c = canvas;

		c.fill ( area, 0.0f, ink ( tone::BASE ) );

		if ( file.type == image::format::RAW ) {

			draw_pending ( area, L"No executable headers", L"This file didn't parse as a PE image, so there's no header structure to lay out. The listing and hex views still show every byte.", icons::FORM );

			return;

		}

		static base::list < header_row > rows;
		bool plus = file.type == image::format::PE64;
		uint64_t dos = 0;
		uint64_t fh = file.pe_offset + 4;
		uint64_t oh = file.optional_offset;

		rows.clear ( );

		auto heading = [ & ] ( const wchar_t* title ) {

			header_row& row = rows.emplace ( );

			row.kind = 0;
			row.name = title;

		};

		auto field_row = [ & ] ( const wchar_t* name, uint64_t at, uint32_t size, const wchar_t* meaning ) {

			header_row& row = rows.emplace ( );
			base::writer value;
			uint64_t number = size == 1 ? file.byte ( at ) : ( size == 2 ? file.read16 ( at ) : ( size == 4 ? file.read32 ( at ) : file.read64 ( at ) ) );

			row.name = name;
			row.offset = at;
			row.size = size;
			value.hex ( number, static_cast < int32_t > ( size * 2 ) );
			copy_to ( row.value, 48, value.text ( ) );
			copy_to ( row.meaning, 160, meaning );

			return number;

		};

		heading ( L"DOS header" );
		field_row ( L"e_magic", dos, 2, L"\"MZ\" signature" );
		field_row ( L"e_cblp", dos + 2, 2, L"Bytes on last page" );
		field_row ( L"e_cp", dos + 4, 2, L"Pages in file" );
		field_row ( L"e_cparhdr", dos + 8, 2, L"Header size in paragraphs" );
		field_row ( L"e_sp", dos + 16, 2, L"Initial SP" );
		field_row ( L"e_ip", dos + 20, 2, L"Initial IP" );
		field_row ( L"e_lfarlc", dos + 24, 2, L"Relocation table offset" );
		field_row ( L"e_lfanew", dos + 60, 4, L"Offset of the PE header" );

		heading ( L"File header" );
		field_row ( L"Signature", file.pe_offset, 4, L"\"PE\\0\\0\"" );

		{

			base::writer meaning;

			meaning.add ( file.machine_name ( ) );
			field_row ( L"Machine", fh, 2, meaning.text ( ) );

		}

		field_row ( L"NumberOfSections", fh + 2, 2, L"" );

		{

			base::writer meaning;

			describe_time ( file.stamp, meaning );
			field_row ( L"TimeDateStamp", fh + 4, 4, meaning.text ( ) );

		}

		field_row ( L"PointerToSymbolTable", fh + 8, 4, L"" );
		field_row ( L"NumberOfSymbols", fh + 12, 4, L"" );
		field_row ( L"SizeOfOptionalHeader", fh + 16, 2, L"" );

		{

			const uint32_t bits [ ] = { 0x1, 0x2, 0x20, 0x100, 0x200, 0x1000, 0x2000 };
			const wchar_t* const labels [ ] = { L"RELOCS_STRIPPED", L"EXECUTABLE", L"LARGE_ADDRESS_AWARE", L"32BIT_MACHINE", L"DEBUG_STRIPPED", L"SYSTEM", L"DLL" };
			base::writer meaning;

			describe_flags ( file.characteristics, bits, labels, 7, meaning );
			field_row ( L"Characteristics", fh + 18, 2, meaning.text ( ) );

		}

		heading ( plus ? L"Optional header (PE32+)" : L"Optional header (PE32)" );
		field_row ( L"Magic", oh, 2, plus ? L"PE32+, 64-bit" : L"PE32, 32-bit" );
		field_row ( L"MajorLinkerVersion", oh + 2, 1, L"" );
		field_row ( L"MinorLinkerVersion", oh + 3, 1, L"" );
		field_row ( L"SizeOfCode", oh + 4, 4, L"" );
		field_row ( L"SizeOfInitializedData", oh + 8, 4, L"" );
		field_row ( L"SizeOfUninitializedData", oh + 12, 4, L"" );
		field_row ( L"AddressOfEntryPoint", oh + 16, 4, L"RVA of the first instruction" );
		field_row ( L"BaseOfCode", oh + 20, 4, L"" );

		if ( !plus )
			field_row ( L"BaseOfData", oh + 24, 4, L"" );

		field_row ( L"ImageBase", plus ? oh + 24 : oh + 28, plus ? 8 : 4, L"Preferred load address" );
		field_row ( L"SectionAlignment", oh + 32, 4, L"" );
		field_row ( L"FileAlignment", oh + 36, 4, L"" );
		field_row ( L"MajorOperatingSystemVersion", oh + 40, 2, L"" );
		field_row ( L"MajorSubsystemVersion", oh + 48, 2, L"" );
		field_row ( L"SizeOfImage", oh + 56, 4, L"" );
		field_row ( L"SizeOfHeaders", oh + 60, 4, L"" );
		field_row ( L"CheckSum", oh + 64, 4, L"" );

		{

			const wchar_t* names_list [ ] = { L"Unknown", L"Native", L"Windows GUI", L"Windows console", L"", L"OS/2 console", L"", L"POSIX console", L"Native Win9x", L"Windows CE GUI", L"EFI application", L"EFI boot driver", L"EFI runtime driver", L"EFI ROM", L"Xbox", L"", L"Boot application" };

			field_row ( L"Subsystem", oh + 68, 2, file.subsystem < 17 ? names_list [ file.subsystem ] : L"" );

		}

		{

			const uint32_t bits [ ] = { 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800, 0x1000, 0x4000, 0x8000 };
			const wchar_t* const labels [ ] = { L"HIGH_ENTROPY_VA", L"DYNAMIC_BASE", L"FORCE_INTEGRITY", L"NX_COMPAT", L"NO_ISOLATION", L"NO_SEH", L"NO_BIND", L"APPCONTAINER", L"GUARD_CF", L"TERMINAL_SERVER_AWARE" };
			base::writer meaning;

			describe_flags ( file.dll_flags, bits, labels, 10, meaning );
			field_row ( L"DllCharacteristics", oh + 70, 2, meaning.text ( ) );

		}

		field_row ( L"SizeOfStackReserve", oh + 72, plus ? 8 : 4, L"" );
		field_row ( L"SizeOfStackCommit", plus ? oh + 80 : oh + 76, plus ? 8 : 4, L"" );
		field_row ( L"SizeOfHeapReserve", plus ? oh + 88 : oh + 80, plus ? 8 : 4, L"" );
		field_row ( L"SizeOfHeapCommit", plus ? oh + 96 : oh + 84, plus ? 8 : 4, L"" );
		field_row ( L"NumberOfRvaAndSizes", plus ? oh + 108 : oh + 92, 4, L"" );

		heading ( L"Data directories" );

		for ( int32_t i = 0; i < file.directory_count; i++ ) {

			const image::directory& entry = file.directories [ i ];
			header_row& row = rows.emplace ( );
			base::writer value;
			base::writer meaning;
			int32_t owner = entry.address ? file.section_at ( file.base + entry.address ) : -1;

			row.name = image::directory_name ( i );
			row.offset = oh + ( plus ? 112 : 96 ) + static_cast < uint64_t > ( i ) * 8;
			row.size = 8;
			value.hex ( entry.address, 8 ).add ( L"  " ).hex ( entry.size, 8 );
			copy_to ( row.value, 48, value.text ( ) );

			if ( !entry.address )
				meaning.add ( L"empty" );
			else {

				meaning.grouped ( entry.size ).add ( L" bytes" );

				if ( owner >= 0 )
					meaning.add ( L" in " ).add ( file.sections [ static_cast < size_t > ( owner ) ].name );

			}

			copy_to ( row.meaning, 160, meaning.text ( ) );

		}

		heading ( L"Section table" );

		for ( int32_t i = 1; i < file.sections.count ( ); i++ ) {

			const image::section& part = file.sections [ static_cast < size_t > ( i ) ];
			header_row& row = rows.emplace ( );
			base::writer value;
			base::writer meaning;
			const uint32_t bits [ ] = { 0x20, 0x40, 0x80, 0x02000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000u };
			const wchar_t* const labels [ ] = { L"CODE", L"INITIALIZED", L"UNINITIALIZED", L"DISCARDABLE", L"SHARED", L"EXECUTE", L"READ", L"WRITE" };

			row.name = part.name;
			row.offset = oh + file.optional_size + static_cast < uint64_t > ( i - 1 ) * 40;
			row.size = 40;
			value.hex ( part.address - file.base, 8 ).add ( L"  " ).hex ( part.size, 8 );
			copy_to ( row.value, 48, value.text ( ) );
			meaning.add ( L"raw @" ).hex ( part.offset ).add ( L" +" ).hex ( part.raw ).add ( L"  \x00B7  " );
			describe_flags ( part.flags, bits, labels, 8, meaning );
			copy_to ( row.meaning, 160, meaning.text ( ) );

		}

		float row_h = 22.0f;
		float head_h = 38.0f;
		float content = 16.0f;

		for ( const header_row& row : rows )
			content += row.kind == 0 ? head_h : row_h;

		uint64_t id = make_id ( "headers" );

		scroll_region ( id, area );
		headers_scroll.target -= take_wheel ( id ) * 66.0f;
		glide ( headers_scroll, content - area.h + 20.0f );

		float x = area.x + 24.0f;
		float x_offset = x + 250.0f;
		float x_value = x_offset + 90.0f;
		float x_meaning = x_value + 160.0f;
		float y = area.y + 12.0f - headers_scroll.offset;

		c.push_clip ( area );

		for ( int32_t pass = 0; pass < 2; pass++ ) {

			float cursor = y;
			int32_t stripe = 0;

			for ( const header_row& row : rows ) {

				float row_height = row.kind == 0 ? head_h : row_h;

				if ( cursor + row_height < area.y || cursor > area.bottom ( ) ) {

					cursor += row_height;
					stripe = row.kind == 0 ? 0 : stripe + 1;

					continue;

				}

				if ( row.kind == 0 ) {

					if ( pass == 1 ) {

						c.text ( row.name, render::font::HEADING, math::vec2 ( x, cursor + 22.0f ), ink ( tone::INK ) );
						c.text ( L"OFFSET", render::font::TINY, math::vec2 ( x_offset, cursor + 23.0f ), ink ( tone::INK_FAINT ) );
						c.text ( L"VALUE", render::font::TINY, math::vec2 ( x_value, cursor + 23.0f ), ink ( tone::INK_FAINT ) );
						c.text ( L"MEANING", render::font::TINY, math::vec2 ( x_meaning, cursor + 23.0f ), ink ( tone::INK_FAINT ) );

					} else
						c.fill ( math::rect ( x, cursor + row_height - 2.0f, area.w - 48.0f, 1.0f ), 0.0f, ink ( tone::LINE ) );

					cursor += row_height;
					stripe = 0;

					continue;

				}

				float cy = cursor + row_h * 0.5f;

				if ( pass == 0 ) {

					if ( stripe % 2 == 1 )
						c.fill ( math::rect ( x - 8.0f, cursor, area.w - 32.0f, row_h ), 2.0f, ink ( tone::HOVER ).fade ( 0.6f ) );

				} else {

					base::writer position;

					position.hex ( row.offset, 4 );
					c.text ( row.name, render::font::MONO, math::vec2 ( x, cy ), ink ( tone::INK ) );
					c.text ( position.text ( ), render::font::MONO_SMALL, math::vec2 ( x_offset, cy ), ink ( tone::ADDRESS ) );
					c.text ( row.value, render::font::MONO, math::vec2 ( x_value, cy ), ink ( tone::NUMBER ) );
					c.text ( row.meaning, render::font::SMALL, math::vec2 ( x_meaning, cy ), ink ( tone::INK_SOFT ), render::align::LEFT, area.right ( ) - x_meaning - 20.0f );

				}

				cursor += row_h;
				stripe++;

			}

		}

		c.pop_clip ( );
		draw_scrollbar ( make_id ( "headers_bar" ), area, headers_scroll, content + 20.0f, area.h );

	}

}