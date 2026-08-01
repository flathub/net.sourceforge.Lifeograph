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


#ifndef LIFEOGRAPH_DIALOG_FILTER_HEADER
#define LIFEOGRAPH_DIALOG_FILTER_HEADER


#include <gtkmm.h>

#include "../widgets/widget_filter.hpp"


namespace LIFEO
{

class DialogFilter : public Gtk::Window
{
    public:
        DialogFilter( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >&, int, Filter* );

        void                        handle_filter_edited();
        void                        handle_filter_renamed();

        void                        update_main_list( const Ustring& );

        void                        apply();
        void                        revert();

        Filter*                     get_filter() const
        { return m_p2filter; }
        void                        set_filter( Filter* );

        LIFEO::Result               get_result() const { return m_result; }

    private:
        Gtk::Button*                m_B_apply;
        Gtk::Button*                m_B_revert;
        Gtk::Entry*                 m_E_name;

        WidgetFilter*               m_W_filter{ nullptr };
        Gtk::Viewport*              m_Vp_contents;

        Filter*                     m_p2filter;
        Ustring                     m_def_original;
        bool                        m_F_editing_main_filter;

        LIFEO::Result               m_result { Result::ABORTED };
};

} // end of namespace LIFEO

#endif
