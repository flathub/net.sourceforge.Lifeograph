/***********************************************************************************

    Copyright (C) 2007-2024 Ahmet Öztürk (aoz_2@yahoo.com)

    This file is part of Lifeograph.

    Lifeograph is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lifeograph is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lifeograph.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************************/


#include <cmath>
#include <cairomm/context.h>

#include "../strings.hpp"
#include "../lifeograph.hpp"
#include "../diaryelements/diary.hpp"
#include "calendar.hpp"


using namespace LIFEO;

// CHART ===========================================================================================
Calendar::Calendar()
{
    m_font_base.set_family( "Sans" );
    m_font_base.set_size( s_font_height * Pango::SCALE );

    m_font_bold = m_font_base;
    m_font_bold.set_weight( Pango::Weight::BOLD );

    m_font_big = m_font_bold;
    m_font_big.set_size( 1.3 * s_font_height * Pango::SCALE );
}

void
Calendar::set_date( DateV date )
{
    m_month = Date::isolate_YM( date );
    Date::set_day( m_month, 1 );

    refresh();
}

void
Calendar::calculate_begin_date( DateV& begin_date )
{
    Date::set_day( begin_date, 1 );
    const auto days_before{ ( Date::get_weekday( begin_date ) + 7 - Date::s_week_start_day ) % 7 };
    if( days_before > 0 )
    {
        Date::backward_months( begin_date, 1 ); // sets to last day of the month
        Date::set_day( begin_date, Date::get_day( begin_date ) - days_before + 1 );
    }
}

void
Calendar::update_col_geom()
{
    m_col_width = ( m_width - ( 2 * s_margin )
                            - ( Diary::d->get_boolean_option( VT::DO::SHOW_WEEK_NOS::I ) ?
                                ( m_unit_text_w * 2 ) + ( s_border_label * 2 ) : 0.0 ) ) / 7;
    m_row_height = ( m_height - s_margin - s_grid_start_y ) / 6;
}

void
Calendar::resize( int w, int h )
{
    //bool flag_first( m_width < 0 );

    m_width = w;
    m_height = h;

    m_x_max = m_width - s_margin;
    m_length = m_x_max - s_margin;
    m_y_max = m_height - s_margin;

    update_col_geom();
}

bool
Calendar::draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
    // BACKGROUND
    cr->rectangle( 0.0, 0.0, m_width, m_height );
    cr->set_source_rgb( 1.0, 1.0, 1.0 );
    cr->fill();

    // HANDLE DATE NOT SET-CASE SPECIALLY
    if( ! Date::is_set( m_month ) )
    {
        cr->set_source_rgb( 0.0, 0.0, 0.0 );
        auto layout = Pango::Layout::create( cr );
        layout->set_text( _( "DATE IS NOT SET" ) );
        layout->set_font_description( m_font_big );
        int w, h;
        layout->get_pixel_size( w, h );
        cr->move_to( ( m_width - w ) / 2.0 , m_height / 2.0 );
        layout->show_in_cairo_context( cr );

        return true;
    }

    // TITLE
    cr->set_source_rgb( 0.2, 0.1, 0.1 );
    auto layout_title = Pango::Layout::create( cr );
    layout_title->set_text( Date::format_string_adv( m_month, "n   Y" ) );
    layout_title->set_font_description( m_font_big );
    int w, h;
    layout_title->get_pixel_size( w, h );
    cr->move_to( ( m_width - w ) / 2.0, s_margin / 2.0 );
    layout_title->show_in_cairo_context( cr );

    // GRID
//    cr->set_line_width( 0.5 );
//    cr->set_source_rgb( 0.5, 0.5, 0.5 );
//
//    for( int i = 0; i <= 7; i++ )
//    {
//        cr->move_to( s_margin + m_col_width * i, s_grid_start_y );
//        cr->line_to( s_margin + m_col_width * i, m_y_max );
//    }
//
//    for( int i = 0; i <= 6; i++ )
//    {
//        cr->move_to( s_margin, s_grid_start_y + m_row_height * i );
//        cr->line_to( m_x_max, s_grid_start_y + m_row_height * i );
//    }

    cr->stroke();

    // ARROWS
    cr->save();
    Gdk::Cairo::set_source_pixbuf(
            cr,
            Lifeograph::icons->go_prev,
            s_margin, s_margin );
    cr->paint();
    Gdk::Cairo::set_source_pixbuf(
            cr,
            Lifeograph::icons->go_home,
            s_margin + 30, s_margin );
    cr->paint();
    Gdk::Cairo::set_source_pixbuf(
            cr,
            Lifeograph::icons->go_next,
            s_margin + 60, s_margin );
    cr->paint();

    cr->set_source_rgb( 0.5, 0.3, 0.3 );

    if     ( m_day_hovered == Date::BCK_MON )
    {
        cr->rectangle( s_margin - 2, s_margin - 2, 20, 20 );
        cr->stroke();
    }
    else if( m_day_hovered == Date::THS_MON )
    {
        cr->rectangle( s_margin + 28, s_margin - 2, 20, 20 );
        cr->stroke();
    }
    else if( m_day_hovered == Date::FWD_MON )
    {
        cr->rectangle( s_margin + 58, s_margin - 2, 20, 20 );
        cr->stroke();
    }

    cr->restore();

    // DAY NUMBERS & OTHER CELL CONTENT
    auto&&      layout  { Pango::Layout::create( cr ) };
    DateV       date    { m_month };
    const auto  today   { Date::get_today() };

    calculate_begin_date( date );

    layout->set_font_description( m_font_big );
    layout->set_text( "1" );
    layout->get_pixel_size( w, h ); // store height

    for( int j = 0; j < 6; j++ )
    {
        for( int i = 0; i < 7; i++ )
        {
            // HOVER AND BG COLOR
            auto milestone{ Diary::d->get_milestone_before( date ) };

            //if( milestone || date.m_date == m_day_hovered || date.m_date == m_day_highlighted )
            {
                cr->rectangle( s_margin + i * m_col_width + 0.5,
                               s_grid_start_y + j * m_row_height + 0.5,
                               m_col_width - 1, m_row_height - 1 );

                if( date == m_day_hovered )
                    cr->set_source_rgb( 1.0, 0.91, 0.91 );
                else if( milestone )
                    Gdk::Cairo::set_source_rgba(
                        cr, midtone( milestone->get_color(), Color( "#FAFAFA" ), 0.8 ) );
                else
                    cr->set_source_rgb( 0.93, 0.93, 0.93 );

                cr->fill_preserve();

                if( date == m_day_highlighted )
                {
                    cr->set_source_rgb( 0.5, 0.3, 0.3 );
                    cr->set_line_width( 1.5 );
                    cr->stroke();
                }
                else
                    cr->begin_new_path(); // clears the path
            }

            // TODAY MARK
            if( date == today )
            {
                cr->set_source_rgb( 0.87, 0.5, 0.5 );

                cr->move_to( s_margin + i * m_col_width + 8.0,
                             s_grid_start_y + j * m_row_height + 3.0 );
                cr->rel_line_to( -5.0, 0.0 );
                cr->rel_line_to( 0.0, m_row_height - 6.0 );
                cr->rel_line_to( 5.0, 0.0 );

                cr->move_to( s_margin + ( i + 1 ) * m_col_width - 8.0,
                             s_grid_start_y + j * m_row_height + 3.0 );
                cr->rel_line_to( 5.0, 0.0 );
                cr->rel_line_to( 0.0, m_row_height - 6.0 );
                cr->rel_line_to( -5.0, 0.0 );
                cr->stroke();
            }

            // DAY NUMBER
            if( Date::get_month( date ) == Date::get_month( m_month ) )
                layout->set_font_description( m_font_big );
            else
                layout->set_font_description( m_font_bold );

            if( Diary::d->is_day_weekend( i + Date::s_week_start_day ) ||
                Diary::d->is_day_holiday( date ) )
                cr->set_source_rgb( 0.8, 0.7, 0.7 );
            else
                cr->set_source_rgb( 0.5, 0.3, 0.3 );

            layout->set_text( STR::compose( Date::get_day( date ) ) );
            cr->move_to( s_margin + i * m_col_width + s_border_label,
                         s_grid_start_y + ( j + 0.05 ) * m_row_height ); // 0.05 = 5% margin
            layout->show_in_cairo_context( cr );

            // ENTRY ICON
            auto entry_count{ Diary::d->get_entry_count_on_day( date ) };
            if( entry_count > 0 )
            {
                if( date == m_day_shown )
                {
                    cr->set_line_width( 1.5 );
                    cr->begin_new_path();
                    cr->arc( s_margin + ( i + 0.4 ) * m_col_width + 8,
                             s_grid_start_y + ( j + 0.5 ) * m_row_height,
                             12.0, 0.0, 2 * HELPERS::PI );
                    cr->stroke();
                }

                Gdk::Cairo::set_source_pixbuf( cr,
                                       entry_count > 1 ? Lifeograph::icons->entry_plus_16 :
                                                         Lifeograph::icons->entry_16,
                                       s_margin + ( i + 0.4 ) * m_col_width,
                                       s_grid_start_y + ( j + 0.5 ) * m_row_height - 8 );
                cr->paint();
            }

            Date::forward_days( date, 1 );
        }
    }

    // DAY NAMES
    layout->set_font_description( m_font_bold );
    layout->set_width( ( m_col_width - 5 ) * Pango::SCALE );
    layout->set_ellipsize( Pango::EllipsizeMode::END );
    cr->move_to( s_margin + s_border_label, s_margin + s_title_bar_height );
    for( int i = 0; ; i++ )
    {
        if( m_F_editable && i == m_column_hovered )
            cr->set_source_rgb( 0.4, 0.2, 0.2 );
        else
            cr->set_source_rgb( 0.5, 0.4, 0.4 );
        layout->set_text( Date::get_day_name( i + Date::s_week_start_day ) );
        layout->show_in_cairo_context( cr );

        if( i == 6 ) break;

        cr->rel_move_to( m_col_width, 0.0 );
    }

    // WEEK NUMBERS
    if( Diary::d->get_boolean_option( VT::DO::SHOW_WEEK_NOS::I ) )
    {
        cr->set_source_rgb( 0.5, 0.4, 0.4 );
        cr->move_to( s_margin + 7 * m_col_width + s_border_label,
                     s_grid_start_y + 0.05 * m_row_height );  // 0.05 = 5% margin
        date = m_month;
        for( int j = 0; ; j++ )
        {
            layout->set_text( Date::format_string_adv( date, "W" ) );
            layout->show_in_cairo_context( cr );

            if( j == 5 ) break;

            cr->rel_move_to( 0.0, m_row_height );
            Date::forward_days( date, 7 );
        }
    }

    return true;
}
