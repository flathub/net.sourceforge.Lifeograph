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


#include <shumate/shumate.h>

#include "../helpers.hpp"
#include "../lifeograph.hpp"
#include "../app_window.hpp"
#include "widget_map.hpp"


using namespace LIFEO;


WidgetMap::WidgetMap()
{
    m_SM_map = shumate_simple_map_new();
    m_W_map = Glib::wrap( GTK_WIDGET( m_SM_map ) );
    m_W_map->set_vexpand( true );
    m_W_map->set_hexpand( true );

    m_registry = shumate_map_source_registry_new_with_defaults();
    auto map_source = shumate_map_source_registry_get_by_id( m_registry,
                                                             SHUMATE_MAP_SOURCE_OSM_MAPNIK );
    shumate_simple_map_set_map_source( m_SM_map, SHUMATE_MAP_SOURCE( map_source ) );

    // not sure about this gtk_widget_init_template( GTK_WIDGET ( m_SM_map ) );

    // shumate_map_source_registry_add( m_registry, SHUMATE_MAP_SOURCE( shumate_test_tile_source_new() ) );

    /*if( shumate_vector_renderer_is_supported() )
    {
        bytes = g_resources_lookup_data( "/org/gnome/Shumate/Demo/osm-liberty/style.json",
                                         G_RESOURCE_LOOKUP_FLAGS_NONE, NULL );
        style_json = ( const char* ) g_bytes_get_data (bytes, NULL);

        ShumateVectorRenderer *renderer = shumate_vector_renderer_new( "vector-tiles",
                                                                       style_json,
                                                                       &error );

        if( error )
        {
            g_warning( "Failed to create vector map style: %s", error->message );
            g_clear_error( &error );
        }
        else
        {
            g_autoptr(GdkPixbuf) sprites_pixbuf = NULL;
            g_autoptr(GBytes) sprites_json = NULL;

            sprites_json = g_resources_lookup_data( "/org/gnome/Shumate/Demo/osm-liberty/sprites.json", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
            sprites_pixbuf = gdk_pixbuf_new_from_resource( "/org/gnome/Shumate/Demo/osm-liberty/sprites.png", NULL);

            shumate_vector_renderer_set_sprite_sheet_data( renderer,
                                                           sprites_pixbuf,
                                                           g_bytes_get_data( sprites_json, NULL ),
                                                           &error );

            if( error )
            {
                g_warning( "Failed to create spritesheet for vector map style: %s", error->message);
                g_clear_error( &error);
            }

            shumate_map_source_set_license( SHUMATE_MAP_SOURCE( renderer ),
                                            "© OpenMapTiles © OpenStreetMap contributors" );
            shumate_map_source_registry_add( m_registry, SHUMATE_MAP_SOURCE( renderer ) );
        }
    }*/

    m_SVp_map = shumate_simple_map_get_viewport( m_SM_map );

    // LAYERS
    m_path_layer = shumate_path_layer_new( m_SVp_map );
    shumate_simple_map_add_overlay_layer( m_SM_map, SHUMATE_LAYER( m_path_layer ) );

    m_marker_layer_path = shumate_marker_layer_new_full( m_SVp_map, GTK_SELECTION_SINGLE );
    shumate_simple_map_add_overlay_layer( m_SM_map, SHUMATE_LAYER( m_marker_layer_path ) );

    m_marker_layer_entries = shumate_marker_layer_new_full( m_SVp_map, GTK_SELECTION_SINGLE );
    shumate_simple_map_add_overlay_layer( m_SM_map, SHUMATE_LAYER( m_marker_layer_entries ) );

    // CONTROLLERS
    m_EC_key = Gtk::EventControllerKey::create();
    m_EC_key->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetMap::on_key_press_event ), false );
    m_W_map->add_controller( m_EC_key );

    m_gesture_click = Gtk::GestureClick::create();
    m_gesture_click->set_button( 0 );
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &WidgetMap::on_button_press_event ), false );
    m_W_map->add_controller( m_gesture_click );

    m_event_controller_scroll = Gtk::EventControllerScroll::create();
    m_event_controller_scroll->signal_scroll().connect(
            sigc::mem_fun( *this, &WidgetMap::on_scroll_event ), false );
    m_W_map->add_controller( m_event_controller_scroll );

    // m_EC_focus = Gtk::EventControllerFocus::create();
    // m_EC_focus->signal_leave().connect(
    //         [ this ]()
    //         {
    //             if( !m_F_inhibit_deselect )
    //             {
    //                 shumate_marker_layer_unselect_all_markers( m_marker_layer_path );
    //                 m_marker_selected = nullptr;
    //             }
    //         }, false );
    // m_W_map->add_controller( m_EC_focus );

    // POPOVER
    m_builder     = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/map.ui" );
    m_Po_actions  = m_builder->get_widget< Gtk::PopoverMenu >( "Po_actions" );
    m_AG          = Gio::SimpleActionGroup::create();

    m_Po_actions->set_parent( *m_W_map );

    m_A_assign_para_coords = m_AG->add_action(
            "assign_coord_to_para",
            [ this ]()
            {
                if( m_p2para_cur )
                {
                    m_p2para_cur->set_location( m_latitude, m_longitude );
                    refresh_path();
                }
            } );
    m_AG->add_action(
            "copy_coords",
            [ this ]()
            {
                m_W_map->get_clipboard()->set_text(
                        STR::compose( "geo:", m_latitude, ",", m_longitude ) );
            } );

    m_A_add_path_point = m_AG->add_action(
            "add_point",
            [ this ]()
            {
                create_path_point_at_location();
            } );
    m_A_insert_path_point = m_AG->add_action(
            "insert_point",
            [ this ]()
            {
                create_path_point_at_location( false );
            } );
    m_A_remove_path_point = m_AG->add_action(
            "remove_selected_point",
            [ this ]()
            {
                remove_selected_path_point_from_entry();
            } );
    m_A_clear_path = m_AG->add_action(
            "clear_path",
            [ this ]()
            {
                if( m_p2entry_cur ) m_p2entry_cur->clear_map_path();
                clear_path();
            } );

    m_A_show_entry_locations = m_AG->add_action_bool(
            "show_locations",
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ) return;
                const bool F_show {
                        !m_p2diary->get_boolean_option( VT::DO::SHOW_ALL_ENTRY_LOCATIONS::I ) };
                m_p2diary->set_boolean_option( VT::DO::SHOW_ALL_ENTRY_LOCATIONS::I, F_show );
                m_A_show_entry_locations->change_state( F_show );
                refresh_entry_locations();
            },
            true );

    m_Po_actions->insert_action_group( "map", m_AG );
}

void
WidgetMap::show_Po( int x, int y )
{
    const bool editable{ m_p2diary->is_in_edit_mode() };

    m_A_assign_para_coords->set_enabled( editable && m_p2para_cur );
    m_A_add_path_point->set_enabled( editable );
    m_A_insert_path_point->set_enabled( editable && m_p2para_cur );
    m_A_remove_path_point->set_enabled( editable && m_p2para_cur );
    m_A_clear_path->set_enabled( editable && m_p2entry_cur && m_p2entry_cur->has_location() );

    m_Po_actions->set_pointing_to( { x, y, 1, 1 } );
    m_Po_actions->show();
}

void
WidgetMap::refresh_entry_locations()
{
    shumate_marker_layer_remove_all( m_marker_layer_entries );

    if( m_p2diary->get_boolean_option( VT::DO::SHOW_ALL_ENTRY_LOCATIONS::I ) )
    {
        for( auto& kv_entry : m_p2diary->get_entries() )
        {
            Entry* entry{ kv_entry.second };

            if( entry->has_location() && entry != m_p2entry_cur )
                add_entry_point( "map-point-entry", entry->get_name(), entry );
        }
    }
}

void
WidgetMap::handle_para_node_click( int n_click, double x, double y, Paragraph* para )
{
    m_p2para_cur = para;
    refresh_path();
    AppWindow::p->UI_entry->show( para );
}
void
WidgetMap::handle_entry_node_click( int n_click, double x, double y, Entry* entry )
{
    m_F_zoom_on_show = false;
    AppWindow::p->show_entry( entry );
    m_F_zoom_on_show = true;
}

void
WidgetMap::create_entry_at_location()
{
    auto entry{ m_p2diary->create_entry( nullptr, false, Date::get_today(),
                                         _( "New Location" ), VT::ETS::NAME_ONLY::I ) };
    m_p2para_cur = entry->add_map_path_point( m_latitude, m_longitude, nullptr );

    m_F_center_on_show = m_F_zoom_on_show = false;
    AppWindow::p->UI_diary->update_for_added_entry( entry );
    m_F_center_on_show = m_F_zoom_on_show = true;
}

void
WidgetMap::create_path_point_at_location( bool F_after )
{
    if( !m_p2diary->is_in_edit_mode() || !m_p2entry_cur ) return;

    m_p2para_cur = m_p2entry_cur->add_map_path_point( m_latitude, m_longitude, m_p2para_cur,
                                                      F_after );

    refresh_path();
    AppWindow::p->UI_entry->refresh();
    AppWindow::p->UI_entry->show( m_p2para_cur );
}

ShumateMarker*
WidgetMap::add_point( const Coords& pt, const String& icon_name, const Ustring& desc,
                      Glib::RefPtr< Gtk::GestureClick>& gesture, const Ustring& tooltip )
{
    auto marker   { shumate_marker_new() };
    auto image    { Gtk::manage( new Gtk::Image ) };

    image->set_from_icon_name( icon_name );
    image->set_cursor( Gdk::Cursor::create( "pointer" ) );

    if( desc.empty() )  // icon only
    {
        if( !tooltip.empty() )
            image->set_tooltip_text( tooltip );
        shumate_marker_set_child( marker, GTK_WIDGET( image->gobj() ) );
    }
    else
    {
        auto box    { Gtk::manage( new Gtk::Box( Gtk::Orientation::VERTICAL, 0 ) ) };
        auto label  { Gtk::manage( new Gtk::Label( desc ) ) };

        label->add_css_class( "lifeo-map-label" );

        box->append( *label );
        box->append( *image );

        shumate_marker_set_child( marker, GTK_WIDGET( box->gobj() ) );
    }

    image->add_controller( gesture );

    shumate_location_set_location( SHUMATE_LOCATION( marker ), pt.latitude, pt.longitude );

    return marker;
}
ShumateMarker*
WidgetMap::add_entry_point( const String& icon_name, const Ustring& desc, Entry* target )
{
    auto gesture  { Gtk::GestureClick::create() };
    auto marker   { add_point( target->get_location(), icon_name, desc, gesture, "" ) };

    gesture->signal_released().connect(
            sigc::bind( sigc::mem_fun( *this, &WidgetMap::handle_entry_node_click ), target ) );

    shumate_marker_layer_add_marker( m_marker_layer_entries, marker );

    return marker;
}
ShumateMarker*
WidgetMap::add_para_point( const String& icon_name, const Ustring& desc, Paragraph* target )
{
    auto gesture  { Gtk::GestureClick::create() };
    auto marker   { add_point( target->m_location, icon_name, desc, gesture,
                               target->get_text_stripped( VT::TCT_CMPNT_PLAIN ) ) };

    gesture->signal_released().connect(
            sigc::bind( sigc::mem_fun( *this, &WidgetMap::handle_para_node_click ), target ) );

    shumate_marker_layer_add_marker( m_marker_layer_path, marker );
    shumate_path_layer_add_node( m_path_layer, SHUMATE_LOCATION( marker ) );

    return marker;
}

void
WidgetMap::remove_last_path_point_from_entry()
{
    if( !m_p2diary->is_in_edit_mode() || m_p2entry_cur->get_map_path().empty() ) return;

    Paragraph* para_last{ m_p2entry_cur->get_map_path().back() };

    if( !para_last ) return;

    m_p2entry_cur->remove_paragraphs( para_last );
    delete para_last;
    m_p2para_cur = nullptr;

    refresh_path();
}

void
WidgetMap::remove_selected_path_point_from_entry()
{
    if( !m_p2diary->is_in_edit_mode() || !m_p2entry_cur->has_location() || !m_p2para_cur ) return;

    m_p2entry_cur->remove_paragraphs( m_p2para_cur );
    delete m_p2para_cur;
    m_p2para_cur = nullptr;

    refresh_path();
}

void
WidgetMap::refresh_path()
{
    clear_path();

    if( m_p2entry_cur->has_location() )
    {
        unsigned int size{ 0 };
        for( auto lp : m_p2entry_cur->get_map_path() )
        {
            const String icon_name{ lp == m_p2para_cur ? "map-point-selected" : "map-point" };

            if( size == 0 )
                add_para_point( icon_name, m_p2entry_cur->get_name(), lp );
            else if( size == m_p2entry_cur->get_map_path().size() - 1 )
                add_para_point( icon_name, get_path_distance_str(), lp );
            else
                add_para_point( icon_name, "", lp );

            size++;
        }
    }
}

void
WidgetMap::clear_path()
{
    shumate_path_layer_remove_all( m_path_layer );
    shumate_marker_layer_remove_all( m_marker_layer_path );
}

Ustring
WidgetMap::get_path_distance_str()
{
    return( STR::compose( m_p2entry_cur ? m_p2entry_cur->get_map_path_length() : 0.0,
                          Lifeograph::settings.use_imperial_units ? " mi" : " km" ) );
}

void
WidgetMap::set_diary( Diary* d )
{
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_p2diary = d;
    m_A_show_entry_locations
            ->change_state( d->get_boolean_option( VT::DO::SHOW_ALL_ENTRY_LOCATIONS::I ) );

    shumate_viewport_set_zoom_level( m_SVp_map, 2 );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetMap::set_entry( Entry* entry )
{
    m_p2entry_cur = entry;
    m_p2para_cur = entry->get_location_para();

    refresh_entry_locations();
    refresh_path();

    if( m_F_center_on_show && entry->has_location() )
        center_on( entry->get_location().latitude, entry->get_location().longitude );
}

void
WidgetMap::handle_editing_enabled()
{
    // //Not available in shumate: shumate_marker_layer_set_all_markers_draggable( m_marker_layer_path );
    // for( GList* elem = shumate_marker_layer_get_markers( m_marker_layer_path );
    //      elem;
    //      elem = elem->next )
    // {
    //     // does not link for some reason: shumate_marker_set_draggable( SHUMATE_MARKER( elem->data ), true );
    //     shumate_marker_set_selectable( SHUMATE_MARKER( elem->data ), true );
    // }
}

void
WidgetMap::center_on( double lat, double lon )
{
    if( m_F_zoom_on_show || shumate_viewport_get_zoom_level( m_SVp_map ) < 10 )
        shumate_viewport_set_zoom_level( m_SVp_map, 10 );
    shumate_map_center_on( shumate_simple_map_get_map( m_SM_map ), lat, lon );
}

bool
WidgetMap::on_scroll_event( double dx, double dy )
{
    if     ( dy > 0 )
        shumate_map_zoom_in( SHUMATE_MAP( m_SM_map ) );
    else if( dy < 0 )
        shumate_map_zoom_out( SHUMATE_MAP( m_SM_map ) );

    return true;
}

void
WidgetMap::on_button_press_event( int n_press, double x, double y )
{
    shumate_viewport_widget_coords_to_location( m_SVp_map, m_W_map->gobj(),
                                                x, y,
                                                &m_latitude, &m_longitude );

    if( m_gesture_click->get_current_button() == 1 && m_p2diary->is_in_edit_mode() )
    {
        const auto state{ m_gesture_click->get_current_event()->get_modifier_state() };

        if( bool( state & Gdk::ModifierType::CONTROL_MASK ) )
            create_path_point_at_location( not bool( state & Gdk::ModifierType::SHIFT_MASK ) );
        else
        if( bool( state & Gdk::ModifierType::SHIFT_MASK ) )
        {
            if( m_p2para_cur )
            {
                m_p2para_cur->set_location( m_latitude, m_longitude );
                // shumate_location_set_location( SHUMATE_LOCATION( m_marker_selected ),
                //                               m_latitude, m_longitude );
                refresh_path();
            }
        }
        else
        if( bool( state & Gdk::ModifierType::ALT_MASK ) )
            create_entry_at_location();
    }
    else
    if( m_gesture_click->get_current_button() == 3 )
    {
        show_Po( x, y );
    }
}

bool
WidgetMap::on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
{
    switch( keyval )
    {
        case GDK_KEY_BackSpace:
            remove_last_path_point_from_entry();
            return true;
        case GDK_KEY_Delete:
            remove_selected_path_point_from_entry();
            return true;
    }
    return false;
}
