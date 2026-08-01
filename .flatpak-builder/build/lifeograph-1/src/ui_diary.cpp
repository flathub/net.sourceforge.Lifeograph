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
#include "ui_diary.hpp"
#include "ui_entry.hpp"
#include "widgets/widget_textview.hpp"
#include "dialogs/dialog_smartadd.hpp"
#include "dialogs/dialog_password.hpp"


using namespace LIFEO;


UIDiary::UIDiary()
{
    Gtk::PopoverMenu* Po_main;

    try
    {
        auto builder{ Lifeograph::get_builder() };

        m_MB_diary_close    = builder->get_widget< Gtk::MenuButton >( "MB_diary_close" );

        m_B_enable_edit     = builder->get_widget< Gtk::Button >( "B_enable_edit" );
        m_B_smart_add       = builder->get_widget< Gtk::Button >( "B_smart_add" );

        m_Bx_diary          = builder->get_widget< Gtk::Box >( "Bx_diary" );
        m_B_close           = builder->get_widget< Gtk::Button >( "B_close" );

        m_MB_diary          = builder->get_widget< Gtk::MenuButton >( "MB_diary" );
        m_MB_main           = builder->get_widget< Gtk::MenuButton >( "MB_tools" );

        m_B_search          = builder->get_widget< Gtk::Button >( "B_search" );

        Po_main             = builder->get_widget< Gtk::PopoverMenu >( "Po_main_menu" );

        m_CB_spellcheck     = builder->get_widget< Gtk::ComboBoxText >( "CB_diary_spellcheck" );

        m_WEP_completn_tag  = Gtk::Builder::get_widget_derived< WidgetEntryPicker >(
                                    builder, "E_diary_completion_tag" );

        m_Bx_extra_options  = builder->get_widget< Gtk::Box >( "Bx_diary_extra_options" );
        m_CB_startup_type   = builder->get_widget< Gtk::ComboBoxText >( "CB_diary_startup_type" );
        m_Bx_startup_elem   = builder->get_widget< Gtk::Box >( "Bx_diary_startup_entry" );
        m_W_startup_entry   = Gtk::Builder::get_widget_derived< WidgetEntryPicker >(
                                    builder, "E_diary_startup_entry" );

        m_WEL_main          = Gtk::Builder::get_widget_derived< WidgetEntryList >(
                                    builder, "LV_entries" );
        // m_Ol_list           = builder->get_widget< Gtk::Overlay >( "Ol_list" );
        m_Rv_list_filter    = builder->get_widget< Gtk::Revealer >( "Rv_list_filter" );
        m_WFP_list_filter   = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    builder, "MB_list_filter" );
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create the diary panel" );
    }

    m_WFP_list_filter->set_clearable( true );
    m_WFP_list_filter->set_show_edit_button( true );
    m_WFP_list_filter->set_show_name( false );
    m_WFP_list_filter->set_hide_popover_during_edit( true );
    m_WFP_list_filter->set_obj_classes( FOC::ENTRIES );
    m_WFP_list_filter->set_diary( Diary::d );

    // ENTRY LIST SIGNALS
    auto controller_motion = Gtk::EventControllerMotion::create();
    controller_motion->signal_motion().connect(
            [ this ]( double x, double y )
            {
                auto&& alloc{ m_WEL_main->get_allocation() };
                m_Rv_list_filter->set_reveal_child( y < ( alloc.get_height() * 0.3 ) &&
                                                    x > ( alloc.get_width() * 0.6 ) );
            },
            false );
    m_WEL_main->add_controller( controller_motion );

    m_WFP_list_filter->Sg_changed.connect(
            [ this ]( const Filter* filter )
            {
                Diary::d->set_filter_list( filter );
                update_all_entries_filter_status();
                update_entry_list();
            } );

    // HEADERBAR SIGNALS
    // at some point we may go back to using the smart add dialog with the add button:
    m_B_smart_add->signal_clicked().connect(      [ this ](){ add_entry_free(); } );

    Po_main->signal_show().connect(               [ this ](){ refresh_main_menu_2(); } );

    m_CB_spellcheck->signal_changed().connect(    [ this ](){ handle_language_changed(); } );

    m_WEP_completn_tag->signal_updated().connect(
            []( Entry* e )
            {
                Diary::d->set_completion_tag( e ? e->get_id() : DEID_UNSET );
            } );

    m_CB_startup_type->signal_changed().connect(  [ this ](){ handle_startup_type_changed(); } );

    m_W_startup_entry->signal_updated().connect(
            []( Entry* e ){ Diary::d->set_startup_entry( e ); } );

    // ENTRY POPOVER SIGNALS
    m_CPo_entry.m_Sg_todo_changed.connect(
            [ this ]( ElemStatus es )
            { AppWindow::p->UI_entry->set_entries_todo( get_selected_entries(), es ); } );

    m_CPo_entry.m_Sg_favorite_changed.connect(
            [ this ]( bool F_fav )
            { AppWindow::p->UI_entry->set_entries_favorite( get_selected_entries(), F_fav ); } );

    m_CPo_entry.m_Sg_spellcheck_changed.connect(
            [ this ]( const Ustring& lang )
            { AppWindow::p->UI_entry->set_entries_spellcheck( get_selected_entries(), lang ); } );

    m_CPo_entry.m_Sg_unit_changed.connect(
            [ this ]( const Ustring& unit )
            { AppWindow::p->UI_entry->set_entries_unit( get_selected_entries(), unit ); } );

    m_CPo_entry.m_Sg_title_type.connect(
            [ this ]( int i )
            { AppWindow::p->UI_entry->set_entries_title_type( get_selected_entries(), i ); } );

    m_CPo_entry.m_Sg_comment_style.connect(
            [ this ]( int i )
            { AppWindow::p->UI_entry->set_entries_comment_style( get_selected_entries(), i ); } );

    m_CPo_entry.m_Sg_color_changed.connect(
            [ this ]( const Color& c )
            { AppWindow::p->UI_entry->set_entries_color( get_selected_entries(), c ); } );

    // ACTIONS
    m_A_enable_editing = Lifeograph::p->add_action(
            "enable_editing",
            [ this ](){ enable_editing(); } );

    m_A_open_diary_folder = Lifeograph::p->add_action(
            "main_open_diary_folder",
            [ this ](){ open_diary_folder(); } );
    m_A_export_diary = Lifeograph::p->add_action(
            "main_export_diary",
            [ this ](){ start_dialog_export(); } );
    m_A_sync_diary = Lifeograph::p->add_action(
            "main_sync_diary",
            [ this ](){ start_dialog_sync(); } );
    m_A_encrypt_diary = Lifeograph::p->add_action(
            "main_encrypt_diary",
            [ this ](){ start_dialog_password(); } );
    m_A_change_password = Lifeograph::p->add_action(
            "main_change_password",
            [ this ](){ start_dialog_password(); } );
    m_A_empty_trash = Lifeograph::p->add_action(
            "main_empty_trash",
            [ this ](){ empty_trash(); } );
    m_A_update_date_seps = Lifeograph::p->add_action(
            "update_date_separators",
            [ this ](){ update_date_separators(); } );

    m_A_add_entry_dated = Lifeograph::p->add_action(
            "add_entry_dated",
            [ this ]() { add_entry_dated( false ); } );

    m_A_add_milestone = Lifeograph::p->add_action(
            "add_milestone",
            [ this ]() { add_entry_dated( true ); } );

    m_A_add_entry_todo = Lifeograph::p->add_action(
            "add_entry_todo",
            [ this ]()
            {
                if( Diary::d->is_in_edit_mode() )
                {
                    Entry* entry = Diary::d->create_entry( nullptr, false, Date::get_today(), "",
                                                           VT::ETS::NAME_ONLY::I );
                    entry->set_todo_status( ES::TODO );
                    update_for_added_entry( entry );
                }
            } );

    m_A_add_entry_free = Lifeograph::p->add_action(
            "add_entry_free",
            [ this ]() { add_entry_free(); } );

    m_A_go_today = Lifeograph::p->add_action(
            "go_to_today",
            [ this ](){ show_today(); } );
    m_A_smart_add = Lifeograph::p->add_action(
            "smart_add",
            [](){ DialogSmartAdd::create(); } );
    m_A_go_list_prev = Lifeograph::p->add_action(
            "go_list_prev",
            [](){ AppWindow::p->go_to_prev(); } );
    m_A_go_list_next = Lifeograph::p->add_action(
            "go_list_next",
            [](){ AppWindow::p->go_to_next(); } );
    m_A_jump_to_entry = Lifeograph::p->add_action(
            "jump_to_current_entry",
            [ this ](){ show_in_list( AppWindow::p->UI_entry->get_cur_entry(), true ); } );
    m_A_go_prev_sess_entry = Lifeograph::p->add_action(
            "go_to_prev_sess_entry",
            [ this ](){ show_prev_session_elem(); } );

    Lifeograph::p->add_action( "entry_duplicate",
            [ this ]()
            {
                if( Diary::d->is_in_edit_mode() )
                {
                    auto p2entry { AppWindow::p->UI_entry->get_cur_entry() };
                    update_for_added_entry( Diary::d->duplicate_entry( p2entry ) );
                }
            } );
    // duplicate action is in "app" group to allow shortcut assignment

    // ENTRY ACTIONS
    m_AG_entry = Gio::SimpleActionGroup::create();

    m_AG_entry->add_action( "trash",
            [ this ]()
            { AppWindow::p->UI_entry->trash_entries( get_selected_entries(), true ); } )
                    ->set_enabled( false );
    m_AG_entry->add_action( "restore",
            [ this ]()
            { AppWindow::p->UI_entry->trash_entries( get_selected_entries(), false ); } )
                    ->set_enabled( false );
    m_AG_entry->add_action( "dismiss",
            [ this ]()
            { AppWindow::p->UI_entry->dismiss_entries( get_selected_entries() ); } )
                    ->set_enabled( false );

    m_AG_entry->add_action( "add" )
                    ->set_enabled( false );
    m_AG_entry->add_action( "add_parent",   [ this ](){ add_parent_to_entries(); } );
    m_AG_entry->add_action( "add_child",
            [ this ]()
            {
                m_CPo_entry.m_p2entry->set_expanded( true );
                update_for_added_entry( Diary::d->create_entry( m_CPo_entry.m_p2entry,
                                                                true,
                                                                Date::get_today(),
                                                                "",
                                                                VT::ETS::INHERIT::I ) );
            } );
    m_AG_entry->add_action( "add_sibling",
            [ this ]()
            {
                update_for_added_entry( Diary::d->create_entry( m_CPo_entry.m_p2entry,
                                                                false,
                                                                Date::get_today(),
                                                                "",
                                                                VT::ETS::INHERIT::I ) );
            } );

    m_AG_entry->add_action( "merge", [ this ](){ merge_entries(); } )
                    ->set_enabled( false );

    m_AG_entry->add_action( "sort_siblings" )
                    ->set_enabled( false );
    m_AG_entry->add_action( "sort_sibl_az",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_names, 1 );
                update_entry_list();
            } );
    m_AG_entry->add_action( "sort_sibl_za",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_names, -1 );
                update_entry_list();
            } );
    m_AG_entry->add_action( "sort_sibl_earliest",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_dates, 1 );
                update_entry_list();
            } );
    m_AG_entry->add_action( "sort_sibl_latest",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_dates, -1 );
                update_entry_list();
            } );
    m_AG_entry->add_action( "sort_sibl_shortest",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_sizes, 1 );
                update_entry_list();
            } );
    m_AG_entry->add_action( "sort_sibl_longest",
            [ this ]()
            {
                Diary::d->sort_entry_siblings( m_CPo_entry.m_p2entry, &Entry::compare_sizes, -1 );
                update_entry_list();
            } );

    AppWindow::p->insert_action_group( "entry", m_AG_entry );

    Lifeograph::p->set_accel_for_action( "app.enable_editing",              "F2" );
    Lifeograph::p->set_accel_for_action( "app.go_to_today",                 "<Ctrl>T" );
    Lifeograph::p->set_accel_for_action( "app.smart_add",                   "F12" );
    Lifeograph::p->set_accel_for_action( "app.add_entry_todo",              "<Ctrl>D" );
    Lifeograph::p->set_accel_for_action( "app.go_list_prev",                "<Ctrl>Page_Up" );
    Lifeograph::p->set_accel_for_action( "app.go_list_next",                "<Ctrl>Page_Down" );
    Lifeograph::p->set_accel_for_action( "app.jump_to_current_entry",       "<Ctrl>J" );
    Lifeograph::p->set_accel_for_action( "app.go_to_prev_sess_entry",       "<Ctrl>O" );
    Lifeograph::p->set_accel_for_action( "app.entry_duplicate",             "<Ctrl><Shift>R" );
}
UIDiary::~UIDiary()
{
    DialogSmartAdd::disband();
}

// Gtk::TreeModel::iterator
// UIDiary::add_entry_to_list( Entry* entry,
//                             std::map< DEID, Gtk::TreeRow >* map_rows,
//                             bool F_add_back_parent_mode )
// {
//     Gtk::TreeModel::iterator iter_elem;
//
//     if( !F_add_back_parent_mode )
//     {
//         entry->set_list_order( ++m_list_order_last );
//         entry->set_sibling_order( entry->get_prev() ?
//                                   entry->get_prev()->get_sibling_order() + 1 : 1 );
//     }
//
//     if( !F_add_back_parent_mode && entry->is_filtered_out() )
//     {
//     }
//     else
//     {
//         auto parent_entry{ entry->get_parent() };
//
//         if( !parent_entry ) // 1st level
//             iter_elem = m_TV_entries->m_treestore->append();
//         else
//         {
//             auto&& result{ map_rows->find( parent_entry->get_id() ) };
//             auto   iter_parent =
//                     ( result == map_rows->end() ) ? // parent is not in the list
//                             add_entry_to_list( parent_entry, map_rows, true ) : // recursion
//                             result->second.get_iter();
//             iter_elem = m_TV_entries->m_treestore->append( iter_parent->children() );
//         }
//
//         Gtk::TreeRow row{ *iter_elem };
//         row[ ListData::colrec->ptr ] = entry; // ptr must be first
//         row[ ListData::colrec->info ] = entry->get_list_str();
//         row[ ListData::colrec->icon ] = entry->get_icon();
//         map_rows->emplace( entry->get_id(), row );
//     }
//
//     if( !F_add_back_parent_mode && entry->get_child_1st() )
//         add_entry_siblings_to_list( entry->get_child_1st(), map_rows ); // indirect recursion
//
//     return iter_elem;
// }
//
// void
// UIDiary::add_entry_siblings_to_list( Entry* entry, std::map< DEID, Gtk::TreeRow >* map_rows )
// {
//     for( Entry* e = entry; e != nullptr; e = e->get_next() )
//         add_entry_to_list( e, map_rows, false );
// }

void
UIDiary::update_entry_list( const Entry* entry2show )
{
    PRINT_DEBUG( "update_entry_list()" );
    Lifeograph::START_INTERNAL_OPERATIONS();

    // store the current scroll position
    // Gtk::TreePath path_bgn, path_end;
    // m_TV_entries->get_visible_range( path_bgn, path_end );
    // TODO: 4.0: an equal of this is not available in Gtk4 now.
    // see: https://gitlab.gnome.org/GNOME/gtk/-/issues/4688

    // entries:
    m_WEL_main->populate();

    // restore scroll position
    // if( path_bgn.empty() == false )
    //     m_TV_entries->scroll_to_row( path_bgn, 0.0 );

    // update current elem if last one is not in the list anymore:
    Entry* entry { AppWindow::p->UI_entry->get_cur_entry() };
    if( entry )
        m_WEL_main->select( entry );

    if ( entry2show )
        m_WEL_main->scroll_to_entry( entry2show );
    else if( entry )
        m_WEL_main->scroll_to_entry( entry );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
UIDiary::update_all_entries_filter_status()
{
    m_WFP_list_filter->set_no_of_filtered_in_items( Diary::d->update_all_entries_filter_status() );
}

void
UIDiary::handle_login()
{
    m_W_startup_entry->set_diary( Diary::d );

    Lifeograph::START_INTERNAL_OPERATIONS();

    update_entry_list(); // this used to be done in UI_extra when there was a filter panel
    m_WFP_list_filter->set_active( Diary::d->m_p2filter_list );

    m_B_search->set_visible( true );
    m_Bx_diary->set_visible( true );
    m_B_close->set_visible( !Lifeograph::settings.rtflag_single_file );

    m_B_enable_edit->set_visible( !Lifeograph::settings.rtflag_read_only );
    m_B_enable_edit->set_sensitive( Diary::d->can_enter_edit_mode() );
    m_MB_diary_close->set_visible( Diary::d->is_encrypted() );
    m_MB_diary->set_visible( true );

    m_MB_diary->set_tooltip_text( Gio::File::create_for_uri( Diary::d->get_uri() )
                                        ->get_parse_name() );

    refresh_main_menu( Diary::LOGGED_IN_RO );

    // SPELL CHECKING
    m_CB_spellcheck->remove_all();
    m_CB_spellcheck->append( _( STRING::OFF ) );
    for( const std::string& lang : Lifeograph::s_lang_list )
    {
        m_CB_spellcheck->append( lang );
    }

    if( Diary::d->m_language.empty() )
        m_CB_spellcheck->set_active_text( _( STRING::OFF ) );
    else
    {
        if( Lifeograph::s_lang_list.find( Diary::d->m_language ) == Lifeograph::s_lang_list.end() )
            m_CB_spellcheck->append( Diary::d->m_language );

        m_CB_spellcheck->set_active_text( Diary::d->m_language );
    }

    m_WEP_completn_tag->set_diary( Diary::d );
    m_WEP_completn_tag->set_entry( Diary::d->get_completion_tag() );

    if( Diary::d->m_startup_entry_id <= HOME_LAST_ENTRY )
        m_CB_startup_type->set_active( Diary::d->m_startup_entry_id - 1 );
    else
        m_CB_startup_type->set_active( 2 );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
UIDiary::enable_editing()
{
    if( !Diary::d->is_open() || Diary::d->is_in_edit_mode() )
        return;

    if( Diary::d->is_old() )
    {
        DialogMessage::init( AppWindow::p, _( "Are You Sure You Want to Upgrade The Diary?" ) )
                    ->add_extra_info( _( STRING::UPGRADE_DIARY_CONFIRM ) )
                    ->add_button( _( "Upgrade The Diary" ),
                                  std::bind( &UIDiary::enable_editing2, this ) )
                    ->show();
    }
    else
        enable_editing2();
}

void
UIDiary::enable_editing2()
{
    switch( Diary::d->enable_editing() )
    {
        case LIFEO::SUCCESS:
            break;
        case LIFEO::FILE_READ_ONLY: // this should never occur?
            AppWindow::p->show_info( _( "Cannot edit a read-only diary" ) );
            return;
        case FILE_NOT_WRITABLE:
            AppWindow::p->show_info( _( STRING::DIARY_NOT_WRITABLE ) );
            return;
        default:
            AppWindow::p->show_info( _( STRING::FAILED_TO_OPEN_DIARY ) );
            return;
    }

    m_B_smart_add->set_visible( true );
    m_MB_diary_close->set_visible( true );
    m_B_enable_edit->set_visible( false );
    m_Bx_extra_options->set_visible( true );

    m_WEL_main->set_editable();

    refresh_main_menu( Diary::LOGGED_IN_EDIT );

    m_CPo_entry.handle_edit_enabled();

    AppWindow::p->handle_edit_enabled();
    AppWindow::p->UI_entry->handle_edit_enabled();
    AppWindow::p->UI_extra->handle_edit_enabled();
}

void
UIDiary::handle_logout()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_B_smart_add->set_visible( false );
    m_Bx_diary->set_visible( false );
    m_B_search->set_visible( false );
    m_B_enable_edit->set_visible( false );
    m_MB_diary->set_visible( false );
    m_Bx_extra_options->set_visible( false );

    m_WEL_main->clear_and_disable();

    refresh_main_menu( Diary::LOGGED_OUT );

    m_CPo_entry.handle_logout();

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
UIDiary::refresh_main_menu( int level )
{
    // const bool F_diary_cls { level == Diary::LOGGED_OUT };
    const bool F_diary_opn { level > Diary::LOGGED_OUT };
    const bool F_diary_edt { level == Diary::LOGGED_IN_EDIT };

    m_A_sync_diary->set_enabled( F_diary_edt );
    m_A_export_diary->set_enabled( F_diary_opn );
    m_A_encrypt_diary->set_enabled( F_diary_edt && !Diary::d->is_encrypted() );
    m_A_change_password->set_enabled( F_diary_edt && Diary::d->is_encrypted() );
    m_A_open_diary_folder->set_enabled( F_diary_opn );
    m_A_update_date_seps->set_enabled( F_diary_edt );
}
// below method is for things that are hard to track so are updated on show:
void
UIDiary::refresh_main_menu_2()
{
    m_A_empty_trash->set_enabled( Diary::d->is_in_edit_mode() && !Diary::d->is_trash_empty() );
}

void
UIDiary::show_in_list( const Entry* entry, bool F_hard )
{
    if( !entry )
        m_WEL_main->select( nullptr ); // deselect all
    else
    if( F_hard ) // i.e. expand to the row
        m_WEL_main->present_entry_hard( entry );
    else
        m_WEL_main->present_entry( entry );
}

Entry*
UIDiary::add_today()
{
    return Diary::d->create_entry_dated( AppWindow::p->UI_entry->get_cur_entry(),
                                      Date::get_today(),
                                      false );
}

void
UIDiary::show_today()
{
    if( !Diary::d->is_open() ) return;

    Entry*  entry_today { Diary::d->get_entry_today() };
    bool    F_add       { false };

    if( !entry_today )   // add new entry if no entry exists on selected date
    {
        if( Diary::d->is_in_edit_mode() )
            F_add = true;
        else
            return;
    }
    // or current entry is already at today
    else if( Date::isolate_YMD( entry_today->get_date() ) ==
             Date::isolate_YMD( AppWindow::p->UI_entry->get_cur_entry()->get_date() ) )
        F_add = true;

    if( F_add )
    {
        entry_today = add_today();
        update_entry_list();
    }

    AppWindow::p->show_entry( entry_today );
}

void
UIDiary::show_Po_entry( Entry* entry, const Gdk::Rectangle&& rect )
{
    // auto pt { m_WEL_main->compute_point( *AppWindow::p,
    //                                        Gdk::Graphene::Point( rect.get_x(),
    //                                                              rect.get_y() ) ) };
    // Gdk::Rectangle rect2{ static_cast< int >( pt->get_x() ), static_cast< int >( pt->get_y() ),
    //                       rect.get_width(), rect.get_height() };
    m_CPo_entry.set( entry, *AppWindow::p, rect );
    m_CPo_entry.show();
}

void
UIDiary::show_prev_session_elem()
{
    Entry* entry{ Diary::d->get_prev_session_entry() };
    if( entry )
        AppWindow::p->show_entry( entry );
    else
        print_info( "Previous sesion element cannot be found" );
}

Entry*
UIDiary::update_for_added_entry( Entry* entry )
{
    if( ! Diary::d->is_in_edit_mode() )
        return nullptr;

    if( entry )
    {
        update_entry_list();

        if( entry->is_empty() )
            AppWindow::p->show_entry( entry );
        else
            AppWindow::p->UI_entry->show( entry->get_paragraph_1st() );
    }

    return entry;
}

Entry*
UIDiary::add_entry_dated( bool F_milestone )
{
    if( ! Diary::d->is_in_edit_mode() ) return nullptr;

    return update_for_added_entry( Diary::d->create_entry_dated( AppWindow::p->UI_entry->get_cur_entry()->get_parent(),
                                                              AppWindow::p->UI_extra->get_selected_date(),
                                                              F_milestone ) );

}
Entry*
UIDiary::add_entry_free()
{
    if( ! Diary::d->is_in_edit_mode() ) return nullptr;

    return update_for_added_entry( Diary::d->create_entry( nullptr,
                                                           false,
                                                           Date::get_today(),
                                                           "",
                                                           VT::ETS::NAME_ONLY::I ) );
}

void
UIDiary::merge_entries()
{
    auto&&    entries2merge   { get_selected_entries() };
    Entry*    p2entry_target  { AppWindow::p->UI_entry->get_cur_entry() };

    for( Entry* e2m : entries2merge )
    {
        if( e2m->get_id() == p2entry_target->get_id() ) continue;

        p2entry_target->append_entry_as_paras( e2m );
        Diary::d->dismiss_entry( e2m );
    }

    update_entry_list();
    AppWindow::p->UI_entry->refresh();
}

void
UIDiary::add_parent_to_entries()
{
    auto&&    entries       { get_selected_entries() };

    if( entries.empty() ) return;

    Entry*    p2entry_first { *entries.begin() };

    // detect the first one (can we trust that the selected entries are ordered properly?)
    for( Entry* e : entries )
    {
        if( e->get_list_order() < p2entry_first->get_list_order() )
            p2entry_first = e;
    }

    auto entry_parent { Diary::d->create_entry( p2entry_first->get_prev_or_up(),
                                                !p2entry_first->get_prev(),
                                                Date::get_today(),
                                                "",
                                                VT::ETS::INHERIT::I ) };

    Diary::d->move_entries( &entries, entry_parent, DropPosition::INTO );

    update_for_added_entry( entry_parent );
}

void
UIDiary::empty_trash()
{
    DialogMessage::init( AppWindow::p, _( "Are You Sure You Want to Empty The Trash?" ) )
                ->add_button( _( "Empty The Trash" ),
                              std::bind( &UIDiary::empty_trash2, this ),
                              "destructive-action" )
                ->show();
}
void
UIDiary::empty_trash2()
{
    AppWindow::p->show_entry( Diary::d->get_entry_first_untrashed() );

    VecEntries trashed_entries;
    for( auto& kv_entry : Diary::d->get_entries() )
        if( kv_entry.second->is_trashed() )
            trashed_entries.push_back( kv_entry.second );

    for( auto&& entry : trashed_entries )
    {
        AppWindow::p->UI_entry->remove_entry_from_history( entry );
        AppWindow::p->UI_extra->remove_entry_from_search( entry );
        Diary::d->dismiss_entry( entry );
    }

    update_entry_list();
}

void
UIDiary::update_date_separators()
{
    for( Entry* entry = Diary::d->get_entry_1st(); entry; entry = entry->get_next_straight() )
    {
        for( Paragraph* p = entry->get_paragraph_1st(); p; p = p->m_p2next )
        {
            for( auto f : p->m_formats )
            {
                if( f->type == VT::HFT_DATE )
                {
                    p->replace_text( f->pos_bgn, f->pos_end - f->pos_bgn,
                                     // reformats using the preferred separator:
                                     Date::format_string( f->ref_id ), // ref_id holds the date
                                     nullptr );
                }
            }
        }
    }

    AppWindow::p->UI_entry->refresh();
}

// DIALOGS
void
UIDiary::start_dialog_password()
{
    AppWindow::p->UI_entry->hide_popover();

    auto            gphrect { m_MB_diary->compute_bounds( *AppWindow::p ) };
    Gdk::Rectangle  rect    { int( gphrect->get_x() ),      int( gphrect->get_y() ),
                              int( gphrect->get_width() ),  int( gphrect->get_height() ) };
    DialogPassword::launch(
            Diary::d->is_encrypted() ? DialogPassword::OT_CHANGE : DialogPassword::OT_ADD,
            Diary::d, &rect, AppWindow::p,
            []{}, []{} );
}

void
UIDiary::start_dialog_sync()
{
    AppWindow::p->UI_entry->hide_popover();

    auto&& builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/dlg_sync.ui" ) };
    m_D_sync = Gtk::Builder::get_widget_derived< DialogSync >( builder, "D_synchronize" );

    m_D_sync->set_transient_for( *AppWindow::p );
    m_D_sync->show();
}

void
UIDiary::start_dialog_export()
{
    AppWindow::p->UI_entry->hide_popover();

    Gdk::Rectangle rect;

    auto do_start = [ this ]()
    {
        DialogPassword::finish( Diary::d->get_uri() );

        auto&& builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/dlg_export.ui" ) };
        m_D_export = Gtk::Builder::get_widget_derived< DialogExport >( builder, "W_export" );

        m_D_export->set_transient_for( *AppWindow::p );
        m_D_export->show();
    };

    if( Diary::d->is_encrypted() )
    {
        auto            gphrect { m_MB_diary->compute_bounds( *AppWindow::p ) };
        Gdk::Rectangle  rect    { int( gphrect->get_x() ),      int( gphrect->get_y() ),
                                  int( gphrect->get_width() ),  int( gphrect->get_height() ) };
        DialogPassword::launch( DialogPassword::OT_AUTHENTICATE, Diary::d,
                                &rect, AppWindow::p, do_start, []{} );
    }
    else
        do_start();
}

void
UIDiary::open_diary_folder() const
{
    auto file           { Gio::File::create_for_uri( Diary::d->m_uri ) };
    auto file_launcher  { Gtk::FileLauncher::create( file ) };
    file_launcher->open_containing_folder( *AppWindow::p,
                                           []( Glib::RefPtr< Gio::AsyncResult >& ){} );
}

// DIARY PROPERTIES
void
UIDiary::handle_language_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;
    std::string lang( m_CB_spellcheck->get_active_text() );
    Diary::d->set_lang( lang == _( STRING::OFF ) ? "" : lang );
}

void
UIDiary::handle_startup_type_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() == false )
        Diary::d->m_startup_entry_id = std::stol( m_CB_startup_type->get_active_id() );

    const bool F_show_fixed_item{ Diary::d->m_startup_entry_id >= DEID_MIN };

    m_Bx_startup_elem->set_visible( F_show_fixed_item );
    if( F_show_fixed_item )
       update_startup_elem();
}

void
UIDiary::update_startup_elem()
{
    m_W_startup_entry->set_entry( Diary::d->get_startup_entry() );
}

bool
UIDiary::go_to_startup_elem( const Ustring& )
{
    Entry* entry{ Diary::d->get_startup_entry() };
    if( entry )
        AppWindow::p->show_entry( entry );

    return true;
}
