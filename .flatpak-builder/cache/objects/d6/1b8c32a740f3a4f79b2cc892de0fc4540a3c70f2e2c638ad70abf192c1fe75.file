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


#ifndef LIFEOGRAPH_WIDGETENTRYPICKER_HEADER
#define LIFEOGRAPH_WIDGETENTRYPICKER_HEADER


#include <gtkmm.h>

#include "../diaryelements/diary.hpp"


namespace LIFEO
{

// TREESTORE FOR DIARY ELEMENT LISTS (NOT USED ANY MORE) ===========================================
/*class TreeStoreDiaryElem : public Gtk::TreeStore
{
    protected:
        TreeStoreDiaryElem();

    public:
        class Colrec : public Gtk::TreeModel::ColumnRecord
        {
            public:
                Colrec()
                {
                    add( icon );
                    add( name );
                    add( ptr );
                }
                Gtk::TreeModelColumn< R2Pixbuf >        icon;
                Gtk::TreeModelColumn< Ustring >         name;
                Gtk::TreeModelColumn< DiaryElement* >   ptr;
        };

        Colrec colrec;

        static Glib::RefPtr< TreeStoreDiaryElem > create();
};*/

class WidgetEntryPicker; // forward declaration

// COMPLETION POPOVER ==============================================================================
class EntryPickerCompletion
{
    public:
                                    EntryPickerCompletion( Gtk::Widget& );
                                    ~EntryPickerCompletion();

        void                        set_diary( Diary* diary );
        Diary*                      get_diary() const
        { return m_p2diary; }

        void                        add_item( Entry* );
        void                        add_new_item();
        void                        clear();

        void                        popup( const Ustring& );
        void                        popup( const Gdk::Rectangle&, const Ustring& );
        void                        popdown();
        bool                        is_on_display() const
        { return m_Po->get_visible(); }
        void                        update_list( const Ustring& );

        bool                        handle_key( guint, bool, bool );

        SignalVoidEntry             signal_entry_activated()
        { return m_Sg_entry_activated; }

        Entry*                      get_selected() const;

        static bool                 s_F_offer_new;

    protected:
        void                        create_new_entry();

        Glib::RefPtr<Gtk::Builder>  m_builder;
        Gtk::Popover*               m_Po{ nullptr };
        Gtk::ListBox*               m_LB;
        Gtk::Box*                   m_Bx_new_under{ nullptr };
        WidgetEntryPicker*          m_WEP_new_under{ nullptr };
        const Gtk::Widget&          m_W_parent;

        VecEntries                  m_items;
        Ustring                     m_filter;

        SignalVoidEntry             m_Sg_entry_activated;

        Diary*                      m_p2diary{ nullptr };
        bool                        m_F_offer_new;
};

// WIDGET FOR ENTERING ENTRY NAMES WITH AUTOCOMPLETION =============================================
class WidgetEntryPicker : public EntryClear
{
    public:
        WidgetEntryPicker( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );
        WidgetEntryPicker( bool, const std::string& = "" );

        void                        set_diary( Diary* diary )
        { m_completion->set_diary( diary ); }

        void                        set_text( const Ustring& text ) = delete;

        void                        set_entry( LIFEO::Entry* );
        LIFEO::Entry*               get_entry()
        { return m_p2entry; }
        DEID                        get_entry_id()
        { return( m_p2entry ? m_p2entry->get_id() : DEID_UNSET ); }
        LIFEO::Entry*               get_entry_create( Diary* );

        SignalVoidEntry             signal_updated()
        { return m_Sg_updated; }

    protected:
        void                        initialize();

        void                        on_changed() override;
        bool                        on_key_press_event( guint, guint, Gdk::ModifierType );
        Gdk::DragAction             handle_drop_motion( double, double );
        bool                        handle_drop( const Glib::ValueBase&, double, double );

        void                        handle_entry_selected( LIFEO::Entry* );

        LIFEO::Entry*               m_p2entry     { nullptr };

        EntryPickerCompletion*      m_completion  { nullptr };

        SignalVoidEntry             m_Sg_updated;

        Glib::RefPtr< Gtk::DropTarget >
                                    m_drop_target;
};

// SIMPLE POPOVER FOR SELECTING AN EENTRY OR A PARAGRAPH FROM A GROUP
class PoElemChooser
{
    public:
        static void
        show( const SetDiaryElemsByName&,
              std::function< void( LIFEO::DiaryElement* ) >&&,
              Gtk::Widget&,
              const Gdk::Rectangle& );

    private:
        static Glib::RefPtr< Gio::ListModel >
        create_model( const Glib::RefPtr< Glib::ObjectBase >& = {} );

        static VecDiaryElems m_elems;
        static bool m_F_para_mode;
        static Gtk::Popover* m_Po;
        static FuncVoidElem handle_activate;
};


} // end of namespace LIFEO

#endif
