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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "po_entry.hpp"
#include "src/app_window.hpp"
#include "../lifeograph.hpp"


using namespace LIFEO;


PoEntry::PoEntry( bool F_list_mode )
:   m_F_list_mode( F_list_mode )
{
    Gtk::Box*      Bx_comment_style;

    try
    {
        m_builder           = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/po_entry.ui" );

        m_Po_entry          = m_builder->get_widget< Gtk::Popover >( "Po_entry" );

        m_Bx_group_editing  = m_builder->get_widget< Gtk::Box >( "Bx_properties" );
        Bx_comment_style    = m_builder->get_widget< Gtk::Box >( "Bx_comment_style" );

        m_RB_not_todo       = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_not_todo" );
        m_RB_todo           = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_todo" );
        m_RB_progressed     = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_progressed" );
        m_RB_done           = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_done" );
        m_RB_canceled       = m_builder->get_widget< Gtk::ToggleButton >( "RB_entry_canceled" );
        m_TB_favorite       = m_builder->get_widget< Gtk::ToggleButton >( "TB_entry_favorite" );
        m_ClB_color         = m_builder->get_widget< Gtk::ColorButton >( "ClB_entry_color" );
        m_DD_title_style    = m_builder->get_widget< Gtk::DropDown >( "DD_title_style" );

        m_CB_spellcheck     = m_builder->get_widget< Gtk::ComboBoxText >( "CB_entry_spellchecking" );

        m_Bx_unit           = m_builder->get_widget< Gtk::Box >( "Bx_entry_unit" );
        m_CB_unit           = m_builder->get_widget< Gtk::ComboBoxText >( "CB_tag_unit" );
        m_E_unit            = m_builder->get_widget< Gtk::Entry >( "E_tag_unit" );

        m_CB_comment_style  = m_builder->get_widget< Gtk::ComboBoxText >( "CB_comment_style" );
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create the entry popover" );
    }

    m_CB_unit->set_name( "noncustom" );

    if( m_F_list_mode )
        Bx_comment_style->hide();

    m_DD_title_style->set_model( Gtk::StringList::create( Lifeograph::s_entry_name_styles ) );

    // SIGNALS
    m_RB_not_todo->signal_toggled().connect(
            [ this ]()
            { if( m_RB_not_todo->get_active() ) m_Sg_todo_changed.emit( ES::NOT_TODO ); } );
    m_RB_todo->signal_toggled().connect(
            [ this ]()
            { if( m_RB_todo->get_active() ) m_Sg_todo_changed.emit( ES::TODO ); } );
    m_RB_progressed->signal_toggled().connect(
            [ this ]()
            { if( m_RB_progressed->get_active() ) m_Sg_todo_changed.emit( ES::PROGRESSED ); } );
    m_RB_done->signal_toggled().connect(
            [ this ]()
            { if( m_RB_done->get_active() ) m_Sg_todo_changed.emit( ES::DONE ); } );
    m_RB_canceled->signal_toggled().connect(
            [ this ]()
            { if( m_RB_canceled->get_active() ) m_Sg_todo_changed.emit( ES::CANCELED ); } );

    m_CB_spellcheck->signal_changed().connect(
            [ this ](){ m_Sg_spellcheck_changed.emit( m_CB_spellcheck->get_active_id() ); } );

    m_TB_favorite->signal_toggled().connect(
            [ this ](){ m_Sg_favorite_changed.emit( m_TB_favorite->get_active() ); } );

    m_DD_title_style->property_selected().signal_changed().connect(
            [ this ](){ m_Sg_title_type.emit( m_DD_title_style->get_selected() ); } );

    m_CB_unit->signal_changed().connect(
            [ this ](){ m_Sg_unit_changed.emit( m_E_unit->get_text() ); } );

    m_CB_comment_style->signal_changed().connect(
            [ this ](){ m_Sg_comment_style.emit( m_CB_comment_style->get_active_row_number() ); } );

    m_ClB_color->signal_color_set().connect(
            [ this ](){ m_Sg_color_changed.emit( m_ClB_color->get_rgba() ); } );

}

inline R2Action
PoEntry::get_action( const Ustring& name )
{
    return std::dynamic_pointer_cast< Gio::SimpleAction >(
            AppWindow::p->UI_diary->m_AG_entry->lookup_action( name ) );
}

void
PoEntry::handle_edit_enabled()
{
    m_F_update_size = 1;

    m_Bx_group_editing->set_visible( true );
}

void
PoEntry::handle_logout()
{
    hide(); // hide if is visible

    get_action( "add" )->set_enabled( false );
    get_action( "merge" )->set_enabled( false );
    get_action( "sort_siblings" )->set_enabled( false );
    get_action( "trash" )->set_enabled( false );
    get_action( "restore" )->set_enabled( false );
    get_action( "dismiss" )->set_enabled( false );

    m_Bx_group_editing->hide();

    m_F_update_size = 0;

    m_Po_entry->unparent();

    m_p2entry = nullptr;
}

void
PoEntry::set( Entry* entry, Gtk::Widget& W_parent, const Gdk::Rectangle& rect )
{
    m_p2entry = entry;

    m_Po_entry->unparent();
    m_Po_entry->set_parent( W_parent );
    m_Po_entry->set_pointing_to( rect );
}

void
PoEntry::show()
{
    refresh();
    m_Po_entry->popup();
}

void
PoEntry::refresh()
{
    refresh_CB_spellchecking();

    if( Diary::d->is_in_edit_mode() )
        refresh_editing();

    Lifeograph::START_INTERNAL_OPERATIONS();
    m_CB_comment_style->set_active( VT::get_v< VT::CS,
                                               unsigned,
                                               int >( m_p2entry->get_comment_style() ) );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
PoEntry::refresh_editing()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    switch( m_p2entry->get_todo_status() )
    {
        case ES::TODO:
            m_RB_todo->set_active();
            break;
        case ES::PROGRESSED:
            m_RB_progressed->set_active();
            break;
        case ES::DONE:
            m_RB_done->set_active();
            break;
        case ES::CANCELED:
            m_RB_canceled->set_active();
            break;
        case ES::NOT_TODO:
        default:
            m_RB_not_todo->set_active();
            break;
    }

    m_TB_favorite->set_active( m_p2entry->is_favorite() );

    m_DD_title_style->set_selected( m_p2entry->get_title_style() );
    m_E_unit->set_text( m_p2entry->get_unit() );
    m_ClB_color->set_rgba( m_p2entry->get_color() );

    get_action( "add" )->set_enabled( m_F_list_mode );
    get_action( "merge" )->set_enabled( m_F_list_mode &&
                                        AppWindow::p->UI_diary->get_selected_count() > 1 );
    get_action( "sort_siblings" )->set_enabled( m_F_list_mode );

    get_action( "trash" )->set_enabled( !m_p2entry->is_trashed() );
    get_action( "restore" )->set_enabled( m_p2entry->is_trashed() );
    get_action( "dismiss" )->set_enabled( m_p2entry->is_trashed() );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
PoEntry::refresh_CB_spellchecking()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    m_CB_spellcheck->remove_all();
    m_CB_spellcheck->append( LANG_INHERIT_DIARY, _( "Diary Default" ) );
    m_CB_spellcheck->append( "", _( STRING::OFF ) );

    for( auto iter = Lifeograph::s_lang_list.begin();
         iter != Lifeograph::s_lang_list.end(); ++iter )
    {
        m_CB_spellcheck->append( *iter, *iter );
    }

    // SOME OTHER LANGUAGE THAT IS NOT SUPPORTED BY THE CURRENT SYSTEM
    if( !m_p2entry->get_lang().empty() && m_p2entry->get_lang() != LANG_INHERIT_DIARY &&
        Lifeograph::s_lang_list.find( m_p2entry->get_lang() ) == Lifeograph::s_lang_list.end() )
    {
        m_CB_spellcheck->append( m_p2entry->get_lang(), m_p2entry->get_lang() + "(!)" );
    }

    m_CB_spellcheck->set_active_id( m_p2entry->get_lang() );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}
