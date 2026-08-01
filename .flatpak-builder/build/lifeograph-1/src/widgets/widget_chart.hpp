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


#ifndef LIFEOGRAPH_WIDGETCHART_HEADER
#define LIFEOGRAPH_WIDGETCHART_HEADER


#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>

#include "../helpers.hpp"
#include "chart.hpp"
#include "widget_entrypicker.hpp"
#include "widget_filterpicker.hpp"
#include "../dialogs/dialog_filter.hpp"


namespace LIFEO
{

using namespace HELPERS;


class WidgetChart : public Gtk::DrawingArea, public Chart
{
    public:
        WidgetChart( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );
        ~WidgetChart() {}

        void                        set_helper_widgets( Gtk::MenuButton*, Gtk::Scale* );
        void                        set_diary( Diary* diary );
        void                        refresh_diary();

        void                        refresh_editabilitiy()
        { m_Po_properties->set_sensitive( get_diary()->is_in_edit_mode() ); }

        void                        refresh() override
        {
            // if( Glib::RefPtr< Gdk::Window > window = get_window() )
            //     window->invalidate( false );
            queue_draw();
        }

        void                        remove_entry( Entry* );

        void                        handle_table_changed();
        void                        handle_x_axis_changed();
        void                        handle_y_axis_changed();
        void                        handle_style_changed();
        void                        handle_underlay_changed();
        void                        handle_period_changed();
        void                        handle_combining_changed();
        void                        handle_col_filter_changed();
        void                        handle_col_filter_v_changed();

        void                        calculate_and_plot( bool = true );

        String                      get_as_string()
        { return m_data.get_as_string(); }
        void                        set_from_string( const Ustring& );

        SignalVoid                  signal_changed()
        { return m_Sg_changed; }

    protected:
        void                        on_resize( int, int ) override;
        bool                        on_scroll_event( double, double );
        void                        on_button_press_event( int, double, double );
        void                        on_button_release_event( int, double, double );
        void                        on_motion_notify_event( double, double );
        void                        on_leave_notify_event();
        void                        on_draw( const Cairo::RefPtr< Cairo::Context >&, int, int );

        void                        update_CB_underlay();
        void                        update_CB_filter_v();
        void                        update_child_widgets_per_data();

        SetUstrings                 m_col_filter_values;

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;

        Gtk::Popover*               m_Po_properties;
        Gtk::Scale*                 m_Sc_zoom;

        Gtk::ToggleButton*          m_RB_style_line;
        Gtk::ToggleButton*          m_RB_style_bars;
        // Gtk::ToggleButton*          m_RB_style_pie;
        Gtk::ComboBoxText*          m_CB_table;
        Gtk::ComboBoxText*          m_CB_x_axis;
        Gtk::ComboBoxText*          m_CB_y_axis;
        Gtk::ComboBoxText*          m_CB_underlay;
        Gtk::Box*                   m_Bx_grouping;
        Gtk::Box*                   m_Bx_combining;
        Gtk::ComboBoxText*          m_CB_period;
        Gtk::ComboBoxText*          m_CB_combining;
        Gtk::Box*                   m_Bx_col_filter_v;
        Gtk::ComboBoxText*          m_CB_col_filter;
        Gtk::DropDown*              m_DD_col_filter_v;

        Glib::RefPtr< Gtk::GestureClick >
                                    m_gesture_click;

        SignalVoid                  m_Sg_changed;
};

} // end of namespace LIFEO

#endif
