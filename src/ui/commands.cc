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
		{ L"Close file", L"Ctrl+Shift+W" },
		{ L"Exit", L"Alt+F4" },
		{ L"Copy", L"Ctrl+C" },
		{ L"Go to address or name", L"G" },
		{ L"Go to entry point", L"Ctrl+E" },
		{ L"Back", L"Esc" },
		{ L"Forward", L"Alt+Right" },
		{ L"Next section", L"Ctrl+PgDn" },
		{ L"Previous section", L"Ctrl+PgUp" },
		{ L"Find in strings", L"Ctrl+F" },
		{ L"Command palette", L"Ctrl+Shift+P" },
		{ L"New disassembly tab", L"Ctrl+1" },
		{ L"Decompile function", L"F5" },
		{ L"New hex tab", L"Ctrl+2" },
		{ L"New headers tab", L"Ctrl+3" },
		{ L"Switch between C and disassembly", L"Tab" },
		{ L"Graph or text view", L"Space" },
		{ L"Split view", L"Ctrl+\\" },
		{ L"Close tab", L"Ctrl+W" },
		{ L"Next tab", L"Ctrl+Tab" },
		{ L"Functions", L"Alt+1" },
		{ L"Imports", L"Alt+2" },
		{ L"Exports", L"Alt+3" },
		{ L"Strings", L"Alt+4" },
		{ L"Sections", L"Alt+5" },
		{ L"Classes from RTTI", L"Alt+6" },
		{ L"References", L"X" },
		{ L"Search results", L"Alt+8" },
		{ L"Toggle browser", L"Ctrl+B" },
		{ L"Toggle inspector", L"Ctrl+I" },
		{ L"Toggle output", L"Ctrl+J" },
		{ L"Switch light or dark theme", L"Ctrl+Shift+L" },
		{ L"Follow reference", L"Enter" },
		{ L"Rename", L"N" },
		{ L"Comment", L";" },
		{ L"Toggle bookmark", L"Ctrl+D" },
		{ L"Number as hex or decimal", L"H" },
		{ L"Number as character", L"R" },
		{ L"Number as binary", L"B" },
		{ L"Number as float", L"F" },
		{ L"Default number format", L"Shift+H" },
		{ L"Make signature", L"Ctrl+Alt+S" },
		{ L"Find signature or bytes", L"Ctrl+Alt+F" },
		{ L"Copy function as C", L"Ctrl+Shift+C" },
		{ L"Keyboard shortcuts", L"F1" },
		{ L"About Plumb", L"" }
	};

	const wchar_t* command_label ( command action ) {

		return commands [ static_cast < int32_t > ( action ) ].label;

	}

	const wchar_t* command_keys ( command action ) {

		return commands [ static_cast < int32_t > ( action ) ].keys;

	}

	bool app::enabled ( command action ) const {

		bool has = file.loaded ( );
		bool analyzed = has && analysis_applied && db && !db->maps.empty ( );

		switch ( action ) {

			case command::OPEN:
			case command::EXIT:
			case command::PALETTE:
			case command::THEME:
			case command::SHORTCUTS:
			case command::ABOUT:
				return true;

			case command::BACK:
				return has && !now->back_stack.empty ( );

			case command::FORWARD:
				return has && !now->forward_stack.empty ( );

			case command::GO_ENTRY:
				return has && file.entry != 0;

			case command::VIEW_PSEUDOCODE:
			case command::SWAP_VIEW:
			case command::COPY_FUNCTION:
				return analyzed && db->function_at ( now->cursor_address ) >= 0;

			case command::MAKE_SIGNATURE:
				return analyzed && db->is_covered ( now->cursor_address );

			case command::NUMBER_RADIX:
			case command::NUMBER_CHAR:
			case command::NUMBER_BINARY:
			case command::NUMBER_FLOAT:
			case command::NUMBER_RESET:
				return analyzed && ( showing == view::LISTING || showing == view::GRAPH ) && db->is_covered ( now->cursor_address );

			case command::TOGGLE_GRAPH:
				return analyzed && ( showing == view::GRAPH || ( showing == view::LISTING && db->function_at ( now->cursor_address ) >= 0 ) );

			case command::CLOSE_TAB:
			case command::NEXT_TAB:
				return has && ( panes [ focused_pane ].count > 1 || pane_count > 1 );

			default:
				return has;

		}

	}

	bool app::checked ( command action ) const {

		switch ( action ) {

			case command::SPLIT:
				return pane_count > 1;

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

			if ( action == command::VIEW_PSEUDOCODE && file.loaded ( ) && !analysis_applied )
				log ( log_kind::WARN, L"Still analyzing. Pseudocode opens as soon as functions are known." );
			else if ( action == command::VIEW_PSEUDOCODE && file.loaded ( ) )
				log ( log_kind::WARN, L"The cursor isn't inside a function." );

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

				if ( showing == view::LISTING && now->ranged && now->select_anchor != now->cursor_row ) {

					uint64_t low = base::min ( now->select_anchor, now->cursor_row );
					uint64_t high = base::min ( base::max ( now->select_anchor, now->cursor_row ), low + 20000 );
					base::list < wchar_t > text;

					for ( uint64_t row = low; row <= high; row++ ) {

						base::writer piece;

						row_text ( row, piece );

						for ( int32_t i = 0; i < piece.size ( ); i++ )
							text.push ( piece.text ( ) [ i ] );

						text.push ( L'\r' );
						text.push ( L'\n' );

					}

					text.push ( 0 );
					copy_text ( text.data ( ) );
					log ( log_kind::INFO, base::writer ( ).add ( L"Copied " ).grouped ( high - low + 1 ).add ( L" lines." ).text ( ) );

					break;

				}

				if ( showing == view::PSEUDOCODE ) {

					const decompiler::result* code = pseudocode_for ( now->function );

					if ( !code || code->lines.empty ( ) )
						break;

					int32_t low = base::clamp ( base::min ( now->code_anchor, now->code_line ), 0, code->lines.count ( ) - 1 );
					int32_t high = base::clamp ( base::max ( now->code_anchor, now->code_line ), 0, code->lines.count ( ) - 1 );
					int32_t base_indent = 1000;
					base::list < wchar_t > text;

					for ( int32_t i = low; i <= high; i++ )
						base_indent = base::min ( base_indent, code->lines [ static_cast < size_t > ( i ) ].indent );

					for ( int32_t i = low; i <= high; i++ ) {

						const decompiler::line& item = code->lines [ static_cast < size_t > ( i ) ];

						for ( int32_t k = base_indent; k < item.indent; k++ ) {

							for ( int32_t s = 0; s < 4; s++ )
								text.push ( L' ' );

						}

						for ( int32_t k = 0; k < item.length; k++ )
							text.push ( code->text [ static_cast < size_t > ( item.text + k ) ] );

						if ( i < high ) {

							text.push ( L'\r' );
							text.push ( L'\n' );

						}

					}

					text.push ( 0 );
					copy_text ( text.data ( ) );
					log ( log_kind::INFO, base::writer ( ).add ( L"Copied " ).number ( high - low + 1 ).add ( high == low ? L" line of C." : L" lines of C." ).text ( ) );

					break;

				}

				if ( showing == view::HEX ) {

					uint64_t low = base::min ( now->hex_cursor, now->hex_anchor );
					uint64_t high = base::min ( base::max ( now->hex_cursor, now->hex_anchor ), file.size ? file.size - 1 : 0 );

					for ( uint64_t i = low; i <= high && i - low < 600; i++ ) {

						if ( i > low )
							line.add ( L' ' );

						line.hex ( file.byte ( i ), 2 );

					}

				} else if ( db && db->is_start ( now->cursor_address ) ) {

					x86::instruction ins;
					wchar_t text [ 300 ];
					struct names_bridge : x86::symbolizer {

						const app* owner;

						bool name ( uint64_t address, bool code, wchar_t* out, int32_t room ) override {

							return owner->symbol_name ( address, code, out, room );

						}

					} bridge;

					bridge.owner = this;

					if ( db->decode ( now->cursor_address, ins ) ) {

						x86::format_text ( ins, text, 300, &bridge, 0 );
						line.add ( text );

					}

				} else
					line.add ( L"0x" ).hex ( now->cursor_address, file.wide ( ) ? 16 : 8 );

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

				if ( !now->back_stack.empty ( ) ) {

					uint64_t target = now->back_stack.back ( );

					now->forward_stack.push ( showing == view::PSEUDOCODE ? now->function : now->cursor_address );
					now->back_stack.pop ( );

					if ( showing == view::PSEUDOCODE ) {

						now->function = target;
						now->code_scroll = scroller ( );
						now->code_line = 0;
						now->code_ready = false;

					} else
						go ( target, false );

				}

				break;

			case command::FORWARD:

				if ( !now->forward_stack.empty ( ) ) {

					uint64_t target = now->forward_stack.back ( );

					now->back_stack.push ( showing == view::PSEUDOCODE ? now->function : now->cursor_address );
					now->forward_stack.pop ( );

					if ( showing == view::PSEUDOCODE ) {

						now->function = target;
						now->code_scroll = scroller ( );
						now->code_line = 0;
						now->code_ready = false;

					} else
						go ( target, false );

				}

				break;

			case command::NEXT_SECTION:
			case command::PREVIOUS_SECTION: {

				int32_t current = file.section_at ( now->cursor_address );
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
				open_document ( view::LISTING, focused_pane, now->cursor_address );
				break;

			case command::VIEW_HEX:
				open_document ( view::HEX, focused_pane, now->cursor_address );
				break;

			case command::VIEW_HEADERS:
				open_document ( view::HEADERS, focused_pane, now->cursor_address );
				break;

			case command::VIEW_PSEUDOCODE:
				open_pseudocode ( now->cursor_address, pane_count > 1 );
				break;

			case command::SWAP_VIEW: {

				uint64_t target = showing == view::PSEUDOCODE ? now->function : now->cursor_address;

				if ( showing == view::PSEUDOCODE ) {

					pane& item = panes [ focused_pane ];

					for ( int32_t i = 0; i < item.count; i++ ) {

						const document& candidate = documents [ item.tabs [ i ] ];

						if ( candidate.kind == view::LISTING ) {

							focus_document ( focused_pane, i );
							go ( target, true );

							return;

						}

					}

					open_document ( view::LISTING, focused_pane, target );

				} else
					open_pseudocode ( target, false );

				break;

			}

			case command::TOGGLE_GRAPH: {

				int32_t current = current_document ( );

				if ( current < 0 )
					break;

				document& item = documents [ current ];

				if ( item.kind == view::GRAPH ) {

					item.kind = view::LISTING;
					select_state ( current );
					set_cursor_row ( row_of ( now->cursor_address ), true );

				} else {

					item.kind = view::GRAPH;
					item.state.graph_ready = false;
					item.state.graph_function = db->functions [ static_cast < size_t > ( db->function_at ( now->cursor_address ) ) ].start;
					select_state ( current );

				}

				item.fade = 0.0f;

				break;

			}

			case command::SPLIT:

				if ( pane_count > 1 ) {

					pane& second = panes [ 1 ];

					for ( int32_t i = 0; i < second.count && panes [ 0 ].count < 16; i++ )
						panes [ 0 ].tabs [ panes [ 0 ].count++ ] = second.tabs [ i ];

					second.count = 0;
					pane_count = 1;
					focus_document ( 0, panes [ 0 ].active );

				} else {

					pane_count = 2;
					panes [ 1 ] = pane ( );
					open_document ( view::LISTING, 1, now->cursor_address );

				}

				break;

			case command::CLOSE_TAB:
				close_document ( focused_pane, panes [ focused_pane ].active );
				break;

			case command::NEXT_TAB: {

				pane& item = panes [ focused_pane ];

				if ( item.count > 1 )
					focus_document ( focused_pane, ( item.active + ( key_shift ? item.count - 1 : 1 ) ) % item.count );
				else if ( pane_count > 1 )
					focus_document ( 1 - focused_pane, panes [ 1 - focused_pane ].active );

				break;

			}

			case command::SHOW_FUNCTIONS:
			case command::SHOW_IMPORTS:
			case command::SHOW_EXPORTS:
			case command::SHOW_STRINGS:
			case command::SHOW_SECTIONS:
			case command::SHOW_CLASSES:
			case command::SHOW_REFERENCES:
			case command::SHOW_SEARCH:
				browsing = static_cast < panel > ( static_cast < int32_t > ( action ) - static_cast < int32_t > ( command::SHOW_FUNCTIONS ) );
				browser_open = true;

				if ( browsing == panel::REFERENCES )
					references_for = ~0ull;

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

			case command::FOLLOW: {

				if ( !db || !analysis_applied )
					break;

				x86::instruction ins;
				uint64_t target = 0;

				if ( db->decode ( now->cursor_address, ins ) ) {

					if ( ins.direct ( ) )
						target = ins.target ( );
					else if ( !ins.memory_target ( target ) ) {

						for ( int32_t i = 0; i < ins.count; i++ ) {

							if ( ins.operands [ i ].type == x86::kind::IMM && file.section_at ( static_cast < uint64_t > ( ins.operands [ i ].value ) ) > 0 )
								target = static_cast < uint64_t > ( ins.operands [ i ].value );

						}

					}

				}

				if ( target && file.section_at ( target ) >= 0 )
					go ( target, true );

				break;

			}

			case command::RENAME:
			case command::COMMENT: {

				uint64_t address = now->cursor_address;

				if ( action == command::RENAME && db && analysis_applied ) {

					int32_t owner = db->function_at ( address );

					if ( owner >= 0 && !db->is_label ( address ) )
						address = db->functions [ static_cast < size_t > ( owner ) ].start;

				}

				open_prompt ( action, address );

				break;

			}

			case command::NUMBER_RADIX:
			case command::NUMBER_CHAR:
			case command::NUMBER_BINARY:
			case command::NUMBER_FLOAT:
			case command::NUMBER_RESET: {

				uint64_t address = db->instruction_at ( now->cursor_address );
				x86::instruction ins;
				uint8_t size = 0;

				if ( db->decode ( address, ins ) ) {

					for ( int32_t i = 0; i < ins.count; i++ ) {

						if ( ins.operands [ i ].type == x86::kind::IMM )
							size = ins.operands [ i ].size;

					}

				}

				if ( !size ) {

					log ( log_kind::WARN, L"This instruction has no immediate to reformat." );

					break;

				}

				const wchar_t* current = user_text ( number_forms, address );
				wchar_t first = current ? current [ 0 ] : 0;
				const wchar_t* form = L"";

				if ( action == command::NUMBER_RADIX )
					form = first == L'd' ? L"hex" : L"dec";
				else if ( action == command::NUMBER_CHAR )
					form = first == L'c' ? L"" : L"char";
				else if ( action == command::NUMBER_BINARY )
					form = first == L'b' ? L"" : L"bin";
				else if ( action == command::NUMBER_FLOAT ) {

					if ( size != 4 && size != 8 ) {

						log ( log_kind::WARN, L"Only 4 and 8 byte immediates can be shown as float." );

						break;

					}

					form = first == L'f' ? L"" : L"float";

				}

				set_user_text ( number_forms, address, form );
				save_project ( );

				break;

			}

			case command::BOOKMARK:

				if ( user_text ( bookmarks, now->cursor_address ) ) {

					set_user_text ( bookmarks, now->cursor_address, L"" );
					log ( log_kind::INFO, L"Bookmark removed." );

				} else {

					wchar_t name [ 160 ];

					if ( !symbol_name ( now->cursor_address, true, name, 160 ) ) {

						base::writer fallback;

						fallback.hex ( now->cursor_address );

						for ( int32_t i = 0; i <= fallback.size ( ) && i < 160; i++ )
							name [ i ] = fallback.text ( ) [ i ];

					}

					set_user_text ( bookmarks, now->cursor_address, name );
					log ( log_kind::GOOD, base::writer ( ).add ( L"Bookmarked " ).add ( name ).text ( ) );

				}

				save_project ( );

				break;

			case command::MAKE_SIGNATURE:
				make_signature ( now->cursor_address );
				break;

			case command::FIND_SIGNATURE:
				open_prompt ( action, now->cursor_address );
				break;

			case command::COPY_FUNCTION: {

				int32_t owner = db->function_at ( now->cursor_address );

				if ( owner < 0 )
					break;

				const decompiler::result* code = pseudocode_for ( db->functions [ static_cast < size_t > ( owner ) ].start );

				if ( !code || !code->ok )
					break;

				base::list < wchar_t > text;

				for ( const decompiler::line& item : code->lines ) {

					for ( int32_t i = 0; i < item.indent; i++ )
						text.push ( L'\t' );

					for ( int32_t i = 0; i < item.length; i++ )
						text.push ( code->text [ static_cast < size_t > ( item.text + i ) ] );

					text.push ( L'\r' );
					text.push ( L'\n' );

				}

				text.push ( 0 );
				copy_text ( text.data ( ) );
				log ( log_kind::INFO, base::writer ( ).add ( L"Copied " ).number ( code->lines.count ( ) ).add ( L" lines of C." ).text ( ) );

				break;

			}

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
			log ( log_kind::INFO, L"Commands: go <address|name>, entry, back, forward, section <name>, view <asm|c|hex|headers>, find <pattern>, sig, rename <name>, comment <text>, split, theme <light|dark>, open, close, clear" );
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
		else if ( base::same_text ( word, L"split" ) )
			run ( command::SPLIT );
		else if ( base::same_text ( word, L"sig" ) )
			run ( command::MAKE_SIGNATURE );
		else if ( base::same_text ( word, L"find" ) && file.loaded ( ) )
			find_signature ( rest );
		else if ( ( base::same_text ( word, L"rename" ) || base::same_text ( word, L"comment" ) ) && file.loaded ( ) ) {

			bool rename = base::same_text ( word, L"rename" );
			uint64_t target = now->cursor_address;

			if ( rename && db && analysis_applied && db->function_at ( target ) >= 0 && !db->is_label ( target ) )
				target = db->functions [ static_cast < size_t > ( db->function_at ( target ) ) ].start;

			set_user_text ( rename ? renamed : comments, target, rest );
			names_revision++;
			save_project ( );

		} else if ( base::same_text ( word, L"theme" ) )
			set_theme ( base::same_text ( rest, L"light" ) || ( !rest [ 0 ] && !light ) );
		else if ( base::same_text ( word, L"view" ) ) {

			if ( base::same_text ( rest, L"hex" ) )
				run ( command::VIEW_HEX );
			else if ( base::same_text ( rest, L"headers" ) )
				run ( command::VIEW_HEADERS );
			else if ( base::same_text ( rest, L"c" ) || base::same_text ( rest, L"pseudocode" ) )
				run ( command::VIEW_PSEUDOCODE );
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

}