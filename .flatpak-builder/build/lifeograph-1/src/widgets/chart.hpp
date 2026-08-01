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


#ifndef LIFEOGRAPH_CHART_HEADER
#define LIFEOGRAPH_CHART_HEADER


#include <gtkmm/widget.h>

#include "../helpers.hpp"
#include "../diaryelements/diarydata.hpp"
#include "../diaryelements/diary.hpp"


namespace LIFEO
{

using namespace HELPERS;


class FiltererContainer; // forward declaration


class Chart
{
    public:
                                    Chart( Glib::RefPtr< Pango::Layout >& layout,
                                           const Pango::FontDescription& fd,
                                           bool F_printing_mode )
                                    : Chart( fd )
                                    {
                                        m_F_printing_mode = F_printing_mode;
                                        m_layout = layout;
                                    }
                                    Chart( const Pango::FontDescription& fd =
                                           Pango::FontDescription( "Sans 11" ) );
        virtual                     ~Chart() { }

        void                        set_diary( Diary* diary )
        { m_data.set_diary( diary ); }
        Diary*                      get_diary()
        { return m_data.m_p2diary; }

       ChartData&                   get_chart_data()
       { return m_data; }

        bool                        is_zoom_possible() const;
        void                        set_zoom( float );

        int                         get_width() const { return m_width; }
        int                         get_height() const { return m_height; }

        void                        resize( int, int );
        void                        update_dims( const bool );
        void                        update_h_x_values();
        void                        scroll( int );

        virtual void                refresh() { }

    protected:
        void                        update_col_geom( bool = false );

        Value
        get_value_at( int i )
        {
            switch( m_data.get_type() )
            {
                case ChartData::TYPE_STRING: return get_yvalue_at( m_data.values_str, i ).v;
                case ChartData::TYPE_NUMBER: return get_yvalue_at( m_data.values_num, i ).v;
                default:                     return get_yvalue_at( m_data.values_date, i ).v;
            }
        }

        const SetDiaryElemsByName&
        get_elems_at( int i )
        {
            switch( m_data.get_type() )
            {
                case ChartData::TYPE_STRING: return get_yvalue_at( m_data.values_str, i ).elems;
                case ChartData::TYPE_NUMBER: return get_yvalue_at( m_data.values_num, i ).elems;
                default:                     return get_yvalue_at( m_data.values_date, i ).elems;
            }
        }
        template< class T >
        const ChartData::YValues&
        get_yvalue_at( const T& values, int i )
        {
            if( abs( i ) >= values.size() )
                throw Error( STR::compose( "YValue (", i, ") out of bounds" ) );

            auto&& iter{ i < 0 ? values.end() : values.begin() };
            std::advance( iter, i < 0 ? i - 1 : i );
            return iter->second;
        }

        Ustring
        get_x_label_at( int i )
        {
            switch( m_data.get_type() )
            {
                case ChartData::TYPE_STRING:
                    return m_data.values_index2str[ i ];
                case ChartData::TYPE_NUMBER:
                {
                    auto&& iter{ m_data.values_num.begin() };
                    std::advance( iter, i );
                    return STR::format_number( iter->first );
                }
                default:
                {
                    auto&& iter{ m_data.values_date.begin() };
                    std::advance( iter, i );
                    return Date::format_string( iter->first );
                }
            }
        }

        template< class T >
        void draw_line( const Cairo::RefPtr< Cairo::Context >& cr,
                        const T& values,
                        const bool F_underlay = false )
        {
            auto it_v{ values.begin() };
            std::advance( it_v, m_step_start - m_pre_steps );

            cr->move_to( m_x_offset + m_x_min - m_step_x * m_pre_steps,
                         m_y_offset + m_y_max
                                    - m_coefficient * ( ( F_underlay ? it_v->second.u
                                                                     : it_v->second.v )
                                                        - m_data.v_min ) );

            for( unsigned int i = 1; i < m_step_count + m_pre_steps + m_post_steps; i++ )
            {
                ++it_v;

                //may not be needed if( vy != std::numeric_limits< double >::max() )
                cr->line_to( m_x_offset + m_x_min + m_step_x * ( i - m_pre_steps ),
                             m_y_offset + m_y_max
                                        - m_coefficient * ( ( F_underlay ? it_v->second.u
                                                                         : it_v->second.v )
                                                            - m_data.v_min ) );
            }
            cr->stroke();
        }

        template< class T >
        void draw_bars( const Cairo::RefPtr< Cairo::Context >& cr,
                        const T& values,
                        const bool F_underlay = false )
        {
            auto       it_v     { values.begin() };
            const bool F_show_v { !F_underlay &&
                                  ( m_step_x * Pango::SCALE ) > ( m_font_bold.get_size() * 4 ) };
            std::advance( it_v, m_step_start - m_pre_steps );

            if( F_show_v )
            {
                m_layout->set_font_description( m_font_bold );
                m_layout->set_width( m_step_x * 0.8 * Pango::SCALE );
                m_layout->set_alignment( Pango::Alignment::CENTER );
            }

            for( unsigned int i = 0; i < m_step_count + m_pre_steps + m_post_steps; i++ )
            {
                const auto bar_h{ m_coefficient * ( F_underlay ? it_v->second.u
                                                               : it_v->second.v ) };

                if( bar_h != 0 )
                    cr->rectangle( m_x_offset + m_x_min + m_step_x * ( i - m_pre_steps + 0.1 ),
                                   m_y_offset + m_y_0,
                                   m_step_x * 0.8,
                                   -bar_h );

                if( F_show_v && it_v->second.v != 0.0 )
                {
                    m_layout->set_text( STR::format_number( it_v->second.v ) );

                    cr->move_to( m_x_offset + m_x_min + m_step_x * ( i - m_pre_steps + 0.1 ),
                                 m_y_offset + m_y_0 - bar_h - ( bar_h < 0 ? 0 : m_label_height ) );
                    m_layout->show_in_cairo_context( cr );
                }

                ++it_v;
            }
            if( F_underlay )
                cr->stroke();
            else
                cr->fill();
        }

        double draw_x_values_str_prepare( const Cairo::RefPtr< Cairo::Context >& cr )
        {
            if( m_data.get_style() == ChartData::STYLE_LINE )
            {
                cr->rectangle( m_x_offset, m_y_offset + m_y_max, m_width, m_h_x_values );
                cr->set_source_rgb( 0.9, 0.9, 0.9 );
                cr->fill();
            }

            m_layout->set_font_description( m_font_main );
            m_layout->set_width( 1.3 * m_h_x_values * Pango::SCALE );
            m_layout->set_alignment( Pango::Alignment::LEFT );
            m_layout->set_ellipsize( Pango::EllipsizeMode::END );

            cr->set_source_rgb( 0.0, 0.0, 0.0 );
            return( m_y_offset + m_height - m_ov_height - m_h_x_values );
        }
        void draw_x_value_str_column( const Cairo::RefPtr< Cairo::Context >& cr,
                                      const Ustring& txt,
                                      int i ,
                                      const double bar_top )
        {
            if( m_data.get_style() == ChartData::STYLE_BARS )
                cr->move_to( m_x_offset + m_x_min + m_step_x * ( i + 0.5 ), bar_top );
            else
                cr->move_to( m_x_offset + m_x_min + m_step_x * i, bar_top );

            if( m_data.get_style() == ChartData::STYLE_LINE )
            {
                cr->rel_line_to( 0, m_label_height );
                cr->rel_move_to( m_border_label, -m_label_height );
            }

            cr->save();
            cr->rotate_degrees( 45 );
            m_layout->set_text( txt );
            m_layout->show_in_cairo_context( cr );
            cr->restore();
        }

        void draw_x_values_num( const Cairo::RefPtr< Cairo::Context >& cr )
        {
            auto&&      it_v        { m_data.values_num.begin() };
            const int   step_grid   { 1 }; //( int ) ceil( WIDTH_COL_MIN / m_step_x ) };
            const auto  bar_top     { draw_x_values_str_prepare( cr ) };

            for( int i = 0; i < ( int ) m_step_count; i += step_grid )
            {
                draw_x_value_str_column( cr, STR::format_number( it_v->first ), i, bar_top );
                ++it_v; //std::advance( it_v, step_grid );
            }
        }

        void draw_x_values_str( const Cairo::RefPtr< Cairo::Context >& cr )
        {
            auto&&      it_v        { m_data.values_index2str.begin() };
            const auto  bar_top     { draw_x_values_str_prepare( cr ) };

            draw_x_values_str_prepare( cr );

            for( int i = 0; i < ( int ) m_step_count; i++ )
            {
                draw_x_value_str_column( cr, it_v->second, i, bar_top );
                ++it_v;
            }
        }

        void                        draw_line( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_line2( const Cairo::RefPtr< Cairo::Context >&, bool );
        void                        draw_bars( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_bars2( const Cairo::RefPtr< Cairo::Context >&, bool );
        void                        draw_x_values( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_x_values_date( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_y_levels( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_milestones( const Cairo::RefPtr< Cairo::Context >& );
        void                        draw_tooltip( const Cairo::RefPtr< Cairo::Context >& );
        bool                        draw( const Cairo::RefPtr< Cairo::Context >&, int, int );

        DateV                       get_period_date( DateV );
        Ustring                     get_date_str( DateV ) const;

        // DATA
        ChartData                   m_data          { nullptr };

        // GEOMETRY
        static constexpr int        Y_DIVISIONS_MAX { 4 };
        double                      m_x_offset      { 0.0 };
        double                      m_y_offset      { 0.0 };
        int                         m_width         { -1 };
        int                         m_height        { -1 };
        unsigned int                m_step_count    { 0 };
        unsigned int                m_step_start    { 0 };
        double                      m_pre_steps;
        double                      m_post_steps;
        double                      m_zoom_level    { 1.0 };

        double                      m_border_curve;
        double                      m_border_label;
        double                      m_label_size;
        double                      m_label_height;
        double                      m_h_x_values;
        double                      m_width_col_min;

        double                      m_x_min;
        double                      m_x_min_bar;
        double                      m_y_min;

        double                      m_x_max{ 0.0 }, m_y_max{ 0.0 }, m_y_0{ 0.0 };
        double                      m_ampli_y{ 0.0 }, m_length{ 0.0 };
        double                      m_ampli_v{ 0.0 };
        double                      m_step_x{ 0.0 }, m_coefficient{ 0.0 };
        double                      m_ov_height{ 0.0 };
        double                      m_step_x_ov{ 0.0 }, m_ampli_ov{ 0.0 }, m_coeff_ov{ 0.0 };
        double                      m_unit_line_thk{ 1.0 };

        int                         m_hovered_step{ 0 };
        int                         m_warning_w{ 0 };

        Glib::RefPtr< Pango::Layout > m_layout;
        Pango::FontDescription      m_font_main;
        Pango::FontDescription      m_font_bold;
        Pango::FontDescription      m_font_big;
        const static std::valarray< double >
                                    s_dash_pattern;

        // FLAGS
        bool                        m_F_button_pressed   { false };
        bool                        m_F_overview_hovered { false };
        bool                        m_F_widget_hovered   { false };
        bool                        m_F_printing_mode    { false };
};

} // end of namespace LIFEO

#endif
