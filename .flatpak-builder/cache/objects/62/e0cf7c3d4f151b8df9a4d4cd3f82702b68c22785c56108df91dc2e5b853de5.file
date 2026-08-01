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


#ifndef LIFEOGRAPH_DIALOG_GETTEXT_HEADER
#define LIFEOGRAPH_DIALOG_GETTEXT_HEADER


#include <gtkmm/popover.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>

#include "../helpers.hpp"


namespace LIFEO
{

using namespace HELPERS;

// PASSWORD DIALOG =================================================================================
class DialogGetText : public Gtk::Popover
{
    public:
        DialogGetText( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );

        static void                 launch( const Gdk::Rectangle*,
                                            Gtk::Widget*,
                                            const FuncVoidUstring&,
                                            const FuncVoid&,
                                            const Ustring& = "" );

    protected:
        void                        launch_internal( const Gdk::Rectangle*,
                                                     Gtk::Widget*,
                                                     const FuncVoidUstring&,
                                                     const FuncVoid&,
                                                     const Ustring& );
        void                        handle_go();

        Gtk::Button*                m_B_go;
        Gtk::Button*                m_B_cancel;
        EntryClear*                 m_E_text;
        //Gtk::Label*                 m_L_msg;

        FuncVoidUstring             m_handler_go;
        FuncVoid                    m_handler_cancel;

        static DialogGetText*       ptr;
};

} // end of namespace LIFEO

#endif
