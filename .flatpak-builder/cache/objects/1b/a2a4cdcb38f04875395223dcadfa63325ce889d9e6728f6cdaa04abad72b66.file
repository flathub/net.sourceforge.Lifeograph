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


#ifndef LIFEOGRAPH_WIDGET_FILTER_HEADER
#define LIFEOGRAPH_WIDGET_FILTER_HEADER


#include <gtkmm.h>

#include "../diaryelements/diarydata.hpp"
#include "../diaryelements/filter.hpp"
#include "src/helpers.hpp"
#include "src/widgets/widget_selector.hpp"
#include "widget_datepicker.hpp"
#include "widget_entrypicker.hpp"
#include "widget_picker.hpp"


namespace LIFEO
{

// FORWARD DECLARATIONS
class WidgetFilterPicker;
class WidgetFilter;

// UI FOR EDITING A FILTERER STACK =================================================================
template < class T >
class FiltererUI : public Gtk::Box
{
    public:
        FiltererUI( WidgetFilter* p2WF, T* p2filterer )
        :   Gtk::Box( Gtk::Orientation::HORIZONTAL, 8 ), m_p2WF( p2WF ),
            m_p2filterer( p2filterer )
        {
            if( p2filterer->m_p2container ) // if it is not the top container
            {
                m_B_remove = Gtk::manage( new Gtk::Button );
                m_TB_not = Gtk::manage( new Gtk::ToggleButton( "!" ) );

                m_B_remove->set_icon_name( "list-remove-symbolic" );
                m_B_remove->set_has_frame( false );

                m_TB_not->set_has_frame( false );
                m_TB_not->set_active( p2filterer->is_not() );

                append( *m_B_remove );
                append( *m_TB_not );

                // TODO: 3.1: somehow indicate that the filterer is inactive
                // ...if it does not match the filter widget's classes
            }
        }
        virtual ~FiltererUI() {}

        void                    toggle_not();

        WidgetFilter*           m_p2WF;
        T*                      m_p2filterer;
        Gtk::ToggleButton*      m_TB_not;
        Gtk::Button*            m_B_remove;

    protected:
};

class FiltererStatusUI : public FiltererUI< FiltererStatus >
{
    public:
        static Ustring          get_label() { return _( "Status" ); }

        FiltererStatusUI( WidgetFilter*, FiltererStatus* );

        void                    update_state();

    protected:
        Gtk::ToggleButton*      m_ChB_not_todo;
        Gtk::ToggleButton*      m_ChB_open;
        Gtk::ToggleButton*      m_ChB_progressed;
        Gtk::ToggleButton*      m_ChB_done;
        Gtk::ToggleButton*      m_ChB_canceled;
};

class FiltererSizeUI : public FiltererUI< FiltererSize >
{
    public:
        static Ustring          get_label() { return _( "Size" ); }

        FiltererSizeUI( WidgetFilter*, FiltererSize* );

    protected:
        Gtk::DropDown*          m_DD_type;
        EntryClear*             m_E_range_b;
        EntryClear*             m_E_range_e;
        Gtk::Label*             m_L_incl_b;
        Gtk::Label*             m_L_incl_e;
};

class FiltererFavoriteUI : public FiltererUI< FiltererFavorite >
{
    public:
        static Ustring          get_label() { return _( "Favorite" ); }

        FiltererFavoriteUI( WidgetFilter*, FiltererFavorite* );

    protected:
};

class FiltererTrashedUI : public FiltererUI< FiltererTrashed >
{
    public:
        static Ustring          get_label() { return _( "Trashed" ); }

        FiltererTrashedUI( WidgetFilter*, FiltererTrashed* );

    protected:
};

class FiltererUnitUI : public FiltererUI< FiltererUnit >
{
    public:
        static Ustring          get_label() { return _( "Has Unit" ); }

        FiltererUnitUI( WidgetFilter*, FiltererUnit* );

    protected:
        EntryClear*             m_E_unit;
};

class FiltererIsUI : public FiltererUI< FiltererIs >
{
    public:
        static Ustring          get_label() { return _( "Is Entry" ); }

        FiltererIsUI( WidgetFilter*, FiltererIs* );

    protected:
        WidgetEntryPicker*      m_W_entry_picker;
};

class FiltererDescendantOfUI : public FiltererUI< FiltererDescendantOf >
{
    public:
        static Ustring          get_label() { return _( "Descendant" ); }

        FiltererDescendantOfUI( WidgetFilter*, FiltererDescendantOf* );

    protected:
        WidgetEntryPicker*      m_W_entry_picker;
        Gtk::CheckButton*       m_ChB_or_itself;
};

class FiltererTaggedByUI : public FiltererUI< FiltererTaggedBy >
{
    public:
        static Ustring          get_label() { return _( "Has Tag" ); }

        FiltererTaggedByUI( WidgetFilter*, FiltererTaggedBy* );

    protected:
        WidgetEntryPicker*      m_W_entry_picker;
};

class FiltererSubtaggedByUI : public FiltererUI< FiltererSubtaggedBy >
{
    public:
        static Ustring          get_label() { return _( "Has SubTag" ); }

        FiltererSubtaggedByUI( WidgetFilter*, FiltererSubtaggedBy* );

    protected:
        WidgetEntryPicker*      m_WEP_tag_p;
        WidgetEntryPicker*      m_WEP_tag_c;
        Gtk::ComboBoxText*      m_CB_type;
        Gtk::DropDown*          m_DD_rel;
};

class FiltererTagValueUI : public FiltererUI< FiltererTagValue >
{
    public:
        static Ustring          get_label() { return _( "Tag Value" ); }

        FiltererTagValueUI( WidgetFilter*, FiltererTagValue* );

    protected:
        WidgetEntryPicker*      m_W_entry_picker;
        Gtk::ComboBoxText*      m_CB_type;
        EntryClear*             m_E_range_b;
        EntryClear*             m_E_range_e;
        Gtk::Label*             m_L_incl_b;
        Gtk::Label*             m_L_incl_e;
};

class FiltererThemeUI : public FiltererUI< FiltererTheme >
{
    public:
        static Ustring          get_label() { return _( "Has Theme" ); }

        FiltererThemeUI( WidgetFilter*, FiltererTheme* );

    protected:
        WidgetPicker< Theme >*  m_W_theme_picker;
};

class FiltererBetweenDatesUI : public FiltererUI< FiltererBetweenDates >
{
    public:
        static Ustring          get_label() { return _( "Between Dates" ); }

        FiltererBetweenDatesUI( WidgetFilter*, FiltererBetweenDates* );

    protected:
        WidgetDatePicker*       m_W_date_picker_b;
        WidgetDatePicker*       m_W_date_picker_e;
        Gtk::Label*             m_L_incl_b;
        Gtk::Label*             m_L_incl_e;
};

class FiltererBetweenEntriesUI : public FiltererUI< FiltererBetweenEntries >
{
    public:
        static Ustring          get_label() { return _( "Between Entries" ); }

        FiltererBetweenEntriesUI( WidgetFilter*, FiltererBetweenEntries* );

    protected:
        WidgetEntryPicker*      m_WEP_bgn;
        WidgetEntryPicker*      m_WEP_end;
        Gtk::Label*             m_L_incl_b;
        Gtk::Label*             m_L_incl_e;
};

class FiltererCompletionUI : public FiltererUI< FiltererCompletion >
{
    public:
        static Ustring          get_label() { return _( "Completion" ); }

        FiltererCompletionUI( WidgetFilter*, FiltererCompletion* );

    protected:
        Gtk::Scale*             m_Sc_bgn;
        Gtk::Scale*             m_Sc_end;
};

class FiltererContainsTextUI : public FiltererUI< FiltererContainsText >
{
    public:
        static Ustring          get_label() { return _( "Contains Text" ); }

        FiltererContainsTextUI( WidgetFilter*, FiltererContainsText* );

    protected:
        EntryClear*             m_E_text;
        Gtk::ToggleButton*      m_TB_match_case;
        Gtk::ComboBoxText*      m_CB_extent;
};

class FiltererChildFilterUI : public FiltererUI< FiltererChildFilter >
{
    public:
        static Ustring          get_label() { return _( "Child Filter" ); }

        FiltererChildFilterUI( WidgetFilter*, FiltererChildFilter* );

    protected:
        WidgetFilterPicker*     m_WFP_filter;
};

class FiltererIsImageUI : public FiltererUI< FiltererIsImage >
{
    public:
        static Ustring          get_label() { return _( "Is Image" ); }

        FiltererIsImageUI( WidgetFilter*, FiltererIsImage* );

    protected:
};

class FiltererHasCoordsUI : public FiltererUI< FiltererHasCoords >
{
    public:
        static Ustring          get_label() { return _( "Has Coordinates" ); }

        FiltererHasCoordsUI( WidgetFilter*, FiltererHasCoords* );

    protected:
};

class FiltererTitleStyleUI : public FiltererUI< FiltererTitleStyle >
{
    public:
        static Ustring          get_label() { return _( "Title Style" ); }

        FiltererTitleStyleUI( WidgetFilter*, FiltererTitleStyle* );

    protected:
        Gtk::DropDown*          m_DD_title_style;
};

class FiltererHeadingLevelUI : public FiltererUI< FiltererHeadingLevel >
{
    public:
        static Ustring          get_label() { return _( "Heading Level" ); }

        FiltererHeadingLevelUI( WidgetFilter*, FiltererHeadingLevel* );

    protected:
        void                    handle_change( int, bool );

        Gtk::ToggleButton*      m_TB_H0;
        Gtk::ToggleButton*      m_TB_H1;
        Gtk::ToggleButton*      m_TB_H2;
        Gtk::ToggleButton*      m_TB_H3;
};

// BASIC FILTERERS =================================================================================
class FiltererEqualsUI : public FiltererUI< FiltererEquals >
{
    public:
        static Ustring          get_label() { return _( "Equals" ); }

        FiltererEqualsUI( WidgetFilter*, FiltererEquals* );

    protected:
        Gtk::DropDown*          m_DD_relation;
        Gtk::Entry*             m_E_value;
};

// BASIC FILTERERS =================================================================================
class FiltererContainerUI : public FiltererUI< FiltererContainer >
{
    public:
        static Ustring          get_label() { return _( "Subgroup" ); }

        FiltererContainerUI( WidgetFilter*, FiltererContainer* );
        ~FiltererContainerUI() { delete m_p2filterer; }

        void                    clear_pipeline();

        void                    update_logic_label();
        void                    update_add_menu();

        template< class TU, class TB >
        TU*                     add_filterer( TB* );

        void                    remove_filterer( FiltererUI* );

    protected:
        void                    set_from_stack();

        template< class TU, class TB >
        void                    add_filterer_to_menu();

        Gtk::Box*               m_Bx_header;
        Gtk::Box*               m_Bx_contents;
        Gtk::Label*             m_L_logic;
        WidgetSelector*         m_WS_add;

        std::set< FiltererContainerUI* >    m_subgroups;
};

// WIDGET FILTER ===================================================================================
class WidgetFilter : public FiltererContainerUI
{
    public:
        WidgetFilter( Diary* p2diary, const Ustring& def, int objects )
        :   FiltererContainerUI( this, new FiltererContainer( p2diary, def ) ),
            m_target_obj_classes( objects )
        {
            set_margin( 5 );
            // we moved this here from FiltererContainerUI constructor as m_target_obj_classes...
            // ...is not ready at the moment when FiltererContainerUI is being constructed:
            update_add_menu();
            for( auto sub : m_subgroups )
               sub->update_add_menu();
        }

        void                    set_from_string( const Ustring& str )
        {
            m_F_emit_signals = false;
            m_p2filterer->set_from_string( str );
            set_from_stack();
            m_F_emit_signals = true;
        }

        void                    get_as_string( Ustring& def )
        { return m_p2filterer->get_as_string( def ); }

        void                    toggle_logic();

        void                    set_enabled( bool );

        void                    emit_changed();

        SignalVoid              signal_changed()
        { return m_Sg_changed; }

        int                     get_target_obj_classes()
        { return m_target_obj_classes; }

        // void                    set_target_obj_classes( int objcls )
        // {
        //     m_target_obj_classes = objcls;
        //     update_add_menu();
        // }

    protected:
        int                     m_target_obj_classes;
        // this flag is used to disable signals during internal updates
        bool                    m_F_emit_signals  { true };
        SignalVoid              m_Sg_changed;
};

// LATE DEFINITIONS OF SOME MEMBERS ================================================================
template< class T >
void
FiltererUI< T >::toggle_not()
{
    m_p2filterer->toggle_not();
    m_p2WF->emit_changed();
}

template< class TU, class TB >
void
FiltererContainerUI::add_filterer_to_menu()
{
    // if( !( T::m_obj_classes & m_obj_classes ) ) return;
    // NOTE:do not restrict, allow interchangability

    // denote that it will be unused in the current context:
    const Ustring label
    { ( TB::m_obj_classes & m_p2WF->get_target_obj_classes() ) ? TU::get_label()
                                                               : "*" + TU::get_label( ) };

    m_WS_add->insert_elem( label,
            [ this ]( int )
            {
                TU* filterer_ui { add_filterer< TU, TB >( m_p2filterer->add< TB >() ) };

                m_p2WF->emit_changed();

                if( typeid( TU ) == typeid( FiltererContainerUI ) )
                {
                    FiltererContainerUI* filterer_ctr_ui { ( FiltererContainerUI* ) filterer_ui };
                    filterer_ctr_ui->update_add_menu();
                }
            } );
}

template< class TU, class TB >
TU*
FiltererContainerUI::add_filterer( TB* filterer )
{
    TU* filterer_ui { Gtk::manage( new TU( m_p2WF, filterer ) ) };

    filterer_ui->insert_before( *m_Bx_contents, *m_WS_add );

    if( typeid( TU ) == typeid( FiltererContainerUI ) )
        m_subgroups.insert( ( FiltererContainerUI* ) filterer_ui );

    filterer_ui->m_B_remove->signal_clicked().connect(
            sigc::bind( sigc::mem_fun( *this, &FiltererContainerUI::remove_filterer ),
                        ( FiltererUI* ) filterer_ui ) );
    filterer_ui->m_TB_not->signal_toggled().connect(
            sigc::mem_fun( * ( FiltererUI* ) filterer_ui, &FiltererUI::toggle_not ) );

    return filterer_ui;
}

} // end of namespace LIFEO

#endif
