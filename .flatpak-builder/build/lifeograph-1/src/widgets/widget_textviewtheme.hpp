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


#ifndef LIFEOGRAPH_TEXTVIEW_DIARY_THEME_HEADER
#define LIFEOGRAPH_TEXTVIEW_DIARY_THEME_HEADER


#include "gtkmm/colordialog.h"
#include "gtkmm/fontdialog.h"
#include "widget_textview.hpp"


namespace LIFEO
{

// TEXTVIEW ========================================================================================
class TextviewDiaryTheme : public TextviewDiary
{
    public:
        TextviewDiaryTheme();
        TextviewDiaryTheme( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );

        Paragraph*                  add_paragraph( const Ustring&, int );
        void                        set_text_preview();
        void                        set_text_edit();

        SignalVoid                  signal_theme_edited()
        { return m_Sg_theme_edited; }
        void                        handle_theme_edited() // called from theme edit links
        { m_Sg_theme_edited.emit(); }

    protected:
        void                        init();

        void                        handle_link_theme();
        void                        process_link_uri( const Paragraph*, int ) override;

        void                        on_motion_notify_event( double x, double y ) override
        {
            if( m_tag_link_theme->property_invisible() )
                m_tag_link_theme->property_invisible() = false;
            TextviewDiary::on_motion_notify_event( x, y );
        }
        void                        on_leave_notify_event()
        {
            m_tag_link_theme->property_invisible() = true;
        }
        //void                        on_draw( const Cairo::RefPtr< Cairo::Context >&, int, int );

        bool                        handle_query_tooltip(
                                        int, int, bool,
                                        const Glib::RefPtr< Gtk::Tooltip >& ) override;

        // bool                        m_F_in_edit_mode{ false };

        SignalVoid                  m_Sg_theme_edited;

    // MIGRATED FROM TextbufferDiaryTheme
    protected:
        // PARSING
        void                        process_paragraph( Paragraph*,
                                                       const Gtk::TextIter&,
                                                       const Gtk::TextIter&,
                                                       bool = true ) override;

        void                        launch_color_dialog( Color* );
        void                        launch_font_dialog( Pango::FontDescription* );
        void                        launch_file_dialog( String* );

        void                        handle_color_result( const Glib::RefPtr< Gio::AsyncResult > &,
                                                         const Glib::RefPtr< Gtk::ColorDialog >&,
                                                         Color* );
        void                        handle_font_result( const Glib::RefPtr< Gio::AsyncResult >&,
                                                        const Glib::RefPtr< Gtk::FontDialog >&,
                                                        Pango::FontDescription* );
        void                        handle_file_result( Glib::RefPtr< Gio::AsyncResult >& ,
                                                        const Glib::RefPtr< Gtk::FileDialog >&,
                                                        String* );

        Glib::RefPtr< Gtk::TextBuffer::Tag >
                                    m_tag_link_theme;

        Entry                       m_entry;
};

} // end of namespace LIFEO

#endif
