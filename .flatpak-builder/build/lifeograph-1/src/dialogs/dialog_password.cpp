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


#include <gtkmm/label.h>

#include "../lifeograph.hpp"
#include "dialog_password.hpp"


using namespace LIFEO;


// PASSWORD DIALOG =================================================================================
DialogPassword* DialogPassword::ptr = nullptr;

DialogPassword::DialogPassword( BaseObjectType* cobject,
                                const Glib::RefPtr< Gtk::Builder >& builder )
:   Gtk::Popover( cobject ), m_ptr2diary( nullptr )
{
    Gtk::Button* B_cancel;

    m_B_go      = builder->get_widget< Gtk::Button >( "B_password_go" );
    B_cancel    = builder->get_widget< Gtk::Button >( "B_password_cancel" );
    m_E_current = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_password_current" );
    m_E_new     = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_password_new" );
    m_E_confirm = Gtk::Builder::get_widget_derived< EntryClear >( builder, "E_password_confirm" );
    m_L_msg     = builder->get_widget<Gtk::Label>( "L_password_msg" );
    m_G_new     = builder->get_widget<Gtk::Grid>( "G_password_new" );
    m_Bx_auth   = builder->get_widget< Gtk::Box >( "Bx_password_authenticate" );
    m_B_auth    = builder->get_widget< Gtk::Button >( "B_password_authenticate" );

    // SIGNALS
    m_E_current->signal_changed().connect(
            sigc::mem_fun( *this, &DialogPassword::handle_entry_changed ) );

    m_B_auth->signal_clicked().connect(
            sigc::mem_fun( *this, &DialogPassword::authenticate ) );

    m_E_new->signal_activate().connect( [ this ]{ m_E_confirm->grab_focus(); } );
    m_E_new->signal_changed().connect(
            sigc::mem_fun( *this, &DialogPassword::check_match ) );
    m_E_confirm->signal_changed().connect(
            sigc::mem_fun( *this, &DialogPassword::check_match ) );

    B_cancel->signal_clicked().connect( [ this ]{ hide(); m_handler_cancel(); } );
    m_B_go->signal_clicked().connect( sigc::mem_fun( *this, &DialogPassword::handle_go ) );
}

void
DialogPassword::launch( OperationType ot, Diary* diary,
        const Gdk::Rectangle* rect, Gtk::Widget* parent,
        const FuncVoid& handler_go,
        const FuncVoid& handler_cancel )
{
    if( ptr ) delete ptr;

    auto&& builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/po_password.ui" ) };
    ptr = Gtk::Builder::get_widget_derived< DialogPassword >( builder, "Po_password" );

    ptr->m_ptr2diary = diary;
    ptr->m_ot = ot;
    ptr->m_handler_go = handler_go;
    ptr->m_handler_cancel = handler_cancel;

    ptr->set_parent( *parent );

    if( rect )
        ptr->set_pointing_to( *rect );

    ptr->launch_internal();
}

void
DialogPassword::launch_internal()
{
    m_E_current->set_visible( m_ot & OTC_CURRENT );
    m_E_current->set_sensitive( m_ot & OTC_CURRENT );
    m_G_new->set_visible( m_ot & OTC_NEW );
    m_Bx_auth->set_visible( m_ot == OT_CHANGE );
    m_E_new->set_sensitive( not( m_ot & OTC_CURRENT ) );
    m_E_confirm->set_sensitive( not( m_ot & OTC_CURRENT ) );
    m_E_new->remove_css_class( "error" );
    m_E_confirm->remove_css_class( "error" );

    if( m_ot == OT_CHANGE )
        set_default_widget( *m_B_auth );
    else
        set_default_widget( *m_B_go );

    switch( m_ot )
    {
        case OT_AUTHENTICATE:
        case OT_OPEN:
            m_B_go->set_label( _( "Continue" ) );
            break;
        case OT_ADD:
            m_B_go->set_label( _( "Add Password" ) );
            break;
        case OT_CHANGE:
            m_B_go->set_label( _( "Change Password" ) );
            break;
        default:
            break;
    }

    if( m_ptr2diary->get_uri() == m_path_prev )
        process_wrong_password();
    else
    {
        switch( m_ot )
        {
            case OT_AUTHENTICATE:
            case OT_OPEN:
                m_L_msg->set_text( _( "Please enter password" ) );
                break;
            case OT_ADD:
                m_L_msg->set_text( _( "Please enter a new password" ) );
                break;
            case OT_CHANGE:
                m_L_msg->set_text( _( "Please enter the current and new passwords" ) );
                break;
            default:
                break;
        }
    }

    m_E_current->set_text( "" );
    m_E_new->set_text( "" );
    m_E_confirm->set_text( "" );

    popup();

    if( m_ot == OT_ADD )
        m_E_new->grab_focus();
    else
        m_E_current->grab_focus();
}

void
DialogPassword::finish( const std::string& path )
{
    if( ptr )
    {
        if( path == ptr->m_path_prev )
            ptr->m_path_prev.clear();
        ptr->popdown();
    }
}

void
DialogPassword::handle_go()
{
    m_path_prev = m_ptr2diary->get_uri();

    if( m_ot & OTC_CHECK )
    {
        if( ! m_ptr2diary->compare_passphrase( ptr->m_E_current->get_text() ) )
        {
            process_wrong_password();
            return;     // do not call go handler
        }
        else
            m_path_prev.clear();
    }

    if( m_ot == OT_OPEN )
    {
        m_ptr2diary->set_passphrase( ptr->m_E_current->get_text() );
        m_handler_go();
        return;
    }
    else if( m_ot & OTC_NEW )
        m_ptr2diary->set_passphrase( ptr->m_E_new->get_text() );

    popdown();

    m_handler_go();
}

void
DialogPassword::handle_entry_changed()
{
    bool size_ok( m_E_current->get_text().size() >= Diary::PASSPHRASE_MIN_SIZE );

    if( m_ot == OT_CHANGE )
        m_B_auth->set_sensitive( size_ok );
    else
        m_B_go->set_sensitive( size_ok );
}

void
DialogPassword::process_wrong_password( int phase )
{
    if( phase < PHASE_COUNT )
    {
        if( phase == 0 )
            m_E_current->set_sensitive( false );

        m_E_current->set_progress_fraction( ++phase / double( PHASE_COUNT ) );
        Glib::signal_timeout().connect_once(
                sigc::bind( sigc::mem_fun( *this, &DialogPassword::process_wrong_password ),
                            phase ), 70 );
    }
    else
    {
        m_L_msg->set_text( _( "Wrong password. Please retry..." ) );
        m_E_current->set_text( "" );
        m_E_current->set_progress_fraction( 0 );
        m_E_current->set_sensitive( true );
        m_E_current->grab_focus();
        m_path_prev.clear();
    }
}

void
DialogPassword::authenticate()
{
    if ( m_ptr2diary->compare_passphrase( m_E_current->get_text() ) )
    {
        m_E_new->set_sensitive( true );
        m_E_confirm->set_sensitive( true );
        m_E_new->grab_focus();
        m_E_current->set_sensitive( false );
        m_B_auth->set_sensitive( false );
        set_default_widget( *m_B_go );
    }
    else
        process_wrong_password();
}

void
DialogPassword::check_match()
{
    Glib::ustring passphrase( m_E_new->get_text() );

    if( passphrase.size() >= Diary::PASSPHRASE_MIN_SIZE )
    {
        m_E_new->remove_css_class( "error" );
        if( passphrase == m_E_confirm->get_text() )
        {
            m_E_confirm->remove_css_class( "error" );
            m_B_go->set_sensitive( true );
        }
        else
        {
            m_E_confirm->add_css_class( "error" );
            m_B_go->set_sensitive( false );
        }
    }
    else
    {
        m_E_new->add_css_class( "error" );
        m_E_confirm->add_css_class( "error" );
        m_B_go->set_sensitive( false );
    }
}
