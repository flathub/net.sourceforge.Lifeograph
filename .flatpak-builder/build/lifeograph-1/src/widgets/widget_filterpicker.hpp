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


#ifndef LIFEOGRAPH_WIDGETFILTERPICKER_HEADER
#define LIFEOGRAPH_WIDGETFILTERPICKER_HEADER


#include <gtkmm.h>

#include "../diaryelements/diary.hpp"
#include "../dialogs/dialog_filter.hpp"


namespace LIFEO
{

using VecFilters        = std::vector< const Filter* >;
using SignalVoidFilter  = sigc::signal< void( const Filter* ) >;

// WIDGET FOR SELECTING FROM DEFINED FILTERS IN THE DIARY ==========================================
class WidgetFilterPicker : public Gtk::MenuButton
{
    public:
        WidgetFilterPicker( BaseObjectType* o, const Glib::RefPtr< Gtk::Builder >& )
        : Gtk::MenuButton( o ) { init(); }
        WidgetFilterPicker()
        { init(); }
        ~WidgetFilterPicker()
        {
            m_F_internal = true;  // to prevent rogue signals being processed
        }

        void                        set_diary( Diary* p2diary )
        { m_p2diary = p2diary; }

        const Filter*               get_active() const
        { return m_p2active; }
        void                        set_active( const Filter* filter )
        {
            m_p2active = filter;
            refresh_text();
        }

        void                        set_obj_classes( int oc )
        { m_obj_classes = oc; }

        void                        set_clearable( bool F_clearable )
        { m_F_clearable = F_clearable; }

        void                        set_show_edit_button( bool F_show_edit )
        { m_F_show_edit = F_show_edit; }

        void                        set_show_cur_entry( bool F_show )
        { m_F_show_cur_entry = F_show; }

        void                        set_show_name( bool F_show_name )
        { m_L_name->set_visible( F_show_name ); }

        void                        set_hide_popover_during_edit( bool F_hide_popover )
        { m_F_hide_popover_during_edit = F_hide_popover; }

        void                        set_modal( bool F_modal )
        { m_Po->set_autohide( F_modal ); }

        void                        clear()
        {
            m_L_name->set_text( "" );
            set_tooltip_text( "" );
        }

        void                        update_list();

        void                        set_no_of_filtered_in_items( int no )
        {
            m_no_of_in_elems = no;
            update_tooltip();
        }

        SignalVoidFilter            Sg_changed;
        // try to hide the parent Popover to prevent parenting issues in Gtk:
        SignalVoid                  Sg_hide_my_Po;

    protected:
        void                        init();

        Gtk::Button*                add_button( Gtk::Box*, const String& icon_name );
        void                        add_item_to_list( Filter* );
        void                        add_all_item_to_list();
        void                        add_default_item_to_list( const Filter* );
        void                        add_default_item_to_list_2( Gtk::Label*, const Filter* );

        void                        clear_list();

        Ustring                     get_all_items_text() const
        // TODO: 3.1: we need to change the name to ALL PARAGRAPHS for paragraph-only situations
        { return STR::compose( '<', STR0/SI::ALL_ENTRIES, '>' ); }

        void                        refresh_text()
        {
            const auto text{ m_p2active ? m_p2active->get_name() : get_all_items_text() };
            if( m_L_name->get_visible() )
                m_L_name->set_text( text );
            else
                set_tooltip_text( text );
        }

        const Filter*               get_selected_from_list()
        {
            auto&& row{ m_LB->get_selected_row() };
            return( row ? m_items.at( row->get_index() ) : nullptr );
        }


        void                        update_tooltip();

        void                        handle_item_duplicate( DEID );
        void                        handle_item_edit( DEID );
        void                        handle_dialog_response( Filter* );
        void                        handle_item_dismiss( DEID );

        bool                        on_key_press_event( guint, guint, Gdk::ModifierType );
        void                        on_button_release_event( int, double, double );

        void                        handle_active_item_changed( const Filter*, bool );

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;
        Gtk::Label*                 m_L_name;
        Gtk::Popover*               m_Po;
        Gtk::ListBox*               m_LB;
        Gtk::Button*                m_B_add;
        Gtk::Button*                m_B_duplicate;
        Gtk::Button*                m_B_edit;
        Gtk::SearchEntry*           m_E_search;

        DialogFilter*               m_D_filter                      { nullptr };

        VecFilters                  m_items;
        int                         m_obj_classes;

        Diary*                      m_p2diary                       { nullptr };
        const Filter*               m_p2active                      { nullptr };

        int                         m_no_of_in_elems                { -1 }; // -1 means unkown

        int                         m_F_internal                    { 1 };
        bool                        m_F_clearable                   { false };
        bool                        m_F_show_cur_entry              { false };
        bool                        m_F_show_edit                   { false };
        bool                        m_F_select_only                 { false };
        bool                        m_F_hide_popover_during_edit    { false };
        bool                        m_F_hide_popover_force_disable  { false };
};

} // end of namespace LIFEO

#endif
