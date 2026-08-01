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


#include "../lifeograph.hpp"
#include "widget_filter.hpp"
#include "src/diaryelements/filter.hpp"
#include "src/helpers.hpp"
#include "widget_filterpicker.hpp"


using namespace LIFEO;

// FILTERER ========================================================================================

// FILTERER STATUS =================================================================================
FiltererStatusUI::FiltererStatusUI( WidgetFilter* ctr, FiltererStatus* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_text{ Gtk::manage( new Gtk::Label( _( "Whose status is one of" ) ) ) };

    auto Bx_status{ Gtk::manage( new Gtk::Box( Gtk::Orientation::HORIZONTAL, 0 ) ) };
    m_ChB_not_todo = Gtk::manage( new Gtk::ToggleButton );
    m_ChB_open = Gtk::manage( new Gtk::ToggleButton );
    m_ChB_progressed = Gtk::manage( new Gtk::ToggleButton );
    m_ChB_done = Gtk::manage( new Gtk::ToggleButton );
    m_ChB_canceled = Gtk::manage( new Gtk::ToggleButton );

    auto I_not_todo{ Gtk::manage( new Gtk::Image ) };
    auto I_open{ Gtk::manage( new Gtk::Image ) };
    auto I_progressed{ Gtk::manage( new Gtk::Image ) };
    auto I_done{ Gtk::manage( new Gtk::Image ) };
    auto I_canceled{ Gtk::manage( new Gtk::Image ) };

    I_not_todo->set_from_icon_name( "entry-16" );
    I_open->set_from_icon_name( "todo_open-16" );
    I_progressed->set_from_icon_name( "todo_progressed-16" );
    I_done->set_from_icon_name( "todo_done-16" );
    I_canceled->set_from_icon_name( "todo_canceled-16" );

    m_ChB_not_todo->set_child( *I_not_todo );
    m_ChB_open->set_child( *I_open );
    m_ChB_progressed->set_child( *I_progressed );
    m_ChB_done->set_child( *I_done );
    m_ChB_canceled->set_child( *I_canceled );

    Bx_status->add_css_class( "linked" );
    Bx_status->append( *m_ChB_not_todo );
    Bx_status->append( *m_ChB_open );
    Bx_status->append( *m_ChB_progressed );
    Bx_status->append( *m_ChB_done );
    Bx_status->append( *m_ChB_canceled );

    m_ChB_not_todo->set_active(   m_p2filterer->m_included_statuses & ES::SHOW_NOT_TODO );
    m_ChB_open->set_active(       m_p2filterer->m_included_statuses & ES::SHOW_TODO );
    m_ChB_progressed->set_active( m_p2filterer->m_included_statuses & ES::SHOW_PROGRESSED );
    m_ChB_done->set_active(       m_p2filterer->m_included_statuses & ES::SHOW_DONE );
    m_ChB_canceled->set_active(   m_p2filterer->m_included_statuses & ES::SHOW_CANCELED );

    m_ChB_not_todo->signal_toggled().connect( [ this ](){ update_state(); } );
    m_ChB_open->signal_toggled().connect( [ this ](){ update_state(); } );
    m_ChB_progressed->signal_toggled().connect( [ this ](){ update_state(); } );
    m_ChB_done->signal_toggled().connect( [ this ](){ update_state(); } );
    m_ChB_canceled->signal_toggled().connect( [ this ](){ update_state(); } );

    append( *L_text );
    append( *Bx_status );
}

void
FiltererStatusUI::update_state()
{
    m_p2filterer->m_included_statuses = 0;

    if( m_ChB_not_todo->get_active() )
        m_p2filterer->m_included_statuses |= ES::SHOW_NOT_TODO;
    if( m_ChB_open->get_active() )
        m_p2filterer->m_included_statuses |= ES::SHOW_TODO;
    if( m_ChB_progressed->get_active() )
        m_p2filterer->m_included_statuses |= ES::SHOW_PROGRESSED;
    if( m_ChB_done->get_active() )
        m_p2filterer->m_included_statuses |= ES::SHOW_DONE;
    if( m_ChB_canceled->get_active() )
        m_p2filterer->m_included_statuses |= ES::SHOW_CANCELED;

    m_p2WF->emit_changed();
}

// FILTERER SIZE ===================================================================================
FiltererSizeUI::FiltererSizeUI( WidgetFilter* ctr, FiltererSize* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*       Bx_range;
    Gtk::Button*    B_incl_b;
    Gtk::Button*    B_incl_e;
    auto&&          builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_range.ui" ) };
    const VecUstrings
                    types   { VT::SO::CHAR_COUNT::S, VT::SO::PARA_COUNT::S };


    m_DD_type   = Gtk::make_managed< Gtk::DropDown >( types );
    Bx_range    = builder->get_widget< Gtk::Box >( "Bx_range" );
    B_incl_b    = builder->get_widget< Gtk::Button >( "B_incl_b" );
    B_incl_e    = builder->get_widget< Gtk::Button >( "B_incl_e" );
    m_L_incl_b  = builder->get_widget< Gtk::Label >( "L_incl_b" );
    m_L_incl_e  = builder->get_widget< Gtk::Label >( "L_incl_e" );
    m_E_range_b = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_bgn" );
    m_E_range_e = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_end" );

    m_DD_type->set_selected( VT::get_v< VT::SO, unsigned, char >( m_p2filterer->m_type ) );

    m_DD_type->property_selected().signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_type = VT::get_v< VT::ETS,
                                    char,
                                    unsigned >( m_DD_type->get_selected() );
                m_p2WF->emit_changed();
            } );

    m_E_range_b->set_placeholder_text( "--" );
    m_E_range_b->set_width_chars( 10 );
    if( m_p2filterer->m_range_b != -1 )
        m_E_range_b->set_text( STR::format_number( m_p2filterer->m_range_b ) );
    m_E_range_b->signal_changed().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ) return;
                const auto& text{ m_E_range_b->get_text() };
                m_p2filterer->m_range_b = text.empty() ? -1 : STR::get_d( text );
                m_p2WF->emit_changed();
            } );

    m_E_range_e->set_placeholder_text( "--" );
    m_E_range_e->set_width_chars( 10 );
    if( m_p2filterer->m_range_e != -1 )
        m_E_range_e->set_text( STR::format_number( m_p2filterer->m_range_e ) );
    m_E_range_e->signal_changed().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ) return;
                const auto& text { m_E_range_e->get_text() };
                m_p2filterer->m_range_e = text.empty() ? -1 : STR::get_d( text );
                m_p2WF->emit_changed();
            } );

    m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
    B_incl_b->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_b = !m_p2filterer->m_F_incl_b;
                m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
                m_p2WF->emit_changed();
            } );

    m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
    B_incl_e->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_e = !m_p2filterer->m_F_incl_e;
                m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
                m_p2WF->emit_changed();
            } );

    append( *m_DD_type );
    append( *Gtk::manage( new Gtk::Label( "∈" ) ) );
    append( *Bx_range );
}

// FILTERER FAVORITE ===============================================================================
FiltererFavoriteUI::FiltererFavoriteUI( WidgetFilter* ctr, FiltererFavorite* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name{ Gtk::manage( new Gtk::Label( _( "Is favorite" ) ) ) };

    append( *L_name );
    //L_name->insert_before( *this, *m_TB_not ); maybe later
}

// FILTERER TRASHED ================================================================================
FiltererTrashedUI::FiltererTrashedUI( WidgetFilter* ctr, FiltererTrashed* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name{ Gtk::manage( new Gtk::Label( _( "Is trashed" ) ) ) };

    append( *L_name );
}

// FILTERER UNIT ===================================================================================
FiltererUnitUI::FiltererUnitUI( WidgetFilter* ctr, FiltererUnit* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name{ Gtk::manage( new Gtk::Label( _( "Has the unit" ) ) ) };
    m_E_unit =  Gtk::manage( new EntryClear );

    m_E_unit->set_text( m_p2filterer->m_unit );
    m_E_unit->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_unit = m_E_unit->get_text();
                m_p2WF->emit_changed();
            } );

    append( *L_name );
    append( *m_E_unit );
}

// FILTERER IS NOT =================================================================================
FiltererIsUI::FiltererIsUI( WidgetFilter* ctr, FiltererIs* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto          L_name  { Gtk::manage( new Gtk::Label( _( "Is entry" ) ) ) };
    Diary* const  p2diary { m_p2filterer->m_p2container->m_p2diary };

    m_W_entry_picker = Gtk::manage( new WidgetEntryPicker( false ) );

    m_W_entry_picker->set_diary( p2diary );
    m_W_entry_picker->set_entry( p2diary->get_entry_by_id( m_p2filterer->m_id ) );
    m_W_entry_picker->signal_updated().connect(
            [ this ]( Entry* e ){
                m_p2filterer->m_id = m_W_entry_picker->get_entry_id();
                m_p2WF->emit_changed(); } );

    append( *L_name );
    append( *m_W_entry_picker );
}

// FILTERER DESCENDANT OF ==========================================================================
FiltererDescendantOfUI::FiltererDescendantOfUI( WidgetFilter* ctr,
                                                FiltererDescendantOf* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name { Gtk::manage( new Gtk::Label( _( "Is descendant of" ) ) ) };

    m_W_entry_picker = Gtk::manage( new WidgetEntryPicker( false ) );
    m_ChB_or_itself = Gtk::manage( new Gtk::CheckButton( _( "Or itself" ) ) );

    m_W_entry_picker->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_W_entry_picker->set_entry( m_p2filterer->m_entry_p );
    m_W_entry_picker->signal_updated().connect(
            [ this ]( Entry* e )
            {
                m_p2filterer->m_entry_p = m_W_entry_picker->get_entry();
                m_p2WF->emit_changed();
            } );

    m_ChB_or_itself->set_active( m_p2filterer->m_F_or_itself );
    m_ChB_or_itself->signal_toggled().connect(
            [ this ](){
                m_p2filterer->m_F_or_itself = m_ChB_or_itself->get_active();
                m_p2WF->emit_changed(); } );

    append( *L_name );
    append( *m_W_entry_picker );
    append( *m_ChB_or_itself );
}

// FILTERER TAGGED BY ==============================================================================
FiltererTaggedByUI::FiltererTaggedByUI( WidgetFilter* ctr, FiltererTaggedBy* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name{ Gtk::manage( new Gtk::Label( _( "Has the tag" ) ) ) };
    m_W_entry_picker = Gtk::manage( new WidgetEntryPicker( false, "tag-16-symbolic" ) );

    m_W_entry_picker->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_W_entry_picker->set_entry( m_p2filterer->m_tag );
    m_W_entry_picker->signal_updated().connect(
            [ this ]( Entry* e )
            {
                m_p2filterer->m_tag = m_W_entry_picker->get_entry();
                m_p2WF->emit_changed();
            } );

    append( *L_name );
    append( *m_W_entry_picker );
}

// FILTERER SUBTAGGED BY ===========================================================================
FiltererSubtaggedByUI::FiltererSubtaggedByUI( WidgetFilter* ctr, FiltererSubtaggedBy* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*   Bx_main;
    auto&&      builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_subtag.ui" ) };

    Bx_main     = builder->get_widget< Gtk::Box >( "Bx_main" );
    m_CB_type   = builder->get_widget< Gtk::ComboBoxText >( "CB_type" );
    m_DD_rel    = builder->get_widget< Gtk::DropDown >( "DD_rel" );
    m_WEP_tag_p = Gtk::Builder::get_widget_derived< WidgetEntryPicker >( builder, "E_tag_parent" );
    m_WEP_tag_c = Gtk::Builder::get_widget_derived< WidgetEntryPicker >( builder, "E_tag_child" );

    m_DD_rel->set_model( Gtk::StringList::create( { "<", "=", ">" } ) );
    m_DD_rel->set_selected( STR::get_pos_c( VT::AR_CHARS, m_p2filterer->m_relation, 1 ) );
    // 1 designates '=' as the default value
    m_DD_rel->property_selected().signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_relation = VT::AR_CHARS[ m_DD_rel->get_selected() ];
                m_p2WF->emit_changed();
            } );

    m_CB_type->set_active( m_p2filterer->m_type & VT::SEQ_FILTER );
    m_CB_type->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_type = m_CB_type->get_active_row_number();
                m_p2WF->emit_changed();
            } );

    m_WEP_tag_p->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_WEP_tag_p->set_entry( m_p2filterer->m_tag_parent );
    m_WEP_tag_p->signal_updated().connect(
            [ this ]( Entry* e )
            {
                m_p2filterer->m_tag_parent = e; //m_WEP_tag_p->get_entry();
                m_p2WF->emit_changed();
            } );

    m_WEP_tag_c->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_WEP_tag_c->set_entry( m_p2filterer->m_tag_child );
    m_WEP_tag_c->signal_updated().connect(
            [ this ]( Entry* e )
            {
                m_p2filterer->m_tag_child = e;
                m_p2WF->emit_changed();
            } );

    append( *Bx_main );
}

// FILTERER TAG VALUE ==============================================================================
FiltererTagValueUI::FiltererTagValueUI( WidgetFilter* ctr, FiltererTagValue* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*    Bx_range;
    Gtk::Button* B_incl_b;
    Gtk::Button* B_incl_e;
    auto&&       builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_range.ui" ) };

    m_W_entry_picker = Gtk::manage( new WidgetEntryPicker( false, "tag-16-symbolic" ) );
    m_CB_type = Gtk::manage( new Gtk::ComboBoxText );

    Bx_range    = builder->get_widget< Gtk::Box >( "Bx_range" );
    B_incl_b    = builder->get_widget< Gtk::Button >( "B_incl_b" );
    B_incl_e    = builder->get_widget< Gtk::Button >( "B_incl_e" );
    m_L_incl_b  = builder->get_widget< Gtk::Label >( "L_incl_b" );
    m_L_incl_e  = builder->get_widget< Gtk::Label >( "L_incl_e" );
    m_E_range_b = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_bgn" );
    m_E_range_e = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_end" );

    m_W_entry_picker->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_W_entry_picker->set_entry( m_p2filterer->m_tag );
    m_W_entry_picker->signal_updated().connect(
            [ this ]( Entry* e )
            {
                m_p2filterer->m_tag = m_W_entry_picker->get_entry();
                m_p2WF->emit_changed();
            } );

    m_CB_type->append( _( "Planned" ) );
    m_CB_type->append( _( "Realized" ) );
    m_CB_type->append( _( "Remaining" ) );
    m_CB_type->set_active( log2( m_p2filterer->m_type / VT::PLANNED ) );

    m_CB_type->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_type = ( VT::PLANNED << m_CB_type->get_active_row_number() );
                m_p2WF->emit_changed();
            } );

    m_E_range_b->set_placeholder_text( "-∞" );
    m_E_range_b->set_width_chars( 10 );
    if( m_p2filterer->m_range_b != Constants::INFINITY_MNS )
        m_E_range_b->set_text( STR::format_number( m_p2filterer->m_range_b ) );
    m_E_range_b->signal_changed().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ) return;
                const auto& text{ m_E_range_b->get_text() };
                m_p2filterer->m_range_b = text.empty() ? Constants::INFINITY_MNS
                                                       : STR::get_d( text );
                m_p2WF->emit_changed();
            } );

    m_E_range_e->set_placeholder_text( "+∞" );
    m_E_range_e->set_width_chars( 10 );
    if( m_p2filterer->m_range_e != Constants::INFINITY_PLS )
        m_E_range_e->set_text( STR::format_number( m_p2filterer->m_range_e ) );
    m_E_range_e->signal_changed().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ) return;
                const auto& text{ m_E_range_e->get_text() };
                m_p2filterer->m_range_e = text.empty() ? Constants::INFINITY_PLS
                                                       : STR::get_d( text );
                m_p2WF->emit_changed();
            } );

    m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
    B_incl_b->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_b = !m_p2filterer->m_F_incl_b;
                m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
                m_p2WF->emit_changed();
            } );

    m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
    B_incl_e->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_e = !m_p2filterer->m_F_incl_e;
                m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
                m_p2WF->emit_changed();
            } );

    append( *m_W_entry_picker );
    append( *m_CB_type );
    append( *Gtk::manage( new Gtk::Label( "∈" ) ) );
    append( *Bx_range );
}

// FILTERER THEME ==================================================================================
FiltererThemeUI::FiltererThemeUI( WidgetFilter* ctr, FiltererTheme* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto L_name { Gtk::manage( new Gtk::Label( _( "Has the theme" ) ) ) };
    m_W_theme_picker = Gtk::manage( new WidgetPicker< Theme > );

    m_W_theme_picker->set_map( m_p2filterer->m_p2container->m_p2diary->get_p2themes(), 
                               &m_p2filterer->m_theme );
    m_W_theme_picker->set_select_only( true );
    if( m_p2filterer->m_theme )
        m_W_theme_picker->set_text( m_p2filterer->m_theme->get_name() );
    m_W_theme_picker->signal_sel_changed().connect(
            [ this ]( const Ustring& name )
            {
                m_p2filterer->m_theme = m_p2filterer->m_p2container->m_p2diary->get_theme( name );
                m_p2WF->emit_changed();
            } );

    append( *L_name );
    append( *m_W_theme_picker );
}

// FILTERER BETWEEN DATES ==========================================================================
FiltererBetweenDatesUI::FiltererBetweenDatesUI( WidgetFilter* ctr,
                                                FiltererBetweenDates* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*    Bx_range;
    Gtk::Button* B_incl_b;
    Gtk::Button* B_incl_e;
    auto&&       builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_range.ui" ) };

    Bx_range    = builder->get_widget< Gtk::Box >( "Bx_range" );
    B_incl_b    = builder->get_widget< Gtk::Button >( "B_incl_b" );
    B_incl_e    = builder->get_widget< Gtk::Button >( "B_incl_e" );
    m_L_incl_b  = builder->get_widget< Gtk::Label >( "L_incl_b" );
    m_L_incl_e  = builder->get_widget< Gtk::Label >( "L_incl_e" );
    m_W_date_picker_b = Gtk::Builder::get_widget_derived< WidgetDatePicker >( builder, "E_bgn" );
    m_W_date_picker_e = Gtk::Builder::get_widget_derived< WidgetDatePicker >( builder, "E_end" );

    m_W_date_picker_b->set_width_chars( 13 );
    m_W_date_picker_b->set_date( m_p2filterer->m_date_b );
    m_W_date_picker_b->signal_date_set().connect(
            [ this ]( DateV d ){ m_p2filterer->m_date_b = d; m_p2WF->emit_changed(); } );

    m_W_date_picker_e->set_width_chars( 13 );
    m_W_date_picker_e->set_date( m_p2filterer->m_date_e );
    m_W_date_picker_e->signal_date_set().connect(
            [ this ]( DateV d ){ m_p2filterer->m_date_e = d; m_p2WF->emit_changed(); } );

    m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
    B_incl_b->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_b = !m_p2filterer->m_F_incl_b;
                m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
                m_p2WF->emit_changed();
            } );

    m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
    B_incl_e->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_e = !m_p2filterer->m_F_incl_e;
                m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
                m_p2WF->emit_changed();
            } );

    append( *Gtk::manage( new Gtk::Label( _( "Between" ) ) ) );
    append( *Bx_range );
}

// FILTERER BETWEEN ENTRIES ========================================================================
FiltererBetweenEntriesUI::FiltererBetweenEntriesUI( WidgetFilter* ctr,
                                                    FiltererBetweenEntries* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*    Bx_range;
    Gtk::Button* B_incl_b;
    Gtk::Button* B_incl_e;
    auto&&       builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_range.ui" ) };

    Bx_range    = builder->get_widget< Gtk::Box >( "Bx_range" );
    B_incl_b    = builder->get_widget< Gtk::Button >( "B_incl_b" );
    B_incl_e    = builder->get_widget< Gtk::Button >( "B_incl_e" );
    m_L_incl_b  = builder->get_widget< Gtk::Label >( "L_incl_b" );
    m_L_incl_e  = builder->get_widget< Gtk::Label >( "L_incl_e" );
    m_WEP_bgn   = Gtk::Builder::get_widget_derived< WidgetEntryPicker >( builder, "E_bgn" );
    m_WEP_end   = Gtk::Builder::get_widget_derived< WidgetEntryPicker >( builder, "E_end" );

    m_WEP_bgn->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_WEP_bgn->set_entry( m_p2filterer->m_entry_b );
    m_WEP_bgn->signal_updated().connect(
            [ this ]( Entry* e ){ m_p2filterer->m_entry_b = e; m_p2WF->emit_changed(); } );

    m_WEP_end->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_WEP_end->set_entry( m_p2filterer->m_entry_e );
    m_WEP_end->signal_updated().connect(
            [ this ]( Entry* e ){ m_p2filterer->m_entry_e = e; m_p2WF->emit_changed(); } );

    m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
    B_incl_b->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_b = !m_p2filterer->m_F_incl_b;
                m_L_incl_b->set_label( m_p2filterer->m_F_incl_b ? "[" : "(" );
                m_p2WF->emit_changed();
            } );

    m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
    B_incl_e->signal_clicked().connect(
            [ this ]()
            {
                m_p2filterer->m_F_incl_e = !m_p2filterer->m_F_incl_e;
                m_L_incl_e->set_label( m_p2filterer->m_F_incl_e ? "]" : ")" );
                m_p2WF->emit_changed();
            } );

    append( *Gtk::manage( new Gtk::Label( _( "Between" ) ) ) );
    append( *Bx_range );
}

// FILTERER COMPLETION =============================================================================
FiltererCompletionUI::FiltererCompletionUI( WidgetFilter* ctr, FiltererCompletion* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    m_Sc_bgn = Gtk::manage( new Gtk::Scale( Gtk::Orientation::HORIZONTAL ) );
    m_Sc_end = Gtk::manage( new Gtk::Scale( Gtk::Orientation::HORIZONTAL ) );

    m_Sc_bgn->set_has_origin( false );
    m_Sc_bgn->set_value_pos( Gtk::PositionType::RIGHT );
    m_Sc_bgn->set_adjustment( Gtk::Adjustment::create( m_p2filterer->m_compl_b, 0.0, 100.0 ) );
    m_Sc_bgn->set_size_request( 160, -1 );
    m_Sc_bgn->signal_value_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_compl_b = m_Sc_bgn->get_value();
                m_p2WF->emit_changed();
            } );

    m_Sc_end->set_has_origin( false );
    m_Sc_end->set_value_pos( Gtk::PositionType::LEFT );
    m_Sc_end->set_adjustment( Gtk::Adjustment::create( m_p2filterer->m_compl_e, 0.0, 100.0 ) );
    m_Sc_end->set_size_request( 160, -1 );
    m_Sc_end->signal_value_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_compl_e = m_Sc_end->get_value();
                m_p2WF->emit_changed();
            } );

    append( *Gtk::manage( new Gtk::Label( _( "Completion is in range" ) ) ) );
    append( *m_Sc_bgn );
    append( *m_Sc_end );
}

// FILTERER CONTAINS TEXT ==========================================================================
FiltererContainsTextUI::FiltererContainsTextUI( WidgetFilter* ctr,
                                                FiltererContainsText* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    Gtk::Box*    Bx_main;
    auto&&       builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/flt_text.ui" ) };

    Bx_main         = builder->get_widget< Gtk::Box >( "Bx_main" );
    m_E_text        = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_text" );
    m_TB_match_case = builder->get_widget< Gtk::ToggleButton >( "TB_match_case" );
    m_CB_extent     = builder->get_widget< Gtk::ComboBoxText >( "CB_extent" );

    m_E_text->set_text( m_p2filterer->m_text );
    m_TB_match_case->set_active( m_p2filterer->m_case_sensitive );
    m_CB_extent->set_active( m_p2filterer->m_name_only ? 1 : 0 );

    m_E_text->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_text = m_E_text->get_text();
                m_p2WF->emit_changed();
            } );

    m_TB_match_case->signal_toggled().connect(
            [ this ]()
            {
                m_p2filterer->m_case_sensitive = m_TB_match_case->get_active();
                m_p2WF->emit_changed();
            });

    m_CB_extent->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_name_only = ( m_CB_extent->get_active_row_number() == 1 );
                m_p2WF->emit_changed();
            } );

    append( *Bx_main );
}

// FILTERER CHILD FILTER ===========================================================================
FiltererChildFilterUI::FiltererChildFilterUI( WidgetFilter* ctr, FiltererChildFilter* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    m_WFP_filter = Gtk::manage( new WidgetFilterPicker );

    m_WFP_filter->set_halign( Gtk::Align::START );

    m_WFP_filter->set_diary( m_p2filterer->m_p2container->m_p2diary );
    m_WFP_filter->set_active( m_p2filterer->m_p2filter );

    m_WFP_filter->set_obj_classes( FOC::PARAGRAPHS );
    m_WFP_filter->Sg_changed.connect(
            [ this ]( const Filter* filter )
            {
                m_p2filterer->m_p2filter = const_cast< Filter* >( filter );
                if( filter )
                    m_p2filterer->m_FC_stack = filter->get_filterer_stack();
                m_p2WF->emit_changed();
            } );

    append( *Gtk::manage( new Gtk::Label( _( "Satisfies filter" ) ) ) );
    append( *m_WFP_filter );
}

// FILTERER IS IMAGE ===============================================================================
FiltererIsImageUI::FiltererIsImageUI( WidgetFilter* ctr, FiltererIsImage* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    append( *Gtk::manage( new Gtk::Label( _( "Is image" ) ) ) );
}

// FILTERER HAS COORD ==============================================================================
FiltererHasCoordsUI::FiltererHasCoordsUI( WidgetFilter* ctr, FiltererHasCoords* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    append( *Gtk::manage( new Gtk::Label( _( "Has map coordinates" ) ) ) );
}

// FILTERER TITLE STYLE ============================================================================
FiltererTitleStyleUI::FiltererTitleStyleUI( WidgetFilter* ctr, FiltererTitleStyle* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    m_DD_title_style = Gtk::make_managed< Gtk::DropDown >( Lifeograph::s_entry_name_styles );
    m_DD_title_style->set_selected( VT::get_v< VT::ETS, unsigned, char >(
                                                m_p2filterer->m_title_style ) );

    append( *Gtk::make_managed< Gtk::Label >( _( "Title style is" ) ) );
    append( *m_DD_title_style );

    m_DD_title_style->property_selected().signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_title_style = VT::get_v< VT::ETS, char, unsigned >(
                                                        m_DD_title_style ->get_selected() );
                m_p2WF->emit_changed();
            } );
}

// FILTERER HEADING LEVEL ==========================================================================
FiltererHeadingLevelUI::FiltererHeadingLevelUI( WidgetFilter* ctr,
                                                FiltererHeadingLevel* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    auto Bx_headings { Gtk::make_managed< Gtk::Box >() };
    m_TB_H0 = Gtk::make_managed< Gtk::ToggleButton >( "H0" );
    m_TB_H1 = Gtk::make_managed< Gtk::ToggleButton >( "H1" );
    m_TB_H2 = Gtk::make_managed< Gtk::ToggleButton >( "H2" );
    m_TB_H3 = Gtk::make_managed< Gtk::ToggleButton >( "H3" );

    Bx_headings->add_css_class( "linked" );
    Bx_headings->append( *m_TB_H0 );
    Bx_headings->append( *m_TB_H1 );
    Bx_headings->append( *m_TB_H2 );
    Bx_headings->append( *m_TB_H3 );

    // H levels are inverse so the entry title is 0
    // this is to make it easy to add furher sub heading levels int the future
    m_TB_H0->set_active( bool( m_p2filterer->m_H_levels & ( 1      ) ) );
    m_TB_H1->set_active( bool( m_p2filterer->m_H_levels & ( 1 << 3 ) ) );
    m_TB_H2->set_active( bool( m_p2filterer->m_H_levels & ( 1 << 2 ) ) );
    m_TB_H3->set_active( bool( m_p2filterer->m_H_levels & ( 1 << 1 ) ) );

    append( *Gtk::make_managed< Gtk::Label >( _( "Heading level is" ) ) );
    append( *Bx_headings );

    m_TB_H0->signal_toggled().connect( [ this ](){ handle_change( 0, m_TB_H0->get_active() ); } );
    m_TB_H1->signal_toggled().connect( [ this ](){ handle_change( 3, m_TB_H1->get_active() ); } );
    m_TB_H2->signal_toggled().connect( [ this ](){ handle_change( 2, m_TB_H2->get_active() ); } );
    m_TB_H3->signal_toggled().connect( [ this ](){ handle_change( 1, m_TB_H3->get_active() ); } );
}

void
FiltererHeadingLevelUI::handle_change( int level, bool active )
{
    if( active )
        m_p2filterer->m_H_levels |= ( 1 << level );
    else
        m_p2filterer->m_H_levels &= ~( 1 << level );

    m_p2WF->emit_changed();
}

// FILTERER EQUALS =================================================================================
FiltererEqualsUI::FiltererEqualsUI( WidgetFilter* ctr, FiltererEquals* p2filterer )
: FiltererUI( ctr, p2filterer )
{
    m_DD_relation = Gtk::make_managed< Gtk::DropDown >(
            STR::make_substr_vector( VT::AR_CHARS ) );
    m_E_value     = Gtk::make_managed< Gtk::Entry >();

    m_DD_relation->set_show_arrow( false );
    m_DD_relation->set_selected( STR::get_pos_c( VT::AR_CHARS, m_p2filterer->m_relation, 1 ) );
    // 1 designates '=' as the default value
    m_DD_relation->property_selected().signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_relation = VT::AR_CHARS[ m_DD_relation->get_selected() ];
                m_p2WF->emit_changed();
            } );

    m_E_value->set_text( STR::format_number( m_p2filterer->m_value ) );
    m_E_value->signal_changed().connect(
            [ this ]()
            {
                m_p2filterer->m_value = STR::get_d( m_E_value->get_text() );
                m_p2WF->emit_changed();
            } );

    append( *m_DD_relation );
    append( *m_E_value );
}


// FILTERER CONTAINER ==============================================================================
FiltererContainerUI::FiltererContainerUI( WidgetFilter* p2WF, FiltererContainer* p2filterer )
:   FiltererUI( p2WF, p2filterer )
{
    set_valign( Gtk::Align::START );

    // HEADER
    m_Bx_header = Gtk::manage( new Gtk::Box( Gtk::Orientation::VERTICAL, 2 ) );
    m_L_logic = Gtk::manage( new Gtk::Label );
    auto Sp_top{ Gtk::manage( new Gtk::Separator( Gtk::Orientation::VERTICAL ) ) };
    auto Sp_btm{ Gtk::manage( new Gtk::Separator( Gtk::Orientation::VERTICAL ) ) };

    Sp_top->set_halign( Gtk::Align::CENTER );
    Sp_btm->set_halign( Gtk::Align::CENTER );
    Sp_top->set_vexpand( true );
    Sp_btm->set_vexpand( true );
    update_logic_label();

    m_Bx_header->append( *Sp_top );
    if( m_p2filterer->m_p2container ) // if it is a subgroup
        m_Bx_header->append( *m_L_logic );
    else
    {
        auto B_logic{ Gtk::manage( new Gtk::Button ) };
        B_logic->set_has_frame( false );
        B_logic->set_child( *m_L_logic );
        B_logic->signal_clicked().connect( [ this ](){ m_p2WF->toggle_logic(); } );
        m_Bx_header->append( *B_logic );
    }
    m_Bx_header->append( *Sp_btm );

    // CONTENTS
    m_Bx_contents   = Gtk::manage( new Gtk::Box( Gtk::Orientation::VERTICAL, 5 ) );
    m_WS_add        = Gtk::make_managed< WidgetSelector >();

    m_WS_add->set_icon_name( "list-add-symbolic" );
    m_WS_add->set_halign( Gtk::Align::START );
    m_Bx_contents->append( *m_WS_add );

    append( *m_Bx_header );
    append( *m_Bx_contents );

    set_from_stack();
}

void
FiltererContainerUI::clear_pipeline()
{
    remove_all_children_from_Bx( m_Bx_contents );
    m_p2filterer->clear_pipeline();
}

void
FiltererContainerUI::update_logic_label()
{
    if( m_p2filterer->m_F_or )
        m_L_logic->set_markup( STR::compose( "<b>", _( "OR" ), "</b>" ) );
    else
        m_L_logic->set_markup( STR::compose( "<b>", _( "AND" ), "</b>" ) );
}

void
FiltererContainerUI::set_from_stack()
{
    for( auto filterer : m_p2filterer->m_pipeline )
        filterer->initialize_ui( this );
}

void
FiltererContainerUI::remove_filterer( FiltererUI* filterer )
{
    m_p2filterer->remove_filterer( filterer->m_p2filterer );
    m_Bx_contents->remove( * dynamic_cast< Gtk::Box* >( filterer ) );

    FiltererContainerUI* filtererui { dynamic_cast< FiltererContainerUI* >( filterer ) };
    if( filtererui )
        m_subgroups.erase( filtererui );

    m_p2WF->emit_changed();
}

void
FiltererContainerUI::update_add_menu()
{
    // clear
    m_WS_add->clear_list();

    add_filterer_to_menu< FiltererTrashedUI, FiltererTrashed >();
    add_filterer_to_menu< FiltererStatusUI, FiltererStatus >();
    add_filterer_to_menu< FiltererSizeUI, FiltererSize >();
    add_filterer_to_menu< FiltererFavoriteUI, FiltererFavorite >();
    add_filterer_to_menu< FiltererUnitUI, FiltererUnit >();
    add_filterer_to_menu< FiltererIsUI, FiltererIs >();
    add_filterer_to_menu< FiltererDescendantOfUI, FiltererDescendantOf >();
    add_filterer_to_menu< FiltererBetweenDatesUI, FiltererBetweenDates >();
    add_filterer_to_menu< FiltererBetweenEntriesUI, FiltererBetweenEntries >();
    add_filterer_to_menu< FiltererCompletionUI, FiltererCompletion >();
    add_filterer_to_menu< FiltererTaggedByUI, FiltererTaggedBy >();
    add_filterer_to_menu< FiltererTagValueUI, FiltererTagValue >();
    add_filterer_to_menu< FiltererSubtaggedByUI, FiltererSubtaggedBy >();
    add_filterer_to_menu< FiltererThemeUI, FiltererTheme >();
    add_filterer_to_menu< FiltererContainsTextUI, FiltererContainsText >();
    add_filterer_to_menu< FiltererIsImageUI, FiltererIsImage >();
    add_filterer_to_menu< FiltererHasCoordsUI, FiltererHasCoords >();
    add_filterer_to_menu< FiltererTitleStyleUI, FiltererTitleStyle >();
    add_filterer_to_menu< FiltererHeadingLevelUI, FiltererHeadingLevel >();
    add_filterer_to_menu< FiltererEqualsUI, FiltererEquals >();
    // TODO: 3.1: check for cyclic references:
    add_filterer_to_menu< FiltererChildFilterUI, FiltererChildFilter >();
    if( m_p2filterer->m_p2container == nullptr ) // top level
        add_filterer_to_menu< FiltererContainerUI, FiltererContainer >();
}

// WIDGET FILTER ===================================================================================
void
WidgetFilter::set_enabled( bool flag_enabled )
{
    m_Bx_contents->set_sensitive( flag_enabled );
}

void
WidgetFilter::toggle_logic()
{
    m_p2filterer->toggle_logic();
    update_logic_label();

    for( auto sub : m_subgroups )
       sub->update_logic_label();

    emit_changed();
}

void
WidgetFilter::emit_changed()
{
    if( m_F_emit_signals )
        m_Sg_changed.emit();
}
