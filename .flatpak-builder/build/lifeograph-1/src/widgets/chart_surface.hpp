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


#ifndef LIFEOGRAPH_CHART_SURFACE_HEADER
#define LIFEOGRAPH_CHART_SURFACE_HEADER


#include <cairomm/surface.h>

#include "../helpers.hpp"
#include "../diaryelements/entry.hpp"
#include "chart.hpp"


namespace LIFEO
{

using namespace HELPERS;


class ChartSurface : public Chart
{
    public:
                                    ChartSurface( Cairo::RefPtr< Cairo::Context >&,
                                                  ChartElem*,
                                                  int,
                                                  int,
                                                  Glib::RefPtr< Pango::Layout >&,
                                                  const Pango::FontDescription&,
                                                  bool = false );
                                    ~ChartSurface() {}

        static Glib::RefPtr< Gdk::Pixbuf >
        create_pixbuf( ChartElem*, int, const Pango::FontDescription& );

        void
        render( Cairo::RefPtr< Cairo::Context >&, double, double );

    protected:
        static constexpr double     WH_RATIO{ 0.6 };
};

} // end of namespace LIFEO

#endif
