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


#include <cairomm/context.h>

#include "../strings.hpp"
#include "../diaryelements/diarydata.hpp"
#include "../app_window.hpp"
#include "widget_calendar.hpp"


using namespace LIFEO;

// WIDGET CALENDAR =================================================================================
WidgetCalendar::WidgetCalendar()
{
    init();
}
WidgetCalendar::WidgetCalendar( BaseObjectType* cobj,
                                const Glib::RefPtr< Gtk::Builder >& refbuilder )
:   Gtk::DrawingArea( cobj )
{
    init();
}

void
WidgetCalendar::init()
{
    set_size_request( 360, 220 );

    m_AG      = Gio::SimpleActionGroup::create();

    m_builder = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/calendar.ui" );

    m_Po_day  = m_builder->get_widget< Gtk::PopoverMenu >( "Po_calendar_day" );
    m_Bx_add  = m_builder->get_widget< Gtk::Box >( "Bx_calendar_add" );

    m_Po_day->set_parent( *this );

    m_Po_day->signal_closed().connect( [ this ](){ m_F_selection_locked = false; } );

    m_A_copy = m_AG->add_action(
            "copy_date",
            [ this ]() { get_clipboard()->set_text( Date::format_string( m_day_hovered ) ); } );

    m_A_weekend = m_AG->add_action_bool(
            "day_is_weekend",
            [ this ](){ handle_weekend_toggled(); },
            false );
    m_A_holiday = m_AG->add_action_bool(
            "day_is_holiday",
            [ this ](){ handle_holiday_toggled(); },
            false );
    m_A_show_week_nos = m_AG->add_action_bool(
            "show_week_nos",
            [ this ](){ handle_week_nos_toggled(); },
            false );

    insert_action_group( "calendar", m_AG );

    // RESIZING & DRAWING
    set_draw_func( sigc::mem_fun( *this, &WidgetCalendar::on_draw ) );

    // EVENT CONTROLLERS
    m_event_controller_scroll = Gtk::EventControllerScroll::create();
    m_event_controller_scroll->signal_scroll().connect(
            sigc::mem_fun( *this, &WidgetCalendar::on_scroll_event ), false );
    m_event_controller_scroll->set_flags( Gtk::EventControllerScroll::Flags::VERTICAL );
    add_controller( m_event_controller_scroll );

    m_gesture_click = Gtk::GestureClick::create();
    m_gesture_click->set_button( 0 );
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &WidgetCalendar::on_button_press_event ), false );
    m_gesture_click->signal_released().connect(
            sigc::mem_fun( *this, &WidgetCalendar::on_button_release_event ), false );
    add_controller( m_gesture_click );

    auto controller_motion = Gtk::EventControllerMotion::create();
    controller_motion->signal_motion().connect(
            sigc::mem_fun( *this, &WidgetCalendar::on_motion_notify_event ), false );
    controller_motion->signal_leave().connect(
            sigc::mem_fun( *this, &WidgetCalendar::on_leave_notify_event ), false );
    add_controller( controller_motion );

    m_drag_source = Gtk::DragSource::create();
    m_drag_source->signal_drag_begin().connect(
        sigc::mem_fun( *this, &WidgetCalendar::handle_drag_begin ), false );
    m_drag_source->signal_prepare().connect(
        sigc::mem_fun( *this, &WidgetCalendar::handle_drag_prepare ), false );
    m_drag_source->signal_drag_end().connect(
        sigc::mem_fun( *this, &WidgetCalendar::handle_drag_end ), false );
    add_controller( m_drag_source );

    m_drop_target = Gtk::DropTarget::create( Glib::Value< Ustring >::value_type(),
                                             Gdk::DragAction::COPY );
    m_drop_target->signal_motion().connect(
            sigc::mem_fun( *this, &WidgetCalendar::handle_drop_motion ), false );
    m_drop_target->signal_drop().connect(
            sigc::mem_fun( *this, &WidgetCalendar::handle_drop ), false );
    // is added in set_editable()

    m_month = Date::get_today();
    Date::set_day( m_month, 1 );
}

void
WidgetCalendar::set_editable( bool F_editable )
{
    if( F_editable == m_F_editable )
        return;

    if( !m_F_editable )
        add_controller( m_drop_target );
    else
        remove_controller( m_drop_target );

    m_F_editable = F_editable;
}

void
WidgetCalendar::handle_weekend_toggled()
{
    Diary::d->set_day_weeekend(
            m_column_hovered + Date::s_week_start_day,
            !Diary::d->is_day_weekend( m_column_hovered + Date::s_week_start_day ) );

    refresh();
}
void
WidgetCalendar::handle_holiday_toggled()
{
    Diary::d->set_day_holiday( m_day_hovered, !Diary::d->is_day_holiday( m_day_hovered ) );

    refresh();
}

void
WidgetCalendar::handle_week_nos_toggled()
{
    Diary::d->set_boolean_option( VT::DO::SHOW_WEEK_NOS::I,
                                  !Diary::d->get_boolean_option( VT::DO::SHOW_WEEK_NOS::I ) );

    update_col_geom();
    refresh();
}

void
WidgetCalendar::show_Po( int cell_x, int cell_y )
{
    Gdk::Rectangle rect{ int( s_margin + m_col_width * cell_x ),
                         int( s_grid_start_y +
                              ( m_column_hovered > -1 ? -s_day_bar_height
                                                      : m_row_height * cell_y ) ),
                         int( m_col_width ), int( m_row_height ) };

    m_F_selection_locked = true;

    m_A_weekend->set_state(
            Glib::Variant< bool >::create(
                    Diary::d->is_day_weekend( m_column_hovered + Date::s_week_start_day ) ) );
    m_A_holiday->set_state(
            Glib::Variant< bool >::create( Diary::d->is_day_holiday( m_day_hovered ) ) );
    m_A_show_week_nos->set_state(
            Glib::Variant< bool >::create(
                    Diary::d->get_boolean_option( VT::DO::SHOW_WEEK_NOS::I ) ) );

    m_Bx_add->set_visible( m_F_editable && m_column_hovered == -1 );
    m_A_copy->set_enabled( m_column_hovered == -1 );
    m_A_weekend->set_enabled( m_F_editable && m_column_hovered != -1 );
    m_A_holiday->set_enabled( m_F_editable && m_column_hovered == -1 );

    m_Po_day->set_pointing_to( rect );
    m_Po_day->show();
}

void
WidgetCalendar::set_selected_day_from_coords( int x, int y )
{
    m_col_active = int( floor( ( x - s_margin ) / m_col_width ) );
    m_row_active = int( floor( ( y - s_grid_start_y ) / m_row_height ) );

    DateV date{ m_month };

    if( x >= s_margin && x <= s_margin + 16 && y >= s_margin && y <= s_margin + 16 )
        date = Date::BCK_MON;
    else
    if( x >= s_margin + 25 && x <= s_margin + 46 && y >= s_margin && y <= s_margin + 16 )
        date = Date::THS_MON;
    else
    if( x >= s_margin + 50 && x <= s_margin + 76 && y >= s_margin && y <= s_margin + 16 )
        date = Date::FWD_MON;
    else
    if( m_col_active < 0 || m_row_active < 0 || m_col_active > 6 || m_row_active > 5 )
        date = Date::NOT_SET;
    else
    {
        calculate_begin_date( date );
        Date::forward_days( date, m_row_active * 7 + m_col_active );
    }

    const int column = { ( m_col_active < 0 || m_col_active > 6 ||
                           y < s_grid_start_y - s_day_bar_height ||
                           y > s_grid_start_y ) ? -1 : m_col_active };

    if( date != m_day_hovered || column != m_column_hovered )
    {
        m_day_hovered = date;
        m_column_hovered = column;
        refresh();
    }
}

Gdk::Rectangle
WidgetCalendar::get_selected_rectangle() const
{
    return( Gdk::Rectangle( s_margin + m_col_active * m_col_width,
                            s_grid_start_y + m_row_active * m_row_height,
                            m_col_width, m_row_height ) );
}

bool
WidgetCalendar::on_scroll_event( double dx, double dy )
{
    const bool F_ctrl_pressed{
            bool( m_event_controller_scroll->get_current_event()->get_modifier_state() &
                  Gdk::ModifierType::CONTROL_MASK ) };

    if( dy < 0 )
    {
        Date::backward_months( m_month, F_ctrl_pressed ? 12  : 1 );
        Date::set_day( m_month, 1 ); // should always point to day 1
        refresh();
    }
    else
    if( dy > 0 )
    {
        Date::forward_months( m_month, F_ctrl_pressed ? 12  : 1 );
        refresh();
    }

    return true;
}

void
WidgetCalendar::on_button_press_event( int n_press, double x, double y )
{
    if( n_press == 2 && Date::is_legit( m_day_hovered ) && m_F_editable )
    {
        PRINT_DEBUG( "double_click" );
        Lifeograph::p->activate_action( "add_entry_dated" );
    }
    else
    if( m_gesture_click->get_current_button() == 3 )
    {
        if( Date::is_legit( m_day_hovered ) || ( m_F_editable && m_column_hovered != -1 ) )
            show_Po( int( floor( ( x - s_margin ) / m_col_width ) ),
                        int( floor( ( y - s_grid_start_y ) / m_row_height ) ) );

    }
}

void
WidgetCalendar::on_button_release_event( int n_press, double x, double y )
{
    if( n_press == 1 && m_gesture_click->get_current_button() == 1 )
    {
        const bool F_ctrl_pressed{
                bool( m_gesture_click->get_current_event()->get_modifier_state() &
                      Gdk::ModifierType::CONTROL_MASK ) };

        switch( m_day_hovered )
        {
            case Date::NOT_SET:
                break;
            case Date::BCK_MON:
                Date::backward_months( m_month, F_ctrl_pressed ? 12  : 1 );
                Date::set_day( m_month, 1 ); // should always point to day 1
                refresh();
                break;
            case Date::FWD_MON:
                Date::forward_months( m_month, F_ctrl_pressed ? 12  : 1 );
                refresh();
                break;
            case Date::THS_MON:
                m_month = Date::get_today();
                refresh();
                break;
            default:
                m_Sg_day_selected.emit( m_day_hovered );
                break;
        }
    }
}

void
WidgetCalendar::on_motion_notify_event( double x, double y )
{
    if( !m_F_selection_locked )
        set_selected_day_from_coords( x, y );
}

void
WidgetCalendar::on_leave_notify_event()
{
    if( !m_F_selection_locked )
    {
        m_day_hovered = Date::NOT_SET;
        m_column_hovered = -1;
        PRINT_DEBUG( "RESET m_column_hovered" );
        refresh();
    }
}

void
WidgetCalendar::on_resize( int w, int h )
{
    auto    layout{ Pango::Layout::create(  get_pango_context() ) };
    int     dummy_h;
    layout->set_text( "0" );
    layout->set_font_description( m_font_bold );
    layout->get_pixel_size( m_unit_text_w, dummy_h );

    resize( w, h );

    // update_col_geom();
}

void
WidgetCalendar::on_draw( const Cairo::RefPtr< Cairo::Context >& cr, int w, int h )
{
    if( Diary::d->is_open() ) Calendar::draw( cr );
}

void
WidgetCalendar::handle_drag_begin( const Glib::RefPtr< Gdk::Drag >& drag )
{
    if( Date::is_valid( m_day_hovered ) )
    {
        m_F_selection_locked = true;
        m_drag_source->set_icon( lookup_default_icon( "x-office-calendar-symbolic", 32 ), 0, 0 );
    }
    else
    {
        m_drag_source->drag_cancel(); // not ideal but kind of works
        PRINT_DEBUG( "Drag denied" );
        return;
    }
}

Glib::RefPtr< Gdk::ContentProvider >
WidgetCalendar::handle_drag_prepare( double x, double y )
{
    Glib::Value< Ustring > value_str;
    value_str.init( Glib::Value< Ustring >::value_type() );
    value_str.set( Date::format_string( m_day_hovered ) );

    return Gdk::ContentProvider::create( value_str );
}

void
WidgetCalendar::handle_drag_end( const Glib::RefPtr< Gdk::Drag >& drag, bool )
{
    m_F_selection_locked = false;
}

Gdk::DragAction
WidgetCalendar::handle_drop_motion( double x, double y )
{
    if( m_F_selection_locked == false /*&& drag_dest_find_target( ctx ) != "NONE"*/ )
    {
        set_selected_day_from_coords( x, y );

        if( Date::is_legit( m_day_hovered ) )
        {
            //ctx->drag_status( ctx->get_suggested_action(), time );
            // below flag is needed to prevent double receiving of drag data, this is all we know:
            m_F_expect_drag_data = true;
            return Gdk::DragAction::COPY;
        }
        else
            m_F_expect_drag_data = false;
    }

    return Gdk::DragAction( 0 );
}

bool
WidgetCalendar::handle_drop( const Glib::ValueBase& value, double x, double y )
{
    if( !m_F_expect_drag_data || !Date::is_legit( m_day_hovered ) )
        return false;

    if( G_VALUE_HOLDS( value.gobj(), Glib::Value< Ustring >::value_type() ) )
    {
        Glib::Value< Ustring > value_ustring;
        value_ustring.init( value.gobj() );

        m_F_expect_drag_data = false;
        m_Sg_day_add.emit( m_day_hovered, value_ustring.get() );
        return true;
    }

    return false;
}
