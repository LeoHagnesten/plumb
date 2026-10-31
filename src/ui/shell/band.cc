#include "ui/app.hh"
#include "ui/icons.hh"
#include <windows.h>

namespace ui {

	static render::color band_color ( const app& owner, loader::kind type, const render::color* tones ) {

		switch ( type ) {

			case loader::kind::CODE:
				return tones [ static_cast < int32_t > ( tone::CODE_BAND ) ];

			case loader::kind::CONSTANT:
				return tones [ static_cast < int32_t > ( tone::CONST_BAND ) ];

			case loader::kind::EMPTY:
				return tones [ static_cast < int32_t > ( tone::EMPTY_BAND ) ];

			case loader::kind::HEADER:
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

		for ( const loader::section& part : file.sections ) {

			float x0 = x_of ( part.address );
			float x1 = x_of ( part.address + base::max ( part.size, part.raw ) );

			c.fill ( math::rect ( x0, area.y, base::max ( x1 - x0 - 1.0f, 1.5f ), area.h ), 1.0f, band_color ( *this, part.type, tones ) );

		}

		int32_t ticks = 0;

		for ( const loader::symbol& function : file.functions ) {

			if ( ticks++ > 3000 )
				break;

			c.fill ( math::rect ( x_of ( function.address ) - 0.5f, area.y, 1.0f, 3.0f ), 0.0f, ink ( tone::LABEL ) );

		}

		float marker = x_of ( now->cursor_address );

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

}