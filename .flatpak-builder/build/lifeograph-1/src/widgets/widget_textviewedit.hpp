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


#ifndef LIFEOGRAPH_WIDGET_TEXTVIEW_DIARY_EDIT_HEADER
#define LIFEOGRAPH_WIDGET_TEXTVIEW_DIARY_EDIT_HEADER


#include <enchant.h>

#include "../undo.hpp"
#include "src/diaryelements/paragraph.hpp"
#include "src/widgets/widget_entrypicker.hpp"
#include "widget_textview.hpp"
#include "widget_calendar.hpp"


namespace LIFEO
{

// TEXTVIEWEDIT ====================================================================================
class TextviewDiaryEdit : public TextviewDiary
{
    public:
        //TextviewDiaryEdit(); not used at the moment
        TextviewDiaryEdit( BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&, int = 140 );

        void                        disband();

        virtual bool                is_the_main_editor() const override { return true; }

        void                        set_entry( Entry* );
        void                        unset_entry()
        {
            set_editable( false );
            m_p2entry = nullptr;
            remove_Po_context_special();
        }

        void                        set_editable( bool );

        void                        set_static_text( const Ustring& ) = delete;

        void                        show_Po_context( int, int = 0 );
        void                        show_Po_paragraph( int, int = 0 );
        void                        show_Po_completion();

        void                        update_per_replaced_match( const HiddenFormat*, int );

        void                        update_highlight_button();

        void                        handle_para_type_changed( Gtk::ToggleButton*, int );
        int                         get_para_letter_type() const;
        void                        change_para_todo_status();
        void                        change_para_hrule();
        void                        change_para_indentation( bool );

        SignalVoid                  signal_changed_before_parse()
        { return m_Sg_changed_before_parse; }
        SignalVoid                  signal_changed()
        { return m_Sg_changed; }

        void                        size_allocate_vfunc( int w, int h, int b ) override;

        Gtk::Popover*               m_Po_context{ nullptr };
        Gtk::Popover*               m_Po_paragraph{ nullptr };
        EntryPickerCompletion*      m_completion{ nullptr };

    protected:
        //void                        check_resize();
        void                        on_button_press_event( int, double, double );
        //bool                        on_button_release_event( int, double, double );
        virtual bool                on_key_press_event( guint, guint, Gdk::ModifierType );
        void                        on_key_release_event( guint, guint, Gdk::ModifierType );
        // bool                        handle_drop_accept( const Glib::RefPtr< Gdk::Drop >& );
        Gdk::DragAction             handle_drop_motion( double, double );
        bool                        handle_drop( const Glib::ValueBase&, double, double );

        void                        update_image_options_scale( Glib::RefPtr< Gtk::Builder >& );
        void                        update_image_options_buttons( Glib::RefPtr< Gtk::Builder >& );
        void                        update_image_options();
        void                        update_chart_options();
        void                        update_table_options();
        void                        update_date_options();
        void                        update_evaluated_link_label( const HiddenFormat* );
        bool                        update_hidden_link_options();
        void                        update_tag_options();

        void                        remove_Po_context_special()
        {
            if( !m_p2Bx_edit_special ) return;

            m_Bx_context->remove( *m_p2Bx_edit_special );
            // delete m_p2Bx_edit_special;  apparently memeory is freed in remove() in gtk4
            m_p2Bx_edit_special = nullptr;
        }

        Gtk::Box*                   m_Bx_context;
        Gtk::Box*                   m_Bx_context_edit_1;
        Gtk::Box*                   m_Bx_context_edit_2;
        Gtk::Button*                m_B_expand_sel;
        Gtk::Button*                m_B_search_sel;
        Gtk::Button*                m_B_cut;
        Gtk::Button*                m_B_copy;
        Gtk::Button*                m_B_paste;
        Gtk::Button*                m_B_insert_link;
        Gtk::Button*                m_B_insert_emoji;
        Gtk::Button*                m_B_insert_image;
        Gtk::Button*                m_B_insert_chart;
        Gtk::Button*                m_B_insert_table;
        Gtk::Box*                   m_Bx_spell_seprtor;
        Gtk::FlowBox*               m_FBx_spell;
        Gtk::Button*                m_B_context_extras;
        Gtk::Stack*                 m_St_context_extras;

        Gtk::Button*                m_B_undo;
        Gtk::Button*                m_B_redo;
        Gtk::ToggleButton*          m_TB_bold;
        Gtk::ToggleButton*          m_TB_italic;
        Gtk::ToggleButton*          m_TB_underline;
        Gtk::ToggleButton*          m_TB_strkthru;
        Gtk::ToggleButton*          m_TB_highlight;
        Gtk::Label*                 m_L_highlight;
        Gtk::ToggleButton*          m_TB_subscript;
        Gtk::ToggleButton*          m_TB_superscript;

        Gtk::Box*                   m_p2Bx_edit_special{ nullptr }; // points to one of the below

        Gtk::Box*                   m_Bx_edit_image{ nullptr };
        Gtk::Box*                   m_Bx_edit_chart{ nullptr };
        Gtk::Box*                   m_Bx_edit_table{ nullptr };
        Gtk::Entry*                 m_E_edit_image_uri;
        FileChooserButton*          m_FCB_select_image;
        Gtk::Scale*                 m_Sc_edit_image_size;
        Gtk::Button*           m_MoB_copy_img_to_rel;
        Gtk::ComboBoxText*          m_CB_chart;
        Gtk::CheckButton*           m_CB_table_expand_all;

        Gtk::Box*                   m_Bx_edit_link{ nullptr };
        Gtk::Stack*                 m_St_edit_link;
        Gtk::Label*                 m_L_edit_link_uri;
        Gtk::Entry*                 m_E_edit_link_uri;
        WidgetEntryPicker*          m_WEP_edit_link_deid;

        Gtk::Box*                   m_Bx_edit_date  { nullptr };
        WidgetCalendar*             m_WCal_date_edit{ nullptr };
        Gtk::Box*                   m_Bx_edit_tag   { nullptr };
        Gtk::ComboBoxText*          m_CB_edit_tag   { nullptr };

        Gtk::Button*                m_B_indent;
        Gtk::Button*                m_B_unindent;
        Gtk::ToggleButton*          m_RB_align_left;
        Gtk::ToggleButton*          m_RB_align_center;
        Gtk::ToggleButton*          m_RB_align_right;
        Gtk::ToggleButton*          m_RB_para_plai;
        Gtk::ToggleButton*          m_RB_para_subh;
        Gtk::ToggleButton*          m_RB_para_hoff; // header off
        Gtk::ToggleButton*          m_RB_para_ssbh;
        Gtk::ToggleButton*          m_RB_para_todo;
        Gtk::ToggleButton*          m_RB_para_prog;
        Gtk::ToggleButton*          m_RB_para_done;
        Gtk::ToggleButton*          m_RB_para_cncl;

        Gtk::ToggleButton*          m_RB_para_bllt;
        Gtk::ToggleButton*          m_RB_para_nmbr;
        Gtk::ToggleButton*          m_RB_para_cltr;
        Gtk::ToggleButton*          m_RB_para_crmn;
        Gtk::Box*                   m_Bx_para_sltr;
        Gtk::Switch*                m_Sw_para_sltr;
        Gtk::Switch*                m_Sw_para_hrule;
        Gtk::Switch*                m_Sw_para_code;
        Gtk::Switch*                m_Sw_para_quote;

        Glib::RefPtr< Gtk::EventControllerKey >
                                    m_controller_key;
        Glib::RefPtr< Gtk::DropTarget >
                                    m_drop_target;

    // MIGRATED FROM TextbufferDiaryEdit
    public:
        Ustring                     get_text_to_save();

        void                        update_for_spell_check_change();

        bool                        toggle_format( int, bool = false );

        void                        remove_tag_at_cursor();
        void                        replace_tag_at_cursor( const Entry* );
        // void                        replace_word_at_cursor( const Ustring& );
        void                        replace_date_at_cursor( DateV );
        void                        modify_numeric_field( int );

        void                        add_empty_line_above();
        void                        remove_empty_line_above();
        void                        open_paragraph_below();
        void                        join_paragraphs();
        void                        move_paragraphs_up();
        void                        move_paragraphs_down();
        void                        delete_paragraphs( Paragraph*, Paragraph* );
        void                        duplicate_paragraphs();
        void                        beautify_text( bool );
        void                        do_for_each_sel_para( const FuncParagraph&, bool );
        void                        save_selection( int = 0 );
        void                        restore_selection();

        void                        insert_comment();
        void                        insert_link( int, Entry* );
        void                        add_link_to_sel( const String& );
        void                        add_link_to_sel_interactive();
        void                        insert_tag( int, Entry* );
        void                        make_selection_entry_tag();
        void                        insert_date_stamp( bool = false );
        void                        insert_hrule();
        void                        insert_image();
        void                        insert_image1( Glib::RefPtr< Gio::AsyncResult >&,
                                                   const Glib::RefPtr< Gtk::FileDialog >& );
        void                        insert_image2( const String&, int );
        void                        insert_chart();
        void                        insert_table();

        void                        paste_clipboard2();
        void                        paste_clipboard_entry_text();
        void                        paste_clipboard_img( const Ustring& );
        void                        paste_clipboard_img2( Glib::RefPtr< Gio::AsyncResult >&,
                                                          const Ustring& );

        void                        change_letter_cases( LetterCase );

        void                        go_to_link();

        // UNDO/REDO
        bool                        can_undo()
        { return( m_p2entry->get_undo_stack()->can_undo() ); }
        bool                        can_redo()
        { return( m_p2entry->get_undo_stack()->can_redo() ); }
        bool                        undo()
        { return process_undo( dynamic_cast< UndoEdit* >
                               ( m_p2entry->get_undo_stack()->get_undo_cur() ), false ); }
        bool                        redo()
        { return process_undo( dynamic_cast< UndoEdit* >
                               ( m_p2entry->get_undo_stack()->get_redo_cur() ), true ); }

    protected:
        void                        on_insert( Gtk::TextIter&, const Ustring&, int );
        void                        on_erase( Gtk::TextIter&, Gtk::TextIter& );
        void                        handle_after_erase( Gtk::TextIter&, Gtk::TextIter& );

        void                        on_mark_set( const Gtk::TextIter&,
                                                 const Glib::RefPtr< Gtk::TextBuffer::Mark >& );

        bool                        handle_backspace();
        bool                        handle_replace_str( const Ustring& );

        void                        update_para_region( int pos_erase_bgn, int pos_erase_end,
                                                        Paragraph* para_bgn, Paragraph* para_end,
                                                        int offset_cursor = -1 )
        {
            TextviewDiary::update_para_region_basic( pos_erase_bgn, pos_erase_end,
                                                     para_bgn, para_end,
                                                     offset_cursor );
            m_Sg_changed.emit(); // the only difference with the above
        }
        void                        update_para_region( int length_change, // new_l - old_l
                                                        Paragraph* p2para,
                                                        int pos = -1 )
        {
            update_para_region( p2para->get_bgn_offset_in_host(),
                                p2para->get_end_offset_in_host() - length_change,
                                p2para, p2para,
                                pos );
        }
        void                        update_para_region_cur( int pos = -1 )
        {
            update_para_region( m_pos_para_sel_bgn, m_pos_para_sel_end,
                                m_para_sel_bgn, m_para_sel_end,
                                pos == -1 ? m_pos_cursor : pos );
        }

        void                        copy_image_file_to_rel();

        // UNDO/REDO
        bool                        process_undo( UndoEdit*, bool );

        // SPELL CHECKING
        void                        set_check_word_handler();
        void                        handle_spellcheck_toggled();
        Ustring                     get_misspelled_word_cur() const;
        void                        update_spell_suggestions();
        void                        handle_spell_correction( Gtk::FlowBoxChild* );
        void                        ignore_misspelled_word();
        void                        replace_misspelled_word( const Ustring& );
        void                        add_word_to_dictionary();

        int                         m_spell_suggest_offset    { 0 };
        VecUstrings                 m_spell_suggestions;

        // OTHER VARIABLES
        bool                        m_F_paste_operation       { false };
        bool                        m_F_inherit_para_style    { true };
        bool                        m_F_show_Po_completion    { false };

        static std::map< const Ustring, std::pair< const Ustring, const Ustring > >
                                   s_auto_replaces;

        // SAVED SELECTION
        int                         m_saved_sel_bgn           { -1 };
        int                         m_saved_sel_end           { -1 };

        SignalVoid                  m_Sg_changed_before_parse;
        SignalVoid                  m_Sg_changed;

    friend class UIEntry;
    friend class TextviewDiaryPara;
    friend class UndoEdit;
};

} // end of namespace LIFEO

#endif
