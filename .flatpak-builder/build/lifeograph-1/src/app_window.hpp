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


#ifndef LIFEOGRAPH_APP_WINDOW_HEADER
#define LIFEOGRAPH_APP_WINDOW_HEADER

#define LIFEOGRAPH_DEVELOPMENT_VERSION


#include <gtkmm.h>

#include "lifeograph.hpp"
#include "helpers.hpp"
#include "settings.hpp"
#include "ui_login.hpp"
#include "ui_entry.hpp"
#include "ui_diary.hpp"
#include "ui_extra.hpp"


namespace LIFEO
{

class AppWindow : public Gtk::ApplicationWindow
{
    public:
        enum InfoResponse { RESP_OK = 10, RESP_UNDO_REMOVE_DIARY = 20 };

                                    AppWindow( BaseObjectType*,
                                               const Glib::RefPtr<Gtk::Builder>& );
                                    ~AppWindow();
        static AppWindow*           p;

        void                        refresh_title();
        void                        set_title( const Ustring& title )
        { m_L_title->set_text( title ); }
        void                        set_subtitle( const Ustring& subt )
        {
            if( subt.empty() )
                m_L_subtitle->hide();
            else
            {
                m_L_subtitle->show();
                m_L_subtitle->set_text( subt );  // TODO: 3.1: use markup
            }
        }

        void                        enter_selection_mode()
        { m_Hb_main->add_css_class( "selection-mode" ); }
        void                        exit_selection_mode()
        { m_Hb_main->remove_css_class( "selection-mode" ); }

        void                        confirm_dismiss_element( const Ustring&,
                                                             const FuncVoid&,
                                                             Gtk::Widget* = nullptr );

        void                        login();
        void                        check_finish_diary_session( bool = true );
        void                        finish_login_session();
        void                        logout( bool );

        void                        handle_login();
        void                        handle_edit_enabled();
        void                        handle_logout();

        void                        handle_undo();
        void                        handle_redo();

        static void                 show_about();

        void                        show_info( const Ustring&,
                                               const Ustring& = "",
                                               InfoResponse = RESP_OK );
        void                        handle_infobar_response();

        bool                        write_backup();

        void                        focus_next_panel();
        void                        refresh_panels();
        void                        fix_paned_positions_if_needed();

        void                        init_textview_theme_classes();
        void                        add_textview_theme_class( Theme* );
        void                        remove_textview_theme_class( Theme* );
        void                        update_textview_theme_class( Theme* );
        void                        clear_textview_theme_classes();

        // METHODS INVOLVING MULTIPLE PANELS AT ONCE
        void                        show_elem( DiaryElement* );
        void                        show_elem( DEID );
        void                        show_entry( Entry* );

        void                        go_to_prev();
        void                        go_to_next();

        // AUTO LOGOUT FUNCTIONALITY
        bool                        handle_idle();
        void                        freeze_auto_close();
        void                        unfreeze_auto_close();
        void                        update_auto_logout_count_down();
        // void                        disable_auto_logout_for_the_session();
        // void                        reenable_auto_logout_for_the_session();
        bool                        is_auto_close_in_use()
        { return( m_auto_close_freeze_level == 0 ); }
        void                        handle_auto_logout_toggled();
        void                        update_toggle_auto_close_gui( bool = false );

        bool                        on_event( const Glib::RefPtr< const Gdk::Event >& );

        // UI FUNCTIONALITY
        UILogin*                    UI_login{ nullptr };
        UIDiary*                    UI_diary{ nullptr };
        UIEntry*                    UI_entry{ nullptr };
        UIExtra*                    UI_extra{ nullptr };

        static int                  parallel_thread_count;

    protected:
        bool                        on_close_request() override;

        // WIDGETS
        Gtk::Box*                   m_Bx_main;
        Gtk::HeaderBar*             m_Hb_main;
        Gtk::Label*                 m_L_title;
        Gtk::Label*                 m_L_subtitle;
        Gtk::Stack*                 m_St_main{ nullptr };
        Gtk::Revealer*              m_Rv_info;
        Gtk::Label*                 m_L_info;
        Gtk::Button*                m_B_info;

        Gtk::Paned*                 m_Pd_main{ nullptr };
        Gtk::Paned*                 m_Pd_extra{ nullptr };

        sigc::connection            m_connection_backup;

        int                         m_seconds_remaining{ LOGOUT_COUNTDOWN + 1 };
        sigc::connection            m_connection_timeout;
        int                         m_auto_close_freeze_level{ 1 }; // reflects the result of all factors
        bool                        m_F_block_backup_on_idle{ false };
        InfoResponse                m_info_resp_cur;

        std::map< const String, Glib::RefPtr< Gtk::CssProvider > > m_textview_css_providers;

        // ACTIONS
        R2Action                    m_A_disable_auto_close;
        R2Action                    m_A_close_diary_wo_save;
};

} // end of namespace LIFEO

#endif
