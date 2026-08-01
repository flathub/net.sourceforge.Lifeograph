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


#include "src/strings.hpp"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _WIN32
#include <winsock2.h> // to silence warnings on Windows
#endif

#include "lifeograph.hpp"
#include "app_window.hpp"
#include "ui_entry.hpp"
#include "ui_extra.hpp"
#include "widgets/widget_map.hpp"


using namespace LIFEO;

// UIEXTRA =========================================================================================
UIExtra::UIExtra()
{
    // COMMON
    auto builder{ Lifeograph::get_builder() };
    m_Bx_panel  = builder->get_widget< Gtk::Box >( "Bx_panel_extra" );
    m_St_view   = builder->get_widget< Gtk::Stack >( "St_pextra_view" );

    // CALENDAR VIEW
    m_W_calendar    = Gtk::Builder::get_widget_derived< WidgetCalendar >( builder, "DA_calendar" );

    // SEARCH VIEW
    m_E_search              = builder->get_widget< Gtk::SearchEntry >( "E_search" );
    m_WFP_search_filter     = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    builder, "MB_search_filter" );
    m_TB_search_match_case  = builder->get_widget< Gtk::ToggleButton >( "TB_search_match_case" );
    m_Bx_replace            = builder->get_widget< Gtk::Box >( "Bx_replace" );
    m_E_replace             = Gtk::Builder::get_widget_derived< EntryClear >(
                                    builder, "E_replace" );
    m_B_replace_all         = builder->get_widget< Gtk::Button >( "B_replace_all" );
    m_TvD_search            = Gtk::Builder::get_widget_derived< TextviewDiarySearch >(
                                    builder, "TvD_search" );

    // CHART VIEW
    auto MB_chart_properties
                        = builder->get_widget< Gtk::MenuButton >( "MB_chart_properties" );
    auto Sc_chart_zoom  = builder->get_widget< Gtk::Scale >( "Sc_chart_zoom" );
    m_W_chart           = Gtk::Builder::get_widget_derived< WidgetChart >( builder, "DA_chart" );
    m_WP_chart          = Gtk::Builder::get_widget_derived< WidgetPicker< ChartElem > >(
                                builder, "E_chart_picker" );
    m_B_chart_new       = builder->get_widget< Gtk::Button >( "B_chart_new" );

    // TABLE VIEW
    auto MB_table_tools = builder->get_widget< Gtk::MenuButton >( "MB_table_properties" );
    m_W_table           = Gtk::Builder::get_widget_derived< WidgetTable >( builder, "DA_table" );
    m_WP_table          = Gtk::Builder::get_widget_derived< WidgetPicker< TableElem > >(
                                builder, "E_table_picker" );
    m_Bx_table_new       = builder->get_widget< Gtk::Box >( "Bx_table_new" );
    m_B_table_new       = builder->get_widget< Gtk::Button >( "B_table_new" );

    // THEME VIEW
    m_FBx_themes        = builder->get_widget< Gtk::FlowBox >( "FBx_themes" );
    m_St_themes         = builder->get_widget< Gtk::Stack >( "St_themes" );
    auto B_theme_close  = builder->get_widget< Gtk::Button >( "B_theme_close" );
    m_TVD_theme_edit    = Gtk::Builder::get_widget_derived< TextviewDiaryTheme >(
                                builder, "TVD_theme_preview" );

    // MAP VIEW
    m_Bx_map    = builder->get_widget< Gtk::Box >( "Bx_map" );
    m_W_map     = new WidgetMap;
    m_Bx_map->append( m_W_map->get_widget() );

    // COMMON
    m_St_view->property_visible_child().signal_changed().connect(
            [ this ](){ refresh_active_panel( true ); } );

    // CALENDAR VIEW
    m_W_calendar->signal_day_selected().connect(
            [ this ]( DateV d ){ handle_calendar_day_selected( d ); } );
    m_W_calendar->signal_day_add().connect(
            []( DateV d, const Ustring& text )
            {
                AppWindow::p->UI_diary->update_for_added_entry(
                        Diary::d->create_entry( nullptr, false, d, text,
                                                VT::ETS::DATE_AND_NAME::I ) );
            } );

    // SEARCH VIEW
    m_E_search->signal_search_changed().connect(       [ this ](){ handle_search_text(); } );
    m_E_search->signal_activate().connect(             [ this ](){ go_to_match_at( 1 ); } );
    m_TB_search_match_case->signal_toggled().connect(  [ this ]()
                                                       { handle_search_case_sensitivity(); } );
    m_WFP_search_filter->set_clearable( true );
    m_WFP_search_filter->set_show_edit_button( true );
    m_WFP_search_filter->set_show_name( false );
    m_WFP_search_filter->set_show_cur_entry( true );
    m_WFP_search_filter->set_obj_classes( FOC::ENTRIES );
    m_WFP_search_filter->Sg_changed.connect(           [ this ]( const Filter* f )
                                                       { handle_search_filter( f ); } );
    m_TvD_search->signal_replace().connect(
            [ this ]( const HiddenFormat* f ){ replace_match( f ); } );
    m_B_replace_all->signal_clicked().connect(         [ this ](){ replace_all_matches(); } );

    Diary::d->m_dispatcher_search.connect( [ this ](){ handle_search_thread_notification(); } );

    // CHART VIEW
    m_W_chart->set_helper_widgets( MB_chart_properties, Sc_chart_zoom );

    m_WP_chart->set_show_add_button( false );
    m_WP_chart->set_icon_drag_source(
            Gdk::ContentProvider::create( MIME_THEME, Glib::Bytes::create( "", 0 ) ) );
    m_B_chart_new->signal_clicked().connect(
            [ this ]()
            { add_new_chart( _( "New Chart" ), ChartElem::DEFINITION_DEFAULT, true ); } );
    m_WP_chart->signal_add().connect(
            [ this ](){ add_new_chart( _( "New Chart" ), ChartElem::DEFINITION_DEFAULT ); } );
    m_WP_chart->signal_duplicate().connect(
            [ this ]( const Ustring& name )
            { add_new_chart( name, Diary::d->get_chart( name )->get_definition() ); } );
    m_WP_chart->signal_dismiss().connect(
            [ this ]( const Ustring& name ){ dismiss_chart( name ); } );
    m_WP_chart->signal_sel_changed().connect(
            [ this ]( const Ustring& name ){ handle_chart_selected( name ); } );
    m_WP_chart->signal_name_edited().connect(
            [ this ]( const Ustring& name )->bool{ return handle_chart_renamed( name ); } );

    m_W_chart->signal_changed().connect( [ this ](){ handle_chart_edited(); } );

    // TABLE VIEW
    m_W_table->set_helper_widgets( MB_table_tools );

    m_WP_table->set_show_add_button( false );
    m_B_table_new->signal_clicked().connect(
            [ this ]()
            { add_new_table( STR0/SI::NEW_TABLE, TableElem::DEFINITION_DEFAULT, true ); } );
    m_WP_table->signal_add().connect(
            [ this ](){ add_new_table( STR0/SI::NEW_TABLE, TableElem::DEFINITION_DEFAULT ); } );
    m_WP_table->signal_duplicate().connect(
            [ this ]( const Ustring& name )
            { add_new_table( name, Diary::d->get_table( name )->get_definition() ); } );
    m_WP_table->signal_dismiss().connect(
            [ this ]( const Ustring& name ){ dismiss_table( name ); } );
    m_WP_table->signal_sel_changed().connect(
            [ this ]( const Ustring& name ){ handle_table_selected( name ); } );
    m_WP_table->signal_name_edited().connect(
            [ this ]( const Ustring& name )->bool{ return handle_table_renamed( name ); } );

    m_W_table->signal_changed().connect( [ this ](){ handle_table_edited(); } );
    m_W_table->signal_elem_clicked().connect(
            []( DiaryElement* elem ) { AppWindow::p->show_elem( elem ); } );

    // THEME VIEW
    m_FBx_themes->signal_child_activated().connect(
            [ this ]( Gtk::FlowBoxChild* child )
            {
                if( Diary::d->is_in_edit_mode() )
                    edit_theme();
            } );

    auto gesture_click_theme { Gtk::GestureClick::create() };
    gesture_click_theme->set_button( 3 );
    gesture_click_theme->signal_pressed().connect(
            sigc::mem_fun( *this, &UIExtra::handle_theme_rclick ) );
    m_FBx_themes->add_controller( gesture_click_theme );

    m_DrS_theme = Gtk::DragSource::create();
    m_DrS_theme->signal_drag_begin().connect(
            [ this ]( const Glib::RefPtr< Gdk::Drag >& drag )
            {
                m_DrS_theme->set_icon( lookup_default_icon( "applications-graphics-symbolic", 32 ),
                                       0, 0 );
            } );
    m_DrS_theme->signal_prepare().connect(
            [ this ]( double x, double y ) -> Glib::RefPtr< Gdk::ContentProvider >
            {
                Theme::GValue value_theme;
                auto          FBx_child   { m_FBx_themes->get_child_at_pos( x, y ) };
                Theme*        theme       { get_theme_from_FBxChild( FBx_child ) };
                value_theme.init( value_theme.value_type() );
                value_theme.set( theme );

                return Gdk::ContentProvider::create( value_theme );
            },
            false );
    m_FBx_themes->add_controller( m_DrS_theme );
    m_DrS_theme->set_propagation_phase( Gtk::PropagationPhase::NONE );

    B_theme_close->signal_clicked().connect(
            [ this ](){ m_St_themes->set_visible_child( "list" ); update_theme_list(); } );
    m_TVD_theme_edit->signal_theme_edited().connect(
            [ this ]()
            {
                AppWindow::p->update_textview_theme_class( get_selected_theme() );
                m_TVD_theme_edit->set_text_edit(); // update text
                // if( AppWindow::p->UI_entry->get_cur_entry()->get_theme() == get_selected_theme() )
                //     AppWindow::p->UI_entry->refresh_theme();
            } );

    // COMMON
    Lifeograph::p->add_action( "show_extra_panel",      [ this ](){ set_view( "" ); } );
    Lifeograph::p->add_action( "hide_extra_panel",      [ this ](){ set_view( "H" ); } );
    Lifeograph::p->add_action( "show_extra_p_calendar", [ this ](){ set_view( "calendar" ); } );
    Lifeograph::p->add_action( "show_extra_p_search",   [ this ](){ set_view( "search" ); } );
    Lifeograph::p->add_action( "show_extra_p_chart",    [ this ](){ set_view( "chart" ); } );
    Lifeograph::p->add_action( "show_extra_p_table",    [ this ](){ set_view( "table" ); } );
    Lifeograph::p->add_action( "show_extra_p_theme",    [ this ](){ set_view( "theme" ); } );
    Lifeograph::p->add_action( "show_extra_p_map",      [ this ](){ set_view( "map" ); } );
    // SEARCH VIEW
    Lifeograph::p->add_action( "begin_searching",       [ this ](){ focus_searching(); } );
    Lifeograph::p->add_action( "go_prev_match",         [ this ](){ go_to_match_at( -1 ); } );
    Lifeograph::p->add_action( "go_next_match",         [ this ](){ go_to_match_at( 1 ); } );
    Lifeograph::p->add_action( "replace_match",         [ this ](){ replace_match_cur(); } );
    // TABLE VIEW
    Lifeograph::p->add_action( "table_standard_report",
            [ this ]()
            { add_new_table( _( STRING::DIARY_REPORT ), TableElem::DEFINITION_REPORT, true ); } );
    // THEME VIEW
    Lifeograph::p->add_action( "theme_duplicate",       [ this ](){ duplicate_theme(); } );
    Lifeograph::p->add_action( "theme_reset",           [ this ](){ reset_theme_to_system(); } );
    Lifeograph::p->add_action( "theme_assign",          [ this ](){ assign_theme(); } );
    Lifeograph::p->add_action( "theme_edit",            [ this ](){ edit_theme(); } );

    // ACCELERATORS
    Lifeograph::p->set_accel_for_action( "app.hide_extra_panel",      "<Ctrl>0" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_calendar", "<Ctrl>1" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_search",   "<Ctrl>2" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_table",    "<Ctrl>3" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_chart",    "<Ctrl>4" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_theme",    "<Ctrl>5" );
    Lifeograph::p->set_accel_for_action( "app.show_extra_p_map",      "<Ctrl>6" );

    Lifeograph::p->set_accel_for_action( "app.begin_searching",       "<Ctrl>F" );
    Lifeograph::p->set_accel_for_action( "app.go_prev_match",         "<Shift>F3" );
    Lifeograph::p->set_accel_for_action( "app.go_next_match",         "F3" );
    Lifeograph::p->set_accel_for_action( "app.replace_match",         "F4" );
}
UIExtra::~UIExtra()
{
    m_TvD_search->disband();
}

void
UIExtra::handle_login()
{
    m_TvD_search->set_diary( Diary::d );
    m_TvD_search->set_text_from_matches( nullptr );

    m_WP_chart->set_select_only( true );
    m_WP_chart->set_map( Diary::d->get_p2charts(), Diary::d->get_p2chart_active() );
    m_W_chart->set_diary( Diary::d );

    m_WP_table->set_select_only( true );
    m_WP_table->set_map( Diary::d->get_p2tables(), Diary::d->get_p2table_active() );
    m_W_table->set_diary( Diary::d );

    update_theme_list();

    m_W_map->set_diary( Diary::d );

    Lifeograph::START_INTERNAL_OPERATIONS();

    m_WFP_search_filter->set_diary( Diary::d );
    m_WFP_search_filter->set_active( Diary::d->get_search_filter() );
    m_TB_search_match_case->set_active(
            Diary::d->get_boolean_option( VT::DO::SEARCH_MATCH_CASE::I ) );

    switch( Diary::d->get_opt_ext_panel_cur() )
    {
        default: m_Bx_panel->hide(); break;
        case 1: set_view( "calendar" ); break;
        case 2: set_view( "search" ); break;
        case 3: set_view( "chart" ); break;
        case 4: set_view( "table" ); break;
        case 5: set_view( "theme" ); break;
        case 6: set_view( "map" ); break;
    }

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
UIExtra::handle_edit_enabled()
{
    m_W_calendar->set_editable( true );

    m_Bx_replace->set_sensitive( true );

    m_WP_chart->set_select_only( false );
    m_W_chart->refresh_editabilitiy();
    m_B_chart_new->show();

    m_WP_table->set_select_only( false );
    m_W_table->refresh_editability();
    m_Bx_table_new->show();

    m_DrS_theme->set_propagation_phase( Gtk::PropagationPhase::BUBBLE );

    m_W_map->handle_editing_enabled();
}

void
UIExtra::handle_logout()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_W_calendar->set_editable( false );

    m_E_search->set_text( "" );
    m_E_replace->set_text( "" );
    m_Bx_replace->set_sensitive( false );

    set_active_chart( "" );
    set_active_table( "" );

    m_B_chart_new->hide();
    m_Bx_table_new->hide();

    m_TVD_theme_edit->set_theme( ThemeSystem::get() );
    m_St_themes->set_visible_child( "list" );
    remove_all_children_from_FBx( m_FBx_themes );
    m_DrS_theme->set_propagation_phase( Gtk::PropagationPhase::NONE );  // disable

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
UIExtra::set_view( const Ustring& view_name )
{
    if( Diary::d->is_open() )
    {
        if( view_name == "H" )
        {
            m_Bx_panel->hide();
            Diary::d->set_opt_ext_panel_cur( 0 );
            return;
        }

        AppWindow::p->fix_paned_positions_if_needed();

        if( view_name.empty() ) // show the last child
        {
            m_Bx_panel->show();
            set_diary_view_cur( m_St_view->get_visible_child_name() );
        }
        else
        {
            m_Bx_panel->show();
            m_St_view->set_visible_child( view_name );
        }
    }
}

void
UIExtra::set_entry( Entry* entry )
{
    m_W_calendar->set_day_shown( entry->get_date() );
    m_W_calendar->set_date( entry->get_date() );

    m_W_table->set_line_cur( entry );

    m_W_map->set_entry( entry );
}

void
UIExtra::refresh_active_panel( bool F_handle_change )
{
    const auto&& active_view{ m_St_view->get_visible_child_name() };

    if     ( active_view == "chart" )
        refresh_chart();
    else if( active_view == "table" )
        refresh_table();
    else if( active_view == "search" )
    {
        if( F_handle_change )
            m_E_search->grab_focus();
        start_search_thread();
    }
    else if( active_view == "map" )
        m_W_map->refresh();

    if( F_handle_change && !Lifeograph::is_internal_operations_ongoing() )
        set_diary_view_cur( active_view );
}

void
UIExtra::refresh_after_sync()
{
    refresh_calendar();
    refresh_chart();
    refresh_table();
    m_W_map->refresh();

    m_WP_chart->update_list();
    m_WP_table->update_list();
    update_theme_list();
}

void
UIExtra::remove_entry( Entry* entry )
{
    // NOTE: this is always called before a subsequent update_for_entry() call

    if( Lifeograph::is_internal_operations_ongoing() ) return;

    const auto&& active_view{ m_St_view->get_visible_child_name() };

    if( active_view == "chart" )
        m_W_chart->remove_entry( entry );
    else
    if( active_view == "table" )
        m_W_table->remove_entry( entry );
    // removal from other panels is not necessary
}
void
UIExtra::update_for_entry( Entry* entry )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    const auto&& active_view{ m_St_view->get_visible_child_name() };

    // TODO: 3.1: if( active_view == "chart" )
    //     m_W_chart->add_entry( entry );
    // else
    if( active_view == "table" )
        m_W_table->add_entry( entry );  // it was removed in before_parse handler
    else
    if( active_view == "search" )
        update_search_for_entry( entry );
    else
    if( active_view == "map" && entry->is_map_path_old() )
        m_W_map->refresh_path();  // CAUTION: always updates the current entry in the map
}

int
UIExtra::set_diary_view_cur( const std::string& name )
{
    int i = 0;
    for( const auto& n : { "calendar", "search", "chart", "table", "theme", "map" } )
    {
        if( name == n )
        {
            Diary::d->set_opt_ext_panel_cur( i + 1 );
            return( i + 1 );
        }
        i++;
    }
    print_error( "Unrecognized view name: ", name );
    return 0;
}

// CALENDAR VIEW
void
UIExtra::handle_calendar_day_selected( DateV date )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    auto&& entries{ Diary::d->get_entries_by_date( date, true ) };

    switch( entries.size() )
    {
        case 0:
            break;
        case 1:
            AppWindow::p->show_entry( entries.at( 0 ) );
            break;
        default:
        {
            SetDiaryElemsByName elems;
            for( auto& e : entries ) elems.insert( e );

            PoElemChooser::show( elems,
                                 []( DiaryElement* e )
                                 { AppWindow::p->show_entry( dynamic_cast< Entry* >( e ) ); },
                                 *m_W_calendar,
                                 m_W_calendar->get_selected_rectangle() );
            break;
        }
    }
}

void
UIExtra::refresh_calendar()
{
    m_W_calendar->refresh();
}

void
UIExtra::show_date_in_cal( DateV date )
{
    m_St_view->set_visible_child( "calendar" );
    m_W_calendar->set_selected_date( date );
    m_W_calendar->refresh();
}

// SEARCH VIEW
void
UIExtra::handle_search_text()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    Diary::d->set_search_str( m_TB_search_match_case->get_active() ?
                              m_E_search->get_text() :
                              STR::lowercase( m_E_search->get_text() ) );

    start_search_thread();
}
void
UIExtra::handle_search_case_sensitivity()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    const bool F_case_sensitive { m_TB_search_match_case->get_active() };

    Diary::d->set_boolean_option( VT::DO::SEARCH_MATCH_CASE::I, F_case_sensitive );
    Diary::d->set_search_str( F_case_sensitive ? m_E_search->get_text()
                                               : STR::lowercase( m_E_search->get_text() ) );

    start_search_thread();
}
void
UIExtra::handle_search_filter( const Filter* filter )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    Diary::d->set_search_filter( filter );

    start_search_thread();
}

void
UIExtra::start_search_thread()
{
    if( Diary::d->is_search_in_progress() )
    {
        PRINT_DEBUG( "Destroying active search thread" );
        m_F_restart_search_thread = true;
        Diary::d->stop_search();
    }
    else
    {
        PRINT_DEBUG( "<SEARCH THREADS STARTED>" );

        m_F_restart_search_thread = false;

        m_TvD_search->set_text_search_in_progress();

        m_index_match_cur = -1;

        Diary::d->start_search( AppWindow::parallel_thread_count );
    }
}

void
UIExtra::handle_search_thread_notification()
{
    PRINT_DEBUG( "<SEARCH THREAD COMPLETED>" );

    Diary::d->destroy_search_threads();

    if( m_F_restart_search_thread )
        start_search_thread();
    else
    {
        m_match_count = Diary::d->get_matches()->size();

        if( Diary::d->is_search_needed() )
            m_E_search->set_tooltip_markup( Ustring::compose( _( "<b>%1</b> instances found" ),
                                                                m_match_count ) );
        else
            m_E_search->set_tooltip_text( _( "Search text within entries" ) );

        m_TvD_search->set_text_from_matches( Diary::d->get_matches() );
        AppWindow::p->UI_entry->get_textview()->update_text_formatting();
    }
}

void
UIExtra::remove_entry_from_search( Entry* entry )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    Diary::d->remove_entry_from_search( entry );
    m_match_count = Diary::d->get_matches()->size();

    // the below 2 should be combined into 1
    m_TvD_search->set_text_from_matches( nullptr );
    m_TvD_search->set_text_from_matches( Diary::d->get_matches() );
}

void
UIExtra::update_search_for_entry( const Entry* entry )
{
    Diary::d->update_search_for_entry( entry );
    m_match_count = Diary::d->get_matches()->size();

    // the below 2 should be combined into 1
    m_TvD_search->set_text_from_matches( nullptr );
    m_TvD_search->set_text_from_matches( Diary::d->get_matches() );
}

void
UIExtra::focus_searching( const Ustring& str )
{
    set_view( "search" );

    if( !str.empty() )
        m_E_search->set_text( str );

    m_E_search->grab_focus();
    m_E_search->select_region( 0, -1 ); // normally this shoouldn't be needed
}

void
UIExtra::go_to_match_at( int offset )
{
    if( m_match_count < 1 || Diary::d->is_search_in_progress() ) return;

    if( m_index_match_cur < 0 )
        m_index_match_cur = 0;
    else
        m_index_match_cur = ( m_index_match_cur + m_match_count + offset ) % m_match_count;

    auto match  { Diary::d->get_match_at( m_index_match_cur ) };
    auto e_host { Diary::d->get_entry_by_id( match->ref_id ) };
    auto para   { e_host->get_paragraph_by_no( match->var_i ) };

    AppWindow::p->UI_entry->show( para, match );
    m_TvD_search->set_para_sel( match );
}

void
UIExtra::replace_match( const HiddenFormat* match )
{
    const Ustring&&   str_new   { m_E_replace->get_text() };
    Entry*            p2entry   { Diary::d->get_entry_by_id( match->ref_id ) };
    Paragraph*        p2para    { p2entry->get_paragraph_by_no( match->var_i ) };
    const auto        pos_para  { p2para->get_bgn_offset_in_host() };
    const auto        pos_end_0 { pos_para + match->pos_end };
    const auto        pos_end_1 { pos_para + match->pos_bgn + str_new.length() };

    p2entry->add_undo_action( UndoableType::MODIFY_TEXT, p2para, 1, pos_end_0, pos_end_1 );

    Diary::d->replace_match( match, str_new );
    if( AppWindow::p->UI_entry->is_cur_entry( p2entry ) )
        // update current entry (also updates the results):
        AppWindow::p->UI_entry->get_textview()->update_per_replaced_match( match, pos_end_1 );
    else
        // only update search results:
        update_search_for_entry( p2entry );
}

void
UIExtra::replace_match_cur()
{
    if( m_index_match_cur < 0 || m_match_count < 1 || !Diary::d->is_in_edit_mode() ) return;

    auto match_cur { Diary::d->get_match_at( m_index_match_cur ) };
    auto e_host    { Diary::d->get_entry_by_id( match_cur->ref_id ) };
    auto para      { e_host->get_paragraph_by_no( match_cur->var_i ) };

    AppWindow::p->UI_entry->show( para, match_cur );

    replace_match( match_cur );
}

void
UIExtra::replace_all_matches()
{
    Diary::d->replace_all_matches( m_E_replace->get_text() );

    start_search_thread();
    AppWindow::p->UI_entry->refresh();
}

// CHART VIEW
void
UIExtra::show_chart( const Ustring& name )
{
    set_active_chart( name );
    set_view( "chart" );
}

void
UIExtra::set_active_chart( const Ustring& name )
{
    ChartElem* chart = Diary::d->get_chart( name );
    m_W_chart->set_from_string( chart ? chart->get_definition() : "" );
    m_WP_chart->set_text( name );
}

void
UIExtra::add_new_chart( const Ustring& name, const Ustring& def, bool F_update_picker )
{
    ChartElem* chart{ Diary::d->create_chart( name, def ) };
    Diary::d->set_chart_active( chart->get_name() );
    m_W_chart->set_from_string( chart->get_definition() );

    if( F_update_picker )
        m_WP_table->update_active_item( true );
}

void
UIExtra::dismiss_chart( const Ustring& name )
{
    if( Diary::d->dismiss_chart( name ) )
        m_W_chart->set_from_string( Diary::d->get_chart_active()->get_definition() );
}

void
UIExtra::handle_chart_selected( const Ustring& name )
{
    if( Diary::d->set_chart_active( name ) == false )
        return;

    m_W_chart->set_from_string( Diary::d->get_chart_active()->get_definition() );
}

bool
UIExtra::handle_chart_renamed( const Ustring& new_name )
{
    return( Diary::d->rename_chart( Diary::d->get_chart_active(), new_name ) );
}

void
UIExtra::handle_chart_edited()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    ChartElem* chart{ Diary::d->get_chart( m_WP_chart->get_text() ) };

    if( !chart )
        return;

    chart->set_definition( m_W_chart->get_as_string() );
}

void
UIExtra::refresh_chart()
{
    //if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_W_chart->get_chart_data().refresh_table();
    m_W_chart->calculate_and_plot( false );
}

// TABLE VIEW
void
UIExtra::show_table( const Ustring& name )
{
    set_active_table( name );
    set_view( "table" );
}

void
UIExtra::set_active_table( const Ustring& name )
{
    m_W_table->set( Diary::d->get_table( name ), AppWindow::p->UI_entry->get_cur_entry() );
    m_WP_table->set_text( name );
}

void
UIExtra::add_new_table( const Ustring& name, const Ustring& def, bool F_update_picker )
{
    TableElem* table{ Diary::d->create_table( name, def ) };
    Diary::d->set_table_active( table->get_name() );
    m_W_table->set( table, AppWindow::p->UI_entry->get_cur_entry() );

    if( F_update_picker )
        m_WP_table->update_active_item( true );
}

void
UIExtra::dismiss_table( const Ustring& name )
{
    if( Diary::d->dismiss_table( name ) )
        m_W_table->set( Diary::d->get_table_active(), AppWindow::p->UI_entry->get_cur_entry() );
}

void
UIExtra::handle_table_selected( const Ustring& name )
{
    if( Diary::d->set_table_active( name ) == false )
        return;

    m_W_table->set( Diary::d->get_table_active(), AppWindow::p->UI_entry->get_cur_entry() );
}

bool
UIExtra::handle_table_renamed( const Ustring& new_name )
{
    return( Diary::d->rename_table( Diary::d->get_table_active(), new_name ) );
}

void
UIExtra::handle_table_edited()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    TableElem* table{ Diary::d->get_table( m_WP_table->get_text() ) };

    if( !table )
        return;

    table->set_definition( m_W_table->get_as_string() );
}

void
UIExtra::refresh_table()
{
    //if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_W_table->calculate_and_plot( false );
}

// THEME VIEW
Gtk::FlowBoxChild*
UIExtra::get_selected_FBx_theme_child()
{
    auto&& selected { m_FBx_themes->get_selected_children() };

    if( selected.empty() ) return nullptr;

    return( selected.front() );
}
Theme*
UIExtra::get_theme_from_FBxChild( Gtk::FlowBoxChild* child )
{
    if( !child ) return nullptr;

    m_p2TvD_theme_prvw =
            ( dynamic_cast< TextviewDiaryTheme* >(
                     dynamic_cast< Gtk::Button* >( child->get_child() )->get_child() ) );

    return const_cast< Theme* >( m_p2TvD_theme_prvw->get_theme() );
}
Theme*
UIExtra::get_selected_theme()
{
    auto child{ get_selected_FBx_theme_child() };

    if( !child ) return nullptr;

    return( get_theme_from_FBxChild( child ) );
}

void
UIExtra::update_theme_list()
{
    int index{ 0 };

    auto add_theme_item = [ this ]( Theme* theme, int index )
    {
        // Button only serves visually here.
        auto B_theme { Gtk::make_managed< Gtk::Button >() };
        auto TvD     { Gtk::make_managed< TextviewDiaryTheme >() };

        B_theme->set_sensitive( false );
        B_theme->add_css_class( theme->get_css_class_name() );

        TvD->set_theme( theme );
        TvD->set_text_preview();
        TvD->set_sensitive( false );

        B_theme->set_child( *TvD );

        m_FBx_themes->append( *B_theme );
    };

    remove_all_children_from_FBx( m_FBx_themes );

    for( auto& kv_theme : *Diary::d->get_p2themes() )
        add_theme_item( kv_theme.second, index++ );
}

void
UIExtra::duplicate_theme()
{
    Theme* theme_orig { get_selected_theme() };
    if( !theme_orig ) return;

    Theme* theme_new  { Diary::d->create_theme( theme_orig->get_name() ) };

    theme_orig->copy_to( theme_new );

    AppWindow::p->add_textview_theme_class( theme_new );

    update_theme_list();
}

void
UIExtra::dismiss_theme()
{
    m_p2theme2dismiss = get_selected_theme();

    if( !m_p2theme2dismiss ) return;

    AppWindow::p->confirm_dismiss_element(
            m_p2theme2dismiss->get_name(),
            [ & ]()
            {
                AppWindow::p->remove_textview_theme_class( m_p2theme2dismiss );
                Diary::d->dismiss_theme( m_p2theme2dismiss );
                update_theme_list();
                // in case the theme was current entry's theme:
                AppWindow::p->UI_entry->refresh_theme();
            } );
}

void
UIExtra::edit_theme()
{
    m_St_themes->set_visible_child( "editor" );

    m_TVD_theme_edit->set_theme( get_selected_theme() );
    m_TVD_theme_edit->set_text_edit();
}

void
UIExtra::assign_theme()
{
    if( Diary::d->is_in_edit_mode() == false )
        return;

    AppWindow::p->UI_entry->set_theme( get_selected_theme() );
}

void
UIExtra::assign_theme_to_sel()
{
    Theme* theme{ get_selected_theme() };

    if( !theme || !Diary::d->is_in_edit_mode() )
        return;

    auto& entries{ AppWindow::p->UI_diary->get_selected_entries() };

    for( auto& entry : entries )
    {
        entry->set_theme( theme );
        if( AppWindow::p->UI_entry->is_cur_entry( entry ) )
            AppWindow::p->UI_entry->refresh_theme();
    }
}

void
UIExtra::reset_theme_to_system()
{
    Theme* theme{ get_selected_theme() };

    if( !theme ) return;

    ThemeSystem::get()->copy_to( theme );

    AppWindow::p->update_textview_theme_class( theme );
    update_theme_list();
    AppWindow::p->UI_entry->refresh_theme();
}

void
UIExtra::handle_theme_rclick( int n_click, double x, double y )
{
    Gtk::FlowBoxChild* FBxCh{ m_FBx_themes->get_child_at_pos( x, y ) };

    m_FBx_themes->select_child( *FBxCh );

    const Theme* theme{ get_selected_theme() };

    if( !Diary::d->is_in_edit_mode() || !theme ) return;

    if( !m_Po_theme )
    {
        auto builder{ Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/po_theme.ui" ) };

        m_AG = Gio::SimpleActionGroup::create();

        m_Po_theme          = builder->get_widget< Gtk::Popover >( "Po_theme" );
        m_E_theme_name      = builder->get_widget< Gtk::Entry >( "E_theme_name" );

        m_Po_theme->set_parent( *m_FBx_themes );

        m_A_theme_duplicate     = m_AG->add_action( "duplicate", 
                                                    [ this ]() { duplicate_theme(); } );
        m_A_theme_reset         = m_AG->add_action( "reset", 
                                                    [ this ]() { reset_theme_to_system(); } );
        m_A_theme_dismiss       = m_AG->add_action( "dismiss", 
                                                    [ this ]() { dismiss_theme(); } );
        m_A_theme_assign_cur    = m_AG->add_action( "assign_to_entry", 
                                                    [ this ]() { assign_theme(); } );
        m_A_theme_assign_sel    = m_AG->add_action( "assign_to_entries", 
                                                    [ this ]() { assign_theme_to_sel(); } );
        m_A_theme_edit          = m_AG->add_action( "edit", [ this ]() { edit_theme(); } );

        m_E_theme_name->signal_changed().connect( [ this ](){ handle_theme_name_changed(); } );

        m_FBx_themes->insert_action_group( "theme", m_AG );
    }

    m_A_theme_dismiss->set_enabled( Diary::d->get_p2themes()->size() > 1 );
    m_A_theme_assign_sel->set_enabled( AppWindow::p->UI_diary->get_selected_count() > 1 );

    m_Po_theme->set_pointing_to( FBxCh->get_allocation() );
    m_Po_theme->show();

    m_E_theme_name->set_text( theme->get_name() );
    m_E_theme_name->grab_focus();
}

void
UIExtra::handle_theme_name_changed()
{
    Ustring new_name  { m_E_theme_name->get_text() };
    Theme* theme      { get_selected_theme() };

    if( theme && !new_name.empty() && new_name != theme->get_name() )
    {
        Diary::d->rename_theme( theme, new_name );
        m_p2TvD_theme_prvw->set_text_preview();
    }
}
