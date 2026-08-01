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

#include <string>
#include <cstdlib>

#include <gtkmm/eventcontrollerlegacy.h>

#include "strings.hpp"
#include "lifeograph.hpp"
#include "app_window.hpp"
#include "dialogs/dialog_preferences.hpp"


using namespace LIFEO;

AppWindow*  AppWindow::p                        = nullptr;
int         AppWindow::parallel_thread_count    = 1;

// CONSTRUCTOR
AppWindow::AppWindow( BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& b )
:   Gtk::ApplicationWindow( cobject )
{
    p = this;

    Cipher::init();

    Diary::d = new Diary;

    const auto processor_count = std::thread::hardware_concurrency();
    PRINT_DEBUG( "Processor_count: ", processor_count );
    parallel_thread_count = std::max( processor_count - 2 , 1u );

#ifdef _WIN32
    set_default_icon_name( "net.sourceforge.Lifeograph" );
#endif

    // CONFIGURATION
    set_default_size( Lifeograph::settings.width, Lifeograph::settings.height );
    // gtkmm4: move is not available
    // if( Lifeograph::settings.position_x != POSITION_NOTSET &&
    //     Lifeograph::settings.position_y != POSITION_NOTSET )
    //     move( Lifeograph::settings.position_x, Lifeograph::settings.position_y );
    if( Lifeograph::settings.state_maximized )
        maximize();

    // DRAW UI
    Gtk::Button* B_info_close;

    try
    {
        auto builder{ Lifeograph::get_builder() };
        m_Bx_main       = builder->get_widget< Gtk::Box >( "Bx_main" );
        m_Hb_main       = builder->get_widget< Gtk::HeaderBar >( "Hb_main" );
        m_L_title       = builder->get_widget< Gtk::Label >( "L_title" );
        m_L_subtitle    = builder->get_widget< Gtk::Label >( "L_subtitle" );
        m_St_main       = builder->get_widget< Gtk::Stack >( "St_main" );
        m_Rv_info       = builder->get_widget< Gtk::Revealer >( "Rv_info" );
        m_L_info        = builder->get_widget< Gtk::Label >( "L_info" );
        m_B_info        = builder->get_widget< Gtk::Button >( "B_info" );
        B_info_close    = builder->get_widget< Gtk::Button >( "B_info_close" );

        UI_login = new UILogin;
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create the login view" );
    }

#ifdef _WIN32
    m_Hb_main->set_decoration_layout( ":minimize,maximize,close" );
#elif GTK_CHECK_VERSION(4, 18, 0)
    m_Hb_main->set_use_native_controls( true );
#endif

    Lifeograph::icons->diary_32 = Gdk::Pixbuf::create_from_file(
            Lifeograph::get_icon_dir() + "/diary-32.png" );

    // SIGNALS
    m_B_info->signal_clicked().connect( [ this ](){ handle_infobar_response(); } );
    B_info_close->signal_clicked().connect( [ this ](){ m_Rv_info->set_reveal_child( false ); } );

    Diary::d->m_dispatcher_postread_operations.connect(
            [ this ]()
            {
                UI_diary->handle_diary_ready();
                UI_extra->handle_diary_ready();
                UI_entry->get_textview()->refresh_entry();
            } );

    // CONTROLLERS
    auto event_controller_legacy{ Gtk::EventControllerLegacy::create() };
    event_controller_legacy->signal_event().connect(
            sigc::mem_fun( *this, &AppWindow::on_event ), false );
    add_controller( event_controller_legacy );

    // ACTIONS
    Lifeograph::p->add_action( "close_diary", [ this ](){ logout( true ); } );
    m_A_close_diary_wo_save = Lifeograph::p->add_action( "close_diary_wo_saving",
                                                         [ this ](){ logout( false ); } );
    m_A_disable_auto_close = Lifeograph::p->add_action_bool(
            "disable_auto_close",
            [ this ](){ handle_auto_logout_toggled(); },
            false );

    Lifeograph::p->add_action( "preferences",
            [](){ DialogPreferences::create(); } )->set_enabled(
                    ! Lifeograph::settings.rtflag_read_only );

    Lifeograph::p->add_action( "move_focus_to_next_panel", [ this ](){ focus_next_panel(); } );
    Lifeograph::p->add_action( "refresh_panels", [ this ](){ refresh_panels(); } );

    Lifeograph::p->set_accel_for_action( "app.move_focus_to_next_panel",    "<Ctrl>Tab" );
    Lifeograph::p->set_accel_for_action( "app.refresh_panels",              "F5" );
    Lifeograph::p->set_accel_for_action( "app.close_diary",                 "<Ctrl>Escape" );

    UI_login->handle_start();

    refresh_title();
}

AppWindow::~AppWindow()
{
}

bool
AppWindow::on_event( const Glib::RefPtr< const Gdk::Event >& )
{
    m_F_block_backup_on_idle = false;

    if( m_auto_close_freeze_level == 0 )
    {
        // restart
        m_seconds_remaining = Lifeograph::settings.idletime;
        m_connection_timeout.disconnect();
        m_connection_timeout = Glib::signal_timeout().connect_seconds(
                sigc::mem_fun( *this, &AppWindow::handle_idle ),
                Lifeograph::settings.idletime - LOGOUT_COUNTDOWN );

        update_auto_logout_count_down();
    }

    return false;
}

bool
AppWindow::on_close_request()
{
    PRINT_DEBUG( "AppWindow::on_close_request()" );

    if( Lifeograph::p->quit() )
    {
        if( UI_login ) delete UI_login;
        if( UI_diary ) delete UI_diary;
        if( UI_entry ) delete UI_entry;
        if( UI_extra ) delete UI_extra;
        if( Diary::d ) delete Diary::d;
        return false;
    }
    else
        return true;
}

bool
AppWindow::write_backup()
{
    if( m_F_block_backup_on_idle )
    {
        PRINT_DEBUG( "Saving backup blocked due to idleness" );
    }
    else
    {
        PRINT_DEBUG( "AppWindow::write_backup()" );

        if( Diary::d->write( Diary::d->get_uri() + LIFEO::LOCK_SUFFIX ) == SUCCESS )
            m_F_block_backup_on_idle = true;
    }

    return true;
}

void
AppWindow::focus_next_panel()
{
    if( Diary::d->is_open() )
    {
        if( UI_entry->get_textview()->has_focus() )
            UI_diary->get_list()->grab_focus();
        else
            UI_entry->get_textview()->grab_focus();
    }
}

void
AppWindow::refresh_panels()
{
    if( Diary::d->is_open() )
    {
        UI_diary->update_all_entries_filter_status();
        UI_diary->update_entry_list();
        UI_extra->refresh_active_panel();
        UI_entry->refresh();
    }
    else
        UI_login->populate_diaries();
}

void
AppWindow::fix_paned_positions_if_needed()
{
    if( m_Pd_extra->get_position() > ( Lifeograph::settings.height * 0.9 ) )
        m_Pd_extra->set_position( Lifeograph::settings.height / 2 );
}

void
AppWindow::init_textview_theme_classes()
{
    add_textview_theme_class( ThemeSystem::get() );

    for( auto kv_theme : *Diary::d->get_p2themes() )
        add_textview_theme_class( kv_theme.second );
}
void
AppWindow::add_textview_theme_class( Theme* theme )
{
    auto css_provider { Gtk::CssProvider::create() };
    css_provider->load_from_data( theme->get_css_class_def() );
    m_textview_css_providers[ theme->get_css_class_name() ] = css_provider;
    Gtk::StyleProvider::add_provider_for_display( Gdk::Display::get_default(),
                                                  css_provider,
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
}
void
AppWindow::remove_textview_theme_class( Theme* theme )
{
    auto class_name   { theme->get_css_class_name() };
    auto css_provider { m_textview_css_providers.find( class_name ) };

    if( css_provider == m_textview_css_providers.end() )
    {
        PRINT_DEBUG( "Provider cannot be found: ", class_name );
        return;
    }

    Gtk::StyleProvider::remove_provider_for_display( Gdk::Display::get_default(),
                                                     css_provider->second );

    m_textview_css_providers.erase( class_name );
}
void
AppWindow::update_textview_theme_class( Theme* theme )
{
    auto class_name   { theme->get_css_class_name() };
    auto css_provider { m_textview_css_providers.find( class_name ) };

    if( css_provider == m_textview_css_providers.end() )
    {
        PRINT_DEBUG( "Provider cannot be found: ", class_name );
        return;
    }

    css_provider->second->load_from_data( theme->get_css_class_def() );
}
void
AppWindow::clear_textview_theme_classes()
{
    for( auto kv_css_provider : m_textview_css_providers )
    {
        Gtk::StyleProvider::remove_provider_for_display( Gdk::Display::get_default(),
                                                         kv_css_provider.second );
    }

    m_textview_css_providers.clear();
}

void
AppWindow::show_elem( DiaryElement* elem )
{
    switch( elem->get_type() )
    {
        case DiaryElement::ET_PARAGRAPH:
            UI_entry->show( dynamic_cast< Paragraph* >( elem ) );
            break;
        case DiaryElement::ET_ENTRY:
            show_entry( dynamic_cast< Entry* >( elem ) );
            break;
        case DiaryElement::ET_CHART:
            UI_extra->show_chart( elem->get_name() );
            break;
        case DiaryElement::ET_TABLE:
            UI_extra->show_table( elem->get_name() );
            break;
        default:
            break;
    }
}

void
AppWindow::show_elem( DEID id )
{
    DiaryElement* elem{ Diary::d->get_element( id ) };

    if( elem )
        show_elem( elem );
}

void
AppWindow::show_entry( Entry* entry )
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    UI_extra->set_entry( entry );
    UI_diary->show_in_list( entry );
    UI_entry->show( entry );  // needs to come last to make sure that the text editor is focused

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
AppWindow::go_to_prev()
{
    // get_prev_straight is not very meaningful here
    Entry* entry_prev{ UI_entry->get_cur_entry()->get_prev() };

    if( !entry_prev )
        entry_prev = UI_entry->get_cur_entry()->get_parent();

    if( entry_prev )
        show_entry( entry_prev );
}
void
AppWindow::go_to_next()
{
    Entry* entry_next{ UI_entry->get_cur_entry()->get_next_straight() };

    if( entry_next )
        show_entry( entry_next );
}

void
AppWindow::show_about()
{
    static Gtk::AboutDialog* s_about_dialog{ nullptr };

    if( s_about_dialog == nullptr )
    {
        s_about_dialog = Lifeograph::get_builder2()->get_widget< Gtk::AboutDialog >(
                "aboutdialog" );
        s_about_dialog->set_name( PROGRAM_NAME );
        s_about_dialog->set_version( Lifeograph::PROGRAM_VERSION_STRING );
        s_about_dialog->set_transient_for( *p );
        s_about_dialog->set_hide_on_close( true );
        // TODO: 3.1: split into its own ui file to be able to destroy
    }

    s_about_dialog->show();
}

void
AppWindow::show_info( const Ustring& text, const Ustring& button_text, InfoResponse response )
{
    m_L_info->set_markup( text );
    m_B_info->set_label( button_text );
    m_B_info->set_visible( !button_text.empty() );
    m_info_resp_cur = response;
    m_Rv_info->set_reveal_child( true );
}

void
AppWindow::handle_infobar_response()
{
    switch( m_info_resp_cur )
    {
        case RESP_UNDO_REMOVE_DIARY:
            UI_login->undo_remove_selected_diary();
            break;
        default:
            break;
    }

    m_Rv_info->set_reveal_child( false );
}

void
AppWindow::handle_undo()
{
    UI_entry->undo();
}

void
AppWindow::handle_redo()
{
    UI_entry->redo();
}

void
AppWindow::handle_login()
{
    if( m_Pd_main == nullptr ) // first login
    {
        try
        {
            // PANES
            m_Pd_main   = Lifeograph::get_builder()->get_widget< Gtk::Paned >( "P_main" );
            m_Pd_extra  = Lifeograph::get_builder()->get_widget< Gtk::Paned >( "P_second" );

            // ICONS
            auto theme { Gtk::IconTheme::get_for_display( Gdk::Display::get_default() ) };
            theme->add_search_path( Lifeograph::get_icon_dir() );

            // is loaded upfront as a Pixbuf: Lifeograph::icons->diary_32 = theme->lookup_icon( "diary-32", 32 );

            Lifeograph::icons->entry_16 =           Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/entry-16.png" );
            Lifeograph::icons->entry_32 =           Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/entry-32.png" );
            Lifeograph::icons->entry_plus_16 =      Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/entry_plus-16.png" );
            Lifeograph::icons->entry_parent_16 =    Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/entry_parent-16.png" );
            Lifeograph::icons->entry_parent_32 =    Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/entry_parent-32.png" );
            Lifeograph::icons->milestone_16 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/milestone-16.png" );
            Lifeograph::icons->milestone_32 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/milestone-32.png" );

            Lifeograph::icons->tag_16 =             Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/tag-16.png" );
            Lifeograph::icons->tag_32 =             Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/tag-32.png" );

            Lifeograph::icons->favorite_16 =        Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/favorite-16.png" );

            // Lifeograph::icons->todo_auto_16 =       Gdk::Pixbuf::create_from_file(
            //         Lifeograph::get_icon_dir() + "/todo_auto-16.png" );
            // Lifeograph::icons->todo_auto_32 =       Gdk::Pixbuf::create_from_file(
            //         Lifeograph::get_icon_dir() + "/todo_auto-32.png" );
            Lifeograph::icons->todo_open_16 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_open-16.png" );
            Lifeograph::icons->todo_open_32 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_open-32.png" );
            Lifeograph::icons->todo_progressed_16 = Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_progressed-16.png" );
            Lifeograph::icons->todo_progressed_32 = Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_progressed-32.png" );
            Lifeograph::icons->todo_done_16 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_done-16.png" );
            Lifeograph::icons->todo_done_32 =       Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_done-32.png" );
            Lifeograph::icons->todo_canceled_16 =   Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_canceled-16.png" );
            Lifeograph::icons->todo_canceled_32 =   Gdk::Pixbuf::create_from_file(
                    Lifeograph::get_icon_dir() + "/todo_canceled-32.png" );

            Lifeograph::icons->filter = lookup_default_icon_pixbuf( "filter-16-symbolic", 16 );
            Lifeograph::icons->table = lookup_default_icon_pixbuf( "table-16-symbolic", 16 );
            Lifeograph::icons->chart = lookup_default_icon_pixbuf( "chart-16-symbolic", 16 );

            Lifeograph::icons->map_point = lookup_default_icon_pixbuf( "map-point", 16 );

            Lifeograph::icons->go_prev = lookup_default_icon_pixbuf( "go-previous-symbolic", 16 );
            Lifeograph::icons->go_next = lookup_default_icon_pixbuf( "go-next-symbolic", 16 );
            Lifeograph::icons->go_home = lookup_default_icon_pixbuf( "go-home-symbolic", 16 );

            // PANELS & VIEWS
            UI_diary = new UIDiary;
            UI_extra = new UIExtra;
            UI_entry = new UIEntry;
        }
        catch( std::exception &ex )
        {
            throw LIFEO::Error( ex.what() );
        }

        // GEOMETRY
        const int width{ get_size( Gtk::Orientation::HORIZONTAL ) };
        // try to keep the same ratio between the panels before the resizing occurs:
        if( width != Lifeograph::settings.width )
        {
            Lifeograph::settings.position_paned_main *= float( width ) / Lifeograph::settings.width;
        }

        // check the diary panel's size against the minimum:
        if( Lifeograph::settings.position_paned_main > ( width - UIDiary::MIN_WIDTH ) )
        {
            PRINT_DEBUG( "Diary panel width set to minimum value" );
            Lifeograph::settings.position_paned_main = width - UIDiary::MIN_WIDTH;
        }

        m_Pd_main->set_position( Lifeograph::settings.position_paned_main );
        m_Pd_extra->set_position( Lifeograph::settings.position_paned_extra );

        // ACTIONS
        using VInt = Glib::Variant< int >;
        Lifeograph::p->add_action( "undo", [ this ](){ handle_undo(); } );
        Lifeograph::p->set_accel_for_action( "app.undo", "<Ctrl>Z" );
        Lifeograph::p->add_action( "redo", [ this ](){ handle_redo(); } );
        Lifeograph::p->set_accel_for_action( "app.redo", "<Ctrl><Shift>Z" );
        Lifeograph::p->add_action_with_parameter(
                "show_elem",
                VInt::variant_type(),
	            []( const Glib::VariantBase& value )
	            {
                    const DEID id{ Glib::VariantBase::cast_dynamic< VInt >( value ).get() };
                    AppWindow::p->show_elem( id );
                } );
    }

    init_textview_theme_classes();

    m_Rv_info->set_reveal_child( false );
    m_St_main->set_visible_child( *m_Pd_main );

    m_A_close_diary_wo_save->set_enabled( false );
    update_toggle_auto_close_gui( true );
}

void
AppWindow::handle_edit_enabled()
{
    m_connection_backup = Glib::signal_timeout().connect_seconds(
            sigc::mem_fun( *this, &AppWindow::write_backup ), BACKUP_INTERVAL );

    m_A_close_diary_wo_save->set_enabled( true );
}

void
AppWindow::handle_logout()
{
    clear_textview_theme_classes();

    if( Diary::d->is_logged_time_out() )
        show_info( _( STRING::ENTER_PASSWORD_TIMEOUT ) );
}

// AUTO LOGOUT SYSTEM
bool
AppWindow::handle_idle()
{
    if( m_auto_close_freeze_level > 0 )
        return false;

    if( m_seconds_remaining > LOGOUT_COUNTDOWN )
        m_seconds_remaining = LOGOUT_COUNTDOWN;
    else
        m_seconds_remaining--;

    if( m_seconds_remaining > 0 )
    {
        m_connection_timeout.disconnect();
        m_connection_timeout = Glib::signal_timeout().connect_seconds(
                sigc::mem_fun( *this, &AppWindow::handle_idle ), 1 );

        update_auto_logout_count_down();
    }
    else
    {
        update_auto_logout_count_down();

        logout( true );
        Diary::d->set_timed_out();
    }

    return false;
}

void
AppWindow::freeze_auto_close()
{
    if( m_auto_close_freeze_level == 0 )
    {
        m_connection_timeout.disconnect();
        m_seconds_remaining = Lifeograph::settings.idletime;
        update_auto_logout_count_down();
    }

    m_auto_close_freeze_level++;
}

void
AppWindow::unfreeze_auto_close()
{
    if( Lifeograph::settings.idletime > 0 && Diary::d->is_encrypted() &&
        m_auto_close_freeze_level == 1 )
    {
        m_connection_timeout = Glib::signal_timeout().connect_seconds(
                sigc::mem_fun( *this, &AppWindow::handle_idle ),
                Lifeograph::settings.idletime - LOGOUT_COUNTDOWN );
    }

    m_auto_close_freeze_level--;
}

void
AppWindow::update_auto_logout_count_down()
{
    static Gtk::Window* W_count_down( nullptr );
    static Gtk::Label* L_count_down( nullptr );

    if( m_seconds_remaining > 0  && m_seconds_remaining <= LOGOUT_COUNTDOWN )
    {
        if( !W_count_down )
        {
            auto builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/auto_logout.ui" ) };
            W_count_down = builder->get_widget< Gtk::Window >( "W_auto_logout" );
            L_count_down = builder->get_widget< Gtk::Label >( "L_auto_logout" );

            W_count_down->set_transient_for( *this );
        }

        L_count_down->set_label( Glib::ustring::compose(
                _( "%1 SECONDS TO CLOSE" ), m_seconds_remaining ) );

        W_count_down->show();
    }
    else if( W_count_down )
        W_count_down->hide();
}

void
AppWindow::handle_auto_logout_toggled()
{
    if( is_auto_close_in_use() )
        freeze_auto_close();
    else
        unfreeze_auto_close();

    update_toggle_auto_close_gui(); // the action's state does not change automatically
}

void
AppWindow::update_toggle_auto_close_gui( bool flag_availabilities_too )
{
    if( flag_availabilities_too )
    {
        m_auto_close_freeze_level =
                ( Lifeograph::settings.idletime && Diary::d->is_encrypted() ) ? 0 : 1;

        m_A_disable_auto_close->set_enabled( !m_auto_close_freeze_level );
    }

    m_A_disable_auto_close->change_state( ! is_auto_close_in_use() );
}

// LOG OUT
void
AppWindow::check_finish_diary_session( bool opt_save )
{
    if( Diary::d->is_in_edit_mode() )
    {
        if( opt_save )
        {
            if( Diary::d->write() != SUCCESS )
            {
                String path{ Diary::d->get_uri() + ".~unsaved~" };

                // try to write the backup
                if( Diary::d->write( path ) != SUCCESS )
                {
                    path = ( Glib::get_tmp_dir() + "/" + Diary::d->get_name() );
                    Diary::d->write( path );
                }

                DialogMessage::init( AppWindow::p,
                                     _( "Diary couldn't be saved. "
                                        "A rescue copy is saved at the location below:" ),
                                     _( "_Close" ) )
                            ->add_extra_info( path )
                            ->show();
            }
        }
        else
        {
            DialogMessage::init( AppWindow::p,
                                 _( "Are you sure you want to close the diary without saving?" ) )
                        ->add_extra_info(
                                Ustring::compose(
                                        _( "Your changes will be backed up in %1. "
                                           "If you exit normally, your diary is saved "
                                           "automatically." ),
                                           "<b>" + Diary::d->get_uri() + ".~unsaved~</b>" ),
                                true )
                        ->add_button(   _( "Close _without Saving" ),
                                        [ & ]()
                                        {
                                            Diary::d->write( Diary::d->get_uri() + ".~unsaved~" );
                                            finish_login_session();
                                        },
                                        "destructive-action" )
                        ->show();

            return;
        }
    }

    finish_login_session();
}

void
AppWindow::finish_login_session()
{
    if( Diary::d->is_in_edit_mode() )
        m_connection_backup.disconnect();

    m_connection_timeout.disconnect();

    Lifeograph::signal_logout().emit(); // only for DialogEvent

    Lifeograph::START_INTERNAL_OPERATIONS();

    UI_diary->handle_logout();
    UI_entry->handle_logout();
    UI_extra->handle_logout();
    this->handle_logout();

    Diary::d->remove_lock_if_necessary();
    Diary::d->clear();

    UI_login->handle_logout();
    m_St_main->set_visible_child( "login" );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    m_auto_close_freeze_level = 1;

    refresh_title();
}

void
AppWindow::logout( bool opt_save )
{
    // positions:
    Lifeograph::settings.position_paned_main = m_Pd_main->get_position();
    Lifeograph::settings.position_paned_extra = m_Pd_extra->get_position();

    // files added to recent list here if not already there:
    if( ! Diary::d->get_uri().empty() )
        Lifeograph::settings.recentfiles.insert( Diary::d->get_uri() );

    // should be reset to prevent logging in again:
    Lifeograph::settings.rtflag_open_directly = false;

    check_finish_diary_session( opt_save );
}

void
AppWindow::confirm_dismiss_element( const Ustring& name,
                                    const FuncVoid& dismiss_handler,
                                    Gtk::Widget* W_extra )
{
    DialogMessage* dlg {
            DialogMessage::init( AppWindow::p,
                                 Ustring::compose( _( "Are you sure, you want to dismiss %1?" ),
                                                   Ustring::compose( "\"%1\"", name ) ) ) };

    if( W_extra ) dlg->add_extra_widget( W_extra ); // must come before add_extra_info

    dlg->add_extra_info( _( "This operation cannot be undone!" ) )
       ->add_button( _( "_Dismiss" ), dismiss_handler, "destructive-action" )->show();
}

void
AppWindow::refresh_title()
{
    if( Diary::d->is_open() )
        set_title( STR::compose( PROGRAM_NAME, " - ", Diary::d->get_name() ) );
        // in this case the headerbar title is set by entry view
    else
    {
        set_title( PROGRAM_NAME );
        set_subtitle( "" );
    }
}

void
AppWindow::login()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    this->handle_login();       // must come first
    UI_login->handle_login();
    UI_diary->handle_login();
    UI_extra->handle_login();
    UI_entry->handle_login();   // must come last

    show_entry( Diary::d->get_startup_entry() );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // NOTE: LOGGED_IN flag used to be set here, now it is set much earlier
}
