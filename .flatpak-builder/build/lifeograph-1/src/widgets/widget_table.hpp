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


#ifndef LIFEOGRAPH_WIDGETTABLE_HEADER
#define LIFEOGRAPH_WIDGETTABLE_HEADER


#include <gtkmm/drawingarea.h>
#include <gtkmm/builder.h>

#include "../helpers.hpp"
#include "table.hpp"
#include "widget_filterpicker.hpp"
#include "widget_entrypicker.hpp"
#include "../dialogs/dialog_filter.hpp"


namespace LIFEO
{

using namespace HELPERS;


class WidgetTable : public Gtk::DrawingArea, public Table
{
    public:
        WidgetTable( BaseObjectType*, const Glib::RefPtr< Gtk::Builder >& );
        ~WidgetTable() {}

        void                        set_helper_widgets( Gtk::MenuButton* );

        void                        set_diary( Diary* diary );
        void                        set_entry_filter( const Filter* );
        void                        set_para_filter( const Filter* );

        void                        set_line_cur( const DiaryElement* );
        void                        update_i_line_cur();

        void                        refresh_editability()
        {
            m_F_editable = ( m_p2diary ? m_p2diary->is_in_edit_mode() : false );
            m_RB_para_based->set_sensitive( m_F_editable );
            m_RB_entry_based->set_sensitive( m_F_editable );
            m_WFP_para->set_sensitive( m_F_editable );
            m_WFP_entry->set_sensitive( m_F_editable );
        }

        void                        show_column_Po();
        void                        place_column_Po();

        void                        refresh() override
        {
            // if( Glib::RefPtr< Gdk::Window > window = get_window() )
            //     window->invalidate( false );
            queue_draw();
        }

        void                        calculate_and_plot( bool = true );
        void                        sort_and_plot();
        void                        update_visible_lines_and_plot( bool = false );
        void                        remove_entry( Entry* );
        void                        add_entry( Entry* );

        std::string                 get_as_string()
        { return m_data.get_as_string(); }
        void                        set( const TableElem*, const DiaryElement* );

        void                        copy_delimited_text();

        SignalVoid                  signal_changed()
        { return m_Sg_changed; }
        SignalVoidElem              signal_elem_clicked()
        { return m_Sg_elem_clicked; }

    protected:
        void                        on_resize( int, int ) override;
        bool                        on_scroll_event( double, double );
        void                        on_button_press_event( int, double, double );
        void                        on_button_release_event( int, double, double );
        void                        on_motion_notify_event( double, double );
        void                        on_leave_notify_event();
        void                        on_draw( const Cairo::RefPtr< Cairo::Context >&, int, int );

        //void                        show_filter_editor( const Filter*, int );

        void                        handle_base_changed();
        void                        handle_order_col_toggled();
        void                        handle_grouping_toggled();

        void                        handle_col_renamed();
        void                        handle_col_type_changed();
        void                        handle_col_source_elem_changed();
        void                        handle_col_tag_changed( Entry* );
        void                        handle_col_opt_int_chg( int32_t );
        void                        handle_col_date_format_type_chg();
        void                        handle_col_date_format_chg();
        void                        handle_col_opts_duratn_chg();
        void                        handle_col_opts_text_chg();
        void                        handle_col_opts_arithmetic_chg( const int32_t );
        void                        handle_col_combine_same_toggled();
        void                        handle_col_show_as_chg();
        void                        handle_col_summary_func_chg();
        void                        handle_col_color_chg( Gtk::ToggleButton* );

        void                        update_col_options();
        void                        update_source_elem_options();
        void                        update_source_elem_filters();
        void                        set_up_col_opts_date();
        void                        set_up_col_opts_bool();
        void                        set_up_col_opts_size();
        void                        set_up_col_opts_duration();
        void                        set_up_col_opts_tag_value();
        void                        set_up_col_opts_subtag();
        void                        set_up_col_opts_text( bool = false );
        void                        set_up_col_opts_arithmetic();
        bool                        update_scrollbar();

        void                        populate_arithmetic_col_opts( Gtk::DropDown*, Gtk::DropDown*,
                                                                  Gtk::Entry*,
                                                                  const int, const double );
        int                         get_col_id_from_arith_opt_i( const int ) const;
        int                         get_arith_opt_i_from_col_id( const int ) const;

        bool                        is_pt_on_col_boundary( int = -1 );
        void                        enter_column_resize_mode( int );
        void                        exit_column_resize_mode();

        int                         CB_i_to_tag_vt( int vt )
        {
            switch( vt )
            {
                case 1:     return VT::TOTAL_REALIZED;
                case 2:     return VT::TOTAL_REMAINING;
                case 3:     return VT::AVG_PLANNED;
                case 4:     return VT::AVG_REALIZED;
                case 5:     return VT::AVG_REMAINING;
                case 6:     return VT::COMPL_PERCENTAGE;
                default:    return VT::TOTAL_PLANNED; // actually case 0
            }
        }
        int                         CB_i_to_date_vt( int vt )
        {
            return( vt | ( m_p2col_sel->get_opt_int() & VT::DATE_CUSTOM_FORMAT ) );
        }

        Glib::RefPtr< Gtk::Builder >
                                    m_builder;

        Gtk::Scrollbar*             m_Sb_vert;
        Glib::RefPtr< Gtk::Adjustment > m_Aj_vert;

        Gtk::Popover*               m_Po_table;
        //Gtk::Box*                   m_Bx_edit;
        WidgetFilterPicker*         m_WFP_entry;
        WidgetFilterPicker*         m_WFP_para;
        Gtk::Box*                   m_Bx_filter_para;
        Gtk::ToggleButton*          m_RB_entry_based;
        Gtk::ToggleButton*          m_RB_para_based;

        Gtk::Popover*               m_Po_column;
        // Gtk::Box*                   m_Bx_col_main;
        EntryClear*                 m_E_col_name;
        Gtk::Box*                   m_Bx_col_basics;
        Gtk::ComboBoxText*          m_CB_col_type;
        Gtk::Box**                  m_p2Bx_col_opts{ nullptr };
        Gtk::Box*                   m_Bx_col_source;
        Gtk::ComboBoxText*          m_CB_col_source_entry;
        Gtk::ComboBoxText*          m_CB_col_source_para;
        WidgetFilterPicker*         m_WFP_col_source_elem;

        // date options:
        Gtk::Box*                   m_Bx_col_opts_date{ nullptr };
        Gtk::ComboBoxText*          m_CB_date_type;
        Gtk::CheckButton*           m_ChB_date_custom_fmt;
        Gtk::Entry*                 m_E_date_custom_format;
        // bool options:
        Gtk::Box*                   m_Bx_col_opts_bool{ nullptr };
        WidgetFilterPicker*         m_WFP_bool_condition;
        // size options:
        Gtk::Box*                   m_Bx_col_opts_size{ nullptr };
        Gtk::ComboBoxText*          m_CB_col_size_type;
        // duration options:
        Gtk::Box*                   m_Bx_col_opts_duration{ nullptr };
        Gtk::ComboBoxText*          m_CB_duration_bgn;
        Gtk::ComboBoxText*          m_CB_duration_end;
        Gtk::ComboBoxText*          m_CB_duration_type;
        // tag value options:
        Gtk::Box*                   m_Bx_col_opts_tag_value{ nullptr };
        Gtk::ComboBoxText*          m_CB_tag_value_type;
        WidgetEntryPicker*          m_WEP_tag_value;
        // subtag options:
        Gtk::Box*                   m_Bx_col_opts_subtag{ nullptr };
        Gtk::ComboBoxText*          m_CB_subtag_type;
        WidgetEntryPicker*          m_WEP_subtag;
        // Text options:
        Gtk::Box*                   m_Bx_col_opts_text{ nullptr };
        Gtk::ToggleButton*          m_TB_text_incl_number;
        Gtk::ToggleButton*          m_TB_text_incl_plain;
        Gtk::ToggleButton*          m_TB_text_incl_tags;
        Gtk::ToggleButton*          m_TB_text_incl_dates;
        Gtk::ToggleButton*          m_TB_text_incl_comnts;
        Gtk::ToggleButton*          m_TB_text_incl_indents;
        Gtk::Box*                   m_Bx_text_includes;
        Gtk::ComboBoxText*          m_CB_text_source;
        // Arithmetic options:
        Gtk::Box*                   m_Bx_col_opts_arithmetic{ nullptr };
        Gtk::DropDown*              m_DD_arith_opA_unary;
        Gtk::DropDown*              m_DD_arith_opB_unary;
        Gtk::DropDown*              m_DD_arith_opA_type;
        Gtk::DropDown*              m_DD_arith_opB_type;
        Gtk::Entry*                 m_E_arith_opA;
        Gtk::Entry*                 m_E_arith_opB;
        Gtk::ToggleButton*          m_RB_arithmetic_add;
        Gtk::ToggleButton*          m_RB_arithmetic_sub;
        Gtk::ToggleButton*          m_RB_arithmetic_mul;
        Gtk::ToggleButton*          m_RB_arithmetic_div;
        Gtk::ToggleButton*          m_RB_arithmetic_pow;
        Gtk::ToggleButton*          m_RB_arithmetic_roo;
        Gtk::ToggleButton*          m_RB_arithmetic_mod;
        // Gtk::ToggleButton*          m_RB_arithmetic_avg; // may be
        Gtk::DropDown*              m_DD_arith_format;
        // name options:
        Gtk::Box*                   m_Bx_col_opts_name{ nullptr };
        Gtk::CheckButton*           m_ChB_name_show_parents;

        Gtk::Box*                   m_Bx_col_show_as;
        Gtk::DropDown*              m_DD_col_show_as;
        Gtk::Box*                   m_Bx_col_count_condtn;
        WidgetFilterPicker*         m_WFP_column;
        DialogFilter*               m_dialog_filter{ nullptr };
        Gtk::ToggleButton*          m_RB_col_sort_off;
        Gtk::ToggleButton*          m_RB_col_sort_asc;
        Gtk::ToggleButton*          m_RB_col_sort_dsc;
        Gtk::ToggleButton*          m_TB_col_combine_same;
        Gtk::DropDown*              m_DD_col_summary_func;
        Gtk::Box*                   m_Bx_col_coloring;
        Gtk::ToggleButton*          m_RB_col_color_off;
        Gtk::ToggleButton*          m_RB_col_color_neg_red;
        Gtk::ToggleButton*          m_RB_col_color_big_green;
        Gtk::ToggleButton*          m_RB_col_color_big_red;

        Gtk::Button*                m_B_col_dismiss;
        Gtk::Button*                m_B_col_add_left;
        Gtk::Button*                m_B_col_add_right;
        Gtk::Button*                m_B_col_move_prev;
        Gtk::Button*                m_B_col_move_next;

        Glib::RefPtr< Gtk::EventControllerScroll >
                                    m_event_controller_scroll;
        Glib::RefPtr< Gtk::GestureClick >
                                    m_gesture_click;

        SignalVoid                  m_Sg_changed;
        SignalVoidElem              m_Sg_elem_clicked;

        TableColumn*                m_p2col_sel;

        DEID                        m_line_id_cur{ DEID_UNSET };

        bool                        m_F_editable              { false };
        bool                        m_F_button_pressed        { false };
        bool                        m_F_needs_update          { false };
        int                         m_x_coord_drag_start      { -1 };
        int                         m_col_i_drag;
        double                      m_col_width_drag_L;
        double                      m_col_width_drag_R;
        int                         m_col_filter_classes;
};

} // end of namespace LIFEO

#endif
