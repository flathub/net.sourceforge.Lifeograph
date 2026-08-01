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


#ifndef LIFEOGRAPH_UI_LOGIN_HEADER
#define LIFEOGRAPH_UI_LOGIN_HEADER


#include "helpers.hpp"
#include "lifeograph.hpp"


namespace LIFEO
{

class UILogin
{
        enum DiarySortType{ DST_NAME, DST_SAVE, DST_READ };

    public:
        class Colrec : public Gtk::TreeModel::ColumnRecord
        {
            public:
                Colrec()
                {
                    add( icon );
                    add( name );
                    add( date );
                    add( tooltip );
                    add( uri );
                    add( date_sort );
                }
                Gtk::TreeModelColumn< Glib::RefPtr< Gdk::Pixbuf > >      icon;
                Gtk::TreeModelColumn< Ustring >     name;
                Gtk::TreeModelColumn< String >      date;
                Gtk::TreeModelColumn< Ustring >     tooltip;
                Gtk::TreeModelColumn< String >      uri;
                Gtk::TreeModelColumn< DateV >       date_sort;
        };
        static Colrec*              colrec;

                                    UILogin();

        void                        handle_start();
        void                        handle_login();
        void                        handle_logout();

        void                        populate_diaries();
        void                        refresh_sort_type();

        void                        undo_remove_selected_diary();

        static String               m_uri_cur;

    protected:
        void                        open_selected_diary( const int, bool = true );
        void                        remove_selected_diary();

        void                        create_new_diary();
        void                        add_existing_diary();

        void                        ask_what_to_do_with_lock();

        void                        handle_select_mode_toggled();

        void                        handle_diary_selection_changed();
        void                        handle_diary_activated( const Gtk::TreePath&,
                                                            Gtk::TreeView::Column* );
        void                        handle_button_release( int, double, double );
        void                        handle_key_release( guint, guint, Gdk::ModifierType );
        bool                        handle_diary_tooltip( int, int, bool,
                                                          const Glib::RefPtr<Gtk::Tooltip>& );

        bool                        handle_drop_file( const Glib::ValueBase&, double, double );

        void                        add_diary_to_list( const String&, bool = true );

        void                        show_sort_Po();

        void                        initialize();

        int                         sort_by_date( const Gtk::TreeModel::const_iterator&,
                                                  const Gtk::TreeModel::const_iterator& );
        int                         sort_by_name( const Gtk::TreeModel::const_iterator&,
                                                  const Gtk::TreeModel::const_iterator& );

        Gtk::Box*                   m_Bx_login;
        Gtk::Grid*                  m_G_welcome;

        Gtk::ToggleButton*          m_TB_select;
        Gtk::Button*                m_B_new;
        Gtk::Button*                m_B_open;
        Gtk::Button*                m_B_remove_diary;

        Gtk::Popover*               m_Po_sort;
        Gtk::ToggleButton*          m_RB_sort_by_name;
        Gtk::ToggleButton*          m_RB_sort_by_save;
        Gtk::ToggleButton*          m_RB_sort_by_read;

        Gtk::TreeView*              m_TV_diaries;
        Glib::RefPtr< Gtk::ListStore >
                                    m_LS_diaries;
        Glib::RefPtr< Gtk::TreeSelection >
                                    m_treesel_diaries;
        Gtk::TreeViewColumn*        m_TVC_date;

        String                      m_uri_removed;
        DiarySortType               m_sort_type{ DST_SAVE };
};

} // end of namespace LIFEO

#endif
