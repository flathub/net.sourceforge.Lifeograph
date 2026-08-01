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

#include "lifeograph.hpp"
#include "app_window.hpp"
#include "ui_entry.hpp"
#include "printing.hpp"


using namespace LIFEO;


UIEntry::UIEntry()
{
    try
    {
        auto builder{ Lifeograph::get_builder() };
        m_Bx_navigation = builder->get_widget< Gtk::Box >( "Bx_navigation" );
        m_St_entry      = builder->get_widget< Gtk::Stack >( "St_entry" );
        m_SW_entry      = builder->get_widget< Gtk::ScrolledWindow >( "SW_entry" );
        m_TvDE          = Gtk::Builder::get_widget_derived< TextviewDiaryEdit >( builder,
                                                                                 "TV_entry" );
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create the entry view" );
    }

    m_TvDE->set_editable( false );

    // SIGNALS
    m_CPo_entry.m_Sg_todo_changed.connect(
            [ this ]( ElemStatus es ) { set_entries_todo( EntrySelection( { m_p2entry } ), es ); } );

    m_CPo_entry.m_Sg_favorite_changed.connect(
            [ this ]( bool F_fav )
            { set_entries_favorite( EntrySelection( { m_p2entry } ), F_fav ); } );

    m_CPo_entry.m_Sg_spellcheck_changed.connect(
            [ this ]( const Ustring& lang )
            { set_entries_spellcheck( EntrySelection( { m_p2entry } ), lang ); } );

    m_CPo_entry.m_Sg_unit_changed.connect(
            [ this ]( const Ustring& unit )
            { set_entries_unit( EntrySelection( { m_p2entry } ), unit ); } );

    m_CPo_entry.m_Sg_title_type.connect(
            [ this ]( int i ) { set_entries_title_type( EntrySelection( { m_p2entry } ), i ); } );

    m_CPo_entry.m_Sg_comment_style.connect(
            [ this ]( int i ) { set_entries_comment_style( EntrySelection( { m_p2entry } ), i ); } );

    m_CPo_entry.m_Sg_color_changed.connect(
            [ this ]( const Color& c ) { set_entries_color( EntrySelection( { m_p2entry } ), c ); } );

    m_TvDE->signal_changed_before_parse().connect(
            [ this ]() { AppWindow::p->UI_extra->remove_entry( m_p2entry ); } );

    m_TvDE->signal_changed().connect(
            [ this ]()
            {
                AppWindow::p->UI_extra->update_for_entry( m_p2entry );
                m_F_bg_jobs_queued = true;
            } );
    m_dispatcher.connect( sigc::mem_fun( *this, &UIEntry::handle_bg_jobs_completed ) );

    // EDITOR ACTIONS
    m_A_go_back =    Lifeograph::p->add_action( "browse_back",    [ this ](){ go_back(); } );
    m_A_go_forward = Lifeograph::p->add_action( "browse_forward", [ this ](){ go_forward(); } );
    Lifeograph::p->add_action(                  "print",          [ this ](){ print(); } );

    Lifeograph::p->set_accel_for_action( "app.browse_back",         "<Alt>Left" );
    Lifeograph::p->set_accel_for_action( "app.browse_forward",      "<Alt>Right" );
    Lifeograph::p->set_accel_for_action( "app.print",               "<Ctrl>P" );
}
UIEntry::~UIEntry()
{
    m_TvDE->disband();
}

void
UIEntry::handle_login()
{
    m_Bx_navigation->show();
    m_A_go_back->set_enabled( false );
    m_A_go_forward->set_enabled( false );
    m_TvDE->update_image_width();
}

void
UIEntry::handle_edit_enabled()
{
    m_TvDE->set_editable( true );
    m_CPo_entry.handle_edit_enabled();

    m_connection_bg_jobs = Glib::signal_timeout().connect_seconds(
            [ this ]()
            {
                if( m_F_bg_jobs_queued )
                    start_bg_jobs();
                return true;
            },
            12 );
}

void
UIEntry::handle_logout()
{
    m_connection_bg_jobs.disconnect();
    m_parser_word_count.stop();
    destroy_bg_jobs_thread();
    m_F_bg_jobs_queued = false;

    m_Bx_navigation->hide();

    m_CPo_entry.handle_logout();

    clear_history();
    m_TvDE->unset_entry(); // also calls m_textview->set_editable( false );

    m_p2entry = nullptr;
}

void
UIEntry::show( Entry* entry )
{
    if( !entry )
        return;
    else if( is_cur_entry( entry ) )
    {
        refresh();
        return;
    }

    prepare_for_hiding(); // firstly, last element must be prepared for hiding

    m_p2entry = entry;

    add_entry_to_history( entry );

    // HEADER
    refresh_title();
    m_A_go_back->set_enabled( ( m_browsing_history.size() - m_bhist_offset ) > 1 );
    m_A_go_forward->set_enabled( m_bhist_offset > 0 );

    Lifeograph::START_INTERNAL_OPERATIONS();

    // BODY
    m_TvDE->set_entry( entry );
    // word count can only be calculated after entry is set:
    start_bg_jobs();

    m_St_entry->set_visible_child( "entry_transition" );
    m_St_entry->set_visible_child( "entry_editor" );

    m_CPo_entry.set( entry, *m_TvDE, Gdk::Rectangle( m_TvDE->PAGE_MARGIN - 45, 10.0,
                                                     32.0, 40.0 ) );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // set the entry as the one to be saved as the last entry in the diary:
    Diary::d->set_current_entry( entry );

    PRINT_DEBUG( "entry shown: ", entry->get_id() );

    m_TvDE->grab_focus();
}

void
UIEntry::refresh()
{
    prepare_for_hiding();

    // HEADER
    refresh_title();
    // refresh_icon();
    // refresh_overlay_icon();

    Diary::d->clear_chart_and_table_images(); // clear cached charts and tables

    Lifeograph::START_INTERNAL_OPERATIONS();

    // BODY
    m_TvDE->refresh_entry( true, true );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    PRINT_DEBUG( "entry refreshed" );
}

void
UIEntry::show( Paragraph* para, const HiddenFormat* format )
{
    if( !is_cur_entry( para->m_host ) )
        AppWindow::p->show_entry( para->m_host );

    if( !para->is_visible() )
    {
        para->make_accessible();
        m_TvDE->refresh_entry( false, false ); // scroll_to and reset_visibs are done separately
    }

    auto&& pos_bgn { para->get_bgn_offset_in_host() + ( format ? format->pos_bgn : 0 ) };
    auto&& pos_end { format ? ( pos_bgn - format->pos_bgn + format->pos_end )
                            : para->get_end_offset_in_host() };
    auto&& it_bgn  { m_TvDE->m_r2buffer->get_iter_at_offset( pos_bgn ) };
    auto&& it_end  { m_TvDE->m_r2buffer->get_iter_at_offset( pos_end ) };

    m_TvDE->m_r2buffer->select_range( it_bgn, it_end );
    m_TvDE->scroll_to_cursor_center();
    m_TvDE->grab_focus();
}

void
UIEntry::go_back()
{
    if( ( m_browsing_history.size() - m_bhist_offset ) > 1 )
    {
        auto i = m_browsing_history.begin();
        m_bhist_offset++;
        std::advance( i, m_bhist_offset );

        m_St_entry->set_transition_type( Gtk::StackTransitionType::SLIDE_RIGHT );

        m_F_index_locked = true;

        AppWindow::p->show_entry( *i );

        m_F_index_locked = false;

        m_St_entry->set_transition_type( Gtk::StackTransitionType::SLIDE_LEFT );

        m_A_go_back->set_enabled( ( m_browsing_history.size() - m_bhist_offset ) > 1 );
        m_A_go_forward->set_enabled( true );
    }
}

void
UIEntry::go_forward()
{
    if( m_bhist_offset > 0 )
    {
        auto i = m_browsing_history.begin();
        m_bhist_offset--;
        std::advance( i, m_bhist_offset );

        m_F_index_locked = true;

        AppWindow::p->show_entry( *i );

        m_F_index_locked = false;
    }
}

void
UIEntry::add_entry_to_history( Entry* entry )
{
    if( ! m_F_index_locked ) // locked while going back
    {
        if( m_bhist_offset > 0 )
        {
            auto&& i_end{ m_browsing_history.begin() };

            std::advance( i_end, m_bhist_offset );
            m_browsing_history.erase( m_browsing_history.begin(), i_end );
        }
        m_bhist_offset = 0;
        m_browsing_history.push_front( entry );
    }
}

void
UIEntry::remove_entry_from_history( Entry* entry )
{
    auto remove_element = [ & ]()
    {
        for( auto i = m_browsing_history.begin(); i != m_browsing_history.end(); ++i )
            if( *i == entry )
            {
                m_browsing_history.erase( i );
                return true;
            }

        return false;
    };

    while( remove_element() ) continue;
}

void
UIEntry::clear_history()
{
    m_browsing_history.clear();
    m_bhist_offset = 0;
}

void
UIEntry::refresh_title()
{
    if( m_p2entry )
        AppWindow::p->set_title( m_p2entry->get_name() );
}

void
UIEntry::refresh_extra_info()
{
    Ustring text { STR::compose( Date::format_string_adv( m_p2entry->get_date(), "F, d  |  " ),
                                 m_parser_word_count.get_word_count(), " ", _( "word(s)" ),
                                 "  |  " ) };
    const auto workload { m_p2entry->get_workload() };

    if( workload )
        text += STR::compose(
                STR::format_percentage( m_p2entry->get_completion() ), " (",
                                        m_p2entry->get_completed(), "/", workload, ")  |  " );

    text += Diary::d->get_name();
    AppWindow::p->set_subtitle( text );
}

void
UIEntry::set_theme( const Theme* theme )
{
    m_p2entry->set_theme( theme );
    refresh_theme();
}

void
UIEntry::refresh_theme()
{
    m_TvDE->set_theme( nullptr ); // nullptr means just refresh
    m_TvDE->update_highlight_button();
}

void
UIEntry::print()
{
    if( not( Diary::d->is_open() ) )
        return;

    hide_popover();

    Glib::RefPtr< PrintOpr > print = PrintOpr::create();

    try
    {
        AppWindow::p->freeze_auto_close();
        print->set_hide_comments( m_p2entry->get_comment_style() == VT::CS::HIDDEN::I );
        print->run( Gtk::PrintOperation::Action::PRINT_DIALOG, *AppWindow::p );
        AppWindow::p->unfreeze_auto_close();
    }
    catch( const Gtk::PrintError &ex )
    {
        // See documentation for exact Gtk::PrintError error codes:
        print_error( "An error occurred while trying to run a print operation:", ex.what() );
    }
}

// void
// UIEntry::set_entry_favorite( Entry* e, bool F_fav )
// {
//     if( Lifeograph::is_internal_operations_ongoing() ) return;
//
// }
void
UIEntry::set_entries_favorite( const EntrySelection& es, bool F_fav )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    for( Entry* e : es )
    {
        if( F_fav != e->is_favorite() )
            e->toggle_favored();

        if( e == m_p2entry )
            m_TvDE->refresh_entry_icons();

        AppWindow::p->UI_diary->refresh_row( e );
    }
}

void
UIEntry::set_entries_todo( const EntrySelection& es, ElemStatus status )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    for( Entry* e : es )
    {
        if( status == ES::NOT_TODO )
            e->update_todo_status();
        else
            e->set_todo_status( status );

        if( e == m_p2entry )
            m_TvDE->refresh_entry_icons();

        AppWindow::p->UI_diary->refresh_row( e );
    }
}

void
UIEntry::set_entries_color( const EntrySelection& es, const Color& color )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    for( Entry* e : es )
    {
        e->set_color( color );

        AppWindow::p->UI_diary->refresh_row( e );
    }
    AppWindow::p->UI_extra->refresh_chart();
    AppWindow::p->UI_extra->refresh_calendar();
    AppWindow::p->UI_diary->refresh_row( m_p2entry );
}

// SPELL CHECKING
void
UIEntry::set_entries_spellcheck( const EntrySelection& es, const Ustring& lang )
{
    if( Lifeograph::is_internal_operations_ongoing() || !m_p2entry ) return;

    for( Entry* e : es )
    {
        e->set_lang( lang );

        if( e->get_id() == m_p2entry->get_id() )
            m_TvDE->update_for_spell_check_change();
    }
}

void
UIEntry::set_entries_unit( const EntrySelection& es, const Ustring& unit )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    for( Entry* e : es )
    {
        e->set_unit( unit );

        if( e->get_id() == m_p2entry->get_id() )
        {
            AppWindow::p->UI_extra->refresh_chart();
            AppWindow::p->UI_diary->refresh_row( m_p2entry );
        }
    }
}

void
UIEntry::set_entries_title_type( const EntrySelection& es, int type )
{
    // TODO: 3.1: consider moving to UIDiary
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    //m_Po_entry->hide();

    for( Entry* e : es )
    {
        e->set_title_style( type );
        AppWindow::p->UI_diary->refresh_row( e );
    }
}

void
UIEntry::set_entries_comment_style( const EntrySelection& es, int i_style )
{
    // TODO: 3.1: consider moving to UIDiary
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    for( Entry* e : es )
    {
        e->set_comment_style( VT::get_v< VT::CS, int, unsigned >( i_style ) );

        if( e->get_id() == m_p2entry->get_id() )
        {
            m_TvDE->set_theme( nullptr ); // nullptr means refresh
        }
    }
}

void
UIEntry::trash_entries( const EntrySelection& es, bool F_trash )
{
    // TODO: 3.1: consider moving to UIDiary
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    bool F_update_cal{ false };

    for( Entry* e : es )
    {
        e->set_trashed( F_trash );

        if( Diary::d->update_entry_filter_status( e ) && !F_update_cal )
            F_update_cal = true;

        AppWindow::p->UI_diary->refresh_row( e );
    }

    if( F_update_cal )
        AppWindow::p->UI_extra->refresh_calendar();
}

void
UIEntry::dismiss_entries( const EntrySelection& es )
{
    // TODO: 3.1: consider moving to UIDiary
    m_p2entries2dismiss = &es;
    m_F_dismiss_children = false;

    if( es.empty() && Diary::d->get_entries().size() < 2 ) // do not delete the last entry
        return;

    // only show when a single entry is selected; otherwise things get too much complicated:
    if( es.size() == 1 && ( * es.begin() )->has_children() )
    {
        m_ChB_dismiss_children =
                Gtk::make_managed< Gtk::CheckButton >( _( "Also dismiss child entries" ) );
        m_ChB_dismiss_children->signal_toggled().connect(
                [ & ](){ m_F_dismiss_children = m_ChB_dismiss_children->get_active(); } );
    }
    else
    {
        m_ChB_dismiss_children = nullptr;
    }

    AppWindow::p->confirm_dismiss_element( ( * es.begin() )->get_name(),
                                           std::bind( &UIEntry::dismiss_entries2, this ),
                                           m_ChB_dismiss_children );
}
void
UIEntry::dismiss_entries2()
{
    // TODO: 3.1: consider moving to UIDiary
    Entry* entry_replacing{ nullptr };

    for( Entry* e : *m_p2entries2dismiss )
    {
        if( m_F_dismiss_children )
            for( Entry* c = e->get_child_1st(); c; )
            {
                remove_entry_from_history( c );
                AppWindow::p->UI_extra->remove_entry_from_search( c );
                entry_replacing = c->get_next_straight( e );
                Diary::d->dismiss_entry( c );
                c = entry_replacing;
            }

        remove_entry_from_history( e );
        AppWindow::p->UI_extra->remove_entry_from_search( e );
        entry_replacing = Diary::d->dismiss_entry( e );

        if( e == m_p2entry )
            m_p2entry = nullptr;
    }

    AppWindow::p->UI_extra->refresh_calendar();
    AppWindow::p->UI_extra->refresh_table();
    AppWindow::p->UI_diary->update_entry_list();
    AppWindow::p->show_entry( entry_replacing );
}

void
UIEntry::prepare_for_hiding()
{
    if( m_p2entry )
    {
        PRINT_DEBUG( "prepare_for_hiding(): ENTRY VIEW SCROLL POS=",
                     m_TvDE->get_vadjustment()->get_value() );

        m_parser_word_count.stop();
        destroy_bg_jobs_thread();

        m_p2entry->set_cursor_pos( m_TvDE->m_pos_cursor );
        // m_p2entry->set_scroll_pos( m_TvDE->get_vadjustment()->get_value() );

        if( m_F_title_edited )
        {
            if( !m_p2entry->get_name_pure().empty() )
                Diary::d->update_entry_name_refs( m_p2entry );
            m_F_title_edited = false;
        }
    }
}

// INDEXING
void
UIEntry::start_bg_jobs()
{
    if( m_thread_bg_jobs )
    {
        print_info( "Background jobs are taking longer than expected!" );
    }
    else if( m_p2entry )
    {
        PRINT_DEBUG( "<BACKGROUND JOBS STARTED>" );
        m_F_bg_jobs_queued = false;
        m_parser_word_count.set_text( m_p2entry->get_text() );
        m_thread_bg_jobs = new std::thread{
                [ this ](){ m_parser_word_count.parse( m_dispatcher ); } };
    }
}

void
UIEntry::handle_bg_jobs_completed()
{
    PRINT_DEBUG( "<BACKGROUND JOBS COMPLETED>" );

    destroy_bg_jobs_thread();

    if( Diary::d->is_open() ) //if not logged out in the meantime
    {
        refresh_extra_info();
    }
}

void
UIEntry::destroy_bg_jobs_thread()
{
    if( !m_thread_bg_jobs )
        return;

    if( m_thread_bg_jobs->joinable() )
        m_thread_bg_jobs->join();
    delete m_thread_bg_jobs;
    m_thread_bg_jobs = nullptr;
}
