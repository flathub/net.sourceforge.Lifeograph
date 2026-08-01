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


#ifndef LIFEOGRAPH_BASE_HEADER
#define LIFEOGRAPH_BASE_HEADER


#include <gtkmm.h>

extern "C"
{
#include <enchant.h>
}

#include "settings.hpp"
#include "diaryelements/diarydata.hpp"


// #define LIFEOGRAPH_BETA_STAGE


namespace LIFEO
{
    static const char   PROGRAM_NAME[]      = "Lifeograph";

    static const int    BACKUP_INTERVAL     = 30;    // seconds
    static const int    LOGOUT_COUNTDOWN    = 10;    // seconds

struct Icons
{
    R2Pixbuf    diary_32;
    R2Pixbuf    entry_16;
    R2Pixbuf    entry_32;
    R2Pixbuf    entry_parent_16;
    R2Pixbuf    entry_plus_16;
    R2Pixbuf    entry_parent_32;
    R2Pixbuf    milestone_16;
    R2Pixbuf    milestone_32;
    R2Pixbuf    tag_16;
    R2Pixbuf    tag_32;
    R2Pixbuf    favorite_16;
    R2Pixbuf    todo_open_16;
    R2Pixbuf    todo_open_32;
    R2Pixbuf    todo_progressed_16;
    R2Pixbuf    todo_progressed_32;
    R2Pixbuf    todo_done_16;
    R2Pixbuf    todo_done_32;
    R2Pixbuf    todo_canceled_16;
    R2Pixbuf    todo_canceled_32;
    R2Pixbuf    filter;
    R2Pixbuf    table;
    R2Pixbuf    chart;
    R2Pixbuf    map_point;
    // below ones are regular Gtk icons but added due to the difficulty of getting them as Pixbuf
    R2Pixbuf    go_prev;
    R2Pixbuf    go_next;
    R2Pixbuf    go_home;
};

enum DRAG_TYPE
{
    DT_ENTRY_ROW, DT_TEXT
};

class AppWindow; // forward declaration

// Lifeograph Application
class Lifeograph : public Gtk::Application
{
    public:
        static const char           PROGRAM_VERSION_STRING[];

        static Glib::RefPtr< Lifeograph >
                                    create();

        bool                        quit();

        virtual                     ~Lifeograph();

        static void                 init_paths();

        static void                 START_INTERNAL_OPERATIONS()
        { s_internal_operation_depth++; }
        static void                 FINISH_INTERNAL_OPERATIONS()
        { s_internal_operation_depth--; }
        static bool                 is_internal_operations_ongoing()
        { return( s_internal_operation_depth > 0 ); }

        static void                 update_gtk_theme()
        {
            Gtk::Settings::get_default()->property_gtk_application_prefer_dark_theme() =
                settings.use_dark_theme;
        }

        static Glib::RefPtr< Gtk::Builder >&
                                    get_builder()
        { return s_builder; }
        static Glib::RefPtr< Gtk::Builder >&
                                    get_builder2();

        static String               get_icon_dir();
        static R2Pixbuf&            get_todo_icon( ElemStatus );
        static R2Pixbuf&            get_todo_icon32( ElemStatus );

        static void                 show_manual();

        // CLIPBOARD
        void                        clear_entry_text_register_cb();
        void                        set_entry_text_register_cb( Paragraph* );
        Paragraph*                  get_entry_text_register_cb();

        // DRAG & DROP
        void                        set_entry_text_register_dd( Paragraph* );
        Paragraph*                  get_entry_text_register_dd();

        // COLORS
        static const String&        get_color_insensitive();

        // ENCHANT
        static EnchantBroker*       s_enchant_broker;
        static SetStrings           s_lang_list;

        static bool                 load_gui( Glib::RefPtr< Gtk::Builder >&, const String& );
        static Glib::RefPtr< Gtk::Builder >
                                    create_gui( const String& );

        static SignalVoid           signal_logout()
        { return s_signal_logout; }

        // CONFIGURATION
        static std::string          BINPATH;
        static std::string          SHAREDIR;
        static std::string          MANUALPATH;
        static std::string          EXAMPLEDIARYPATH;
        static std::string          LOCALEDIR;

        // OBJECTS
        static Lifeograph*          p; // static pointer to itself

        static Settings             settings;
        static Icons*               icons;
        static VecUstrings          s_entry_name_styles;

        R2Action                    get_action( const Ustring& name )
        { return std::dynamic_pointer_cast< Gio::SimpleAction >( lookup_action( name ) ); }

    protected:
                                    Lifeograph();

        static void                 fill_lang_list_cb( const char* const l, const char* const,
                                                       const char* const, const char* const,
                                                       void* )
        { s_lang_list.insert( l ); }

        static bool                 load_gui( const std::string& path )
        { return load_gui( s_builder, path ); }

        void                        on_startup();
        void                        on_activate();
        int                         on_command_line(
                                        const Glib::RefPtr< Gio::ApplicationCommandLine >& );

        static String               s_color_insensitive;

        static volatile int         s_internal_operation_depth;

        static SignalVoid           s_signal_logout;

        // BUILDER
        static Glib::RefPtr< Gtk::Builder >
                                    s_builder;
        static bool                 m_flag_ui2_loaded;

        Paragraph*                  m_entry_text_register_cb { nullptr };
        Paragraph*                  m_entry_text_register_dd { nullptr };

        // APPLICATION MENU
        //Glib::RefPtr< Gio::Menu >   m_menu;
};

} // end of namespace LIFEO

#endif

