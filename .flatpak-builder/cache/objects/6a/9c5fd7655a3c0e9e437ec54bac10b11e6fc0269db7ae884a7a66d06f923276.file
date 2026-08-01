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


#ifndef LIFEOGRAPH_TEXTVIEW_DIARY_SEARCH_HEADER
#define LIFEOGRAPH_TEXTVIEW_DIARY_SEARCH_HEADER


#include "widget_textview.hpp"


namespace LIFEO
{

// TEXTVIEW ========================================================================================
class TextviewDiarySearch : public TextviewDiary
{
    public:
    typedef sigc::signal< void( const HiddenFormat* ) >
                                    Signal_void_HiddenFormat;

        TextviewDiarySearch();
        TextviewDiarySearch( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );

        void                        disband();

        bool                        is_a_search_editor() const override { return true; }

        void                        set_diary( Diary* diary )
        {
            m_p2diary = diary;
        }

        void                        set_static_text( const Ustring&, bool ) = delete;

        void                        show_Po_find( int, int );

        // void                        replace_match( const HiddenFormat*, const Ustring& );

        Signal_void_HiddenFormat    signal_replace()
        { return m_Sg_replace; }

    protected:
        void                        handle_link_match();
        void                        process_link_uri( const Paragraph*, int ) override;

        void                        init();

        void                        on_button_press_event( int, double, double );

        bool                        handle_query_tooltip(
                                        int, int, bool,
                                        const Glib::RefPtr< Gtk::Tooltip >& ) override;

        Gtk::PopoverMenu*           m_Po_find{ nullptr };
        R2ActionGroup               m_AG;
        R2Action                    m_A_replace;

        Signal_void_HiddenFormat    m_Sg_replace;

    // MIGRATED FROM TextbufferDiarySearch
    public:
        void                        set_para_sel( int );
        void                        set_para_sel( const HiddenFormat* );

        void                        set_theme( const Theme* ) override;

        void                        set_text_from_matches( Diary::SetMatches* );

        void                        set_text_search_in_progress()
        {
            m_entry.clear_text();
            m_entry.add_paragraph_before( _( "Searching..." ), nullptr )->
                    set_heading_level( VT::PS_SUBHDR );

            set_hovered_link( VT::HFT_UNSET );
            m_edit_operation_type = EOT::SET_TEXT;
            m_r2buffer->set_text( m_entry.get_text() );
            update_text_formatting();

            m_edit_operation_type = EOT::USER;
        }

    protected:
        // PARSING
        void                        process_paragraph( Paragraph*,
                                                       const Gtk::TextIter&,
                                                       const Gtk::TextIter&,
                                                       bool = true ) override;

        // TAGS
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_para_sel;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_separator_entry;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_separator_para;

        // OTHER VARIABLES
        unsigned int                m_index_match{ 0 }; // current index in parsing
        int                         m_para_no_sel{ -1 };
        Entry                       m_entry;

    friend class LinkMatch;
};

} // end of namespace LIFEO

#endif
