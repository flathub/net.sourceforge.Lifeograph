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


#ifndef LIFEOGRAPH_TABLE_HEADER
#define LIFEOGRAPH_TABLE_HEADER


#include <gtkmm/widget.h>

#include "../helpers.hpp"
#include "../diaryelements/diary.hpp"


namespace LIFEO
{

using namespace HELPERS;

class Table
{
    public:
        // CONSTANTS
        static constexpr double     S_MARGIN{ 6.0 };
        static constexpr int        S_MIN_LINES = 3;

                                    Table( Glib::RefPtr< Pango::Layout >& layout,
                                           const Pango::FontDescription& fd )
                                    : Table( fd ) { m_layout = layout; }
                                    Table( const Pango::FontDescription& fd =
                                           Pango::FontDescription( "Sans 11" ) );
        virtual                     ~Table() { }

        void                        set_diary( Diary* diary )
        { m_p2diary = diary; m_data.set_diary( diary ); }
        Diary*                      get_diary()
        { return m_p2diary; }

        TableData&                  get_table_data()
        { return m_data; }

        int                         get_width() const { return m_width; }
        int                         get_height() const { return m_height; }

        void                        resize( int, int );
        void                        scroll( int );
        bool                        has_more() const
        { return( ( m_i_line_top + m_line_c_act ) < m_data.m_lines.size() ); }

        virtual void                refresh() { }

    protected:
        void                        calculate_row_h();
        void                        update_col_geom();
        void                        update_height_vline();
        void                        update_line_c_vis();

        bool                        draw( const Cairo::RefPtr< Cairo::Context >& );

        void                        draw_group_bg( const Cairo::RefPtr< Cairo::Context >&, int );
        void                        draw_cell_progress( const Cairo::RefPtr< Cairo::Context >&,
                                                        int, int );
        void                        draw_cell_text_main( const Cairo::RefPtr< Cairo::Context >&,
                                                         int, int,
                                                         const Ustring& );
        void                        draw_cell_text_extra( const Cairo::RefPtr< Cairo::Context >&,
                                                          int, int,
                                                          const Ustring& );
        void                        draw_cell_text( const Cairo::RefPtr< Cairo::Context >&,
                                                    int, int );
        void                        draw_group_size( const Cairo::RefPtr< Cairo::Context >&,
                                                     int, int );
        void                        draw_sort_arrow( const Cairo::RefPtr< Cairo::Context >&,
                                                     TableColumn* );
        void                        draw_cell_todo_status( const Cairo::RefPtr< Cairo::Context >&,
                                                           int, int );
        void                        draw_cell_gantt( const Cairo::RefPtr< Cairo::Context >&,
                                                     int, int );
        void                        draw_cell_bool( const Cairo::RefPtr< Cairo::Context >&,
                                                    int, int );
        void                        draw_header_std( const Cairo::RefPtr< Cairo::Context >&,
                                                     TableColumn* );
        void                        draw_header_gantt( const Cairo::RefPtr< Cairo::Context >&,
                                                       int );

        // DATA
        TableData                   m_data;

        // GEOMETRY
        double                      m_cell_margin   { 3.0 };
        double                      m_cell_marg_ttl { m_cell_margin * 2 };
        double                      m_comb_margin   { S_MARGIN + m_cell_margin };
        double                      m_x_offset      { S_MARGIN };
        double                      m_y_offset      { S_MARGIN };
        double                      m_x_offset_cell { m_comb_margin };
        double                      m_y_offset_cell { m_comb_margin };
        int                         m_width         { -1 };
        int                         m_height        { -1 };

        double                      m_width_net     { 0.0 };
        double                      m_height_net    { 0.0 }; // capacity
        double                      m_height_vline  { 0.0 }; // actually used by lines

        std::vector< double >       m_col_widths;
        std::vector< double >       m_col_widths_cum;
        std::set< int >             m_line_breaks;
        double                      m_row_h         { 0.0 };
        int                         m_row_h_net     { 0 };
        double                      m_unit_line_thk { 1.0 };

        // COLORS
        static const std::valarray< double >    clrR;
        static const std::valarray< double >    clrG;

        unsigned int                m_line_c_act    { 0 }; // actual visible line count
        unsigned int                m_line_c_max    { 0 }; // max visible line count per h
        int                         m_i_line_top    { 0 };
        int                         m_i_col_cur     { -1 };
        int                         m_i_line_hover  { -1 };
        int                         m_i_line_cur    { -1 }; // current entry in the text editor

        Glib::RefPtr< Pango::Layout > m_layout;
        Pango::FontDescription      m_font_main;
        Pango::FontDescription      m_font_header;
        Pango::FontDescription      m_font_gantt;
//        const static std::valarray< double >
//                                    s_dash_pattern;

        // FLAGS
        bool                        m_F_widget_hovered  { false };
        bool                        m_F_printing_mode   { false };

        Diary*                      m_p2diary           { nullptr };
};

} // end of namespace LIFEO

#endif
