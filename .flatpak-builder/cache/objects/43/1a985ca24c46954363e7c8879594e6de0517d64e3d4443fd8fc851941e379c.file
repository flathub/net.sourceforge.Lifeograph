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

#include "chart_surface.hpp"


using namespace LIFEO;


// WIDGETCHART =====================================================================================lib::RefPtr< Gdk::Pixbuf >
Glib::RefPtr< Gdk::Pixbuf >
ChartSurface::create_pixbuf( ChartElem* chart_elem,
                             int w,
                             const Pango::FontDescription& fd )
{
    auto is      { Cairo::ImageSurface::create( Cairo::Surface::Format::RGB24, w, w * WH_RATIO ) };
    auto cr      { Cairo::Context::create( is ) };
    auto layout  { Pango::Layout::create( cr ) };
    auto cs      { new ChartSurface( cr, chart_elem, w, w * WH_RATIO, layout, fd ) };

    cs->draw( cr, w, cs->m_height );

    auto pb      { Gdk::Pixbuf::create( is, 0, 0, cs->m_width, cs->m_height ) };

    delete cs;
    return pb;
}

ChartSurface::ChartSurface( Cairo::RefPtr< Cairo::Context >& cr,
                            ChartElem* chart_elem,
                            int w, int h,
                            Glib::RefPtr< Pango::Layout >& layout,
                            const Pango::FontDescription& fd,
                            bool F_printing_mode )
:   Chart( layout, fd, F_printing_mode )
{
    m_hovered_step = -1;
    set_diary( chart_elem->get_diary() );
    m_data.set_from_string( chart_elem->get_definition() );
    m_data.calculate_points();

    resize( w, h == -1 ? w * WH_RATIO : h );
}

void
ChartSurface::render( Cairo::RefPtr< Cairo::Context >& cr, double x, double y )
{
    m_x_offset = x;
    m_y_offset = y;
    draw( cr, x, y );
}
