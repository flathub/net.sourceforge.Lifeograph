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


#ifndef LIFEOGRAPH_DIALOG_SMART_ADD_HEADER
#define LIFEOGRAPH_DIALOG_SMART_ADD_HEADER


#include <gtkmm.h>

#include "../diaryelements/diary.hpp"
#include "../lifeograph.hpp"
#include "../widgets/widget_textviewpara.hpp"


namespace LIFEO
{


class DialogSmartAdd : public Gtk::Window
{
    public:
        enum class SuggestionType
        {
            AS_A_NEW_ENTRY, AS_A_NEW_PARA, APPEND_TO_TODAY, SET_PARENT_ENTRY, GO_UP
        };
        struct Suggestion
        {
            SuggestionType  type;
            Ustring         name;
            Ustring         contents;
            Entry*          p2entry_parent;
            Paragraph*      p2para_parent;
        };

        using VecSuggestions = std::vector< Suggestion >;

                                    DialogSmartAdd( BaseObjectType*,
                                                    const Glib::RefPtr< Gtk::Builder >& );

        static void                 create();
        static void                 disband();

        void                        clear_suggestions();
        void                        reset();
        void                        update_suggestions();
        void                        add_suggestion( SuggestionType,
                                                    const Ustring&,
                                                    const Ustring&,
                                                    const Ustring&,
                                                    Entry*,
                                                    Paragraph* );
        void                        add_suggestion( Entry*          p2ep )
        {
            add_suggestion( SuggestionType::SET_PARENT_ENTRY,
                            STR::compose( _( "Go to:" ), "   ", p2ep->get_title_ancestral() ),
                            p2ep->get_description(),
                            "",
                            p2ep,
                            nullptr );
        }
        void                        add_suggestion( SuggestionType  type,
                                                    const Ustring&  name,
                                                    Paragraph*      p2pp )
        { add_suggestion( type, name, "", "", nullptr, p2pp ); }
        void                        add_suggestion( SuggestionType  type,
                                                    const Ustring&  name,
                                                    const Ustring&  contents )
        { add_suggestion( type, name, "", contents, nullptr, nullptr ); }
        void                        add_suggestion( SuggestionType type, const Ustring& name )
        { add_suggestion( type, name, "", "", nullptr, nullptr ); }

    protected:
        bool                        on_key_press_event( guint, guint, Gdk::ModifierType );

        void                        apply_suggestion( const Suggestion& );
        void                        append_to_today();
        void                        add_as_a_new_entry();
        void                        add_as_a_new_para();

        bool                        has_selection() const
        { return( m_LB_suggestions->get_selected_row() != nullptr ); }
        Suggestion&                 get_selected()
        { return m_suggestions[ m_LB_suggestions->get_selected_row()->get_index() ]; }

        static DialogSmartAdd*      ptr;
        Gtk::Label*                 m_L_path;
        TextviewDiaryPara*          m_Tv_para;
        Gtk::ListBox*               m_LB_suggestions;

        VecSuggestions              m_suggestions;
        Paragraph*                  m_p2para;
        Entry*                      m_p2entry_parent    { nullptr };
        Paragraph*                  m_p2para_parent     { nullptr };
};

} // end of namespace LIFEO

#endif

