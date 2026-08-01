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


#ifndef LIFEOGRAPH_WIDGET_CALENDAR_HEADER
#define LIFEOGRAPH_WIDGET_CALENDAR_HEADER


#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>

#include "../helpers.hpp"
#include "calendar.hpp"


namespace LIFEO
{

using namespace HELPERS;


class WidgetCalendar : public Gtk::DrawingArea, public Calendar
{
    public:
        WidgetCalendar();
        WidgetCalendar( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );

        void                        refresh() override
        {
            // if( Glib::RefPtr< Gdk::Window > window = get_window() )
            //     window->invalidate( false );
            queue_draw();
        }

        void                        set_editable( bool );

        void                        handle_weekend_toggled();
        void                        handle_holiday_toggled();
        void                        handle_week_nos_toggled();

        void                        set_selected_day_from_coords( int, int );
        Gdk::Rectangle              get_selected_rectangle() const;

        void                        show_Po( int, int );

        SignalVoid                  signal_month_changed()
        { return m_Sg_month_changed; }
        SignalVoidDate              signal_day_selected()
        { return m_Sg_day_selected; }
        SignalVoidDateUstring       signal_day_add()
        { return m_Sg_day_add; }

    protected:
        void                        init();

        void                        on_resize( int, int ) override;
        bool                        on_scroll_event( double, double );
        void                        on_button_press_event( int, double, double );
        void                        on_button_release_event( int, double, double );
        void                        on_motion_notify_event( double, double );
        void                        on_leave_notify_event();
        void                        on_draw( const Cairo::RefPtr< Cairo::Context >&, int, int );

        // DRAG SOURCE METHODS
        void                        handle_drag_begin( const Glib::RefPtr< Gdk::Drag >& );
        Glib::RefPtr< Gdk::ContentProvider >
                                    handle_drag_prepare( double, double );
        void                        handle_drag_end( const Glib::RefPtr< Gdk::Drag >&, bool );

        // DROP TARGET METHODS
        Gdk::DragAction             handle_drop_motion( double, double );
        bool                        handle_drop( const Glib::ValueBase&, double, double );

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;
        Gtk::PopoverMenu*           m_Po_day;
        Gtk::Box*                   m_Bx_add;

        Glib::RefPtr< Gtk::EventControllerScroll >
                                    m_event_controller_scroll;
        Glib::RefPtr< Gtk::GestureClick >
                                    m_gesture_click;
        Glib::RefPtr< Gtk::DragSource >
                                    m_drag_source;
        Glib::RefPtr< Gtk::DropTarget >
                                    m_drop_target;

        R2ActionGroup               m_AG;
        R2Action                    m_A_copy;
        R2Action                    m_A_weekend;
        R2Action                    m_A_holiday;
        R2Action                    m_A_show_week_nos;

        SignalVoid                  m_Sg_month_changed;
        SignalVoidDate              m_Sg_day_selected;
        SignalVoidDateUstring       m_Sg_day_add;

        bool                        m_F_expect_drag_data{ false };
};

} // end of namespace LIFEO

#endif
