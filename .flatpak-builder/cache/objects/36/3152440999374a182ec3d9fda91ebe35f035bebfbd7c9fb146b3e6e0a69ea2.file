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


#ifndef LIFEOGRAPH_TABLE_SURFACE_HEADER
#define LIFEOGRAPH_TABLE_SURFACE_HEADER


#include <cairomm/surface.h>

#include "../helpers.hpp"
#include "table.hpp"


namespace LIFEO
{

using namespace HELPERS;


class TableSurface : public Table
{
    public:
        TableSurface( Cairo::RefPtr< Cairo::Context >&,
                      TableElem*,
                      int, int,
                      Glib::RefPtr< Pango::Layout >&,
                      const Pango::FontDescription&,
                      bool,
                      int = 0 );
        TableSurface( Cairo::RefPtr< Cairo::Context >& cr,
                      TableElem* te,
                      int w, int h,
                      Glib::RefPtr< Pango::Layout >& lo,
                      const Pango::FontDescription& fd,
                      bool Fe,
                      bool Fp,
                      int it )
        : TableSurface( cr, te, w, h, lo, fd, Fe, it ) { m_F_printing_mode = Fp; }
        ~TableSurface() {}

        static Glib::RefPtr< Gdk::Pixbuf >
        create_pixbuf( TableElem*, int, const Pango::FontDescription&, bool );

        void
        render( Cairo::RefPtr< Cairo::Context >&, double, double );

        int
        get_i_line_top_next() const
        { return( m_i_line_top + m_line_c_act ); }

    protected:
        static constexpr double     WH_RATIO{ 0.75 };
};

} // end of namespace LIFEO

#endif
