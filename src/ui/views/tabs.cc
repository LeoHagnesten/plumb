#include "ui/app.hh"
#include "ui/icons.hh"
#include "ui/format.hh"

namespace ui {

	static const wchar_t* view_titles [ static_cast < int32_t > ( view::COUNT ) ] = { L"Disassembly", L"Graph", L"C", L"Hex", L"Headers" };
	static const uint32_t view_glyphs [ static_cast < int32_t > ( view::COUNT ) ] = { icons::CODE, icons::BRANCH, icons::FORM, icons::CHIP, icons::INFO };

	void app::draw_panes ( ) {

		render::canvas& c = canvas;

		for ( int32_t i = 0; i < pane_count; i++ ) {

			if ( panes [ i ].area.w < 4.0f || panes [ i ].area.h < 4.0f )
				continue;

			draw_document ( i );
			draw_tabs ( i );

		}

		if ( pane_count == 2 ) {

			math::rect seam ( panes [ 1 ].area.x - 1.0f, view_area.y, 1.0f, view_area.h );

			c.fill ( seam, 0.0f, ink ( tone::LINE_STRONG ) );

			if ( splitter ( make_id ( "split_panes" ), math::rect ( seam.x - 4.0f, view_area.y, 9.0f, view_area.h ), true ) )
				pane_split = ( mouse.x - view_area.x ) / base::max ( view_area.w, 1.0f );

		}

		select_state ( current_document ( ) );

	}

	void app::draw_tabs ( int32_t index ) {

		render::canvas& c = canvas;
		pane& item = panes [ index ];
		math::rect area = item.strip;
		bool focused_here = index == focused_pane;

		c.fill ( area, 0.0f, ink ( tone::PANEL ) );
		c.fill ( math::rect ( area.x, area.bottom ( ) - 1.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );
		c.push_clip ( area );

		float x = area.x + 6.0f;
		float active_x = 0.0f;
		float active_w = 0.0f;
		float tools = 150.0f;
		float room = base::max ( area.w - tools - 40.0f, 80.0f );
		float total = 0.0f;
		wchar_t label [ 200 ];
		int32_t close_tab = -1;

		auto tab_label = [ & ] ( const document& doc ) {

			base::writer text;

			text.add ( view_titles [ static_cast < int32_t > ( doc.kind ) ] );

			if ( ( doc.kind == view::PSEUDOCODE || doc.kind == view::GRAPH ) && db && analysis_applied ) {

				int32_t owner = db->function_starting ( doc.kind == view::GRAPH ? doc.state.graph_function : doc.state.function );

				if ( owner >= 0 ) {

					wchar_t name [ 160 ];

					function_name ( owner, name, 160 );
					text.add ( L" \x00B7 " ).add ( name );

				}

			}

			int32_t i = 0;

			for ( ; i <= text.size ( ) && i < 199; i++ )
				label [ i ] = text.text ( ) [ i ];

			label [ 199 ] = 0;

		};

		for ( int32_t t = 0; t < item.count; t++ ) {

			tab_label ( documents [ item.tabs [ t ] ] );
			total += base::min ( c.measure ( label, render::font::UI ), 220.0f ) + 56.0f;

		}

		float squeeze = total > room ? room / total : 1.0f;
		float scan = x;

		for ( int32_t t = 0; t < item.count; t++ ) {

			tab_label ( documents [ item.tabs [ t ] ] );

			float slot_w = base::max ( ( base::min ( c.measure ( label, render::font::UI ), 220.0f ) + 56.0f ) * squeeze, 64.0f );

			if ( item.active == t ) {

				active_x = scan;
				active_w = slot_w;

			}

			scan += slot_w;

		}

		if ( !item.tab_ready ) {

			item.tab_x = active_x;
			item.tab_w = active_w;
			item.tab_ready = true;

		}

		spring ( item.tab_x, item.tab_vx, active_x, 460.0f, 42.9f );
		spring ( item.tab_w, item.tab_vw, active_w, 460.0f, 42.9f );

		if ( item.count )
			c.fill ( math::rect ( item.tab_x + 2.0f, area.y + 4.0f, base::max ( item.tab_w - 4.0f, 0.0f ), area.h - 9.0f ), 4.0f, ink ( tone::BASE ).fade ( 0.7f ) );

		for ( int32_t t = 0; t < item.count; t++ ) {

			const document& doc = documents [ item.tabs [ t ] ];

			tab_label ( doc );

			float text_w = base::min ( c.measure ( label, render::font::UI ), 220.0f );
			float slot_w = base::max ( ( text_w + 56.0f ) * squeeze, 64.0f );
			math::rect slot ( x, area.y, slot_w, area.h - 1.0f );
			uint64_t id = make_id ( "tab", static_cast < int64_t > ( doc.serial ) );
			widget& state_item = state ( id );
			bool selected = item.active == t;

			if ( interact ( id, slot, state_item ) )
				focus_document ( index, t );

			if ( middle_id == id )
				close_tab = t;

			bool inside = over ( slot ) && menu_open < 0;
			widget& lit = state ( make_id ( "tab_lit", static_cast < int64_t > ( doc.serial ) ) );

			approach ( state_item.value, inside ? 1.0f : 0.0f, 22.0f );
			approach ( lit.value, selected ? 1.0f : 0.0f, 20.0f );

			float lift = state_item.value;
			float on = lit.value;

			if ( !selected )
				c.fill ( math::rect ( slot.x + 2.0f, slot.y + 4.0f, slot.w - 4.0f, slot.h - 8.0f ), 4.0f, ink ( tone::HOVER ).fade ( lift * 2.4f ) );

			c.icon ( view_glyphs [ static_cast < int32_t > ( doc.kind ) ], math::vec2 ( slot.x + 16.0f, slot.center ( ).y ), 12.0f, render::mix ( render::mix ( ink ( tone::INK_FAINT ), ink ( tone::INK_SOFT ), lift ), ink ( tone::ACCENT ), on ) );
			c.text ( label, render::font::UI, math::vec2 ( slot.x + 30.0f, slot.center ( ).y ), render::mix ( render::mix ( ink ( tone::INK_SOFT ), ink ( tone::INK ), lift ), ink ( tone::INK ), on ), render::align::LEFT, slot.w - 54.0f );

			float reveal = selected ? base::max ( 0.6f, lift ) : lift;

			if ( reveal > 0.02f && ( item.count > 1 || pane_count > 1 ) ) {

				c.push_alpha ( reveal );

				if ( icon_button ( make_id ( "tab_close", static_cast < int64_t > ( doc.serial ) ), math::vec2 ( slot.right ( ) - 14.0f, slot.center ( ).y ), 20.0f, icons::CANCEL, 9.0f, L"Close tab (Ctrl+W)" ) )
					close_tab = t;

				c.pop ( );

			}

			x = slot.right ( );

		}

		if ( icon_button ( make_id ( "tab_add", index ), math::vec2 ( x + 16.0f, area.center ( ).y ), 26.0f, icons::ADD, 11.0f, L"New disassembly tab (Ctrl+1)" ) ) {

			focused_pane = index;
			select_state ( pane_document ( index ) );
			run ( command::VIEW_LISTING );

		}

		c.fill ( math::rect ( item.tab_x + 8.0f, area.bottom ( ) - 2.5f, base::max ( item.tab_w - 16.0f, 0.0f ), 2.0f ), 1.0f, focused_here ? ink ( tone::ACCENT ) : ink ( tone::LINE_STRONG ) );

		float right = area.right ( ) - 10.0f;

		if ( right - x > 140.0f ) {

			if ( icon_button ( make_id ( "tab_split", index ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::LAYERS, 13.0f, pane_count > 1 ? L"Join panes (Ctrl+\\)" : L"Split side by side (Ctrl+\\)" ) ) {

				focused_pane = index;
				run ( command::SPLIT );

			}

			right -= 32.0f;

			if ( icon_button ( make_id ( "tab_forward", index ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::FORWARD, 13.0f, L"Forward (Alt+Right)" ) ) {

				focus_document ( index, item.active );
				run ( command::FORWARD );

			}

			right -= 30.0f;

			if ( icon_button ( make_id ( "tab_back", index ), math::vec2 ( right - 14.0f, area.center ( ).y ), 28.0f, icons::BACK, 13.0f, L"Back (Esc)" ) ) {

				focus_document ( index, item.active );
				run ( command::BACK );

			}

			right -= 36.0f;

			int32_t doc = pane_document ( index );

			if ( doc >= 0 && right - x > 230.0f ) {

				base::writer where;
				uint64_t address = documents [ doc ].kind == view::PSEUDOCODE ? documents [ doc ].state.function : documents [ doc ].state.cursor_address;

				where.hex ( address, file.wide ( ) ? 9 : 8 );
				c.text ( where.text ( ), render::font::MONO_SMALL, math::vec2 ( right, area.center ( ).y ), ink ( tone::INK_SOFT ), render::align::RIGHT );

			}

		}

		c.pop_clip ( );

		if ( close_tab >= 0 )
			close_document ( index, close_tab );

	}

	void app::draw_document ( int32_t index ) {

		render::canvas& c = canvas;
		pane& item = panes [ index ];
		int32_t doc = pane_document ( index );

		if ( doc < 0 ) {

			c.fill ( item.body, 0.0f, ink ( tone::BASE ) );

			return;

		}

		if ( press_event && over ( item.area ) && focused_pane != index && menu_open < 0 ) {

			focused_pane = index;

		}

		document& current = documents [ doc ];

		select_state ( doc );
		drawing_serial = current.serial;
		approach ( current.fade, 1.0f, 18.0f );

		float fade = math::saturate ( current.fade );

		c.push ( math::vec2 ( item.body.x, item.body.y ), 1.0f, 0.35f + 0.65f * fade, math::vec2 ( 0.0f, 3.0f * ( 1.0f - fade ) ) );

		switch ( current.kind ) {

			case view::PSEUDOCODE:
				draw_pseudocode ( item.body );
				break;

			case view::GRAPH:
				draw_graph ( item.body );
				break;

			case view::HEX:
				draw_hex ( item.body );
				break;

			case view::HEADERS:
				draw_headers ( item.body );
				break;

			default:
				draw_listing ( item.body );
				break;

		}

		c.pop ( );

		if ( pane_count > 1 && index != focused_pane )
			c.fill ( item.body, 0.0f, ink ( tone::BASE ).fade ( 0.12f ) );

	}

}