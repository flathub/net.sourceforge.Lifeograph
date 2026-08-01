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


#ifndef LIFEOGRAPH_WIDGET_TEXTVIEW_DIARY_PARA_HEADER
#define LIFEOGRAPH_WIDGET_TEXTVIEW_DIARY_PARA_HEADER


#include "widget_textviewedit.hpp"


namespace LIFEO
{

// TEXTVIEWEDIT ====================================================================================
class TextviewDiaryPara : public TextviewDiaryEdit
{
    public:
        TextviewDiaryPara( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );
        // ~TextviewDiaryPara();

        bool                        is_the_main_editor() const override { return false; }

        void                        set_entry( Entry* ) = delete;
        void                        unset_entry() = delete;

        Paragraph*                  get_paragraph()
        { return m_p2paragraph; }
        // void                        set_paragraph( Paragraph* p2para )
        // { m_p2paragraph = p2para; }
        void                        clear()
        {
            m_p2paragraph->set_para_type_raw( VT::PS_PLAIN );
            m_r2buffer->erase( m_r2buffer->get_iter_at_offset( 0 ),
                               m_r2buffer->get_iter_at_offset( m_entry.get_size() ) );
        }

    protected:
        bool                        on_key_press_event( guint, guint, Gdk::ModifierType ) override;
        //bool                        on_key_release_event(  );

        Paragraph*                  m_p2paragraph;
        Entry                       m_entry;
};

} // end of namespace LIFEO

#endif
