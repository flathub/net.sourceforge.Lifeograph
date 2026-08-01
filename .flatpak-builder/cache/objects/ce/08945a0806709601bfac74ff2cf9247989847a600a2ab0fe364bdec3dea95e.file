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


#ifndef LIFEOGRAPH_WIDGETPICKER_HEADER
#define LIFEOGRAPH_WIDGETPICKER_HEADER


#include <gtkmm.h>

#include "../lifeograph.hpp"
#include "src/helpers.hpp"


namespace LIFEO
{

// WIDGET FOR SELECTING FROM DEFINED FILTERS IN THE DIARY ==========================================
template< class T >
class WidgetPicker : public Gtk::Entry
{
    public:
        WidgetPicker( BaseObjectType* o, const Glib::RefPtr< Gtk::Builder >& )
        : Gtk::Entry( o ) { init(); }
        WidgetPicker()
        { init(); }

        void                        set_map( std::map< Ustring, T*, FuncCmpStrings >* map,
                                             T* const * active )
        {
            m_p2map = map;
            m_p2p2active = active;
            update_editability();
        }

        void                        set_filter_slot( const sigc::slot< bool, T* >& handler )
        {
            handler_filter = handler;
            m_F_filter_slot_active = true;
        }

        void                        set_text( const Ustring& text )
        {
            m_F_internal++;
            Gtk::Entry::set_text( text );
            m_F_internal--;
        }

        void                        set_clearable( bool F_clearable )
        { m_F_clearable = F_clearable; }

        void
        set_show_edit_button( bool F_show_edit )
        { m_F_show_edit = F_show_edit; }
        void
        set_show_add_button( bool F_show_add )
        { m_F_show_add = F_show_add; }

        void                        set_select_only( bool F_select_only )
        {
            m_F_select_only = F_select_only;

            m_B_add->set_visible( m_F_show_add && !F_select_only );

            set_editable( !F_select_only );
        }

        void                        set_modal( bool F_modal )
        { m_Po->set_autohide( F_modal ); }

        void                        clear()
        { set_text( "" ); }
        bool                        update_list()
        {
            if( !m_p2map )
                return false;

            bool         F_exact_match{ false };
            //const auto&& lowercase_filter{ STR::lowercase( m_text_cur ) };
            const auto&& filter_text{ STR::lowercase( m_E_search->get_text() ) };

            clear_list();

            if( m_F_clearable )
                add_none_item_to_list();

            for( auto& kv : *m_p2map )
            {
                if( m_F_filter_slot_active && !handler_filter( kv.second ) )
                    continue;

                if( !filter_text.empty() &&
                    STR::lowercase( kv.first ).find( filter_text ) == Ustring::npos )
                    continue;

//                if( m_F_select_only == false || m_F_ignore_entered_text ||
//                    STR::lowercase( kv.first ).find( lowercase_filter ) != Ustring::npos )
                add_item_to_list( kv.first );
                if( kv.first == m_text_cur )
                {
                    m_LB->select_row( *m_LB->get_row_at_index( m_items.size() - 1 ) );
                    F_exact_match = true;
                }

                scroll_to_selected_LB_row( m_LB );
            }

            return F_exact_match;
        }

        void                        update_editability()
        {
            set_editable( !m_F_select_only && *m_p2p2active != nullptr );
        }

        void                        update_active_item( bool F_hide_Po )
        {
            set_text( *m_p2p2active ? ( *m_p2p2active )->get_name() : "" );
            update_editability();
            if( F_hide_Po )
                m_Po->hide();
            remove_css_class( "error" );
        }

        SignalBoolUstring           signal_name_edited()
        { return m_Sg_name_edited; }
        SignalVoidUstring           signal_sel_changed()
        { return m_Sg_selection_changed; }
        SignalVoid                  signal_add()
        { return m_Sg_add; }
        SignalVoidUstring           signal_duplicate()
        { return m_Sg_duplicate; }
        SignalVoidUstring           signal_dismiss()
        { return m_Sg_dismiss; }
        SignalVoidUstring           signal_edit_item()
        { return m_Sg_edit_item; }

    protected:
        void                        init()
        {
            m_builder       = Lifeograph::create_gui( Lifeograph::SHAREDIR +
                                                      "/ui/widget_picker.ui" );

            m_Po            = m_builder->get_widget< Gtk::Popover >( "Po_picker" );
            auto SW_list    = m_builder->get_widget< Gtk::ScrolledWindow >( "SW_list" );
            m_B_add         = m_builder->get_widget< Gtk::Button >( "B_add" );
            m_E_search      = m_builder->get_widget< Gtk::SearchEntry >( "E_search" );
            m_LB            = Gtk::manage( new Gtk::ListBox );

            set_icon_from_icon_name( "pan-down-symbolic", Gtk::Entry::IconPosition::SECONDARY );

            SW_list->set_child( *m_LB );

            m_Po->set_parent( *this );

            m_LB->show();
            m_B_add->set_visible( m_F_show_add && !m_F_select_only );

            set_editable( !m_F_select_only );

            signal_icon_release().connect(
                    [ this ]( Gtk::Entry::IconPosition pos )
                    {
                        if( pos == Gtk::Entry::IconPosition::SECONDARY && !m_Po->is_visible() )
                        {
                            m_F_ignore_entered_text = true;
                            popup_list();
                            m_F_ignore_entered_text = false;
                        }
                        else
                            m_Po->hide();
                    } );

            m_LB->signal_row_selected().connect(
                    [ this ]( Gtk::ListBoxRow* )
                    {
                        if( m_F_internal ) return;
                        m_Sg_selection_changed.emit( get_selected_from_list() );
                        update_active_item( false );
                    } );
            m_LB->signal_row_activated().connect(
                    [ this ]( Gtk::ListBoxRow* )
                    {
                        if( m_F_internal ) return;
//                        m_Sg_selection_changed.emit( get_selected_from_list() );
//                        update_active_item( true );
                        m_Po->hide();
                    } );

            m_B_add->signal_clicked().connect(
                    [ this ]()
                    {
                        if( !m_p2p2active ) return;
                        m_Sg_add.emit();
                        update_active_item( false );
                        update_list();
                    } );

            m_E_search->signal_changed().connect(
                    [ this ]()
                    {
                        if( m_F_internal ) return;
                        update_list();
                    } );
            m_E_search->signal_activate().connect(
                    [ this ]() { handle_key( GDK_KEY_Return ); } );

            // CONTROLLERS
            auto controller_key{ Gtk::EventControllerKey::create() };
            controller_key->signal_key_pressed().connect(
                    sigc::mem_fun( *this, &WidgetPicker::on_key_press_event ), false );
            add_controller( controller_key );

            auto controller_key_entry { Gtk::EventControllerKey::create() };
            controller_key_entry->signal_key_pressed().connect(
                    sigc::mem_fun( *this, &WidgetPicker::on_key_press_event ), false );
            m_E_search->add_controller( controller_key_entry );
        }

        Gtk::Button*                create_button( const String& icon_name )
        {
            auto button { Gtk::manage( new Gtk::Button ) };
            auto icon   { Gtk::manage( new Gtk::Image ) };

            icon->set_from_icon_name( icon_name );
            button->set_child( *icon );
            //button->set_relief( Gtk::RELIEF_NONE );

            return button;
        }

        void                        add_item_to_list( const Ustring& name )
        {
            auto Bx_item     { Gtk::manage( new Gtk::Box( Gtk::Orientation::HORIZONTAL, 5 ) ) };
            auto L_item_name { Gtk::manage( new Gtk::Label( name, Gtk::Align::START ) ) };

            L_item_name->set_margin_end( 15 ); // this makes it look better
            L_item_name->set_ellipsize( Pango::EllipsizeMode::END );
            L_item_name->set_hexpand( true );

            Bx_item->append( *L_item_name );

            m_LB->append( *Bx_item );

            if( !m_F_select_only )
            {
                auto&& B_dupli{ create_button( "edit-copy-symbolic" ) };
                B_dupli->set_has_frame( false );
                Bx_item->append( *B_dupli );

                B_dupli->signal_clicked().connect(
                        sigc::bind( sigc::mem_fun( *this, &WidgetPicker::handle_item_duplicate ),
                                    name ) );

                if( m_F_show_edit )
                {
                    auto&& B_edit{ create_button( "document-edit-symbolic" ) };
                    B_edit->set_has_frame( false );
                    Bx_item->append( *B_edit );
                    B_edit->signal_clicked().connect(
                            sigc::bind( sigc::mem_fun( *this, &WidgetPicker::handle_item_edit ),
                                        name ) );
                }

                if( m_p2map->size() > 1 )
                {
                    auto&& B_remove{ create_button( "edit-delete-symbolic" ) };
                    B_remove->set_has_frame( false );
                    Bx_item->append( *B_remove );
                    B_remove->signal_clicked().connect(
                            sigc::bind( sigc::mem_fun( *this, &WidgetPicker::handle_item_remove ),
                                        name ) );
                }
            }

            m_items.push_back( name );
        }
        void                        add_none_item_to_list()
        {
            auto L_item_none{ Gtk::manage( new Gtk::Label( "<i>&lt;" + STR0/SI::NONE + "&gt;</i>",
                                                           Gtk::Align::START ) ) };
            L_item_none->set_use_markup( true );
            L_item_none->show();
            m_LB->append( *L_item_none );

            m_items.push_back( "" );
        }
        void                        clear_list()
        {
            m_F_internal++;
            remove_all_children_from_LBx( m_LB );
            m_items.clear();
            m_F_internal--;
        }
        void                        popup_list()
        {
            if( m_p2map == nullptr )
                return;

            m_F_internal++;
            m_E_search->set_text( "" );
            update_list();

            // select the correct row
            const auto text{ get_text() };
            for( unsigned int i = 0; i < m_items.size(); i++ )
            {
                if( m_items[ i ] == text )
                {
                    m_LB->select_row( * m_LB->get_row_at_index( i ) );
                    break;
                }
            }
            m_Po->set_pointing_to( get_icon_area( Gtk::Entry::IconPosition::SECONDARY ) );
            m_Po->show();
            m_E_search->grab_focus();
            m_F_internal--;
        }

        void                        handle_item_duplicate( const Ustring& name )
        {
            m_Sg_duplicate.emit( name );
            update_active_item( false );
            update_list();
        }
        void                        handle_item_edit( const Ustring& name )
        {
            m_Sg_edit_item.emit( name );
        }
        void                        handle_item_remove( const Ustring& name )
        {
            m_Sg_dismiss.emit( name );
            update_active_item( false );
            update_list();
        }
        void                        on_button_press_event( int, double, double )
        {
            if( m_F_select_only && m_Po->is_visible() == false )
            {
                m_F_ignore_entered_text = true;
                popup_list();
                m_F_ignore_entered_text = false;
            }
        }

        void                        on_changed() override
        {
            m_text_cur = get_text();

            if( !m_F_internal && !m_F_select_only )
            {
                if( m_Sg_name_edited.emit( m_text_cur ) )
                    remove_css_class( "error" );
                else
                    add_css_class( "error" );
            }

            Gtk::Entry::on_changed();
        }

        bool
        on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
        {
            if( state == Gdk::ModifierType( 0 ) )
            {
                if( m_Po->is_visible() && handle_key( keyval ) )
                    return true;
            }
            return false;
        }

        Ustring                     get_selected_from_list()
        {
            auto&& row{ m_LB->get_selected_row() };
            return( row ? m_items.at( row->get_index() ) : "" );
        }

        bool                        handle_key( guint keyval )
        {
            switch( keyval )
            {
                case GDK_KEY_Return:
                    m_Sg_selection_changed.emit( get_selected_from_list() );
                    update_active_item( true );
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

            return false;
        }

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;
        Gtk::Popover*               m_Po;
        Gtk::ListBox*               m_LB;
        Gtk::Button*                m_B_add;
        Gtk::Button*                m_B_duplicate;
        Gtk::Button*                m_B_edit;
        Gtk::SearchEntry*           m_E_search;

        std::vector< Ustring >      m_items;
        std::function< bool( T* ) > handler_filter;

        SignalBoolUstring           m_Sg_name_edited;
        SignalVoidUstring           m_Sg_selection_changed;
        SignalVoid                  m_Sg_add;
        SignalVoidUstring           m_Sg_duplicate;
        SignalVoidUstring           m_Sg_dismiss;
        SignalVoidUstring           m_Sg_edit_item;

        const std::map< Ustring, T*, FuncCmpStrings >*
                                    m_p2map                 { nullptr };
        const T* const *            m_p2p2active            { nullptr };
        Ustring                     m_text_cur;

        int                         m_F_internal            { 0 };
        bool                        m_F_clearable           { false };
        bool                        m_F_show_edit           { false };
        bool                        m_F_show_add            { true };
        bool                        m_F_select_only         { false };
        bool                        m_F_ignore_entered_text { false };
        bool                        m_F_filter_slot_active  { false };
};

} // end of namespace LIFEO

#endif
