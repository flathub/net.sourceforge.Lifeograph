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


#include "widget_filterpicker.hpp"
#include "src/helpers.hpp"


using namespace LIFEO;

// FILTER PICKER WIDGET =============================================================================
void
WidgetFilterPicker::update_list()
{
    if( !m_p2diary ) return;

    m_p2diary->update_filter_user_counts();

    const auto&& search_text_lc { STR::lowercase( m_E_search->get_text() ) };
    int          i_row_sel      { 0 };

    clear_list();

    m_B_add->set_visible( m_p2diary->is_in_edit_mode() && !m_F_select_only );

    if( m_F_clearable )
        add_all_item_to_list();

    {
        auto filter{ Diary::d->get_filter_trashed() };
        add_default_item_to_list( filter );

        if( filter == m_p2active )
            i_row_sel = ( m_items.size() - 1 );
    }

    if( m_F_show_cur_entry )
    {
        auto filter{ Diary::d->get_filter_cur_entry() };
        add_default_item_to_list( filter );

        if( filter == m_p2active )
            i_row_sel = ( m_items.size() - 1 );

        filter = Diary::d->get_filter_cur_entry_tree();
        add_default_item_to_list( filter );

        if( filter == m_p2active )
            i_row_sel = ( m_items.size() - 1 );
    }

    for( auto& kv : m_p2diary->get_filters() )
    {
        if( ! kv.second->can_filter_class( m_obj_classes ) )
            continue;

        if( !search_text_lc.empty() &&
            STR::lowercase( kv.first ).find( search_text_lc ) == Ustring::npos )
            continue;

        add_item_to_list( kv.second );

        if( kv.second == m_p2active )
            i_row_sel = ( m_items.size() - 1 );
    }

    m_F_internal++;
    m_LB->select_row( *m_LB->get_row_at_index( i_row_sel ) );
    m_F_internal--;

    scroll_to_selected_LB_row( m_LB );
}

void
WidgetFilterPicker::init()
{
    Gtk::Box*            Bx_button;

    m_builder = Gtk::Builder::create();
    Lifeograph::load_gui( m_builder, Lifeograph::SHAREDIR + "/ui/widget_picker.ui" );
    Lifeograph::load_gui( m_builder, Lifeograph::SHAREDIR + "/ui/widget_filter_picker.ui" );

    m_Po        = m_builder->get_widget< Gtk::Popover >( "Po_picker" );
    m_LB        = m_builder->get_widget< Gtk::ListBox >( "LB_list" );
    m_B_add     = m_builder->get_widget< Gtk::Button >( "B_add" );
    m_E_search  = m_builder->get_widget< Gtk::SearchEntry >( "E_search" );
    Bx_button   = m_builder->get_widget< Gtk::Box >( "Bx_button" );
    m_L_name    = m_builder->get_widget< Gtk::Label >( "L_name" );

    set_child( *Bx_button );

    set_popover( *m_Po );

    set_create_popup_func( [ this ]() { update_list(); } );

    // SIGNALS
    m_LB->signal_row_selected().connect(
            [ this ]( Gtk::ListBoxRow* )
            {
                if( m_F_internal ) return;
                handle_active_item_changed( get_selected_from_list(), false );
            } );
    m_LB->signal_row_activated().connect(
            [ this ]( Gtk::ListBoxRow* )
            {
                if( m_F_internal ) return;
                m_Po->hide();
            } );

    m_B_add->signal_clicked().connect(
            [ this ]()
            {
                handle_active_item_changed( m_p2diary->create_filter( _( "New Filter" ) ), false );
                update_list();
                handle_item_edit( m_p2active->get_id() );
            } );

    m_E_search->signal_changed().connect( [ this ]() { update_list(); } );

    m_Po->signal_show().connect( [ this ]() { m_E_search->grab_focus(); m_F_internal--; } );
    m_Po->signal_hide().connect( [ this ]() { m_F_internal++; } );

    // CONTROLLERS
    auto controller_key { Gtk::EventControllerKey::create() };
    controller_key->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetFilterPicker::on_key_press_event ), false );
    add_controller( controller_key );

    auto controller_key_entry { Gtk::EventControllerKey::create() };
    controller_key_entry->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetFilterPicker::on_key_press_event ), false );
    m_E_search->add_controller( controller_key_entry );

    auto gesture_click { Gtk::GestureClick::create() };
    gesture_click->set_button( 3 );
    gesture_click->signal_released().connect(
            sigc::mem_fun( *this, &WidgetFilterPicker::on_button_release_event ), false );
    add_controller( gesture_click );
}

Gtk::Button*
WidgetFilterPicker::add_button( Gtk::Box* box, const String& icon_name )
{
    auto button { Gtk::manage( new Gtk::Button ) };
    auto icon   { Gtk::manage( new Gtk::Image ) };

    icon->set_from_icon_name( icon_name );
    button->set_child( *icon );
    button->set_has_frame( false );

    box->append( *button );

    return button;
}

void
WidgetFilterPicker::add_item_to_list( Filter* filter )
{
    const auto& name          { filter->get_name() };
    const auto& id            { filter->get_id() };
    auto        Bx_item       { Gtk::manage( new Gtk::Box( Gtk::Orientation::HORIZONTAL, 1 ) ) };
    auto        L_item_name   { Gtk::manage( new Gtk::Label( name, Gtk::Align::START ) ) };
    auto        L_user_count  { Gtk::manage( new Gtk::Label( std::to_string( filter->m_num_users ),
                                                             Gtk::Align::START ) ) };

    L_item_name->set_hexpand( true );
    L_item_name->set_margin_end( 15 ); // this makes it look better
    L_item_name->set_ellipsize( Pango::EllipsizeMode::END );
    L_user_count->set_sensitive( false );
    L_user_count->set_margin_end( 8 ); // this makes it look better

    Bx_item->append( *L_item_name );
    Bx_item->append( *L_user_count );

    if( m_p2diary->is_in_edit_mode() && !m_F_select_only )
    {
        if( m_p2diary->get_filters().size() > 1 )
        {
            add_button( Bx_item, "edit-delete-symbolic" )->signal_clicked().connect(
                    sigc::bind( sigc::mem_fun( *this, &WidgetFilterPicker::handle_item_dismiss ),
                                id ) );
        }

        if( m_F_show_edit )
        {
            add_button( Bx_item, "document-edit-symbolic" )->signal_clicked().connect(
                    sigc::bind( sigc::mem_fun( *this, &WidgetFilterPicker::handle_item_edit ),
                                id ) );
        }

        add_button( Bx_item, "edit-copy-symbolic" )->signal_clicked().connect(
                sigc::bind( sigc::mem_fun( *this, &WidgetFilterPicker::handle_item_duplicate ),
                            id ) );
    }

    m_LB->append( *Bx_item );

    m_items.push_back( filter );
}
void
WidgetFilterPicker::add_all_item_to_list()
{
    add_default_item_to_list_2(
            Gtk::manage( new Gtk::Label( "<i>" +
                                             Glib::Markup::escape_text( get_all_items_text() ) +
                                         "</i>",
                                         Gtk::Align::START ) ),
            nullptr );
}
void
WidgetFilterPicker::add_default_item_to_list( const Filter* filter )
{
    add_default_item_to_list_2(
            Gtk::manage(
                    new Gtk::Label( "<i>" +
                                        Glib::Markup::escape_text( filter->get_name() ) +
                                    "</i>",
                    Gtk::Align::START ) ),
            filter );
}
inline void
WidgetFilterPicker::add_default_item_to_list_2( Gtk::Label* L_item_def, const Filter* filter )
{
    L_item_def->set_use_markup( true );
    L_item_def->show();
    m_LB->append( *L_item_def );

    m_items.push_back( filter );
}

void
WidgetFilterPicker::clear_list()
{
    m_F_internal++;
    remove_all_children_from_LBx( m_LB );
    m_items.clear();
    m_F_internal--;
}

void
WidgetFilterPicker::handle_item_duplicate( DEID id )
{
    if( !m_p2diary ) return;

    const auto&& name { m_p2diary->get_filter( id )->get_name() };
    handle_active_item_changed( m_p2diary->duplicate_filter( name ), false );
    update_list();
    handle_item_edit( m_p2active->get_id() );
}
void
WidgetFilterPicker::handle_item_edit( DEID id )
{
    if( !m_p2diary ) return;

    auto&& builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/dlg_filter.ui" ) };
    auto&& filter  { m_p2diary->get_filter( id ) };
    m_D_filter = Gtk::Builder::get_widget_derived< DialogFilter >( builder, "D_filter",
                                                                   m_obj_classes | FOC::SUBGROUPS,
                                                                   filter );

    if( m_F_hide_popover_during_edit && !m_F_hide_popover_force_disable ) m_Po->hide();
    Sg_hide_my_Po.emit();

    m_D_filter->signal_hide().connect(
        std::bind( &WidgetFilterPicker::handle_dialog_response, this, filter ) );

    m_D_filter->show();
}
void
WidgetFilterPicker::handle_dialog_response( Filter* filter )
{
    if( m_D_filter->get_result() == Result::OK )
        handle_active_item_changed( filter, false );

    m_D_filter->hide();

    if( m_F_hide_popover_during_edit && !m_F_hide_popover_force_disable )
        m_Po->show();
    else
        m_F_hide_popover_force_disable = false;
}

void
WidgetFilterPicker::handle_item_dismiss( DEID id )
{
    const Ustring&& name_cur{ m_p2active ? m_p2active->get_name() : "" };
    const Ustring&& name_del{ m_p2diary->get_filter( id )->get_name() };
    if( m_p2diary->dismiss_filter( name_del ) && name_cur == name_del )
    {
        handle_active_item_changed( nullptr, false );
    }
    update_list();
}

bool
WidgetFilterPicker::on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
{
    // if( ( state & ( Gdk::ModifierType::CONTROL_MASK |
    //                 Gdk::ModifierType::ALT_MASK |
    //                 Gdk::ModifierType::SHIFT_MASK ) ) == Gdk::ModifierType( 0 ) )
    if( state == Gdk::ModifierType( 0 ) )
    {
        if( m_Po->is_visible() )
        {
            switch( keyval )
            {
                case GDK_KEY_Return:
                    handle_active_item_changed( get_selected_from_list(), true );
                    return true;
                case GDK_KEY_Escape:
                    m_Po->hide();
                    return true;
                case GDK_KEY_Down:
                    select_LB_item_next( m_LB );
                    return true;
                case GDK_KEY_Up:
                    select_LB_item_prev( m_LB );
                    return true;
            }
        }
    }
    return false;
}

void
WidgetFilterPicker::on_button_release_event( int n_press, double x, double y )
{
    if( m_p2active && m_p2diary->is_in_edit_mode()  )
    {
        m_F_hide_popover_force_disable = true;
        handle_item_edit( m_p2active->get_id() );
    }
    else
        print_info( "No item is selected!" );
}

void
WidgetFilterPicker::handle_active_item_changed( const Filter* filter, bool F_hide_Po )
{
    m_p2active = filter;

    refresh_text();

    if( F_hide_Po )
        m_Po->hide();

    // remove_css_class( "error" );

    Sg_changed.emit( filter );

    update_tooltip();
}

void
WidgetFilterPicker::update_tooltip()
{
    Ustring text{ m_p2active ? m_p2active->get_name() : get_all_items_text() };

    if( m_p2active && m_no_of_in_elems >= 0 )
        text += STR::compose( "  (", m_no_of_in_elems, " / ", m_p2diary->get_size(), ")" );
    else if( !m_p2active ) // show the total number only when nothing is selected
        text += STR::compose( "  (", m_p2diary->get_size(), ")" );

    if( m_p2active && m_p2diary->is_in_edit_mode() )
        text += STR::compose( '\n', _( "Right click to quick edit the selected filter" ) );

    set_tooltip_text( text );
}
