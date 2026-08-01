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


#include "../lifeograph.hpp"
#include "../diaryelements/entry.hpp"
#include "widget_datepicker.hpp"


using namespace LIFEO;

// TAG WIDGET ======================================================================================
WidgetDatePicker::WidgetDatePicker( BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& b )
: Gtk::Entry( o )
{
    initialize();
}

WidgetDatePicker::WidgetDatePicker()
{
    initialize();
}

WidgetDatePicker::~WidgetDatePicker()
{
    // following creates problems for some reason:
    //if( m_Po )
    //    delete m_Po;
}

void
WidgetDatePicker::initialize()
{
    m_Po = new Gtk::Popover();
    m_W_calendar = Gtk::manage( new WidgetCalendar );

    m_Po->set_parent( *this );
    m_W_calendar->set_margin( 5 );
    m_Po->set_child( *m_W_calendar );

    m_W_calendar->show();

    set_icon_from_icon_name( "x-office-calendar-symbolic", Gtk::Entry::IconPosition::SECONDARY );

    // SIGNALS
    signal_icon_release().connect( [ this ]( Gtk::Entry::IconPosition ){
            m_Po->is_visible() ? popdown_cal() : popup_cal(); } );

    m_W_calendar->signal_day_selected().connect( [ this ]( DateV date ){
        set_date( date );
        popdown_cal();
        m_Sg_date_set.emit( m_date ); } );

    // CONTROLLERS
    auto controller_key{ Gtk::EventControllerKey::create() };
    controller_key->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetDatePicker::on_key_press_event ), false );
    add_controller( controller_key );

    m_drop_target = Gtk::DropTarget::create( LIFEO::Entry::GValue::value_type(),
                                             Gdk::DragAction::COPY );
    // m_drop_target->signal_motion().connect(
    //         sigc::mem_fun( *this, &EntryPickerCompletion::handle_drop_motion ), false );
    m_drop_target->signal_drop().connect(
            sigc::mem_fun( *this, &WidgetDatePicker::handle_drop ), false );
    add_controller( m_drop_target );
}

void
WidgetDatePicker::set_date( DateV date )
{
    if( Date::is_valid( date ) )
    {
        Lifeograph::START_INTERNAL_OPERATIONS();
        Gtk::Entry::set_text( Date::format_string( date ) );
        remove_css_class( "error" );
        m_W_calendar->set_day_highlighted( date );
        m_date = date;
        Lifeograph::FINISH_INTERNAL_OPERATIONS();
    }
}

void
WidgetDatePicker::on_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    const Ustring text_new{ get_text() };
    bool flag_ok{ false };

    // empty
    if( text_new.empty() )
    {
        m_date = Date::NOT_SET;
    }
    else
    {
        m_date = Date::make( text_new );
        if( Date::is_set( m_date ) )
        {
            m_W_calendar->set_date( m_date );
            flag_ok = true;
        }
    }

    m_Sg_date_set.emit( m_date );

    if( flag_ok )
        remove_css_class( "error" );
    else
        add_css_class( "error" );

    Gtk::Entry::on_changed();
}

bool
WidgetDatePicker::on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
{
    // if( ( state & Gdk::ModifierType::CONTROL_MASK ) != Gdk::ModifierType::CONTROL_MASK &&
    //     ( state & Gdk::ModifierType::ALT_MASK ) != Gdk::ModifierType::ALT_MASK &&
    //     ( state & Gdk::ModifierType::SHIFT_MASK ) != Gdk::ModifierType::SHIFT_MASK )
    if( state == Gdk::ModifierType( 0 ) )
    {
        switch( keyval )
        {
            //case GDK_KEY_Return:
            case GDK_KEY_Escape:
                popdown_cal();
                return true;
            case GDK_KEY_Down:
            case GDK_KEY_Up:
                popup_cal();
                return true;
        }
    }
    return false;
}

Gdk::DragAction
WidgetDatePicker::handle_drop_motion( double x, double y )
// TODO: 3.1: gtkmm4: this function is probably no longer needed
{
    // if( Lifeograph::get_dragged_elem() != nullptr )
    //     if( Lifeograph::get_dragged_elem()->get_type() >= DiaryElement::ET_ENTRY )
    //     {
            //drag_highlight();
            return Gdk::DragAction::COPY;
        // }

    //drag_unhighlight();  // is called many times unnecessarily :(
    //context->drag_refuse( time );
    return Gdk::DragAction( 0 );
}

bool
WidgetDatePicker::handle_drop( const Glib::ValueBase& value, double x, double y )
{
    if( G_VALUE_HOLDS( value.gobj(), LIFEO::Entry::GValue::value_type() ) )
    {
        LIFEO::Entry::GValue value_entry;
        value_entry.init( value.gobj() );

        set_date( value_entry.get()->get_date() );
        m_Sg_date_set.emit( m_date );
        return true;
    }

    return false;
}

void
WidgetDatePicker::popup_cal()
{
    m_Po->set_pointing_to( get_icon_area( Gtk::Entry::IconPosition::SECONDARY ) );
    m_Po->show();
}

void
WidgetDatePicker::popdown_cal()
{
    m_Po->hide();
}
