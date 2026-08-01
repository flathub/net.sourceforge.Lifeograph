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


#ifndef LIFEOGRAPH_DIALOG_SYNC_HEADER
#define LIFEOGRAPH_DIALOG_SYNC_HEADER


#include <gtkmm.h>

#include "../diaryelements/diary.hpp"
#include "../widgets/widget_entrypicker.hpp"


namespace LIFEO
{

// CUSTOM TREESTORE
class TreeStoreImport : public Gtk::TreeStore
{
    protected:
        TreeStoreImport()
        { set_column_types( colrec ); }

    public:
        // HEADER TYPES
        constexpr static int HT_NONE           = 0;
        constexpr static int HT_THEMES         = 1;
        constexpr static int HT_FILTERS        = 2;
        constexpr static int HT_CHARTS         = 3;
        constexpr static int HT_CHAPTER_CTG    = 4;
        constexpr static int HT_TABLES         = 5;
        constexpr static int HT_ENTRIES        = 7;
        constexpr static int HT_DIARY          = 8;

        class Colrec : public Gtk::TreeModel::ColumnRecord
        {
            public:
                Colrec()
                {
                    add( icon );
                    add( name );
                    add( status );
                    add( action_combo );
                    add( action );
                    add( ptr );
                    add( header_type );
                    add( comp_res );
                }
                Gtk::TreeModelColumn< R2Pixbuf >                        icon;
                Gtk::TreeModelColumn< Ustring >                         name;
                Gtk::TreeModelColumn< Ustring >                         status;
                Gtk::TreeModelColumn< Glib::RefPtr< Gtk::TreeModel> >   action_combo;
                Gtk::TreeModelColumn< Ustring >                         action;
                Gtk::TreeModelColumn< DiaryElement* >                   ptr;
                Gtk::TreeModelColumn< int >                             header_type;
                Gtk::TreeModelColumn< SI >                              comp_res;
        };

        Colrec colrec;

        static Glib::RefPtr< TreeStoreImport > create()
        {
            return Glib::RefPtr< TreeStoreImport >( new TreeStoreImport() );
        }
};

// IMPORT DIALOG
class DialogSync : public Gtk::Window
{
    public:
        static constexpr int FILTER_SHOW_NEW            =       0x10;
        static constexpr int FILTER_SHOW_CHANGED        =       0x20;
        static constexpr int FILTER_SHOW_ACCEPTED       =   0x1'0000;
        static constexpr int FILTER_SHOW_IGNORED        =   0x2'0000;
        static constexpr int FILTER_ONLY_SHOW_DIFFERENT = 0x100'0000;
        static constexpr int FILTER_DEFAULT = FILTER_SHOW_NEW|FILTER_SHOW_CHANGED|
                                              FILTER_SHOW_ACCEPTED|FILTER_SHOW_IGNORED;

        class ColrecComp : public Gtk::TreeModel::ColumnRecord
        {
            public:
                ColrecComp()
                {
                    add( name );
                    add( str_l );
                    add( str_r );
                    add( result );
                }
                Gtk::TreeModelColumn< Ustring > name;
                Gtk::TreeModelColumn< Ustring > str_l;
                Gtk::TreeModelColumn< Ustring > str_r;
                Gtk::TreeModelColumn< SI >      result;
        };
        class ColRecCombo : public Gtk::TreeModel::ColumnRecord
        {
            public:
                ColRecCombo()
                { add( name ); }

                Gtk::TreeModelColumn< Ustring > name;
        };

        DialogSync( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );

        void                        on_show() override;
        void                        go_synchonize();

        void                        open_remote_diary( const String& );
        void                        open_remote_diary2();
        void                        open_remote_diary3();
        void                        ask_for_password();

        void                        populate_content();
        void                        set_ready();
        void                        set_not_ready( const Ustring& =
                                                   _( "Select a diary file for synchronizing" ) );

        bool                        is_ready();

    private:
        void                        update_maps();
        void                        show_details();
        void                        handle_TV_action_edited( const Ustring&,
                                                             const Ustring& );
        void                        set_cell_color_s( Gtk::CellRenderer*,
                                                      const Gtk::TreeModel::const_iterator& );
        void                        set_cell_color_a( Gtk::CellRenderer*,
                                                      const Gtk::TreeModel::const_iterator& );

        void                        set_cell_color_comp_l( Gtk::CellRenderer*,
                                                           const Gtk::TreeModel::const_iterator& );
        void                        set_cell_color_comp_r( Gtk::CellRenderer*,
                                                           const Gtk::TreeModel::const_iterator& );

        void                        toggle_filter( int, R2Action& );

        void                        change_action( SI );

        Gtk::Button*                m_B_go;
        Gtk::Button*                m_B_cancel;
        FileChooserButton*          m_FCB_diary;
        Gtk::InfoBar*               m_IB_sync;
        Gtk::Label*                 m_L_info;
        WidgetEntryPicker*          m_WEP_widget;
        Gtk::CheckButton*           m_RB_changed_ignore;
        Gtk::CheckButton*           m_RB_changed_ovrwrt;
        Gtk::CheckButton*           m_RB_changed_add;
        Gtk::CheckButton*           m_RB_new_ignore;
        Gtk::CheckButton*           m_RB_new_add;
        Gtk::Button*                m_B_apply_scheme;

        Gtk::Paned*                 m_P_contents;

        Gtk::TreeView*              m_TV_contents;
        Glib::RefPtr< TreeStoreImport >
                                    m_TS_content;

        ColRecCombo                 m_colrec_combo;
        Glib::RefPtr< Gtk::ListStore >
                                    m_LS_action_ow,
                                    m_LS_action_add,
                                    m_LS_action_add_ow;
        Gtk::TreeViewColumn*        m_TVC_status;
        Gtk::TreeViewColumn*        m_TVC_action;

        // Gtk::Popover*               m_Po_status;
        // Gtk::Popover*               m_Po_action;

        R2ActionGroup               m_AG;
        R2Action                    m_A_show_accepted;
        R2Action                    m_A_show_ignored;
        R2Action                    m_A_show_new;
        R2Action                    m_A_show_changed;
        R2Action                    m_A_only_show_different; // this is for details

        Gtk::Box*                   m_Bx_operations;
        Gtk::TreeView*              m_TV_compare;
        ColrecComp                  m_CR_compare;
        Glib::RefPtr< Gtk::ListStore >
                                    m_LS_compare;
        Gtk::Button*                m_B_unlock;
        Gtk::Button*                m_B_add;
        Gtk::Button*                m_B_ow;
        Gtk::Switch*                m_Sw_similar;

        Diary*                      m_diary_r;
        std::map< DEID, SI >        m_action_map;
        std::map< DEID, SI >        m_comparison_map;

        bool                        m_flag_diary_is_ready;
        bool                        m_F_identical;
        int                         m_content_filter;
};

} // end of namespace LIFEO

#endif
