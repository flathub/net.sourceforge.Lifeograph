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
#include "../app_window.hpp"
#include "../strings.hpp"
#include "../diaryelements/diarydata.hpp"
#include "widget_chart.hpp"


using namespace LIFEO;


// WIDGETCHART =====================================================================================
WidgetChart::WidgetChart( BaseObjectType* cobj, const Glib::RefPtr< Gtk::Builder >& refbuilder )
:   Gtk::DrawingArea( cobj )
{
    m_builder           = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/chart.ui" );

    m_Po_properties     = m_builder->get_widget< Gtk::Popover >( "Po_properties" );
    m_RB_style_line     = m_builder->get_widget< Gtk::ToggleButton >( "RB_style_line" );
    m_RB_style_bars     = m_builder->get_widget< Gtk::ToggleButton >( "RB_style_bars" );
    // m_RB_style_pie      = m_builder->get_widget< Gtk::ToggleButton >( "RB_style_pie" );
    m_CB_table          = m_builder->get_widget< Gtk::ComboBoxText >( "CB_table" );
    m_CB_x_axis         = m_builder->get_widget< Gtk::ComboBoxText >( "CB_x_axis" );
    m_CB_y_axis         = m_builder->get_widget< Gtk::ComboBoxText >( "CB_y_axis" );
    m_Bx_grouping       = m_builder->get_widget< Gtk::Box >( "Bx_grouping" );
    m_CB_underlay       = m_builder->get_widget< Gtk::ComboBoxText >( "CB_underlay" );
    m_CB_period         = m_builder->get_widget< Gtk::ComboBoxText >( "CB_period" );
    m_CB_combining      = m_builder->get_widget< Gtk::ComboBoxText >( "CB_combining" );
    m_CB_col_filter     = m_builder->get_widget< Gtk::ComboBoxText >( "CB_col_filter"  );
    m_Bx_col_filter_v   = m_builder->get_widget< Gtk::Box >( "Bx_col_filter_v"  );
    m_DD_col_filter_v   = m_builder->get_widget< Gtk::DropDown >( "DD_col_filter_v" );


    auto expression = Gtk::ClosureExpression< Ustring >::create(
            []( const Glib::RefPtr< Glib::ObjectBase >& item ) -> Ustring
            {
                // An item in a StringList is a StringObject.
                const auto string_object = std::dynamic_pointer_cast< Gtk::StringObject >( item );
                return string_object ? string_object->get_string() : "";
            } );
    m_DD_col_filter_v->set_expression( expression );

    m_Po_properties->signal_show().connect(
            [ this ]()
            {
                refresh_diary();
                update_child_widgets_per_data();
            } );

    m_CB_table->signal_changed().connect( [ this ](){ handle_table_changed(); } );
    m_CB_x_axis->signal_changed().connect( [ this ](){ handle_x_axis_changed(); } );
    m_CB_y_axis->signal_changed().connect( [ this ](){ handle_y_axis_changed(); } );
    m_CB_underlay->signal_changed().connect( [ this ](){ handle_underlay_changed(); } );
    m_CB_period->signal_changed().connect( [ this ](){ handle_period_changed(); } );
    m_CB_combining->signal_changed().connect( [ this ](){ handle_combining_changed(); } );

    m_CB_col_filter->signal_changed().connect( [ this ](){ handle_col_filter_changed(); } );
    m_DD_col_filter_v->property_selected().signal_changed().connect(
            [ this ](){ handle_col_filter_v_changed(); } );

    m_RB_style_line->signal_toggled().connect( [ this ](){ handle_style_changed(); } );
    m_RB_style_bars->signal_toggled().connect( [ this ](){ handle_style_changed(); } );

    // DRAWING
    set_draw_func( sigc::mem_fun( *this, &WidgetChart::on_draw ) );

    // EVENT CONTROLLERS
    auto controller_scroll = Gtk::EventControllerScroll::create();
    controller_scroll->signal_scroll().connect(
            sigc::mem_fun( *this, &WidgetChart::on_scroll_event ), false );
    controller_scroll->set_flags( Gtk::EventControllerScroll::Flags::VERTICAL );
    add_controller( controller_scroll );

    m_gesture_click = Gtk::GestureClick::create();
    m_gesture_click->signal_pressed().connect(
            sigc::mem_fun( *this, &WidgetChart::on_button_press_event ), false );
    m_gesture_click->signal_released().connect(
            sigc::mem_fun( *this, &WidgetChart::on_button_release_event ), false );
    add_controller( m_gesture_click );

    auto controller_motion = Gtk::EventControllerMotion::create();
    controller_motion->signal_motion().connect(
            sigc::mem_fun( *this, &WidgetChart::on_motion_notify_event ), false );
    controller_motion->signal_leave().connect(
            sigc::mem_fun( *this, &WidgetChart::on_leave_notify_event ), false );
    add_controller( controller_motion );
}

void
WidgetChart::set_helper_widgets( Gtk::MenuButton* MB_properties, Gtk::Scale* Sc_zoom )
{
    m_Sc_zoom = Sc_zoom;
    MB_properties->set_popover( *m_Po_properties );

    Sc_zoom->signal_value_changed().connect(
            [ this ]()
            {
                if( ! Lifeograph::is_internal_operations_ongoing() )
                    set_zoom( m_Sc_zoom->get_value() );
            } );
}

void
WidgetChart::set_diary( Diary* diary )
{
    Chart::set_diary( diary );
    refresh_editabilitiy(); // per the editability of the new diary
}

void
WidgetChart::refresh_diary()
{
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_table->remove_all();

    m_CB_table->append( std::to_string( TABLE_ID_ALL_ENTRIES ),
                        STR::compose( "<", STR0/SI::ALL_ENTRIES, ">" ) );
    m_CB_table->append( std::to_string( TABLE_ID_ALL_PARAGRAPHS ),
                        STR::compose( "<", STR0/SI::ALL_PARAGRAPHS, ">" ) );
    for( auto kv_table : m_data.m_p2diary->get_tables() )
        m_CB_table->append( std::to_string( kv_table.second->get_id() ), kv_table.first );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetChart::remove_entry( Entry* entry )
{
    for( auto& vd : m_data.values_date )
    {
        for( auto&& it = vd.second.elems.begin(); it != vd.second.elems.end(); )
        {
            if( ( *it )->get_type() == DiaryElement::ET_PARAGRAPH )
            {
                auto para{ dynamic_cast< Paragraph* >( *it ) };
                if( para->m_host == entry )
                    it = vd.second.elems.erase( it );
                else
                    ++it;
            }
            else
            {
                if( ( *it )->get_id() == entry->get_id() )
                    it = vd.second.elems.erase( it );
                else
                    ++it;
            }
        }
    }
}

void
WidgetChart::handle_table_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.set_table( std::stoul( m_CB_table->get_active_id() ) );

    update_child_widgets_per_data(); // to update the axis options
    calculate_and_plot();
}

void
WidgetChart::handle_x_axis_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_tcidx = std::stoi( m_CB_x_axis->get_active_id() );

    m_data.refresh_type();
    // m_RB_style_pie->set_visible( m_data.get_type() != ChartData::TYPE_DATE );
    m_Bx_grouping->set_visible( m_data.get_type() == ChartData::TYPE_DATE );
    update_CB_underlay();

    calculate_and_plot();
}

void
WidgetChart::handle_y_axis_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_tcidy = std::stoi( m_CB_y_axis->get_active_id() );

    m_data.refresh_unit();
    calculate_and_plot();
}

void
WidgetChart::handle_underlay_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_tcidu = std::stoi( m_CB_underlay->get_active_id() );

    calculate_and_plot();
}

void
WidgetChart::handle_style_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    if     ( m_RB_style_bars->get_active() ) m_data.set_properties( ChartData::STYLE_BARS );
    // else if( m_RB_style_pie->get_active() )  m_data.set_properties( ChartData::STYLE_PIE );
    else                                     m_data.set_properties( ChartData::STYLE_LINE );

    calculate_and_plot();
}

void
WidgetChart::handle_period_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.set_properties( m_CB_period->get_active_row_number() + 1 );
    calculate_and_plot();
}

void
WidgetChart::handle_combining_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.set_properties(
            ( m_CB_combining->get_active_row_number() + 1 ) << ChartData::COMBINE_SHIFT );
    calculate_and_plot();
}

void
WidgetChart::handle_col_filter_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_data.m_tcidf = std::stoi( m_CB_col_filter->get_active_id() );
    m_data.m_filter_v = "";

    update_CB_filter_v();

    calculate_and_plot();
}

void
WidgetChart::handle_col_filter_v_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    auto  it_filter_v   { m_col_filter_values.begin() };
    auto  i_selected_v  { m_DD_col_filter_v->get_selected() };

    if( i_selected_v == 0 )
        m_data.m_filter_v = "";
    else
    {
        std::advance( it_filter_v, i_selected_v - 1 );
        m_data.m_filter_v = *it_filter_v;
    }

    calculate_and_plot();
}

void
WidgetChart::calculate_and_plot( bool F_emit_changed )
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_data.calculate_points();
    update_h_x_values();
    set_zoom( m_data.get_type() == ChartData::TYPE_DATE ? 1.0 : 0.0 );
    // no zoom out in charts except when x=date

    m_Sc_zoom->set_value( m_zoom_level );
    m_Sc_zoom->set_visible( is_zoom_possible() );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    if( F_emit_changed )
        m_Sg_changed.emit();
}

void
WidgetChart::update_CB_underlay()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_CB_underlay->remove_all();

    // PRESET VALUES
    m_CB_underlay->append( std::to_string( ChartData::COLUMN_NONE ),
                           STR::compose( "<", _( "None" ), ">" ) );
    if( m_data.get_type() == ChartData::TYPE_DATE )
        m_CB_underlay->append( std::to_string( ChartData::COLUMN_PREV_YEAR ),
                               STR::compose( "<", _( "Previous Year" ), ">" ) );

    // COLUMN VALUES
    for( auto col : m_data.m_td->m_columns )
    {
        if( col->is_numeric() )
            m_CB_underlay->append( std::to_string( col->m_id ), col->get_name() );
    }

    m_CB_underlay->set_active_id( std::to_string( m_data.m_tcidu ) );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetChart::update_CB_filter_v()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    auto        col_filter        { m_data.m_td->get_column_by_id( m_data.m_tcidf ) };
    VecUstrings unique_lines_vec;
    unsigned    i_selected_v      { 0 };

    m_col_filter_values.clear();
    unique_lines_vec.push_back( STR::compose( "<", _( "None" ), ">" ) );

    // COLUMN VALUES
    if( col_filter )
    {
        for( unsigned il = 0; il < m_data.m_td->m_lines.size(); ++il )
        {
            const auto str_val { m_data.m_td->get_value_str( il, col_filter->get_index() ) };
            m_col_filter_values.insert( str_val );
        }

        for( auto str : m_col_filter_values )
        {
            if( !m_data.m_filter_v.empty() && str == m_data.m_filter_v )
                i_selected_v = unique_lines_vec.size();

            unique_lines_vec.push_back( str.substr( 0, 15 ) );
        }
    }

    m_DD_col_filter_v->set_model( Gtk::StringList::create( unique_lines_vec ) );
    m_DD_col_filter_v->set_selected( m_data.m_filter_v.empty() ? 0 : i_selected_v );

    m_Bx_col_filter_v->set_visible( col_filter );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetChart::update_child_widgets_per_data()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    // VISIBILITIES
    // m_RB_style_pie->set_visible( m_data.get_type() != ChartData::TYPE_DATE );
    m_Bx_grouping->set_visible( m_data.get_type() == ChartData::TYPE_DATE );

    // STYLE
    switch( m_data.get_style() )
    {
        default:
            m_RB_style_line->set_active();
            break;
        case ChartData::STYLE_BARS:
            m_RB_style_bars->set_active();
            break;
        // case ChartData::STYLE_PIE:
        //     m_RB_style_pie->set_active();
        //     break;
    }

    m_CB_table->set_active_id( std::to_string( m_data.m_table_id ) );

    m_CB_x_axis->remove_all();
    m_CB_y_axis->remove_all();
    m_CB_col_filter->remove_all();

    // PRESET VALUES
    m_CB_y_axis->append( std::to_string( ChartData::COLUMN_COUNT ),
                         STR::compose( "<", _( "Count" ), ">" ) );

    m_CB_col_filter->append( std::to_string( ChartData::COLUMN_NONE ),
                             STR::compose( "<", _( "None" ), ">" ) );
    // TODO: 3.1: splitting functionality:
    // m_CB_col_filter_v->append( STR::compose( "<", _( "Split" ), ">" ) );

    // COLUMN VALUES
    for( const auto col : m_data.m_td->m_columns )
    {
        m_CB_x_axis->append( std::to_string( col->m_id ), col->get_name() );
        m_CB_col_filter->append( std::to_string( col->m_id ), col->get_name() );
        if( col->is_numeric() )
            m_CB_y_axis->append( std::to_string( col->m_id ), col->get_name() );
    }

    // ACTIVE VALUES
    m_CB_x_axis->set_active_id( std::to_string( m_data.m_tcidx ) );
    m_CB_y_axis->set_active_id( std::to_string( m_data.m_tcidy ) );
    m_CB_period->set_active( m_data.get_period() - 1 );
    m_CB_combining->set_active( ( m_data.get_combining() >> ChartData::COMBINE_SHIFT ) - 1 );

    update_CB_underlay();
    update_CB_filter_v();

    m_CB_col_filter->set_active_id( std::to_string( m_data.m_tcidf ) );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetChart::set_from_string( const Ustring& chart_def )
{
    if( !chart_def.empty() ) // this occurs during logout
    {
        m_data.set_from_string( chart_def );
        update_child_widgets_per_data();
        calculate_and_plot( false );
    }
    else
        m_data.clear();
}

bool
WidgetChart::on_scroll_event( double dx, double dy )
{
    if     ( dy < 0 ) scroll( -1 );
    else if( dy > 0 ) scroll( 1 );

    return true;
}

void
WidgetChart::on_button_press_event( int n_press, double x, double y )
{
    if( m_gesture_click->get_current_button() == 1 && y > ( m_height - m_ov_height ) )
    {
        int col_start{ int( ( x / m_width ) * m_data.m_span - m_step_count / 2.0 ) };
        const unsigned int col_start_max{ m_data.m_span - m_step_count };
        if     ( col_start > int( col_start_max ) )
            col_start = col_start_max;
        else if( col_start < 0 )
            col_start = 0;
        m_step_start = col_start;

        refresh();

        m_F_button_pressed = true;
    }
}

void
WidgetChart::on_button_release_event( int n_press, double x, double y )
{
    if( m_gesture_click->get_current_button() == 1 )
    {
        if( n_press == 1 &&
            y < m_height - m_ov_height &&
            !m_F_button_pressed )
        {
            try
            {
                SetDiaryElemsByName el;
                const auto&         es{ get_elems_at( m_hovered_step + m_step_start ) };
                if( !es.empty() )
                {
                    for( auto& e : es )
                        el.insert( e );

                    PoElemChooser::show(
                            el,
                            []( DiaryElement* e ) { AppWindow::p->show_elem( e ); },
                            *this,
                            // Gdk::Rectangle( m_hovered_step * m_step_x, y, ??? ) );
                            Gdk::Rectangle( x, y, 0, 0 ) );
                }
            }
            catch( Error& err )
            {

            }
        }

        m_F_button_pressed = false;
    }
}

void
WidgetChart::on_motion_notify_event( double x, double y )
{
    if( m_F_button_pressed )
    {
        int col_start = ( x / m_width ) * m_data.m_span - m_step_count / 2.0;
        if     ( col_start > int( m_data.m_span - m_step_count ) )
            col_start = m_data.m_span - m_step_count;
        else if( col_start < 0 )
            col_start = 0;

        if( col_start != ( int ) m_step_start )
        {
            m_step_start = col_start;
            refresh();
        }
    }
    int hovered_step{ m_data.get_style() == ChartData::STYLE_LINE
                      ? int( round( ( x - m_x_min ) / m_step_x ) )
                      : int( floor( ( x - m_x_min ) / m_step_x ) ) };

    if     ( hovered_step < 0 )                     hovered_step = 0;
    else if( hovered_step >= int( m_step_count ) )  hovered_step = m_step_count - 1;

    bool flag_pointer_hovered = ( y > m_height - m_ov_height );
    if( flag_pointer_hovered != m_F_overview_hovered )
    {
        m_F_overview_hovered = flag_pointer_hovered;
        refresh();
    }
    else if( hovered_step != m_hovered_step )
    {
        m_hovered_step = hovered_step;
        refresh();  // TODO: 3.1: limit to tooltip area only
    }

    m_F_widget_hovered = true;
}

void
WidgetChart::on_leave_notify_event()
{
    m_F_overview_hovered = false;
    m_F_widget_hovered = false;
    refresh();
}

void
WidgetChart::on_resize( int w, int h )
{
    resize( w, h );
    m_Sc_zoom->set_visible( is_zoom_possible() );
}

void
WidgetChart::on_draw( const Cairo::RefPtr< Cairo::Context >& cr, int w, int h )
{
    if( !Diary::d->is_ready() ) return;

    if( !m_layout )
    {
        m_layout = Pango::Layout::create( cr );
        update_dims( true );
        m_Sc_zoom->set_visible( is_zoom_possible() );
    }

    Chart::draw( cr, w, h );
}
