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


#include "dialog_gettext.hpp"
#include "../lifeograph.hpp"


using namespace LIFEO;


// GET TEXT DIALOG =================================================================================
DialogGetText* DialogGetText::ptr = nullptr;

DialogGetText::DialogGetText( BaseObjectType* cobject,
                              const Glib::RefPtr< Gtk::Builder >& refbuilder )
:   Gtk::Popover( cobject )
{
    Gtk::Button* B_cancel;

    m_E_text    = Gtk::Builder::get_widget_derived< EntryClear >( refbuilder, "E_text" );
    m_B_go      = refbuilder->get_widget< Gtk::Button >( "B_go" );
    B_cancel    = refbuilder->get_widget< Gtk::Button >( "B_cancel" );
    //builder->get_widget( "L_msg", m_L_msg );

    set_default_widget( *m_B_go );

    // SIGNALS
    m_E_text->signal_changed().connect(  [ this ]
                                         { m_B_go->set_sensitive( !m_E_text->is_empty() ); } );
    //m_E_text->signal_activate().connect( [ this ]{ handle_go(); } );

    B_cancel->signal_clicked().connect(  [ this ]{ popdown(); m_handler_cancel(); } );
    m_B_go->signal_clicked().connect(    [ this ]{ handle_go(); } );
}

void
DialogGetText::launch( const Gdk::Rectangle* rect, Gtk::Widget* parent,
                       const FuncVoidUstring& handler_go,
                       const FuncVoid& handler_cancel,
                       const Ustring& text_init )
{
    if( ptr == nullptr )
    {
        auto&& builder { Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/dlg_gettext.ui" ) };
        ptr = Gtk::Builder::get_widget_derived< DialogGetText >( builder, "Po_gettext" );
        ptr->set_parent( *parent ); // FIXME: if this goes to _internal a SIGSGV occurs, why?
    }

    ptr->launch_internal( rect, parent, handler_go, handler_cancel, text_init );
}

void
DialogGetText::launch_internal( const Gdk::Rectangle* rect, Gtk::Widget* parent,
                                const FuncVoidUstring& handler_go,
                                const FuncVoid& handler_cancel,
                                const Ustring& text_init )
{
    m_handler_go = handler_go;
    m_handler_cancel = handler_cancel;

    //set_parent( *parent ); FIXME: moved to static launch to prevent SIGSEGV
    set_pointing_to( rect ? *rect :
                            Gdk::Rectangle( 0, 0,
                                            parent->get_allocated_width(),
                                            parent->get_allocated_height() ) );

    m_E_text->remove_css_class( "error" );
    m_E_text->set_text( text_init );

    popup();

    m_E_text->grab_focus();
}

void
DialogGetText::handle_go()
{
    if( !m_E_text->is_empty() )
    {
        popdown();
        m_handler_go( m_E_text->get_text() );
    }
}
