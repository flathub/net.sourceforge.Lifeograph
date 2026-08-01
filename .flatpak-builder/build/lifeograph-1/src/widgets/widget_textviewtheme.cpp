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


#include "glibmm/refptr.h"
#include "gtkmm/filedialog.h"
#include "sigc++/functors/mem_fun.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../helpers.hpp"
#include "../lifeograph.hpp"
#include "../app_window.hpp"
#include "widget_textviewtheme.hpp"


using namespace LIFEO;

// TEXTVIEW ========================================================================================
TextviewDiaryTheme::TextviewDiaryTheme()
:   TextviewDiary( 0 ), m_entry( nullptr, Date::NOT_SET )
{
    TextviewDiaryTheme::init();
}

TextviewDiaryTheme::TextviewDiaryTheme( BaseObjectType* obj,
                                        const Glib::RefPtr< Gtk::Builder >& builder )
:   TextviewDiary( obj, builder, 0 ), m_entry( nullptr, Date::NOT_SET )
{
    TextviewDiaryTheme::init();
}

inline void
TextviewDiaryTheme::init()
{
    // TAGS
    // NOTE: order is significant. the later a tag is added the more dominant it is.

    m_tag_link_theme = m_r2buffer->create_tag( "theme_link" );
    m_tag_link_theme->property_weight() = Pango::Weight::BOLD;
    m_tag_link_theme->property_underline() = Pango::Underline::SINGLE;
    m_tag_link_theme->property_size_points() = 6;
    m_tag_link_theme->property_rise() = 3000;
    m_tag_link_theme->property_foreground_rgba() = Gdk::RGBA( "#333333" );
    m_tag_link_theme->property_background_rgba() = Gdk::RGBA( "#cccccc" );
    m_tag_link_theme->property_background_full_height() = false;

    m_p2entry = &m_entry;

    set_editable( false );

    m_cursor_default = Gdk::Cursor::create( "default" );

    m_controller_motion->signal_leave().connect(
            sigc::mem_fun( *this, &TextviewDiaryTheme::on_leave_notify_event ) );
}


// PARSING
void
TextviewDiaryTheme::process_paragraph( Paragraph* para,
                                       const Gtk::TextIter& it_bgn,
                                       const Gtk::TextIter& it_end,
                                       bool )
{
    auto pos_para_bgn = it_bgn.get_offset();

    TextviewDiary::process_paragraph( para, it_bgn, it_end );

    // HIDDEN LINKS
    for( auto format : para->m_formats )
    {
        if( format->type == VT::HFT_LINK_THEME )
            m_r2buffer->apply_tag( m_tag_link_theme,
                       m_r2buffer->get_iter_at_offset( pos_para_bgn + format->pos_bgn ),
                       m_r2buffer->get_iter_at_offset( pos_para_bgn + format->pos_end ) );
    }
}

Paragraph*
TextviewDiaryTheme::add_paragraph( const Ustring& text, int type )
{
    Paragraph* para = m_entry.add_paragraph_before( text, nullptr );
    para->set_para_type_raw( type );
    return para;
}

void
TextviewDiaryTheme::set_text_preview()
{
    set_left_margin( 0 );
    set_right_margin( 0 );

    m_entry.clear_text();
    add_paragraph( m_p2theme->get_name(),               VT::PS_HEADER );
    add_paragraph( "Sample Subheading",                 VT::PS_SUBHDR );
    Paragraph* p = add_paragraph( "Highlighted",        VT::PS_PLAIN );
    p->add_format( VT::HFT_HIGHLIGHT, "", 0, 11 );

    m_r2buffer->set_text( m_entry.get_text() );
    update_text_formatting();

    // m_F_in_edit_mode = false;
}

void
TextviewDiaryTheme::set_text_edit()
{
    const auto theme{ m_p2theme };
    Paragraph* p;

    m_entry.clear_text();
    add_paragraph( theme->get_name() + " ", VT::PS_HEADER )->
            append( "EDIT COLOR", VT::HFT_LINK_THEME, "H" );
    add_paragraph( "Subheading ",           VT::PS_SUBHDR )->
            append( "EDIT COLOR", VT::HFT_LINK_THEME, "S" );
    add_paragraph( "Subsubheading",         VT::PS_SSBHDR );
    add_paragraph( "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
                   "eiusmod tempor incididunt ut labore et dolore magna aliqua. ", VT::PS_PLAIN )->
            append( "EDIT COLOR", VT::HFT_LINK_THEME, "T" );

    p = add_paragraph( "",                  VT::PS_BULLET );
    p->set_indent_level( 1 );
    p->append( "Highlighted Text", VT::HFT_HIGHLIGHT, "" );
    p->append( " ", nullptr );
    p->append( "EDIT COLOR", VT::HFT_LINK_THEME, "L" );

    p = add_paragraph( "",                  VT::PS_BULLET );
    p->set_indent_level( 1 );
    p->append( "Sample Tag", VT::HFT_TAG, theme->color_text.to_string() )->ref_id = DEID_MIN;
    p->append( "=", nullptr );
    p->append( "10/10", VT::HFT_TAG_VALUE, "" );
    p->append( "    Link: ", nullptr );
    p->append( "http://lifeograph.sf.net", VT::HFT_LINK_URI, "" );
    p->append( "   |   ", nullptr );
    p->append( "Broken Link", VT::HFT_LINK_BROKEN, "" );
    p->append( "   |   ", nullptr );
    p->append( "[[Comment]]", VT::HFT_COMMENT, "" );

    add_paragraph( "Background Color",      VT::PS_SUBHDR );
    add_paragraph( convert_gdkcolor_to_html( theme->color_base ) + " ", VT::PS_PLAIN )->
            append( "CHANGE", VT::HFT_LINK_THEME, "B" );

    add_paragraph( "Background Image",      VT::PS_SUBHDR );
    if( theme->image_bg.empty() )
        add_paragraph( "none ",             VT::PS_PLAIN )->
                append( "SET IMAGE", VT::HFT_LINK_THEME, "I" );
    else
    {
        p = add_paragraph( theme->image_bg + " ", VT::PS_PLAIN );
        p->append( "SET IMAGE", VT::HFT_LINK_THEME, "I" );
        p->append( " | ", nullptr );
        p->append( "CLEAR", VT::HFT_LINK_THEME, "i" );
    }

    add_paragraph( "Font",                  VT::PS_SUBHDR );
    add_paragraph( theme->font.to_string() + " ", VT::PS_PLAIN )->
            append( "EDIT", VT::HFT_LINK_THEME, "F" );

    add_paragraph( "Todo List",             VT::PS_SUBHDR );
    add_paragraph( "Open Item",             VT::PS_TODO )->set_indent_level( 1 );
    add_paragraph( "In Progress Item",      VT::PS_PROGRS )->set_indent_level( 1 );
    add_paragraph( "Done Item",             VT::PS_DONE )->set_indent_level( 1 );
    add_paragraph( "Canceled Item",         VT::PS_CANCLD )->set_indent_level( 1 );

    m_r2buffer->set_text( m_entry.get_text() );
    update_text_formatting();

    // m_F_in_edit_mode = true;
}

void
TextviewDiaryTheme::launch_color_dialog( Color* color )
{
    auto dlg { Gtk::ColorDialog::create() };
    dlg->set_with_alpha( false );

    dlg->choose_rgba( *AppWindow::p,
                      *color,
                      sigc::bind( sigc::mem_fun( *this, &TextviewDiaryTheme::handle_color_result ),
                                  dlg,
                                  color ) );
}
void
TextviewDiaryTheme::handle_color_result( const Glib::RefPtr< Gio::AsyncResult > & result,
                                         const Glib::RefPtr< Gtk::ColorDialog >& dlg,
                                         Color* color )
{
    try
    {
        auto color_new { dlg->choose_rgba_finish( result ) };
        color->set( color_new.to_string() );
        const_cast< Theme* >( m_p2theme )->calculate_derived_colors();
        set_theme( m_p2theme );
        handle_theme_edited();
    }
    catch( const Gtk::DialogError& err)
    {
        print_info( "No color selected. ", err.what() );
    }
    catch( const Glib::Error& err)
    {
        print_error( "Unexpected exception. ", err.what() );
    }
}

void
TextviewDiaryTheme::launch_font_dialog( Pango::FontDescription* font )
{
    auto dlg { Gtk::FontDialog::create() };

    dlg->choose_font( *AppWindow::p,
                      sigc::bind( sigc::mem_fun( *this, &TextviewDiaryTheme::handle_font_result ),
                                  dlg,
                                  font ),
                      *font );
};
void
TextviewDiaryTheme::handle_font_result( const Glib::RefPtr< Gio::AsyncResult > & result,
                                        const Glib::RefPtr< Gtk::FontDialog >& dlg,
                                        Pango::FontDescription* font )
{
    try
    {
        auto font_new { dlg->choose_font_finish( result ) };
        *font = font_new;
        set_theme( m_p2theme );
        handle_theme_edited();
    }
    catch( const Gtk::DialogError& err)
    {
        print_info( "No font selected. ", err.what() );
    }
    catch( const Glib::Error& err)
    {
        print_error( "Unexpected exception. ", err.what() );
    }
}

void
TextviewDiaryTheme::launch_file_dialog( String* uri )
{
    auto dlg { Gtk::FileDialog::create() };
    dlg->set_title( _( "Select Image" ) );
    dlg->set_initial_folder( Gio::File::create_for_path( Lifeograph::SHAREDIR + "/backgrounds" ) );

    // FILTERS
    FileChooserButton::add_image_file_filters( dlg );

    dlg->open( *AppWindow::p,
               sigc::bind( sigc::mem_fun( *this, &TextviewDiaryTheme::handle_file_result ),
                           dlg,
                           uri ) );
};
void
TextviewDiaryTheme::handle_file_result( Glib::RefPtr< Gio::AsyncResult >& result,
                                        const Glib::RefPtr< Gtk::FileDialog >& dlg,
                                        String* uri )
{
    try
    {
        auto file { dlg->open_finish( result ) };

        uri->assign( file->get_uri() );
        set_theme( m_p2theme );
        handle_theme_edited();
    }
    catch( const Gtk::DialogError& err )
    {
        print_info( "No file selected. ", err.what() );
    }
    catch( const Glib::Error& err )
    {
        print_error( "Unexpected exception. ", err.what() );
    }
}

void
TextviewDiaryTheme::handle_link_theme()
{
    Theme* theme{ const_cast< Theme* >( get_theme() ) };

    switch( m_p2hflink_hovered->uri[ 0 ] )
    {
        case 'H':   // heading
            launch_color_dialog( &theme->color_heading );
            break;
        case 'S':   // subheading
            launch_color_dialog( &theme->color_subheading );
            break;
        case 'T':   // text
            launch_color_dialog( &theme->color_text );
            break;
        case 'L':   // highlight
            launch_color_dialog( &theme->color_highlight );
            break;
        case 'B':
            launch_color_dialog( &theme->color_base );
            break;
        case 'I':
            launch_file_dialog( &theme->image_bg );
            break;
        case 'i':
            theme->image_bg.clear();
            break;
        case 'F':
            launch_font_dialog( &theme->font );
            break;
    }
}

void
TextviewDiaryTheme::process_link_uri( const Paragraph* para_hovered, int offset )
{
    set_hovered_link( para_hovered->get_format_at( VT::HFT_LINK_THEME, offset ) );

    if( m_p2hflink_hovered )
        m_link_hovered_go = [ & ](){ handle_link_theme(); };
}

/*void
TextviewDiaryTheme::on_draw( const Cairo::RefPtr< Cairo::Context >& cr, int w, int h )
{
    TextviewDiary::on_draw( cr );

    if( m_buffer_theme->m_p2theme && m_buffer_theme->m_p2theme->is_default() )
    {
        using namespace Cairo;
        R2Pixbuf buf{ Gtk::IconTheme::get_default()->load_icon( "default-32", 32 ) };
        const int w_buf      { buf->get_width() };
        const int h_buf      { buf->get_height() };
        auto&&    IS_img     { ImageSurface::create( FORMAT_ARGB32, w_buf, h_buf ) };
        auto&&    IC_img     { Context::create( IS_img ) };
        Gdk::Rectangle rect_Tv;

        get_visible_rect( rect_Tv );

        Gdk::Cairo::set_source_pixbuf( IC_img, buf, 0.0, 0.0 );
        IC_img->paint();

        cr->set_source( IS_img,rect_Tv.get_width() - w_buf, 0 );
        cr->rectangle( rect_Tv.get_width() - w_buf, 0, w_buf, h_buf );
        cr->clip();
        cr->paint();
        cr->reset_clip();
    }

    return true;
}*/

bool
TextviewDiaryTheme::handle_query_tooltip( int x, int y, bool keyboard_mode,
                                          const Glib::RefPtr< Gtk::Tooltip >& tooltip )
{
    if( get_hovered_link_type() == VT::HFT_LINK_THEME )
        tooltip->set_text( _( "Click to edit the theme" ) );
    else
        return false;

    return true;
}
