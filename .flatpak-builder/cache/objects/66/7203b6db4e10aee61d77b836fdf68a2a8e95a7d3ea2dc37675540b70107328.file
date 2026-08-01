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


#ifndef LIFEOGRAPH_UI_ENTRY_HEADER
#define LIFEOGRAPH_UI_ENTRY_HEADER


#include <thread>

#include "helpers.hpp"  // i18n headers
#include "widgets/widget_textviewedit.hpp"
#include "widgets/po_entry.hpp"


namespace LIFEO
{

class UIEntry
{
    public:
                                UIEntry();
                                ~UIEntry();

        void                    handle_login();
        void                    handle_edit_enabled();
        void                    handle_logout();

        void                    show( Entry* );
        void                    show( Paragraph*, const HiddenFormat* = nullptr );
        void                    refresh(); // updates text with changes made elsewhere

        void                    show_popover()
        { m_CPo_entry.show(); }
        void                    hide_popover()
        { m_CPo_entry.hide(); }

        bool                    is_cur_entry( const Entry* e, bool F_or_descendant = false ) const
        { return( e == m_p2entry || ( F_or_descendant && e->is_descendant_of( m_p2entry ) ) ); }
        Entry*                  get_cur_entry() const
        { return m_p2entry; }
        void                    go_back();
        void                    go_forward();
        void                    add_entry_to_history( Entry* );
        void                    remove_entry_from_history( Entry* );
        void                    clear_history();

        void                    refresh_title();
        void                    refresh_extra_info();
        void                    update_PoEntry_size()
        {   m_CPo_entry.update_size(); }

        void                    handle_title_edited()
        {
            m_F_title_edited = true;
            refresh_title();
        }

        void                    undo() { m_TvDE->undo(); }
        void                    redo() { m_TvDE->redo(); }

        void                    prepare_for_hiding();

        void                    set_entries_favorite( const EntrySelection&, bool );
        void                    set_entries_todo( const EntrySelection&, ElemStatus );
        void                    trash_entries( const EntrySelection&, bool );
        void                    dismiss_entries( const EntrySelection& );
        void                    dismiss_entries2();
        void                    set_entries_color( const EntrySelection&, const Color& );
        void                    set_entries_spellcheck( const EntrySelection&, const Ustring& );
        void                    set_entries_unit( const EntrySelection&, const Ustring& );
        void                    set_entries_title_type( const EntrySelection&, int );
        void                    set_entries_comment_style( const EntrySelection&, int );
        void                    set_theme( const Theme* );
        void                    refresh_theme();

        TextviewDiaryEdit*      get_textview() { return m_TvDE; }
        //TextbufferDiaryEdit*    get_buffer() { return m_TvDE->m_buffer_edit; }

    protected:
        void                    print();


        // MULTI-THREADING
        void                    start_bg_jobs();
        void                    handle_bg_jobs_completed();
        void                    destroy_bg_jobs_thread();

        // WIDGETS
        Gtk::Box*               m_Bx_navigation;

        PoEntry                 m_CPo_entry           { false };

        Gtk::Stack*             m_St_entry;
        Gtk::ScrolledWindow*    m_SW_entry;
        TextviewDiaryEdit*      m_TvDE;

        Gtk::CheckButton*       m_ChB_dismiss_children;

        // ACTIONS
        R2Action                m_A_go_back;
        R2Action                m_A_go_forward;

        bool                    m_F_index_locked      { false }; // history index
        bool                    m_F_title_edited      { false };
        bool                    m_F_dismiss_children  { false };

        Entry*                  m_p2entry             { nullptr };
        const EntrySelection*   m_p2entries2dismiss;

        std::list< Entry* >     m_browsing_history;
        unsigned int            m_bhist_offset        { 0 };  // browsing history index

        // MULTI-THREADING
        sigc::connection        m_connection_bg_jobs;
        std::thread*            m_thread_bg_jobs      { nullptr };
        Glib::Dispatcher        m_dispatcher;
        bool                    m_F_bg_jobs_queued    { false };

        ParserString            m_parser_word_count; // updates the word count in the bg

    friend class Lifeograph;
};

} // end of namespace LIFEO

#endif
