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


#include "dialog_filter.hpp"
#include "../lifeograph.hpp"
#include "../app_window.hpp"

using namespace LIFEO;


DialogFilter::DialogFilter( BaseObjectType* cobj, const Glib::RefPtr< Gtk::Builder >& refbuilder,
                            int objects, Filter* p2filter )
:   Gtk::Window( cobj ), m_p2filter( p2filter ), m_def_original( p2filter->get_definition() ),
    m_F_editing_main_filter ( p2filter == Diary::d->get_filter_list() )
{
    Gtk::Button* B_cancel;

    try
    {
        m_W_filter      = Gtk::manage( new WidgetFilter( Diary::d, m_def_original, objects ) );
        B_cancel        = refbuilder->get_widget< Gtk::Button >( "B_cancel" );
        m_B_apply       = refbuilder->get_widget< Gtk::Button >( "B_apply" );
        m_B_revert      = refbuilder->get_widget< Gtk::Button >( "B_revert" );
        m_E_name        = refbuilder->get_widget< Gtk::Entry >( "E_name" );
        m_Vp_contents   = refbuilder->get_widget< Gtk::Viewport >( "Vp_contents" );
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create filter dialog" );
    }

    set_title( STR::compose( _( "Edit Filter" ), ": ", p2filter->get_name() ) );
    m_Vp_contents->set_child( *m_W_filter );
    m_E_name->set_text( p2filter->get_name() );

    m_B_apply->hide();
    m_B_revert->hide();

    // SIGNALS
    m_B_apply->signal_clicked().connect( [ this ](){ apply(); } );
    m_B_revert->signal_clicked().connect( [ this ](){ revert(); } );

    m_E_name->signal_changed().connect( [ this ](){ handle_filter_renamed();} );

    m_W_filter->signal_changed().connect( [ this ](){ handle_filter_edited(); } );
    //m_W_filter->signal_sorting_changed().connect( [ this ](){ handle_filter_sorting_changed(); } );

    B_cancel->signal_clicked().connect(
            [ this ]()
            {
                if( m_F_editing_main_filter ) update_main_list( m_def_original );
                hide();
            } );

    set_transient_for( *AppWindow::p );
    m_result = Result::ABORTED;
}

void
DialogFilter::handle_filter_edited()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    if( m_F_editing_main_filter )
    {
        Ustring def;
        m_W_filter->m_p2filterer->get_as_string( def );
        update_main_list( def );
    }

    m_B_apply->show();
    m_B_revert->show();
}

inline void
DialogFilter::update_main_list( const Ustring& def )
{
    m_p2filter->set_definition( def );

    AppWindow::p->UI_diary->update_all_entries_filter_status();
    AppWindow::p->UI_diary->update_entry_list();
}

void
DialogFilter::handle_filter_renamed()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    const Ustring& new_name { m_E_name->get_text() };
    auto&          filters  { m_p2filter->get_diary()->get_filters() };

    if( new_name.empty() || filters.find( new_name ) != filters.end() )
        m_E_name->add_css_class( "error" );
    else
        m_E_name->remove_css_class( "error" );

    m_B_apply->show();
    m_B_revert->show();
}

void
DialogFilter::apply()
{
    Ustring def;
    m_W_filter->get_as_string( def );
    m_p2filter->set_definition( def );
    m_p2filter->get_diary()->rename_filter( m_p2filter, m_E_name->get_text() );

    m_result = Result::OK;
    hide();
}

void
DialogFilter::revert()
{
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_W_filter->set_from_string( m_def_original );
    m_E_name->set_text( m_p2filter->get_name() );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();

    if( m_F_editing_main_filter ) update_main_list( m_def_original );

    m_B_apply->hide();
    m_B_revert->hide();
}
