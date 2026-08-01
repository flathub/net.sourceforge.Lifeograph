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


#ifndef LIFEOGRAPH_WIDGETSELECTOR_HEADER
#define LIFEOGRAPH_WIDGETSELECTOR_HEADER


#include <gtkmm.h>

#include "../lifeograph.hpp"
#include "gtkmm/image.h"
#include "src/helpers.hpp"


namespace LIFEO
{

// WIDGET FOR SELECTING FROM DEFINED FILTERS IN THE DIARY ==========================================
class WidgetSelector : public Gtk::MenuButton
{
    public:
        WidgetSelector( BaseObjectType* o, const Glib::RefPtr< Gtk::Builder >& )
        : Gtk::MenuButton( o )
        {
            init();
        }
        WidgetSelector()
        {
            init();
        }
        ~WidgetSelector()
        {
            m_F_internal = true;
        } // to prevent rogue signals being processed

        void
        insert_elem( const Ustring& name, const FuncVoidInt& handler )
        {
            m_names.push_back( name );
            m_handlers.push_back( handler );
        }

        const Ustring
        get_active_str() const
        {
            if( m_active_i_unfiltered >= 0 && m_active_i_unfiltered < int( m_names.size() ) )
                return m_names[ m_active_i_unfiltered ];
            else
                return "";
        }
        // void
        // set_active( const Ustring& active )
        // {
        //     m_active = active;
        //     refresh_text();
        // }

        void
        set_show_name( bool F_show_name )
        { m_L_name->set_visible( F_show_name ); }

        void
        set_modal( bool F_modal )
        { m_Po->set_autohide( F_modal ); }

        // void                        clear()
        // {
        //     m_active.clear();
        //     m_L_name->set_text( "" );
        //     set_tooltip_text( "" );
        // }
        void
        clear_list()
        {
            clear_list_visual();
            m_handlers.clear();
            m_names.clear();
        }
        void
        clear_list_visual()
        {
            m_F_internal++;
            remove_all_children_from_LBx( m_LB );
            m_map_items.clear();
            m_F_internal--;
        }

        void
        update_list()
        {
            const auto&& search_text_lc { STR::lowercase( m_E_search->get_text() ) };
            int          i_row_sel      { -1 };
            int          num_rows       { 0 };

            clear_list_visual();

            for( unsigned int i = 0; i < m_handlers.size(); i++ )
            {
                if( !search_text_lc.empty() &&
                    STR::lowercase( m_names[ i ] ).find( search_text_lc ) == Ustring::npos )
                    continue;

                add_item_to_list_visual( num_rows, i );

                if( int( i ) == m_active_i_unfiltered )
                    i_row_sel = num_rows;

                num_rows++;
            }

            if( i_row_sel > 0 )
            {
                m_F_internal++;
                m_LB->select_row( *m_LB->get_row_at_index( i_row_sel ) );
                m_F_internal--;
            }
        }

    protected:
        void
        init()
        {
            auto I_icon { Gtk::manage( new Gtk::Image ) };

            I_icon->set_from_icon_name( "list-add-symbolic" );
            set_child( *I_icon );

            m_builder = Gtk::Builder::create();
            Lifeograph::load_gui( m_builder, Lifeograph::SHAREDIR + "/ui/widget_picker.ui" );

            auto B_add  { m_builder->get_widget< Gtk::Button >( "B_add" ) };
            m_Po        = m_builder->get_widget< Gtk::Popover >( "Po_picker" );
            m_LB        = m_builder->get_widget< Gtk::ListBox >( "LB_list" );
            m_E_search  = m_builder->get_widget< Gtk::SearchEntry >( "E_search" );
            m_L_name    = m_builder->get_widget< Gtk::Label >( "L_name" );

            B_add->hide();

            set_popover( *m_Po );

            set_create_popup_func( [ this ]() { update_list(); } );

            // SIGNALS
            // m_LB->signal_row_selected().connect(
            //         [ this ]( Gtk::ListBoxRow* row )
            //         {
            //             if( m_F_internal ) return;
            //         } );
            m_LB->signal_row_activated().connect(
                    [ this ]( Gtk::ListBoxRow* row )
                    {
                        if( m_F_internal ) return;
                        handle_active_item_changed( row->get_index(), true );
                    } );

            m_E_search->signal_changed().connect( [ this ]() { update_list(); } );
            m_E_search->signal_activate().connect(
                    [ this ]()
                    {
                        handle_active_item_changed( get_selected_item_i_filtered(), true );
                    } );

            m_Po->signal_show().connect(
                    [ this ]()
                    {
                        m_E_search->set_text( "" );
                        m_E_search->grab_focus();
                        m_F_internal--;
                    } );
            m_Po->signal_hide().connect( [ this ]() { m_F_internal++; } );

            // CONTROLLERS
            auto controller_key { Gtk::EventControllerKey::create() };
            controller_key->signal_key_pressed().connect(
                    sigc::mem_fun( *this, &WidgetSelector::on_key_press_event ), false );
            add_controller( controller_key );

            auto controller_key_entry { Gtk::EventControllerKey::create() };
            controller_key_entry->signal_key_pressed().connect(
                    sigc::mem_fun( *this, &WidgetSelector::on_key_press_event ), false );
            m_E_search->add_controller( controller_key_entry );
        }

        void
        add_item_to_list_visual( int i_filtered, int i_unfiltered )
        {
            auto Bx_item     { Gtk::manage( new Gtk::Box( Gtk::Orientation::HORIZONTAL, 5 ) ) };
            auto L_item_name { Gtk::manage( new Gtk::Label( m_names[ i_unfiltered ],
                                                            Gtk::Align::START ) ) };

            L_item_name->set_margin_end( 15 ); // this makes it look better
            L_item_name->set_ellipsize( Pango::EllipsizeMode::END );
            L_item_name->set_hexpand( true );

            Bx_item->append( *L_item_name );

            m_LB->append( *Bx_item );
            m_map_items[ i_filtered ] = i_unfiltered;
        }

        // void
        // refresh_text()
        // {
        //     const auto text { m_active > 0 ? m_names[ m_active ] : "-" };
        //     if( m_L_name->get_visible() )
        //         m_L_name->set_text( text );
        //     else
        //         set_tooltip_text( text );
        // }

        int
        unfilter_i( const int i_filtered )
        {
            auto kv { m_map_items.find( i_filtered ) };
            return( kv == m_map_items.end() ? -1 : kv->second );
        }

        int
        get_selected_item_i_filtered()
        {
            auto&& row{ m_LB->get_selected_row() };
            return( row ? row->get_index() : -1 );
        }

        bool
        on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
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
                            handle_active_item_changed( get_selected_item_i_filtered(), true );
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
        handle_active_item_changed( const int i_filtered, bool F_hide_Po )
        {
            m_active_i_unfiltered = unfilter_i( i_filtered );

            // refresh_text();

            if( F_hide_Po )
                m_Po->hide();

            const FuncVoidInt& handler { m_handlers.at( m_active_i_unfiltered ) };
            handler( m_active_i_unfiltered );
        }

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;
        Gtk::Label*                 m_L_name;
        Gtk::Popover*               m_Po;
        Gtk::ListBox*               m_LB;
        Gtk::SearchEntry*           m_E_search;

        std::map< int, int >        m_map_items;
        VecUstrings                 m_names;
        std::vector< FuncVoidInt >  m_handlers;
        int                         m_active_i_unfiltered { 0 };

        int                         m_F_internal          { 1 };
};

} // end of namespace LIFEO

#endif
