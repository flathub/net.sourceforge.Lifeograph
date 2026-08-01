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


#ifndef LIFEOGRAPH_DIALOG_PREFERENCES_HEADER
#define LIFEOGRAPH_DIALOG_PREFERENCES_HEADER


#include <gtkmm.h>

#include "../diaryelements/diary.hpp"
#include "../lifeograph.hpp"


namespace LIFEO
{


class DialogPreferences : public Gtk::Window
{
    public:
                                    DialogPreferences( BaseObjectType*,
                                                       const Glib::RefPtr< Gtk::Builder >& );

        static void                 create();

    protected:
        void                        update_date_format();
        void                        handle_close();

        static DialogPreferences*   ptr;
        Gtk::ComboBoxText*          m_CB_autologout;
        Gtk::Switch*                m_Sw_save_backups;
        FileChooserButton*          m_FCB_backup_path;
        Gtk::ComboBoxText*          m_CB_date_order;
        Gtk::ComboBoxText*          m_CB_1st_week_day;
        Gtk::ComboBoxText*          m_CB_date_separator;
        Gtk::ToggleButton*          m_RB_units_imperial;
        Gtk::Switch*                m_Sw_use_dark_theme;
};

} // end of namespace LIFEO

#endif

