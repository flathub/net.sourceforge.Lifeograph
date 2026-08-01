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


#ifndef LIFEOGRAPH_WIDGETDATEPICKER_HEADER
#define LIFEOGRAPH_WIDGETDATEPICKER_HEADER


#include <gtkmm.h>

#include "../helpers.hpp"
#include "widget_calendar.hpp"


namespace LIFEO
{

using namespace HELPERS;


// WIDGET FOR ENTERING ENTRY NAMES WITH AUTOCOMPLETION =============================================
class WidgetDatePicker : public Gtk::Entry
{
    public:
        WidgetDatePicker( BaseObjectType*, const Glib::RefPtr<Gtk::Builder>& );
        WidgetDatePicker();
        ~WidgetDatePicker();

        void                        set_text( const Ustring& text ) = delete;

        void                        set_date( DateV );
        DateV                       get_date() const
        { return m_date; }
        void                        clear()
        { Gtk::Entry::set_text( "" ); }

        void                        popup_cal();
        void                        popdown_cal();

        SignalVoidDate              signal_date_set()
        { return m_Sg_date_set; }

    protected:
        void                        initialize();

        void                        on_changed() override;
        bool                        on_key_press_event( guint, guint, Gdk::ModifierType );
        Gdk::DragAction             handle_drop_motion( double, double );
        bool                        handle_drop( const Glib::ValueBase&, double, double );

        DateV                       m_date{ Date::NOT_SET };

        Gtk::Popover*               m_Po{ nullptr };
        WidgetCalendar*             m_W_calendar;

        SignalVoidDate              m_Sg_date_set;

        Glib::RefPtr< Gtk::DropTarget >
                                    m_drop_target;
};

} // end of namespace LIFEO

#endif
