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
#include "../ui_entry.hpp"
#include "widget_textview.hpp"


#define GTKSPELL_MISSPELLED_TAG "gtkspell-misspelled"


using namespace LIFEO;

// STATIC MEMBERS
PredicateNL                     TextviewDiary::s_predicate_nl;
PredicateBlank                  TextviewDiary::s_predicate_blank;

// TEXTBUFFERDIARY =================================================================================
void
TextviewDiary::init_tags()
{
    // TAGS
    // NOTE: order is significant. the later a tag is added the more dominant it is.

    m_tag_heading = m_r2buffer->create_tag( "heading" );
    m_tag_heading->property_weight() = Pango::Weight::BOLD;
    m_tag_heading->property_scale() = 1.7;

    m_tag_subheading = m_r2buffer->create_tag( "subheading" );
    m_tag_subheading->property_weight() = Pango::Weight::BOLD;
    m_tag_subheading->property_scale() = 1.4;
    m_tag_subheading->property_pixels_above_lines() = PIXELS_ABOVESUBHDR;

    m_tag_subsubheading = m_r2buffer->create_tag( "subsubheading" );
    m_tag_subsubheading->property_weight() = Pango::Weight::BOLD;
    m_tag_subsubheading->property_scale() = 1.15;
    m_tag_subsubheading->property_pixels_above_lines() = PIXELS_ABOVESUBHDR;

    m_tag_region = m_r2buffer->create_tag( "region" );
    m_tag_region->property_font() = "Monospace";

    m_tag_quotation = m_r2buffer->create_tag( "quotation_" );
    // left margin cannot go below indent level 1 as a property of the paragraphs
    // right margin also uses TEXT_MARGIN_L for symmetry:
    m_tag_quotation->property_right_margin() = TEXT_MARGIN_L + INDENT_WIDTH;

    m_tag_todo = m_r2buffer->create_tag( "todo" );
    // m_tag_todo->property_weight() = Pango::Weight::BOLD;

    m_tag_done = m_r2buffer->create_tag( "done" );

    m_tag_canceled = m_r2buffer->create_tag( "canceled" );
    m_tag_canceled->property_strikethrough() = true;

    m_tag_smaller = m_r2buffer->create_tag( "smaller" );
    m_tag_smaller->property_scale() = 0.85;

    m_tag_link = m_r2buffer->create_tag( "link" );
    m_tag_link->property_underline() = Pango::Underline::SINGLE;

    m_tag_link_no_underline = m_r2buffer->create_tag( "link.no_underline" );

    m_tag_link_broken = m_r2buffer->create_tag( "link.broken" );
    m_tag_link_broken->property_underline() = Pango::Underline::SINGLE;

    m_tag_inline_tag = m_r2buffer->create_tag( "inline tag" );
    // m_tag_inline_tag->property_scale() = 0.85;
    // m_tag_inline_tag->property_rise() = 1 * Pango::SCALE;
    m_tag_inline_tag->property_letter_spacing() = 1200;
    m_tag_inline_tag->property_weight() = Pango::Weight::NORMAL;
    m_tag_inline_tag->property_strikethrough() = false;

    m_tag_inline_value = m_r2buffer->create_tag( "inline value" );
    // m_tag_inline_value->property_scale() = 0.85;
    // m_tag_inline_value->property_rise() = 1 * Pango::SCALE;
    m_tag_inline_value->property_letter_spacing() = 1000;
    m_tag_inline_value->property_weight() = Pango::Weight::NORMAL;
    m_tag_inline_value->property_strikethrough() = false;

    m_tag_comment = m_r2buffer->create_tag( "comment" );
    m_tag_comment->property_scale() = 0.8;
    m_tag_comment->property_rise() = 3.5 * Pango::SCALE;
    m_tag_comment->property_weight() = Pango::Weight::NORMAL;
    m_tag_comment->property_strikethrough() = false; // for comments in canceled check list items

    m_tag_bold = m_r2buffer->create_tag( "bold" );
    m_tag_bold->property_weight() = Pango::Weight::BOLD;

    m_tag_italic = m_r2buffer->create_tag( "italic" );
    m_tag_italic->property_style() = Pango::Style::ITALIC;

    m_tag_underline = m_r2buffer->create_tag( "underline" );
    m_tag_underline->property_underline() = Pango::Underline::SINGLE;

    m_tag_strikethru = m_r2buffer->create_tag( "strikethrough" );
    m_tag_strikethru->property_strikethrough() = true;

    m_tag_highlight = m_r2buffer->create_tag( "highlight" );

    m_tag_subscript = m_r2buffer->create_tag( "subscript" );
    m_tag_subscript->property_rise() = -3 * Pango::SCALE;
    m_tag_subscript->property_scale() = 0.7;

    m_tag_superscript = m_r2buffer->create_tag( "superscript" );
    m_tag_superscript->property_rise() = 5 * Pango::SCALE;
    m_tag_superscript->property_scale() = 0.7;

    m_tag_misspelled = m_r2buffer->create_tag( GTKSPELL_MISSPELLED_TAG );
    m_tag_misspelled->property_underline() = Pango::Underline::ERROR;

    m_tag_markup = m_r2buffer->create_tag( "markup" );
    m_tag_markup->property_scale() = 0.8;
    m_tag_markup->property_rise() = 3.5 * Pango::SCALE;
    m_tag_markup->property_weight() = Pango::Weight::NORMAL;
    m_tag_markup->property_style() = Pango::Style::NORMAL;
    m_tag_markup->property_underline() = Pango::Underline::NONE;
    m_tag_markup->property_strikethrough() = false;

    m_tag_justify_left = m_r2buffer->create_tag( "justify_left" );
    m_tag_justify_left->property_justification() = Gtk::Justification::LEFT;

    m_tag_justify_center = m_r2buffer->create_tag( "justify_center" );
    m_tag_justify_center->property_justification() = Gtk::Justification::CENTER;

    m_tag_justify_right = m_r2buffer->create_tag( "justify_right" );
    m_tag_justify_right->property_justification() = Gtk::Justification::RIGHT;

    m_tag_indent_1 = m_r2buffer->create_tag( "indent.1" );
    m_tag_indent_1->property_left_margin() = TEXT_MARGIN_L + INDENT_WIDTH;

    m_tag_indent_2 = m_r2buffer->create_tag( "indent.2" );
    m_tag_indent_2->property_left_margin() = TEXT_MARGIN_L + INDENT_WIDTH * 2;

    m_tag_indent_3 = m_r2buffer->create_tag( "indent.3" );
    m_tag_indent_3->property_left_margin() = TEXT_MARGIN_L + INDENT_WIDTH * 3;

    m_tag_indent_4 = m_r2buffer->create_tag( "indent.4" );
    m_tag_indent_4->property_left_margin() = TEXT_MARGIN_L + INDENT_WIDTH * 4;

    m_tag_match = m_r2buffer->create_tag( "match" ); // highest priority
}

void
TextviewDiary::set_static_text( const Ustring& text )
{
    m_edit_operation_type = EOT::SET_TEXT;
    set_editable( false );
    m_p2entry = nullptr;
    m_r2buffer->set_text( text );
    update_text_formatting();
    m_edit_operation_type = EOT::USER;
}

void
TextviewDiary::set_entry( Entry* entry )
{
    m_edit_operation_type = EOT::SET_TEXT;
    m_p2entry = entry;
    m_p2diary = ( entry->get_diary() ? entry->get_diary() : Diary::d ); // safe for now
    m_p2entry->parse( &m_p2diary->m_parser_bg, true ); // parses if needed for spellcheck
    set_theme( entry->get_theme() );
    // m_para_sel_bgn = m_para_sel_end = nullptr; should not be needed as positons will be set later on anyway
    m_r2buffer->set_text( entry->get_text_visible() ); // recalculates para visibilities
    update_text_formatting();
    set_cursor_position( entry->get_cursor_pos() );
    update_cursor_dependent_positions();
    scroll_to_cursor_center();
    m_edit_operation_type = EOT::USER;
}

void
TextviewDiary::refresh_entry( bool F_scroll_to, bool F_reset_visibilities )
{
    const auto pos_cursor { m_pos_cursor };
    m_edit_operation_type = EOT::SET_TEXT;
    m_r2buffer->set_text( m_p2entry->get_text_visible( F_reset_visibilities ) );
    if( F_scroll_to )
    {
        set_cursor_position( pos_cursor );
        scroll_to_cursor_center();
    }
    update_cursor_dependent_positions();
    update_text_formatting();
    m_edit_operation_type = EOT::USER;
}

int
TextviewDiary::get_text_width( const Pango::FontDescription& fd, const Ustring& str )
{
    int width, height;

    Glib::RefPtr< Pango::Layout > layout{ create_pango_layout( str ) };
    layout->set_font_description( fd );
    layout->get_pixel_size( width, height );

    return width;
}

Ustring
TextviewDiary::get_selected_text() const
{
    if( m_r2buffer->get_has_selection() )
    {
        Gtk::TextConstIter iter_start, iter_end;
        m_r2buffer->get_selection_bounds( iter_start, iter_end );
        return m_r2buffer->get_text( iter_start, iter_end );
    }
    else
        return "";
}

void
TextviewDiary::expand_selection()
{
    static std::map< char, String >
                        delimiters_table;
    const static String delimiters_bgn  { "- \t([{.'\"\n" };   // in ascending order of strength
    const static String delimiters_end  { "- \t)]}.'\"\n" };   // in ascending order of strength
    const auto          delim_no_last   { delimiters_bgn.size() - 1 };
    Gtk::TextIter       it_bgn, it_end;

    auto                stgth_delim_bgn { String::npos };
    auto                stgth_delim_end { String::npos };
    // try to find a limiter stronger than the ones already within the selection:
    String::size_type   stgth_delim_min { 0 };
    int                 num_round       { 0 };  // number of matching delimiter find attempts

    // check if a pair of good delimiters is detected
    auto is_properly_delimited = [ & ]()
        {
            auto kv_end_delim{ delimiters_table.find( it_bgn.get_char() ) };
            if( kv_end_delim == delimiters_table.end() )
                return false;
            const String end_delims{ kv_end_delim->second };
            return( end_delims.find( it_end.get_char() ) != String::npos );
        };

    auto update_stgth_delim_bgn = [ & ]()
        {
            stgth_delim_bgn = ( it_bgn.backward_char() ?
                                delimiters_bgn.find( it_bgn.get_char(), stgth_delim_min ) :
                                delim_no_last );
            // = delim_no_last means boundary of the entry text
        };
    auto update_stgth_delim_end = [ & ]()
        {
            stgth_delim_end = ( it_end.forward_char() ?
                                delimiters_end.find( it_end.get_char(), stgth_delim_min ) :
                                delim_no_last );
            // = delim_no_last means boundary of the entry text
        };

    // initialize the table if needed:
    if( delimiters_table.empty() )
    {
        delimiters_table[ ' ' ]  = " \t.-\n";
        delimiters_table[ '-' ]  = " \t.-\n";
        delimiters_table[ '.' ]  = " \t.\n";
        delimiters_table[ '\t' ] = " \t.-\n";
        delimiters_table[ '\n' ] = " \t\n";
        delimiters_table[ '\"' ] = "\"";
        delimiters_table[ '\'' ] = "\'";
        delimiters_table[ '(' ]  = ")";
        delimiters_table[ '[' ]  = "]";
        delimiters_table[ '{' ]  = "}";
    }

    // get selection:
    m_r2buffer->get_selection_bounds( it_bgn, it_end );

    // set the initial strength values:
    update_stgth_delim_bgn(); // enlarges the area to include the existing delimiter if any
    stgth_delim_end = delimiters_end.find( it_end.get_char() );

    if( stgth_delim_bgn != String::npos && stgth_delim_end != String::npos )
        stgth_delim_min = std::min( stgth_delim_bgn, stgth_delim_end );

    // try to find stronger delimiters outside:
    while( !is_properly_delimited() )
    {
        if( stgth_delim_end == String::npos ||
            ( stgth_delim_bgn != String::npos && stgth_delim_end < stgth_delim_bgn ) )
        {
            update_stgth_delim_end();
        }

        // check if the change to the end already made the delimiters match:
        if( is_properly_delimited() ) break;

        // beware of the last \n that is supposed to be unreachable
        if( stgth_delim_bgn == String::npos ||
            ( stgth_delim_end != String::npos && stgth_delim_bgn < stgth_delim_end ) )
        {
            update_stgth_delim_bgn();
        }

        ++num_round;
    }

    if( !it_bgn.is_start() ) ++it_bgn;
    // if previous selection is within delimiters, match is found on round 0.
    // just include the closing delim in this case:
    if( num_round == 0 )
    {
        if( stgth_delim_bgn < stgth_delim_end )
            m_r2buffer->select_range( --it_bgn, it_end );
        else
            m_r2buffer->select_range( it_bgn, ++it_end );
    }
    else
        m_r2buffer->select_range( it_bgn, it_end );
}

void
TextviewDiary::select_paragraph( const Paragraph* para )
{
    m_r2buffer->select_range( m_r2buffer->get_iter_at_offset( para->get_bgn_offset_in_host() ),
                              m_r2buffer->get_iter_at_offset( para->get_end_offset_in_host() ) );
}

void
TextviewDiary::update_cursor_dependent_positions()
{
    Paragraph* para_sel_bgn, * para_sel_end;
    const bool F_cursor_at_end{ calculate_sel_bounding_paras( para_sel_bgn, para_sel_end ) };
    m_pos_para_sel_bgn = para_sel_bgn->get_bgn_offset_in_host();
    m_pos_para_sel_end = para_sel_end->get_end_offset_in_host();

    m_para_sel_bgn = para_sel_bgn;
    m_para_sel_end = ( m_r2buffer->get_has_selection() ? para_sel_end : nullptr );
    m_para_cursor = ( F_cursor_at_end && m_para_sel_end ? para_sel_end : para_sel_bgn );
    m_pos_cursor_in_para = ( m_pos_cursor - m_para_cursor->get_bgn_offset_in_host() );
}

void
TextviewDiary::update_para_region_basic( int pos_erase_bgn, int pos_erase_end,
                                         Paragraph* para_bgn, Paragraph* para_end,
                                         int offset_cursor )
{
    m_edit_operation_type = EOT::SET_TEXT;

    auto iter{ m_r2buffer->get_iter_at_offset( pos_erase_bgn ) };

    if( pos_erase_bgn <= pos_erase_end ) // otherwise deletion is not desired
        iter = m_r2buffer->erase( iter, m_r2buffer->get_iter_at_offset( pos_erase_end + 1 ) );
        // + 1 is to delete the \n at the end

    if( para_bgn )
        m_r2buffer->insert( iter, m_p2entry->get_text_partial( para_bgn, para_end, true ) );

    m_edit_operation_type = EOT::USER;

    if( para_bgn )
        update_text_formatting( para_bgn, para_end );

    if( offset_cursor >= 0 )
    {
        iter = m_r2buffer->get_iter_at_offset( offset_cursor );
        m_r2buffer->place_cursor( iter );
        scroll_to( iter, 0.05 );
    }
}

void
TextviewDiary::toggle_fold( Paragraph* para )
{
    if( !para->is_foldable() )
        para = para->get_parent();
    if( !para )
        return;

    const bool  F_expanded    { para->is_expanded() };
    auto        para_end_ers  { para->get_sub_last_visible() };
    if( !para_end_ers ) return;

    auto        para_sub_last { para->get_sub_last() };
    auto        para_end_ins  { F_expanded ? para : para_sub_last };
    if( !para_end_ins ) return;

    const int   pos_bgn      { para->get_bgn_offset_in_host() };
    const int   pos_end      { para_end_ers->get_end_offset_in_host() };

    para->set_expanded( !F_expanded );

    update_para_region_basic( pos_bgn, pos_end, para, para_end_ins, pos_bgn );
}

void
TextviewDiary::toggle_collapse_all_subheaders()
{
    int   F_all_subhs_collapsed     { 0 };
    int   F_all_subhs_expanded      { 0 };
    int   F_all_subsubhs_collapsed  { 0 };

    for( Paragraph* para = m_p2entry->get_paragraph_1st(); para; para = para->get_next() )
    {
        if( para->is_subheader() && para->has_subs() )
        {
            if( para->get_heading_level() == VT::PS_SUBHDR )
            {
                if( para->is_expanded() )
                {
                    F_all_subhs_collapsed = -1;
                    if( !F_all_subhs_expanded ) F_all_subhs_expanded = 1;
                }
                else
                {
                    F_all_subhs_expanded = -1;
                    if( !F_all_subhs_collapsed ) F_all_subhs_collapsed = 1;
                }
            }
            else // subsubheader
            {
                if( para->is_expanded() ) F_all_subsubhs_collapsed = -1;
                else if( !F_all_subsubhs_collapsed ) F_all_subsubhs_collapsed = 1;
            }
        }
    }

    m_ongoing_operation_depth++;
    for( Paragraph* para = m_p2entry->get_paragraph_1st(); para; para = para->get_next() )
    {
        if     ( para->get_heading_level() == VT::PS_SUBHDR )
        {
            if( F_all_subhs_collapsed > 0 )
                para->set_expanded( true );
            else if( !( F_all_subhs_expanded > 0 && F_all_subsubhs_collapsed > 0 )
                     && para->is_expanded() )
                para->set_expanded( false );
        }
        else if( para->get_heading_level() == VT::PS_SSBHDR )
        {
            // expand subsubhs if subheaders are expanded (i.e. 1) or non-existent (i.e. -1)
            if( F_all_subhs_expanded >= 0 && F_all_subsubhs_collapsed > 0 )
                para->set_expanded( true );
            else if( para->is_expanded() )
                para->set_expanded( false );
        }
    }

    refresh_entry();

    m_ongoing_operation_depth--;
}

void
TextviewDiary::set_theme( const Theme* theme )
{
    // when theme == nullptr, it works in refresh mode:
    m_p2theme = theme ? theme : m_p2entry->get_theme();

    // set theme by changing the name:
    set_name( m_p2theme->get_css_class_name() );

    m_tag_heading->property_foreground_rgba() = m_p2theme->color_heading;
    m_tag_subheading->property_foreground_rgba() = m_p2theme->color_subheading;
    m_tag_subsubheading->property_foreground_rgba() = m_p2theme->color_subsubheading;

    m_tag_highlight->property_background_rgba() = m_p2theme->color_highlight;

    //m_tag_inline_tag_bg->property_background_rgba() =  m_p2theme->color_inline_tag;
    m_tag_inline_value->property_background_rgba() = m_p2theme->color_inline_tag;

    m_tag_comment->property_foreground_rgba() =
            ( m_p2entry && m_p2entry->get_comment_style() == VT::CS::HILITED::I ?
                                                             m_p2theme->color_text :
                                                             m_p2theme->color_mid );
    m_tag_markup->property_foreground_rgba() = m_p2theme->color_pale;
    // disables #highlighting#
    m_tag_comment->property_background_rgba() =
            ( m_p2entry && m_p2entry->get_comment_style() == VT::CS::HILITED::I ?
                                                             m_p2theme->color_highlight :
                                                             m_p2theme->color_base );
    m_tag_markup->property_background_rgba() = m_p2theme->color_base;

    m_tag_comment->property_invisible() = ( m_p2entry &&
                                            m_p2entry->get_comment_style() == VT::CS::HIDDEN::I );
    m_tag_markup->property_invisible() = ( m_p2entry &&
                                           m_p2entry->get_comment_style() == VT::CS::HIDDEN::I );

    m_tag_region->property_paragraph_background_rgba() = m_p2theme->color_region_bg;
    //m_tag_quotation->property_paragraph_background_rgba() = m_p2theme->get_color_region_bg();

    m_tag_match->property_foreground_rgba() = m_p2theme->color_base;
    m_tag_match->property_background_rgba() = m_p2theme->color_match_bg;

    m_tag_link->property_foreground_rgba() = m_p2theme->color_link;
    m_tag_link_no_underline->property_foreground_rgba() = m_p2theme->color_link;
    m_tag_link_broken->property_foreground_rgba() = m_p2theme->color_link_broken;

    m_tag_todo->property_foreground_rgba() = m_p2theme->color_link_broken;
    m_tag_done->property_foreground_rgba() = m_p2theme->color_done_text;
    m_tag_done->property_background_rgba() = m_p2theme->color_done_bg;
}

// PARSING
void
TextviewDiary::update_text_formatting( int pos_update_bgn, int pos_update_end )
{
    int pos_para_bgn { 0 }, pos_para_end { 0 };

    m_ongoing_operation_depth++;

    for( Paragraph* p = m_p2entry->get_paragraph_1st(); p; p = p->get_next_visible() )
    {
        pos_para_end = ( pos_para_bgn + p->get_size() );

        if( pos_para_end >= pos_update_bgn ) // pos_para_end == pos_update_bgn means empty para
            process_paragraph( p, m_r2buffer->get_iter_at_offset( pos_para_bgn ),
                                  m_r2buffer->get_iter_at_offset( pos_para_end ) );

        if( pos_para_end >= pos_update_end )
            break;

        pos_para_bgn = ( pos_para_end + 1 );
    }

    m_ongoing_operation_depth--;
}

void
TextviewDiary::update_text_formatting( Paragraph* para_bgn, Paragraph* para_end )
{
    int offset{ para_bgn->get_bgn_offset_in_host() }, offset_end;

    m_ongoing_operation_depth++;

    for( Paragraph* p = para_bgn; p; p = p->get_next_visible() )
    {
        offset_end = ( offset + p->get_size() );

        process_paragraph( p, m_r2buffer->get_iter_at_offset( offset ),
                              m_r2buffer->get_iter_at_offset( offset_end ) );

        if( p == para_end )
            break;

        offset = ( offset_end + 1 );
    }

    m_ongoing_operation_depth--;
}

void
TextviewDiary::process_paragraph( Paragraph* para,
                                  const Gtk::TextIter& it_bgn,
                                  const Gtk::TextIter& it_end0,
                                  bool F_remove_tags )
{
    bool F_update_title         { false };
    bool F_update_status        { false };
    const Gtk::TextIter it_end  { it_bgn == it_end0 ? std::next( it_end0 ) : it_end0 };
    // we need to guard against the empty para situation

    if( F_remove_tags )
        m_r2buffer->remove_all_tags( it_bgn, it_end );

    // ALIGNMENT
    switch( para->get_alignment() )
    {
        case VT::PS_ALIGN_L:
            m_r2buffer->apply_tag( m_tag_justify_left, it_bgn, it_end );
            break;
        case VT::PS_ALIGN_C:
            m_r2buffer->apply_tag( m_tag_justify_center, it_bgn, it_end );
            break;
        case VT::PS_ALIGN_R:
            m_r2buffer->apply_tag( m_tag_justify_right, it_bgn, it_end );
            break;
    }

    // HEADING TYPE
    switch( para->get_heading_level() )
    {
        case VT::PS_HEADER:
            m_r2buffer->apply_tag( m_tag_heading, it_bgn, it_end );
            if( m_edit_operation_type != EOT::SET_TEXT && is_the_main_editor() )
            {
                AppWindow::p->UI_diary->handle_entry_title_changed( m_p2entry );
                AppWindow::p->UI_entry->handle_title_edited();
            }
            break;
        case VT::PS_SUBHDR:
            m_r2buffer->apply_tag( m_tag_subheading, it_bgn, it_end );
            break;
        case VT::PS_SSBHDR:
            m_r2buffer->apply_tag( m_tag_subsubheading, it_bgn, it_end );
            break;
    }

    // first paragraph is exempt from below
    // checking for search editor is not needed as para hosts point to original host now
    if( !para->is_header() )
    {
        // LIST ITEM TYPE
        // applying these tags to empty paragraphs breaks the formatting of the following para:
        if( it_bgn != it_end0 )
        {
            switch( para->get_list_type() )
            {
                // case VT::PS_BULLET:
                //     break;
                case VT::PS_TODO:
                    m_r2buffer->apply_tag( m_tag_todo, it_bgn, it_end );
                    break;
                // case VT::PS_PROGRS:
                //     break;
                case VT::PS_DONE:
                    m_r2buffer->apply_tag( m_tag_done, it_bgn, it_end );
                    break;
                case VT::PS_CANCLD:
                    m_r2buffer->apply_tag( m_tag_canceled, it_bgn, it_end );
                    break;
            }
        }

        if( para->is_code() )
            m_r2buffer->apply_tag( m_tag_region, it_bgn, it_end );

        if( para->is_quote() )
            m_r2buffer->apply_tag( m_tag_quotation, it_bgn, it_end );

        if( para->is_image() && m_max_thumbnail_w > 0 )
        {
            try
            {
                auto buf { para->get_image( m_max_thumbnail_w, m_p2theme->font ) };
                if( buf )
                    show_image( buf, it_bgn, it_end );
            }
            catch( Glib::FileError& er )
            {
                print_error( "Link target not found" );
                m_r2buffer->apply_tag( m_tag_region, it_bgn, it_end );
            }
            catch( LIFEO::Error& er )
            {
                print_error( "Chart/Table not found" );
                m_r2buffer->apply_tag( m_tag_region, it_bgn, it_end );
            }
            catch( Gdk::PixbufError& er )
            {
                print_error( "File is not an image" );
                m_r2buffer->apply_tag( m_tag_region, it_bgn, it_end );
            }
        }

        // INDENTATION
        // TODO: 3.1: for empty paragraphs this breaks the indentation of the following paragraph
        switch( para->get_indent_level() )
        {
            case 1: m_r2buffer->apply_tag( m_tag_indent_1, it_bgn, it_end ); break;
            case 2: m_r2buffer->apply_tag( m_tag_indent_2, it_bgn, it_end ); break;
            case 3: m_r2buffer->apply_tag( m_tag_indent_3, it_bgn, it_end ); break;
            case 4: m_r2buffer->apply_tag( m_tag_indent_4, it_bgn, it_end ); break;
        }
    }

    if( para->m_order_in_host == 1 &&
        m_p2entry->get_title_style() == VT::ETS::NAME_AND_DESCRIPT::I )
        F_update_title = true;

    // HIDDEN FORMATS
    for( auto format : para->m_formats )
    {
        Glib::RefPtr< Gtk::TextTag > formatting_tag;
        switch( format->type )
        {
            case VT::HFT_TIME_MS:       formatting_tag = m_tag_smaller; break;
            case VT::HFT_DATE:
                if( m_edit_operation_type != EOT::SET_TEXT &&
                    m_p2entry->get_title_style() == VT::ETS::DATE_AND_NAME::I )
                    F_update_title = true;
            case VT::HFT_LINK_ONTHEFLY:
            case VT::HFT_LINK_ID:
            case VT::HFT_LINK_EVAL:
            case VT::HFT_LINK_URI:      formatting_tag = m_tag_link; break;
            case VT::HFT_DATE_ELLIPSIS: formatting_tag = m_tag_link_no_underline; break;
            case VT::HFT_LINK_BROKEN:   formatting_tag = m_tag_link_broken; break;
            case VT::HFT_BOLD:          formatting_tag = m_tag_bold; break;
            case VT::HFT_ITALIC:        formatting_tag = m_tag_italic; break;
            case VT::HFT_HIGHLIGHT:     formatting_tag = m_tag_highlight; break;
            case VT::HFT_STRIKETHRU:    formatting_tag = m_tag_strikethru; break;
            case VT::HFT_UNDERLINE:     formatting_tag = m_tag_underline; break;
            case VT::HFT_SUBSCRIPT:     formatting_tag = m_tag_subscript; break;
            case VT::HFT_SUPERSCRIPT:   formatting_tag = m_tag_superscript; break;
            case VT::HFT_MISSPELLED:    formatting_tag = m_tag_misspelled; break;
            case VT::HFT_COMMENT:
            {
                const auto pos_brackets_bgn{ it_bgn.get_offset() };

                m_r2buffer->apply_tag( m_tag_markup,
                        m_r2buffer->get_iter_at_offset( pos_brackets_bgn + format->pos_bgn ),
                        m_r2buffer->get_iter_at_offset( pos_brackets_bgn + format->pos_bgn + 2 ) );
                if( format->pos_bgn < format->pos_end - 4 )
                    m_r2buffer->apply_tag( m_tag_comment,
                            m_r2buffer->get_iter_at_offset( pos_brackets_bgn
                                                            +
                                                            format->pos_bgn + 2 ),
                            m_r2buffer->get_iter_at_offset( pos_brackets_bgn
                                                            +
                                                            format->pos_end - 2 ) );
                m_r2buffer->apply_tag( m_tag_markup,
                        m_r2buffer->get_iter_at_offset( pos_brackets_bgn + format->pos_end - 2 ),
                        m_r2buffer->get_iter_at_offset( pos_brackets_bgn + format->pos_end ) );
                continue;
            }
            case VT::HFT_MATCH:         formatting_tag = m_tag_match; break;
            case VT::HFT_TAG_VALUE:     formatting_tag = m_tag_inline_value; break;
            case VT::HFT_TAG:           formatting_tag = m_tag_inline_tag;
                if( m_edit_operation_type != EOT::SET_TEXT &&
                    m_p2diary && m_p2diary->get_completion_tag() &&
                    format->ref_id == m_p2diary->get_completion_tag()->get_id() )
                    F_update_status = true;
                break;
            default:                    continue;
        }

        Gtk::TextIter it_format_bgn = m_r2buffer->get_iter_at_offset( it_bgn.get_offset()
                                                                      + format->pos_bgn );
        Gtk::TextIter it_format_end = m_r2buffer->get_iter_at_offset( it_bgn.get_offset()
                                                                      + format->pos_end );
        m_r2buffer->apply_tag( formatting_tag, it_format_bgn, it_format_end );
    }

    if( F_update_title )
        AppWindow::p->UI_diary->handle_entry_title_changed( m_p2entry );
    if( F_update_status )
    {
        m_p2entry->update_todo_status();
        AppWindow::p->UI_diary->refresh_row( m_p2entry );
    }
}

// LINKS & IMAGES
void
TextviewDiary::show_image( const R2Pixbuf& buf, Gtk::TextIter it_bgn, Gtk::TextIter it_end )
{
    const auto&& tag_name   { STR::compose( "height.", buf->get_height() ) };
    auto         tag_height { m_r2buffer->get_tag_table()->lookup( tag_name ) };
    if( !tag_height )
    {
        tag_height = m_r2buffer->create_tag( tag_name );
        tag_height->property_pixels_below_lines() = buf->get_height();
    }
    m_r2buffer->apply_tag( tag_height, it_bgn, it_end );
}

bool
TextviewDiary::update_image_width()
{
    int width{ get_width() };

    if( m_last_width == width )
        return false;
    else
        m_last_width = width;

    width -= int( 2.1 * TEXT_MARGIN_L );
    // only multitudes of 100 are accepted:
    width -= ( width % 100 );

    if( width == m_max_thumbnail_w )
        return false;
    else
    {
        m_max_thumbnail_w = width;
        return true;
    }
}

// HELPER FUCNTIONS
// bool
// TextviewDiary::check_cursor_is_in_para()
// {
//     return( ( m_parser_pos_cur > m_pos_para_sel_bgn ) &&
//             ( m_parser_pos_cur <= m_pos_para_sel_end ) );
// }

void
TextviewDiary::calculate_para_bounds( Gtk::TextIter& it_bgn, Gtk::TextIter& it_end )
{
    if( ! it_bgn.backward_find_char( s_predicate_nl ) )
        it_bgn.set_offset( 0 );
    else
        it_bgn++;

    if( !it_end.ends_line() )
        if( !it_end.forward_find_char( s_predicate_nl ) )
            it_end.forward_to_end();
}

void
TextviewDiary::calculate_para_bounds( UstringSize& pos_bgn, UstringSize& pos_end )
{
    auto para_bgn{ m_p2entry->get_paragraph( pos_bgn ) };
    auto para_end{ m_p2entry->get_paragraph( pos_end ) };

    if( !para_bgn || !para_end ) return;

    pos_bgn = para_bgn->get_bgn_offset_in_host();
    pos_end = para_end->get_end_offset_in_host();
}

bool    // returns true if boundaries change at the end
TextviewDiary::calculate_sel_word_bounds( Gtk::TextIter& iter_bgn, Gtk::TextIter& iter_end )
{
    bool flag_iters_moved{ true };

    m_r2buffer->get_selection_bounds( iter_bgn, iter_end );

    if( iter_bgn.starts_word() )
        flag_iters_moved = false;
    else
        iter_bgn.backward_word_start();

    if( iter_end.ends_word() )
        return flag_iters_moved;
    else
        iter_end.forward_word_end();

    return true;
}

bool    // returns true if boundaries reach the end of text
TextviewDiary::calculate_sel_para_bounds( Gtk::TextIter& iter_bgn, Gtk::TextIter& iter_end )
{
    if( m_r2buffer->get_has_selection() )
        m_r2buffer->get_selection_bounds( iter_bgn, iter_end );
    else
        iter_bgn = iter_end = m_r2buffer->get_iter_at_mark( m_r2buffer->get_insert() );

    calculate_para_bounds( iter_bgn, iter_end );

    return iter_end.is_end();
}

bool
TextviewDiary::calculate_sel_bounding_paras( Paragraph*& para_bgn, Paragraph*& para_end )
{
    Gtk::TextIter iter_bgn, iter_end;
    m_r2buffer->get_selection_bounds( iter_bgn, iter_end );
    para_bgn = m_p2entry->get_paragraph( iter_bgn.get_offset(), true );
    para_end = m_p2entry->get_paragraph( iter_end.get_offset(), true );

    return iter_end.is_cursor_position();
}

Ustring
TextviewDiary::calculate_word_bounds( Gtk::TextIter& iter_bgn, Gtk::TextIter& iter_end )
{
    if( ! iter_bgn.backward_find_char( s_predicate_blank ) )
        iter_bgn.set_offset( 0 );
    else
        iter_bgn++;

    iter_end--;
    if( !iter_end.forward_find_char( s_predicate_blank ) )
        iter_end.forward_to_end();

    return m_r2buffer->get_slice( iter_bgn, iter_end );
}

// CLIPBOARD
void
TextviewDiary::copy_clipboard( bool F_cut )
{
    Gtk::TextIter     it_bgn, it_end;
    m_r2buffer->get_selection_bounds( it_bgn, it_end );
    const UstringSize pos_bgn{ ( UstringSize ) it_bgn.get_offset() };
    const UstringSize pos_end{ ( UstringSize ) it_end.get_offset() };

    // internal register for entry text:
    Lifeograph::p->set_entry_text_register_cb(
            m_p2entry->get_text_partial_paras( pos_bgn, pos_end ) );

    // text/plain:
    Glib::Value< Ustring > value_ustring;
    value_ustring.init( value_ustring.value_type() );
    value_ustring.set( m_p2entry->get_text_partial( pos_bgn, pos_end, true ) ); // true: decorated
    auto content_provider_text_pln{ Gdk::ContentProvider::create( value_ustring ) };
    // text/lifeograph entry:
    auto bytes { Glib::Bytes::create( Lifeograph::p->get_entry_text_register_cb(), 1 ) };
    auto content_provider_text_log{ Gdk::ContentProvider::create( MIME_TEXT_LOG, bytes ) };
    // combined:
    std::vector< Glib::RefPtr< Gdk::ContentProvider > > content_providers;
    content_providers.push_back( content_provider_text_pln );
    content_providers.push_back( content_provider_text_log );
    auto content_provider_combined{ Gdk::ContentProvider::create( content_providers ) };

    if( !get_clipboard()->set_content( content_provider_combined ) )
        print_error( "Copying to clipboard failed!" );

    // TODO: 3.2: implement rich text for external applications

    if( F_cut )
        m_r2buffer->erase( it_bgn, it_end );
}

void
TextviewDiary::calculate_token_bounds( Gtk::TextIter& it_bgn, Gtk::TextIter& it_end, int type )
{
    if( m_r2buffer->get_has_selection() )
    {
        m_r2buffer->get_selection_bounds( it_bgn, it_end );
        return;
    }
    else if( m_para_sel_bgn )
    {
        if( auto f = m_para_sel_bgn->get_format_at( type, m_pos_cursor_in_para ) )
        {
            it_bgn = m_r2buffer->get_iter_at_offset( m_pos_para_sel_bgn + f->pos_bgn );
            it_end = m_r2buffer->get_iter_at_offset( m_pos_para_sel_bgn + f->pos_end );
            return;
        }
    }

    // if no selection and no existing tag:
    it_bgn = it_end = m_r2buffer->get_insert()->get_iter();
    const Wchar ch_end { it_end.get_char() };

    // find word boundaries:
    if( it_bgn.backward_find_char( type == VT::HFT_TAG ? &STR::is_char_not_name
                                                       : &STR::is_char_space )  )
        ++it_bgn;

    if( type == VT::HFT_TAG )
    {
        if( STR::is_char_name( ch_end ) )
            it_end.forward_find_char( &STR::is_char_not_name );
    }
    else if( !STR::is_char_space( ch_end ) )
        it_end.forward_find_char( &STR::is_char_space );

    // if omitting punctuation did not end well:
    if( type == VT::HFT_TAG && it_bgn == it_end )
    {
        if( it_bgn.backward_find_char( &STR::is_char_space ) )
            ++it_bgn;

        if( !STR::is_char_space( ch_end ) )
            it_end.forward_find_char( &STR::is_char_space );
    }
}

// TEXTVIEW ========================================================================================
TextviewDiary::TextviewDiary( int text_margin )
:   TEXT_MARGIN_L( text_margin + EXPANDER_WIDTH ),
    TEXT_MARGIN_R( text_margin ),
    PAGE_MARGIN( text_margin - BULLET_COL_WIDTH ),
    QUOT_MARGIN( PAGE_MARGIN + INDENT_WIDTH )
{
    init();
}

TextviewDiary::TextviewDiary( BaseObjectType* cobject, const Glib::RefPtr< Gtk::Builder >&,
                              int text_margin )
:   Gtk::TextView( cobject ),
    TEXT_MARGIN_L( text_margin + EXPANDER_WIDTH ),
    TEXT_MARGIN_R( text_margin ),
    PAGE_MARGIN( text_margin - BULLET_COL_WIDTH ),
    QUOT_MARGIN( PAGE_MARGIN + INDENT_WIDTH )
{
    init();
}

inline void
TextviewDiary::init()
{
    set_buffer( m_r2buffer = TextbufferDiary::create() );

    set_wrap_mode( Gtk::WrapMode::WORD );
    set_left_margin( TEXT_MARGIN_L );
    set_right_margin( TEXT_MARGIN_R );
    set_pixels_above_lines( 4 );
    set_has_tooltip();

    init_tags();

    signal_query_tooltip().connect( sigc::mem_fun( *this, &TextviewDiary::handle_query_tooltip ),
                                    true );

    m_controller_motion = Gtk::EventControllerMotion::create();
    m_controller_motion->signal_motion().connect(
            sigc::mem_fun( *this, &TextviewDiary::on_motion_notify_event ), false );
    add_controller( m_controller_motion );

    m_gesture_click = Gtk::GestureClick::create();
    m_gesture_click->set_button( 0 );
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &TextviewDiary::on_button_pressed ), true );
    m_gesture_click->signal_released().connect(
            sigc::mem_fun( *this, &TextviewDiary::on_button_release_event ), true );
    add_controller( m_gesture_click );
}

bool
TextviewDiary::get_selection_rect( Gdk::Rectangle& rect )
{
    int             xb, xe, yb, ye; // ye is a placeholder
    Gtk::TextIter   iter_bgn, iter_end;
    Gdk::Rectangle  rect_tmp;

    m_r2buffer->get_selection_bounds( iter_bgn, iter_end );
    get_iter_location( iter_bgn, rect );
    xb = rect.get_x();
    xe = xb + rect.get_width();

    for( auto it = iter_bgn; it != iter_end; ++it )
    {
        get_iter_location( it, rect_tmp );
        xb = std::min( xb, rect_tmp.get_x() );
        xe = std::max( xe, rect_tmp.get_x() + rect_tmp.get_width() );
    }

    buffer_to_window_coords( Gtk::TextWindowType::TEXT, xb, rect.get_y(), xb, yb );
    buffer_to_window_coords( Gtk::TextWindowType::TEXT, xe, rect.get_y(), xe, ye );
    rect.set_x( xb );
    rect.set_y( std::max( 0, yb ) ); // ensure that y is within the visible portion

    rect.set_width( xe - xb );

    return true; // reserved
}

void
TextviewDiary::refresh_entry_icons()
{
    // if( Glib::RefPtr< Gdk::Window > window = get_window( Gtk::TextWindowType::TEXT ) )
    // {
    //     Gdk::Rectangle rect{ PAGE_MARGIN - 47, 8, 36, 120 };
    //     window->invalidate_rect( rect, false );
    // }
    queue_draw();
}

// putting win32 includes in the beginning causes problems
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif
void
TextviewDiary::handle_link_uri( const String& uri )
{
#ifndef _WIN32
    Gio::AppInfo::launch_default_for_uri( m_p2diary->convert_rel_uri( uri ) );
#else
    ShellExecuteW( NULL, L"open", PATH2( m_p2diary->convert_rel_uri( uri ) ),
                   NULL, NULL, SW_SHOWNORMAL );
#endif
}

void
TextviewDiary::handle_para_uri( const Paragraph* para )
{
    switch( para->m_style & VT::PS_FLT_IMAGE )
    {
        case VT::PS_IMAGE_FILE:
            handle_link_uri( para->get_uri() );
            break;
        case  VT::PS_IMAGE_CHART:
        case  VT::PS_IMAGE_TABLE:
            AppWindow::p->show_elem( std::stoul( para->get_uri() ) );
            break;
    }
}
void
TextviewDiary::handle_link_id( DEID id )
{
    Entry* entry{ Diary::d->get_entry_by_id( id ) };
    if( entry )
        AppWindow::p->show_entry( entry );
    else
        print_info( "No link target found with ID=", id );
}
void
TextviewDiary::handle_link_refs_menu()
{
    PoElemChooser::show( m_p2entry->get_references(),
                         []( DiaryElement* e ) { AppWindow::p->show_elem( e ); },
                         *this,
                         Gdk::Rectangle( PAGE_MARGIN - 31, 79, 20, 20 ) );
}

void
TextviewDiary::process_link_uri( const Paragraph* para_hovered, int offset )
{
    set_hovered_link( para_hovered->get_format_oneof_at( VT::HFT_F_LINK, offset ) );

    if( !m_p2hflink_hovered ) return;

    switch( m_p2hflink_hovered->type )
    {
        case VT::HFT_TAG:
        case VT::HFT_LINK_ID:
            m_link_hovered_go = [ this ]() { handle_link_id( m_p2hflink_hovered->ref_id ); };
            break;
        case VT::HFT_DATE:
            m_link_hovered_go = [ this ]() { AppWindow::p->UI_extra->show_date_in_cal(
                                                m_p2hflink_hovered->ref_id ); };
            break;
        case VT::HFT_LINK_URI:
        case VT::HFT_LINK_ONTHEFLY:
            m_link_hovered_go = [ this ]() { handle_link_uri( m_p2hflink_hovered->uri ); };
            break;
        case VT::HFT_LINK_EVAL:
            m_link_hovered_go = [ this ]()
            {
                auto para { m_p2diary->get_element2< Paragraph >( m_p2hflink_hovered->ref_id ) };

                if( para )
                    handle_link_uri( HELPERS::evaluate_path(
                            para->get_substr( m_p2hflink_hovered->pos_bgn,
                                              m_p2hflink_hovered->pos_end ) ) );
            };
            break;
        default:
            set_hovered_link( nullptr );
            break;
    }
}

inline void
TextviewDiary::update_link( double x_pointer, double y_pointer, Gdk::ModifierType modifiers )
{
    const static auto&& s_Cu_hand       { Gdk::Cursor::create( "pointer" ) };
    const static auto&& s_Cu_arrow      { Gdk::Cursor::create( "default" ) };

    Gtk::TextIter       iter;
    auto                ptr2cursor      { &s_Cu_arrow };
    int                 x_buf, y_buf;
    Gdk::Rectangle      rect_iter;
    int                 trailing;
    bool                F_invalidate    { false };

    window_to_buffer_coords( Gtk::TextWindowType::WIDGET, x_pointer, y_pointer, x_buf, y_buf );
    get_iter_at_position( iter, trailing, x_buf, y_buf );
    get_iter_location( iter, rect_iter );

    // check if the cursor is actually on the iter:
    const bool F_is_on_iter         { x_pointer >= rect_iter.get_x() &&
                                      x_pointer <= rect_iter.get_x() + rect_iter.get_width() };
    const bool F_was_on_entry_icon  { get_hovered_link_type() == VT::HFT_ENTRY_MENU };
    const bool F_is_on_entry_icon   { x_pointer < PAGE_MARGIN && y_pointer < 60 };
    const bool F_was_on_refs_icon   { get_hovered_link_type() == VT::HFT_REFS_MENU };
    const bool F_is_on_refs_icon    { x_pointer < PAGE_MARGIN &&
                                      y_pointer > 78 && y_pointer < 99 };

    set_hovered_link( VT::HFT_UNSET );

    m_pos_hovered_char = ( F_is_on_iter ? iter.get_offset() : -1 );

    if( m_p2entry )
    {
        Paragraph* para_hovered { m_p2entry->get_paragraph( iter.get_offset(), true ) };
        Paragraph* para_menu    { para_hovered &&
                                  ( para_hovered->is_header() || // ignore titles
                                    x_pointer > ( TEXT_MARGIN_L +
                                                  para_hovered->get_indent_level() *
                                                  INDENT_WIDTH ) ||
                                    x_pointer < PAGE_MARGIN ||
                                    y_pointer > rect_iter.get_y() + rect_iter.get_height() )
                                  ? nullptr : para_hovered };
        const int  expander_max { para_menu ?
                                  ( para_menu->get_list_type() == VT::PS_BULLET ?
                                    TEXT_MARGIN_L + para_menu->get_indent_level() * INDENT_WIDTH :
                                    TEXT_MARGIN_L + para_menu->get_indent_level() * INDENT_WIDTH -
                                            BULLET_COL_WIDTH )
                                  : TEXT_MARGIN_L };
        const int  left_gap_max { ( para_menu && para_menu->get_list_type() == VT::PS_BULLET ) ?
                                  expander_max : expander_max + BULLET_COL_WIDTH };

        const bool F_fold_hvrd  { x_pointer < left_gap_max };

        // show folding triangles when folding area is hovered:
        if( m_F_fold_area_hovered != F_fold_hvrd )
        {
            m_F_fold_area_hovered = F_fold_hvrd;
            F_invalidate = true;
        }
        else if( F_was_on_entry_icon != F_is_on_entry_icon )
            F_invalidate = true;
        else if( F_was_on_refs_icon != F_is_on_refs_icon )
            F_invalidate = true;

        if( m_para_menu != para_menu )
        {
            m_para_menu = para_menu;
            F_invalidate = true;
        }

        // entry icon
        if( F_is_on_entry_icon /*&& F_was_on_entry_icon*/ )
        {
            set_hovered_link( VT::HFT_ENTRY_MENU );
        }
        // referring entries icon
        else if( F_is_on_refs_icon /* && F_was_on_refs_icon */ && m_p2entry->get_reference_count() > 0 )
        {
            set_hovered_link( VT::HFT_REFS_MENU );
        }
        else if( !para_hovered ) { }
        else if( x_pointer < expander_max &&
                 x_pointer > ( expander_max - EXPANDER_WIDTH ) &&
                 para_hovered->is_foldable() && para_hovered->has_subs() )
        {
            set_hovered_link( para_hovered->is_expanded() ? VT::HFT_FOLD_EXPANDED
                                                          : VT::HFT_FOLD_COLLAPSED );
        }
        else if( para_hovered->is_image() )
        {
            set_hovered_link( VT::HFT_IMAGE );
        }
        // hidden links
        else if( F_is_on_iter )
        {
            process_link_uri( para_hovered,
                              iter.get_offset() - para_hovered->get_bgn_offset_in_host() );
        }
    }

    if( get_hovered_link_type() != VT::HFT_UNSET &&
        ( ( get_hovered_link_type() & VT::HFT_F_ALWAYS ) ||
          ( get_editable() == bool( modifiers & Gdk::ModifierType::CONTROL_MASK ) ) ) )
    {
        ptr2cursor = &s_Cu_hand;
    }
    else if( F_is_on_iter )
        ptr2cursor = &m_cursor_default;

    if( ptr2cursor != m_ptr2cursor_last )
    {
        m_ptr2cursor_last = ptr2cursor;
        set_cursor( *ptr2cursor );
    }

    if( F_invalidate )
        queue_draw();
}

void
TextviewDiary::on_motion_notify_event( double x, double y )
{
    update_link( x, y, m_controller_motion->get_current_event()->get_modifier_state() );
}

void
TextviewDiary::on_button_pressed( int n_press, double x, double y )
{
    // m_link_hovered_go is set here to make sure that a perfect click event occured
    // e.g. not a button release at the end of a drop event
    if( m_p2entry )
    {
        Gtk::TextIter       iter;
        int                 x_buf, y_buf;
        int                 trailing;

        window_to_buffer_coords( Gtk::TextWindowType::WIDGET, x, y, x_buf, y_buf );
        get_iter_at_position( iter, trailing, x_buf, y_buf );

        Paragraph* para_hovered { m_p2entry->get_paragraph( iter.get_offset(), true ) };
        switch( get_hovered_link_type() )
        {
            case VT::HFT_UNSET:
                m_link_hovered_go = nullptr;
                break;
            case VT::HFT_ENTRY_MENU:
                m_link_hovered_go = [ & ](){ AppWindow::p->UI_entry->show_popover(); };
                break;
            case VT::HFT_REFS_MENU:
                m_link_hovered_go = std::bind( &TextviewDiary::handle_link_refs_menu, this );
                break;
            case VT::HFT_FOLD_EXPANDED:
            case VT::HFT_FOLD_COLLAPSED:
                m_link_hovered_go = std::bind( &TextviewDiary::toggle_fold, this, para_hovered );
                break;
            case VT::HFT_IMAGE:
                m_link_hovered_go = [ = ](){ handle_para_uri( para_hovered ); };
                break;
        }
    }
}

void
TextviewDiary::on_button_release_event( int n_press, double x, double y )
{
    if( !m_link_hovered_go ) return;

    const bool  F_ctrl_pressed
                { bool( m_gesture_click->get_current_event()->get_modifier_state() &
                        Gdk::ModifierType::CONTROL_MASK ) };
    const int   link_type{ get_hovered_link_type() };

    if( m_gesture_click->get_current_button() == 3 && ( link_type & VT::HFT_F_R_CLICK_TOO ) )
        m_link_hovered_go();
    else
    if( m_gesture_click->get_current_button() == 1 &&
        link_type != VT::HFT_UNSET &&
        ( ( link_type & VT::HFT_F_ALWAYS ) || get_editable() == F_ctrl_pressed ) )
        m_link_hovered_go();
    else
    if( m_gesture_click->get_current_button() == 1 && m_para_menu )
        select_paragraph( m_para_menu );
}

void
TextviewDiary::snapshot_vfunc( const Glib::RefPtr< Gtk::Snapshot >& snapshot )
{
    Gtk::TextView::snapshot_vfunc( snapshot );

    update_image_width(); //if necessary

    if( Diary::d->is_open() && m_r2buffer && m_p2theme && m_p2entry )
    {
        int unused, y_Tv_top; // treeview coords
        Gtk::TextIter it_visible_bgn, it_visible_end;
        Gdk::Rectangle rect;

        int     w{ get_width() }, h{ get_height() };
        auto    cr{ snapshot->append_cairo( Gdk::Rectangle( 0, 0, w, h ) ) };

        get_visible_rect( rect );
        const int w_Tv = rect.get_width();
        const int h_Tv = rect.get_height();
        buffer_to_window_coords( Gtk::TextWindowType::TEXT, 0, rect.get_y(), unused, y_Tv_top );

        get_iter_at_location( it_visible_bgn, rect.get_x(), rect.get_y() );
        get_iter_at_location( it_visible_end, rect.get_x() + w_Tv, rect.get_y() + h_Tv );

        Paragraph* para_bgn = m_p2entry->get_paragraph( it_visible_bgn.get_offset(), true );
        Paragraph* para_end = m_p2entry->get_paragraph( it_visible_end.get_offset(), true );

        cr->set_line_join( Cairo::Context::LineJoin::ROUND );

        // MARGINS
        if( is_the_main_editor() )
        {
            Gdk::Cairo::set_source_rgba( cr, Gdk::RGBA( "rgba(100, 100, 100, 0.1)" ) );
            cr->rectangle( 0.0, 0.0, PAGE_MARGIN, h_Tv );
            cr->fill();
            cr->rectangle( w_Tv - PAGE_MARGIN, 0.0, PAGE_MARGIN, h_Tv );
            cr->fill();

            if( get_hovered_link_type() == VT::HFT_ENTRY_MENU )
            {
                cr->set_line_width( 2.0 );
                Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_heading );
                cr->rectangle( PAGE_MARGIN - 47, 8.0, 36.0, 36.0 );
                cr->stroke();
            }

            Gdk::Cairo::set_source_pixbuf( cr, m_p2entry->get_icon32(), PAGE_MARGIN - 45, 10.0 );
            cr->paint();

            if( m_p2entry->is_favorite() )
            {
                Gdk::Cairo::set_source_pixbuf( cr, Lifeograph::icons->favorite_16,
                                               PAGE_MARGIN - 29, 55.0 );
                cr->paint();
            }

            if( m_p2entry->get_reference_count() > 0 )
            {
                Gdk::Cairo::set_source_pixbuf( cr, Lifeograph::icons->tag_16,
                                               PAGE_MARGIN - 29, 81.0 );
                cr->paint();

                if( get_hovered_link_type() == VT::HFT_REFS_MENU )
                {
                    cr->set_line_width( 2.0 );
                    Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_heading );
                    cr->rectangle( PAGE_MARGIN - 31, 79, 20.0, 20.0 );
                    cr->stroke();
                }
            }
        }

        // HANDLE PARAGRAPH ATTRIBUTES
        if( para_end && para_end->m_p2next ) para_end = para_end->m_p2next;
        int             para_bgn_offset { para_bgn->get_bgn_offset_in_host() };
        Gtk::TextIter   it_para_bgn;
        Gdk::Rectangle  rect_iter;
        int             x_para{ 0 }, y_para{ 0 }, y_para_margin; // left, top and margin-top coords
        int             y_hover_top{ 0 }, y_quot_top{ 0 }, y_quot_bottom{ 0 }, h_line{ 0 };
        Paragraph*      p, * p_last{ nullptr };
        auto&&          textlayout{ Pango::Layout::create( cr ) };
        auto&&          textlayout_header{ Pango::Layout::create( cr ) };
        Pango::AttrList attrlist;
        auto&&          attscale { Pango::Attribute::create_attr_scale( 1.4 ) };

        auto draw_icon = [ & ]( R2Pixbuf& buf )
        {
            Gdk::Cairo::set_source_pixbuf( cr, buf,
                    PAGE_MARGIN + EXPANDER_WIDTH + 10 + p->get_indent_level() * INDENT_WIDTH,
                    y_para + ( ( rect_iter.get_height() - buf->get_height() ) / 2.0 ) );
            cr->paint();
        };
        auto draw_icon_location = [ & ]()
        {
            // ICON
            Gdk::Cairo::set_source_pixbuf( cr, Lifeograph::icons->map_point,
                    PAGE_MARGIN + 2,
                    y_para + ( ( rect_iter.get_height() - 16 ) / 2.0 ) );
            cr->paint();

            // PLACEHOLDER TEXT
            if( !p->is_empty() ) return;

            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_pale );
            textlayout->set_text( STR::compose( '(', p->m_location.latitude, ", ",
                                                p->m_location.longitude, ')' ) );
            cr->move_to( PAGE_MARGIN + EXPANDER_WIDTH + 45 +
                         p->get_indent_level() * INDENT_WIDTH,
                         y_para );
            textlayout->show_in_cairo_context( cr );
        };
        auto draw_bullet = [ & ]()
        {
            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_subsubheading );
            cr->begin_new_path();
            cr->set_line_width( 1.5 );
            cr->arc( PAGE_MARGIN + EXPANDER_WIDTH + 18 + p->get_indent_level() * INDENT_WIDTH,
                     y_para + ( rect_iter.get_height() / 2.0 ),
                     3.5, 0.0, 2 * HELPERS::PI );
            if( p->get_indent_level() % 2 )
                cr->fill();
            else
                cr->stroke();
        };
        auto draw_folding = [ & ]()
        {
            const int indetation { p->get_indent_level() * INDENT_WIDTH +
                                   ( p->get_list_type() == VT::PS_BULLET ? EXPANDER_WIDTH - 7
                                                                         : 0 ) };
            if( p->is_expanded() )
            {
                cr->move_to( PAGE_MARGIN + indetation + 17,
                             y_para - 4 + ( rect_iter.get_height() / 2.0 ) );
                cr->rel_line_to( 16, 0 );
                cr->rel_line_to( -8, 8 );
            }
            else
            {
                cr->move_to( PAGE_MARGIN + indetation + 21,
                             y_para - 8 + ( rect_iter.get_height() / 2.0 ) );
                cr->rel_line_to( 0, 16 );
                cr->rel_line_to( 8, -8 );
            }

            Gdk::Cairo::set_source_rgba(
                    cr,
                    ( p->get_heading_level() == VT::PS_SUBHDR ) ? m_p2theme->color_subheading
                                                                : m_p2theme->color_subsubheading );
            cr->set_line_width( 1.5 );
            cr->close_path();
            cr->fill();
        };
        auto draw_number = [ & ]()
        {
            int textw, texth;
            auto& tl { p->get_heading_level() == VT::PS_SUBHDR ? textlayout_header : textlayout };
            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_subsubheading );
            tl->set_text( p->get_list_order_str() );
            tl->get_pixel_size( textw, texth );
            cr->move_to( PAGE_MARGIN + EXPANDER_WIDTH + 27 +
                         p->get_indent_level() * INDENT_WIDTH - textw,
                         y_para );
            tl->show_in_cairo_context( cr );
        };
        auto draw_quotation_end = [ & ]()
        {
            cr->set_line_width( 1.0 );
            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_heading );
            cr->rectangle( QUOT_MARGIN, y_quot_top,
                           w_Tv - 2 * QUOT_MARGIN, y_quot_bottom - y_quot_top );
            cr->stroke();
        };
        auto draw_tag = [ & ]( HiddenFormat* format )
        {
            const double half_height { ( m_tag_coords[ 3 ] - m_tag_coords[ 1 ] ) / 2.0 };

            Gdk::Cairo::set_source_rgba(
                cr,
                format->uri.empty() ? m_p2entry->get_theme()->color_mid
                                    : Gdk::RGBA( format->uri ) );
            cr->set_line_width( 1.2 );

            for( unsigned i = 0; i < m_tag_coords.size(); i += 4 )
            {
                const double width { double( m_tag_coords[ i + 2 ] - m_tag_coords[ i ] ) };
                cr->move_to( m_tag_coords[ i + 2 ], m_tag_coords[ i + 3 ] );
                cr->rel_line_to( -width, 0.0 ); // top horizontal

                if( i == 0 ) // beginning
                    cr->rel_line_to( 0.0, -2.0 * half_height );
                else
                    cr->rel_move_to( 0.0, -2.0 * half_height );

                cr->rel_line_to( width, 0.0 ); // bottom horizontal

                if( i == m_tag_coords.size() - 4 ) // end
                {
                    cr->rel_line_to( half_height / 3.0, half_height );
                    cr->line_to( m_tag_coords[ i + 2 ], m_tag_coords[ i + 3 ] );
                }
            }

            if( format->ref_id == DEID_UNSET )
            {
                cr->set_dash( m_dash_pattern, 0.0 );
                cr->stroke();
                cr->unset_dash();
            }
            else
                cr->stroke();
        };
        auto draw_hover_rect = [ & ]()
        {
            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_subheading );
            cr->set_line_width( 2.0 );
            cr->rectangle( PAGE_MARGIN, y_hover_top,
                           w_Tv - 2 * PAGE_MARGIN, y_para_margin - y_hover_top + h_line );
            cr->stroke();
        };

        textlayout->set_font_description( m_p2theme->font );
        textlayout_header->set_font_description( m_p2theme->font );
        attrlist.insert( attscale );
        textlayout_header->set_attributes( attrlist );

        // PARAGRAPHS
        for( p = para_bgn; p; p = p->get_next_visible() )
        {
            it_para_bgn = m_r2buffer->get_iter_at_offset( para_bgn_offset );
            get_iter_location( it_para_bgn, rect_iter );
            buffer_to_window_coords( Gtk::TextWindowType::TEXT, rect_iter.get_x(),
                                     rect_iter.get_y(), x_para, y_para );

            y_para_margin = ( p->is_subheader() ? y_para - PIXELS_ABOVESUBHDR : y_para );

            get_line_yrange( it_para_bgn, unused, h_line );

            switch( p->get_list_type() )
            {
                case VT::PS_BULLET:
                    if( !p->is_foldable() || !p->has_subs() ) draw_bullet();
                    break;
                case VT::PS_NUMBER:
                case VT::PS_CLTTR:
                case VT::PS_SLTTR:
                case VT::PS_CROMAN:
                case VT::PS_SROMAN: draw_number(); break;
                case VT::PS_TODO:   draw_icon( Lifeograph::icons->todo_open_16 ); break;
                case VT::PS_PROGRS: draw_icon( Lifeograph::icons->todo_progressed_16 ); break;
                case VT::PS_DONE:   draw_icon( Lifeograph::icons->todo_done_16 ); break;
                case VT::PS_CANCLD: draw_icon( Lifeograph::icons->todo_canceled_16 ); break;
            }

            // IMAGE
            if( p->is_image() )
            {
                try
                {
                    R2Pixbuf buf{ p->get_image( m_max_thumbnail_w, m_p2theme->font ) };
                    if( buf )
                    {
                        Gdk::Cairo::set_source_pixbuf( cr, buf,
                                                       ( w_Tv - buf->get_width() ) / 2.0,
                                                       y_para + rect_iter.get_height() / 2.0 );
                        cr->paint();
                    }
                    else
                    {
                        int w, h;
                        Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_pale );
                        textlayout->set_text( _( "Image Not Found" ) );
                        textlayout->get_pixel_size( w, h );
                        const int line_len
                                { ( w_Tv - TEXT_MARGIN_L - TEXT_MARGIN_R - 10 - w ) / 2 };
                        cr->move_to( TEXT_MARGIN_L + line_len + 5, y_para );
                        textlayout->show_in_cairo_context( cr );

                        cr->set_line_width( 3.0 );
                        cr->move_to( TEXT_MARGIN_L, y_para + h / 2.0 );
                        cr->rel_line_to( line_len, 0.0 );
                        cr->rel_move_to( w + 10, 0.0 );
                        cr->rel_line_to( line_len, 0.0 );
                        cr->stroke();
                    }
                }
                catch( ... ) { }
            }

            // HORIZONTAL RULE
            if( p->is_hrule() )
            {
                const double y_pos{ p->is_empty() ? ( y_para_margin + h_line / 2.0 )
                                                  : ( y_para_margin + h_line - 2.0 ) };
                Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_text );
                cr->set_line_width( 1.0 );
                cr->move_to( TEXT_MARGIN_L, y_pos );
                cr->rel_line_to( w_Tv - TEXT_MARGIN_L
                                      - ( p->is_quote()
                                            ? ( PAGE_MARGIN + BULLET_COL_WIDTH + EXPANDER_WIDTH )
                                            : TEXT_MARGIN_R ),
                                 0.0 );
                cr->stroke();
            }

            // QUOTATION
            if( p->is_quote() )
            {
                if( !p_last || !p_last->is_quote() )
                    y_quot_top = y_para - 3;
                y_quot_bottom = ( y_para + h_line - 3 );
            }
            else if( p_last && p_last->is_quote() )
            {
                draw_quotation_end();
            }

            // FOLDING
            if( is_the_main_editor() && p->is_foldable() &&
                ( m_F_fold_area_hovered || p->get_list_type() == VT::PS_BULLET ||
                  !p->is_expanded() ) )
            {
                draw_folding();
            }

            // MAP LOCATION
            if( p->has_location() )
                draw_icon_location();

            // TAGS
            for( auto format : p->m_formats )
            {
                if( format->type == VT::HFT_TAG )
                {
                    m_tag_coords.clear();
                    get_tag_coordinates( para_bgn_offset + format->pos_bgn,
                                         para_bgn_offset + format->pos_end - 1 );
                    draw_tag( format );
                }
            }

            // HOVERED PARA
            if( m_para_menu && is_the_main_editor() )
            {
                // single line case:
                if( p == m_para_menu &&
                    ( !m_para_sel_end || // when there is no selection, sel_end is null
                    p->get_para_no() < m_para_sel_bgn->get_para_no() ||
                    p->get_para_no() > m_para_sel_end->get_para_no() ) )
                {
                    y_hover_top = ( y_para_margin - 2 );
                    draw_hover_rect();
                }
                // multi-line case:
                else
                if( m_para_sel_end &&
                    m_para_menu->get_para_no() >= m_para_sel_bgn->get_para_no() &&
                    m_para_menu->get_para_no() <= m_para_sel_end->get_para_no() )
                {
                    if( p == m_para_sel_bgn ) y_hover_top = ( y_para_margin - 2 );
                    if( p == m_para_sel_end ) draw_hover_rect();
                }
            }

            // LOOP END OPERATIONS
            p_last = p;

            if( p == para_end ) break;

            para_bgn_offset += ( p->get_size() + 1 );
        }
        if( p_last && p_last->is_quote() ) // last para is a quotation case
            draw_quotation_end();

        // END OF ENTRY MARK
        if( is_the_main_editor() && it_visible_end.is_end() )
        {
            get_iter_location( it_visible_end, rect_iter );
            buffer_to_window_coords( Gtk::TextWindowType::TEXT,
                                     rect_iter.get_x(), rect_iter.get_y(),
                                     x_para, y_para );
            Gdk::Cairo::set_source_rgba( cr, m_p2theme->color_subheading );
            cr->set_line_width( 3.0 );

            cr->move_to( x_para, y_para + 5 );
            cr->rel_line_to( 20, 0 );

            cr->stroke();
        }
    }
}

bool
TextviewDiary::handle_query_tooltip( int x, int y, bool keyboard_mode,
                                     const Glib::RefPtr< Gtk::Tooltip >& tooltip )
{
    Ustring tooltip_text;

    switch( get_hovered_link_type() )
    {
        case VT::HFT_FOLD_COLLAPSED:
            tooltip->set_text( _( "Click to expand the paragraph" ) );
            return true;
        case VT::HFT_FOLD_EXPANDED:
            tooltip->set_text( _( "Click to collapse the paragraph" ) );
            return true;
        case VT::HFT_ENTRY_MENU:
            if( m_p2entry )
                tooltip->set_markup( m_p2entry->get_title_ancestral() + "\n" +
                                     m_p2entry->get_info_str() + "\n" +
                                     Glib::Markup::escape_text(
                                            _( "Click to edit entry properties" ) ) );
            return true;
        case VT::HFT_REFS_MENU:
            tooltip->set_text( _( "Click to see list of entries tagged by this entry" ) );
            return true;
        case VT::HFT_DATE:
            tooltip_text = Glib::Markup::escape_text(
                                Date::get_weekday_str( m_p2hflink_hovered->ref_id ) );
            break;
        case VT::HFT_LINK_URI:
            tooltip_text = Glib::Markup::escape_text( m_p2hflink_hovered->uri );
            break;
        case VT::HFT_LINK_EVAL:
            tooltip_text = _( "Entry text evaluated as link address" );
            break;
        case VT::HFT_TAG:
        case VT::HFT_LINK_ID:
        {
            Entry* entry{ Diary::d->get_entry_by_id( m_p2hflink_hovered->ref_id ) };
            if( entry )
            {
                tooltip_text = entry->get_title_ancestral() + "\n" +
                               Glib::Markup::escape_text( entry->get_description() );
            }
            break;
        }
        default:
            if( m_para_menu )
            {
                tooltip->set_text( _( "Right click to edit paragraph" ) );
                return true;
            }
            else
                return false;
    }

    // add ctrl hint:
    tooltip_text += STR::compose( "\n<span color=\"", Lifeograph::get_color_insensitive() + "\">" +
                                  Glib::Markup::escape_text(
                                        get_editable() ? _( "Press Ctrl to follow the link" )
                                                       : _( "Press Ctrl to select" ) ),
                                       "</span>" );

    tooltip->set_markup( tooltip_text );
    return true;
}

void
TextviewDiary::get_tag_coordinates( const int pos_bgn, const int pos_end )
{
    Gdk::Rectangle  rect;
    Gtk::TextIter   it { m_r2buffer->get_iter_at_offset( pos_bgn ) };
    int             x, y;
    auto            add_point = [ & ]( bool F_top_left )
    {
        get_iter_location( it, rect );
        if( F_top_left )
            buffer_to_window_coords( Gtk::TextWindowType::TEXT, rect.get_x(), rect.get_y(), x, y );
        else
            buffer_to_window_coords( Gtk::TextWindowType::TEXT,
                                     rect.get_x() + rect.get_width(),
                                     rect.get_y() + rect.get_height(),
                                     x, y );
        m_tag_coords.push_back( x );
        m_tag_coords.push_back( y - 1 ); // shifting 1 px gives better alignment
    };

    add_point( true );
    forward_display_line( it );

    while( it.get_offset() < pos_end )
    {
        --it;
        add_point( false );

        ++it;
        add_point( true );

        forward_display_line( it );
    }

    it = m_r2buffer->get_iter_at_offset( pos_end );
    add_point( false );
}

