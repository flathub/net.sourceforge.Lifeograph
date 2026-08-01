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


#include <cairomm/surface.h>

#include "table_surface.hpp"


using namespace LIFEO;


// WIDGETCHART =====================================================================================
Glib::RefPtr< Gdk::Pixbuf >
TableSurface::create_pixbuf( TableElem* table_elem,
                             int w,
                             const Pango::FontDescription& fd,
                             bool F_expanded )
{
    auto is      { Cairo::ImageSurface::create( Cairo::Surface::Format::RGB24, w, w * WH_RATIO ) };
    auto cr      { Cairo::Context::create( is ) };
    auto layout  { Pango::Layout::create( cr ) };
    auto ts      { new TableSurface( cr, table_elem, w, w * WH_RATIO, layout, fd, F_expanded ) };

    ts->draw( cr );

    auto pb      { Gdk::Pixbuf::create( is, 0, 0, ts->m_width, ts->m_height ) };

    delete ts;
    return pb;
}

TableSurface::TableSurface( Cairo::RefPtr< Cairo::Context >& cr,
                            TableElem* table_elem,
                            int w, int h_max,
                            Glib::RefPtr< Pango::Layout >& layout,
                            const Pango::FontDescription& fd,
                            bool F_expanded,
                            int i_line_top )
:   Table( layout, fd )
{
    m_i_line_top = i_line_top;
    set_diary( table_elem->get_diary() );
    m_data.set_from_string( table_elem->get_definition() );
    m_data.populate_lines( F_expanded );

    resize( w, h_max );
    calculate_row_h();
    update_line_c_vis();

    // reduce height back per m_line_c_vis:
    resize( w, ( ( m_line_c_act + 2 ) * m_row_h ) + ( 2 * S_MARGIN ) );
    update_height_vline();
    update_col_geom();
}

void
TableSurface::render( Cairo::RefPtr< Cairo::Context >& cr, double x, double y )
{
    m_x_offset = ( x + S_MARGIN );
    m_y_offset = ( y + S_MARGIN );
    m_x_offset_cell = ( x + m_comb_margin );
    m_y_offset_cell = ( y + m_comb_margin );
    draw( cr );
}
