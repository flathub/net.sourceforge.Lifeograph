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

#include "../lifeograph.hpp"
#include "src/helpers.hpp"
#include "table.hpp"


using namespace LIFEO;

// TABLE ===========================================================================================
//const std::valarray< double > Table::s_dash_pattern = { 3.0 };
const std::valarray< double > Table::clrR = { 1.00, 0.92, 0.87, 0.80, 0.72, 0.65 };
const std::valarray< double > Table::clrG = { 0.60, 0.70, 0.75, 0.80, 0.90, 1.00 };

Table::Table( const Pango::FontDescription& fd )
{
    m_font_main = fd;
    m_font_header = fd;
    m_font_gantt = fd;

    m_font_header.set_weight( Pango::Weight::BOLD );

    m_font_gantt.set_size( fd.get_size() * 0.7 );
}

void
Table::calculate_row_h()
{
    m_layout->set_font_description( m_font_header );
    m_layout->set_text( "LLL" );
    m_row_h_net = m_layout->get_pixel_logical_extents().get_height();

    // UPDATE THE MARGINS
    m_cell_margin   = ( m_row_h_net * 0.1 );
    m_cell_marg_ttl = ( m_cell_margin * 2 );
    m_comb_margin   = ( S_MARGIN + m_cell_margin );
    m_x_offset_cell = m_comb_margin;
    m_y_offset_cell = m_comb_margin;

    m_row_h = m_row_h_net + m_cell_marg_ttl;

    m_unit_line_thk = ( m_row_h / 25 );
}

void
Table::update_col_geom()
{
    if( m_data.m_columns.empty() ) return;

    double w_cumulative{ 0.0 };

    m_col_widths.clear();
    m_col_widths_cum.clear();
    for( auto& col : m_data.m_columns )
    {
        m_col_widths.push_back( col->m_width * m_width_net );
        m_col_widths_cum.push_back( w_cumulative ); // starts with 0
        w_cumulative += m_col_widths.back();
    }
    m_col_widths_cum.push_back( m_width_net ); // ends with net width
}

void
Table::update_height_vline()
{
    m_height_vline = std::min( m_height_net,
                               m_row_h * ( m_data.m_lines.size() - m_i_line_top + 1 ) );
                               // +1 for the header
}

void
Table::update_line_c_vis()
{
    // -2 for the header & sum:
    m_line_c_max = static_cast< unsigned int >( m_height_net / m_row_h - 2 );
    m_line_c_act = std::min( static_cast< unsigned int >( m_data.m_lines.size() - m_i_line_top ),
                             m_line_c_max );
}

void
Table::resize( int w, int h )
{
    m_width = w;
    m_height = h;

    m_width_net = w - 2 * S_MARGIN;
    m_height_net = h - 2 * S_MARGIN;
}

void
Table::scroll( int offset )
{
    if( offset < 0 && ( m_i_line_top + offset ) >= 0 )
        m_i_line_top += offset;
    else if( offset > 0 &&
             ( m_i_line_top + offset ) <=
                    int( m_data.m_lines.size() - m_line_c_max + S_MIN_LINES ) )
        m_i_line_top += offset;
    else
        return;

    update_line_c_vis();
    update_height_vline();
    refresh();
}

inline void
Table::draw_group_bg( const Cairo::RefPtr< Cairo::Context >& cr, int il )
{
    cr->save();

    cr->set_source_rgb( 1.0, 0.9, 0.8 );
    cr->rectangle( m_x_offset, m_y_offset + ( il + 1 ) * m_row_h, m_width_net, m_row_h );
    cr->fill();

    cr->set_source_rgb( 0.2, 0.1, 0.1 );
    cr->set_line_width( 2 * m_unit_line_thk );
    cr->move_to( m_x_offset, m_y_offset + ( il + 1 ) * m_row_h );
    cr->rel_line_to( m_width_net, 0 );
    cr->stroke();

    cr->restore();
}

inline void
Table::draw_cell_progress( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    const auto   i_line_abs { m_i_line_top + il };
    // TODO: 3.1: migrate to std::clamp after C++17 upgrade:
    const double comp{ std::max( 0.0,
                       std::min( 1.0, m_data.get_value( i_line_abs, ic ) /
                                      m_data.get_weight( i_line_abs, ic ) ) ) };
    const int    clr_i{ int( comp * 5 ) };

    cr->set_source_rgb( clrR[ clr_i ], clrG[ clr_i ], 0.6 );

    cr->rectangle( m_x_offset_cell + m_col_widths_cum[ ic ],
                   m_y_offset_cell + ( il + 1 ) * m_row_h,
                   comp == 0.0 ? 2.0 : ( m_col_widths[ ic ] - m_cell_marg_ttl ) * comp,
                   m_row_h_net );
    cr->fill();

    cr->set_source_rgb( 0.0, 0.0, 0.0 );

    draw_cell_text_main( cr, il, ic, m_data.get_value_str( il + m_i_line_top, ic ) );
}

inline void
Table::draw_cell_text_main( const Cairo::RefPtr< Cairo::Context >& cr,
                            int il, int ic, const Ustring& text )
{
    m_layout->set_text( text );
    cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ],
                 m_y_offset_cell + ( il + 1 ) * m_row_h );
    m_layout->show_in_cairo_context( cr );
}

inline void
Table::draw_cell_text_extra( const Cairo::RefPtr< Cairo::Context >& cr,
                             int il, int ic, const Ustring& text )
{
    cr->save();
    auto&& alignment_old{ m_layout->get_alignment() };

    m_layout->set_font_description( m_font_main );
    m_layout->set_alignment( alignment_old == Pango::Alignment::LEFT ? Pango::Alignment::RIGHT
                                                                     : Pango::Alignment::LEFT );
    cr->set_source_rgb( 0.45, 0.45, 0.45 );

    m_layout->set_markup( text );
    cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ],
                 m_y_offset_cell + ( il + 1 ) * m_row_h );
    m_layout->show_in_cairo_context( cr );
    pango_layout_set_attributes( m_layout->gobj(), nullptr ); // needed after set_markup

    m_layout->set_alignment( alignment_old );
    cr->restore();
}

inline void
Table::draw_cell_text( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    const auto col{ m_data.m_columns[ ic ] };

    if( !( col->is_combine_same() && m_data.is_col_value_same( il + m_i_line_top, ic ) ) ||
        il == 0 )
    {
        const auto&& color { col->get_color( m_data.get_value( il + m_i_line_top, ic ) ) };
        cr->set_source_rgb( color.r, color.g, color.b );
        draw_cell_text_main( cr, il, ic, m_data.get_value_str( il + m_i_line_top, ic ) );
    }
    else
        m_line_breaks.insert( ( il + 1 ) * m_data.m_columns.size() + ic );
}

inline void
Table::draw_group_size( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    draw_cell_text_extra( cr, il, ic,
                          STR::compose( "(",
                                        m_data.m_lines[ il + m_i_line_top ]->m_sublines.size(),
                                        ")" ) );
}

inline void
Table::draw_sort_arrow( const Cairo::RefPtr< Cairo::Context >& cr,
                        TableColumn* col )
{
    draw_cell_text_extra( cr, -1, col->m_index,
                          STR::compose( col->m_sort_desc ? "↑" : "↓",
                                        m_data.m_columns_sort.size() > 1 ?
                                        STR::compose( "<sup>", col->m_sort_order, "</sup>" ) :
                                        "" ) );
}

inline void
Table::draw_cell_todo_status( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    R2Pixbuf        buf;
    const double    half_w{ m_row_h_net / 2.0 };

    switch( int( m_data.get_value( il + m_i_line_top, ic ) ) )
    {
        case ES::PROGRESSED: buf = Lifeograph::icons->todo_progressed_16; break;
        case ES::DONE:       buf = Lifeograph::icons->todo_done_16; break;
        case ES::CANCELED:   buf = Lifeograph::icons->todo_canceled_16; break;
        case ES::TODO:       buf = Lifeograph::icons->todo_open_16; break;
        default:             return;
    }

    buf = buf->scale_simple( m_row_h_net, m_row_h_net, Gdk::InterpType::BILINEAR );

    cr->save();
    Gdk::Cairo::set_source_pixbuf( cr, buf,
                    m_x_offset + m_col_widths_cum[ ic ] + m_col_widths[ ic ] / 2 - half_w,
                    m_y_offset + ( il + 1.5 ) * m_row_h - half_w );
    cr->paint();

    cr->restore();
}

inline void
Table::draw_cell_gantt( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    cr->save();

    const auto   col_w_net  { m_col_widths[ ic ] - m_cell_marg_ttl };
    const auto   p2elem     { m_data.m_lines[ il + m_i_line_top ]->m_p2elem };
    const auto&  segments   { m_data.m_lines[ il + m_i_line_top ]->m_periods };
    const auto&& color      { p2elem ? p2elem->get_color() : Color( "#AAAAAA" ) };

    for( auto segment : segments )
    {
        cr->set_source_rgb( 0.1, 0.1, 0.1 );
        cr->rectangle( m_x_offset_cell + m_col_widths_cum[ ic ] + col_w_net * segment.x,
                       m_y_offset_cell + ( il + 1 ) * m_row_h + ( m_row_h_net / 4.0 ),
                       col_w_net * segment.w,
                       m_row_h_net / 2.0 );
        cr->stroke_preserve();

        Gdk::Cairo::set_source_rgba( cr, color );
        cr->fill();
    }

    cr->restore();
}

inline void
Table::draw_cell_bool( const Cairo::RefPtr< Cairo::Context >& cr, int il, int ic )
{
    const auto v{ m_data.get_value( il + m_i_line_top, ic ) };
    if( v != 0.0 )
    {
        cr->save();

        const auto   col_w_net  { m_col_widths[ ic ] - m_cell_marg_ttl };
        // const auto   p2elem     { m_data.m_lines[ il + m_i_line_top ]->m_p2elem };
        // const auto&& color      { p2elem ? p2elem->get_color() : Color( "#AAAAAA" ) };

        if( v == 1 )
            cr->set_source_rgb( 0.1, 0.1, 0.1 );
        else
            cr->set_source_rgb( 0.6, 0.6, 0.6 );

        cr->arc( m_x_offset_cell + m_col_widths_cum[ ic ] + col_w_net/2,
                m_y_offset_cell + ( il + 1 ) * m_row_h + ( m_row_h_net / 2.0 ),
                m_row_h_net / 3.0, 0.0, 2 * HELPERS::PI );
        cr->fill();

        cr->restore();
    }
}

inline void
Table::draw_header_std( const Cairo::RefPtr< Cairo::Context >& cr, TableColumn* col )
{
    cr->save();

    m_layout->set_width( ( m_col_widths[ col->m_index ] - m_cell_marg_ttl ) * Pango::SCALE );
    if( col->m_sort_order > 0 )
        draw_sort_arrow( cr, col );
    m_layout->set_font_description( m_font_header );
    draw_cell_text_main( cr, -1, col->m_index, col->get_name() );

    cr->restore();
}

inline void
Table::draw_header_gantt( const Cairo::RefPtr< Cairo::Context >& cr, int ic )
{
    const int       period      { int( m_data.m_master_header.m_weights[ ic ] ) };
    DateV           date        { m_data.m_date_bgn };
    DateV           date_prev   { Date::NOT_SET };
    int             hday_cnt    { 0 };
    const double    day_w       { ( m_col_widths[ ic ] - m_cell_marg_ttl ) / period };
    const int       step_type   { day_w >= 10.0 ? 1 // daily
                                    : ( day_w >= 1.0 ? 7 // weekly
                                        : ( day_w >= 0.3 ? 30 // monthly
                                            : ( day_w >= 0.025 ? 365 : 0 ) ) ) };
    int             step_first;
    int             step_size   { 1 };
    int             text_w      { 0 };
    int             text_skp    { 0 };
    double          v_ln_offset { m_y_offset + m_row_h * 0.5 };

    switch( step_type )
    {
        case 0: // no subdivision
            return;
        case 1: // daily
            step_first = 0;
            break;
        case 7: // weekly
            step_size = 7;
            step_first = ( 7 + Date::s_week_start_day - Date::get_weekday( date ) ) % 7;
            break;
        case 30: // monthly
            step_first = ( Date::get_day( date ) == 1 ? 0 :
                           1 + Date::get_days_in_month( date ) - Date::get_day( date ) );
            break;
        case 365: // yearly
            step_first = ( Date::get_yearday( date ) == 1 ? 0 :
                           1 + Date::get_days_in_year( date ) - Date::get_yearday( date ) );
            break;
    }

    Date::forward_days( date, step_first );

    cr->save();

    m_layout->set_width( ( m_col_widths[ ic ] - m_cell_marg_ttl ) * Pango::SCALE );
    m_layout->set_font_description( m_font_gantt );

    for( int i = step_first; i < period; i += step_size )
    {
        // HOLIDAYS (not just for the header but the whole table)
        if( Diary::d->is_day_weekend( Date::get_weekday( date ) ) ||
            Diary::d->is_day_holiday( date ) )
        {
            hday_cnt++;

            // TODO: check last visible day being a holiday case
        }
        else if( hday_cnt > 0 )
        {
            cr->set_source_rgba( 0.3, 0.3, 0.3, 0.15 );
            cr->rectangle( m_x_offset_cell + m_col_widths_cum[ ic ] + ( i - hday_cnt ) * day_w,
                           m_y_offset,
                           day_w * hday_cnt,
                           m_height_vline );
            cr->fill();
            hday_cnt = 0;
        }

        // VERTICAL LINES (not just for the header but the whole table)
        cr->set_source_rgb( 0.75, 0.75, 0.75 );

        cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ] + i * day_w, v_ln_offset );
        cr->rel_line_to( 0.0, m_height_vline - v_ln_offset + m_y_offset );

        cr->stroke();

        // TEXT
        if( text_skp <= 0 &&
            ( text_w < ( m_col_widths[ ic ] - m_cell_marg_ttl - i * day_w ) ) )
        {
            cr->set_source_rgb( 0.0, 0.0, 0.0 );

            if( Date::get_year( date ) != Date::get_year( date_prev ) )
            {
                cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ] + i * day_w,
                             m_y_offset_cell );
                m_layout->set_text( std::to_string( Date::get_year( date ) ) );
                m_layout->show_in_cairo_context( cr );
            }
            else if ( Date::get_month( date ) != Date::get_month( date_prev ) )
            {
                cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ] + i * day_w,
                             m_y_offset_cell );
                m_layout->set_text( Date::get_month_name( date ).substr( 0, 3 ) );
                m_layout->show_in_cairo_context( cr );
            }

            cr->move_to( m_x_offset_cell + m_col_widths_cum[ ic ] + i * day_w,
                         m_y_offset_cell + ( m_row_h_net / 2.0 ) );

            switch( step_type )
            {
                case 1:
                case 7:
                    m_layout->set_text( std::to_string( Date::get_day( date ) ) );
                    break;
                case 30:
                    m_layout->set_text( std::to_string( Date::get_month( date ) ) );
                    break;
                case 365:
                    m_layout->set_text( Date::get_month_name( date ).substr( 0, 3 ) );
                    break;
            }

            m_layout->get_pixel_size( text_w, text_skp ); // text_v is a placeholder here
            m_layout->show_in_cairo_context( cr );

            text_skp = floor( text_w / ( day_w * step_size - 5.0 ) ); // 5 is the label margin
            date_prev = date;
            v_ln_offset = ( m_y_offset + m_row_h * 0.5 );
        }
        else
        {
            text_skp--;
            v_ln_offset = ( m_y_offset + m_row_h );
        }

        // STEP FORWARD
        switch( step_type )
        {
            case 30: // monthly
                step_size = Date::get_days_in_month( date ); break;
            case 365: // yearly
                step_size = Date::get_days_in_year( date ); break;
        }

        Date::forward_days( date, step_size );
    }

    cr->restore();
}

bool
Table::draw( const Cairo::RefPtr< Cairo::Context >& cr )
{
    if( !m_data.m_p2diary->is_open() ) return true;

    // BACKGROUND
    if( !m_F_printing_mode )
    {
        cr->rectangle( 0.0, 0.0, m_width, m_height );
        cr->set_source_rgb( 1.0, 1.0, 1.0 );
        cr->fill();
    }

    m_layout->set_font_description( m_font_header );

    // HANDLE NOTHING TO DRAW-CASE SPECIALLY
    if( m_data.m_columns.empty() )
    {
        m_layout->set_text( _( "ADD COLUMNS" ) );
        m_layout->set_alignment( Pango::Alignment::CENTER );
        m_layout->set_width( m_width * Pango::SCALE );

        cr->set_source_rgb( 0.0, 0.0, 0.0 );
        cr->move_to( 0, m_height / 2.0 );
        m_layout->show_in_cairo_context( cr );

        return true;
    }

    // HEADER BG FILL
    cr->rectangle( m_x_offset, m_y_offset, m_width_net, m_row_h );
    cr->set_source_rgb( 0.93, 0.93, 0.93 );
    cr->fill();

    // GROUP HEADER BG FILLS
    if( m_data.m_grouping_depth > 0 )
        for( unsigned int il = 0 ; il < m_line_c_act; il++ )
        {
            if( m_data.m_lines[ il + m_i_line_top ]->is_group_head() )
                draw_group_bg( cr, il );
        }

    // HOVERED HEADER
    if( m_i_col_cur >= 0 && m_i_line_hover == 0 && m_data.m_p2diary->is_in_edit_mode() &&
        m_data.m_columns[ m_i_col_cur ]->m_type != TableColumn::TCT_ORDER )
    {
        cr->rectangle( m_x_offset + m_col_widths_cum[ m_i_col_cur ], m_y_offset,
                       m_col_widths[ m_i_col_cur ], m_row_h );
        cr->set_source_rgb( 0.92, 0.82, 0.82 );
        cr->fill();
    }
    // HOVERED LINE
    else if( m_i_line_hover > 0 )
    {
        cr->rectangle( m_x_offset, m_y_offset + m_i_line_hover * m_row_h, m_width_net, m_row_h );
        cr->set_source_rgb( 0.92, 0.82, 0.82 );
        cr->fill();
    }

    // CURRENT ENTRY LINE
    if( m_i_line_cur > 0 &&
        m_i_line_cur != ( m_i_line_hover + m_i_line_top ) &&
        m_i_line_cur >= m_i_line_top &&
        m_i_line_cur <= int( m_line_c_act + m_i_line_top ) )
    {
        cr->rectangle( m_x_offset, m_y_offset + ( m_i_line_cur - m_i_line_top ) * m_row_h,
                       m_width_net, m_row_h );
        cr->set_source_rgb( 0.96, 0.9, 0.9 );
        cr->fill();
    }

    // TEXT
    m_layout->set_ellipsize( Pango::EllipsizeMode::END );

    // COLUMN HEADERS
    cr->set_source_rgb( 0.0, 0.0, 0.0 );
    for( auto& col : m_data.m_columns )
    {
        m_layout->set_alignment( col->get_alignment() );

        if( col->m_type == TableColumn::TCT_GANTT )
            draw_header_gantt( cr, col->m_index );
        else
            draw_header_std( cr, col );
    }

    // CELLS
    const auto ic_sort   { ( * m_data.m_columns_sort.begin() )->m_index };
    const auto col_count { m_data.m_columns.size() };

    m_line_breaks.clear();

    for( ListTableColumns::size_type ic = 0; ic < col_count; ic++ )
    {
        m_layout->set_width( ( m_col_widths[ ic ] - m_cell_marg_ttl ) * Pango::SCALE );
        m_layout->set_alignment( m_data.m_columns[ ic ]->get_alignment() );

        for( unsigned int il = 0 ; il < m_line_c_act; il++ )
        {
            const bool F_head_sort_cell{ m_data.m_grouping_depth > 0 &&
                                         m_data.m_lines[ il + m_i_line_top ]->is_group_head() &&
                                         int( ic ) == ic_sort };

            m_layout->set_font_description( F_head_sort_cell ? m_font_header : m_font_main );

            switch( m_data.m_columns[ ic ]->m_type )
            {
                case TableColumn::TCT_COMPLETION:  draw_cell_progress( cr, il, ic ); break;
                case TableColumn::TCT_TODO_STATUS: draw_cell_todo_status( cr, il, ic ); break;
                case TableColumn::TCT_GANTT:       draw_cell_gantt( cr, il, ic ); break;
                case TableColumn::TCT_BOOL:        draw_cell_bool( cr, il, ic ); break;
                case TableColumn::TCT_ARITHMETIC:
                case TableColumn::TCT_TAG_V:
                    if( m_data.m_columns[ ic ]->is_percentage() )
                    {                              draw_cell_progress( cr, il, ic ); break; }
                    // else no break
                default:                           draw_cell_text( cr, il, ic ); break;
            }

            if( F_head_sort_cell )
                draw_group_size( cr, il, ic );
        }

        // TOTAL LINE (not printed at the end of intermediate pages when printing)
        if( !m_F_printing_mode || !has_more() )
        {
            cr->save();
            cr->set_source_rgb( 0.2, 0.2, 0.2 );
            m_layout->set_font_description( m_font_header );
            draw_cell_text_main( cr, m_line_c_act, ic, m_data.get_total_value_str( ic ) );
            cr->restore();

            if( m_data.m_grouping_depth > 0 && m_data.m_columns[ ic ]->m_sort_order == 1 )
                draw_cell_text_extra( cr, m_line_c_act, ic,
                                      STR::compose( "(", m_data.m_count_lines, ")" ) );
        }
    }

    // VERTICAL LINES
    cr->set_line_width( m_unit_line_thk );
    cr->set_source_rgb( 0.5, 0.4, 0.4 );

    for( ListTableColumns::size_type i = 0; i < m_col_widths_cum.size(); i++ )
    {
        cr->move_to( m_x_offset + m_col_widths_cum[ i ], m_y_offset );
        cr->rel_line_to( 0, m_height_vline );
    }

    // HORIZONTAL LINES
    for( unsigned int il = 0; il <= m_line_c_act + 1; il++ )
    {
        const double line_y { m_y_offset + il * m_row_h };

        cr->move_to( m_x_offset, line_y );

        if( m_line_breaks.empty() ) // no line combination
            cr->rel_line_to( m_width_net, 0 );
        else
        {
            bool F_line_on { true };

            for( unsigned ic = 0; ic < col_count; ++ic )
            {
                // current cell is combined:
                if( m_line_breaks.find( ( il ) * col_count + ic ) == m_line_breaks.end() )
                {
                    if( !F_line_on )
                    {
                        cr->move_to( m_x_offset + m_col_widths_cum[ ic ], line_y );
                        F_line_on = true;
                    }
                }
                else if( F_line_on )
                {
                    cr->line_to( m_x_offset + m_col_widths_cum[ ic ], line_y );
                    F_line_on = false;
                }
            }

            if( F_line_on )
                cr->line_to( m_x_offset + m_width_net, line_y );
        }
    }

    cr->stroke();

    return true;
}
