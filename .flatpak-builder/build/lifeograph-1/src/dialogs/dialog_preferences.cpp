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


#include "dialog_preferences.hpp"
#include "../app_window.hpp"
#include "../widgets/widget_entrypicker.hpp"

namespace LIFEO
{

// STATIC MEMBERS
DialogPreferences *DialogPreferences::ptr;


DialogPreferences::DialogPreferences( BaseObjectType* cobject,
                                      const Glib::RefPtr< Gtk::Builder >& builder )
:    Gtk::Window( cobject )
{
    m_CB_autologout     = builder->get_widget< Gtk::ComboBoxText >( "CB_prefs_autologout" );
    m_Sw_save_backups   = builder->get_widget< Gtk::Switch >( "Sw_prefs_save_backups" );
    m_FCB_backup_path   = Gtk::Builder::get_widget_derived< FileChooserButton >(
                                builder, "FCB_prefs_backup_path" );
    m_CB_date_order     = builder->get_widget< Gtk::ComboBoxText >( "CB_prefs_date_order" );
    m_CB_date_separator = builder->get_widget< Gtk::ComboBoxText >( "CB_prefs_date_separator" );
    m_CB_1st_week_day   = builder->get_widget< Gtk::ComboBoxText >( "CB_prefs_1st_week_day" );
    m_RB_units_imperial = builder->get_widget< Gtk::ToggleButton >( "RB_prefs_units_imperial" );
    m_Sw_use_dark_theme = builder->get_widget< Gtk::Switch >( "Sw_prefs_use_dark_theme" );

    m_CB_autologout->set_active_id( std::to_string( Lifeograph::settings.idletime ) );
    if( m_CB_autologout->get_active_row_number() == -1 ) // for non-conforming values
    {
        Lifeograph::settings.idletime = IDLETIME_DEFAULT;
        m_CB_autologout->set_active_id( std::to_string( IDLETIME_DEFAULT ) );
    }
    m_Sw_save_backups->set_active( Lifeograph::settings.save_backups );
    m_FCB_backup_path->set_pick_folder_mode();
    m_FCB_backup_path->set_uri( Lifeograph::settings.backup_folder_uri );
    m_CB_date_order->set_active( Lifeograph::settings.date_format_order );
    m_CB_date_separator->set_active( Lifeograph::settings.date_format_separator );
    m_CB_1st_week_day->set_active( Date::s_week_start_day );
    m_RB_units_imperial->set_active( Lifeograph::settings.use_imperial_units );
    m_Sw_use_dark_theme->set_active( Lifeograph::settings.use_dark_theme );

    m_Sw_use_dark_theme->property_active().signal_changed().connect(
            [ this ]()
            {
                Lifeograph::settings.use_dark_theme = m_Sw_use_dark_theme->get_active();
                Lifeograph::update_gtk_theme();
            } );

    m_CB_date_order->signal_changed().connect(     [ this ]() { update_date_format(); } );
    m_CB_date_separator->signal_changed().connect( [ this ]() { update_date_format(); } );
    m_CB_1st_week_day->signal_changed().connect(
            [ this ]()
            {
                Date::s_week_start_day = m_CB_1st_week_day->get_active_row_number();
                if( Diary::d->is_open() )
                    AppWindow::p->UI_extra->refresh_calendar();
            } );

    m_RB_units_imperial->signal_toggled().connect(
            [ this ]()
            {
                Lifeograph::settings.use_imperial_units = m_RB_units_imperial->get_active();
                if( Diary::d->is_open() )
                    AppWindow::p->UI_extra->refresh_active_panel();
            } );

    m_FCB_backup_path->m_signal_file_set.connect(
            []( const String uri )
            {
                Lifeograph::settings.backup_folder_uri = uri;
            } );

    signal_close_request().connect( [ this ]() { handle_close(); return false; }, false );
}

void
DialogPreferences::update_date_format()
{
    Lifeograph::settings.date_format_order = m_CB_date_order->get_active_row_number();
    Lifeograph::settings.date_format_separator = m_CB_date_separator->get_active_row_number();
    Lifeograph::settings.update_date_format();

    if( Diary::d->is_open() )
    {
        AppWindow::p->UI_diary->update_entry_list();
        AppWindow::p->UI_entry->refresh_title();
    }
}

void
DialogPreferences::handle_close()
{
    // APPLY CHANGES
    Lifeograph::settings.idletime = std::stoi( m_CB_autologout->get_active_id() );
    Lifeograph::settings.save_backups = m_Sw_save_backups->get_active();
    AppWindow::p->update_toggle_auto_close_gui( true );
}

void
DialogPreferences::create()
{
    auto&& builder{ Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/preferences.ui" ) };
    ptr = Gtk::Builder::get_widget_derived< DialogPreferences >( builder, "D_preferences" );
    ptr->set_transient_for( *AppWindow::p );
    ptr->show();
}

} // end of namespace LIFEO
