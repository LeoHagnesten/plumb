#include "ui/app.hh"
#include "ui/icons.hh"
#include "ui/format.hh"
#include "ui/tokens.hh"

namespace ui {

	void app::draw_listing ( math::rect area ) {

		render::canvas& c = canvas;
		float row_h = 20.0f;
		float adv = c.measure ( L"0000000000", render::font::MONO ) * 0.1f;
		uint64_t id = make_id ( "listing", drawing_serial );
		widget& surface = state ( id );
		float visible = area.h / row_h;
		bool analyzed = db && analysis_applied && !db->maps.empty ( );

		c.fill ( area, 0.0f, ink ( tone::BASE ) );
		scroll_region ( id, area );
		now->listing_scroll.target -= take_wheel ( id ) * 3.0f;
		glide ( now->listing_scroll, static_cast < float > ( total_rows ) - visible + 3.0f, row_h );
		interact ( id, area, surface );

		float offset = base::max ( now->listing_scroll.shown, 0.0f );
		uint64_t first = static_cast < uint64_t > ( offset );
		float shift = ( offset - static_cast < float > ( first ) ) * row_h;
		int32_t count = static_cast < int32_t > ( visible ) + 2;

		if ( total_rows && !now->selecting && ( now->row_build != listing_build || now->row_for != now->cursor_address ) ) {

			uint64_t address = now->cursor_address;
			uint64_t row = row_of ( address );
			float place = static_cast < float > ( row );
			bool outside = place < now->listing_scroll.target || place > now->listing_scroll.target + visible - 2.0f;

			set_cursor_row ( row, outside );

			if ( address )
				now->cursor_address = address;

			now->row_for = now->cursor_address;
			now->cursor_y = place;
			now->cursor_velocity = 0.0f;
			now->ranged = false;

		}

		if ( !now->ranged )
			now->select_anchor = now->cursor_row;

		if ( active_id == id && mouse_down && total_rows ) {

			float local = ( mouse.y - area.y + shift ) / row_h;
			int64_t pointed = static_cast < int64_t > ( first ) + static_cast < int64_t > ( local < 0.0f ? local - 1.0f : local );
			uint64_t row = static_cast < uint64_t > ( base::clamp < int64_t > ( pointed, 0, static_cast < int64_t > ( total_rows ) - 1 ) );

			if ( press_event && over ( area ) ) {

				set_cursor_row ( row, false );

				if ( key_shift )
					now->ranged = true;
				else {

					now->select_anchor = row;
					now->ranged = false;

				}

				now->selecting = true;

			} else if ( now->selecting && row != now->cursor_row ) {

				set_cursor_row ( row, false );
				now->ranged = true;

				if ( mouse.y < area.y || mouse.y > area.bottom ( ) )
					animating = true;

			}

		}

		if ( !mouse_down )
			now->selecting = false;

		if ( !now->cursor_ready ) {

			now->cursor_y = static_cast < float > ( now->cursor_row );
			now->cursor_ready = true;

		}

		now->cursor_y = static_cast < float > ( now->cursor_row );
		now->cursor_velocity = 0.0f;

		uint64_t high = file.high ( );
		int32_t digits = 1;

		while ( digits < 16 && ( high >> ( digits * 4 ) ) )
			digits++;

		float x0 = area.x + 22.0f;
		bool show_bytes = area.w >= adv * 106.0f;
		float x_bytes = x0 + adv * static_cast < float > ( digits + 2 );
		float x_mnem = show_bytes ? x_bytes + adv * 25.0f : x_bytes;
		float x_ops = x_mnem + adv * 8.0f;
		float x_note = base::max ( base::min ( x_ops + adv * 46.0f, area.right ( ) - adv * 28.0f ), x_ops + adv * 18.0f );
		float ops_room = x_note - x_ops - adv * 2.0f;
		int32_t hover_row = over ( area ) ? static_cast < int32_t > ( ( mouse.y - area.y + shift ) / row_h ) : -1;
		uint64_t follow_target = 0;
		bool hand = false;
		listing_names namer;
		wchar_t name [ 200 ];

		namer.owner = this;
		c.push_clip ( area );

		for ( int32_t pass = 0; pass < 2; pass++ ) {

			if ( pass == 1 && now->ranged && now->select_anchor != now->cursor_row ) {

				uint64_t low = base::min ( now->select_anchor, now->cursor_row );
				uint64_t high = base::max ( now->select_anchor, now->cursor_row );
				uint64_t top = base::max ( low, first );
				uint64_t bottom = base::min ( high, first + static_cast < uint64_t > ( count ) );

				if ( top <= bottom ) {

					float y0 = area.y + static_cast < float > ( top - first ) * row_h - shift;
					float y1 = area.y + static_cast < float > ( bottom - first + 1 ) * row_h - shift;

					c.fill ( math::rect ( area.x, y0, area.w, y1 - y0 ), 0.0f, ink ( tone::SELECT ).fade ( 0.75f ) );

				}

			}

			if ( pass == 1 ) {

				float cy = area.y + ( now->cursor_y - offset ) * row_h;

				if ( cy > area.y - row_h && cy < area.bottom ( ) ) {

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
				const loader::section& owner = file.sections [ static_cast < size_t > ( item.section ) ];
				bool label_row = false;
				uint64_t address = item.address;

				if ( item.kind == line_kind::CODE && analyzed )
					address = db->row_address ( item.address, item.address + item.size, sub, label_row );
				else if ( item.kind == line_kind::DATA || item.kind == line_kind::TABLE || item.kind == line_kind::VTABLE )
					address = item.address + static_cast < uint64_t > ( sub ) * item.stride;

				int32_t function = analyzed ? db->function_at ( address ) : -1;
				bool inside = function >= 0 && item.kind != line_kind::BANNER && item.kind != line_kind::BLANK;

				if ( pass == 0 ) {

					if ( item.kind == line_kind::BANNER && sub == 0 ) {

						c.fill ( math::rect ( area.x, y + 1.0f, area.w, row_h - 2.0f ), 0.0f, ink ( tone::PANEL ) );
						c.fill ( math::rect ( area.x, y + 1.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );
						c.fill ( math::rect ( area.x, y + row_h - 2.0f, area.w, 1.0f ), 0.0f, ink ( tone::LINE ) );

					} else if ( item.kind == line_kind::FUNCTION ) {

						c.fill ( math::rect ( area.x, y, area.w, row_h ), 0.0f, ink ( tone::RAISED ) );
						c.fill ( math::rect ( area.x, y, area.w, 1.0f ), 0.0f, ink ( tone::LINE_STRONG ) );

					} else if ( item.kind == line_kind::END )
						c.fill ( math::rect ( x0, cy, area.right ( ) - x0 - 12.0f, 1.0f ), 0.0f, ink ( tone::LINE ) );

					if ( inside ) {

						float top = item.kind == line_kind::FUNCTION ? cy : y;
						float bottom = item.kind == line_kind::END ? cy + 1.0f : y + row_h;

						c.fill ( math::rect ( area.x + 9.0f, top, 2.0f, bottom - top ), 0.0f, ink ( tone::GUIDE ) );

						if ( item.kind == line_kind::END )
							c.fill ( math::rect ( area.x + 9.0f, cy - 1.0f, x0 - area.x - 12.0f, 2.0f ), 0.0f, ink ( tone::GUIDE ) );

					}

					if ( k == hover_row && item.kind != line_kind::BANNER && item.kind != line_kind::BLANK && row != now->cursor_row )
						c.fill ( math::rect ( area.x, y, area.w, row_h ), 0.0f, ink ( tone::HOVER ) );

					continue;

				}

				if ( item.kind == line_kind::BLANK || ( item.kind == line_kind::BANNER && sub > 0 ) )
					continue;

				base::writer text;

				if ( item.kind == line_kind::BANNER ) {

					base::writer detail;
					uint64_t span = base::max ( owner.size, owner.raw );

					detail.hex ( owner.address, digits ).add ( L" \x2013 " ).hex ( owner.address + span, digits ).add ( L"   " ).add ( loader::kind_name ( owner.type ) ).add ( L"   " ).add ( owner.read ? L"R" : L"-" ).add ( owner.write ? L"W" : L"-" ).add ( owner.execute ? L"X" : L"-" ).add ( L"   " ).grouped ( span ).add ( L" bytes" );
					c.text ( owner.name, render::font::MONO_STRONG, math::vec2 ( x0, cy ), ink ( tone::LABEL ) );
					c.text ( detail.text ( ), render::font::MONO_SMALL, math::vec2 ( x0 + adv * 10.0f, cy ), ink ( tone::INK_SOFT ) );

					continue;

				}

				if ( item.kind == line_kind::FUNCTION ) {

					const analysis::function_info& info = db->functions [ static_cast < size_t > ( item.ref ) ];
					base::writer meta;

					function_name ( item.ref, name, 200 );

					float name_w = c.text ( name, render::font::MONO_STRONG, math::vec2 ( x0, cy ), ink ( ( info.flags & analysis::THUNK ) ? tone::IMPORT : tone::FUNCTION ) );

					meta.grouped ( info.instructions ).add ( info.instructions == 1 ? L" instruction" : L" instructions" );

					if ( info.blocks > 1 )
						meta.add ( L" \x00B7 " ).number ( info.blocks ).add ( L" blocks" );

					meta.add ( L" \x00B7 " ).number ( info.callers ).add ( info.callers == 1 ? L" caller" : L" callers" );

					if ( info.flags & analysis::THUNK )
						meta.add ( L" \x00B7 import thunk" );
					else if ( info.flags & analysis::FROM_ENTRY )
						meta.add ( L" \x00B7 entry point" );
					else if ( info.flags & analysis::FROM_EXPORT )
						meta.add ( L" \x00B7 exported" );
					else if ( info.flags & analysis::FROM_TLS )
						meta.add ( L" \x00B7 TLS callback" );

					c.text ( meta.text ( ), render::font::SMALL, math::vec2 ( base::max ( x0 + name_w + adv * 3.0f, x_ops ), cy ), ink ( tone::INK_SOFT ), render::align::LEFT, area.right ( ) - x_ops - 16.0f );

					const wchar_t* note = user_text ( comments, info.start );

					if ( note )
						c.text ( note, render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ), render::align::LEFT, area.right ( ) - x_note - 12.0f );

					continue;

				}

				if ( item.kind == line_kind::END ) {

					const analysis::function_info& info = db->functions [ static_cast < size_t > ( item.ref ) ];
					base::writer end_text;

					function_name ( item.ref, name, 200 );
					end_text.add ( L" end of " ).add ( name ).add ( L"  \x00B7  " ).grouped ( info.end - info.start ).add ( L" bytes " );

					float tag_w = c.measure ( end_text.text ( ), render::font::SMALL );

					c.fill ( math::rect ( x_bytes - 6.0f, cy - 8.0f, tag_w + 8.0f, 16.0f ), 3.0f, ink ( tone::BASE ) );
					c.text ( end_text.text ( ), render::font::SMALL, math::vec2 ( x_bytes - 2.0f, cy ), ink ( tone::INK_FAINT ) );

					continue;

				}

				if ( item.kind == line_kind::CHUNK ) {

					const analysis::chunk_info& chunk = db->chunks [ static_cast < size_t > ( item.ref ) ];

					text.add ( L"chunk of " );

					if ( symbol_name ( chunk.owner, true, name, 200 ) )
						text.add ( name );
					else
						text.hex ( chunk.owner );

					c.text ( text.text ( ), render::font::SMALL, math::vec2 ( x_bytes, cy ), ink ( tone::LABEL ) );

					continue;

				}

				text.hex ( address, digits );
				c.text ( text.text ( ), render::font::MONO, math::vec2 ( x0, cy ), ink ( tone::ADDRESS ) );

				uint64_t offset_at = 0;
				bool mapped = file.offset_of ( address, offset_at );

				switch ( item.kind ) {

					case line_kind::CODE: {

						if ( !analyzed )
							break;

						if ( label_row ) {

							base::writer label;
							int32_t first_ref = 0;
							int32_t ref_count = 0;

							if ( symbol_name ( address, true, name, 200 ) )
								label.add ( name );
							else
								label.add ( L"loc_" ).hex ( address );

							label.add ( L':' );
							c.text ( label.text ( ), render::font::MONO_STRONG, math::vec2 ( x_bytes, cy ), ink ( tone::LABEL ) );
							db->refs_to ( address, first_ref, ref_count );

							if ( ref_count ) {

								base::writer refs;
								const analysis::xref& from = db->refs [ static_cast < size_t > ( first_ref ) ];
								uint64_t source = file.base + from.source;
								int32_t from_function = db->function_at ( source );

								refs.add ( L"\x2190 " );

								if ( from_function >= 0 && from_function == function ) {

									x86::instruction from_ins;

									if ( db->decode ( source, from_ins ) ) {

										wchar_t word [ 32 ];

										x86::mnemonic_text ( from_ins, word, 32 );
										refs.add ( word ).add ( L" " );

									}

									refs.add ( L"at +" ).hex ( source - db->functions [ static_cast < size_t > ( from_function ) ].start );

								}
								else if ( from_function >= 0 ) {

									function_name ( from_function, name, 200 );
									refs.add ( name );

								} else
									refs.hex ( source );

								if ( ref_count > 1 )
									refs.add ( L"  +" ).number ( ref_count - 1 );

								c.text ( refs.text ( ), render::font::MONO_SMALL, math::vec2 ( x_note, cy ), ink ( tone::INK_FAINT ), render::align::LEFT, area.right ( ) - x_note - 12.0f );

							}

							break;

						}

						x86::instruction ins;

						if ( !db->decode ( address, ins ) )
							break;

						if ( show_bytes && mapped ) {

							base::writer bytes;
							int32_t shown_bytes = base::min ( static_cast < int32_t > ( ins.length ), 7 );

							for ( int32_t b = 0; b < shown_bytes; b++ ) {

								if ( b )
									bytes.add ( L' ' );

								bytes.hex ( file.byte ( offset_at + static_cast < uint64_t > ( b ) ), 2 );

							}

							if ( ins.length > 7 )
								bytes.add ( L"\x2026" );

							c.text ( bytes.text ( ), render::font::MONO, math::vec2 ( x_bytes, cy ), ink ( tone::BYTES ) );

						}

						token_line line;

						line.owner = this;
						line.db = db;
						x86::format ( ins, line, &namer, 0 );

						int32_t split = line.operand_start < 0 ? line.length : line.operand_start;

						for ( int32_t r = 0; r < line.count; r++ ) {

							const token_run& run = line.runs [ r ];
							bool head = run.start < split;
							float base_x = head ? x_mnem : x_ops;
							int32_t column = head ? run.start : run.start - split;
							float rx = base_x + static_cast < float > ( column ) * adv;
							wchar_t piece [ 200 ];
							int32_t length = base::min ( run.length, 199 );

							if ( !head && rx > x_ops + ops_room )
								break;

							for ( int32_t i = 0; i < length; i++ )
								piece [ i ] = line.text [ run.start + i ];

							piece [ length ] = 0;

							tone color = run.color;

							if ( run.color == tone::MNEMONIC )
								color = mnemonic_tone ( ins );

							c.text ( piece, render::font::MONO, math::vec2 ( rx, cy ), ink ( color ), render::align::LEFT, head ? x_ops - rx - 4.0f : x_ops + ops_room - rx );

							if ( run.target && !head ) {

								math::rect token_area ( rx, y, static_cast < float > ( length ) * adv, row_h );

								if ( over ( token_area ) && over ( area ) ) {

									c.fill ( math::rect ( rx, cy + 7.0f, token_area.w, 1.0f ), 0.0f, ink ( color ).fade ( 0.7f ) );
									hand = true;

									if ( double_id == id || ( clicked_id == id && key_control ) )
										follow_target = run.target;

								}

							}

						}

						const wchar_t* note = user_text ( comments, address );
						base::writer annotation;
						tone note_tone = tone::COMMENT;

						if ( note ) {

							annotation.add ( L"; " ).add ( note );

						} else {

							uint64_t target = 0;

							if ( !ins.memory_target ( target ) ) {

								for ( int32_t i = 0; i < ins.count; i++ ) {

									if ( ins.operands [ i ].type == x86::kind::IMM && ins.operands [ i ].size >= 4 )
										target = static_cast < uint64_t > ( ins.operands [ i ].value );

								}

							}

							int32_t text_index = target ? string_at ( target ) : -1;

							if ( text_index >= 0 ) {

								const loader::found_string& found = file.strings [ static_cast < size_t > ( text_index ) ];

								annotation.add ( found.wide ? L"L\"" : L"\"" );

								for ( uint32_t i = 0; i < found.length && i < 80; i++ ) {

									wchar_t ch = static_cast < wchar_t > ( file.byte ( found.offset + static_cast < uint64_t > ( i ) * ( found.wide ? 2 : 1 ) ) );

									annotation.add ( ch == L'\t' ? L' ' : ch );

								}

								annotation.add ( found.length > 80 ? L"...\"" : L"\"" );
								note_tone = tone::STRING;

							} else if ( ins.control == x86::flow::JUMP && !ins.direct ( ) && db->table_for_jump ( address ) >= 0 ) {

								annotation.add ( L"switch, " ).number ( db->tables [ static_cast < size_t > ( db->table_for_jump ( address ) ) ].count ).add ( L" cases" );

							} else if ( ( ins.control == x86::flow::CALL || ins.control == x86::flow::JUMP ) && !ins.direct ( ) && indirect_note ( ins, annotation ) ) {

								note_tone = tone::FUNCTION;

							} else if ( ins.control == x86::flow::NONE && ins.immediate_size >= 4 && !user_text ( number_forms, address ) ) {

								for ( int32_t i = 0; i < ins.count; i++ ) {

									const x86::operand& item = ins.operands [ i ];

									if ( item.type == x86::kind::IMM && item.size == ins.immediate_size && float_hint ( annotation, static_cast < uint64_t > ( item.value ), item.size ) ) {

										note_tone = tone::NUMBER;

										break;

									}

								}

							}

						}

						if ( annotation.size ( ) )
							c.text ( annotation.text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( note_tone ), render::align::LEFT, area.right ( ) - x_note - 12.0f );

						break;

					}

					case line_kind::ALIGN: {

						base::writer value;

						value.number ( static_cast < int64_t > ( item.size ) ).add ( item.size == 1 ? L" byte of " : L" bytes of " );

						if ( item.ref == 0x90 )
							value.add ( L"nop" );
						else
							value.hex ( static_cast < uint64_t > ( item.ref ), 2 );
						c.text ( L"align", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::INK_FAINT ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::INK_FAINT ) );

						break;

					}

					case line_kind::TABLE: {

						if ( item.ref <= -2 ) {

							int32_t table_index = -item.ref - 2;
							base::writer bytes;
							uint64_t take = base::min ( static_cast < uint64_t > ( 8 ), item.address + item.size - address );

							for ( uint64_t b = 0; b < take && mapped; b++ ) {

								if ( b )
									bytes.add ( L", " );

								bytes.number ( file.byte ( offset_at + b ) );

							}

							c.text ( L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
							c.text ( bytes.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::NUMBER ), render::align::LEFT, ops_room );

							if ( sub == 0 )
								c.text ( base::writer ( ).add ( L"; case index for switch " ).number ( table_index ).text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

							break;

						}

						const analysis::jump_table& table = db->tables [ static_cast < size_t > ( item.ref ) ];
						uint64_t raw = table.entry_size == 8 ? file.read64 ( offset_at ) : file.read32 ( offset_at );
						uint64_t target = table.type == analysis::table_kind::IMAGE_BASED ? file.base + static_cast < uint32_t > ( raw ) : ( table.type == analysis::table_kind::TABLE_BASED ? table.anchor + static_cast < uint64_t > ( static_cast < int64_t > ( static_cast < int32_t > ( raw ) ) ) : raw );
						base::writer value;

						if ( symbol_name ( target, true, name, 200 ) )
							value.add ( name );
						else
							value.hex ( target );

						c.text ( table.entry_size == 8 ? L"dq" : L"dd", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::LABEL ) );
						c.text ( base::writer ( ).add ( L"; case " ).number ( sub ).text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						math::rect token_area ( x_ops, y, static_cast < float > ( value.size ( ) ) * adv, row_h );

						if ( over ( token_area ) ) {

							hand = true;

							if ( double_id == id )
								follow_target = target;

						}

						break;

					}

					case line_kind::VTABLE: {

						uint64_t target = item.stride == 8 ? file.read64 ( offset_at ) : file.read32 ( offset_at );
						base::writer value;

						if ( symbol_name ( target, true, name, 200 ) )
							value.add ( name );
						else
							value.hex ( target );

						c.text ( item.stride == 8 ? L"dq" : L"dd", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::FUNCTION ), render::align::LEFT, ops_room );
						c.text ( base::writer ( ).add ( L"; slot " ).number ( sub ).text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						math::rect token_area ( x_ops, y, static_cast < float > ( value.size ( ) ) * adv, row_h );

						if ( over ( token_area ) ) {

							hand = true;

							if ( double_id == id )
								follow_target = target;

						}

						break;

					}

					case line_kind::LABEL: {

						base::writer label;
						const wchar_t* note = nullptr;

						if ( item.ref >= 0 ) {

							label.add ( names [ static_cast < size_t > ( item.ref ) ].get ( ) );
							note = item.function ? L"; function" : L"; exported data";

						} else if ( symbol_name ( item.address, false, name, 200 ) )
							label.add ( name );
						else
							label.add ( L"g_" ).hex ( item.address );

						label.add ( L':' );

						float label_w = c.text ( label.text ( ), render::font::MONO_STRONG, math::vec2 ( x_bytes, cy ), ink ( tone::GLOBAL ) );
						int32_t first_ref = 0;
						int32_t ref_count = 0;
						base::writer refs;

						if ( analyzed )
							db->refs_to ( item.address, first_ref, ref_count );

						if ( ref_count ) {

							int32_t writes = 0;

							for ( int32_t i = 0; i < ref_count; i++ ) {

								analysis::ref type = db->refs [ static_cast < size_t > ( first_ref + i ) ].type;

								if ( type == analysis::ref::WRITE )
									writes++;

							}

							refs.add ( L"\x2190 " ).number ( ref_count ).add ( ref_count == 1 ? L" ref" : L" refs" );

							if ( writes )
								refs.add ( L", " ).number ( writes ).add ( writes == 1 ? L" write" : L" writes" );

							note = refs.text ( );

						}

						const wchar_t* user = user_text ( comments, item.address );

						if ( user )
							note = user;

						if ( note )
							c.text ( note, render::font::MONO, math::vec2 ( base::max ( x_note, x_bytes + label_w + adv * 2.0f ), cy ), ink ( user ? tone::COMMENT : tone::INK_FAINT ) );

						break;

					}

					case line_kind::DATA: {

						base::writer bytes;
						uint64_t end = item.address + item.size;
						uint32_t take = static_cast < uint32_t > ( base::min ( static_cast < uint64_t > ( item.stride ), end > address ? end - address : 0 ) );

						for ( uint32_t b = 0; b < take && mapped; b++ ) {

							uint8_t value = file.byte ( offset_at + b );

							if ( b )
								bytes.add ( L' ' );

							bytes.hex ( value, 2 );

						}

						c.text ( bytes.text ( ), render::font::MONO, math::vec2 ( x_bytes, cy ), ink ( tone::BYTES ) );

						break;

					}

					case line_kind::STRING: {

						const loader::found_string& found = file.strings [ static_cast < size_t > ( item.ref ) ];
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

						value.add ( L"\", 0" );
						c.text ( found.wide ? L"du" : L"db", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::STRING ), render::align::LEFT, area.right ( ) - x_ops - 12.0f );

						break;

					}

					case line_kind::POINTER: {

						const loader::import& entry = file.imports [ static_cast < size_t > ( item.ref ) ];
						const loader::module& from = file.modules [ static_cast < size_t > ( entry.owner ) ];
						base::writer label;
						base::writer note;

						if ( entry.by_ordinal || entry.name.empty ( ) )
							label.add ( L"#" ).number ( entry.ordinal );
						else
							label.add ( entry.name.get ( ) );

						note.add ( L"; import from " ).add ( from.name.get ( ) );
						c.text ( file.wide ( ) ? L"dq" : L"dd", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( label.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::IMPORT ) );
						c.text ( note.text ( ), render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						break;

					}

					case line_kind::EMPTY: {

						base::writer value;

						value.grouped ( item.size ).add ( L" bytes" );
						c.text ( L"reserve", render::font::MONO, math::vec2 ( x_mnem, cy ), ink ( tone::MNEMONIC ) );
						c.text ( value.text ( ), render::font::MONO, math::vec2 ( x_ops, cy ), ink ( tone::NUMBER ) );
						c.text ( L"; uninitialized, not in the file", render::font::MONO, math::vec2 ( x_note, cy ), ink ( tone::COMMENT ) );

						break;

					}

					default:
						break;

				}

			}

		}

		c.pop_clip ( );

		if ( hand )
			wanted_cursor = pointer::HAND;

		if ( follow_target )
			go ( follow_target, true );
		else if ( double_id == id && over ( area ) )
			run ( command::FOLLOW );

		draw_scrollbar ( make_id ( "listing_bar", drawing_serial ), area, now->listing_scroll, static_cast < float > ( total_rows ) + 3.0f, visible );

	}

}