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


#ifndef LIFEOGRAPH_TEXTVIEW_DIARY_HEADER
#define LIFEOGRAPH_TEXTVIEW_DIARY_HEADER


#include "../helpers.hpp"
#include "../diaryelements/diarydata.hpp"
#include "../diaryelements/entry.hpp"
#include "../parsers/parser_base.hpp"


namespace LIFEO
{

// NEW LINE PREDICATE ==============================================================================
class PredicateNL
{
    public:
        bool operator()( gunichar ch ) const
        {
            return( ch == '\n' );
        }
};

class PredicateBlank
{
    public:
        bool operator()( gunichar ch ) const
        {
            return( ch == '\n' || ch == ' ' || ch == '\t' );
        }
};

// TEXTBUFFER ======================================================================================
// a helper class to block certain default handlers when needed
class TextbufferDiary : public Gtk::TextBuffer
{
    public:
        static Glib::RefPtr< TextbufferDiary > create()
        {
            return Glib::make_refptr_for_instance( new TextbufferDiary );
        }

    protected:
        void
        on_insert( TextBuffer::iterator& pos, const Ustring& text, int bytes ) override
        {
            if( m_F_skip_gtk_handlers )
                m_F_skip_gtk_handlers = false;
            else
                Gtk::TextBuffer::on_insert( pos, text, bytes );
        }
        void
        on_erase( TextBuffer::iterator& it_bgn, TextBuffer::iterator& it_end ) override
        {
            if( m_F_skip_gtk_handlers )
                m_F_skip_gtk_handlers = false;
            else
                Gtk::TextBuffer::on_erase( it_bgn, it_end );
        }
        void
        on_mark_set( const TextBuffer::iterator& iter,
                     const Glib::RefPtr< Gtk::TextMark >& mark ) override
        {
            if( !iter.is_end() || iter.get_offset() == 0 )
                Gtk::TextBuffer::on_mark_set( iter, mark );
            else
                PRINT_DEBUG( "on mark_set() REJECTED" );
        }

        bool m_F_skip_gtk_handlers { false };

    friend class TextviewDiaryEdit;
};

// TEXTVIEW ========================================================================================
class TextviewDiary : public Gtk::TextView
{
    protected: // constructors are protected to make it abstract
        static constexpr int        BULLET_COL_WIDTH    { 36 };
        static constexpr int        EXPANDER_WIDTH      { 30 };
        static constexpr int        INDENT_WIDTH        { 50 };
        const int                   TEXT_MARGIN_L;
        const int                   TEXT_MARGIN_R;
        static constexpr int        PIXELS_ABOVESUBHDR  { 15 };
        const int                   PAGE_MARGIN;
        const int                   QUOT_MARGIN;

        TextviewDiary( int = 10 );
        TextviewDiary( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >&,
                       int = 10 );

    public:
        virtual bool                is_the_main_editor() const { return false; }
        virtual bool                is_a_search_editor() const { return false; }

        bool                        get_selection_rect( Gdk::Rectangle& );

        bool                        is_hovered_para_within_selection() const
        {
            return( m_para_menu && m_para_sel_end &&
                    m_para_menu->get_para_no() >= m_para_sel_bgn->get_para_no() &&
                    m_para_menu->get_para_no() <= m_para_sel_end->get_para_no() );
        }
        bool                        is_pos_within_selection( const int pos ) const
        {
            return( m_para_sel_end &&
                    pos >= m_para_sel_bgn->get_bgn_offset_in_host() &&
                    pos < m_para_sel_end->get_end_offset_in_host() );
        }

        void                        refresh_entry_icons();

        void                        copy_clipboard( bool = false );

    protected:
        void                        init();

        void                        handle_link_uri( const String& );
        void                        handle_para_uri( const Paragraph* );
        void                        handle_link_id( DEID );
        void                        handle_link_refs_menu();
        void                        update_link( double, double, Gdk::ModifierType );
        virtual void                process_link_uri( const Paragraph*, int );
        //void                        update_link_at_insert();

        virtual void                on_motion_notify_event( double, double );
        virtual void                on_button_pressed( int, double, double );
        virtual void                on_button_release_event( int, double, double );
        void                        snapshot_vfunc( const Glib::RefPtr< Gtk::Snapshot >& ) override;

        virtual bool                handle_query_tooltip( int, int, bool,
                                                          const Glib::RefPtr< Gtk::Tooltip >& );

        void                        get_tag_coordinates( const int, const int );

        void                        set_hovered_link( int type )
        {
            m_hflink_internal.type = type;
            m_p2hflink_hovered = &m_hflink_internal;
        }
        void                        set_hovered_link( const HiddenFormat* link )
        {
            m_hflink_internal.type = VT::HFT_UNSET;
            m_p2hflink_hovered = ( link ? link : &m_hflink_internal );
        }
        int                         get_hovered_link_type()
        { return( m_p2hflink_hovered ? m_p2hflink_hovered->type : VT::HFT_UNSET ); }

        Glib::RefPtr< TextbufferDiary >
                                    m_r2buffer;

        const Glib::RefPtr< Gdk::Cursor >*
                                    m_ptr2cursor_last   { nullptr };
        HiddenFormat                m_hflink_internal   { VT::HFT_UNSET, "", 0, 0 };
        const HiddenFormat*         m_p2hflink_hovered  { &m_hflink_internal };
        std::function< void() >     m_link_hovered_go   { [](){} };
        Paragraph*                  m_para_menu         { nullptr };
        Paragraph*                  m_para_sel_bgn      { nullptr };
        Paragraph*                  m_para_sel_end      { nullptr };
        Paragraph*                  m_para_cursor       { nullptr };
        int                         m_pos_cursor_in_para{ 0 }; // only used for hidden links now
        int                         m_pos_hovered_char  { -1 };

        bool                        m_F_fold_area_hovered { false };

        const std::vector< double > m_dash_pattern = { 5.0, 5.0 };

        Glib::RefPtr< Gtk::GestureClick >
                                    m_gesture_click;
        Glib::RefPtr< Gtk::EventControllerMotion >
                                    m_controller_motion;

    // MIGRATED FROM THE TextbufferDiary:
    public:
        enum class EOT // Edit Operation Type
        {
            USER,
            //MANAGED,
            SET_TEXT
        };

        void                        init_tags();

        Ustring                     get_selected_text() const;
        void                        expand_selection();
        void                        select_paragraph( const Paragraph* );

        void                        set_cursor_position( int pos )
        { m_r2buffer->place_cursor( m_r2buffer->get_iter_at_offset( pos ) ); }
        void                        set_scroll_position( int pos )
        {
            auto mark{ m_r2buffer->create_mark( m_r2buffer->get_iter_at_offset( pos ) ) };
            scroll_to( mark, 0.05, 0.5, 0.5 );
            m_r2buffer->delete_mark( mark );
        }
        void                        scroll_to_cursor_center()
        { scroll_to( m_r2buffer->get_insert(), 0.05, 0.5, 0.5 ); }
        void                        scroll_to_cursor_min()
        { scroll_to( m_r2buffer->get_insert(), 0.08 ); }
        void                        scroll_to_top()
        {
            set_cursor_position( 0 );
            scroll_to( m_r2buffer->get_insert(), 0.05 );
        }
        void                        scroll_to_bottom()
        {
            set_cursor_position( m_p2entry->get_size() );
            scroll_to( m_r2buffer->get_insert(), 0.05 );
        }

        void                        update_cursor_dependent_positions();
        void                        update_para_region_basic( int, int, Paragraph*, Paragraph*,
                                                              int = -1 );

        void                        toggle_fold( Paragraph* );
        void                        toggle_collapse_all_subheaders();

        void                        refresh_entry( bool = true, bool = false );
        void                        update_text_formatting( int, int );
        void                        update_text_formatting( Paragraph*, Paragraph* );
        void                        update_text_formatting()
        { update_text_formatting( 0, m_r2buffer->end().get_offset() ); }

        virtual void                set_theme( const Theme* );
        const Theme*                get_theme() const { return m_p2theme; }

    protected:
        void                        set_static_text( const Ustring& );
        void                        set_entry( Entry* );

        int                         get_text_width( const Pango::FontDescription&,
                                                    const Ustring& );
        int                         get_first_nl_offset()
        {
            auto firstnl = m_r2buffer->begin();
            if( ! firstnl.forward_find_char( s_predicate_nl ) )
                return m_r2buffer->end().get_offset();
            return firstnl.get_offset();
        }

        // HELPERS
        void                        calculate_para_bounds( Gtk::TextIter&, Gtk::TextIter& );
        void                        calculate_para_bounds( UstringSize&, UstringSize& );
        bool                        calculate_sel_word_bounds( Gtk::TextIter&, Gtk::TextIter& );
        bool                        calculate_sel_para_bounds( Gtk::TextIter&, Gtk::TextIter& );
        bool                        calculate_sel_bounding_paras( Paragraph*&, Paragraph*& );
        Ustring                     calculate_word_bounds( Gtk::TextIter&, Gtk::TextIter& );
        void                        calculate_token_bounds( Gtk::TextIter&, Gtk::TextIter&, int );

        // PARSING
        virtual void                process_paragraph( Paragraph*,
                                                       const Gtk::TextIter&,
                                                       const Gtk::TextIter&,
                                                       bool = true );

        int                         m_pos_cursor        { 0 };
        UstringSize                 m_pos_para_sel_bgn  { 0 };
        UstringSize                 m_pos_para_sel_end  { 0 };

        static PredicateNL          s_predicate_nl;
        static PredicateBlank       s_predicate_blank;

        void                        show_image( const R2Pixbuf&, Gtk::TextIter, Gtk::TextIter );
        bool                        update_image_width();

        // TAGS
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_heading;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_subheading;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_subsubheading;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_match;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_bold;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_italic;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_underline;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_strikethru;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_highlight;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_subscript;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_superscript;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_comment;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_markup;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_region;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_quotation;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_link;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_link_no_underline;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_link_broken;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_todo;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_done;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_canceled;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_smaller;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_inline_tag;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_inline_value;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_misspelled;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_justify_left;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_justify_center;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_justify_right;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_indent_1;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_indent_2;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_indent_3;
        Glib::RefPtr< Gtk::TextBuffer::Tag >         m_tag_indent_4;

        // OTHER VARIABLES
        EOT                         m_edit_operation_type     { EOT::USER };
        bool                        m_F_popover_link          { false };
        int                         m_ongoing_operation_depth { 0 }; // incremental to allow nesting
        Entry*                      m_p2entry                 { nullptr };
        Diary*                      m_p2diary                 { nullptr };
        const Theme*                m_p2theme                 { nullptr };
        Glib::RefPtr< Gdk::Cursor > m_cursor_default;

        int                         m_max_thumbnail_w         { 0 };
        int                         m_last_width              { 0 }; // for size change detection

        std::vector< int >          m_tag_coords;

    friend class UIEntry;
};

} // end of namespace LIFEO

#endif
