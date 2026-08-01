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


#ifndef LIFEOGRAPH_PO_ENTRY_HEADER
#define LIFEOGRAPH_PO_ENTRY_HEADER


#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>

#include "../helpers.hpp"
#include "../diaryelements/entry.hpp"


namespace LIFEO
{

using namespace HELPERS;


class PoEntry
{
    public:
        PoEntry( bool );

        void
        set( Entry*, Gtk::Widget&, const Gdk::Rectangle& );

        void                        refresh();
        void                        refresh_CB_spellchecking();
        void                        refresh_editing();

        void                        update_size()
        {
            if( m_F_update_size > 0 && m_F_update_size < 3 )
            // TODO: 3.1: this is a very hackish solution to ensure the popover is sized properly
            {
                m_Po_entry->present();
                PRINT_DEBUG( "po size updated" );
            }
        }
        R2Action                    get_action( const Ustring& );

        void                        show();

        void
        hide()
        { if( m_Po_entry ) m_Po_entry->hide(); }

        void                        handle_edit_enabled();
        void                        handle_logout();

        SignalVoidElemstatus        m_Sg_todo_changed;
        SignalVoidBool              m_Sg_favorite_changed;
        SignalVoidUstring           m_Sg_unit_changed;
        SignalVoidUstring           m_Sg_spellcheck_changed;
        SignalVoidInt               m_Sg_comment_style;
        SignalVoidInt               m_Sg_title_type;
        SignalVoidColor             m_Sg_color_changed;

    protected:
        Glib::RefPtr< Gtk::Builder >
                                    m_builder;

        Gtk::Popover*               m_Po_entry;
        Gtk::Box*                   m_Bx_group_editing;
        Gtk::ToggleButton*          m_RB_not_todo;
        Gtk::ToggleButton*          m_RB_todo;
        Gtk::ToggleButton*          m_RB_progressed;
        Gtk::ToggleButton*          m_RB_done;
        Gtk::ToggleButton*          m_RB_canceled;
        Gtk::ToggleButton*          m_TB_favorite;
        Gtk::ColorButton*           m_ClB_color;
        Gtk::DropDown*              m_DD_title_style;

        Gtk::Box*                   m_Bx_unit;
        Gtk::ComboBoxText*          m_CB_unit;
        Gtk::Entry*                 m_E_unit;

        Gtk::ComboBoxText*          m_CB_spellcheck;

        Gtk::ComboBoxText*          m_CB_comment_style;

        Entry*                      m_p2entry           { nullptr };

        bool                        m_F_list_mode;
        int                         m_F_update_size     { 0 };

    friend class UIDiary;
};

} // end of namespace LIFEO

#endif
