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


#ifndef LIFEOGRAPH_WIDGETMAP_HEADER
#define LIFEOGRAPH_WIDGETMAP_HEADER


#include "src/helpers.hpp"
extern "C"
{
#include <shumate/shumate.h>
}

#include <gtkmm.h>

#include "../diaryelements/diarydata.hpp"

namespace LIFEO
{

// WRAPPER FOR MAP WIDGET ==========================================================================
class WidgetMap
{
    public:
        WidgetMap();

        void                        refresh()
        { m_W_map->queue_draw(); }
        void                        refresh_path();

        Gtk::Widget&                get_widget()
        { return *m_W_map; }
        double                      get_latitude()
        { return m_latitude; }
        double                      get_longitude()
        { return m_longitude; }

        void                        set_diary( Diary* );
        void                        set_entry( Entry* );

        void                        handle_editing_enabled();

        void                        center_on( double, double );

    protected:
        void                        show_Po( int, int );

        void                        refresh_entry_locations();

        void                        create_entry_at_location();
        void                        create_path_point_at_location( bool = true );

        void                        handle_para_node_click( int, double, double, Paragraph* );
        void                        handle_entry_node_click( int, double, double, Entry* );

        ShumateMarker*              add_point( const Coords&, const String&, const Ustring&,
                                               Glib::RefPtr< Gtk::GestureClick>&, const Ustring& );
        ShumateMarker*              add_entry_point( const String&, const Ustring&, Entry* );
        ShumateMarker*              add_para_point( const String&, const Ustring&, Paragraph* );

        void                        remove_last_path_point_from_entry();
        void                        remove_selected_path_point_from_entry();
        void                        clear_path();
        Ustring                     get_path_distance_str();

        bool                        on_scroll_event( double, double );
        void                        on_button_press_event( int, double, double );
        bool                        on_key_press_event( guint, guint, Gdk::ModifierType );

        Gtk::Widget*                m_W_map;
        ShumateSimpleMap*           m_SM_map;
        ShumateViewport*            m_SVp_map;
        ShumateMapSourceRegistry*   m_registry;

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;
        R2ActionGroup               m_AG;
        R2Action                    m_A_assign_para_coords;
        R2Action                    m_A_add_path_point;
        R2Action                    m_A_insert_path_point;
        R2Action                    m_A_remove_path_point;
        R2Action                    m_A_clear_path;
        R2Action                    m_A_show_entry_locations;
        Gtk::PopoverMenu*           m_Po_actions;

        ShumateMapLayer*            m_map_layer;
        ShumatePathLayer*           m_path_layer;
        ShumateMarkerLayer*         m_marker_layer_path;
        ShumateMarkerLayer*         m_marker_layer_entries;

        Glib::RefPtr< Gtk::EventControllerKey >
                                    m_EC_key;
        Glib::RefPtr< Gtk::GestureClick >
                                    m_gesture_click;
        Glib::RefPtr< Gtk::EventControllerScroll >
                                    m_event_controller_scroll;
        // Glib::RefPtr< Gtk::EventControllerFocus >
        //                             m_EC_focus;

        double                      m_latitude;
        double                      m_longitude;

        Diary*                      m_p2diary     { nullptr };
        Entry*                      m_p2entry_cur { nullptr };
        Paragraph*                  m_p2para_cur  { nullptr };

        bool                        m_F_zoom_on_show { true };
        bool                        m_F_center_on_show { true };
};

} // end of namespace LIFEO

#endif
