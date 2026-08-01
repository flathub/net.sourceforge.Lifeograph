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


#ifndef LIFEOGRAPH_UI_EXTRA_HEADER
#define LIFEOGRAPH_UI_EXTRA_HEADER


#include <gtkmm.h>

#include "widgets/widget_calendar.hpp"
#include "widgets/widget_filter.hpp"
#include "widgets/widget_chart.hpp"
#include "widgets/widget_table.hpp"
#include "widgets/widget_picker.hpp"
#include "widgets/widget_textviewsearch.hpp"
#include "widgets/widget_textviewtheme.hpp"


namespace LIFEO
{

// FORWARD DECLARATION OF THE MAP WIDGET (to keep its problems in its own module)
class WidgetMap;

// PANEL OF EXTRA ITEMS
class UIExtra
{
    public:
                                    UIExtra();
                                    ~UIExtra();

        void                        handle_login();
        void                        handle_diary_ready()
        {
            set_active_chart( Diary::d->get_chart_active()->get_name() );
            set_active_table( Diary::d->get_table_active()->get_name() );
        }
        void                        handle_edit_enabled();
        void                        handle_logout();

        void                        set_view( const Ustring& );
        void                        set_entry( Entry * );

        void                        refresh_active_panel( bool = false );
        void                        refresh_after_sync();
        void                        remove_entry( Entry* );
        void                        update_for_entry( Entry* );

        // CALENDAR VIEW
        void                        refresh_calendar();
        void                        show_date_in_cal( DateV );
        DateV                       get_selected_date()
        { return m_W_calendar->get_selected_date(); }

        // SEARCH VIEW
        void                        remove_entry_from_search( Entry* );
        void                        update_search_for_entry( const Entry* );
        void                        focus_searching( const Ustring& = "" );
        void                        go_to_match_at( int );
        void                        replace_match( const HiddenFormat* );
        void                        replace_match_cur();
        void                        replace_all_matches();

        // CHART VIEW
        void                        show_chart( const Ustring& );
        void                        set_active_chart( const Ustring& );
        void                        add_new_chart( const Ustring&, const Ustring&, bool = false );
        void                        dismiss_chart( const Ustring& );
        void                        handle_chart_selected( const Ustring& );
        bool                        handle_chart_renamed( const Ustring& );
        void                        handle_chart_edited();
        void                        refresh_chart();

        // TABLE VIEW
        void                        show_table( const Ustring& );
        void                        set_active_table( const Ustring& );
        void                        add_new_table( const Ustring&, const Ustring&, bool = false );
        void                        dismiss_table( const Ustring& );
        void                        handle_table_selected( const Ustring& );
        bool                        handle_table_renamed( const Ustring& );
        void                        handle_table_edited();
        void                        refresh_table();

        // THEME VIEW
        void                        update_theme_list();
        void                        edit_theme();
        void                        assign_theme();
        void                        assign_theme_to_sel();

        void                        handle_theme_name_changed();
        void                        handle_theme_rclick( int, double, double );

        // MAP VIEW
        WidgetMap*                  get_W_map()
        { return m_W_map; }

    protected:
        // COMMON
        int                         set_diary_view_cur( const std::string& );

        // CALENDAR VIEW
        void                        handle_calendar_day_selected( DateV );

        // SEARCH VIEW
        void                        handle_search_text();
        void                        handle_search_case_sensitivity();
        void                        handle_search_filter( const Filter* );
        void                        start_search_thread();
        void                        handle_search_thread_notification();

        // FILTER VIEW
        void                        reflect_active_filter();
        void                        apply_filter();

        // THEME VIEW
        Gtk::FlowBoxChild*          get_selected_FBx_theme_child();
        Theme*                      get_theme_from_FBxChild( Gtk::FlowBoxChild* ); 
        Theme*                      get_selected_theme();
        void                        reset_theme_to_system();
        void                        duplicate_theme();
        void                        dismiss_theme();

        // COMMON OBJECTS
        Gtk::Box*                   m_Bx_panel;
        Gtk::Stack*                 m_St_view;

        // CALENDAR VIEW
        WidgetCalendar*             m_W_calendar;

        // SEARCH VIEW
        Gtk::SearchEntry*           m_E_search;
        Gtk::ToggleButton*          m_TB_search_match_case;
        WidgetFilterPicker*         m_WFP_search_filter;
        Gtk::Box*                   m_Bx_replace;
        // for some reason Glade cannot save a Listbox in a ScrolledWindow
        EntryClear*                 m_E_replace;
        Gtk::Button*                m_B_replace_all;
        TextviewDiarySearch*        m_TvD_search;

        int                         m_match_count{ 0 };
        int                         m_index_match_cur{ -1 };
        bool                        m_F_restart_search_thread{ false };

        // CHART VIEW
        WidgetPicker< ChartElem >*  m_WP_chart;
        WidgetChart*                m_W_chart;
        Gtk::Button*                m_B_chart_new;

        // TABLE VIEW
        WidgetPicker< TableElem >*  m_WP_table;
        WidgetTable*                m_W_table;
        Gtk::Box*                   m_Bx_table_new;
        Gtk::Button*                m_B_table_new;

        // THEME VIEW
        Theme*                      m_p2theme2dismiss   { nullptr };

        Glib::RefPtr< Gio::SimpleActionGroup >
                                    m_AG;
        R2Action                    m_A_theme_assign_cur;
        R2Action                    m_A_theme_assign_sel;
        R2Action                    m_A_theme_edit;
        R2Action                    m_A_theme_duplicate;
        R2Action                    m_A_theme_reset;
        R2Action                    m_A_theme_dismiss;

        Gtk::Stack*                 m_St_themes;
        Gtk::FlowBox*               m_FBx_themes;
        TextviewDiaryTheme*         m_p2TvD_theme_prvw  { nullptr };
        TextviewDiaryTheme*         m_TVD_theme_edit;
        Gtk::Popover*               m_Po_theme          { nullptr };
        Gtk::Entry*                 m_E_theme_name;
        Glib::RefPtr< Gtk::DragSource >
                                    m_DrS_theme;

        // MAP VIEW
        Gtk::Box*                   m_Bx_map;
        WidgetMap*                  m_W_map;

    friend class Lifeograph;
};

} // end of namespace LIFEO

#endif

