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


#include <cairomm/context.h>

#include "../lifeograph.hpp"
#include "../diaryelements/diarydata.hpp"
#include "widget_table.hpp"


using namespace LIFEO;

// WIDGETTABLE =====================================================================================
WidgetTable::WidgetTable( BaseObjectType* cobj, const Glib::RefPtr< Gtk::Builder >& refbuilder )
:   Gtk::DrawingArea( cobj )
{
    m_builder               = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/table.ui" );

    m_Po_table              = m_builder->get_widget< Gtk::Popover >( "Po_table" );
    //m_Bx_edit               = m_builder->get_widget< Gtk::Box >( "Bx_edit" );
    m_WFP_entry             = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    m_builder, "MB_entry_filter" );
    m_WFP_para              = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    m_builder, "MB_para_filter" );
    m_Bx_filter_para        = m_builder->get_widget< Gtk::Box >( "Bx_para_filter" );
    m_RB_entry_based        = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_based" );
    m_RB_para_based         = m_builder->get_widget< Gtk::ToggleButton >( "RB_para_based" );

    m_Po_column             = m_builder->get_widget< Gtk::Popover >( "Po_column" );
    // m_Bx_col_main           = m_builder->get_widget< Gtk::Box >( "Bx_col_main" );
    m_E_col_name            = Gtk::Builder::get_widget_derived< EntryClear >(
                                    m_builder, "E_col_name" );
    m_Bx_col_basics         = m_builder->get_widget< Gtk::Box >( "Bx_col_basics" );
    m_CB_col_type           = m_builder->get_widget< Gtk::ComboBoxText >( "CB_col_type" );
    m_Bx_col_source         = m_builder->get_widget< Gtk::Box >( "Bx_col_src" );
    m_WFP_col_source_elem   = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    m_builder, "MB_col_src_elem" );
    m_CB_col_source_entry   = m_builder->get_widget< Gtk::ComboBoxText >( "CB_col_src_entry" );
    m_CB_col_source_para    = m_builder->get_widget< Gtk::ComboBoxText >( "CB_col_src_para" );
    m_Bx_col_show_as        = m_builder->get_widget< Gtk::Box >( "Bx_col_show_as" );
    m_DD_col_show_as        = m_builder->get_widget< Gtk::DropDown >( "DD_col_show_as" );
    m_Bx_col_count_condtn   = m_builder->get_widget< Gtk::Box >( "Bx_col_count_condition" );
    m_WFP_column            = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                    m_builder, "MB_col_filter" );
    m_DD_col_summary_func   = m_builder->get_widget< Gtk::DropDown >( "DD_col_summary_func" );
    m_RB_col_sort_off       = m_builder->get_widget< Gtk::ToggleButton >( "RB_sort_off" );
    m_RB_col_sort_asc       = m_builder->get_widget< Gtk::ToggleButton >( "RB_sort_asc" );
    m_RB_col_sort_dsc       = m_builder->get_widget< Gtk::ToggleButton >( "RB_sort_dsc" );
    m_TB_col_combine_same   = m_builder->get_widget< Gtk::ToggleButton >( "TB_col_combine_same" );
    m_Bx_col_coloring       = m_builder->get_widget< Gtk::Box >( "Bx_col_coloring" );
    m_RB_col_color_off      = m_builder->get_widget< Gtk::ToggleButton >( "RB_color_off" );
    m_RB_col_color_neg_red  = m_builder->get_widget< Gtk::ToggleButton >( "RB_color_neg_red" );
    m_RB_col_color_big_green = m_builder->get_widget< Gtk::ToggleButton >( "RB_color_big_green" );
    m_RB_col_color_big_red  = m_builder->get_widget< Gtk::ToggleButton >( "RB_color_big_red" );

    m_B_col_dismiss         = m_builder->get_widget< Gtk::Button >( "B_col_dismiss" );
    m_B_col_add_left        = m_builder->get_widget< Gtk::Button >( "B_col_add_left" );
    m_B_col_add_right       = m_builder->get_widget< Gtk::Button >( "B_col_add_right" );
    m_B_col_move_prev       = m_builder->get_widget< Gtk::Button >( "B_col_move_prev" );
    m_B_col_move_next       = m_builder->get_widget< Gtk::Button >( "B_col_move_next" );

    m_Aj_vert = Gtk::Adjustment::create( 0.0, 0.0, 100.0 );

    m_Sb_vert = refbuilder->get_widget< Gtk::Scrollbar >( "Sb_table" );
    m_Sb_vert->set_adjustment( m_Aj_vert );
    //m_Sb_vert->set_round_digits( 0 ); gtkmm4: scrollbar no longer derives from Range

    m_WFP_entry->set_clearable( true );
    m_WFP_entry->set_show_edit_button( true );

    m_WFP_para->set_clearable( true );
    m_WFP_para->set_show_edit_button( true );

    m_WFP_col_source_elem->set_clearable( true );
    m_WFP_col_source_elem->set_show_edit_button( true );

    m_WFP_column->set_clearable( true );
    m_WFP_column->set_show_edit_button( true );

    m_Po_column->set_parent( *this );

    m_Aj_vert->signal_value_changed().connect(
            [ this ]() { scroll( m_Aj_vert->get_value() - m_i_line_top ); } );

    m_RB_para_based->signal_toggled().connect( [ this ](){ handle_base_changed(); } );

    m_WFP_entry->set_obj_classes( FOC::DIARYELEMS );
    m_WFP_entry->Sg_changed.connect(
            [ this ]( const Filter* filter ) { set_entry_filter( filter ); } );
    m_WFP_entry->Sg_hide_my_Po.connect( [ this ]() { m_Po_table->hide(); } );

    m_WFP_para->set_obj_classes( FOC::PARAGRAPHS );
    m_WFP_para->Sg_changed.connect(
            [ this ]( const Filter* filter ) { set_para_filter( filter ); } );
    m_WFP_para->Sg_hide_my_Po.connect( [ this ]() { m_Po_table->hide(); } );

    m_CB_col_type->signal_changed().connect( [ this ](){ handle_col_type_changed(); } );

    m_E_col_name->signal_changed().connect( [ this ](){ handle_col_renamed(); } );

    m_CB_col_source_entry->signal_changed().connect(
            [ this ]() { handle_col_source_elem_changed(); } );
    m_CB_col_source_para->signal_changed().connect(
            [ this ]() { handle_col_source_elem_changed(); } );

    m_WFP_col_source_elem->Sg_changed.connect(
            [ this ]( const Filter* filter )
            {
                m_p2col_sel->set_source_elem_filter( filter );
                calculate_and_plot();
            } );

    // SHOW AS
    const VecUstrings sas_options( { VT::SAS::NORMAL::S, VT::SAS::DELTA::S, VT::SAS::COUNT::S } );
    m_DD_col_show_as->set_model( Gtk::StringList::create( sas_options ) );

    m_DD_col_show_as->property_selected().signal_changed().connect(
            [ this ](){ handle_col_show_as_chg(); } );

    m_WFP_column->Sg_changed.connect(
            [ this ]( const Filter* filter )
            {
                m_p2col_sel->set_count_filter( filter );
                calculate_and_plot();
            } );

    // SUMMARY FUNCTION
    const VecUstrings sumf_options( { VT::SUMF::NONE::S, VT::SUMF::SUM::S, VT::SUMF::AVG::S } );
    m_DD_col_summary_func->set_model( Gtk::StringList::create( sumf_options ) );

    m_DD_col_summary_func->property_selected().signal_changed().connect(
            [ this ]() { handle_col_summary_func_chg(); } );

    // SORTING
    m_TB_col_combine_same->signal_toggled().connect(
            [ this ]() { handle_col_combine_same_toggled(); } );

    m_RB_col_sort_off->signal_toggled().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ||
                    not( m_RB_col_sort_off->get_active() ) ) return;
                m_data.unset_column_sort( m_i_col_cur );
                m_TB_col_combine_same->set_visible( false );
                sort_and_plot();
            } );
    m_RB_col_sort_asc->signal_toggled().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ||
                    not( m_RB_col_sort_asc->get_active() ) ) return;
                m_data.set_column_sort_dir( m_i_col_cur, false );
                m_RB_col_sort_off->set_visible( m_data.m_columns_sort.size() > 1 );
                m_TB_col_combine_same->set_visible( true );
                sort_and_plot();
            } );
    m_RB_col_sort_dsc->signal_toggled().connect(
            [ this ]()
            {
                if( Lifeograph::is_internal_operations_ongoing() ||
                    not( m_RB_col_sort_dsc->get_active() ) ) return;
                m_data.set_column_sort_dir( m_i_col_cur, true );
                m_RB_col_sort_off->set_visible( m_data.m_columns_sort.size() > 1 );
                m_TB_col_combine_same->set_visible( true );
                sort_and_plot();
            } );

    // COLORING
    m_RB_col_color_off->signal_toggled().connect(
            [ this ]() { handle_col_color_chg( m_RB_col_color_off ); } );
    m_RB_col_color_neg_red->signal_toggled().connect(
            [ this ]() { handle_col_color_chg( m_RB_col_color_neg_red ); } );
    m_RB_col_color_big_green->signal_toggled().connect(
            [ this ]() { handle_col_color_chg( m_RB_col_color_big_green ); } );
    m_RB_col_color_big_red->signal_toggled().connect(
            [ this ]() { handle_col_color_chg( m_RB_col_color_big_red ); } );

    // ADD, REMOVE, MOVE
    m_B_col_move_prev->signal_clicked().connect(
            [ this ]()
            {
                m_data.move_column( m_i_col_cur, m_i_col_cur-1 );
                update_col_geom();
                refresh();
                m_Sg_changed.emit();
                m_i_col_cur--;
                place_column_Po();
            } );

    m_B_col_move_next->signal_clicked().connect(
            [ this ]()
            {
                m_data.move_column( m_i_col_cur, m_i_col_cur+1 );
                update_col_geom();
                refresh();
                m_Sg_changed.emit();
                m_i_col_cur++;
                place_column_Po();
            } );

    m_B_col_dismiss->signal_clicked().connect(
            [ this ]()
            {
                m_Po_column->hide();
                m_data.dismiss_column( m_i_col_cur );
                update_col_geom();
                calculate_and_plot();
            } );

    m_B_col_add_left->signal_clicked().connect(
            [ this ]()
            {
                m_data.add_column( m_i_col_cur );
                update_col_geom();
                calculate_and_plot();
                show_column_Po();
            } );
    m_B_col_add_right->signal_clicked().connect(
            [ this ]()
            {
                m_data.add_column( ++m_i_col_cur );
                update_col_geom();
                calculate_and_plot();
                show_column_Po();
            } );

    // RESIZING & DRAWING
    set_draw_func( sigc::mem_fun( *this, &WidgetTable::on_draw ) );

    // EVENT CONTROLLERS
    m_event_controller_scroll = Gtk::EventControllerScroll::create();
    m_event_controller_scroll->signal_scroll().connect(
            sigc::mem_fun( *this, &WidgetTable::on_scroll_event ), false );
    m_event_controller_scroll->set_flags( Gtk::EventControllerScroll::Flags::VERTICAL );
    add_controller( m_event_controller_scroll );

    m_gesture_click = Gtk::GestureClick::create();
    m_gesture_click->set_button( 0 );
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &WidgetTable::on_button_press_event ), false );
    m_gesture_click->signal_released().connect(
            sigc::mem_fun( *this, &WidgetTable::on_button_release_event ), false );
    add_controller( m_gesture_click );

    auto controller_motion = Gtk::EventControllerMotion::create();
    controller_motion->signal_motion().connect(
            sigc::mem_fun( *this, &WidgetTable::on_motion_notify_event ), false );
    controller_motion->signal_leave().connect(
            sigc::mem_fun( *this, &WidgetTable::on_leave_notify_event ), false );
    add_controller( controller_motion );

    // ACTIONS
    Lifeograph::p->add_action_bool( "table_show_order_col",
                                    [ this ](){ handle_order_col_toggled(); },
                                    false );
    Lifeograph::p->add_action_bool( "table_group_lines",
                                    [ this ](){ handle_grouping_toggled(); },
                                    false );
    Lifeograph::p->add_action( "table_copy_as_text", [ this ](){ copy_delimited_text(); } );
}

void
WidgetTable::set_helper_widgets( Gtk::MenuButton* MB_properties )
{
    MB_properties->set_popover( *m_Po_table );
}

bool
WidgetTable::on_scroll_event( double dx, double dy )
{
    auto modifiers{ m_event_controller_scroll->get_current_event()->get_modifier_state() };

    if( m_line_c_max >= m_data.m_lines.size() )
        return false;
    else if( dy < 0 )
        scroll( bool( modifiers & Gdk::ModifierType::CONTROL_MASK ) ? -10 : -1 );
    else if( dy > 0 )
        scroll( bool( modifiers & Gdk::ModifierType::CONTROL_MASK ) ? 10 : 1 );

    m_Aj_vert->set_value( m_i_line_top );

    return true;
}

void
WidgetTable::on_button_press_event( int n_press, double x, double y )
{
    if( m_gesture_click->get_current_button() == 1 )
    {
        TableLine* line{ m_i_line_hover > 0 ?
                         m_data.m_lines[ m_i_line_hover + m_i_line_top - 1 ] :
                         nullptr };

        if( n_press == 2 )
        {
            if( m_i_line_hover == 0 && Diary::d->is_in_edit_mode() &&
                m_data.m_columns[ m_i_col_cur ]->m_type != TableColumn::TCT_ORDER )
            {
                if ( bool( m_gesture_click->get_current_event_state() &
                           Gdk::ModifierType::SHIFT_MASK ) )
                    m_data.set_column_sort_dir(
                            m_i_col_cur,
                            m_data.m_columns[ m_i_col_cur ]->m_sort_order > 0 ?
                                    !m_data.m_columns[ m_i_col_cur ]->m_sort_desc : false );
                else
                    m_data.set_column_sort_or_change_dir( m_i_col_cur );
                sort_and_plot();
            }
            else if( line && line->is_group_head() )
            {
                m_data.m_master_header.toggle_all_expanded();
                update_visible_lines_and_plot( true );
            }
        }
        else if( is_pt_on_col_boundary() ) // in do-not-recalculate mode
            enter_column_resize_mode( x );
        else if( m_i_line_hover > 0 )
        {
            if( line->is_group_head() )
            {
                line->toggle_expanded();
                update_visible_lines_and_plot();
            }
            else
                m_Sg_elem_clicked.emit( line->m_p2elem );
        }
        m_F_button_pressed = true;
    }
    else if( m_gesture_click->get_current_button() == 3 )
    {
        //if( m_i_line_hover == 0 ) why limit?
        {
            if( m_F_editable && m_i_col_cur >= 0 &&
                m_data.m_columns[ m_i_col_cur ]->m_type != TableColumn::TCT_ORDER )
                show_column_Po();
        }
    }
}

void
WidgetTable::on_button_release_event( int n_press, double x, double y )
{
    if( m_gesture_click->get_current_button() == 1 )
    {
        m_F_button_pressed = false;
        if( m_x_coord_drag_start > 0 ) // i.e. if in resize mode
            exit_column_resize_mode();
    }
}

void
WidgetTable::on_motion_notify_event( double x, double y )
{
    if( m_data.m_columns.empty() )
    {
        //do nothing and go to the return at the end
    }
    else if( m_x_coord_drag_start > 0 ) // i.e. if in resize mode
    {
        double width_diff{ x - m_x_coord_drag_start };
        auto&& iter_col{ m_data.m_columns.begin() };

        std::advance( iter_col, m_col_i_drag );

        // ensure the limits are not exceeded:
        if( m_col_width_drag_L + width_diff < 20 ) width_diff = 20 - m_col_width_drag_L;
        if( m_col_width_drag_R - width_diff < 20 ) width_diff = m_col_width_drag_R - 20;

        ( *iter_col )->m_width = ( m_col_width_drag_L + width_diff ) / m_width_net;

        iter_col++;
        ( *iter_col )->m_width = ( m_col_width_drag_R - width_diff ) / m_width_net;

        update_col_geom();
        refresh();
    }
    else if( !is_pt_on_col_boundary( x ) )
    {
        int i_col_hover{ -1 };
        for( unsigned int i = 0; i < m_col_widths_cum.size() - 1; i++ )
        {
            if( m_col_widths_cum[ i ] < x && x < m_col_widths_cum[ i + 1 ] )
            {
                i_col_hover = i;
                break;
            }
        }

        int i_line_hover{ int( ( y - S_MARGIN ) / m_row_h ) };

        if( i_line_hover < 0 || i_line_hover > int( m_line_c_act ) )
            i_line_hover = -1;

        if( i_col_hover != m_i_col_cur || i_line_hover != m_i_line_hover )
        {
            m_i_col_cur = i_col_hover;
            m_i_line_hover = i_line_hover;
            refresh();
        }

        m_F_widget_hovered = true;
    }
}

void
WidgetTable::on_leave_notify_event()
{
    m_F_widget_hovered = false;
    m_i_line_hover = -1;
    refresh();
}

void
WidgetTable::on_resize( int w, int h )
{
    if( w != m_width || h != m_height )
    {
        resize( w, h );
        m_row_h = 0.0; // to trigger updates during draw
    }
}

void
WidgetTable::on_draw( const Cairo::RefPtr< Cairo::Context >& cr, int w, int h )
{
    if( !m_layout )
        m_layout = Pango::Layout::create( cr );

    if( m_row_h == 0.0 )
    {
        calculate_row_h();
        update_line_c_vis();
        if( !update_scrollbar() )
        {
            update_height_vline();
            update_col_geom();
        }
    }

    Table::draw( cr );
}

void
WidgetTable::set_diary( Diary* diary )
{
    Table::set_diary( diary );
    m_WFP_entry->set_diary( diary );
    m_WFP_para->set_diary( diary );
    m_WFP_col_source_elem->set_diary( diary );
    m_WFP_column->set_diary( diary );
    refresh_editability(); // per the editability of the new diary
}

void
WidgetTable::set_entry_filter( const Filter* filter )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_filter_entry = filter;

    calculate_and_plot();
}

void
WidgetTable::set_para_filter( const Filter* filter )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_filter_para = filter;

    calculate_and_plot();
}

void
WidgetTable::set_line_cur( const DiaryElement* elem )
{
    m_line_id_cur = ( elem ? elem->get_id() : DEID_UNSET );
    update_i_line_cur();
    refresh();
}

void
WidgetTable::update_i_line_cur()
{
    if( m_line_id_cur != DEID_UNSET && !m_data.m_master_header.m_sublines.empty() )
    {
        int i{ 1 };
        for( auto& line : m_data.m_lines )
        {
            if( line->m_p2elem )
            {
                try
                {
                    DEID id{ m_data.m_F_para_based ?
                             dynamic_cast< Paragraph* >( line->m_p2elem )->m_host->get_id() :
                             line->m_p2elem->get_id() };
                    if( id == m_line_id_cur )
                    {
                        m_i_line_cur = i;
                        return;
                    }
                }
                catch( const std::exception& e )
                {
                    print_error( e.what() );
                    break;
                }
            }

            i++;
        }
    }

    m_i_line_cur = -1;
}

void
WidgetTable::handle_base_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.set_para_based( m_RB_para_based->get_active() );

    m_Bx_filter_para->set_visible( m_data.m_F_para_based );

    calculate_and_plot();
}

void
WidgetTable::handle_order_col_toggled()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.set_show_order_column( !m_data.has_order_column() );
    Lifeograph::p->get_action( "table_show_order_col" )->change_state( m_data.has_order_column() );

    update_col_geom();
    calculate_and_plot();
}

void
WidgetTable::handle_grouping_toggled()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    if( m_data.m_grouping_depth > 0 )
        m_data.m_grouping_depth = 0;
    else
        m_data.m_grouping_depth = 1;

    Lifeograph::p->get_action( "table_group_lines" )->change_state( m_data.m_grouping_depth > 0 );

    calculate_and_plot();
}

void
WidgetTable::show_column_Po()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_p2col_sel = m_data.m_columns[ m_i_col_cur ];

    m_E_col_name->set_text( m_p2col_sel->get_name() );

    m_CB_col_type->set_active( m_p2col_sel->get_type() );

    update_col_options();

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    place_column_Po();
    m_Po_column->show();
}

void
WidgetTable::place_column_Po()
{
    Gdk::Rectangle rect{ int( S_MARGIN + m_col_widths_cum[ m_i_col_cur ] ), int( S_MARGIN ),
                         int( m_col_widths[ m_i_col_cur ] ), int( m_row_h ) };

    // these need to be updated when column is moved that's why they are here
    m_B_col_move_prev->set_sensitive( m_i_col_cur > ( m_data.has_order_column() ? 1 : 0 ) );
    m_B_col_move_next->set_sensitive( m_i_col_cur < int( m_data.m_columns.size() - 1 ) );

    m_Po_column->set_pointing_to( rect );
}

void
WidgetTable::update_col_options()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    if( m_p2Bx_col_opts )
    {
        m_Bx_col_basics->remove( **m_p2Bx_col_opts );
        //*m_p2Bx_col_opts = nullptr;
        m_p2Bx_col_opts = nullptr;
    }

    switch( m_p2col_sel->get_type() )
    {
        case TableColumn::TCT_ARITHMETIC:   set_up_col_opts_arithmetic();   break;
        case TableColumn::TCT_SIZE:         set_up_col_opts_size();         break;
        case TableColumn::TCT_BOOL:         set_up_col_opts_bool();         break;
        case TableColumn::TCT_DATE:         set_up_col_opts_date();         break;
        case TableColumn::TCT_DURATION:     set_up_col_opts_duration();     break;
        case TableColumn::TCT_TAG_V:        set_up_col_opts_tag_value();    break;
        case TableColumn::TCT_TEXT:         set_up_col_opts_text();
            m_col_filter_classes = FOC::NUMBERS;
            break;
        case TableColumn::TCT_SUB:          set_up_col_opts_subtag();
            m_col_filter_classes = FOC::DIARYELEMS;
            break;
        default:
            break;
    }

    m_Bx_col_show_as->set_visible( m_p2col_sel->is_numeric() ||
                                   m_p2col_sel->get_type() == TableColumn::TCT_SUB );
    m_Bx_col_count_condtn->set_visible( m_p2col_sel->is_counting() );

    m_Po_column->present();

    m_Bx_col_source->set_visible( m_p2col_sel->get_type() != TableColumn::TCT_ARITHMETIC );

    // show as:
    m_DD_col_show_as->set_selected( VT::get_v< VT::SAS,
                                               unsigned,
                                               char >( m_p2col_sel->m_show_as ) );

    // summary func:
    m_DD_col_summary_func->set_selected( VT::get_v< VT::SUMF,
                                                    unsigned,
                                                    char >( m_p2col_sel->m_summary_func ) );

    update_source_elem_options();

    m_WFP_column->set_active( m_p2col_sel->get_count_filter() );
    m_WFP_column->set_obj_classes( m_col_filter_classes );

    if( const auto& filter = m_p2col_sel->get_count_filter() )
    {
        if( ! filter->can_filter_class( m_col_filter_classes ) )
            m_p2col_sel->set_count_filter( nullptr );
    }

    // sorting:
    if( m_p2col_sel->m_sort_order < 1 )
        m_RB_col_sort_off->set_active();
    else if( m_p2col_sel->m_sort_desc )
        m_RB_col_sort_dsc->set_active();
    else
        m_RB_col_sort_asc->set_active();

    m_RB_col_sort_off->set_visible( m_data.m_columns_sort.size() > 1 );

    m_TB_col_combine_same->set_visible( m_p2col_sel->m_sort_order > 0 );
    m_TB_col_combine_same->set_active( m_p2col_sel->is_combine_same() );

    // coloring:
    m_Bx_col_coloring->set_visible( m_p2col_sel->is_enumerable() );
    switch( m_p2col_sel->get_coloring_scheme() )
    {
        case TableColumn::Coloring::COLOR_OFF: m_RB_col_color_off->set_active(); break;
        case TableColumn::Coloring::NEG_RED:   m_RB_col_color_neg_red->set_active(); break;
        case TableColumn::Coloring::BIG_GREEN: m_RB_col_color_big_green->set_active(); break;
        case TableColumn::Coloring::BIG_RED:   m_RB_col_color_big_red->set_active(); break;
    }

    // dismiss:
    m_B_col_dismiss->set_visible( m_data.m_columns.size() > ( m_data.has_order_column() ? 2 : 1 ) );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetTable::update_source_elem_options()
{
    const auto src_elem { m_p2col_sel->get_source_elem() };

    if( m_data.m_F_para_based )
    {
        m_CB_col_source_entry->hide();
        m_CB_col_source_para->show();
        m_CB_col_source_para->set_active( src_elem );
    }
    else
    {
        m_CB_col_source_para->hide();
        m_CB_col_source_entry->show();
        m_CB_col_source_entry->set_active( src_elem );
    }

    update_source_elem_filters();
}

void
WidgetTable::update_source_elem_filters()
{
    const auto src_elem { m_p2col_sel->get_source_elem() };

    m_WFP_col_source_elem->set_visible( src_elem == VT::SRC_PARENT_FILTER ||
                                        src_elem > VT::SRC_ITSELF );
    m_WFP_col_source_elem->set_active( m_p2col_sel->get_source_elem_filter() );

    if( src_elem == VT::SRC_FCHILD_FILTER || src_elem == VT::SRC_FCHILD_FILTER )
        m_WFP_col_source_elem->set_obj_classes( FOC::PARAGRAPHS );
    else
        m_WFP_col_source_elem->set_obj_classes( FOC::ENTRIES );
}

void
WidgetTable::set_up_col_opts_date()
{
    // if( !m_Bx_col_opts_date )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_date.ui" );

        m_Bx_col_opts_date      = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_CB_date_type          = builder->get_widget< Gtk::ComboBoxText >( "CB_date_type" );
        m_ChB_date_custom_fmt   = builder->get_widget< Gtk::CheckButton >( "ChB_custom_format" );
        m_E_date_custom_format  = builder->get_widget< Gtk::Entry >( "E_custom_format" );

        // SIGNALS
        m_CB_date_type->signal_changed().connect(
                [ this ]() { handle_col_opt_int_chg(
                        CB_i_to_date_vt( m_CB_date_type->get_active_row_number() ) ); } );

        m_ChB_date_custom_fmt->signal_toggled().connect(
                [ this ]()
                { handle_col_date_format_type_chg(); } );

        m_E_date_custom_format->signal_changed().connect(
                [ this ]()
                { handle_col_date_format_chg(); } );
    }

    // CURRENT VALUES
    const int vt{ m_p2col_sel->get_opt_int() };
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_date_type->set_active( vt & VT::SEQ_FILTER );
    m_ChB_date_custom_fmt->set_active( vt & VT::DATE_CUSTOM_FORMAT );
    m_E_date_custom_format->set_sensitive( vt & VT::DATE_CUSTOM_FORMAT );
    m_E_date_custom_format->set_text( m_p2col_sel->m_opt_str );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_date, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_date;
}

void
WidgetTable::set_up_col_opts_bool()
{
    // if( !m_Bx_col_opts_bool )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_bool.ui" );

        m_Bx_col_opts_bool      = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_WFP_bool_condition    = Gtk::Builder::get_widget_derived< WidgetFilterPicker >(
                                        builder, "MB_filter" );

        m_WFP_bool_condition->set_clearable( true );
        m_WFP_bool_condition->set_show_edit_button( true );
        m_WFP_bool_condition->set_obj_classes( FOC::DIARYELEMS );

        // SIGNALS
        m_WFP_bool_condition->Sg_changed.connect(
            [ this ]( const Filter* filter )
            {
                handle_col_opt_int_chg( filter ? filter->get_id() : 0 );
            } );

        m_WFP_bool_condition->set_diary( m_p2diary );
    }

    // CURRENT VALUES
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_WFP_bool_condition->set_active( m_p2diary->get_filter( m_p2col_sel->get_opt_int() ) );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_bool, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_bool;
}

void
WidgetTable::set_up_col_opts_size()
{
    // if( !m_Bx_col_opts_size )
    {
        auto&& builder { Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_size.ui" );

        m_Bx_col_opts_size  = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_CB_col_size_type  = builder->get_widget< Gtk::ComboBoxText >( "CB_size_type" );

        // SIGNALS
        m_CB_col_size_type->signal_changed().connect(
                [ this ]() { handle_col_opt_int_chg(
                        VT::get_v< VT::SO,
                                   char,
                                   unsigned >( m_CB_col_size_type->get_active_row_number() ) ); } );
    }

    // CURRENT VALUES
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_col_size_type->set_active( VT::get_v< VT::SO,
                                               unsigned,
                                               char >( char( m_p2col_sel->get_opt_int() ) ) );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_size, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_size;
}

void
WidgetTable::set_up_col_opts_duration()
{
    // if( !m_Bx_col_opts_duration )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_duration.ui" );

        m_Bx_col_opts_duration  = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_CB_duration_bgn       = builder->get_widget< Gtk::ComboBoxText >( "CB_date_begin" );
        m_CB_duration_end       = builder->get_widget< Gtk::ComboBoxText >( "CB_date_end" );
        m_CB_duration_type      = builder->get_widget< Gtk::ComboBoxText >( "CB_duration_type" );

        // SIGNALS
        m_CB_duration_bgn->signal_changed().connect( [ this ](){ handle_col_opts_duratn_chg(); } );
        m_CB_duration_end->signal_changed().connect( [ this ](){ handle_col_opts_duratn_chg(); } );
        m_CB_duration_type->signal_changed().connect( [ this ](){ handle_col_opts_duratn_chg(); } );
    }

    // CURRENT VALUES
    const int vt{ m_p2col_sel->get_opt_int() };
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_duration_bgn->set_active( ( vt & VT::TCD_FILTER_BGN ) - 1 );
    m_CB_duration_end->set_active( ( ( vt & VT::TCD_FILTER_END ) >> 4 ) - 1 );
    m_CB_duration_type->set_active( ( ( vt & VT::TCD_FILTER_TYPE ) >> 8 ) );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_duration, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_duration;
}

void
WidgetTable::set_up_col_opts_tag_value()
{
    // if( !m_Bx_col_opts_tag_value )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_tag_value.ui" );

        m_Bx_col_opts_tag_value = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_WEP_subtag            = Gtk::Builder::get_widget_derived< WidgetEntryPicker >(
                                            builder, "E_tag_picker" );
        m_CB_tag_value_type     = builder->get_widget< Gtk::ComboBoxText >( "CB_value_type" );

        m_WEP_subtag->set_diary( m_p2diary );

        // SIGNALS
        m_WEP_subtag->signal_updated().connect(
                [ this ]( Entry* e ){ handle_col_tag_changed( e ); } );
        m_CB_tag_value_type->signal_changed().connect(
                [ this ](){ handle_col_opt_int_chg(
                        CB_i_to_tag_vt( m_CB_tag_value_type->get_active_row_number() ) ); } );
    }

    // CURRENT VALUES
    const int vt{ m_p2col_sel->get_opt_int() };
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_WEP_subtag->set_entry( m_p2col_sel->get_tag() );
    m_CB_tag_value_type->set_active( vt & VT::SEQ_FILTER );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_tag_value, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_tag_value;
}

void
WidgetTable::set_up_col_opts_subtag()
{
    // if( !m_Bx_col_opts_subtag )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_subtag.ui" );

        m_Bx_col_opts_subtag    = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_WEP_subtag            = Gtk::Builder::get_widget_derived< WidgetEntryPicker >(
                                        builder, "E_tag_picker" );
        m_CB_subtag_type        = builder->get_widget< Gtk::ComboBoxText >( "CB_subtag_type" );

        m_WEP_subtag->set_diary( m_p2diary );

        // SIGNALS
        m_WEP_subtag->signal_updated().connect(
                [ this ]( Entry* e ){ handle_col_tag_changed( e ); } );
        m_CB_subtag_type->signal_changed().connect(
                [ this ](){ handle_col_opt_int_chg(
                        m_CB_subtag_type->get_active_row_number() ); } );
    }

    // CURRENT VALUES
    const int vt{ m_p2col_sel->get_opt_int() };
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_WEP_subtag->set_entry( m_p2col_sel->get_tag() );
    m_CB_subtag_type->set_active( vt & VT::SEQ_FILTER );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_subtag, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_subtag;
}

void
WidgetTable::set_up_col_opts_text( bool F_update_only )
{
    if( !F_update_only )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_text.ui" );

        m_Bx_col_opts_text      = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_TB_text_incl_number   = builder->get_widget< Gtk::ToggleButton >( "TB_number" );
        m_TB_text_incl_plain    = builder->get_widget< Gtk::ToggleButton >( "TB_plain" );
        m_TB_text_incl_tags     = builder->get_widget< Gtk::ToggleButton >( "TB_tags" );
        m_TB_text_incl_dates    = builder->get_widget< Gtk::ToggleButton >( "TB_dates" );
        m_TB_text_incl_comnts   = builder->get_widget< Gtk::ToggleButton >( "TB_comments" );
        m_TB_text_incl_indents  = builder->get_widget< Gtk::ToggleButton >( "TB_indentations" );
        m_Bx_text_includes      = builder->get_widget< Gtk::Box >( "Bx_includes" );
        m_CB_text_source        = builder->get_widget< Gtk::ComboBoxText >( "CB_source_field" );

        // SIGNALS
        m_TB_text_incl_number->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );
        m_TB_text_incl_plain->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );
        m_TB_text_incl_tags->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );
        m_TB_text_incl_dates->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );
        m_TB_text_incl_comnts->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );
        m_TB_text_incl_indents->signal_toggled().connect(
                [ this ](){ handle_col_opts_text_chg(); } );

        m_CB_text_source->signal_changed().connect( [ this ]() { handle_col_opts_text_chg(); } );
    }

    // CURRENT VALUES
    const auto vt     { m_p2col_sel->get_opt_int() };
    const auto i_vt   { vt & VT::SEQ_FILTER };

    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_text_source->remove_all();

    if( m_p2col_sel->is_source_elem_para() )
    {
        m_CB_text_source->append( "T", _( "Text" ) ); // the same id and SRC_value as title
        m_CB_text_source->append( "U", _( "Uri" ) );
    }
    else
    {
        m_CB_text_source->append( "T", _( "Title" ) );
        m_CB_text_source->append( "A", _( "Ancestors + Title" ) );
        m_CB_text_source->append( "D", _( "Description" ) );
    }

    switch( i_vt )
    {
        case VT::TCT_SRC_ANCESTRAL:     m_CB_text_source->set_active_id( "A" ); break;
        case VT::TCT_SRC_DESCRIPTION:   m_CB_text_source->set_active_id( "D" ); break;
        case VT::TCT_SRC_TITLE:         m_CB_text_source->set_active_id( "T" ); break;
        case VT::TCT_SRC_URI:           m_CB_text_source->set_active_id( "U" ); break;
    }

    if( i_vt == VT::TCT_SRC_URI )
        m_Bx_text_includes->set_visible( false );
    else
    {
        m_Bx_text_includes->set_visible( true );
        m_TB_text_incl_number->set_active(  vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_NUMBER );
        m_TB_text_incl_plain->set_active(   vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_PLAIN );
        m_TB_text_incl_tags->set_active(    vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_TAG );
        m_TB_text_incl_dates->set_active(   vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_DATE );
        m_TB_text_incl_comnts->set_active(  vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_COMMENT );
        m_TB_text_incl_indents->set_active( vt & VT::TCT_FILTER_COMPONENT & VT::TCT_CMPNT_INDENT );
    }

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    if( !F_update_only )
    {
        m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_text, *m_CB_col_type );
        m_p2Bx_col_opts = &m_Bx_col_opts_text;
    }
}

#define set_up_RB( RB, action ) \
    RB->signal_toggled().connect( [ this ](){ if( RB->get_active() ) action; } );

void
WidgetTable::populate_arithmetic_col_opts( Gtk::DropDown* DD, Gtk::DropDown* DD_unr, Gtk::Entry* E,
                                           const int id_sel, const double val )
{
    VecUstrings options { { _( "Constant" ) } };
    int         i_sel   { get_arith_opt_i_from_col_id( id_sel ) };

    for( auto col : m_data.m_columns )
    {
        if( col->m_index < m_p2col_sel->m_index && col->is_numeric() )
            options.push_back( col->get_name() );
    }

    DD->set_model( Gtk::StringList::create( options ) );
    DD->set_selected( i_sel );

    E->set_visible( i_sel == 0 );

    if( i_sel == 0 )
        E->set_text( STR::format_number( val ) );

    DD_unr->set_visible( i_sel > 0 );
}
int
WidgetTable::get_col_id_from_arith_opt_i( const int opt_col_i ) const
{
    int i { 1 };

    for( auto col : m_data.m_columns )
    {
        if( col->m_index >= m_p2col_sel->m_index ) break;
        if( col->is_numeric() )
        {
            if( i == opt_col_i )
                return col->m_id;
            else
                ++i;
        }
    }

    return 0;
}
int
WidgetTable::get_arith_opt_i_from_col_id( const int opt_col_id ) const
{
    int i { 1 };

    for( auto col : m_data.m_columns )
    {
        if( col->m_index >= m_p2col_sel->m_index ) break;
        if( col->is_numeric() )
        {
            if( col->m_id == opt_col_id )
                return i;
            else
                ++i;
        }
    }

    return 0;
}

void
WidgetTable::set_up_col_opts_arithmetic()
{
    // if( !m_Bx_col_opts_arithmetic )
    {
        auto&& builder{ Lifeograph::get_builder() }; //Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/col_opt_arithmetic.ui" );

        m_Bx_col_opts_arithmetic    = builder->get_widget< Gtk::Box >( "Bx_container" );
        m_DD_arith_opA_unary        = builder->get_widget< Gtk::DropDown >( "DD_opA_unary" );
        m_DD_arith_opB_unary        = builder->get_widget< Gtk::DropDown >( "DD_opB_unary" );
        m_DD_arith_opA_type         = builder->get_widget< Gtk::DropDown >( "DD_opA_type" );
        m_DD_arith_opB_type         = builder->get_widget< Gtk::DropDown >( "DD_opB_type" );
        m_E_arith_opA               = builder->get_widget< Gtk::Entry >( "E_opA_v" );
        m_E_arith_opB               = builder->get_widget< Gtk::Entry >( "E_opB_v" );
        m_RB_arithmetic_add         = builder->get_widget< Gtk::ToggleButton >( "RB_add" );
        m_RB_arithmetic_sub         = builder->get_widget< Gtk::ToggleButton >( "RB_sub" );
        m_RB_arithmetic_mul         = builder->get_widget< Gtk::ToggleButton >( "RB_mul" );
        m_RB_arithmetic_div         = builder->get_widget< Gtk::ToggleButton >( "RB_div" );
        m_RB_arithmetic_pow         = builder->get_widget< Gtk::ToggleButton >( "RB_pow" );
        m_RB_arithmetic_roo         = builder->get_widget< Gtk::ToggleButton >( "RB_rot" );
        m_RB_arithmetic_mod         = builder->get_widget< Gtk::ToggleButton >( "RB_mod" );
        m_DD_arith_format           = builder->get_widget< Gtk::DropDown >( "DD_format" );

        // SIGNALS
        m_DD_arith_opA_unary->property_selected().signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCAu::FILTER_A ); } );
        m_DD_arith_opB_unary->property_selected().signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCAu::FILTER_B ); } );
        m_DD_arith_opA_type->property_selected().signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCA_FLAG_OPD_A ); } );
        m_DD_arith_opB_type->property_selected().signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCA_FLAG_OPD_B ); } );
        m_E_arith_opA->signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCA_FLAG_CONST_A ); } );
        m_E_arith_opB->signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCA_FLAG_CONST_B ); } );
        set_up_RB( m_RB_arithmetic_add, handle_col_opts_arithmetic_chg( VT::TCAo::ADD::I ) );
        set_up_RB( m_RB_arithmetic_sub, handle_col_opts_arithmetic_chg( VT::TCAo::SUB::I ) );
        set_up_RB( m_RB_arithmetic_mul, handle_col_opts_arithmetic_chg( VT::TCAo::MUL::I ) );
        set_up_RB( m_RB_arithmetic_div, handle_col_opts_arithmetic_chg( VT::TCAo::DIV::I ) );
        set_up_RB( m_RB_arithmetic_pow, handle_col_opts_arithmetic_chg( VT::TCAo::POW::I ) );
        set_up_RB( m_RB_arithmetic_roo, handle_col_opts_arithmetic_chg( VT::TCAo::ROO::I ) );
        set_up_RB( m_RB_arithmetic_mod, handle_col_opts_arithmetic_chg( VT::TCAo::MOD::I ) );
        m_DD_arith_format->property_selected().signal_changed().connect(
                [ this ](){ handle_col_opts_arithmetic_chg( VT::TCAf::FILTER ); } );
    }

    // CURRENT VALUES
    const auto vt { m_p2col_sel->get_opt_int() };

    Lifeograph::START_INTERNAL_OPERATIONS();

    switch( vt & VT::TCAo::FILTER )
    {
        case VT::TCAo::ADD::I: m_RB_arithmetic_add->set_active(); break;
        case VT::TCAo::SUB::I: m_RB_arithmetic_sub->set_active(); break;
        case VT::TCAo::MUL::I: m_RB_arithmetic_mul->set_active(); break;
        case VT::TCAo::DIV::I: m_RB_arithmetic_div->set_active(); break;
        case VT::TCAo::POW::I: m_RB_arithmetic_pow->set_active(); break;
        case VT::TCAo::ROO::I: m_RB_arithmetic_roo->set_active(); break;
        case VT::TCAo::MOD::I: m_RB_arithmetic_mod->set_active(); break;
    }

    const VecUstrings unary_opts( { VT::TCAu::NON::S, VT::TCAu::MNS::S,
                                    VT::TCAu::RCP::S, VT::TCAu::ABS::S } );
    const auto        strlist_unary { Gtk::StringList::create( unary_opts ) };
    m_DD_arith_opA_unary->set_model( strlist_unary );
    m_DD_arith_opA_unary->set_selected( VT::get_v< VT::TCAu,
                                                   unsigned, int >( vt & VT::TCAu::FILTER_A ) );
    m_DD_arith_opB_unary->set_model( strlist_unary );
    m_DD_arith_opB_unary->set_selected( VT::get_v< VT::TCAu,
                                                   unsigned, int >( ( vt & VT::TCAu::FILTER_B ) >> 4 ) );

    populate_arithmetic_col_opts( m_DD_arith_opA_type, m_DD_arith_opA_unary, m_E_arith_opA,
                                  m_p2col_sel->m_opt_int1, m_p2col_sel->m_opt_double1 );
    populate_arithmetic_col_opts( m_DD_arith_opB_type, m_DD_arith_opB_unary, m_E_arith_opB,
                                  m_p2col_sel->m_opt_int2, m_p2col_sel->m_opt_double2 );

    const VecUstrings format_opts( { VT::TCAf::INT::S, VT::TCAf::REA::S,
                                     VT::TCAf::RE2::S, VT::TCAf::PTG::S } );
    m_DD_arith_format->set_model( Gtk::StringList::create( format_opts ) );
    m_DD_arith_format->set_selected( VT::get_v< VT::TCAf, unsigned, int >( vt & VT::TCAf::FILTER ) );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    // ATTACH
    m_Bx_col_basics->insert_child_after( *m_Bx_col_opts_arithmetic, *m_CB_col_type );
    m_p2Bx_col_opts = &m_Bx_col_opts_arithmetic;
}

void
WidgetTable::handle_col_renamed()
{
    m_p2col_sel->set_name( m_E_col_name->get_text() );
    refresh();
    m_Sg_changed.emit(); // in the absence of calculate_and_plot()
}

void
WidgetTable::handle_col_type_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    if( !m_p2col_sel->set_type( TableColumn::Type( m_CB_col_type->get_active_row_number() ) ) )
        return;

    m_p2col_sel->m_p2tag = nullptr;
    m_p2col_sel->m_opt_str.clear();

    // reset type options
    switch( m_p2col_sel->get_type() )
    {
        case TableColumn::TCT_ARITHMETIC:
            m_p2col_sel->set_opt_int( VT::TCA_DEFAULT );
            m_p2col_sel->m_opt_double1 = 0.0;
            m_p2col_sel->m_opt_double2 = 0.0;
            break;
        case TableColumn::TCT_TEXT:
            m_p2col_sel->set_opt_int( VT::TCT_DEFAULT );
            break;
        case TableColumn::TCT_SIZE:
            m_p2col_sel->set_opt_int( VT::SO::DEFAULT::C );
            break;
        case TableColumn::TCT_DATE:
            m_p2col_sel->set_opt_int( VT::DATE_START );
            m_p2col_sel->m_opt_str = Date::get_format_str_default();
            break;
        // case TableColumn::TCT_BOOL:
        //     m_p2col_sel->set_opt_int( 0 );
        //     break;
        case TableColumn::TCT_DURATION:
            m_p2col_sel->set_opt_int( VT::TCD_DEFAULT );
            break;
        case TableColumn::TCT_SUB:
            m_p2col_sel->set_opt_int( VT::FIRST );
            break;
        case TableColumn::TCT_TAG_V:
            m_p2col_sel->set_opt_int( VT::TOTAL_REALIZED );
            break;
        default:
            m_p2col_sel->set_opt_int( 0 );
            break;
    }
    update_col_options();
    calculate_and_plot();
}

void
WidgetTable::handle_col_source_elem_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    auto source_elem_opt{ m_data.m_F_para_based ? m_CB_col_source_para->get_active_row_number()
                                                : m_CB_col_source_entry->get_active_row_number() };
    m_p2col_sel->set_source_elem( source_elem_opt );

    update_source_elem_filters();

    if( m_p2col_sel->get_type() == TableColumn::TCT_TEXT )
    {
        m_p2col_sel->set_opt_int_field( VT::SEQ_FILTER, VT::TCT_SRC_TITLE );
        set_up_col_opts_text( true ); // update per the source being a para or entry
    }

    calculate_and_plot();
}

void
WidgetTable::handle_col_tag_changed( Entry* tag )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->set_tag( tag );

    calculate_and_plot();
}

void
WidgetTable::handle_col_opt_int_chg( int32_t vt )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;
    m_p2col_sel->set_opt_int( vt );
    calculate_and_plot();
}

void
WidgetTable::handle_col_date_format_type_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    if( m_ChB_date_custom_fmt->get_active() )
    {
        m_E_date_custom_format->set_sensitive( true );
        m_p2col_sel->m_opt_int |= VT::DATE_CUSTOM_FORMAT;
    }
    else
    {
        //m_p2col_sel->m_opt_str = Date::get_format_str_default();
        m_E_date_custom_format->set_sensitive( false );
        m_E_date_custom_format->set_text( Date::get_format_str_default() );
        m_p2col_sel->m_opt_int &= ~( VT::DATE_CUSTOM_FORMAT );
    }

    calculate_and_plot();
}

void
WidgetTable::handle_col_date_format_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->m_opt_str = m_E_date_custom_format->get_text();
    calculate_and_plot();
}

void
WidgetTable::handle_col_opts_duratn_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->set_opt_int( ( m_CB_duration_bgn->get_active_row_number() + 1 ) |
                              ( ( m_CB_duration_end->get_active_row_number() + 1 ) << 4 ) |
                              ( m_CB_duration_type->get_active_row_number() << 8 ) );

    calculate_and_plot();
}

void
WidgetTable::handle_col_opts_text_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    int flags{ 0 };

    switch( m_CB_text_source->get_active_id()[ 0 ] )
    {
        case 'A': flags = VT::TCT_SRC_ANCESTRAL; break;
        case 'D': flags = VT::TCT_SRC_DESCRIPTION; break;
        case 'T': flags = VT::TCT_SRC_TITLE; break;
        case 'U': flags = VT::TCT_SRC_URI; break;
    }

    m_Bx_text_includes->set_visible( flags != VT::TCT_SRC_URI );

    if( m_TB_text_incl_number->get_active() )   flags |= VT::TCT_CMPNT_NUMBER;
    if( m_TB_text_incl_plain->get_active() )    flags |= VT::TCT_CMPNT_PLAIN;
    if( m_TB_text_incl_tags->get_active() )     flags |= VT::TCT_CMPNT_TAG;
    if( m_TB_text_incl_dates->get_active() )    flags |= VT::TCT_CMPNT_DATE;
    if( m_TB_text_incl_comnts->get_active() )   flags |= VT::TCT_CMPNT_COMMENT;
    if( m_TB_text_incl_indents->get_active() )  flags |= VT::TCT_CMPNT_INDENT;

    m_p2col_sel->set_opt_int( flags );

    calculate_and_plot();
}

void
WidgetTable::handle_col_opts_arithmetic_chg( const int32_t type )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    if     ( type & VT::TCAo::FILTER )
    {
        m_p2col_sel->m_opt_int = ( m_p2col_sel->m_opt_int & ~VT::TCAo::FILTER ) |
                                 ( type & VT::TCAo::FILTER );
    }
    else if( type == VT::TCAu::FILTER_A )
    {
        const auto um_A { VT::get_v< VT::TCAu, int, unsigned >( m_DD_arith_opA_unary->get_selected() ) };
        m_p2col_sel->m_opt_int = ( m_p2col_sel->m_opt_int & ~VT::TCAu::FILTER_A ) | um_A;
    }
    else if( type == VT::TCAu::FILTER_B )
    {
        const auto um_B { VT::get_v< VT::TCAu, int, unsigned >( m_DD_arith_opB_unary->get_selected() ) };
        m_p2col_sel->m_opt_int = ( m_p2col_sel->m_opt_int & ~VT::TCAu::FILTER_B ) | ( um_B << 4 );
    }
    else if( type == VT::TCA_FLAG_OPD_A )
    {
        const auto i_opA { m_DD_arith_opA_type->get_selected() };
        m_p2col_sel->m_opt_int1 = ( i_opA == 0 ? 0 : get_col_id_from_arith_opt_i( i_opA ) );
        m_E_arith_opA->set_visible( i_opA == 0 );
        m_DD_arith_opA_unary->set_visible( i_opA > 0 );
    }
    else if( type == VT::TCA_FLAG_OPD_B )
    {
        const auto i_opB { m_DD_arith_opB_type->get_selected() };
        m_p2col_sel->m_opt_int2 = ( i_opB == 0 ? 0 : get_col_id_from_arith_opt_i( i_opB ) );
        m_E_arith_opB->set_visible( i_opB == 0 );
        m_DD_arith_opB_unary->set_visible( i_opB > 0 );
    }
    else if( type == VT::TCA_FLAG_CONST_A )
        m_p2col_sel->m_opt_double1 = STR::get_d( m_E_arith_opA->get_text() );
    else if( type == VT::TCA_FLAG_CONST_B )
        m_p2col_sel->m_opt_double2 = STR::get_d( m_E_arith_opB->get_text() );

    else if( type == VT::TCAf::FILTER )
    {
        const auto format { VT::get_v< VT::TCAf, int, unsigned >( m_DD_arith_format->get_selected() ) };
        m_p2col_sel->m_opt_int = ( ( m_p2col_sel->m_opt_int & ~VT::TCAf::FILTER ) | format );
    }

    calculate_and_plot();
}

void
WidgetTable::handle_col_combine_same_toggled()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->set_combine_same( !m_p2col_sel->is_combine_same() );

    calculate_and_plot();
}

void
WidgetTable::handle_col_show_as_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->m_show_as = VT::get_v< VT::SAS,
                                        char,
                                        unsigned >( m_DD_col_show_as->get_selected() );

    m_Bx_col_count_condtn->set_visible( m_p2col_sel->is_counting() );
    calculate_and_plot();
}

void
WidgetTable::handle_col_summary_func_chg()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_p2col_sel->m_summary_func = VT::get_v< VT::SUMF,
                                             char,
                                             unsigned >( m_DD_col_summary_func->get_selected() );

    calculate_and_plot();
}

void
WidgetTable::handle_col_color_chg( Gtk::ToggleButton* rb )
{
    if( Lifeograph::is_internal_operations_ongoing() || !rb->get_active() ) return;

    if     ( rb == m_RB_col_color_big_green )
        m_p2col_sel->set_coloring_scheme( TableColumn::Coloring::BIG_GREEN );
    else if( rb == m_RB_col_color_neg_red )
        m_p2col_sel->set_coloring_scheme( TableColumn::Coloring::NEG_RED );
    else if( rb == m_RB_col_color_big_red )
        m_p2col_sel->set_coloring_scheme( TableColumn::Coloring::BIG_RED );
    else //if( rb == m_RB_col_color_off )
        m_p2col_sel->set_coloring_scheme( TableColumn::Coloring::COLOR_OFF );

    refresh();
    m_Sg_changed.emit();
}

/* APPARENTLY NOT USED void
WidgetTable::show_filter_editor( const Filter* filter, int focs )
{
    if( !filter ) return;

    if( m_dialog_filter == nullptr )
    {
        static auto&& builder{ Gtk::Builder::create() };
        Lifeograph::load_gui( builder, Lifeograph::SHAREDIR + "/ui/dlg_filter.ui" );
        m_dialog_filter = Gtk::Builder::get_widget_derived< DialogFilter >( builder, "D_filter" );
    }

    m_dialog_filter->set_filter( const_cast< Filter* >( filter ), focs );
    m_dialog_filter->present();
    m_dialog_filter->signal_response().connect(
        [ & ]( int rv )
        {
            if( rv == Result::OK )
            {
                m_p2col_sel->set_source_elem_filter( filter );
                calculate_and_plot( false );

            }
        },
        true );
} */

void
WidgetTable::calculate_and_plot( bool F_emit_changed )
{
    m_i_line_top = 0;
    m_data.populate_lines( false );
    update_line_c_vis();
    update_i_line_cur();

    if( m_row_h > 0.0 ) // ignore pre size allocate call
    {
        calculate_row_h();
        update_height_vline();

        if( !update_scrollbar() )
            refresh();

        if( F_emit_changed )
            m_Sg_changed.emit();
    }
}

void
WidgetTable::sort_and_plot()
{
    m_i_line_top = 0;
    m_data.resort_lines();

    if( !update_scrollbar() )
        refresh();

    m_Sg_changed.emit();
}

void
WidgetTable::update_visible_lines_and_plot( bool F_reset_scroll_pos )
{
    m_data.update_lines_cache();

    if( F_reset_scroll_pos || m_line_c_max >= m_data.m_lines.size() )
        m_i_line_top = 0;

    update_line_c_vis();
    update_i_line_cur();
    if( !update_scrollbar() )
    {
        update_height_vline();
        refresh();
    }

    //m_Sg_changed.emit();
}

void
WidgetTable::remove_entry( Entry* entry )
{
    // a call to remove_entry() must always be accompanied by a subsequent call to add_entry()

    if( m_data.m_columns.empty() ) return;

    m_F_needs_update = m_data.m_master_header.remove_entry( entry );
        // m_data.update_lines_cache();
}
void
WidgetTable::add_entry( Entry* entry )
{
    if( !m_data.populate_lines( false, entry ) && !m_F_needs_update ) return;

    // TODO: 3.2: try to maintain the expanded statuses

    update_line_c_vis();
    update_scrollbar();
    update_height_vline();
    refresh();

    m_F_needs_update = false;

    PRINT_DEBUG( "TABLE UPDATED!!!" );
}

bool // true if Sb visibility changes
WidgetTable::update_scrollbar()
{
    const bool F_Sb_was_visible{ m_Sb_vert->get_visible() };

    if( m_line_c_max < m_data.m_lines.size() )
    {
        m_Sb_vert->show();
        m_Aj_vert->configure( m_i_line_top, 0.0, m_data.m_lines.size() + S_MIN_LINES,
                              2.0, m_line_c_max, m_line_c_max );

        return !F_Sb_was_visible;
    }
    else
    {
        // m_Aj_vert->configure( 0.0, 0.0, m_data.m_lines.size() + S_MIN_LINES - 1,
        //                       2.0, m_line_c_max, m_line_c_max );

        m_Sb_vert->hide();

        return F_Sb_was_visible;
    }
}

bool
WidgetTable::is_pt_on_col_boundary( int x_coord )
{
    const static auto&& s_Cu_resize{ Gdk::Cursor::create( "ew-resize" ) };
    const static auto&& s_Cu_default{ Gdk::Cursor::create( "default" ) };
    static const Glib::RefPtr< Gdk::Cursor >* s_p2cursor_last{ &s_Cu_default };

    if( x_coord == -1 ) // do not recalculate
        return( s_p2cursor_last == &s_Cu_resize );

    for( unsigned int i = 1; i < m_col_widths_cum.size() - 1; i++ )
    {
        int width_cumulative = m_col_widths_cum[ i ] + S_MARGIN;
        if( width_cumulative - 3 < x_coord && x_coord < width_cumulative + 3 )
        {
            if( s_p2cursor_last != &s_Cu_resize )
                set_cursor( *( s_p2cursor_last = &s_Cu_resize ) );
            return true;
        }
    }

    if( s_p2cursor_last != &s_Cu_default )
        set_cursor( *( s_p2cursor_last = &s_Cu_default ) );
    return false;
}

void
WidgetTable::enter_column_resize_mode( int x_coord )
{
    m_x_coord_drag_start = x_coord;

    for( unsigned int i = 1; i < m_col_widths_cum.size() - 1; i++ )
    {
        int width_cumulative = m_col_widths_cum[ i ] + S_MARGIN;
        if( width_cumulative - 3 < x_coord && x_coord < width_cumulative + 3 )
        {
            m_col_i_drag = i - 1;
            m_col_width_drag_L = m_col_widths[ i - 1 ];
            m_col_width_drag_R = m_col_widths[ i ];
            return;
        }
    }
}

void
WidgetTable::exit_column_resize_mode()
{
    m_x_coord_drag_start = -1;
    m_Sg_changed.emit(); // to update the definition
}

void
WidgetTable::set( const TableElem* table, const DiaryElement* elem_cur )
{
    if( !table )
    {
        m_data.clear();
        return;
    }

    m_data.set_from_string( table->get_definition() );
    m_line_id_cur = ( elem_cur ? elem_cur->get_id() : DEID_UNSET );
    m_i_line_hover = -1;

    Lifeograph::START_INTERNAL_OPERATIONS();

    Lifeograph::p->get_action( "table_show_order_col" )->change_state( m_data.has_order_column() );
    Lifeograph::p->get_action( "table_group_lines" )->change_state( m_data.m_grouping_depth > 0 );

    m_WFP_entry->set_active( m_data.m_filter_entry );
    m_WFP_para->set_active( m_data.m_filter_para );

    if( m_data.m_F_para_based )
        m_RB_para_based->set_active();
    else
        m_RB_entry_based->set_active();
    m_Bx_filter_para->set_visible( m_data.m_F_para_based );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    update_col_geom();
    calculate_and_plot( false );
}

void
WidgetTable::copy_delimited_text()
{
    Ustring str;
    bool f_first{ true };

    for( auto& col : m_data.m_columns )
    {
        if( f_first )
            f_first = false;
        else
            str += '\t';

        str += col->get_name();
    }

    str += '\n';

    for( unsigned int i = 0; i < m_data.m_lines.size(); i++ )
    {
        f_first = true;

        for( ListTableColumns::size_type j = 0; j < m_data.m_columns.size(); j++ )
        {
            if( f_first )
                f_first = false;
            else
                str += '\t';

            str += m_data.get_value_str( i, j );
        }

        str += '\n';
    }

    get_clipboard()->set_text( str );
}
