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


#include "widget_textviewpara.hpp"


using namespace LIFEO;


// TEXTVIEW ========================================================================================
TextviewDiaryPara::TextviewDiaryPara( BaseObjectType* cobject,
                                      const Glib::RefPtr< Gtk::Builder >& parent_builder )
:   TextviewDiaryEdit( cobject, parent_builder, 0 ), m_entry( Diary::d, Date::NOT_SET )
{
    m_p2paragraph = m_entry.add_paragraph_before( "", nullptr );
    m_p2paragraph->m_style |= VT::PS_REORDERED;
    TextviewDiaryEdit::set_entry( &m_entry );
    // m_completion->set_diary( Diary::d ); // needed as m_entry->m_p2diary is nullptr
}

// TextviewDiaryPara::~TextviewDiaryPara()
// {
//
// }

bool
TextviewDiaryPara::on_key_press_event( guint keyval, guint g, Gdk::ModifierType state )
{
    switch( int( state ) & CTRL_ALT_SHIFT_MASK )
    {
        case 0:
            if( m_completion && m_completion->is_on_display() &&
                m_completion->handle_key( keyval, false, false ) )
                return true;
            switch( keyval )
            {
                case GDK_KEY_Up:
                case GDK_KEY_Down:
                case GDK_KEY_Return:
                    return true;
            }
            break;
            break;
        case int( Gdk::ModifierType::SHIFT_MASK ):
            switch( keyval )
            {
                case GDK_KEY_BackSpace:
                    clear();
                    return true;
            }
            break;
        case int( Gdk::ModifierType::ALT_MASK ):
            switch( keyval )
            {
                // disable actions that will break the single paragraph nature:
                case GDK_KEY_x:
                case GDK_KEY_X:
                case GDK_KEY_2:
                    return true;
            }
            break;
        // case CTRL_SHIFT_MASK:
        //     switch( event->keyval )
        //     {
        //         case GDK_KEY_space: show_Po_context( -1 ); break;
        //     }
        //     break;
        default:
            break;
    }

    return TextviewDiaryEdit::on_key_press_event( keyval, g, state );
}

// bool
// TextviewDiaryEdit::on_key_release_event( GdkEventKey* event )
// {
//     if( event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R )
//         if( m_p2hflink_hovered && ( m_p2hflink_hovered->type & VT::HFT_F_LINK ) )
//             update_link();
// }
