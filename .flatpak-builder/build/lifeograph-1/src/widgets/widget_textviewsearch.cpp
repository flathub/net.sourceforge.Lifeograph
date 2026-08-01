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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../lifeograph.hpp"
#include "../app_window.hpp"
#include "widget_textviewsearch.hpp"


using namespace LIFEO;


// TEXTBUFFERDIARYSEARCH ===========================================================================
void
TextviewDiarySearch::set_para_sel( int para_no )
{
    if( para_no == m_para_no_sel ) return;

    Gtk::TextIter it_bgn, it_end;

    if( m_para_no_sel >= 0 )
    {
        it_bgn = it_end = m_r2buffer->get_iter_at_line( m_para_no_sel );
        it_end.forward_to_line_end();
        m_r2buffer->remove_tag( m_tag_para_sel, it_bgn, it_end );
    }

    it_bgn = it_end = m_r2buffer->get_iter_at_line( para_no );
    it_end.forward_to_line_end();
    m_r2buffer->apply_tag( m_tag_para_sel, it_bgn, it_end );

    m_para_no_sel = para_no;
}

void
TextviewDiarySearch::set_para_sel( const HiddenFormat* match )
{
    bool        F_found { false };
    Paragraph*  para    { m_p2entry->get_paragraph_1st() };

    for( ; para; para = para->m_p2next )
    {
        for( auto format : para->m_formats )
        {
            if( format->type == VT::HFT_MATCH && format->pos_bgn == match->pos_bgn &&
                format->var_i == match->var_i && format->ref_id == match->ref_id )
            {
                F_found = true;
                break;
            }
        }
        if( F_found ) break;
    }
    auto&& it_bgn{ m_r2buffer->get_iter_at_offset( para->get_bgn_offset_in_host() ) };
    set_para_sel( para->get_para_no() );
    scroll_to( it_bgn, 0.05 );
}

void
TextviewDiarySearch::set_text_from_matches( Diary::SetMatches* matches )
{
    m_edit_operation_type = EOT::SET_TEXT;

    m_index_match = 0;
    m_para_no_sel = -1;

    set_hovered_link( VT::HFT_UNSET );

    m_entry.clear_text();

    if( m_p2diary->get_search_text().empty() )
        m_entry.add_paragraph_before( _( "Please enter a search phrase" ), nullptr )->
                set_heading_level( VT::PS_SUBHDR );
    else if( !matches || matches->empty() )
        m_entry.add_paragraph_before( _( "No matches" ), nullptr )->
                set_heading_level( VT::PS_SUBHDR );
    else
    {
        DEID       last_entry_id    { DEID_UNSET };
        Paragraph* last_remote_para { nullptr };
        Paragraph* last_local_para  { nullptr };
        int        i                { 0 };

        for( auto format : *matches )
        {
            if( ++i > 200 ) break;

            Entry*     e_host { m_p2diary->get_entry_by_id( format->ref_id ) };
            Paragraph* para   { e_host->get_paragraph_by_no( format->var_i ) };

            if( last_local_para )
            {
                int type{ 0 };

                if     ( last_entry_id != format->ref_id ) // separate entries more strongly
                    type = VT::PS_SEPRTR_E;
                else if( last_remote_para != para->m_p2prev )
                    type = VT::PS_SEPRTR_P;

                if( type != 0 )
                {
                    last_local_para = m_entry.add_paragraph_before( " ", nullptr );
                    // on gtk4, text cannot be empty
                    last_local_para->set_para_type_raw( type );
                }
            }

            if( para == last_remote_para ) continue;

            last_local_para = m_entry.add_paragraph_after( new Paragraph( para ), last_local_para );
            last_local_para->set_visible( true ); // search result always have to be visible
            last_local_para->m_host = para->m_host;

            if( !para->m_p2prev )
                last_local_para->set_heading_level( VT::PS_HEADER );
            if( i == 1 )
                last_local_para->m_style |= VT::PS_REORDERED;

            last_remote_para = para;
            last_entry_id = format->ref_id;
        }
    }

    m_r2buffer->set_text( m_entry.get_text() );
    update_text_formatting();

    m_edit_operation_type = EOT::USER;
}

void
TextviewDiarySearch::set_theme( const Theme* theme )
{
    if( m_p2theme )
        remove_css_class( m_p2theme->get_css_class_name() );

    m_p2theme = theme;

    add_css_class( m_p2theme->get_css_class_name() );

    m_tag_highlight->property_background_rgba() = m_p2theme->color_region_bg;

    m_tag_comment->property_foreground_rgba() = m_p2theme->color_mid;

    // separator:
    m_tag_separator_entry->property_paragraph_background_rgba() = m_p2theme->color_region_bg;

    m_tag_para_sel->property_paragraph_background_rgba() = m_p2theme->color_highlight;

    m_tag_match->property_foreground_rgba() = m_p2theme->color_base;
    m_tag_match->property_background_rgba() = m_p2theme->color_match_bg;
}

// PARSING
void
TextviewDiarySearch::process_paragraph( Paragraph* para,
                                        const Gtk::TextIter& it_bgn,
                                        const Gtk::TextIter& it_end,
                                        bool )
{
    TextviewDiary::process_paragraph( para, it_bgn, it_end );

    if( para->get_para_type() == VT::PS_SEPRTR_E )
        m_r2buffer->apply_tag( m_tag_separator_entry, it_bgn, it_end );
    else
    if( para->get_para_type() == VT::PS_SEPRTR_P )
        m_r2buffer->apply_tag( m_tag_separator_para, it_bgn, it_end );
}

// TEXTVIEW ========================================================================================
TextviewDiarySearch::TextviewDiarySearch()
:   TextviewDiary( 10 ), m_entry( nullptr, Date::NOT_SET )
{
    TextviewDiarySearch::init();
}

TextviewDiarySearch::TextviewDiarySearch( BaseObjectType* obj,
                                          const Glib::RefPtr< Gtk::Builder >& builder )
:   TextviewDiary( obj, builder, 10 ), m_entry( nullptr, Date::NOT_SET )
{
    TextviewDiarySearch::init();
}

void
TextviewDiarySearch::disband()
{
    m_Po_find->unparent();
}

void
TextviewDiarySearch::init()
{
    // TAGS
    // NOTE: order is significant. the later a tag is added the more dominant it is.

    m_tag_para_sel = m_r2buffer->create_tag( "para.sel" );

    m_tag_separator_entry = m_r2buffer->create_tag( "separator.entry" );
    m_tag_separator_entry->property_scale() = 0.5;

    m_tag_separator_para = m_r2buffer->create_tag( "separator.para" );
    m_tag_separator_para->property_scale() = 0.7;

    m_p2entry = &m_entry;

    auto builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/tv_diary.ui" ) };

    m_Po_find    = builder->get_widget< Gtk::PopoverMenu >( "Po_find" );
    m_AG         = Gio::SimpleActionGroup::create();
    m_A_replace  = m_AG->add_action(
            "replace",
            [ this ]() { if( m_p2hflink_hovered ) m_Sg_replace.emit( m_p2hflink_hovered ); } );

    insert_action_group( "find", m_AG );

    m_Po_find->set_parent( *this );

    m_gesture_click->set_propagation_phase( Gtk::PropagationPhase::CAPTURE );
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &TextviewDiarySearch::on_button_press_event ), false );

    set_editable( false );

    TextviewDiarySearch::set_theme( ThemeSystem::get() );

    m_cursor_default = Gdk::Cursor::create( "default" );
}

void
TextviewDiarySearch::handle_link_match()
{
    Entry*     entry { m_p2diary->get_entry_by_id( m_p2hflink_hovered->ref_id ) };
    Paragraph* para  { entry->get_paragraph_by_no( m_p2hflink_hovered->var_i ) };
    set_para_sel( m_p2hflink_hovered );
    AppWindow::p->UI_entry->show( para, m_p2hflink_hovered );
}

void
TextviewDiarySearch::process_link_uri( const Paragraph* para_hovered, int offset )
{
    set_hovered_link( para_hovered->get_format_at( VT::HFT_MATCH, offset ) );

    if( get_hovered_link_type() != VT::HFT_UNSET )
        m_link_hovered_go = [ & ](){ handle_link_match(); };
}

void
TextviewDiarySearch::show_Po_find( int x, int y )
{
    if( !m_p2hflink_hovered ) return;

    m_Po_find->set_pointing_to( Gdk::Rectangle( x - 1, y - 1, 2, 2 ) );
    m_Po_find->show();
}

// this is no longer needed as on replace events, the results are updated for entry in ui_extra:
// void
// TextviewDiarySearch::replace_match( const HiddenFormat* match, const Ustring& text_new )
// {
//     for( Paragraph* p = m_entry.get_paragraph_1st(); p; p = p->m_p2next )
//     {
//         for( auto it = p->m_formats.begin(); it != p->m_formats.end(); )
//         {
//             HiddenFormat* f { *it };
//
//             if( f == match )
//             {
//                 const auto  pos_bgn   { f->pos_bgn };
//                 const auto  pos_end   { f->pos_end };
//                 const auto  pos_para  { p->get_bgn_offset_in_host() };
//                 auto&&      it_bgn    { m_r2buffer->get_iter_at_offset( pos_para + pos_bgn ) };
//                 auto&&      it_end    { m_r2buffer->get_iter_at_offset( pos_para + pos_end ) };
//                 // delete the match format in advance to prevent double deletion:
//                 delete f;
//                 it = p->m_formats.erase( it );
//                 p->replace_text( pos_bgn, pos_end - pos_bgn, text_new );
//
//                 m_edit_operation_type = EOT::SET_TEXT;
//
//                 it_bgn = m_r2buffer->erase( it_bgn, it_end );
//                 m_r2buffer->insert( it_bgn, text_new );
//
//                 m_edit_operation_type = EOT::USER;
//             }
//             else
//                 ++it;
//         }
//     }
// }

void
TextviewDiarySearch::on_button_press_event( int n_press, double x, double y )
{
    if( m_p2hflink_hovered && m_p2hflink_hovered->type == VT::HFT_MATCH &&
        m_gesture_click->get_current_button() == 3 && Diary::d->is_in_edit_mode() )
    {
        m_gesture_click->set_state( Gtk::EventSequenceState::CLAIMED );
        show_Po_find( x, y );
    }
}

bool
TextviewDiarySearch::handle_query_tooltip( int x, int y, bool keyboard_mode,
                                           const Glib::RefPtr< Gtk::Tooltip >& tooltip )
{
    if( Diary::d->is_open() && m_p2hflink_hovered && m_p2hflink_hovered->type == VT::HFT_MATCH )
    // Diary::d->is_open() is added to prevent some segfaults
    {
        auto   host_entry   { m_p2diary->get_entry_by_id( m_p2hflink_hovered->ref_id ) };
        auto&& tooltip_text { host_entry->get_title_ancestral() + "\n" + _( "Click to jump to" ) };

        if( Diary::d->is_in_edit_mode() )
        {
            tooltip_text += "  |  ";
            tooltip_text += _( "Right click to replace" );
        }

        tooltip->set_markup( tooltip_text );
    }
    else
        return false;

    return true;
}
