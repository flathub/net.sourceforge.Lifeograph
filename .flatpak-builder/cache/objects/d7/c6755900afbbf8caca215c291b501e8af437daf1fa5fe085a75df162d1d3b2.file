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
#include "../diaryelements/filter.hpp"
#include "../lifeograph.hpp"
#include "chart.hpp"


using namespace LIFEO;

// CHART ===========================================================================================
const std::valarray< double > Chart::s_dash_pattern = { 3.0 };

Chart::Chart( const Pango::FontDescription& fd )
{
    m_font_main = fd;
    m_font_bold = fd;
    m_font_big = fd;

    m_font_bold.set_weight( Pango::Weight::BOLD );

    m_font_big.set_weight( Pango::Weight::BOLD );
    m_font_big.set_size( fd.get_size() * 1.5 );
}

inline DateV
Chart::get_period_date( DateV d )
{
    switch( m_data.m_properties & ChartData::PERIOD_MASK )
    {
        case ChartData::PERIOD_WEEKLY:  Date::backward_to_week_start( d ); break;
        case ChartData::PERIOD_MONTHLY: Date::backward_to_month_start( d ); break;
        case ChartData::PERIOD_YEARLY:  Date::backward_to_year_start( d ); break;
    }

    return d;
}

Ustring
Chart::get_date_str( DateV date ) const
{
    switch( m_data.m_properties & ChartData::PERIOD_MASK )
    {
        case ChartData::PERIOD_MONTHLY: return Date::format_string( date, "YM" );
        case ChartData::PERIOD_YEARLY:  return Date::get_year_str( date );
        // weekly & daily:
        default:                        return Date::format_string( date, "YMD" );
    }
}

void
Chart::set_zoom( float level )
{
    if( level != m_zoom_level )
        m_zoom_level = ( level > 1.0f ? 1.0f : ( level < 0.0f ) ? 0.0f : level );

    if( m_width > 0 ) // if on_size_allocate is executed before
    {
        update_col_geom( false );
        refresh();
    }
}

bool
Chart::is_zoom_possible() const
{
    return( !( m_step_count == m_data.m_span && m_step_x >= m_width_col_min ) &&
            m_data.get_type() == ChartData::TYPE_DATE );
}

double
find_nearest_grid( double base, double amplitude, int exp )
{
    if( amplitude == 0 ) return amplitude;

    const double step_5         { 5 * pow( 10, exp - 1 ) };
    const double step_10        { pow( 10, exp ) };
    const double rounded_out    { abs( std::fmod( base, step_10 ) ) };
    const double nearest_grid_0 { abs( rounded_out ) };
    const double nearest_grid_5 { abs( step_5 - rounded_out ) };
    const double nearest_grid_10{ abs( step_10 - rounded_out ) };

    double nearest_grid{ std::min( std::min( nearest_grid_0, nearest_grid_5 ),
                                   nearest_grid_10 ) };

    if( nearest_grid == nearest_grid_0 )
        nearest_grid = ( base - rounded_out );
    else if( nearest_grid == nearest_grid_10 )
        nearest_grid = ( base - rounded_out + step_10 );
    else
        nearest_grid = ( base - rounded_out + step_5 );

    if( abs( nearest_grid - base ) < amplitude / 10 )
        return nearest_grid;

    return find_nearest_grid( base, amplitude, exp - 1 );
}

void
Chart::update_col_geom( bool F_new )
{
    // 100% zoom:
    const bool F_style_line  { m_data.get_style() == ChartData::STYLE_LINE };
    const auto step_count_nom{ unsigned( m_length / m_width_col_min ) + ( F_style_line ? 1U
                                                                                       : 0U ) };
    const auto step_count_min{ m_data.m_span > step_count_nom ? step_count_nom : m_data.m_span };

    m_step_count = ( m_zoom_level * ( m_data.m_span - step_count_min ) ) + step_count_min;
    m_step_x = m_length / ( m_step_count - ( F_style_line ? 1 : 0 ) );

    m_ov_height = ( m_step_count < m_data.m_span ? ( m_height * 0.1 ) : 0.0 );

    m_y_max = m_height - m_h_x_values - m_ov_height;

    if( m_data.v_min < 0 )
        m_y_max -= m_label_height;

    m_ampli_y = m_y_max - m_y_min;

    const unsigned int col_start_max{ m_data.m_span - m_step_count };
    if( F_new || m_step_start > col_start_max )
        m_step_start = col_start_max;

    if( m_data.v_max != Constants::INFINITY_MNS ) // i.e. was initialized
    {
        const auto delta_v  { m_data.v_max - m_data.v_min };
        auto       grid_max { find_nearest_grid( m_data.v_max,
                                                 delta_v,
                                                 log10( abs( m_data.v_max ) ) ) };
        const auto grid_min { find_nearest_grid( m_data.v_min,
                                                 delta_v,
                                                 log10( abs( m_data.v_min ) ) ) };
        grid_max += ( ( grid_max - grid_min ) / Y_DIVISIONS_MAX );
        m_ampli_v = ( grid_max - grid_min );

        if( m_ampli_v == 0 )
            m_ampli_v = 1;
    }
    else
        m_ampli_v = 1;

    m_coefficient = m_ampli_y / m_ampli_v;
    m_y_0 = ( m_data.v_min < 0 ? ( m_y_max + m_data.v_min * m_coefficient ) : m_y_max );

    // OVERVIEW PARAMETERS
    m_ampli_ov = m_ov_height - 2 * m_border_label;
    m_coeff_ov = m_ampli_ov / m_ampli_v;
    m_step_x_ov = m_width - 2 * m_border_label;
    if( m_data.m_span > 1 )
        m_step_x_ov /= m_data.m_span - 1;
}

void
Chart::resize( int w, int h )
{
    const bool F_first{ m_width < 0 };

    m_width = w;
    m_height = h;

    if( m_layout )
        update_dims( F_first );
}

void
Chart::update_dims( const bool F_first )
{
    m_layout->set_font_description( m_font_main );
    m_layout->set_text( "LLL" );

    m_label_size      = m_layout->get_pixel_logical_extents().get_height();

    m_border_curve    = m_width * 0.06;
    m_border_label    = m_label_size * 0.075;

    m_width_col_min   = m_border_curve * 0.8;
    m_label_height    = m_label_size + 2 * m_border_label;

    m_x_min           = m_border_curve + m_border_label;
    m_x_min_bar       = m_x_min + m_width_col_min / 2;
    m_y_min           = m_label_height + m_label_size; // m_label_size is for margin

    m_x_max           = m_width - m_border_curve;
    m_length          = m_x_max - m_x_min;

    m_unit_line_thk   = ( m_label_size / 25 );

    update_h_x_values();

    update_col_geom( F_first );
}

void
Chart::update_h_x_values()
{
    if( m_data.get_type() == ChartData::TYPE_DATE )
    {
        if( ( m_data.m_properties & ChartData::PERIOD_MASK ) == ChartData::PERIOD_YEARLY )
            m_h_x_values = m_label_height * 1.5;
        else
            m_h_x_values = m_label_height * 3;
    }
    else
        m_h_x_values = m_label_height * 5;
}

void
Chart::scroll( int offset )
{
    //if( m_points )
    {
        if( offset < 0 && m_step_start > 0 )
            m_step_start--;
        else
        if( offset > 0 && m_step_start < ( m_data.m_span - m_step_count ) )
            m_step_start++;
        else
            return;

        refresh();
    }
}

inline void
Chart::draw_line( const Cairo::RefPtr< Cairo::Context >& cr )
{
    cr->set_source_rgb( 0.7, 0.4, 0.4 );
    cr->set_line_join( Cairo::Context::LineJoin::BEVEL );
    cr->set_line_width( m_unit_line_thk * 3 );

    draw_line2( cr, false );

    // UNDERLAY
    if( m_data.m_tcidu != ChartData::COLUMN_NONE )
    {
        cr->set_source_rgb( 1.0, 0.7, 0.7 );
        cr->set_dash( s_dash_pattern, 0 );
        cr->set_line_width( m_unit_line_thk * 2 );

        draw_line2( cr, true );

        cr->unset_dash();
    }
}
inline void
Chart::draw_line2( const Cairo::RefPtr< Cairo::Context >& cr, bool F_underlay )
{
    switch( m_data.get_type() )
    {
        case ChartData::TYPE_STRING:
            draw_line( cr, m_data.values_str, F_underlay );
            break;
        case ChartData::TYPE_NUMBER:
            draw_line( cr, m_data.values_num, F_underlay );
            break;
        case ChartData::TYPE_DATE:
            draw_line( cr, m_data.values_date, F_underlay );
            break;
    }
}

inline void
Chart::draw_bars( const Cairo::RefPtr< Cairo::Context >& cr )
{
    cr->set_source_rgb( 0.7, 0.4, 0.4 );
    cr->set_line_join( Cairo::Context::LineJoin::BEVEL );

    draw_bars2( cr, false );

    // UNDERLAY
    if( m_data.m_tcidu != ChartData::COLUMN_NONE )
    {
        cr->set_source_rgb( 1.0, 0.7, 0.7 );
        cr->set_dash( s_dash_pattern, 0 );
        cr->set_line_width( m_unit_line_thk * 3 );

        draw_bars2( cr, true );

        cr->unset_dash();
    }
}
inline void
Chart::draw_bars2( const Cairo::RefPtr< Cairo::Context >& cr, bool F_underlay )
{
    switch( m_data.get_type() )
    {
        case ChartData::TYPE_STRING:
            draw_bars( cr, m_data.values_str, F_underlay );
            break;
        case ChartData::TYPE_NUMBER:
            draw_bars( cr, m_data.values_num, F_underlay );
            break;
        case ChartData::TYPE_DATE:
            draw_bars( cr, m_data.values_date, F_underlay );
            break;
    }
}

inline void
Chart::draw_x_values_date( const Cairo::RefPtr< Cairo::Context >& cr )
{
    // YEAR & MONTH BAR
    const int   period          { m_data.get_period() };
    int         step_grid       { ( int ) ceil( m_width_col_min / m_step_x ) };
    int         step_grid_first { 0 };
    const auto  bar_top         { m_y_offset + m_height - m_ov_height - m_h_x_values };
    auto&&      it_date         { m_data.values_date.begin() };
    unsigned    year_last       { 0 };

    std::advance( it_date, m_step_start );

    if( period != ChartData::PERIOD_YEARLY )
    {
        if( step_grid > 12 )
            step_grid += 12 - ( step_grid % 12 );
        else if( step_grid > 6 )
            step_grid = 12;
        else if( step_grid > 4 )
            step_grid = 6;
        else if( step_grid > 3 )
            step_grid = 4;
        else if( step_grid > 2 )
            step_grid = 3;

        const auto sg_reduced{ step_grid > 12 ? 12 : step_grid };
        step_grid_first = ( sg_reduced - ( Date::get_month( it_date->first ) % sg_reduced ) + 1 )
                          % sg_reduced;
    }

    cr->rectangle( m_x_offset, bar_top, m_width, m_h_x_values );
    cr->set_source_rgb( 0.9, 0.9, 0.9 );
    cr->fill();

    m_layout->set_font_description( m_font_main );
    m_layout->set_alignment( Pango::Alignment::LEFT );

    cr->set_source_rgb( 0.0, 0.0, 0.0 );

    std::advance( it_date, step_grid_first );

    for( int i = step_grid_first; ;  )
    {
        const DateV date{ it_date->first };

        if( m_data.get_style() == ChartData::STYLE_BARS )
            cr->move_to( m_x_offset + m_x_min + m_step_x * ( i + 0.5 ), bar_top );
        else
            cr->move_to( m_x_offset + m_x_min + m_step_x * i, bar_top );
        cr->rel_line_to( 0, m_label_height );
        cr->rel_move_to( m_border_label, -m_label_height );

        if( period == ChartData::PERIOD_YEARLY )
        {
            m_layout->set_text( Date::get_year_str( date ) );
            m_layout->show_in_cairo_context( cr );
        }
        else
        {
            if( step_grid < 12 )
            {
                if( period == ChartData::PERIOD_MONTHLY )
                    m_layout->set_text( Date::get_month_str( date ) );
                else // weekly
                    m_layout->set_text( Date::format_string( date, "MD" ) );
                m_layout->show_in_cairo_context( cr );
            }

            if( year_last != Date::get_year( date ) )
            {
                cr->rel_move_to( 0, m_label_height );
                m_layout->set_text( Date::get_year_str( date ) );
                m_layout->show_in_cairo_context( cr );
                year_last = Date::get_year( date );
            }
        }

        i += step_grid;
        if( i < ( int ) m_step_count )
            std::advance( it_date, step_grid );
        else
            break;
    }
}

inline void
Chart::draw_x_values( const Cairo::RefPtr< Cairo::Context >& cr )
{
    switch( m_data.get_type() )
    {
        case ChartData::TYPE_STRING: draw_x_values_str( cr ); break;
        case ChartData::TYPE_NUMBER: draw_x_values_num( cr ); break;
        case ChartData::TYPE_DATE:   draw_x_values_date( cr ); break;
    }

    cr->set_source_rgb( 0.7, 0.7, 0.7 );
    cr->stroke();
}

inline void
Chart::draw_y_levels( const Cairo::RefPtr< Cairo::Context >& cr )
{
    // HORIZONTAL LINES
    const auto  step_v{ m_ampli_v / ( Y_DIVISIONS_MAX + 1 ) };
    const auto  step_y{ step_v * m_coefficient };

    cr->set_source_rgb( 0.6, 0.6, 0.6 );
    cr->set_line_width( m_unit_line_thk );
    m_layout->set_font_description( m_font_main );
    m_layout->set_width( 150 * Pango::SCALE );
    m_layout->set_alignment( Pango::Alignment::LEFT );

    // zero line and lines above:
    for( int i = 0; ( i * step_y ) <= ( m_y_0 - m_y_min ); i++ )
    {
        // + 0.5 offset needed to get crisp lines:
        cr->move_to( m_x_offset + m_width, m_y_offset + m_y_0 - i * step_y + 0.5 );
        cr->rel_line_to( -m_width, 0.0 );

        cr->rel_move_to( 4 * m_border_label, -m_label_size );
        m_layout->set_text( STR::format_number( step_v * i ) + " " + m_data.m_unit );
        m_layout->show_in_cairo_context( cr );

        cr->stroke();
    }

    // lines below zero line:
    for( int i = 1; ( i * step_y ) < ( m_y_max - m_y_0 ); i++ )
    {
        cr->move_to( m_x_offset + m_width, m_y_offset + m_y_0 + i * step_y + 0.5 );
        cr->rel_line_to( -m_width, 0.0 );

        cr->rel_move_to( 4 * m_border_label, -m_label_size );
        m_layout->set_text( STR::format_number( step_v * -i ) + " " + m_data.m_unit );
        m_layout->show_in_cairo_context( cr );

        cr->stroke();
    }
}

inline void
Chart::draw_milestones( const Cairo::RefPtr< Cairo::Context >& cr )
{
    auto&&  it_vd   { m_data.values_date.begin() };
    std::advance( it_vd, m_step_start );
    DateV   date_b  { it_vd->first };
    std::advance( it_vd, m_step_count - 1 );
    DateV   date_e  { it_vd->first };
    double  day_x   { m_length / Date::calculate_days_between_abs( date_b, date_e ) };

    for( auto& vd : m_data.values_date )
    {
        for( auto& elem : vd.second.elems )
        {
            if( elem->is_entry() )
            {
                Entry* e{ dynamic_cast< Entry* >( elem ) };
                if( e->get_title_style() == VT::ETS::MILESTONE::I )
                {
                    auto&& color = e->get_color();
                    Gdk::Cairo::set_source_rgba( cr, e->get_color() );
                    cr->rectangle( m_x_offset + m_x_min
                                              + day_x * Date::calculate_days_between_abs(
                                                                date_b, e->get_date() ),
                                   m_y_offset,
                                   day_x * e->get_duration(),
                                   m_y_max );
                    cr->fill();
                }
            }
        }
    }
}

inline void
Chart::draw_tooltip( const Cairo::RefPtr< Cairo::Context >& cr )
{
    const bool      F_line      { m_data.get_style() == ChartData::STYLE_LINE };
    const Value     v_hov       { get_value_at( m_hovered_step + m_step_start ) };
    const double    tip_pt_x    { m_x_min + m_step_x * ( m_hovered_step + ( F_line ? 0 : 0.5 ) ) };
    const double    tip_pt_y    { m_y_max - m_coefficient * ( v_hov - m_data.v_min ) };
    int             w, h;

    m_layout->set_font_description( m_font_bold );
    m_layout->set_text( STR::format_number( v_hov ) );
    m_layout->get_pixel_size( w, h );

    cr->set_source_rgb( 0.8, 0.5, 0.5 );
    cr->set_line_width( m_unit_line_thk );
    cr->move_to( m_x_offset, m_y_offset + tip_pt_y );
    cr->rel_line_to( m_width, 0.0 );
    if( F_line )
    {
        cr->move_to( m_x_offset + tip_pt_x, m_y_offset );
        cr->rel_line_to( 0.0, m_height - m_ov_height );
    }
    cr->stroke();

    if( F_line )
    {
        // Y RECT
        if( F_line ) // right aligned
            cr->rectangle( m_x_offset + tip_pt_x - w - 4,
                           m_y_offset + tip_pt_y - h - 4,
                           w + 4, h + 4 );
        else // centered
            cr->rectangle( m_x_offset + tip_pt_x - w/2.0 - 2,
                           m_y_offset + tip_pt_y - h - 4,
                           w + 4, h + 4 );

        cr->set_source_rgba( 0.8, 0.5, 0.5, 0.6 );
        cr->fill();

        // Y VALUE
        cr->set_source_rgb( 1.0, 1.0, 1.0 );
        if( F_line )
            cr->move_to( m_x_offset + tip_pt_x - w - 2, m_y_offset + tip_pt_y - h - 2 );
        else
            cr->move_to( m_x_offset + tip_pt_x - w/2.0, m_y_offset + tip_pt_y - h - 2 );
        m_layout->show_in_cairo_context( cr );

        // X RECT
        m_layout->set_text( get_x_label_at( m_step_start + m_hovered_step ) );
        m_layout->get_pixel_size( w, h );
        cr->rectangle( m_x_offset + tip_pt_x - w/2.0 - 2,
                       m_y_offset + m_height - m_ov_height - h,
                       w + 4, h + 4 );
        cr->set_source_rgb( 0.8, 0.5, 0.5 );
        cr->fill();

        // X VALUE
        cr->set_source_rgb( 1.0, 1.0, 1.0 );
        cr->move_to( m_x_offset + tip_pt_x - w/2.0, m_y_offset + m_height - m_ov_height - h + 2 );
        m_layout->show_in_cairo_context( cr );
    }
}

bool
Chart::draw( const Cairo::RefPtr< Cairo::Context >& cr, int w, int h )
{
    // TODO: 3.1: we may consider using the supplied w and h
    // BACKGROUND
    if( !m_F_printing_mode )
    {
        cr->rectangle( m_x_offset, m_y_offset, m_width, m_height );
        cr->set_source_rgb( 1.0, 1.0, 1.0 );
        cr->fill();
    }

    // HANDLE THERE-IS-TOO-FEW-ENTRIES-CASE SPECIALLY
    if( m_data.m_span < 2 )
    {
        cr->set_source_rgb( 0.0, 0.0, 0.0 );
        //auto layout = Pango::Layout::create( cr );
        m_layout->set_width( - 1 );
        m_layout->set_text( _( "INSUFFICIENT DATA" ) );
        m_layout->set_font_description( m_font_big );
        int w_layout, h_layout;
        m_layout->get_pixel_size( w_layout, h_layout );
        cr->move_to( m_x_offset + ( m_width - w_layout ) / 2.0 , m_y_offset + m_height / 2.0 );
        m_layout->show_in_cairo_context( cr );

        return true;
    }

    // NUMBER OF STEPS IN THE PRE AND POST BORDERS
    m_pre_steps = ceil( m_x_min / m_step_x );
    if( m_pre_steps > m_step_start )
        m_pre_steps = m_step_start;

    m_post_steps = ceil( m_border_curve / m_step_x );
    if( m_post_steps > m_data.m_span - m_step_count - m_step_start )
        m_post_steps = m_data.m_span - m_step_count - m_step_start;

    // ENTRY BACKGROUNDS
    if( m_data.get_type() == ChartData::TYPE_DATE )
        draw_milestones( cr );

    // YEAR & MONTH BAR
    draw_x_values( cr );

    // Y LEVELS
    draw_y_levels( cr );

    // GRAPH LINE OR BARS
    if( m_data.get_style() == ChartData::STYLE_LINE )
        draw_line( cr );
    else
        draw_bars( cr );
    // TODO: 3.1: add pie

    // TOOLTIP
    if( m_F_widget_hovered && m_hovered_step >= 0 )
        draw_tooltip( cr );

    // OVERVIEW
    if( m_step_count < m_data.m_span )
    {
        // OVERVIEW REGION
        cr->set_source_rgb( 0.7, 0.7, 0.7 );
        cr->rectangle( m_x_offset, m_y_offset + m_height - m_ov_height, m_width, m_ov_height );
        cr->fill();

        if( m_F_overview_hovered )
            cr->set_source_rgb( 1.0, 1.0, 1.0 );
        else
            cr->set_source_rgb( 0.95, 0.95, 0.95 );
        cr->rectangle( m_x_offset + m_border_label + m_step_start * m_step_x_ov,
                       m_y_offset + m_height - m_ov_height,
                       ( m_step_count - 1 ) * m_step_x_ov, m_ov_height );
        cr->fill();
        //cr->restore();

        // OVERVIEW LINE
        cr->set_source_rgb( 0.9, 0.3, 0.3 );
        cr->set_line_join( Cairo::Context::LineJoin::BEVEL );
        cr->set_line_width( m_unit_line_thk * 2 );

        cr->move_to( m_x_offset + m_border_label,
                     m_y_offset + m_height
                                - m_border_label
                                - m_coeff_ov * ( get_value_at( 0 ) - m_data.v_min ) );
        for( unsigned int i = 1; i < m_data.m_span; ++i )
        {
            cr->rel_line_to( m_step_x_ov,
                             - m_coeff_ov * ( get_value_at( i ) - get_value_at( i - 1 ) ) );
        }
        cr->stroke();

        // DIVIDER
        if( m_F_overview_hovered )
            cr->set_source_rgb( 0.2, 0.2, 0.2 );
        else
            cr->set_source_rgb( 0.45, 0.45, 0.45 );
        cr->rectangle( m_x_offset + 1.0,
                       m_y_offset + m_height - m_ov_height,
                       m_width - 2.0, m_ov_height - 1.0 );
        cr->stroke();
    }

    return true;
}
