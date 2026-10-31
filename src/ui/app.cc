#include "ui/app.hh"
#include "ui/icons.hh"
#include "base/files.hh"
#include <windows.h>

namespace ui {

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
		0xE2B65B, 0x7FB0E8, 0xB5CEA8, 0xD69D85, 0x8CD1C8, 0x6A9955, 0x4F7DBF, 0x8C7A52, 0x6E6A90, 0x3A3F47,
		0x5A6170, 0xE5675B, 0x7DBE7A, 0xE3B341, 0x569CD6, 0xD8A0DF, 0x4EC9B0, 0xDCDCAA, 0x9CDCFE, 0x7FD1E6,
		0xB8D7A3, 0xB4B4B4, 0xA9C7E8, 0xE5C07B, 0xD4A24C
	};

	static const uint32_t light_tones [ static_cast < int32_t > ( tone::COUNT ) ] = {
		0xFAF9F6, 0xF1EFEA, 0xFFFFFF, 0xE9E6E0, 0xE6E3DD, 0xDCD8D0, 0xC5C0B6, 0x1E2126, 0x5B616B, 0x99A0A8,
		0xA86F1C, 0xA86F1C, 0xFFFFFF, 0xDCE6F4, 0x4A78B8, 0x000000, 0x000000, 0x000000, 0x6D7580, 0x8D939B,
		0x96620E, 0x285CA6, 0x098658, 0xA31515, 0x1D776D, 0x2E7D32, 0x6F95CC, 0xB8A57A, 0x9C97B5, 0xD6D2CA,
		0x9AA0A8, 0xC8473C, 0x3F8F3C, 0xB7860F, 0x0F54D6, 0x8F08C4, 0x2B91AF, 0x74531F, 0x1F377F, 0x1F6F8F,
		0x5E6B12, 0x5A5A5A, 0x2F4F7F, 0x8A5A00, 0xA86F1C
	};

	static const float tone_alpha [ static_cast < int32_t > ( tone::COUNT ) ] [ 2 ] = {
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 0.16f, 0.14f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.045f, 0.045f }, { 0.085f, 0.08f }, { 0.5f, 0.16f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f },
		{ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.32f, 0.32f }
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
		x86::prepare ( );
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

		wait_analysis ( );
		save_project ( );
		files::settle ( );
		file.close ( );

		if ( db ) {

			delete db;
			db = nullptr;

		}

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

		return animating || pending > 0 || ( analysis_thread && !analysis_done );

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

		if ( button == 2 ) {

			if ( down )
				middle_event = true;

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
		halt_glide = press_event && over ( area );

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

	void app::glide ( scroller& item, float limit, float unit ) {

		float pixels = base::max ( unit * dpi, 0.001f );

		if ( halt_glide ) {

			item.target = item.offset;
			item.velocity = 0.0f;

		}

		item.limit = base::max ( limit, 0.0f );
		item.target = base::clamp ( item.target, 0.0f, item.limit );

		float gap = item.target - item.offset;

		if ( math::abs ( gap ) * pixels < 0.25f && math::abs ( item.velocity ) * pixels < 2.0f ) {

			item.offset = item.target;
			item.velocity = 0.0f;
			item.shown = math::round ( item.offset * pixels ) / pixels;

			return;

		}

		spring ( item.offset, item.velocity, item.target, 300.0f, 35.0f );
		item.offset = base::clamp ( item.offset, -40.0f, item.limit + 40.0f );
		item.shown = math::round ( item.offset * pixels ) / pixels;
		animating = true;

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
		middle_id = 0;

		if ( press_event ) {

			active_id = hot;

			if ( double_event )
				double_id = hot;

		}

		if ( right_event )
			right_id = hot;

		if ( middle_event )
			middle_id = hot;

		if ( release_event && active_id && active_id == hot )
			clicked_id = active_id;

	}

	void app::end_input ( ) {

		if ( release_event ) {

			active_id = 0;
			dragging_split = 0;
			now->hex_selecting = false;

			for ( document& item : documents )
				item.state.hex_selecting = false;

		}

		press_event = false;
		release_event = false;
		right_event = false;
		double_event = false;
		middle_event = false;
		wheel_steps = 0.0f;
		wheel_side = 0.0f;
		halt_glide = false;
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
		view_area = math::rect ( center_area.x, top, center_area.w, output_area.y - top );

		float strip = has ? 34.0f : 0.0f;

		if ( pane_count == 2 ) {

			pane_split = base::clamp ( pane_split, 0.2f, 0.8f );

			float left = base::max ( view_area.w * pane_split, 200.0f );

			panes [ 0 ].area = math::rect ( view_area.x, view_area.y, left - 1.0f, view_area.h );
			panes [ 1 ].area = math::rect ( view_area.x + left, view_area.y, view_area.w - left, view_area.h );

		} else
			panes [ 0 ].area = view_area;

		for ( int32_t i = 0; i < pane_count; i++ ) {

			pane& item = panes [ i ];

			item.strip = math::rect ( item.area.x, item.area.y, item.area.w, strip );
			item.body = math::rect ( item.area.x, item.area.y + strip, item.area.w, item.area.h - strip );

		}

	}

	void app::frame ( double seconds ) {

		begin_frame ( seconds );
		begin_input ( );

		if ( analysis_done && !analysis_applied )
			analysis_finished ( );

		select_state ( current_document ( ) );
		handle_keys ( );
		select_state ( current_document ( ) );

		if ( browser_open && browsing == panel::REFERENCES && file.loaded ( ) )
			refresh_references ( );
		update_theme ( );
		layout ( );
		canvas.begin ( gpu, dpi, math::vec2 ( 0.0f, 0.0f ), static_cast < float > ( clock ) );
		canvas.fill ( math::rect ( 0.0f, 0.0f, w, h ), 0.0f, ink ( tone::BASE ) );

		if ( welcome_show > 0.01f )
			draw_welcome ( math::rect ( 0.0f, title_area.bottom ( ), w, status_area.y - title_area.bottom ( ) ) );

		if ( file.loaded ( ) ) {

			canvas.push_alpha ( 1.0f - welcome_show );
			draw_band ( );
			draw_panes ( );
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
		draw_prompt ( );
		draw_signature ( );
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