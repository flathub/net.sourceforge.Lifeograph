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


#ifndef LIFEOGRAPH_CALENDAR_HEADER
#define LIFEOGRAPH_CALENDAR_HEADER


#include <gtkmm/widget.h>

#include "../helpers.hpp"


namespace LIFEO
{

using namespace HELPERS;


class Calendar
{
    public:
        // CONSTANTS
        static constexpr float      s_margin = 10.0;
        static constexpr float      s_border_label = 7.0;
        static constexpr float      s_font_height = 10.0;
        static constexpr float      s_title_bar_height = 28.0;
        static constexpr float      s_day_bar_height = 22.0;
        static constexpr float      COLUMN_WIDTH_MIN= 50.0;
        // CALCULATED CONSTANTS
        static constexpr float      s_grid_start_y = s_margin + s_title_bar_height + s_day_bar_height;

                                    Calendar();
        virtual                     ~Calendar(){ }

        void                        set_date( DateV );
        void                        set_day_shown( DateV d )
        { m_day_shown = d; }
        void                        set_selected_date( DateV date )
        { m_day_hovered = date; set_date( date ); }
        DateV                       get_selected_date() const
        { return m_day_hovered; }

        void                        set_day_highlighted( DateV date )
        { m_day_highlighted = date; set_date( date ); }
        void                        resize( int, int );

        virtual void                refresh(){ }

    protected:
        void                        update_col_geom();

        static void                 calculate_begin_date( DateV& begin_date );

        bool                        draw( const Cairo::RefPtr< Cairo::Context >& );

        // GEOMETRY
        int                         m_width{ -1 };
        int                         m_height{ -1 };

        float                       m_x_max{ 0.0f }, m_y_max{ 0.0f };
        float                       m_length{ 0.0f };
        int                         m_unit_text_w{ 0 };

        double                      m_col_width{ 0.0 };
        double                      m_row_height{ 0.0 };

        Pango::FontDescription      m_font_base;
        Pango::FontDescription      m_font_bold;
        Pango::FontDescription      m_font_medium;
        Pango::FontDescription      m_font_big;

        // FLAGS
        bool                        m_F_editable         { false };
        bool                        m_F_selection_locked { false };

        DateV                       m_month              { Date::NOT_SET };
        DateV                       m_day_shown          { Date::NOT_SET };
        DateV                       m_day_hovered        { Date::NOT_SET };
        int                         m_column_hovered     { -1 };
        int                         m_col_active         { -1 };
        int                         m_row_active         { -1 };
        DateV                       m_day_highlighted    { Date::NOT_SET };
};

} // end of namespace LIFEO

#endif
