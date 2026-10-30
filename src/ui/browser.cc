#include "ui/app.hh"
#include "ui/icons.hh"

namespace ui {

	static const uint32_t panel_glyphs [ static_cast < int32_t > ( panel::COUNT ) ] = { icons::CODE, icons::IMPORT, icons::EXPORT, icons::FONT, icons::LAYERS };
	static const wchar_t* panel_titles [ static_cast < int32_t > ( panel::COUNT ) ] = { L"Functions", L"Imports", L"Exports", L"Strings", L"Sections" };
	static const wchar_t* panel_hints [ static_cast < int32_t > ( panel::COUNT ) ] = { L"Filter functions", L"Filter imports or modules", L"Filter exports", L"Search strings", L"Filter sections" };

	void app::draw_rail ( ) {

		if ( rail_area.w < 1.0f )
			return;

		render::canvas& c = canvas;
		math::rect area = rail_area;

		c.fill ( area, 0.0f, ink ( tone::TITLE ) );
		c.fill ( math::rect ( area.right ( ) - 1.0f, area.y, 1.0f, area.h ), 0.0f, ink ( tone::LINE ) );

		float slot = 44.0f;
		float top = area.y + 8.0f;
		float target = static_cast < float > ( static_cast < int32_t > ( browsing ) );

		spring ( rail_marker, rail_velocity, target, 520.0f, 40.0f );

		if ( browser_open )
			c.fill ( math::rect ( area.x, top + rail_marker * slot + 10.0f, 2.5f, slot - 20.0f ), 1.0f, ink ( tone::ACCENT ) );

		for ( int32_t i = 0; i < static_cast < int32_t > ( panel::COUNT ); i++ ) {

			math::vec2 center ( area.x + area.w * 0.5f, top + static_cast < float > ( i ) * slot + slot * 0.5f );
			uint64_t id = make_id ( "rail", i );
			widget& item = state ( id );
			math::rect hit_area ( area.x, center.y - slot * 0.5f, area.w, slot );
			bool selected = browser_open && browsing == static_cast < panel > ( i );

			if ( interact ( id, hit_area, item ) ) {

				if ( selected )
					browser_open = false;
				else {

					browsing = static_cast < panel > ( i );
					browser_open = true;

				}

			}

			c.fill ( math::rect ( center.x - 16.0f, center.y - 16.0f, 32.0f, 32.0f ), 4.0f, ink ( tone::HOVER ).fade ( item.hover * 1.4f ) );
			c.icon ( panel_glyphs [ i ], center, 16.0f, selected ? ink ( tone::INK ) : render::mix ( ink ( tone::INK_FAINT ), ink ( tone::INK_SOFT ), item.hover ) );
			tip ( id, panel_titles [ i ], hit_area );

		}

		float bottom = area.bottom ( ) - 8.0f;

		if ( icon_button ( make_id ( "rail_theme" ), math::vec2 ( area.x + area.w * 0.5f, bottom - 22.0f ), 32.0f, light ? icons::MOON : icons::SUN, 15.0f, light ? L"Dark theme" : L"Light theme" ) )
			run ( command::THEME );

		if ( icon_button ( make_id ( "rail_palette" ), math::vec2 ( area.x + area.w * 0.5f, bottom - 62.0f ), 32.0f, icons::MENU, 15.0f, L"Command palette" ) )
			run ( command::PALETTE );

	}

	void app::draw_browser ( ) {

		if ( browser_area.w < 2.0f )
			return;

		render::canvas& c = canvas;
		math::rect area = browser_area;
		int32_t slot = static_cast < int32_t > ( browsing );
		float inner_w = browser_fit;

		c.push_clip ( area );
		c.fill ( area, 0.0f, ink ( tone::PANEL ) );

		float x = area.right ( ) - inner_w;
		base::writer count;

		if ( filters [ slot ].revision != filter_seen [ slot ] )
			refresh_filter ( browsing );

		int32_t total = 0;

		switch ( browsing ) {

			case panel::FUNCTIONS:
				total = file.functions.count ( );
				break;

			case panel::IMPORTS:
				total = file.imports.count ( );
				break;

			case panel::EXPORTS:
				total = file.exports.count ( );
				break;

			case panel::STRINGS:
				total = file.strings.count ( );
				break;

			default:
				total = file.sections.count ( );
				break;

		}

		count.grouped ( static_cast < uint64_t > ( total ) );

		math::rect header ( x + 14.0f, area.y + 10.0f, inner_w - 28.0f, 20.0f );

		c.text ( panel_titles [ slot ], render::font::HEADING, math::vec2 ( header.x, header.center ( ).y ), ink ( tone::INK ) );
		c.text ( count.text ( ), render::font::SMALL, math::vec2 ( header.right ( ), header.center ( ).y ), ink ( tone::INK_FAINT ), render::align::RIGHT );
		text_field ( make_id ( "filter", slot ), filters [ slot ], math::rect ( x + 12.0f, area.y + 38.0f, inner_w - 24.0f, 30.0f ), panel_hints [ slot ], icons::SEARCH, render::font::UI );

		math::rect list ( x, area.y + 78.0f, inner_w, area.h - 78.0f );

		c.fill ( math::rect ( x, list.y - 1.0f, inner_w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		draw_browser_rows ( browsing, list );
		c.pop_clip ( );
		c.fill ( math::rect ( area.right ( ) - 1.0f, area.y, 1.0f, area.h ), 0.0f, ink ( tone::LINE ) );

		if ( browser_show > 0.98f && splitter ( make_id ( "split_browser" ), math::rect ( area.right ( ) - 4.0f, area.y, 8.0f, area.h ), true ) )
			browser_width = base::clamp ( mouse.x - area.x, 200.0f, base::max ( w * 0.4f, 200.0f ) );

	}

	void app::draw_browser_rows ( panel which, math::rect area ) {

		render::canvas& c = canvas;
		int32_t slot = static_cast < int32_t > ( which );
		base::list < int32_t >& list = shown [ slot ];
		scroller& scroll = browser_scroll [ slot ];
		float row_h = 26.0f;
		float content = static_cast < float > ( list.count ( ) ) * row_h + 8.0f;
		uint64_t list_id = make_id ( "browser_list", slot );

		scroll_region ( list_id, area );
		scroll.target -= take_wheel ( list_id ) * row_h * 3.0f;
		glide ( scroll, content - area.h );

		if ( list.empty ( ) ) {

			c.text ( filters [ slot ].length ? L"No matches." : L"Nothing here in this image.", render::font::UI, math::vec2 ( area.x + 16.0f, area.y + 24.0f ), ink ( tone::INK_FAINT ) );

			return;

		}

		int32_t first = base::max ( static_cast < int32_t > ( scroll.offset / row_h ), 0 );
		int32_t last = base::min ( list.count ( ), first + static_cast < int32_t > ( area.h / row_h ) + 2 );
		bool wide = file.wide ( );

		c.push_clip ( area );

		for ( int32_t pass = 0; pass < 2; pass++ ) {

			for ( int32_t i = first; i < last; i++ ) {

				int32_t value = list [ static_cast < size_t > ( i ) ];
				math::rect row ( area.x + 6.0f, area.y + 4.0f + static_cast < float > ( i ) * row_h - scroll.offset, area.w - 12.0f, row_h );
				uint64_t id = make_id ( "browser_row", static_cast < int64_t > ( slot ) * 0x100000000ll + i );
				bool header = which == panel::IMPORTS && value < 0;
				float cy = row.center ( ).y;

				if ( pass == 0 ) {

					widget& item = state ( id );
					bool clicked = interact ( id, row, item );

					if ( header ) {

						c.fill ( math::rect ( area.x, row.y, area.w, row.h ), 0.0f, ink ( tone::BASE ).fade ( 0.55f ) );

						continue;

					}

					if ( browser_pick [ slot ] == i )
						c.fill ( row, 3.0f, ink ( tone::SELECT ) );

					c.fill ( row, 3.0f, ink ( tone::HOVER ).fade ( item.hover ) );

					if ( clicked ) {

						browser_pick [ slot ] = i;

						switch ( which ) {

							case panel::FUNCTIONS:
								go ( file.functions [ static_cast < size_t > ( value ) ].address, true );
								break;

							case panel::IMPORTS:
								go ( file.imports [ static_cast < size_t > ( value ) ].slot, true );
								break;

							case panel::EXPORTS:

								if ( file.exports [ static_cast < size_t > ( value ) ].address )
									go ( file.exports [ static_cast < size_t > ( value ) ].address, true );

								break;

							case panel::STRINGS: {

								uint64_t address = 0;
								uint64_t offset = file.strings [ static_cast < size_t > ( value ) ].offset;

								if ( file.address_of ( offset, address ) && showing != view::HEX )
									go ( address, true );
								else {

									showing = view::HEX;
									go_offset ( offset );

								}

								break;

							}

							default:
								go ( file.sections [ static_cast < size_t > ( value ) ].address, true );
								break;

						}

					}

					continue;

				}

				base::writer where;

				switch ( which ) {

					case panel::FUNCTIONS: {

						const image::symbol& item = file.functions [ static_cast < size_t > ( value ) ];
						render::color tag = item.type == image::symbol_kind::ENTRY ? ink ( tone::ACCENT ) : ( item.type == image::symbol_kind::TLS ? ink ( tone::WARNING ) : ink ( tone::NAME ) );

						c.fill ( math::rect ( row.x + 8.0f, cy - 3.0f, 6.0f, 6.0f ), 1.0f, tag );
						where.hex ( item.address, wide ? 16 : 8 );
						c.text ( item.name.get ( ), render::font::UI, math::vec2 ( row.x + 22.0f, cy ), ink ( tone::INK ), render::align::LEFT, row.w - 150.0f );
						c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), ink ( tone::ADDRESS ), render::align::RIGHT );

						break;

					}

					case panel::IMPORTS: {

						if ( header ) {

							const image::module& owner = file.modules [ static_cast < size_t > ( -value - 1 ) ];

							where.number ( owner.count );
							c.icon ( icons::LIBRARY, math::vec2 ( row.x + 11.0f, cy ), 12.0f, ink ( tone::INK_SOFT ) );
							c.text ( owner.name.get ( ), render::font::UI_STRONG, math::vec2 ( row.x + 26.0f, cy ), ink ( tone::INK ), render::align::LEFT, row.w - 70.0f );
							c.text ( where.text ( ), render::font::SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), ink ( tone::INK_FAINT ), render::align::RIGHT );

							break;

						}

						const image::import& item = file.imports [ static_cast < size_t > ( value ) ];
						base::writer label;

						if ( item.by_ordinal || item.name.empty ( ) )
							label.add ( L"#" ).number ( item.ordinal );
						else
							label.add ( item.name.get ( ) );

						where.hex ( item.slot, wide ? 16 : 8 );
						c.text ( label.text ( ), render::font::UI, math::vec2 ( row.x + 26.0f, cy ), item.by_ordinal ? ink ( tone::INK_SOFT ) : ink ( tone::INK ), render::align::LEFT, row.w - 160.0f );
						c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), ink ( tone::ADDRESS ), render::align::RIGHT );

						break;

					}

					case panel::EXPORTS: {

						const image::exported& item = file.exports [ static_cast < size_t > ( value ) ];
						base::writer label;

						if ( item.name.empty ( ) )
							label.add ( L"#" ).number ( item.ordinal );
						else
							label.add ( item.name.get ( ) );

						if ( item.forward.empty ( ) )
							where.hex ( item.address, wide ? 16 : 8 );
						else
							where.add ( L"\x2192 " ).add ( item.forward.get ( ) );

						c.text ( label.text ( ), render::font::UI, math::vec2 ( row.x + 10.0f, cy ), ink ( tone::INK ), render::align::LEFT, row.w - 160.0f );
						c.text ( where.text ( ), item.forward.empty ( ) ? render::font::MONO_SMALL : render::font::SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), item.forward.empty ( ) ? ink ( tone::ADDRESS ) : ink ( tone::INK_SOFT ), render::align::RIGHT, 140.0f );

						break;

					}

					case panel::STRINGS: {

						const image::found_string& item = file.strings [ static_cast < size_t > ( value ) ];
						wchar_t preview [ 120 ];
						int32_t length = base::min ( static_cast < int32_t > ( item.length ), 119 );
						uint64_t address = 0;

						for ( int32_t k = 0; k < length; k++ )
							preview [ k ] = static_cast < wchar_t > ( file.byte ( item.offset + static_cast < uint64_t > ( k ) * ( item.wide ? 2 : 1 ) ) );

						preview [ length ] = 0;

						if ( file.address_of ( item.offset, address ) )
							where.hex ( address, wide ? 16 : 8 );
						else
							where.add ( L"@" ).hex ( item.offset );

						c.text ( item.wide ? L"W" : L"A", render::font::TINY, math::vec2 ( row.x + 8.0f, cy ), item.wide ? ink ( tone::NAME ) : ink ( tone::INK_FAINT ) );
						c.text ( preview, render::font::MONO_SMALL, math::vec2 ( row.x + 24.0f, cy ), ink ( tone::INK ), render::align::LEFT, row.w - 154.0f );
						c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), ink ( tone::ADDRESS ), render::align::RIGHT );

						break;

					}

					default: {

						const image::section& item = file.sections [ static_cast < size_t > ( value ) ];
						render::color chip = item.type == image::kind::CODE ? ink ( tone::CODE_BAND ) : ( item.type == image::kind::CONSTANT ? ink ( tone::CONST_BAND ) : ( item.type == image::kind::EMPTY ? ink ( tone::EMPTY_BAND ) : ( item.type == image::kind::HEADER ? ink ( tone::HEADER_BAND ) : ink ( tone::DATA_BAND ) ) ) );
						wchar_t perms [ 4 ] = { item.read ? L'R' : L'\x00B7', item.write ? L'W' : L'\x00B7', item.execute ? L'X' : L'\x00B7', 0 };

						c.fill ( math::rect ( row.x + 8.0f, cy - 6.0f, 3.0f, 12.0f ), 1.0f, chip );
						c.text ( item.name, render::font::MONO_STRONG, math::vec2 ( row.x + 20.0f, cy ), ink ( tone::INK ) );
						c.text ( perms, render::font::MONO_SMALL, math::vec2 ( row.x + 104.0f, cy ), ink ( tone::INK_SOFT ) );
						where.hex ( item.address, wide ? 16 : 8 );
						c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( row.right ( ) - 8.0f, cy ), ink ( tone::ADDRESS ), render::align::RIGHT );

						break;

					}

				}

			}

		}

		c.pop_clip ( );
		draw_scrollbar ( make_id ( "browser_bar", slot ), area, scroll, content, area.h );

	}

}