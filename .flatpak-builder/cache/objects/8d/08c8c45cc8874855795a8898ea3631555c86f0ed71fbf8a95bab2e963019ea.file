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


#ifndef LIFEOGRAPH_PRINTING_HEADER
#define LIFEOGRAPH_PRINTING_HEADER


#include <pangomm.h>
#include <gtkmm.h>
#include <vector>

#include "diaryelements/entry.hpp"
#include "widgets/table_surface.hpp"
#include "widgets/chart_surface.hpp"
#include "parsers/parser_base.hpp"


namespace LIFEO
{

class Printable
{
    public:
        enum class Type { PT_PAGE_START, PT_TEXT, PT_IMAGE, PT_BULLET, PT_VSPACE, PT_END };

        Printable( double x = 0.0, double y = 0.0 ) : m_x( x ), m_y( y ) {}
        virtual ~Printable(){}

        virtual Type            get_type() const = 0;
        virtual double          get_height() const { return 0.0; }
        virtual void            render() {}
        virtual bool            is_empty() const { return false; }

        double m_x;
        double m_y;
};

typedef std::list< Printable* > ListPrintables;

class PrintablePageStart : public Printable
{
    public:
        PrintablePageStart( const Color& color )
        : m_bg_color( color ) { }

        Type                    get_type() const override { return Type::PT_PAGE_START; }

        void                    render() override;

        const Color             m_bg_color;
};

class PrintableVSpace : public Printable
{
    public:
        PrintableVSpace() { }

        Type                    get_type() const override { return Type::PT_VSPACE; }
        double                  get_height() const override;
        bool                    is_empty() const override { return true; }

        void                    render() override {}
};

class PrintableTextLine : public Printable
{
    public:
        PrintableTextLine( const Glib::RefPtr< Pango::LayoutLine >& line, bool F_empty,
                           double x, double y, const Color& color )
        : Printable( x, y ), m_layout( line ), m_F_empty( F_empty ), m_color( color ) { }

        Type                    get_type() const override { return Type::PT_TEXT; }
        double                  get_height() const override
        { return m_layout->get_pixel_logical_extents().get_height(); }
        bool                    is_empty() const override
        { return m_F_empty; }
        void                    render() override;

        Glib::RefPtr< Pango::LayoutLine >
                                m_layout;
        const bool              m_F_empty;
        const Color             m_color;
};

class PrintableImage : public Printable
{
    public:
        PrintableImage( Glib::RefPtr< Gdk::Pixbuf > pixbuf, double x, double y )
        : Printable( x, y ), m_pixbuf( pixbuf ) { }

        Type                    get_type() const override { return Type::PT_IMAGE; }
        double                  get_height() const override
        { return( m_height > 0 ? m_height : m_pixbuf->get_height() ); }
        void                    render() override;

        Glib::RefPtr< Gdk::Pixbuf >
                                m_pixbuf;
        double                  m_height { -1.0 };
};

class PrintableChart : public Printable
{
    public:
        PrintableChart( ChartSurface* surface, double x, double y )
        : Printable( x, y ), m_surface( surface ) { }
        ~PrintableChart()
        { if( m_surface ) delete m_surface; }

        Type                    get_type() const override { return Type::PT_IMAGE; }
        double                  get_height() const override { return m_surface->get_height(); }
        void                    render() override;

        ChartSurface*           m_surface{ nullptr };
};

class PrintableTable : public Printable
{
    public:
        PrintableTable( TableSurface* surface, double x, double y )
        : Printable( x, y ), m_surface( surface ) { }
        ~PrintableTable()
        { if( m_surface ) delete m_surface; }

        Type                    get_type() const override { return Type::PT_IMAGE; }
        double                  get_height() const override { return m_surface->get_height(); }
        void                    render() override;

        TableSurface*           m_surface{ nullptr };
};

class PrintableBullet : public Printable
{
    public:
        PrintableBullet( double x, double y, double radius, bool F_fill )
        : Printable( x, y ), m_radius( radius ), m_F_fill( F_fill ) { }

        Type                    get_type() const override { return Type::PT_BULLET; }
        double                  get_height() const override { return 16; }
        void                    render() override;

        const double            m_radius;
        const bool              m_F_fill;
};


class EntryParserPango
{
    public:
        // CHARACTER STYLES
        static const int        CS_HEADING          =     0x10;
        static const int        CS_SUBHEADING       =     0x11;
        static const int        CS_SUBSUBHEADING    =     0x12;
        static const int        CS_HEADING_FILTER   =     0x1F;
        static const int        CS_BOLD             =     0x20;
        static const int        CS_ITALIC           =     0x40;
        static const int        CS_HIGHLIGHT        =     0x80;
        static const int        CS_STRIKETHROUGH    =    0x100;
        static const int        CS_UNDERLINE        =    0x200;
        static const int        CS_SUBSCRIPT        =    0x400;
        static const int        CS_SUPERSCRIPT      =    0x401; // cannot coexist with subscript
        static const int        CS_LINK             =   0x1000;
        static const int        CS_COMMENT          =   0x2000;
        static const int        CS_MONOSPACE        =   0x4000;
        static const int        CS_TODO_OPEN        =   0x4001;
        static const int        CS_TODO_DONE        =   0x400F;
        static const int        CS_TODO_FILTER      =   0x8000;
        static const int        CS_INLINE_TAG       =  0x10000;
        static const int        CS_SKIP             =  0x20000;
        static const int        CS_QUOTATION        =  0x40000;
        static const int        CS_SMALLER          =  0x80000;
        // CHARACTER FUNCTIONS
        static const int        CF_TAG              = 0x100002;
        static const int        CF_FILTER           = 0x10000F;

                                EntryParserPango()
        {
        }

        void                    init();

        void                    add_entry( const Entry* );
        void                    add_diary_cover(); // create diary cover for multi-entry prints

        Pango::FontDescription  m_font;
        bool                    m_opt_one_entry{ false };    // print only the current entry
        bool                    m_opt_filtered_only{ false };
        double                  m_opt_margin{ 1.0 };
        bool                    m_opt_use_theme_font{ true };
        bool                    m_opt_use_theme_colors{ false };
        bool                    m_opt_hide_comments{ false };
        bool                    m_opt_entries_at_page_start{ false };

        ListPrintables          m_content;
        int                     m_n_pages{ 0 };

    protected:
        void                    add_page();
        void                    add_entry_break();
        void                    add_printable( Printable*, bool = true );
        void                    add_text( double );
        void                    add_image( const R2Pixbuf& );
        bool                    add_para_icon( const R2Pixbuf& );
        void                    add_para_number( const Paragraph* );
        void                    add_chart( const Paragraph* );
        void                    add_table( const Paragraph* );
        bool                    add_para_bullet( bool );

        void                    append_markups();

        void                    process_paragraph( Paragraph* );
        void                    apply_format( int, UstringSize, UstringSize );

        Ustring                 get_format_substr( UstringSize, int );

        const Entry*            m_p2entry{ nullptr };
        const Theme*            m_p2theme{ nullptr };
        Pango::Alignment        m_alignment{ Pango::Alignment::LEFT };
        Ustring                 m_source_text;
        Ustring                 m_marked_up_text;
        std::vector< int >      m_format_map;
        UstringSize             m_format_map_offset{ 0 };
        int                     m_format_prev{ 0 };
        int                     m_format_cur{ 0 };
        int                     m_indent_level_cur{ 0 };

        // METRICS
        double                  m_margin_x{ 0.0 };
        double                  m_margin_y{ 0.0 };
        double                  m_page_w{ 0.0 };
        double                  m_page_h{ 0.0 };
        double                  m_total_h_cur{ 0.0 };
        double                  m_entry_break_h{ 0.0 };
        double                  m_unit_indent_w{ 0.0 };
        double                  m_line_h{ 0.0 };
        double                  m_line_h_title{ 0.0 };
};


class PrintOpr : public Gtk::PrintOperation
{
    public:
        static Glib::RefPtr< PrintOpr >
                                create();

        virtual                 ~PrintOpr();

        void                    show_page_setup();

        void                    set_hide_comments( bool );

        static Glib::RefPtr< Gtk::PrintContext >
                                s_print_context;

    protected:
                                PrintOpr();

        void                    clear_content();
        void                    init_variables();
        void                    parse();

        // PrintOperation default signal handler overrides:
        Gtk::Widget*            on_create_custom_widget() override;
        void                    on_custom_widget_apply( Gtk::Widget* ) override;

        void                    on_begin_print( const Glib::RefPtr< Gtk::PrintContext >& ) override;
        void                    on_draw_page( const Glib::RefPtr< Gtk::PrintContext >&,
                                              int ) override;
        void                    on_status_changed() override;
        void                    on_done( Gtk::PrintOperation::Result ) override;

        // Printing-related objects:
        Glib::RefPtr< Gtk::PageSetup >
                                m_refPageSetup;
        Glib::RefPtr< Gtk::PrintSettings >
                                m_refSettings;

        Glib::RefPtr< Gtk::Builder >
                                m_builder;
        Gtk::Box*               m_Bx_print            { nullptr };
        Gtk::CheckButton*       m_RB_current          { nullptr };
        Gtk::CheckButton*       m_RB_filtered         { nullptr };
        Gtk::CheckButton*       m_ChB_1_entry_1_page  { nullptr };
        Gtk::FontButton*        m_FB                  { nullptr };
        Gtk::CheckButton*       m_ChB_theme_font      { nullptr };
        Gtk::CheckButton*       m_ChB_theme_colors    { nullptr };
        Gtk::ToggleButton*      m_RB_margin_off       { nullptr };
        Gtk::ToggleButton*      m_RB_margin_half      { nullptr };

        // PARSING
        EntryParserPango        m_parser;
        ListPrintables          m_content;
        const Entry*            m_p2entry_bgn;
        const Entry*            m_p2entry_end;
};

}   // end of namespace LIFEO

#endif
