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

#include <cstdio>   // for file operations
#include <string>
#include <sstream>
#include <iostream>
#include <gcrypt.h>
#include <chrono>

#include "diary.hpp"
#include "../helpers.hpp"
#include "../lifeograph.hpp"
#include "../widgets/chart_surface.hpp"
#include "../widgets/table_surface.hpp"
#include "../parsers/parser_upgrader.hpp"


using namespace LIFEO;


// STATIC MEMBERS
Diary*                  Diary::d;
//bool                    Diary::s_flag_ignore_locks{ false };

// PARSING HELPERS
DateV
get_db_line_date( const Ustring& line )
{
    DateV date{ 0 };

    for( unsigned int i = 2;
         i < line.size() && i < 12 && int ( line[ i ] ) >= '0' && int ( line[ i ] ) <= '9';
         i++ )
    {
        date = ( date * 10 ) + int ( line[ i ] ) - '0';
    }

    return date;
}

Ustring
get_db_line_name( const Ustring& line )
{
    Ustring::size_type begin( line.find( '\t' ) );
    if( begin == std::string::npos )
        begin = 2;
    else
        begin++;

    return( line.substr( begin ) );
}

// MATCHES COMPARATOR
bool
LIFEO::compare_matches( HiddenFormat* l, HiddenFormat* r )
{
    if( l->var_d == r->var_d )
    {
        if( l->ref_id == r->ref_id )
        {
            if( l->var_i == r->var_i )
                return( l->pos_bgn < r->pos_bgn );  // earlier in para first
            else
                return( l->var_i < r->var_i );      // earlier in entry first
        }
        return( l->ref_id < r->ref_id );        // to ensure that different entries are separated
    }
    return( l->var_d > r->var_d );                  // latest in diary first
}

// DIARY ===========================================================================================
Diary::Diary()
:   m_random_gen( m_random_device() ), m_matches( &compare_matches )
{
    m_dispatcher_postread_operations.connect(
            sigc::mem_fun( *this, &Diary::handle_postread_operations_finished ) );

    m_force_id = FILTER_ID_TRASHED;
    m_filter_trashed        = new Filter( this, "<" + STR0/SI::TRASHED + ">",
                                          Filter::DEFINITION_TRASHED );
    m_force_id = FILTER_ID_CUR_ENTRY;
    m_filter_cur_entry      = new Filter( this, "<" + STR0/SI::CURRENT_ENTRY + ">",
                                          Filter::DEFINITION_CUR_ENTRY + '_' );
    m_force_id = FILTER_ID_CUR_ENTRY_TREE;
    m_filter_cur_entry_tree = new Filter( this, "<" + STR0/SI::CURRENT_ENTRY_TREE + ">",
                                          Filter::DEFINITION_CUR_ENTRY + 'D' );
}

Diary::~Diary()
{
    remove_lock_if_necessary();
}

SKVVec
Diary::get_as_skvvec() const
{
    SKVVec sv;

    // TODO: make options more human readable
    sv.push_back( { SI::LANGUAGE,       m_language } );
    sv.push_back( { SI::OPTIONS,        VT::stringize_bitmap< VT::DO >( m_options ) } );
    sv.push_back( { SI::FILTER_LIST,    m_p2filter_list ? m_p2filter_list->get_name()
                                                        : STR0/SI::NA } );
    sv.push_back( { SI::FILTER_SEARCH,  m_p2filter_search ? m_p2filter_search->get_name()
                                                          : STR0/SI::NA } );
    sv.push_back( { SI::STARTUP_ENTRY,  get_entry_name( m_startup_entry_id ) } );
    sv.push_back( { SI::COMPLETION_TAG, get_entry_name( m_completion_tag_id ) } );

    return sv;
}

Result
Diary::init_new( const std::string& path, const std::string& pw )
{
    clear();

    set_id( create_new_id( this ) ); // adds itself to the ID pool with a unique ID

    m_read_version = DB_FILE_VERSION_INT;
    Result result{ set_path( path, SPT_NEW ) };

    if( result != LIFEO::SUCCESS )
    {
        clear();
        return result;
    }

    m_p2chart_active = create_chart( _( STRING::DEFAULT ), ChartElem::DEFINITION_DEFAULT );
    m_p2table_active = create_table( _( STRING::DIARY_REPORT ), TableElem::DEFINITION_REPORT );

    ThemeSystem::get()->copy_to( create_theme( ThemeSystem::get()->get_name() ) );
    //-------------NAME-------------FONT----BASE-------TEXT-------HEADING----SUBHEAD----HLIGHT----
    create_theme( "Dark",       "Sans 10", "#111111", "#CCCCCC", "#FF6666", "#DD3366", "#661133" );
    create_theme( "Artemisia", "Serif 10", "#FFEEEE", "#000000", "#CC0000", "#A00000", "#F1BBC4" );
    create_theme( "Urgent",     "Sans 10", "#A00000", "#FFAA33", "#FFFFFF", "#FFEE44", "#000000" );
    create_theme( "Well Noted", "Sans 11", "#BBEEEE", "#000000", "#553366", "#224488", "#90E033" );

    m_options = VT::DO::DEFAULT;

    create_entry_dummy(); // must come after m_ptr2chapter_ctg_cur is set
    set_passphrase( pw );

    m_login_status = LOGGED_IN_RO;

    return write();
}

void
Diary::clear()
{
    if( m_thread_postread_operations )
    {
        {
            std::lock_guard< std::mutex > lock( m_mutex_postread_operations );
            m_F_stop_postread_operations = true;
        }

        while( m_F_stop_postread_operations )
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
    }

    if( m_parser_bg_postread )
    {
        delete m_parser_bg_postread;
        m_parser_bg_postread = nullptr;
    }

    if( m_sstream )
    {
        delete m_sstream;
        m_sstream = nullptr;
    }
    m_size_bytes = 0;

    m_uri.clear();

    m_read_version = 0;

    set_id( DEID_UNSET );
    m_force_id = DEID_UNSET;
    m_ids.clear();

    m_entries.clear();
    m_p2entry_1st = nullptr;

    m_search_text.clear();
    if( m_fc_search )
    {
        m_p2filter_search = nullptr;
        delete m_fc_search;
        m_fc_search = nullptr;
    }
    m_matches.clear();

    m_filters.clear();
    m_p2filter_list = nullptr;

    m_charts.clear();
    m_p2chart_active = nullptr;

    m_tables.clear();
    m_p2table_active = nullptr;

    clear_themes();

    m_weekends[ 0 ] = true;
    m_weekends[ 6 ] = true;
    for( int i = 1; i < 6; i++ ) m_weekends[ i ] = false;
    m_holidays.clear();

    m_startup_entry_id = HOME_CURRENT_ENTRY;
    m_last_entry_id = DEID_UNSET;
    m_completion_tag_id = DEID_UNSET;

    m_passphrase.clear();

    // NOTE: only reset body options here:
    m_language.clear();
    m_options = 0;
    m_opt_ext_panel_cur = 1;

    m_F_read_only = false;
    m_F_continue_from_lock = false;
    m_login_status = LOGGED_OUT;
}

R2Pixbuf
Diary::get_image_file( const String& uri, int width )
{
    R2Pixbuf      buf;
    const auto&&  rc_name { STR::compose( uri, "/", width ) };
    auto&&        iter    { m_map_images_f.find( rc_name ) };

    if( iter == m_map_images_f.end() )
    {
        auto file { Gio::File::create_for_uri( convert_rel_uri( uri ) ) };

        if( !file->query_exists() ) return buf;

        try
        {
            buf = Gdk::Pixbuf::create_from_file( file->get_path() );
// #ifndef _WIN32
//             buf = Gdk::Pixbuf::create_from_file(
//                     Glib::filename_from_uri( convert_rel_uri( uri ) ) );
// #else
//             const auto&& pth = convert_rel_uri( uri );
//             buf = Gdk::Pixbuf::create_from_file( PATH( pth.substr( 7, pth.length() - 7 ) ) );
// #endif
        }
        catch( ... )
        {
            throw LIFEO::Error( "Image not found" );
        }
        if( buf->get_width() > width )
            buf = buf->scale_simple( width, ( buf->get_height() *  width ) / buf->get_width(),
                                     Gdk::InterpType::BILINEAR );

        m_map_images_f[ rc_name ] = buf;
    }
    else
        buf = iter->second;

    return buf;
}
R2Pixbuf
Diary::get_image_chart( const String& uri, int width, const Pango::FontDescription& fd )
{
    R2Pixbuf     buf;
    const auto&& rc_name { STR::compose( uri, "/", width ) };
    auto&&       iter    { m_map_images_c.find( rc_name ) };

    if( iter == m_map_images_c.end() )
    {
        auto chart{ get_element2< ChartElem >( std::stoul( uri ) ) };

        if( !chart ) throw LIFEO::Error( "Chart not found" );

        buf = ChartSurface::create_pixbuf( chart, width, fd );

        m_map_images_c[ rc_name ] = buf;
    }
    else
        buf = iter->second;

    return buf;
}
R2Pixbuf
Diary::get_image_table( const String& uri, int width, const Pango::FontDescription& fd,
                        bool F_expanded )
{
    R2Pixbuf     buf;
    const auto&& rc_name { STR::compose( uri, "/", width ) };
    auto&&       iter    { m_map_images_t.find( rc_name ) };

    if( iter == m_map_images_t.end() )
    {
        auto table{ get_element2< TableElem >( std::stoul( uri ) ) };

        if( !table ) throw LIFEO::Error( "Table not found" );

        buf = TableSurface::create_pixbuf( table, width, fd, F_expanded );

        m_map_images_t[ rc_name ] = buf;
    }
    else
        buf = iter->second;

    return buf;
}

void
Diary::clear_chart_and_table_images()
{
    m_map_images_c.clear();
    m_map_images_t.clear();
}
void
Diary::clear_table_images( const String& uri )
{
    for( auto&& it = m_map_images_t.begin(); it != m_map_images_t.end(); )
    {
        if( STR::begins_with( it->first, uri ) )
            it = m_map_images_t.erase( it );
        else
            ++it;
    }
}

LIFEO::Result
Diary::set_path( const String& path0, SetPathType type )
{
    auto    file   { Gio::File::create_for_commandline_arg( path0 ) };
    String  uri    { file->get_uri() };

    // CHECK FILE SYSTEM PERMISSIONS
    if( type != SPT_NEW )
    {
        if( !file->query_exists() )
        {
            PRINT_DEBUG( "File is not found" );
            return LIFEO::FILE_NOT_FOUND;
        }

        auto finfo { file->query_info() };

        if( !finfo->get_attribute_boolean( G_FILE_ATTRIBUTE_ACCESS_CAN_READ ) )
        {
            PRINT_DEBUG( "File is not readable" );
            return LIFEO::FILE_NOT_READABLE;
        }
        else if( type != SPT_READ_ONLY &&
                 !finfo->get_attribute_boolean( G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE ) )
        {
            PRINT_DEBUG( "File is not writable" );
            return LIFEO::FILE_NOT_WRITABLE;
        }

        // RESOLVE SYMBOLIC LINK
        if( finfo->get_file_type() == Gio::FileType::SYMBOLIC_LINK )
        {
            uri = finfo->get_symlink_target(); // TODO: 3.1: is this necessary with Gio::File?
            print_info( "Symbolic link resolved to path: ", uri );
        }
    }

    // REMOVE PREVIOUS PATH'S LOCK IF ANY
    if( type != SPT_READ_ONLY )
        remove_lock_if_necessary();

    // ACCEPT PATH
    m_uri = uri;
    m_name = get_filename_base( uri );
    m_F_read_only = ( type == SPT_READ_ONLY );

    // CHECK IF LOCKED
    if( !m_F_read_only && is_locked() )
        return LIFEO::FILE_LOCKED;
    else
        return LIFEO::SUCCESS;
}

void
Diary::set_continue_from_lock()
{
    m_F_continue_from_lock = true;
}

String
Diary::convert_rel_uri( String uri )
{
    if( uri.find( "rel://" ) == 0 )
    {
        auto file { Gio::File::create_for_uri( m_uri ) };
        uri.replace( 0, 5, file->get_parent()->get_uri() );
    }

    return uri;
}

LIFEO::Result
Diary::enable_editing()
{
    if( m_F_read_only )
    {
        PRINT_DEBUG( "Diary: editing cannot be enabled. Diary is read-only" );
        return LIFEO::FILE_READ_ONLY;
    }

    if( !check_uri_writable( m_uri ) )
    {
        PRINT_DEBUG( "File is not writable" );
        return LIFEO::FILE_NOT_WRITABLE;
    }

    // Check and "touch" the new lock:
    try
    {
        auto file_lock { Gio::File::create_for_uri( m_uri + LOCK_SUFFIX ) };

        if     ( !file_lock->query_exists() )
            file_lock->create_file()->close();
        else if( !m_F_continue_from_lock )
        {
            auto file_dst { Gio::File::create_for_uri( m_uri + LOCK_SUFFIX +
                                                       Date::format_string_adv( Date::get_now(),
                                                                                ".F_hms" ) ) };

            file_lock->copy( file_dst );
        }
    }
    catch( const Glib::Error& err )
    {
        print_error( err.what() );
        return LIFEO::FAILURE;
    }

    m_login_status = LOGGED_IN_EDIT;

    return LIFEO::SUCCESS;
}

bool
Diary::set_passphrase( const std::string& passphrase )
{
    if( passphrase.size() >= PASSPHRASE_MIN_SIZE )
    {
        m_passphrase = passphrase;
        return true;
    }
    else
        return false;
}

void
Diary::clear_passphrase()
{
    m_passphrase.clear();
}

const std::string&
Diary::get_passphrase() const
{
    return m_passphrase;
}

bool
Diary::compare_passphrase( const std::string& passphrase ) const
{
    return( m_passphrase == passphrase );
}

bool
Diary::is_passphrase_set() const
{
    return( ( bool ) m_passphrase.size() );
}

// PARSING HELPERS
inline void
parse_todo_status( Entry* p2entry, char c )
{
    switch( c )
    {
        case 't': p2entry->set_todo_status( ES::TODO ); break;
        case 'T': p2entry->set_todo_status( ES::NOT_TODO | ES::TODO ); break;
        case 'p': p2entry->set_todo_status( ES::PROGRESSED ); break;
        case 'P': p2entry->set_todo_status( ES::NOT_TODO | ES::PROGRESSED ); break;
        case 'd': p2entry->set_todo_status( ES::DONE ); break;
        case 'D': p2entry->set_todo_status( ES::NOT_TODO | ES::DONE ); break;
        case 'c': p2entry->set_todo_status( ES::CANCELED ); break;
        case 'C': p2entry->set_todo_status( ES::NOT_TODO | ES::CANCELED ); break;
    }
}

inline void
parse_theme( Theme* ptr2theme, const std::string& line )
{
    switch( line[ 1 ] )
    {
        case 'f':   // font
            ptr2theme->font = Pango::FontDescription( line.substr( 2 ) );
            break;
        case 'b':   // base color
            ptr2theme->color_base.set( line.substr( 2 ) );
            break;
        case 't':   // text color
            ptr2theme->color_text.set( line.substr( 2 ) );
            break;
        case 'h':   // heading color
            ptr2theme->color_heading.set( line.substr( 2 ) );
            break;
        case 's':   // subheading color
            ptr2theme->color_subheading.set( line.substr( 2 ) );
            break;
        case 'l':   // highlight color
            ptr2theme->color_highlight.set( line.substr( 2 ) );
            break;
        case 'i':   // background image
            ptr2theme->image_bg = line.substr( 2 );
            break;
    }
}

inline void
parse_para_alignment( Paragraph* para, const char c )
{
    switch( c )
    {
        case '|': para->set_alignment( VT::PS_ALIGN_C ); break;
        case '>': para->set_alignment( VT::PS_ALIGN_R ); break;
        //case '<':
        default:  para->set_alignment( VT::PS_ALIGN_L ); break;
    }
}

inline void
parse_para_type( Paragraph* para, const char c )
{
    switch( c )
    {
        case 'I': para->set_code( true ); break; // legacy support
        case 'S': para->set_heading_level( VT::PS_SUBHDR ); break;
        case 'B': para->set_heading_level( VT::PS_SSBHDR ); break;
        case 'O': para->set_list_type( VT::PS_TODO );   break;
        case '~': para->set_list_type( VT::PS_PROGRS ); break;
        case '+': para->set_list_type( VT::PS_DONE );   break;
        case 'X': para->set_list_type( VT::PS_CANCLD ); break;
        case '-': para->set_list_type( VT::PS_BULLET ); break;
        case '1': para->set_list_type( VT::PS_NUMBER ); break;
        case 'A': para->set_list_type( VT::PS_CLTTR ); break;
        case 'a': para->set_list_type( VT::PS_SLTTR ); break;
        case 'R': para->set_list_type( VT::PS_CROMAN ); break;
        case 'r': para->set_list_type( VT::PS_SROMAN ); break;
        //default:  para->set_para_type( VT::PS_PLAIN );  break;
    }
}

void
Diary::tmp_upgrade_ordinal_date_to_2000( DateV& old_date )
{
    if( DateOld::is_ordinal( old_date ) == false )
        return;

    if( m_read_version < 1020 )
    {
        if( old_date & DateOld::FLAG_VISIBLE )
            old_date -= DateOld::FLAG_VISIBLE;
        else
            old_date |= DateOld::FLAG_VISIBLE;
    }

    old_date = DateOld::make_ordinal( !DateOld::is_hidden( old_date ),
                                      DateOld::get_order_2nd( old_date ),
                                      DateOld::get_order_3rd( old_date ),
                                      0 );
}

void
Diary::tmp_upgrade_date_to_3000( DateV& date )
{
    date = Date::make( DateOld::get_year( date ),
                       DateOld::get_month( date ),
                       DateOld::get_day( date ) );
}

inline void
tmp_add_tags_as_paragraph( Entry* entry_new,
                           std::map< Entry*, Value >& entry_tags )
{
    if( entry_new )
    {
        for( auto& kv_tag : entry_tags )
        {
            entry_new->add_tag( kv_tag.first, kv_tag.second );
        }

        entry_tags.clear();
    }
}

inline void
tmp_create_chart_from_tag( Entry* tag, long type, Diary* diary )
{
    auto o2{ ( type & ChartData::UNDERLAY_MASK ) == ChartData::UNDERLAY_PREV_YEAR ? "Y" : "-" };
    auto o3{ ( type & ChartData::PERIOD_MASK )   == ChartData::DEL_PERIOD_MONTHLY ? "M" : "Y" };
    auto o4{ ( type & ChartData::COMBINE_MASK )  == ChartData::DEL_AVERAGE        ? "A" : "P" };
    // create predefined charts for non-boolean tags
    if( tag && ( type & ChartData::COMBINE_MASK ) != ChartData::DEL_BOOLEAN )
    {
        diary->create_chart( tag->get_name(),
                             STR::compose( "Gyt", tag->get_id(), "\nGoT", o2, o3, o4 ) );
    }
}

void
Diary::tmp_upgrade_table_defs()
{
    for( auto& kv_table : m_tables )
    {
        std::string&&           def{ kv_table.second->get_definition() };
        std::string             line;
        std::string::size_type  line_offset{ 0 };
        int                     col_count{ 0 };

        while( STR::get_line( def, line_offset, line ) )
            if( line[ 1 ] == 'c' && line[ 2 ] == 'n' ) col_count++;

        if( col_count == 0 )
            continue;

        line_offset = 0;

        while( STR::get_line( def, line_offset, line ) )
            if( line[ 1 ] == 'c' && line[ 2 ] == 'o' )
                def.insert( line_offset, STR::compose( "Mcw", 1.0/col_count, '\n' ) );

        kv_table.second->set_definition( def );
    }
}

void
Diary::upgrade_to_1030()
{
    // initialize the status dates:
    for( auto& kv_entry : m_entries )
        kv_entry.second->m_date_finish = kv_entry.second->m_date_created;

    // replace old to-do boxes:
    replace_all_matches( "☐", "[ ]" );
    replace_all_matches( "☑", "[+]" );
    replace_all_matches( "☒", "[x]" );
}

void
Diary::upgrade_to_1050()
{
    for( auto& kv_entry : m_entries )
    {
        Entry* entry{ kv_entry.second };

        if( entry->get_todo_status() == ES::NOT_TODO )
            entry->update_todo_status();
    }
}

void prnt_siblings( Entry* e1st, int indent_lvl )
{
    String indent;
    for( int i= 0; i < indent_lvl; i++ ) indent += "  ";

    for( Entry* esib = e1st; esib; esib = esib->get_next() )
    {
        PRINT_DEBUG( "       ", indent, "* ",  esib->get_name(),
                     "(par=", esib->get_parent() ? esib->get_parent()->get_name() :  "0x0", ")" );
        if( esib->get_child_1st() )
            prnt_siblings( esib->get_child_1st(), indent_lvl + 1 );
    }
}

bool
Diary::upgrade_to_3000()
{
    // startup entry:
    if( m_startup_entry_id == DEID_MIN ) // this was used in diaries <2000
        m_startup_entry_id = HOME_CURRENT_ENTRY;

    // fill up the hierarchy:
    if( m_read_version < 2011 && !m_entries.empty() )
    {
        Entry* e_last_l1        { nullptr };
        Entry* e_last_l2        { nullptr };
        Entry* e_last_l3        { nullptr };
        Entry* e_first_temporal { nullptr };
        Entry* e_last_temporal  { nullptr };

        auto add_children_to_parent_entry = []( Entry* es1, Entry* ep )
        {
            Entry* e_last_sibling = es1;
            while( e_last_sibling->m_p2next != nullptr )
            {
                e_last_sibling = e_last_sibling->get_next();
                e_last_sibling->m_p2parent = ep;
            }

            if( ep->m_p2child_1st )
            {
                ep->m_p2child_1st->m_p2prev = e_last_sibling;
                e_last_sibling->m_p2next = ep->m_p2child_1st;
            }

            es1->m_p2parent = ep;
            es1->m_p2prev = nullptr;
            ep->m_p2child_1st = es1;
        };

        for( auto&& iter = m_entries.begin(); iter != m_entries.end(); ++iter )
        {
            Entry* e { iter->second };

            print_info( "Upgrading: ", e->get_name() );

            switch( DateOld::get_level( e->get_date() ) )
            {
                case 1:
                    if( e_last_l1 )
                        e_last_l1->add_sibling_before( e );
                    if( e_last_l2 )
                    {
                        add_children_to_parent_entry( e_last_l2, e );
                        e_last_l2 = nullptr;
                    }
                    e_last_l1 = e;
                    break;
                case 2:
                    if( e_last_l2 )
                        e_last_l2->add_sibling_before( e ); // as the map is ordered descendingly
                    if( e_last_l3 )
                    {
                        add_children_to_parent_entry( e_last_l3, e );
                        e_last_l3 = nullptr;
                    }
                    e_last_l2 = e;
                    break;
                case 3:
                    if( e_last_l3 )
                        e_last_l3->add_sibling_before( e ); // as the map is ordered descendingly
                    e_last_l3 = e;
                    break;
                case 4:
                    if( e_last_temporal )
                        e_last_temporal->add_sibling_after( e );
                    else if( !e_first_temporal )
                        e_first_temporal = e;
                    e_last_temporal = e;
                    break;
            }
        }

        if( e_last_l1 )
        {
            m_p2entry_1st = e_last_l1;
            if( e_first_temporal )
                m_p2entry_1st->get_sibling_last()->add_sibling_chain_after( e_first_temporal );
        }
        else if( e_first_temporal )
        {
            m_p2entry_1st = e_first_temporal;
        }
        else
        {
            print_error( "Upgrade error: Diary does not have a first level entry!" );
            return false;
        }

        // take care of orphan entries:
        if( e_last_l2 )
        {
            add_children_to_parent_entry( e_last_l2, e_last_l1 );
            int sorder{ 0 };
            for( Entry* e2o = e_last_l1->m_p2child_1st; e2o; e2o = e2o->m_p2next )
                e2o->m_sibling_order = ++sorder;
        }
        if( e_last_l3 )
        {
            add_children_to_parent_entry( e_last_l3, e_last_l1 );
            int sorder{ 0 };
            for( Entry* e2o = e_last_l1->m_p2child_1st; e2o; e2o = e2o->m_p2next )
                e2o->m_sibling_order = ++sorder;
        }

        // check if anything was left out:
        unsigned n_hierarchical_entries { 0 };
        for( Entry* eh = m_p2entry_1st; eh; eh = eh->get_next_straight() )
            ++n_hierarchical_entries;
        if( n_hierarchical_entries != m_entries.size() )
        {
            print_error( "Upgrade error: ", m_entries.size() - n_hierarchical_entries,
                         " entries could not be upgraded!" );
            return false;
        }

        for( auto&& kv_e : m_entries )
        {
            Entry* e{ kv_e.second };

            if( DateOld::is_ordinal( e->m_date ) )
            {
                if( DateOld::is_hidden( e->m_date ) )
                    e->set_title_style( VT::ETS::NAME_ONLY::I );
                else
                    e->set_title_style( VT::ETS::NUMBER_AND_NAME::I );

                // all entries are assigned to a date now:
                if( e->m_date_created < DateV( DateOld::DATE_MAX ) )
                    e->m_date = Date::make_from_ctime( e->m_date_created );
                else
                    e->m_date = e->m_date_created;
            }
            // if not already set as milestone:
            else if( e->get_title_style() != VT::ETS::MILESTONE::I )
            {
                e->set_title_style( VT::ETS::DATE_AND_NAME::I );
            }
        }
    }

    // update the dates:
    if( m_read_version < 2016 )
    {
        m_entries.clear_but_keep();

        for( Entry* e = m_p2entry_1st; e; e = e->get_next_straight() )
        {
            if( e->m_date < DateV( DateOld::DATE_MAX ) ) // if not already upgraded
                tmp_upgrade_date_to_3000( e->m_date );

            if( e->m_date_created < DateV( DateOld::DATE_MAX ) ) // if not already upgraded
                e->m_date_created = Date::make_from_ctime( e->m_date_created );

            if( e->m_date_edited < DateV( DateOld::DATE_MAX ) ) // if not already upgraded
                e->m_date_edited = Date::make_from_ctime( e->m_date_edited );
            if( m_read_version < 2014 )
                e->m_date_finish = Date::make_from_ctime( e->m_date_finish );
            else
                tmp_upgrade_date_to_3000( e->m_date_finish );

            m_entries.emplace( e->m_date, e );

            if( e->get_title_style() == VT::ETS::DATE_AND_NAME::I &&
                e->m_date != e->m_date_created )
            {
                if( e->m_p2para_1st->m_p2next )
                    e->m_p2para_1st->m_p2next->insert_text_with_spaces(
                            0, Date::format_string( e->m_date ), nullptr );
                else
                    e->add_paragraph_before( Date::format_string( e->m_date ), nullptr );
            }
        }
        for( auto kv_filter : m_filters )
        {
            auto        def{ kv_filter.second->get_definition() };
            Ustring     def_new;
            String      line;
            UstringSize line_offset{ 0 };
            bool        F_replace_def;

            while( STR::get_line( def, line_offset, line ) )
            {
                if( STR::begins_with( line, "Fd" ) )
                {
                    int       i_date{ 3 };
                    DateV     date_b{ STR::get_i64( line, i_date ) };
                    const int i_f_icl_e{ i_date };
                    DateV     date_e{ STR::get_i64( line, ++i_date ) };

                    date_b = Date::make( DateOld::get_year( date_b ),
                                            DateOld::get_month( date_b ),
                                            DateOld::get_day( date_b ) );
                    date_e = Date::make( DateOld::get_year( date_e ),
                                            DateOld::get_month( date_e ),
                                            DateOld::get_day( date_e ) );

                    def_new += STR::compose( "\nFd", line[ 2 ], date_b, line[ i_f_icl_e ], date_e );
                    F_replace_def = true;
                }
                else if( line_offset > 3 )
                    def_new += ( "\n" + line );
                else
                    def_new = line;
            }

            if( F_replace_def )
                kv_filter.second->set_definition( def_new );
        }
    }

    // convert old paragraphs to 2013:
    if( m_read_version < 2013 )
    {
        for( Entry* e = m_p2entry_1st; e; e = e->get_next_straight() )
        {
            for( Paragraph* para = e->m_p2para_1st; para; para = para->m_p2next )
            {
                if( para->m_text.empty() ) continue;

                if( ( STR::begins_with( para->m_text, "chart:" ) ||
                      STR::begins_with( para->m_text, "file:" )  ||
                      STR::begins_with( para->m_text, "rel:" ) ) &&
                    para->m_text.find( ' ' ) == Ustring::npos &&
                    m_read_version < 2013 )
                {
                    if( para->m_text[ 0 ] == 'c' ) para->set_image_type( VT::PS_IMAGE_CHART );
                    else                           para->set_image_type( VT::PS_IMAGE_FILE );
                    para->m_uri = para->m_text;
                    para->m_text.clear();
                }
                else
                {
                    ParserUpgrader parser_upg;
                    parser_upg.parse( para, m_read_version );
                }

                if( m_read_version > 2011 ) continue; // 2012 changes above

                unsigned int indentation_level{ 0 };
                for( unsigned int i = 0; i < para->m_text.length(); i++ )
                {
                    if( para->m_text[ i ] == '\t' ) indentation_level++;
                    else if( para->m_text[ i ] != '.' || i != 0 ) break;
                }

                // SUBHEADER
                if( para->m_text[ 0 ] == ' ' && para->m_text[ 1 ] != ' ' )
                {
                    para->set_heading_level( VT::PS_SUBHDR );
                    para->m_text = para->m_text.substr( 1 );
                }
                // SUBSUBHEADER
                else if( STR::begins_with( para->m_text, "  " ) && para->m_text[ 3 ] != ' ' )
                {
                    para->set_heading_level( VT::PS_SSBHDR );
                    para->m_text = para->m_text.substr( 2 );
                }
                // IGNORE
                else if( STR::begins_with( para->m_text, ".\t" ) )
                {
                    para->set_code( true );
                    para->m_style |= ( VT::PS_INDENT_1 * indentation_level );
                    para->m_text = para->m_text.substr( indentation_level + 1 );
                }
                // TO-DO
                else if( indentation_level > 0 && para->m_text.length() >= indentation_level + 4 &&
                        para->m_text[ indentation_level ] == '[' &&
                        para->m_text[ indentation_level + 2 ] == ']' )
                {
                    switch( para->m_text[ indentation_level + 1 ] )
                    {
                        case ' ':
                            para->set_list_type( VT::PS_TODO );
                            break;
                        case '~':
                            para->set_list_type( VT::PS_PROGRS );
                            break;
                        case '+':
                            para->set_list_type( VT::PS_DONE );
                            break;
                        case 'x':
                        case 'X':
                        case '>':
                            para->set_list_type( VT::PS_CANCLD );
                            break;
                    }
                    for( auto format : para->m_formats )
                    {
                        format->pos_bgn -= 4;
                        format->pos_end -= 4;
                    }

                    para->m_style |= ( VT::PS_INDENT_1 * indentation_level );
                    para->m_text = para->m_text.substr( indentation_level + 4 );
                }
                // INDENTATION (if not already processed above)
                if( indentation_level > 0 && para->get_indent_level() == 0 )
                {
                    para->m_style |= ( VT::PS_INDENT_1 * indentation_level );
                    para->m_text = para->m_text.substr( indentation_level );
                    para->predict_list_style_from_text();
                }

                if( indentation_level > 0 )
                {
                    for( auto format : para->m_formats )
                    {
                        format->pos_bgn -= indentation_level;
                        format->pos_end -= indentation_level;
                    }
                }
            }
        }
    }

    if( m_read_version < 2017 )
    {
        for( auto kv_chart : m_charts )
        {
            ChartData cd( this );
            cd.set_from_string_old( kv_chart.second->get_definition() );
            kv_chart.second->set_definition( cd.get_as_string() );
        }

        for( auto& kv_table : m_tables )
        {
            const String&   def_old{ kv_table.second->get_definition() };
            String          def_new;
            String          line;
            StringSize      line_offset{ 0 };

            while( STR::get_line( def_old, line_offset, line ) )
            {
                if( STR::begins_with( line, "McoR" ) )
                    def_new += ( "McoRSF~~~~~" + line.substr( 5 ) + '\n' );
                else
                if( STR::begins_with( line, "Mco" ) )
                    def_new += ( line.substr( 0, 5 ) + "_~~~~~" + line.substr( 5 ) + '\n' );
                else
                    def_new += ( line + '\n' );
            }
            kv_table.second->set_definition( def_new );
        }
    }

    // convert filter refs by name
    if( m_read_version < 2018 )
    {
        for( auto kv_table : m_tables )
        {
            const Ustring   def         { kv_table.second->get_definition() };
            Ustring         def_new;
            String          line;
            StringSize      line_offset { 0 };

            while( STR::get_line( def, line_offset, line ) )
            {
                if( line.substr( 0, 4 ) == "Mccf" )
                {
                    auto filter{ get_filter( line.substr( 4 ) ) };
                    if( filter )
                        def_new += STR::compose( "Mccf",
                                                 filter->get_id(),
                                                 '\n' );
                }
                else
                if( line.substr( 0, 2 ) == "Mf" )
                {
                    auto filter{ get_filter( line.substr( 2 ) ) };
                    if( filter )
                        def_new += STR::compose( "Mf",
                                                 get_filter( line.substr( 2 ) )->get_id(),
                                                 '\n' );
                }
                else
                if( line.substr( 0, 2 ) == "Ml" )
                {
                    auto filter{ get_filter( line.substr( 2 ) ) };
                    if( filter )
                        def_new += STR::compose( "Ml",
                                                 get_filter( line.substr( 2 ) )->get_id(),
                                                 '\n' );
                }
                else
                    def_new += ( line + '\n' );
            }

            kv_table.second->set_definition( def_new );
        }
    }

    //prnt_siblings( e_last_l1, 0 );

    return true;
}

void
Diary::do_standard_checks_after_read()
{
    if( m_themes.empty() )
    {
        ThemeSystem::get()->copy_to( create_theme( ThemeSystem::get()->get_name() ) );
    }

    // DEFAULT CHART AND TABLE
    if( m_p2chart_active == nullptr )
        m_p2chart_active = create_chart( _( STRING::DEFAULT ), ChartElem::DEFINITION_DEFAULT );
    if( m_p2table_active == nullptr )
        m_p2table_active = create_table( _( STRING::DEFAULT ), TableElem::DEFINITION_DEFAULT );

    // initialize derived theme colors
    for( auto& kv_theme : m_themes )
        kv_theme.second->calculate_derived_colors();

    if( m_startup_entry_id > DEID_MIN )
    {
        if( get_element( m_startup_entry_id ) == nullptr )
        {
            print_error( "Startup element ", m_startup_entry_id, " cannot be found in db" );
            m_startup_entry_id = HOME_CURRENT_ENTRY;
        }
    }

    if( m_p2filter_search )
        m_fc_search = m_p2filter_search->get_filterer_stack();

    if( m_entries.empty() )
    {
        print_info( "A dummy entry added to the diary" );
        create_entry_dummy();
    }

    m_thread_postread_operations = new std::thread( &Diary::do_postread_operations, Diary::d );
}

void
Diary::do_postread_operations()
{
    {
        std::lock_guard< std::mutex > lock( m_mutex_postread_operations );
        m_F_stop_postread_operations = false;
        m_parser_bg_postread = new ParserBackGround;
    }

    for( auto e = m_p2entry_1st; e; e = e->get_next_straight() )
    {
        if( m_F_stop_postread_operations ) break;

        e->parse( m_parser_bg_postread );

        if( m_F_stop_postread_operations ) break;
    }

#if LIFEOGRAPH_DEBUG_BUILD
    if( m_F_stop_postread_operations )
        PRINT_DEBUG( "--- INTERRUPTED POST-READ OPERATIONS ---" );
    else
        PRINT_DEBUG( "--- FINISHED POST-READ OPERATIONS ---" );
#endif

    if( m_F_stop_postread_operations )
    {
        std::lock_guard< std::mutex > lock( m_mutex_postread_operations );
        m_F_stop_postread_operations = false;
    }
    else
        m_dispatcher_postread_operations.emit();
}

void
Diary::handle_postread_operations_finished()
{
    if( !m_thread_postread_operations )
        return;

    if( m_thread_postread_operations->joinable() )
        m_thread_postread_operations->join();

    delete m_thread_postread_operations;
    m_thread_postread_operations = nullptr;

    delete m_parser_bg_postread;
    m_parser_bg_postread = nullptr;

    clear_chart_and_table_images();
}

// PARSING FUNCTIONS
inline LIFEO::Result
Diary::parse_db_body_text()
{
    if     ( m_read_version >= 1997 && m_read_version <= 3000 )
        return parse_db_body_text_2000();
    else if( m_read_version >= 1010 && m_read_version <= 1050 )
        return parse_db_body_text_1050();
    else
        return LIFEO::FAILURE;
}

LIFEO::Result
Diary::parse_db_body_text_2000()
{
    String        line;
    Theme*        p2theme       { nullptr };
    Filter*       p2filter      { nullptr };
    ChartElem*    p2chart       { nullptr };
    TableElem*    p2table       { nullptr };
    Entry*        p2entry       { nullptr };
    Paragraph*    p2para        { nullptr };
    Paragraph*    p2para_coord  { nullptr };  // for upgrade
    Paragraph*    p2para_after  { nullptr };  // for upgrade

    while( getline( *m_sstream, line ) )
    {
        if( line.size() < 2 )
            continue;

        switch( line[ 0 ] )
        {
            // DIARY OPTION
            case 'D':
                switch( line[ 1 ] )
                {
                    case 'o':   // options
                        if( m_read_version > 2000 )
                            m_options |= VT::bitmapize_string< VT::DO >( line.substr( 2 ) );
                        else
                        {
                            // NOTE: sorting criteria is omitted in the upgrade
                            if( line[ 2 ] == 'A' ) m_options |= VT::DO::SHOW_ALL_ENTRY_LOCATIONS::I;
                            if( line.size() > 6 )
                                m_opt_ext_panel_cur = std::stoi( line.substr( 6, 1 ) );
                        }
                        break;
                    case 'p':   // spell checking language
                        m_opt_ext_panel_cur = std::stoi( line.substr( 2 ) );
                        break;
                    case 's':   // spell checking language
                        m_language = line.substr( 2 );
                        break;
                    case 'f':   // first entry to show
                        m_startup_entry_id = std::stoul( line.substr( 2 ) );
                        break;
                    case 'l':   // last entry shown in the previous session
                        m_last_entry_id = std::stoul( line.substr( 2 ) );
                        break;
                    case 'c':   // completion tag
                        m_completion_tag_id = std::stoul( line.substr( 2 ) );
                        break;
                }
                break;
            // ID (START OF A NEW ELEMENT)
            case 'I':   // id
                m_force_id = std::stoul( line.substr( 2 ) );
                break;
            // THEME
            case 'T':
                if( line[ 1 ] == ' ' ) // declaration
                {
                    p2theme = create_theme( line.substr( 3 ) );
                    // if( line[ 2 ] == 'D' ) // ignore, no longer relevant
                    //     m_p2theme_default = p2theme;
                }
                else
                    parse_theme( p2theme, line );
                break;
            // FILTER
            case 'F':
                if( line[ 1 ] == ' ' ) // declaration
                {
                    p2filter = create_filter( line.substr( m_read_version > 2000 ? 4 : 3 ), "" );
                    if( line[ 2 ] == 'A' )
                        m_p2filter_list = p2filter;
                    if( m_read_version > 2000 && line[ 3 ] == 'S' )
                        m_p2filter_search = p2filter;
                }
                else
                    p2filter->add_definition_line( line );
                break;
            // CHART
            case 'G':
                if( line[ 1 ] == ' ' )  // declaration
                {
                    p2chart = create_chart( line.substr( 3 ), "" );
                    if( line[ 2 ] == 'A' )
                        m_p2chart_active = p2chart;
                }
                else
                    p2chart->add_definition_line( line );
                break;
            // TABLE (MATRIX)
            case 'M':
                if( line[ 1 ] == ' ' )  // declaration
                {
                    p2table = create_table( line.substr( 3 ), "" );
                    if( line[ 2 ] == 'A' )
                        m_p2table_active = p2table;
                }
                else
                    p2table->add_definition_line( line );
                break;
            // CHAPTER CATEGORY
            case 'C':
                p2filter = create_filter( _( "Chapter Category:" ) + line.substr( 3 ), "F|" );
//                if( line[ 2 ] == 'A' ) // ignored
//                    m_p2chapter_ctg_cur = ptr2chapter_ctg;
                break;
            case 'W':
                for( int i = 0; i < 7; i++ ) m_weekends[ i ] = line[ 2 + i ] == '1';
                break;
            case 'H':
                m_holidays.insert( std::stoull( line.substr( 2 ) ) );
                break;
            // ENTRY / CHAPTER
            case 'E':
                switch( line[ 1 ] )
                {
                    case ' ':   // declaration
                    case '+':   // chapter declaration ( deprecated )
                    {
                        p2entry = create_entry( std::stoull( line.substr(
                                                        m_read_version == 2000 ? 6 : 17 ) ),
                                                line[ 2 ] == 'F',
                                                line[ 3 ] == 'T',
                                                line[ 5 ] == 'E' );

                        p2para_after = nullptr;

                        parse_todo_status( p2entry, line[ 4 ] );
                        if( line[ 1 ] == '+' )
                        {
                            p2entry->set_title_style( VT::ETS::MILESTONE::I );
                            p2filter->add_definition_line(
                                    STR::compose( "FiT", p2entry->get_id() ) );
                        }
                        if( m_read_version > 2000 )
                        {
                            p2entry->set_title_style( VT::get_v< VT::ETS,
                                                                 int,
                                                                 char >( line[ 6 ] ) );
                            p2entry->set_comment_style( VT::get_v< VT::CS,
                                                                   int,
                                                                   char >( line[ 7 ] ) );
                            // cached value:
                            p2entry->set_filtered_out( line[ 8 ] != '_', line[ 8 ] == 'h' );
                        }
                        break;
                    }
                    case 'b':   // bg color
                        p2entry->set_color( Color( line.substr( 2 ) ) );
                        break;
                    case 'c':
                        p2entry->m_date_created = std::stoull( line.substr( 2 ) );
                        break;
                    case 'e':
                        p2entry->m_date_edited = std::stoull( line.substr( 2 ) );
                        break;
                    case 't':   // finish date (cached)
                        // on v2000 this meant status change date
                        p2entry->m_date_finish = std::stoull( line.substr( 2 ) );
                        break;
                    case 'h':   // hierarchy
                        if( line[ 2 ] == 'a' ) // after
                        {
                            auto esb = get_entry_by_id( std::stoul( line.substr( 3 ) ) );
                            if( esb ) esb->add_sibling_after( p2entry );
                            // should never be the case but is here to prevent data loss when things go astray:
                            else m_p2entry_1st->add_sibling_after( p2entry );
                        }
                        else
                        if( line[ 2 ] == 'u' ) // under
                        {
                            auto ep = get_entry_by_id( std::stoul( line.substr( 3 ) ) );
                            if( ep ) ep->add_child_1st( p2entry );
                            // should never be the case but is here to prevent data loss when things go astray:
                            else m_p2entry_1st->add_sibling_after( p2entry );
                        }
                        else // 'r' i.e. root
                            m_p2entry_1st = p2entry;
                        break;
                    case 'm':
                        p2entry->set_theme( m_themes[ line.substr( 2 ) ] );
                        break;
                    case 's':   // spell checking language
                        p2entry->set_lang( line.substr( 2 ) );
                        break;
                    case 'u':   // unit
                        p2entry->set_unit( line.substr( 2 ) );
                        break;
                    case 'l':   // location -v2000
                    case 'r':   // path (route) -v2000
                        if( line[ 2 ] == 'a' )
                        {
                            p2para_coord =
                                    p2entry->add_map_path_point( STR::get_d( line.substr( 3 ) ),
                                                                 0.0, nullptr );
                            if( !p2para_after )
                                p2para_after = p2para_coord;
                        }
                        else if( line[ 2 ] == 'o' )
                            p2para_coord->m_location.longitude = STR::get_d( line.substr( 3 ) );
                        break;
                    case 'p':   // paragraph
                        if( line[ 2 ] == 'l' )  // LOCATION
                        {
                            if( line[ 3 ] == 'a' )
                                p2para->m_location.latitude = STR::get_d( line.substr( 4 ) );
                            else if(  line[ 3 ] == 'o' )
                                p2para->m_location.longitude = STR::get_d( line.substr( 4 ) );
                            break;
                        }
                        p2para = p2entry->add_paragraph_before(
                                line.substr( m_read_version == 2000 || line[ 2 ] == 'D' ? 3 : 18 ),
                                p2para_after ); // non-null only in old entries

                        if( line[ 2 ] == 'D' ) break; // default options

                        parse_para_alignment( p2para, line[ 2 ] );
                        if( m_read_version > 2000 )
                        {
                            parse_para_type( p2para, line[ 3 ] ); // heading type
                            parse_para_type( p2para, line[ 4 ] ); // list type
                            p2para->set_expanded( line[ 5 ] == 'E' );
                            p2para->set_quote( line[ 6 ] == 'Q' );
                            p2para->set_indent_level( std::stoi( line.substr( 7, 1 ) ) );

                            switch( line[ 8 ] )
                            {
                                case 'I':   p2para->set_image_type( VT::PS_IMAGE_FILE ); break;
                                case 'C':   p2para->set_image_type( VT::PS_IMAGE_CHART ); break;
                                case 'T':   p2para->set_image_type( VT::PS_IMAGE_TABLE ); break;
                            }
                            if( p2para->is_image() )
                            {
                                p2para->set_image_size( std::stoi( line.substr( 9, 1 ) ) );
                                // TODO: 3.2: below steps are dumb, try to set text to uri at first
                                p2para->m_uri = p2para->m_text;
                                p2para->m_text.clear();
                            }

                            if( line[ 10 ] == 'X' )
                                p2para->m_style |= VT::PS_IMAGE_EXPND;
                            if( line[ 11 ] == 'C' )
                                p2para->set_code( true );
                            if( line[ 12 ] == 'R' )
                                p2para->set_hrule( true );
                        }
                        break;
                    case 'f':   // formatting
                    {
                        int        index        { m_read_version == 2000 ? 2 : 3 };
                        int        format_type  { VT::HFT_LINK_URI };
                        const auto pos_bgn      { STR::get_i32( line, index ) };
                        const auto pos_end      { STR::get_i32( line, ++index ) };

                        format_type = HiddenFormat::get_type_from_char( line[ 2 ] );

                        auto&& format = p2para->add_format( format_type, line.substr( ++index ),
                                                            pos_bgn, pos_end );

                        // temporary fix for diaries of development versions
                        if( STR::begins_with( format->uri, "deid:" ) )
                        {
                            format->ref_id = std::stoul( format->uri.substr( 5 ) );
                            format->uri = "";
                            if( format->type != VT::HFT_TAG )
                                format->type = VT::HFT_LINK_ID;
                        }
                        else if( format_type == VT::HFT_TAG || format_type == VT::HFT_LINK_ID )
                        {
                            format->ref_id = std::stoul( format->uri );
                            format->uri = "";
                        }

                        break;
                    }
                }
                break;
            default:
                print_error( "Unrecognized line: [", line, "]" );
                clear();
                return LIFEO::CORRUPT_FILE;
        }
    }

    if( m_read_version == 2000 )
        tmp_upgrade_table_defs();

    if( m_read_version < DB_FILE_VERSION_INT )
        if( !upgrade_to_3000() )
            return LIFEO::FAILURE;

    do_standard_checks_after_read();

    return LIFEO::SUCCESS;
}

LIFEO::Result
Diary::parse_db_body_text_1050()
{
    std::string         read_buffer;
    std::string         line;
    std::string::size_type line_offset{ 0 };
    Entry*              p2entry{ nullptr };
    unsigned int        tag_o1{ 0 };
    unsigned int        tag_o2{ 0 };
    unsigned int        tag_o3{ 0 };
    Filter*             p2filter{ nullptr };
    Theme*              p2theme{ nullptr }, * p2theme_untagged{ nullptr };
    bool                flag_in_tag_ctg{ false };
    Ustring             filter_def{ "F&" };
    std::map< Entry*, Value >     entry_tags;
    Ustring             chart_def_default{ ChartElem::DEFINITION_DEFAULT };
    //SortCriterion       sorting_criteria{ SoCr_DEFAULT };

    // PREPROCESSING TO DETERMINE FIRST AVAILABLE ORDER
    while( getline( *m_sstream, line ) )
    {
        read_buffer += ( line + '\n' );

        if( line[ 0 ] == 'C' && line[ 1 ] == 'G' )
            tag_o1++;
    }

    // TAGS TOP LEVEL
    p2entry = new Entry( this, DateOld::make_ordinal( false, ++tag_o1, 0 ) );
    m_entries.emplace( p2entry->m_date, p2entry );
    p2entry->add_paragraph_before( "[###>", nullptr );
    p2entry->set_expanded( true );

    // TAG DEFINITIONS & CHAPTERS
    while( STR::get_line( read_buffer, line_offset, line ) )
    {
        if( line.empty() )    // end of section
            break;
        else if( line.size() >= 3 )
        {
            switch( line[ 0 ] )
            {
                case 'I':   // id
                    m_force_id = std::stoul( line.substr( 2 ) );
                break;
                // TAGS
                case 'T':   // tag category
                    p2entry = new Entry( this, DateOld::make_ordinal( false, tag_o1, ++tag_o2 ) );
                    m_entries.emplace( p2entry->m_date, p2entry );
                    p2entry->add_paragraph_before( line.substr( 2 ), nullptr );
                    p2entry->set_expanded( line[ 1 ] == 'e' );
                    flag_in_tag_ctg = true;
                    tag_o3 = 0;
                    break;
                case 't':   // tag
                    switch( line[ 1 ] )
                    {
                        case ' ':
                            p2entry = new Entry(
                                    this,
                                    flag_in_tag_ctg ?
                                            DateOld::make_ordinal( false, tag_o1, tag_o2,
                                                                                  ++tag_o3 ) :
                                            DateOld::make_ordinal( false, tag_o1, ++tag_o2 ) );
                            m_entries.emplace( p2entry->m_date, p2entry );
                            p2entry->add_paragraph_before( line.substr( 2 ), nullptr );
                            p2theme = nullptr;
                            break;
                        case 'c': // not used in 1010
                            tmp_create_chart_from_tag(
                                    p2entry, std::stol( line.substr( 2 ) ), this );
                            break;
                        case 'u': // not used in 1010
                            if( p2entry )
                                p2entry->set_unit( line.substr( 2 ) );
                            break;
                    }
                    break;
                case 'u':
                    if( !p2theme_untagged && line[ 1 ] != 'c' ) // chart is ignored
                        p2theme_untagged = create_theme( ThemeSystem::get()->get_name() );
                    parse_theme( p2theme_untagged, line );
                    break;
                case 'm':
                    if( p2theme == nullptr )
                        p2theme = create_theme( p2entry->get_name() );
                    parse_theme( p2theme, line );
                    break;
                // DEFAULT FILTER
                case 'f':
                    switch( line[ 1 ] )
                    {
                        case 's':   // status
                            if( m_read_version < 1020 )
                            {
                                filter_def += "\nFsN";
                                filter_def += ( line[ 6 ] == 'T' ) ? 'O' : 'o';
                                // made in-progress entries depend on the preference for open ones:
                                filter_def += ( line[ 6 ] == 'T' ) ? 'P' : 'p';
                                filter_def += ( line[ 7 ] == 'D' ) ? 'D' : 'd';
                                filter_def += ( line[ 8 ] == 'C' ) ? 'C' : 'c';
                            }
                            else
                            {
                                filter_def += "\nFs";
                                filter_def += ( line[ 6 ] == 'N' ) ? 'N' : 'n';
                                filter_def += ( line[ 7 ] == 'T' ) ? 'O' : 'o';
                                filter_def += ( line[ 8 ] == 'P' ) ? 'P' : 'p';
                                filter_def += ( line[ 9 ] == 'D' ) ? 'D' : 'd';
                                filter_def += ( line[ 10 ] == 'C' ) ? 'C' : 'c';
                            }
                            if( line[ 2 ] == 'T' && line[ 3 ] != 't' ) filter_def += "\nFty";
                            else if( line[ 2 ] != 'T' && line[ 3 ] == 't' ) filter_def += "\nFtn";
                            if( line[ 4 ] == 'F' && line[ 5 ] != 'f' ) filter_def += "\nFfy";
                            else if( line[ 4 ] != 'F' && line[ 5 ] == 'f' ) filter_def += "\nFfn";
                            break;
                        case 't':   // tag
                        {
                            auto e_tag{ get_entry_by_name( line.substr( 2 ) ) };
                            if( e_tag )
                                filter_def += STR::compose( "\nFt", e_tag->get_id() );
                            else
                                print_error( "Reference to undefined tag: ", line.substr( 2 ) );
                            break;
                        }
                        case 'b':   // begin date: in the new system this is an after filter
                            filter_def += STR::compose( "\nFa", line.substr( 2 ) );
                            break;
                        case 'e':   // end date: in the new system this is a before filter
                            filter_def += STR::compose( "\nFb", line.substr( 2 ) );
                            break;
                    }
                    break;
                // CHAPTERS
                case 'o':   // ordinal chapter (topic) (<1020)
                    p2entry = new Entry( this, get_db_line_date( line ) );
                    tmp_upgrade_ordinal_date_to_2000( p2entry->m_date );
                    m_entries.emplace( p2entry->m_date, p2entry );
                    p2entry->set_text( get_db_line_name( line ), nullptr );
                    p2entry->set_expanded( line[ 1 ] == 'e' );
                    break;
                case 'd':   // to-do group (<1020)
                    if( line[ 1 ] == ':' ) // declaration
                    {
                        p2entry = new Entry( this, get_db_line_date( line ) );
                        tmp_upgrade_ordinal_date_to_2000( p2entry->m_date );
                        m_entries.emplace( p2entry->m_date, p2entry );
                        p2entry->set_text( get_db_line_name( line ), nullptr );
                    }
                    else // options
                    {
                        p2entry->set_expanded( line[ 2 ] == 'e' );
                        if( line[ 3 ] == 'd' )
                            p2entry->set_todo_status( ES::DONE );
                        else if( line[ 3 ] == 'c' )
                            p2entry->set_todo_status( ES::CANCELED );
                        else
                            p2entry->set_todo_status( ES::TODO );
                    }
                    break;
                case 'c':   // temporal chapter (<1020)
                    if( p2filter )
                    {
                        p2entry = create_entry( get_db_line_date( line ), false, false, true );
                        p2entry->set_text( get_db_line_name( line ), nullptr );
                        p2filter->add_definition_line( STR::compose( "FiT", p2entry->get_id() ) );
                        p2entry->set_title_style( VT::ETS::MILESTONE::I );
                    }
                    else
                        print_error( "No chapter category defined" );
                    break;
                case 'C':
                    if( m_read_version < 1020 ) // chapter category
                    {
                        p2filter = create_filter( _( "Chapter Category:" ) + line.substr( 2 ),
                                                  "F|" );
                        break;
                    }
                    switch( line[ 1 ] ) // any chapter item based on line[1] (>=1020)
                    {
                        case 'C':   // chapter category
                            p2filter = create_filter( _( "Chapter Category:" ) + line.substr( 3 ),
                                                      "F|" );
                            break;
                        case 'c':   // chapter color
                            p2entry->set_color( Color( line.substr( 2 ) ) );
                            break;
                        case 'T':   // temporal chapter
                            p2entry = create_entry( get_db_line_date( line ), false, false, true );
                            p2entry->set_text( get_db_line_name( line ), nullptr );
                            p2filter->add_definition_line( STR::compose( "FiT",
                                                                         p2entry->get_id() ) );
                            p2entry->set_title_style( VT::ETS::MILESTONE::I );
                            break;
                        case 'O':   // ordinal chapter (used to be called topic)
                        case 'G':   // free chapter (replaced todo_group in v1020)
                            p2entry = new Entry( this, get_db_line_date( line ) );
                            tmp_upgrade_ordinal_date_to_2000( p2entry->m_date );
                            m_entries.emplace( p2entry->m_date, p2entry );
                            p2entry->set_text( get_db_line_name( line ), nullptr );
                            break;
                        case 'p':   // chapter preferences
                            p2entry->set_expanded( line[ 2 ] == 'e' );
                            parse_todo_status( p2entry, line[ 3 ] );
                            //line[ 4 ] (Y) is ignored as we no longer create charts for chapters
                            break;
                    }
                    break;
                case 'O':   // options
//                    switch( line[ 2 ] )
//                    {
//                        case 'd': sorting_criteria = SoCr_DATE_ASC; break;
//                        case 's': sorting_criteria = SoCr_SIZE_C_ASC; break;
//                        case 'c': sorting_criteria = SoCr_CHANGE_ASC; break;
                        // TODO: 3.1: find a way to pass this to the filters
//                    }
                    if( m_read_version == 1050 )
                    {
                        //m_sorting_criteria |= ( line[ 3 ] == 'd' ? SoCr_DESCENDING : SoCr_ASCENDING );
                        // NOTE: It is not worth the effort to incorporate the sorting direction
                        if( line.size() > 4 && line[ 4 ] == 'Y' )
                            chart_def_default = ChartElem::DEFINITION_DEFAULT_Y;
                    }
                    else if( m_read_version == 1040 )
                    {
                        if( line.size() > 3 && line[ 3 ] == 'Y' )
                            chart_def_default = ChartElem::DEFINITION_DEFAULT_Y;
                    }
                    break;
                case 'l':   // language
                    m_language = line.substr( 2 );
                    break;
                case 'S':   // startup action
                    m_startup_entry_id = std::stoul( line.substr( 2 ) );
                    break;
                case 'L':
                    m_last_entry_id = std::stoul( line.substr( 2 ) );
                    break;
                default:
                    print_error( "Unrecognized line:\n", line );
                    clear();
                    return LIFEO::CORRUPT_FILE;
            }
        }
    }

    // ENTRIES
    p2entry = nullptr;
    while( STR::get_line( read_buffer, line_offset, line ) )
    {
        if( line.size() < 2 )
            continue;
        else if( line[ 0 ] != 'I' && line[ 0 ] != 'E' && line[ 0 ] != 'e' && p2entry == nullptr )
        {
            print_error( "No entry declared for the attribute" );
            continue;
        }

        switch( line[ 0 ] )
        {
            case 'I':
                m_force_id = std::stoul( line.substr( 2 ) );
                break;
            case 'E':   // new entry
            case 'e':   // trashed
                if( line.size() < 5 )
                    continue;

                // add tags as inline tags
                tmp_add_tags_as_paragraph( p2entry, entry_tags );

                p2entry = new Entry( this, std::stoul( line.substr( 4 ) ),
                                     line[ 1 ] == 'f' ? ES::ENTRY_DEFAULT_FAV : ES::ENTRY_DEFAULT );
                tmp_upgrade_ordinal_date_to_2000( p2entry->m_date );
                m_entries.emplace( p2entry->m_date, p2entry );

                if( line[ 0 ] == 'e' )
                    p2entry->set_trashed( true );
                if( line[ 2 ] == 'h' )
                    filter_def += STR::compose( "\nFn", p2entry->get_id() );

                parse_todo_status( p2entry, line[ 3 ] );

                // all hidden entries were to-do items once:
                if( m_read_version < 1020 && DateOld::is_hidden( p2entry->get_date() ) )
                    p2entry->set_todo_status( ES::TODO );

                break;
            case 'D':   // creation & change dates (optional)
                switch( line[ 1 ] )
                {
                    case 'r':
                        p2entry->m_date_created = std::stoul( line.substr( 2 ) );
                        break;
                    case 'h':
                        p2entry->m_date_edited = std::stoul( line.substr( 2 ) );
                        break;
                    case 's':
                        p2entry->m_date_finish = std::stoul( line.substr( 2 ) );
                        //p2entry->set_date_finish_explicit( true );
                        break;
                }
                break;
            case 'T':   // tag
            {
                NameAndValue&& nav{ NameAndValue::parse( line.substr( 2 ) ) };
                Entry* e_tag{ get_entry_by_name( nav.name ) };
                if( e_tag )
                {
                    entry_tags.emplace( e_tag, nav.value );
                    if( line[ 1 ] == 'T' )
                        p2entry->set_theme( m_themes[ nav.name ] );
                }
                else
                    print_error( "Reference to undefined tag: ", nav.name );
                break;
            }
            case 'l':   // language
                p2entry->set_lang( line.substr( 2 ) );
                break;
            case 'P':    // paragraph
                p2entry->add_paragraph_before( line.substr( 2 ), nullptr );
                break;
            default:
                print_error( "Unrecognized line:\n", line );
                clear();
                return LIFEO::CORRUPT_FILE;
        }
    }

    // add tags to the last entry as inline tags
    tmp_add_tags_as_paragraph( p2entry, entry_tags );

    if( m_read_version < 1030 )
        upgrade_to_1030();

    if( !upgrade_to_3000() )
        return LIFEO::FAILURE;

    // this has to be done after 2*** update as it relies on paragraph statuses to be set:
    if( m_read_version < 1050 )
        upgrade_to_1050();

    // DEFAULT FILTER AND CHART
    m_p2filter_list = create_filter( _( STRING::DEFAULT ), filter_def );
    m_p2chart_active = create_chart( _( STRING::DEFAULT ), chart_def_default );

    do_standard_checks_after_read();

    return LIFEO::SUCCESS;
}

// READING
LIFEO::Result
Diary::read_header()
{
    m_sstream = new std::stringstream;

    try
    {
        auto  giofile   { Gio::File::create_for_uri( m_F_continue_from_lock ? m_uri + LOCK_SUFFIX
                                                                            : m_uri ) };
        auto  ifstream  { giofile->read() };
        auto  finfo     { giofile->query_info() };
        gsize bytes_read;

        m_size_bytes = finfo->get_size();
        auto  buffer    { new char[ m_size_bytes ] };

        if( !ifstream->read_all( buffer, m_size_bytes, bytes_read ) )
        {
            throw LIFEO::Error( "Couldn't read all!" );
        }

        m_sstream->write( buffer, bytes_read );

        delete[] buffer;
        ifstream->close();
    }
    catch( Glib::Error& error )
    {
        print_error( "Failed to open diary file: ", m_uri, "\n", error.what() );
        clear();
        return LIFEO::COULD_NOT_START;
    }

    String line;
    getline( *m_sstream, line );

    if( line != DB_FILE_HEADER )
    {
        clear();
        return LIFEO::CORRUPT_FILE;
    }

    while( getline( *m_sstream, line ) )
    {
        switch( line[ 0 ] )
        {
            case 'V':
                m_read_version = std::stoi( line.substr( 2 ) );
                if( m_read_version < DB_FILE_VERSION_INT_MIN )
                {
                    clear();
                    return LIFEO::INCOMPATIBLE_FILE_OLD;
                }
                else if( m_read_version > DB_FILE_VERSION_INT )
                {
                    clear();
                    return LIFEO::INCOMPATIBLE_FILE_NEW;
                }
                break;
            case 'E':
                // passphrase is set to a dummy value to indicate that diary
                // is an encrypted one until user enters the real passphrase
                m_passphrase = ( line[ 2 ] == 'y' ? " " : "" );
                break;
            case 'I':
                m_force_id = std::stoul( line.substr( 2 ) );
                break;
            case 0:
                set_id( create_new_id( this ) );
                return( m_read_version ? LIFEO::SUCCESS : LIFEO::CORRUPT_FILE );
            default:
                print_error( "Unrecognized header line: ", line );
                break;
        }
    }

    clear();
    return LIFEO::CORRUPT_FILE;
}

LIFEO::Result
Diary::read_body()
{
    Result res { SUCCESS };

    if( !m_passphrase.empty() )
        res = decrypt_buffer();

    if( res == SUCCESS )
        res = parse_db_body_text() ;

    if( m_sstream )
    {
        delete m_sstream;
        m_sstream = nullptr;
    }

    if( res == SUCCESS )
        m_login_status = LOGGED_IN_RO;

    return res;
}

LIFEO::Result
Diary::decrypt_buffer()
{
    CipherBuffers buf;

    try
    {
        // allocate memory for salt
        buf.salt = new unsigned char[ LIFEO::Cipher::cSALT_SIZE ];
        // read salt value
        m_sstream->read( ( char* ) buf.salt, LIFEO::Cipher::cSALT_SIZE );

        buf.iv = new unsigned char[ LIFEO::Cipher::cIV_SIZE ];
        // read IV
        m_sstream->read( ( char* ) buf.iv, LIFEO::Cipher::cIV_SIZE );

        LIFEO::Cipher::expand_key( m_passphrase.c_str(), buf.salt, &buf.key );

        // calculate bytes of data in file
        const size_t size { m_size_bytes - m_sstream->tellg() };
        if( size <= 3 )
        {
            buf.clear();
            clear();
            return LIFEO::CORRUPT_FILE;
        }
        buf.buffer = new unsigned char[ size ];
        if( ! buf.buffer )
            throw LIFEO::Error( "Unable to allocate memory for buffer" );

        m_sstream->read( ( char* ) buf.buffer, size );
        LIFEO::Cipher::decrypt_buffer( buf.buffer, size, buf.key, buf.iv );

        // passphrase check
        if( buf.buffer[ 0 ] != m_passphrase[ 0 ] || buf.buffer[ 1 ] != '\n' )
        {
            buf.clear();
            clear();
            return LIFEO::WRONG_PASSWORD;
        }
    }
    catch( ... )
    {
        buf.clear();
        clear();
        return LIFEO::COULD_NOT_START;
    }

    delete m_sstream;
    m_sstream = new std::stringstream;
    buf.buffer += 2;          // ignore first two chars which are for passphrase checking
    *m_sstream << buf.buffer; // pass the decrypted contents form the temp buffer to the sstream
    buf.buffer -= 2;          // restore pointer to the start of the buffer before deletion
    buf.clear();

    return LIFEO::SUCCESS;
}

// WRITING
inline char
get_entry_todo_status_char( const Entry* p2entry )
{
    switch( p2entry->get_todo_status() )
    {
        case ES::TODO:                          return 't';
        case ( ES::NOT_TODO | ES::TODO ):       return 'T';
        case ES::PROGRESSED:                    return 'p';
        case ( ES::NOT_TODO | ES::PROGRESSED ): return 'P';
        case ES::DONE:                          return 'd';
        case ( ES::NOT_TODO | ES::DONE ):       return 'D';
        case ES::CANCELED:                      return 'c';
        case ( ES::NOT_TODO | ES::CANCELED ):   return 'C';
        case ES::NOT_TODO:
        default:    /* should never occur */    return 'n';
    }
}
inline char
get_para_alignment_char( const int style )
{
    switch( style & VT::PS_FLT_ALIGN )
    {
        case VT::PS_ALIGN_C:    return '|';
        case VT::PS_ALIGN_R:    return '>';
        //case VT::PS_ALIGN_L:
        default:                return '<';
    }
}
inline char
get_para_heading_char( const int style )
{
    switch( style & VT::PS_FLT_HEADER )
    {
        case VT::PS_SUBHDR:     return 'S';
        case VT::PS_SSBHDR:     return 'B';
        default:                return '_';
    }
}
inline char
get_para_list_type_char( const int style )
{
    switch( style & VT::PS_FLT_LIST )
    {
        case VT::PS_TODO:       return 'O';
        case VT::PS_PROGRS:     return '~';
        case VT::PS_DONE:       return '+';
        case VT::PS_CANCLD:     return 'X';
        case VT::PS_BULLET:     return '-';
        case VT::PS_NUMBER:     return '1';
        case VT::PS_CLTTR:      return 'A';
        case VT::PS_SLTTR:      return 'a';
        case VT::PS_CROMAN:     return 'R';
        case VT::PS_SROMAN:     return 'r';
        default:                return '_';
    }
}
inline String
get_para_image_chars( const Paragraph* para )
{
    char t;
    switch( para->m_style & VT::PS_FLT_IMAGE )
    {
        case VT::PS_IMAGE_FILE:     t = 'I'; break;
        case VT::PS_IMAGE_CHART:    t = 'C'; break;
        case VT::PS_IMAGE_TABLE:    t = 'T'; break;
        default:                    return "__";
    }

    return STR::compose( t, para->get_image_size() );
}

inline void
Diary::create_db_entry_text( const Entry* entry )
{
    // ENTRY DATE
    *m_sstream << "\n\nID" << entry->get_id()
           << "\nE " << ( entry->is_favorite() ? 'F' : '_' )
                     << ( entry->is_trashed() ? 'T' : '_' )
                     << get_entry_todo_status_char( entry )
                     << ( entry->is_expanded() ? 'E' : '_' )
                     << VT::get_v< VT::ETS, char, int >( entry->get_title_style() )
                     << VT::get_v< VT::CS, char, int >( entry->get_comment_style() )
                     << entry->is_filtered_out_completely()  // cached value
                     << "~~~~~~~~"    // reserved (8)
                     << entry->m_date;

    *m_sstream << "\nEc" << entry->m_date_created;
    *m_sstream << "\nEe" << entry->m_date_edited;
    if( entry->m_date_finish != entry->m_date )
        *m_sstream << "\nEt" << entry->m_date_finish;

    // HIERARCHY
    if( m_p2filter_save )
    {
        auto fc_save{ m_p2filter_save ? m_p2filter_save->get_filterer_stack() : nullptr };

        if( entry->get_prev_unfiltered( fc_save ) )
            *m_sstream << "\nEha" << entry->m_p2prev->get_id();
        else if( entry->get_parent_unfiltered( fc_save ) )
            *m_sstream << "\nEhu" << entry->m_p2parent->get_id();
        else
            *m_sstream << "\nEhr";

        if( fc_save ) delete fc_save;
    }
    else
    {
        if( entry->m_p2prev )
        *m_sstream << "\nEha" << entry->m_p2prev->get_id();
        else
        if( entry->m_p2parent )
            *m_sstream << "\nEhu" << entry->m_p2parent->get_id();
        else
            *m_sstream << "\nEhr";
    }

    // THEME
    if( entry->is_theme_set() )
        *m_sstream << "\nEm" << entry->get_theme()->get_name_std();

    // SPELLCHECKING LANGUAGE
    if( entry->get_lang() != LANG_INHERIT_DIARY )
        *m_sstream << "\nEs" << entry->get_lang().c_str();

    // UNIT
    if( entry->m_unit.empty() == false )
        *m_sstream << "\nEu" << entry->m_unit;

    if( entry->m_color != Color( "White" ) )
        *m_sstream << "\nEb" << convert_gdkrgba_to_html( entry->m_color );

    // PARAGRAPHS
    for( Paragraph* para = entry->m_p2para_1st; para; para = para->m_p2next )
    {
        if( para->m_style == VT::PS_DEFAULT && para->is_expanded() )
            *m_sstream << "\nEpD";
        else
            *m_sstream << "\nEp" << get_para_alignment_char( para->m_style )
                                 << get_para_heading_char( para->m_style )
                                 << get_para_list_type_char( para->m_style )
                                 << ( para->is_expanded() ? 'E' : 'C' )
                                 << ( para->is_quote() ? 'Q' : '_' )
                                 << ( para->get_indent_level() )
                                 << get_para_image_chars( para )
                                 << ( ( para->m_style & VT::PS_IMAGE_EXPND ) ? 'X' : '_' )
                                 << ( ( para->is_code() ) ? 'C' : '_' )
                                 << ( ( para->is_hrule() ) ? 'R' : '_' )
                                 << "~~~~~"; // reserved (5)
        *m_sstream << ( para->is_image() ? para->get_uri() : para->get_text_std() );

        if( para->has_location() )
            *m_sstream << "\nEpla" << para->m_location.latitude
                   << "\nEplo" << para->m_location.longitude;

        for( auto format : para->m_formats )
            if( ( format->type & ( VT::HFT_F_ONTHEFLY|VT::HFT_F_NOT_SAVED ) ) == 0 )
                *m_sstream << "\nEf" << VT::HFT_CHARS[ format->type & VT::HFT_FILTER_CHARS ]
                                     << format->pos_bgn << "|"
                                     << format->pos_end << "|"
                                     << ( ( format->type == VT::HFT_LINK_ID ||
                                            format->type == VT::HFT_TAG ) ?
                                          std::to_string( format->ref_id ) : format->uri );
    }
}

void
Diary::create_db_header_text( bool encrypted )
{
    *m_sstream << DB_FILE_HEADER;
    *m_sstream << "\nV " << DB_FILE_VERSION_INT;
    *m_sstream << "\nE " << ( encrypted ? 'y' : 'n' );
    *m_sstream << "\nId" << get_id(); // diary id
    *m_sstream << "\n\n"; // end of header
}

bool
Diary::create_db_body_text()
{
    // DIARY OPTIONS
    *m_sstream << "Do" << VT::stringize_bitmap< VT::DO >( m_options );

    *m_sstream << "\nDp" << m_opt_ext_panel_cur;

    // DEFAULT SPELLCHECKING LANGUAGE
    if( !m_language.empty() )
        *m_sstream << "\nDs" << m_language;

    // FIRST ENTRY TO SHOW AT STARTUP (HOME ITEM) & LAST ENTRY SHOWN IN PREVIOUS SESSION
    *m_sstream << "\nDf" << m_startup_entry_id;
    *m_sstream << "\nDl" << m_current_entry_id;

    // COMPLETION TAG
    if( m_completion_tag_id != DEID_UNSET )
        *m_sstream << "\nDc" << m_completion_tag_id;

    // THEMES
    for( auto& kv_theme : m_themes )
    {
        Theme* theme{ kv_theme.second };
        *m_sstream << "\n\nID" << theme->get_id();
        *m_sstream << "\nT ~" << theme->get_name_std(); // 1 reserved, remnant from default theme
        *m_sstream << "\nTf" << theme->font.to_string().c_str();
        *m_sstream << "\nTb" << convert_gdkrgba_to_html( theme->color_base );
        *m_sstream << "\nTt" << convert_gdkrgba_to_html( theme->color_text );
        *m_sstream << "\nTh" << convert_gdkrgba_to_html( theme->color_heading );
        *m_sstream << "\nTs" << convert_gdkrgba_to_html( theme->color_subheading );
        *m_sstream << "\nTl" << convert_gdkrgba_to_html( theme->color_highlight );
        if( theme->image_bg.empty() == false )
            *m_sstream << "\nTi" << theme->image_bg;
    }

    // FILTERS
    for( auto& kv_filter : m_filters )
    {
        *m_sstream << "\n\nID" << kv_filter.second->get_id();
        *m_sstream << "\nF " << ( kv_filter.second == m_p2filter_list ? 'A' : '_' )
                             << ( kv_filter.second == m_p2filter_search ? 'S' : '_' )
                             << kv_filter.first.c_str();
        *m_sstream << '\n' << kv_filter.second->get_definition().c_str();
    }

    // CHARTS
    for( auto& kv_chart : m_charts )
    {
        *m_sstream << "\n\nID" << kv_chart.second->get_id();
        *m_sstream << "\nG " << ( kv_chart.second == m_p2chart_active ? 'A' : '_' )
                             << kv_chart.first.c_str();
        *m_sstream << '\n' << kv_chart.second->get_definition().c_str();
    }

    // TABLES
    for( auto& kv_table : m_tables )
    {
        *m_sstream << "\n\nID" << kv_table.second->get_id();
        *m_sstream << "\nM " << ( kv_table.second == m_p2table_active ? 'A' : '_' )
                             << kv_table.first.c_str();
        *m_sstream << '\n' << kv_table.second->get_definition().c_str();
    }

    // HOLIDAYS
    *m_sstream << "\n\nWe" << m_weekends[ 0 ] << m_weekends[ 1 ] << m_weekends[ 2 ]
                           << m_weekends[ 3 ] << m_weekends[ 4 ] << m_weekends[ 5 ]
                           << m_weekends[ 6 ];

    for( auto& holiday : m_holidays ) *m_sstream << "\nHo" << holiday;

    auto fc_save{ m_p2filter_save ? m_p2filter_save->get_filterer_stack() : nullptr };

    // ENTRIES
    for( Entry* entry = m_p2entry_1st; entry; entry = entry->get_next_straight() )
    {
        // optionally only save filtered entries:
        if( fc_save && !fc_save->filter( entry ) ) continue;

        create_db_entry_text( entry );
    }

    if( fc_save ) delete fc_save;

    return true;
}

LIFEO::Result
Diary::write()
{
    if( m_F_read_only ) throw LIFEO::Error( "Trying to save read-only diary!" );

    // BACKUP FOR THE LAST VERSION BEFORE UPGRADE
    if( m_read_version != DB_FILE_VERSION_INT )
        copy_file_suffix( m_uri, ".", m_read_version, true );

    // BACKUP THE PREVIOUS VERSION
    auto file_diary { Gio::File::create_for_uri( m_uri ) };
    if( file_diary->query_exists() )
    {
        auto file_prev { Gio::File::create_for_uri( m_uri + ".~previousversion~" ) };
        file_diary->move( file_prev, Gio::File::CopyFlags::OVERWRITE );
    }

    // WRITE THE FILE
    const Result result{ write( m_uri ) };

    // DAILY BACKUP SAVES
    if( Lifeograph::settings.save_backups && !Lifeograph::settings.backup_folder_uri.empty() )
    {
        const auto&&  name_strp
        { STR::ends_with( m_name, ".diary" ) ? m_name.substr( 0, m_name.size() - 6 ) : m_name };
        auto          folder_backup
        { Gio::File::create_for_uri( Lifeograph::settings.backup_folder_uri ) };
        auto          file_backup
        { folder_backup->get_child( name_strp + "_(" +std::to_string( get_id() ) + ")_"
                                              + Date::format_string( Date::get_today(), "YMD", '-' )
                                              + ".diary" ) };

        file_diary->copy( file_backup, Gio::File::CopyFlags::OVERWRITE );

        print_info( "Daily backup saved" );
    }

    return result;
}

LIFEO::Result
Diary::write_copy( const String& uri, const String& passphrase, const Filter* filter )
{
    m_p2filter_save = filter;

    String passphrase_actual = m_passphrase;
    m_passphrase = passphrase;
    Result result{ write( uri ) };
    m_passphrase = passphrase_actual;

    m_p2filter_save = nullptr;

    return result;
}

LIFEO::Result
Diary::write_txt( const String& uri, const Filter* filter )
{
    auto      file_txt  { Gio::File::create_for_uri( uri ) }; // is PATH() is necessary on win?
    auto      ofstream  { file_txt->query_exists() ? file_txt->replace()
                                                   : file_txt->create_file() };
    StrStream ss;

    ss.imbue( std::locale( "C" ) ); // to prevent thousands separators

    // HELPERS
    const String separator         = "---------------------------------------------\n";
    const String separator_favored = "+++++++++++++++++++++++++++++++++++++++++++++\n";
    const String separator_thick   = "=============================================\n";

    // DIARY TITLE
    ss << separator_thick << m_name << '\n' << separator_thick << "\n\n";

    // ENTRIES
    auto fc_save { filter ? filter->get_filterer_stack() : nullptr };

    for( EntryIterReverse it_entry = m_entries.rbegin(); it_entry != m_entries.rend(); ++it_entry )
    {
        Entry* entry{ it_entry->second };

        // PURGE EMPTY OR FILTERED OUT ENTRIES
        if( entry->is_empty() || ( fc_save && !fc_save->filter( entry ) ) )
            continue;

        // FAVOREDNESS AND DATE
        ss << ( entry->is_favorite() ? separator_favored : separator );
        if( entry->get_title_style() == VT::ETS::DATE_AND_NAME::I )
            ss << Date::format_string( entry->get_date() ) << '\n';
        else if( entry->get_title_style() == VT::ETS::NUMBER_AND_NAME::I )
            ss << entry->get_number_str() << '\n';

        // TO-DO STATUS
        ss << entry->get_todo_status_as_text();

        // CONTENT
        bool F_first_para { true };

        entry->do_for_each_para(
            [ & ]( Paragraph* para )
            {
                ss << para->get_text_decorated().c_str() << '\n';
                if( F_first_para )
                {
                    ss << ( entry->is_favorite() ? separator_favored : separator );
                    F_first_para = false;
                }
            } );

        ss << "\n\n";
    }

    ss << '\n';

    gsize bytes_written;
    ofstream->write_all( ss.str(), bytes_written );
    ofstream->close();

    if( fc_save ) delete fc_save;

    return SUCCESS;
}

LIFEO::Result
Diary::write( const String& uri )
{
    Glib::RefPtr< Gio::FileOutputStream > ofstream;

    try
    {
        auto file { Gio::File::create_for_uri( uri ) }; // is PATH() is necessary on win?
        ofstream = ( file->query_exists() ? file->replace() : file->create_file() );
    }
    catch ( const Glib::Error& err )
    {
        print_error( err.what() );
        return LIFEO::FAILURE;
    }

    m_sstream = new std::stringstream;
    m_sstream->imbue( std::locale( "C" ) ); // to prevent thousands separators

    // WRITING THE HEADER
    create_db_header_text( is_encrypted() );

    gsize bytes_written;
    ofstream->write_all( m_sstream->str(), bytes_written );

    delete m_sstream;
    m_sstream = new std::stringstream;
    m_sstream->imbue( std::locale( "C" ) ); // to prevent thousands separators

    // WRITING THE BODY
    if( is_encrypted() )
        *m_sstream << m_passphrase[ 0 ] << '\n'; // first char of passphrase for validity checking

    create_db_body_text();

    // encryption
    if( is_encrypted() )
    {
        CipherBuffers buf;

        try {
            size_t size {  m_sstream->str().size() + 1 };

            LIFEO::Cipher::create_new_key( m_passphrase.c_str(), &buf.salt, &buf.key );

            LIFEO::Cipher::create_iv( &buf.iv );

            buf.buffer = new unsigned char[ size ];
            memcpy( buf.buffer, m_sstream->str().c_str(), size );

            LIFEO::Cipher::encrypt_buffer( buf.buffer, size, buf.key, buf.iv );

            gsize bytes_written;
            ofstream->write_all( buf.salt, LIFEO::Cipher::cSALT_SIZE, bytes_written );
            ofstream->write_all( buf.iv, LIFEO::Cipher::cIV_SIZE, bytes_written );
            ofstream->write_all( buf.buffer, size, bytes_written );

            buf.clear();
        }
        catch( const Glib::Error& err )
        {
            print_error( err.what() );
            buf.clear();
            return LIFEO::FAILURE;
        }
    }
    else
    {
        gsize bytes_written;
        ofstream->write_all( m_sstream->str(), bytes_written );
    }

    ofstream->close();
    return LIFEO::SUCCESS;
}

bool
Diary::remove_lock_if_necessary()
{
    if( m_login_status != LOGGED_IN_EDIT || m_uri.empty() )
        return false;

    auto file_lock { Gio::File::create_for_uri( m_uri + LOCK_SUFFIX ) };
    if( file_lock->query_exists() )
        file_lock->remove();

    return true;
}

bool
Diary::is_locked() const
{
    if( m_uri.empty() )
        return false;

    try
    {
        auto file_lock  { Gio::File::create_for_uri( m_uri + LOCK_SUFFIX ) };
#if LIFEOGRAPH_DEBUG_BUILD
        // the vleowcheck is redundant but the exception creates hassles during debug
        if( !file_lock->query_exists() ) return false;
#endif
        auto finfo      { file_lock->query_info( "standard::*" ) };
        return( finfo->get_size() > 0 );
    }
    catch( ... )
    {
        PRINT_DEBUG( "Lock file size query failed!" );
    }

    return false;
}

// ELEMENTS
DEID
Diary::create_new_id( DiaryElement* element )
{
    DEID retval;
    if( m_force_id == DEID_UNSET )
    {
        std::uniform_int_distribution<> dis0( 10'000'000, 99'999'999 );

        do { retval = dis0( m_random_gen ); }
        while( m_ids.find( retval ) != m_ids.end() );

        m_ids[ retval ] = element;
    }
    else
    {
        // never take over an already mapped id here. e.g. during duplication for undo
        if( m_ids.find( m_force_id ) == m_ids.end() )
            m_ids[ m_force_id ] = element;
        retval = m_force_id;
        m_force_id = DEID_UNSET;
    }

    return retval;
}

DiaryElement*
Diary::get_element( DEID id ) const
{
    PoolDEIDs::const_iterator iter( m_ids.find( id ) );
    return( iter == m_ids.end() ? nullptr : iter->second );
}

Entry*
Diary::get_startup_entry() const
{
    Entry* entry{ nullptr };

    switch( m_startup_entry_id )
    {
        case HOME_CURRENT_ENTRY:
            entry = get_entry_most_current();
            break;
        case HOME_LAST_ENTRY:
            entry = get_entry_by_id( m_last_entry_id );
            break;
        case DEID_UNSET:
            break;
        default:
            entry = get_entry_by_id( m_startup_entry_id );
            break;
    }

    if( entry == nullptr )
    {
        entry = m_p2entry_1st;
        print_info( "Failed to detect a startup entry. Will show the first entry.");
    }

    return entry;
}

Entry*
Diary::get_entry_most_current() const
{
    DateV   date        { Date::get_today() };
    DateV   diff_final  { Date::LATEST };
    DateV   diff_cur;
    Entry*  e_most_curr { nullptr };

    for( auto& kv_entry : m_entries )
    {
        Entry* entry { kv_entry.second };

        if( ! entry->is_filtered_out() )
        {
            diff_cur = ( entry->get_date() < date ? date - entry->get_date()
                                                  : entry->get_date() - date );
            if( diff_cur < diff_final )
            {
                diff_final = diff_cur;
                e_most_curr = entry;
            }
            else
                break;
        }
    }

    return e_most_curr;
}

// ENTRIES
Entry*
Diary::get_entry_today() const
{
    return get_entry_by_date( Date::get_today() );
}

Entry*
Diary::get_entry_by_date( const DateV date, bool filtered_too ) const
{
    EntryIterConst iter( m_entries.find( Date::isolate_YMD( date ) ) );
    if( iter != m_entries.end() )
        if( filtered_too || iter->second->is_filtered_out() == false )
            return iter->second;

    return nullptr;
}

VecEntries
Diary::get_entries_by_date( DateV date, bool filtered_too ) const // takes pure date
{
    VecEntries ev;
    auto       range{ m_entries.equal_range( Date::isolate_YMD( date ) ) };

    for( auto& iter = range.first; iter!=range.second; ++iter )
        if( filtered_too || iter->second->is_filtered_out() == false )
            ev.push_back( iter->second );

    return ev;
}

Entry*
Diary::get_entry_by_name( const Ustring& name ) const
{
    // iterating using m_p2entry_1st is not an option here because during db upgrade
    // m_p2entry_1st is not ready when this function is called
    for( auto kv_entry : m_entries )
    {
        if( kv_entry.second->m_name == name )
            return kv_entry.second;
    }

    return nullptr;
}

VecEntries
Diary::get_entries_by_filter( const Ustring& filter_name )
{
    VecEntries ev;
    auto&& it_filter{ m_filters.find( filter_name ) };

    if( it_filter != m_filters.end() )
    {
        FiltererContainer* fc{ it_filter->second->get_filterer_stack() };

        for( auto& kv_entry : m_entries )
            if( fc->filter( kv_entry.second ) )
                ev.push_back( kv_entry.second );

        delete fc;
    }

    return ev;
}

unsigned int
Diary::get_entry_count_on_day( const DateV date ) const
{
    return m_entries.count( date );
}

// Entry*
// Diary::get_entry_next_in_day( const Date& date ) const
// {
//     if( date.is_ordinal() )
//         return nullptr;

//     for( auto iter = m_entries.rbegin(); iter != m_entries.rend(); iter++ )
//     {
//         if( date.get_day() == Date::get_day( iter->first ) &&
//             date.get_order_3rd() < Date::get_order_3rd( iter->first ) )
//             return( iter->second );
//     }

//     return nullptr;
// }

Entry*
Diary::get_entry_first_untrashed() const
{
    for( auto& kv_entry : m_entries )
    {
        if( ! kv_entry.second->is_trashed() )
            return kv_entry.second;
    }
    return nullptr;
}

Entry*
Diary::get_entry_latest() const
{
    return( m_entries.rbegin()->second );
}

Ustring
Diary::get_entry_name( DEID id ) const
{
    switch( id )
    {
        case DEID_UNSET:            return STR0/SI::NONE;
        case HOME_CURRENT_ENTRY:    return STR0/SI::CURRENT_ENTRY;
        case HOME_LAST_ENTRY:       return STR0/SI::LAST_ENTRY;
    }

    auto entry { get_entry_by_id( id ) };
    return( entry ? entry->get_name() : STR0/SI::NA ) ;
}

void
Diary::set_entry_date( Entry* entry, DateV date )
{
    for( auto&& iter = m_entries.begin(); iter != m_entries.end(); ++iter )
    {
        if( iter->second == entry )
        {
            m_entries.erase( iter );
            break;
        }
    }

    entry->m_date = date;
    m_entries.emplace( Date::isolate_YMD( date ), entry );
}

void
Diary::update_entry_name_refs( Entry* entry )
{
    for( Entry* e = m_p2entry_1st; e; e = e->get_next_straight() )
    {
        for( Paragraph* para = e->m_p2para_1st; para; para = para->m_p2next )
        {
            for( auto&& format : para->m_formats )
            {
                if( format->type == VT::HFT_TAG && format->ref_id == entry->get_id() )
                {
                    para->replace_text( format->pos_bgn,
                                        format->pos_end - format->pos_bgn,
                                        entry->get_name_pure(), nullptr );
                }
            }
        }
    }
}

void
Diary::sort_entry_siblings( Entry* entry, EntryComparer&& compare, int direction )
{
    auto swap_siblings = []( Entry* ep, Entry* en )
    {
        Entry* epp = ep->m_p2prev;
        Entry* enn = en->m_p2next;
        en->m_p2prev = epp;
        en->m_p2next = ep;
        ep->m_p2prev = en;
        ep->m_p2next = enn;
        if( epp ) epp->m_p2next = en;
        if( enn ) enn->m_p2prev = ep;
    };

    auto normalize = []( int64_t i  ) -> int
    {
        if( i < 0 ) return -1;
        if( i > 0 ) return 1;
        return 0;
    };

    Entry* entry_bgn = entry->get_sibling_1st();
    Entry* entry_end = entry->get_sibling_last();
    do
    {
        Entry* entry_bgn_new = entry_end;
        Entry* entry_end_new = entry_bgn;

        // forward loop
        for( Entry* e = entry_bgn; e != entry_end; )
        {
            if( normalize( compare( e, e->m_p2next ) ) == direction )
            {
                swap_siblings( e, e->m_p2next );
                if( e == entry_bgn )
                    entry_bgn = e->m_p2prev; // e->m_p2prev is the former e->m_p2next
                if( e->m_p2prev == entry_end )
                {
                    entry_end_new = e->m_p2prev;
                    break;
                }
                else
                    entry_end_new = e;
            }
            else
                e = e->m_p2next;
        }

        entry_end = entry_end_new;

        // backward loop
        for( Entry* e = entry_end; e != entry_bgn; )
        {
            if( normalize( compare( e->m_p2prev, e ) ) == direction )
            {
                swap_siblings( e->m_p2prev, e ); // note that order is different here than above
                if( e == entry_end )
                    entry_end = e->m_p2next;
                if( e->m_p2next == entry_bgn )
                {
                    entry_bgn_new = e->m_p2next;
                    break;
                }
                else
                    entry_bgn_new = e;
            }
            else
                e = e->m_p2prev;
        }

        entry_bgn = entry_bgn_new;
    }
    while( entry_bgn->get_offset_from_1st_sibling() < entry_end->get_offset_from_1st_sibling() );

    if( entry->m_p2parent )
        entry->m_p2parent->m_p2child_1st = entry->get_sibling_1st();
    else
        m_p2entry_1st = entry->get_sibling_1st();
}

Entry*
Diary::get_milestone_before( const DateV date ) const
{
    for( auto& kv_entry : m_entries )
    {
        if( kv_entry.first <= date &&
            kv_entry.second->get_date_finish() >= date &&
            kv_entry.second->is_filtered_out() == false &&
            kv_entry.second->get_title_style() == VT::ETS::MILESTONE::I )
            return kv_entry.second;
    }

    return nullptr;
}

Entry*
Diary::create_entry( Entry* entry_rel, bool F_parent, DateV date, const Ustring& content,
                     int style )
{
    Entry* entry { new Entry( this, date, ES::ENTRY_DEFAULT ) };

    entry->set_text( content, &m_parser_bg );

    if( !entry_rel )
    {
        if( m_p2entry_1st )
            m_p2entry_1st->get_sibling_last()->add_sibling_after( entry );
        else
            m_p2entry_1st = entry;
    }
    else if( F_parent )
        entry_rel->add_child_last( entry );
    else
        entry_rel->add_sibling_after( entry );

    // Entry* entry_stylistic_src{ entry->m_p2prev ? entry->m_p2prev :
    //                                ( entry->m_p2next ? entry->m_p2next : entry->m_p2parent ) };

    if( style == VT::ETS::INHERIT::I ) // must never be the case when entry_rel = NULL
    {
        if( entry_rel && entry_rel->get_title_style() != VT::ETS::MILESTONE::I )
            entry->set_title_style( entry_rel->get_title_style() );
        else
            entry->set_title_style( VT::ETS::DATE_AND_NAME::I );
    }
    else
        entry->set_title_style( style );

    if( entry_rel )
    {
        entry->set_theme( entry_rel->get_theme() );
    }

    m_entries.emplace( Date::isolate_YMD( entry->m_date ), entry );

    return( entry );
}

Entry*
Diary::duplicate_entry( Entry* entry_rel )
{
    Entry* entry = new Entry( this, entry_rel->get_date(), entry_rel->get_todo_status() );

    for( Paragraph* p = entry_rel->get_paragraph_1st(); p; p = p->m_p2next )
        entry->add_paragraph_after( new Paragraph( p ), entry->get_paragraph_last() );

    if( entry->get_paragraph_1st() )
    {
        entry->get_paragraph_1st()->append( " (*)", nullptr );
        entry->update_name();
    }

    entry_rel->add_sibling_after( entry );

    entry->set_title_style( entry_rel->get_title_style() );
    entry->set_comment_style( entry_rel->get_comment_style() );

    if( entry_rel->is_theme_set() )
        entry->set_theme( entry_rel->get_theme() );

    m_entries.emplace( Date::isolate_YMD( entry->m_date ), entry );

    return( entry );
}

// THIS VERSION IS USED DURING DIARY FILE READ ONLY:
Entry*
Diary::create_entry( DateV date, bool flag_favorite, bool flag_trashed, bool flag_expanded )
{
    ElemStatus status{ ES::NOT_TODO |
                       ( flag_favorite ? ES::FAVORED : ES::NOT_FAVORED ) |
                       ( flag_trashed ? ES::TRASHED : ES::NOT_TRASHED ) };
    if( flag_expanded ) status |= ES::EXPANDED;

    Entry* entry = new Entry( this, date, status );

    m_entries.emplace( Date::isolate_YMD( date ), entry );

    return( entry );
}

Entry*
Diary::create_entry_dummy()
{
    return create_entry( nullptr, false, Date::get_today(), _( "New Entry" ),
                         VT::ETS::NAME_ONLY::I );
}
Entry*
Diary::create_entry_dated( Entry* entry_parent, DateV date, bool F_milestone )
{
    // FIRST DETECT THE IDEAL POSITION FOR THE ENTRY
    Entry*  entry_bgn   { entry_parent ? entry_parent->m_p2child_1st : m_p2entry_1st };
    Entry*  entry_sibl1 { nullptr };
    Entry*  entry_sibl2 { nullptr };
    Entry*  entry_rel   { entry_parent };
    bool    F_parent    { true };

    for( Entry* e = entry_bgn; e; e = e->m_p2next )
    {
        if( e->get_title_style() == VT::ETS::DATE_AND_NAME::I )
        {
            if( !entry_sibl1 )
                entry_sibl1 = e;
            else if( e->get_date() != entry_sibl1->get_date() )
            {
                entry_sibl2 = e;
                break;
            }
        }
    }

    if( entry_sibl1 || entry_sibl2 )
    {
        const bool F_ascending { entry_sibl2 ? entry_sibl1->get_date() < entry_sibl2->get_date()
                                             : false }; // descending is the default order

        for( Entry* e = entry_bgn; e; e = e->m_p2next )
        {
            if( e->get_title_style() == VT::ETS::DATE_AND_NAME::I &&
                ( ( F_ascending && e->get_date() >= date ) ||
                  ( !F_ascending && e->get_date() <= date ) ) )
            {
                entry_rel = ( e->m_p2prev ? e->m_p2prev : e->m_p2parent );
                F_parent = !( e->m_p2prev );
                break;
            }
        }
    }

    // CREATE THE ENTRY
    Entry* entry { create_entry( entry_rel,
                                 F_parent,
                                 date,
                                 "",
                                 F_milestone ? VT::ETS::MILESTONE::I
                                             : VT::ETS::DATE_AND_NAME::I ) };

    entry->add_paragraph_before( Date::format_string( date ), nullptr );

    return entry;
}

Entry*
Diary::remove_entry_from_hierarchy( Entry* entry )
{
    const Entry* parent { entry->get_parent() };

    if( entry->m_p2prev )
    {
        if( entry->m_p2child_1st ) // if has children
        {
            entry->m_p2prev->m_p2next = entry->m_p2child_1st;
            entry->m_p2child_1st->m_p2prev = entry->m_p2prev;
        }
        else
            entry->m_p2prev->m_p2next = entry->m_p2next;
    }

    if( entry->m_p2next )
    {
        if( entry->m_p2child_1st ) // if has children
        {
            Entry* child_last = entry->m_p2child_1st->get_sibling_last();
            entry->m_p2next->m_p2prev = child_last;
            child_last->m_p2next = entry->m_p2next;
        }
        else
            entry->m_p2next->m_p2prev = entry->m_p2prev;
    }

    if( entry->m_p2parent && entry->m_p2parent->m_p2child_1st == entry )
    {
        if( entry->m_p2child_1st )
        {
            entry->m_p2parent->m_p2child_1st = entry->m_p2child_1st;
        }
        else
            entry->m_p2parent->m_p2child_1st = entry->m_p2next;
    }

    // raise parent links for the children one level up:
    for( Entry* e = entry->m_p2child_1st; e != nullptr; e = e->m_p2next )
         e->m_p2parent = entry->m_p2parent;

    // the entry that will replace this after removal
    Entry* e_replacing = ( entry->m_p2child_1st ? entry->m_p2child_1st
                                : ( entry->m_p2next ? entry->m_p2next
                                        : ( entry->m_p2prev ? entry->m_p2prev
                                                : entry->m_p2parent ) ) );

    // fix Diary's 1st pointer if needed
    if( m_p2entry_1st == entry )
        m_p2entry_1st = e_replacing;

    entry->m_p2parent = nullptr;
    entry->m_p2prev = nullptr;
    entry->m_p2next = nullptr;

    // update the sibling orders:
    if( parent )
    {
        if( parent->get_child_1st() )
            parent->get_child_1st()->update_sibling_orders();
    }
    else
        m_p2entry_1st->update_sibling_orders();

    return e_replacing;
}

void
Diary::remove_entry_from_hierarchy_with_descendants( Entry* entry )
{
    if( entry->m_p2prev ) entry->m_p2prev->m_p2next = entry->m_p2next;
    if( entry->m_p2next ) entry->m_p2next->m_p2prev = entry->m_p2prev;
    if( entry->m_p2parent && entry->m_p2parent->m_p2child_1st == entry )
        entry->m_p2parent->m_p2child_1st = entry->m_p2next;

    if( m_p2entry_1st == entry )
        m_p2entry_1st = entry->m_p2next;

    entry->m_p2parent = nullptr;
    entry->m_p2prev = nullptr;
    entry->m_p2next = nullptr;
}

void
Diary::move_entry( Entry* p2entry2move, Entry* p2entry_target, const DropPosition& position )
{
    // CAUTION: tampers with the entry list order. list orders have to be updated after this func
    if( p2entry2move->get_list_order() == -400 ) return;

    p2entry2move->set_list_order( -400 );
    p2entry2move->do_for_each_descendant( []( Entry* e ){ e->set_list_order( -400 ); } );

    remove_entry_from_hierarchy_with_descendants( p2entry2move );

    switch( position )
    {
        default: // BEFORE
            p2entry_target->add_sibling_before( p2entry2move );
            break;
        case DropPosition::INTO:
            if( !p2entry_target->has_children() ) // ensure that the child will be visible
                p2entry_target->set_expanded( true );
            p2entry_target->add_child_last( p2entry2move );
            break;
        case DropPosition::AFTER:
            if( p2entry_target->is_expanded() && p2entry_target->has_children() )
                p2entry_target->get_child_1st()->add_sibling_before( p2entry2move );
            else
                p2entry_target->add_sibling_after( p2entry2move );
            break;
    }
}

void
Diary::move_entries( const EntrySelection* p2entries2move,
                     Entry* p2e_target,
                     const DropPosition& dp )
{
    // CAUTION: tampers with the entry list order. list orders have to be updated after this func
    Entry* e_prev { nullptr };
    for( Entry* e2m : *p2entries2move )
    {
        if( e_prev == nullptr )
            move_entry( e2m, p2e_target, dp );
        else
            move_entry( e2m, e_prev, DropPosition::AFTER );

        e_prev = e2m;
    }
}

Entry*
Diary::dismiss_entry( Entry* entry )
{
    // fix startup element:
    if( m_startup_entry_id == entry->get_id() )
        m_startup_entry_id = HOME_CURRENT_ENTRY;

    // remove from filters:
    remove_entry_from_filters( entry );

    // remove from map:
    auto range{ m_entries.equal_range( Date::isolate_YMD( entry->m_date ) ) };
    for( auto& iter = range.first; iter!=range.second; ++iter )
        if( iter->second->is_equal_to( entry ) )
        {
            m_entries.erase( iter );
            break;
        }

    // remove from hierarchy:
    Entry* entry_next = remove_entry_from_hierarchy( entry );

    // an open diary must always contain at least one entry
    if( entry_next == nullptr )
        entry_next = create_entry_dummy();

    delete entry;

    return entry_next;
}

bool
Diary::is_trash_empty() const
{
    for( auto& kv_entry : m_entries )
        if( kv_entry.second->is_trashed() )
            return false;

    return true;
}

int
Diary::get_time_span() const
{
    DateV dfd{ Date::NOT_SET };

    for( auto kv_e : m_entries )
    {
        const auto dfe{ kv_e.second->get_date_finish() };
        if( dfe > dfd )
            dfd = dfe;
    }

    return Date::calculate_days_between_abs( m_entries.rbegin()->first, dfd );
}

// SEARCHING
void
Diary::set_search_str( const Ustring& text )
{
    std::lock_guard< std::mutex > lock( m_mutex_search );
    m_search_text = text;
}
void
Diary::set_search_filter( const Filter* filter )
{
    //std::lock_guard< std::mutex > lock( m_mutex_search );
    m_p2filter_search = filter;
    if( m_fc_search ) delete m_fc_search;
    m_fc_search = ( filter ? filter->get_filterer_stack() : nullptr );
}

void
Diary::start_search( const int thread_count )
{
    const int entries_per_thread{ int( ceil( double( Diary::d->get_size() ) / thread_count ) ) };

    clear_matches();

    m_active_search_thread_count = thread_count;

    for( int i = 0; i < thread_count; i++ )
    {
        PRINT_DEBUG( "Adding thread #", i );
        const int i_bgn{ i * entries_per_thread };
        m_threads_search.push_back( new std::thread( &Diary::search_internal,
                                                     this,
                                                     i_bgn,
                                                     i_bgn + entries_per_thread ) );
    }
}

void
Diary::search_internal( int i_bgn, int i_end )
{
    if( !m_search_text.empty() )
    {
        // std::lock_guard< std::mutex > lock( m_mutex_search );

        FiltererContainer*  fc { m_p2filter_search ? m_p2filter_search->get_filterer_stack()
                                                   : nullptr };
        int                 i  { -1 };

        for( Entry* entry = m_p2entry_1st; entry; entry = entry->get_next_straight() )
        {
            if( ++i < i_bgn ) continue;
            else if( i >= i_end ) break;

            if( m_F_stop_search_thread )
            { PRINT_DEBUG( "--- INTERRUPTED SEARCH AT 1 ---" ); break; }

            if( fc && !fc->filter( entry ) )
                continue;

            search_internal_entry( entry );
        }
    }

    {
        std::lock_guard< std::mutex > lock( m_mutex_search );
        m_active_search_thread_count--;
        PRINT_DEBUG( "Searchng in entries: ", i_bgn, "...", i_end, " FINISHED" );
    }

    if( m_active_search_thread_count == 0 )
    {
        m_dispatcher_search.emit();
    }
}

inline void
Diary::search_internal_entry( const Entry* entry )
{
    const bool      F_match_case{ bool( m_options & VT::DO::SEARCH_MATCH_CASE::I ) };
    const auto      length      { m_search_text.length() };
    UstringSize     pos_para;

    for( Paragraph* para = entry->m_p2para_1st; para; para = para->m_p2next )
    {
        if( m_F_stop_search_thread )
        { PRINT_DEBUG( "--- INTERRUPTED SEARCH AT 2 ---" ); return; }

        const Ustring&& para_text{ F_match_case ? para->get_text()
                                                : STR::lowercase( para->get_text() ) };
        pos_para = 0;

        while( ( pos_para = para_text.find( m_search_text, pos_para ) ) != Ustring::npos )
        {
            if( m_F_stop_search_thread )
            { PRINT_DEBUG( "--- INTERRUPTED SEARCH AT 3 ---" ); return; }

            {
                std::lock_guard< std::mutex > lock( m_mutex_search );
                auto f = para->add_format( VT::HFT_MATCH, "", pos_para, pos_para + length );
                f->ref_id = entry->get_id();
                f->var_i = para->get_para_no();
                f->var_d = entry->get_date();   // this is used for sorting
                m_matches.insert( new HiddenFormat( *f ) );
            }

            pos_para += length;
        }
    }
}

void
Diary::remove_entry_from_search( const Entry* entry )
{
    if( m_search_text.empty() || !entry ) return;

    for( auto&& it_match = m_matches.begin(); it_match != m_matches.end(); )
    {
        if( ( *it_match )->ref_id == entry->get_id() )
        {
            delete( *it_match );
            it_match = m_matches.erase( it_match );
        }
        else
            ++it_match;
    }

    for( Paragraph* para = entry->m_p2para_1st; para; para = para->m_p2next )
    {
        para->remove_formats_of_type( VT::HFT_MATCH );
    }
}

void
Diary::update_search_for_entry( const Entry* entry )
{
    if( m_search_text.empty() || !entry ) return;
    if( m_p2filter_search && !m_fc_search->filter( entry ) ) return;

    remove_entry_from_search( entry );
    search_internal_entry( entry );
}

void
Diary::replace_match( const HiddenFormat* match, const Ustring& text_new )
{
    Entry*      e_host    { get_entry_by_id( match->ref_id ) };
    Paragraph*  para      { e_host->get_paragraph_by_no( match->var_i ) };
    const int   pos_bgn   { para->get_bgn_offset_in_host() + int( match->pos_bgn ) };

    e_host->add_undo_action( UndoableType::INSERT_TEXT, para, 1,
                             pos_bgn + m_search_text.length(),
                             pos_bgn + text_new.length() );

    para->erase_text( match->pos_bgn, m_search_text.length(), nullptr );
    para->insert_text( match->pos_bgn, text_new, &m_parser_bg );

    for( auto it{ m_matches.begin() }, end{ m_matches.end() }; it != end; )
    {
        HiddenFormat* f{ *it };
        if( f->ref_id == match->ref_id && f->type == match->type && f->pos_bgn == match->pos_bgn )
        {
            delete f;
            it = m_matches.erase( it );
        }
        else
            ++it;
    }
}

void
Diary::replace_all_matches( const Ustring& text_new )
{
    // TODO: v3.1: add undo support
    for( Entry* e = m_p2entry_1st; e; e = e->get_next_straight() )
    {
        for( Paragraph* p = e->m_p2para_1st; p; p = p->m_p2next )
        {
            for( auto it = p->m_formats.begin(); it != p->m_formats.end(); )
            {
                HiddenFormat* f { *it };

                if( f->type == VT::HFT_MATCH )
                {
                    const auto fmt_pos_bgn { f->pos_bgn };
                    const auto fmt_pos_end { f->pos_end };
                    // delete the match format in advance to prevent double deletion:
                    delete f;
                    it = p->m_formats.erase( it );
                    p->replace_text( fmt_pos_bgn, fmt_pos_end - fmt_pos_bgn, text_new );
                }
                else
                    ++it;
            }
        }
    }

    for( auto f : m_matches ) delete f;
    m_matches.clear();
}

// this version does not disturb the active search and is case-sensitive
void
Diary::replace_all_matches( const Ustring& oldtext, const Ustring& newtext )
{
    if( oldtext.empty() )
        return;

    UstringSize pos_para;
    // UstringSize pos_entry;
    const auto  length_old { oldtext.length() };
    const int   delta      { ( int ) newtext.length() - ( int ) length_old };
    int         delta_cum  { 0 };
    Paragraph*  para_cur   { nullptr };
    std::vector< std::pair< Paragraph*, int > >  matches;

    // FIND
    for( auto& kv_entry : m_entries )
    {
        // pos_entry = 0;

        for( Paragraph* para = kv_entry.second->m_p2para_1st; para; para = para->m_p2next )
        {
            const auto& para_text{ para->get_text() };
            pos_para = 0;

            while( ( pos_para = para_text.find( oldtext, pos_para ) ) != Ustring::npos )
            {
                matches.push_back( { para, pos_para } );
                pos_para += length_old;
            }

            // pos_entry += ( para_text.length() + 1 );
        }
    }

    // REPLACE
    for( auto&& m : matches )
    {
        if( m.first != para_cur )
        {
            delta_cum = 0;
            para_cur = m.first;
        }
        else
            m.second += delta_cum;

        m.first->erase_text( m.second, length_old, nullptr );
        m.first->insert_text( m.second, newtext, &m_parser_bg );

        delta_cum += delta;
    }
}

HiddenFormat*
Diary::get_match_at( int i )
{
    std::lock_guard< std::mutex > lock( m_mutex_search );

    if( m_matches.empty() || i >= int( m_matches.size() ) )
        return nullptr;

    auto&& iter{ m_matches.begin() };
    std::advance( iter, i );

    return( *iter );
}

void
Diary::clear_matches()
{
    std::lock_guard< std::mutex > lock( m_mutex_search );

    for( auto&& it = m_matches.begin(); it != m_matches.end(); ++it )
    {
        HiddenFormat* f { *it };
        Paragraph*    p { get_entry_by_id( f->ref_id )->get_paragraph_by_no( f->var_i ) };
        if( p )
            p->remove_format( f );
        else
            print_error( "Match cannot be found in the entry. This is unexpected!" );
    }

    m_matches.clear();
    m_F_stop_search_thread = false;
}

// FILTERS
bool
Diary::dismiss_filter( const Ustring& name )
{
    if( m_filters.size() < 2 )
        return false;

    auto&& it_filter{ m_filters.find( name ) };

    if( it_filter == m_filters.end() )
        return false;

    Filter* filter_to_delete{ it_filter->second };

    if( m_p2filter_list && filter_to_delete->get_id() == m_p2filter_list->get_id() )
    {
        Filter* filter_active_new{ nullptr };

        for( auto& kv_flt : m_filters )
            if( kv_flt.second->can_filter_class( FOC::ENTRIES ) &&
                kv_flt.second != filter_to_delete )
            {
                filter_active_new = kv_flt.second;
                break;
            }

        if( !filter_active_new )
        {
            print_info( "Last entry filter cannot be dismissed." );
            return false;
        }
        else
            m_p2filter_list = filter_active_new;
    }

    m_filters.erase( name );

    delete filter_to_delete;

    return true;
}

void
Diary::remove_entry_from_filters( Entry* entry )
{
    const Ustring     rep_str    { STR::compose( "Fr", DEID_UNSET ) };

    for( auto& kv_filter : m_filters )
    {
        Filter*       filter     { kv_filter.second };
        Ustring       definition { filter->get_definition() };
        const Ustring ref_str    { STR::compose( "Fr", entry->get_id() ) };

        while( true )
        {
            auto pos{ definition.find( ref_str ) };
            if( pos != Ustring::npos )
                definition.replace( pos, ref_str.size(), rep_str );
            else
                break;
        }

        filter->set_definition( definition );
    }
}

bool
Diary::update_entry_filter_status( Entry* entry )
{
    auto       fc         { get_filter_list_stack() };
    const bool old_status { entry->is_filtered_out() };
    const bool new_status { fc ? !fc->filter( entry ) : false };

    entry->set_filtered_out( new_status );

    if( fc )
        delete fc;

    return ( old_status != new_status );
}
int
Diary::update_all_entries_filter_status()
{
    auto f     { get_filter_list() };
    auto fc    { f ? f->get_filterer_stack() : nullptr };
    int  count { 0 };

    for( Entry* entry = m_p2entry_1st; entry; entry = entry->get_next_straight() )
    {
        const bool F_filtered_in { fc ? fc->filter( entry ) : true };
        entry->set_filtered_out( ! F_filtered_in );
        if( F_filtered_in ) count++;
    }

    if( fc )
        delete fc;

    return count;
}

void
Diary::update_filter_user_counts()
{
    for( auto& kv_filter : m_filters )
        kv_filter.second->m_num_users = 0;

    // OTHER FILTERS
    for( const auto& kv_filter : m_filters )
    {
        const String&   def    { kv_filter.second->get_definition() };
        String          line;
        StringSize      line_o { 0 };

        while( STR::get_line( def, line_o, line ) )
        {
            if( STR::begins_with( line, "Fl" ) )
            {
                Filter* p2f{ get_filter( std::stoul( line.substr( 2 ) ) ) };

                if( !p2f )
                    print_info( "Dangling reference to Filter: ", line );
                else
                    p2f->m_num_users++;
            }
        }
    }
    // TODO: 3.2: if filter referring to the other filter is itself unused we can detect it...
    // ...and reset the user count here

    // CHARTS (not now)
    // for( const auto& kv_chart : m_charts )
    // {

    // }

    // TABLES
    for( const auto& kv_table : m_tables )
    {
        const String&   def    { kv_table.second->get_definition() };
        String          line;
        StringSize      line_o { 0 };

        while( STR::get_line( def, line_o, line ) )
        {
            Filter* p2f{ ( Filter* ) 0x1 };

            if     ( STR::begins_with( line, "Mcgf" ) )
                p2f = get_filter( std::stoul( line.substr( 4 ) ) );
            else if( STR::begins_with( line, "Mccf" ) )
                p2f = get_filter( std::stoul( line.substr( 4 ) ) );
            else if( STR::begins_with( line, "Mf" ) )
                p2f = get_filter( std::stoul( line.substr( 2 ) ) );
            else if( STR::begins_with( line, "Ml" ) )
                p2f = get_filter( std::stoul( line.substr( 2 ) ) );

            if( !p2f )
                print_info( "Dangling reference to Filter: ", line );
            else
            if( p2f != ( Filter* ) 0x1 )
                p2f->m_num_users++;
        }
    }

    if( m_p2filter_search ) const_cast< Filter* >( m_p2filter_search )->m_num_users++;
    if( m_p2filter_list ) const_cast< Filter* >( m_p2filter_list )->m_num_users++;
    // an alternative way of dropping constness:
    // get_filter( m_p2filter_active->get_id() )->m_num_users++;
}

// CHARTS

// TABLES

// THEMES
Theme*
Diary::create_theme( const Ustring& name0,
                     const Ustring& font,
                     const std::string& base, const std::string& text,
                     const std::string& head, const std::string& subh,
                     const std::string& hlgt )
{
    Ustring name = create_unique_name_for_map( m_themes, name0 );
    Theme* theme = new Theme( this, name, font, base, text, head, subh, hlgt );
    m_themes.emplace( name, theme );

    return theme;
}

void
Diary::clear_themes()
{
    for( auto iter = m_themes.begin(); iter != m_themes.end(); ++iter )
        delete iter->second;

    m_themes.clear();
}

void
Diary::dismiss_theme( Theme* theme )
{
    if( m_themes.size() == 1 ) throw LIFEO::Error( "Trying to delete the last theme" );

    for( auto& kv_entry : m_entries )
    {
        if( kv_entry.second->get_theme() == theme )
            kv_entry.second->set_theme( nullptr );
    }

    // remove from associated filters
    for( auto& kv_filter : m_filters )
    {
        Ustring def{ kv_filter.second->get_definition() };
        auto    pos{ def.find( "\nFh" + theme->get_name() ) };
        if( pos != Ustring::npos )
        {
            def.erase( pos, theme->get_name().length() + 3 );
            kv_filter.second->set_definition( def );
        }
    }

    m_themes.erase( theme->get_name() );
    delete theme;
}

/*
void
Diary::dismiss_chapter( Chapter* chapter, bool flag_dismiss_contained )
{
    auto&& iter{ m_p2chapter_ctg_cur->find( chapter->get_date() ) };
    if( iter == m_p2chapter_ctg_cur->end() )
    {
        print_error( "Chapter could not be found in assumed category" );
        return;
    }
    else if( ( ++iter ) != m_p2chapter_ctg_cur->end() )  // fix time span
    {
        Chapter* chapter_earlier( iter->second );
        if( chapter->get_time_span() > 0 )
            chapter_earlier->set_time_span(
                    chapter_earlier->get_time_span() + chapter->get_time_span() );
        else
            chapter_earlier->set_time_span( 0 );
    }

    if( flag_dismiss_contained )
    {
        for( Entry* entry : *chapter )
            dismiss_entry( entry, false );
    }

    m_p2chapter_ctg_cur->erase( chapter->get_date() );

    delete chapter;
    update_entries_in_chapters();
}

int
Diary::get_chapter_count() const
{
    int count{ 0 };

    for( auto& kv_ctg : m_chapter_categories )
        count += kv_ctg.second->size();

    return count;
}

void
Diary::update_entries_in_chapters()
{
    PRINT_DEBUG( "Diary::update_entries_in_chapters()" );

    Chapter*   chapter;
    auto&&     itr_entry{ m_entries.begin() };
    Entry*     entry{ itr_entry != m_entries.end() ? itr_entry->second : nullptr };

    while( entry && entry->get_date().is_ordinal() )
    {
        entry = ( ++itr_entry != m_entries.end() ? itr_entry->second : nullptr );
    }

    for( auto& kv_chapter : *m_p2chapter_ctg_cur )
    {
        chapter = kv_chapter.second;
        chapter->clear();

        while( entry && entry->get_date() > chapter->get_date() )
        {
            chapter->insert( entry );
            entry = ( ++itr_entry != m_entries.end() ? itr_entry->second : nullptr );
        }
    }
}

void
Diary::add_entry_to_related_chapter( Entry* entry )
{
    // NOTE: works as per the current listing options needs to be updated when something
    // changes the arrangement such as a change in the current chapter category

    if( not( entry->get_date().is_ordinal() ) )
    {
        for( auto& kv_chapter : *m_p2chapter_ctg_cur )
        {
            Chapter* chapter{ kv_chapter.second };

            if( entry->get_date() > chapter->get_date() )
            {
                chapter->insert( entry );
                return;
            }
        }
    }
}

void
Diary::remove_entry_from_chapters( Entry* entry )
{
    for( auto& kv_chapter : *m_p2chapter_ctg_cur )
    {
        Chapter* chapter( kv_chapter.second );

        if( chapter->find( entry ) != chapter->end() )
        {
            chapter->erase( entry );
            return;
        }
    }
}
*/

// WORK DAYS
int
Diary::calculate_work_days_between( const DateV d1, const DateV d2 )
{
    // NOTE: also check Date::calculate_days_between
    unsigned int    dist        { 0 };
    DateV           date_former { Date::isolate_YMD( d1 < d2 ? d1 : d2 ) };
    const DateV     date_latter { Date::isolate_YMD( d1 < d2 ? d2 : d1 ) };

    while( date_former != date_latter )
    {
        if( is_day_workday( date_former ) ) dist++;

        Date::forward_days( date_former, 1 );
    }

    return dist;
}

// IMPORTING
void
Diary::synchronize_options( const Diary* diary_r )
{
    m_language = diary_r->m_language;
    m_options = diary_r->m_options;

    if( diary_r->m_p2filter_list )
    {
        auto p2filter { get_element2< Filter >( diary_r->m_p2filter_list->get_id() ) };
        if( p2filter )
            m_p2filter_list = p2filter;
    }
    else
        m_p2filter_list = nullptr;

    if( diary_r->m_p2filter_search )
    {
        auto p2filter { get_element2< Filter >( diary_r->m_p2filter_search->get_id() ) };
        if( p2filter )
            m_p2filter_search = p2filter;
    }
    else
        m_p2filter_search = nullptr;

    if( diary_r->m_startup_entry_id < DEID_MIN || get_entry_by_id( diary_r->m_startup_entry_id ) )
    {
        m_startup_entry_id = diary_r->m_startup_entry_id;
    }

    if( diary_r->m_completion_tag_id < DEID_MIN || get_entry_by_id( diary_r->m_completion_tag_id ) )
    {
        m_completion_tag_id = diary_r->m_completion_tag_id;
    }
}

bool
Diary::import_entry( const Entry* entry_r, Entry* tag_for_imported, bool F_add )
{
    if( F_add && !get_element( entry_r->get_id() ) ) // if id is available
        m_force_id = entry_r->get_id();

    Entry* entry_l { F_add ? create_entry( nullptr, false, entry_r->m_date, "" )
                           : get_entry_by_id( entry_r->get_id() ) };

    if( !entry_l ) return false;


    if( !F_add )
        entry_l->clear_text();

    // copy the text:
    Paragraph* p_new { nullptr };
    for( auto p = entry_r->m_p2para_1st; p; p = p->m_p2next )
    {
        p_new = entry_l->add_paragraph_after( new Paragraph( p, this ), p_new );
    }

    // preserve the theme:
    if( entry_r->is_theme_set() )
    {
        auto theme_l { get_element2< Theme >( entry_r->get_theme()->get_id() ) };
        if( theme_l )
            entry_l->set_theme( theme_l);
    }

    if( tag_for_imported )
        entry_l->add_tag( tag_for_imported );

    // copy other data:
    entry_l->m_date_created = entry_r->m_date_created;
    entry_l->m_date_edited = entry_r->m_date_edited;
    // entry_l->m_date_finish = entry_r->m_date_finish; can be calculated
    entry_l->m_language = entry_r->m_language;
    entry_l->m_status = entry_r->m_status;
    entry_l->m_style = entry_r->m_style;
    entry_l->m_unit = entry_r->m_unit;
    entry_l->m_color = entry_r->m_color;

    entry_l->m_F_map_path_old = true; // force update

    // update per the filtering configuration of the new host diary:
    update_entry_filter_status( entry_l );

    return true;
}

bool
Diary::import_theme( const Theme* theme_r, bool F_add )
{
    if( F_add && !get_element( theme_r->get_id() ) )
        m_force_id = theme_r->get_id();

    Theme* theme_l{ F_add ?
            create_theme( create_unique_name_for_map( m_themes, theme_r->get_name() ) ) :
            dynamic_cast< Theme* >( get_element( theme_r->get_id() ) ) };

    if( theme_l == nullptr )
        return false;

    if( !F_add )
        rename_theme( theme_l, theme_r->get_name() );

    theme_r->copy_to( theme_l );

    return true;
}

const DiaryElement*
Diary::get_corresponding_elem( const DiaryElement* elem_r ) const
{
    DiaryElement* elem_l{ get_element( elem_r->get_id() ) };
    if( elem_l && elem_r->get_type() == elem_l->get_type() )
        return elem_l;
    else
        return nullptr;
}

SI
Diary::compare_foreign_elem( const DiaryElement* elem_r,
                             const DiaryElement*& elem_l ) const
{
    elem_l = get_corresponding_elem( elem_r );

    if( elem_l == nullptr )
        return SI::NEW;
    else if( elem_r->get_as_skvvec() == elem_l->get_as_skvvec() )
        return SI::INTACT;

    //else
    return SI::CHANGED;
}
