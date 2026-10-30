#pragma once

#include "render/render.hh"
#include "image/image.hh"

namespace ui {

	enum class hit : int32_t {

		NONE,
		CLIENT,
		CAPTION,
		MAXIMIZE,

	};

	enum class request : int32_t {

		NONE,
		MINIMIZE,
		MAXIMIZE,
		CLOSE,
		OPEN,

	};

	enum class pointer : int32_t {

		ARROW,
		TEXT,
		HAND,
		SIZE_WE,
		SIZE_NS,

	};

	enum class view : int32_t {

		LISTING,
		HEX,
		HEADERS,
		GRAPH,
		PSEUDOCODE,
		COUNT,

	};

	enum class panel : int32_t {

		FUNCTIONS,
		IMPORTS,
		EXPORTS,
		STRINGS,
		SECTIONS,
		COUNT,

	};

	enum class tone : int32_t {

		BASE,
		PANEL,
		RAISED,
		SUNKEN,
		TITLE,
		LINE,
		LINE_STRONG,
		INK,
		INK_SOFT,
		INK_FAINT,
		ACCENT,
		ACCENT_SOFT,
		ACCENT_INK,
		SELECT,
		SELECT_LINE,
		HOVER,
		PRESS,
		SHADOW,
		ADDRESS,
		BYTES,
		LABEL,
		MNEMONIC,
		NUMBER,
		STRING,
		NAME,
		COMMENT,
		CODE_BAND,
		DATA_BAND,
		CONST_BAND,
		EMPTY_BAND,
		HEADER_BAND,
		DANGER,
		SUCCESS,
		WARNING,
		COUNT,

	};

	enum class command : int32_t {

		OPEN,
		CLOSE,
		EXIT,
		COPY,
		GO_ADDRESS,
		GO_ENTRY,
		BACK,
		FORWARD,
		NEXT_SECTION,
		PREVIOUS_SECTION,
		FIND,
		PALETTE,
		VIEW_LISTING,
		VIEW_HEX,
		VIEW_HEADERS,
		VIEW_GRAPH,
		VIEW_PSEUDOCODE,
		SHOW_FUNCTIONS,
		SHOW_IMPORTS,
		SHOW_EXPORTS,
		SHOW_STRINGS,
		SHOW_SECTIONS,
		TOGGLE_BROWSER,
		TOGGLE_INSPECTOR,
		TOGGLE_OUTPUT,
		THEME,
		ANALYZE,
		DECODE_HERE,
		SHORTCUTS,
		ABOUT,
		COUNT,

	};

	enum class line_kind : int32_t {

		BANNER,
		BLANK,
		LABEL,
		CODE,
		DATA,
		STRING,
		POINTER,
		EMPTY,

	};

	enum class log_kind : int32_t {

		INFO,
		GOOD,
		WARN,
		FAIL,

	};

	struct widget {

		uint64_t id = 0;
		float hover = 0.0f;
		float press = 0.0f;
		float value = 0.0f;
		float velocity = 0.0f;
		uint32_t seen = 0;

	};

	struct hit_rect {

		uint64_t id;
		math::rect area;
		hit kind;

	};

	struct field {

		wchar_t text [ 256 ] = { };
		int32_t length = 0;
		int32_t caret = 0;
		int32_t anchor = 0;
		float scroll = 0.0f;
		uint32_t revision = 0;

	};

	struct scroller {

		float offset = 0.0f;
		float target = 0.0f;
		float velocity = 0.0f;
		float limit = 0.0f;

	};

	struct line_item {

		line_kind kind = line_kind::BLANK;
		int32_t section = 0;
		uint64_t address = 0;
		uint64_t size = 0;
		uint64_t first_row = 0;
		uint32_t rows = 1;
		uint32_t stride = 8;
		int32_t ref = -1;
		bool function = false;

	};

	struct label_entry {

		uint64_t address = 0;
		int32_t name = -1;
		bool function = false;

	};

	struct log_line {

		log_kind kind = log_kind::INFO;
		base::text text;
		double time = 0.0;

	};

	struct palette_entry {

		command action = command::COUNT;
		int32_t score = 0;

	};

	uint64_t make_id ( const char* name, int64_t index = 0 );

	class app {

	public:

		bool setup ( render::gpu* device, void* window_handle );

		void rebuild_gpu ( render::gpu* device );

		void shutdown ( );

		void resize ( int32_t width_px, int32_t height_px, float dpi_scale );

		void frame ( double seconds );

		bool busy ( ) const;

		double wake_after ( ) const;

		void mouse_move ( float x, float y );

		void mouse_button ( int32_t button, bool down );

		void mouse_leave ( );

		void mouse_wheel ( float steps, bool horizontal );

		void key ( uint32_t code, bool control, bool shift, bool alt );

		void character ( wchar_t value );

		hit hit_test ( float x, float y ) const;

		request take_request ( );

		pointer cursor ( ) const;

		void set_maximized ( bool value );

		void set_focus ( bool value );

		bool open_file ( const wchar_t* path );

		void close_file ( );

		void run ( command action );

		void set_theme ( bool light );

		void load_settings ( );

		void save_settings ( );

		void log ( log_kind kind, const wchar_t* text );

		render::gpu* gpu = nullptr;
		void* window = nullptr;
		render::canvas canvas;
		image::binary file;
		int32_t width = 0;
		int32_t height = 0;
		float dpi = 1.0f;
		float w = 0.0f;
		float h = 0.0f;
		double clock = 0.0;
		double last = -1.0;
		float dt = 0.0f;
		bool fixed_step = false;
		bool persist = true;
		bool maximized = false;
		bool focused = true;
		bool animating = false;
		mutable int32_t pending = 3;
		bool light = false;
		float theme_mix = 0.0f;
		float theme_velocity = 0.0f;
		render::color tones [ static_cast < int32_t > ( tone::COUNT ) ];
		int32_t placement [ 4 ] = { };
		bool placement_valid = false;
		bool placement_maximized = false;

		math::vec2 mouse = math::vec2 ( -1000.0f, -1000.0f );
		bool mouse_inside = false;
		bool mouse_down = false;
		bool press_event = false;
		bool release_event = false;
		bool right_event = false;
		bool double_event = false;
		double last_press = -10.0;
		math::vec2 press_at;
		float wheel_steps = 0.0f;
		float wheel_side = 0.0f;
		uint64_t hot = 0;
		uint64_t active_id = 0;
		uint64_t clicked_id = 0;
		uint64_t double_id = 0;
		uint64_t right_id = 0;
		uint64_t scroll_hot = 0;
		base::list < hit_rect > hits;
		base::list < hit_rect > next_hits;
		base::list < hit_rect > scrolls;
		base::list < hit_rect > next_scrolls;
		base::list < uint32_t > keys;
		base::list < wchar_t > typed;
		bool key_control = false;
		bool key_shift = false;
		bool key_alt = false;
		request pending_request = request::NONE;
		pointer wanted_cursor = pointer::ARROW;
		widget* widgets = nullptr;
		static constexpr int32_t widget_room = 4096;
		uint32_t frame_index = 1;
		uint64_t focus_id = 0;
		field* focused_field = nullptr;
		bool field_submit = false;
		double caret_time = 0.0;
		const wchar_t* tip_text = nullptr;
		uint64_t tip_id = 0;
		float tip_time = 0.0f;
		float tip_show = 0.0f;
		math::rect tip_anchor;
		uint64_t tip_candidate = 0;
		const wchar_t* tip_candidate_text = nullptr;
		math::rect tip_candidate_anchor;

		math::rect title_area;
		math::rect band_area;
		math::rect status_area;
		math::rect rail_area;
		math::rect browser_area;
		math::rect inspector_area;
		math::rect output_area;
		math::rect center_area;
		math::rect tabs_area;
		math::rect view_area;

		float browser_width = 288.0f;
		float inspector_width = 300.0f;
		float browser_fit = 288.0f;
		float inspector_fit = 300.0f;
		float output_height = 190.0f;
		bool browser_open = true;
		bool inspector_open = true;
		bool output_open = true;
		float browser_show = 1.0f;
		float browser_velocity = 0.0f;
		float inspector_show = 1.0f;
		float inspector_velocity = 0.0f;
		float output_show = 1.0f;
		float output_velocity = 0.0f;
		int32_t dragging_split = 0;
		float split_grab = 0.0f;

		panel browsing = panel::FUNCTIONS;
		float rail_marker = 0.0f;
		float rail_velocity = 0.0f;
		view showing = view::LISTING;
		float tab_x = 0.0f;
		float tab_w = 0.0f;
		float tab_vx = 0.0f;
		float tab_vw = 0.0f;
		bool tab_ready = false;
		float view_fade = 1.0f;

		int32_t menu_open = -1;
		float menu_show = 0.0f;
		float menu_velocity = 0.0f;
		math::rect menu_rect;
		bool menu_keyboard = false;
		int32_t menu_focus = -1;

		bool palette_open = false;
		float palette_show = 0.0f;
		float palette_velocity = 0.0f;
		bool palette_address = false;
		field palette_field;
		uint32_t palette_seen = 0xFFFFFFFFu;
		base::list < palette_entry > palette_items;
		int32_t palette_pick = 0;
		float palette_pick_y = 0.0f;
		float palette_pick_velocity = 0.0f;

		bool about_open = false;
		float about_show = 0.0f;
		float about_velocity = 0.0f;
		bool keys_open = false;
		float keys_show = 0.0f;
		float keys_velocity = 0.0f;

		field filters [ static_cast < int32_t > ( panel::COUNT ) ];
		uint32_t filter_seen [ static_cast < int32_t > ( panel::COUNT ) ] = { };
		base::list < int32_t > shown [ static_cast < int32_t > ( panel::COUNT ) ];
		scroller browser_scroll [ static_cast < int32_t > ( panel::COUNT ) ];
		int32_t browser_pick [ static_cast < int32_t > ( panel::COUNT ) ] = { -1, -1, -1, -1, -1 };

		base::list < line_item > items;
		base::list < label_entry > labels;
		base::list < base::text > names;
		uint64_t total_rows = 0;
		scroller listing_scroll;
		uint64_t cursor_row = 0;
		float cursor_y = 0.0f;
		float cursor_velocity = 0.0f;
		uint64_t cursor_address = 0;
		bool cursor_ready = false;

		scroller hex_scroll;
		uint64_t hex_cursor = 0;
		uint64_t hex_anchor = 0;
		bool hex_selecting = false;

		scroller headers_scroll;
		scroller inspector_scroll;
		scroller output_scroll;

		base::list < uint64_t > back_stack;
		base::list < uint64_t > forward_stack;

		base::list < log_line > output;
		field command_field;
		base::list < base::text > recent;
		base::text load_problem;
		double load_time = -10.0;
		float welcome_show = 1.0f;
		float welcome_velocity = 0.0f;
		float band_hover = -1.0f;

	private:

		widget& state ( uint64_t id );

		bool over ( math::rect area ) const;

		bool interact ( uint64_t id, math::rect area, widget& item, hit kind = hit::CLIENT );

		void scroll_region ( uint64_t id, math::rect area );

		float take_wheel ( uint64_t id );

		void tip ( uint64_t id, const wchar_t* text, math::rect anchor );

		void approach ( float& value, float target, float rate );

		void spring ( float& value, float& velocity, float target, float stiffness, float damping );

		void glide ( scroller& item, float limit );

		bool begin_frame ( double seconds );

		void begin_input ( );

		void end_input ( );

		void handle_keys ( );

		bool edit_keys ( field& value, uint32_t code );

		void update_theme ( );

		render::color ink ( tone value ) const;

		void layout ( );

		bool enabled ( command action ) const;

		bool checked ( command action ) const;

		void build_listing ( );

		uint64_t row_of ( uint64_t address ) const;

		int32_t item_at ( uint64_t row ) const;

		uint64_t address_at_row ( uint64_t row ) const;

		void go ( uint64_t address, bool remember );

		void go_offset ( uint64_t offset );

		void set_cursor_row ( uint64_t row, bool center );

		bool parse_address ( const wchar_t* text, uint64_t& result ) const;

		void refresh_filter ( panel which );

		void run_command_line ( );

		void copy_text ( const wchar_t* text );

		void remember_recent ( const wchar_t* path );

		void label_name ( int32_t name, wchar_t* out, int32_t room ) const;

		void draw_brand ( math::vec2 center, float height );

		void draw_title ( );

		void draw_menu_bar ( math::rect area );

		void draw_menu ( );

		void draw_band ( );

		void draw_status ( );

		void draw_rail ( );

		void draw_browser ( );

		void draw_browser_rows ( panel which, math::rect area );

		void draw_tabs ( );

		void draw_view ( );

		void draw_listing ( math::rect area );

		void draw_hex ( math::rect area );

		void draw_headers ( math::rect area );

		void draw_pending ( math::rect area, const wchar_t* title, const wchar_t* detail, uint32_t glyph );

		void draw_inspector ( );

		void draw_output ( );

		void draw_welcome ( math::rect area );

		void draw_palette ( );

		void draw_dialogs ( );

		void draw_tip ( );

		void draw_scrollbar ( uint64_t id, math::rect track, scroller& item, float content, float page );

		bool splitter ( uint64_t id, math::rect area, bool vertical );

		bool button ( uint64_t id, math::rect area, const wchar_t* label, uint32_t glyph, bool primary );

		bool icon_button ( uint64_t id, math::vec2 center, float size, uint32_t glyph, float glyph_size, const wchar_t* text, hit kind = hit::CLIENT );

		bool text_field ( uint64_t id, field& value, math::rect area, const wchar_t* placeholder, uint32_t glyph, render::font role );

		void focus_field ( uint64_t id, field& value, bool select );

		void section_heading ( math::rect area, const wchar_t* text, const wchar_t* detail );

		void key_cap ( math::vec2 at, const wchar_t* keys, render::color tint, bool right );

	};

	const wchar_t* command_label ( command action );

	const wchar_t* command_keys ( command action );

}