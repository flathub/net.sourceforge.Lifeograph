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


#include "paragraph.hpp"
#include "diary.hpp"
#include "../parsers/parser_stripper.hpp"
#include "../parsers/parser_background.hpp"
#include "src/diaryelements/diarydata.hpp"


using namespace LIFEO;


// PARAGRAPH =======================================================================================
bool
FuncCmpParagraphs::operator()( const Paragraph* l, const Paragraph* r ) const
{
    return( l->m_host == r->m_host ? l->get_para_no() > r->get_para_no()
                                   : l->m_host->get_date() > r->m_host->get_date() );
}


Paragraph::Paragraph( Entry* host, const Ustring& text, ParserBackGround* parser )
:   DiaryElemDataSrc( host ? host->get_diary() : nullptr ),
    m_host( host ), m_text( text )
{
    if( parser ) parser->parse( this );
}

Paragraph::Paragraph( Paragraph* p, Diary* p2diary )
:   DiaryElemDataSrc( p2diary ? p2diary : p->m_p2diary, p->m_status ),
    m_style( p->m_style ), m_host( p->m_host ),
    m_text( p->m_text ), m_uri( p->m_uri ), m_image_size( p->m_image_size )
{
    for( auto& format : p->m_formats )
        m_formats.insert( new HiddenFormat( *format ) );
}

Paragraph*
Paragraph::get_prev_visible() const
{
    Paragraph* prev = m_p2prev;

    while( prev && !prev->is_visible() )
    {
        prev = prev->m_p2prev;
    }

    return prev;
}
Paragraph*
Paragraph::get_prev_sibling() const
{
    for( auto prev = m_p2prev; prev; prev = prev->m_p2prev )
    {
        if( prev->get_indent_level() < get_indent_level() )
            break;
        if( prev->get_indent_level() == get_indent_level() )
            return prev;
    }

    return nullptr;
}
Paragraph*
Paragraph::get_next_sibling() const
{
    for( auto next = m_p2next; next; next = next->m_p2next )
    {
        if( next->get_indent_level() < get_indent_level() )
            break;
        if( next->get_indent_level() == get_indent_level() )
            return next;
    }

    return nullptr;
}

Paragraph*
Paragraph::get_nth_next( int n )
{
    Paragraph* p { this };

    for( int i = 0; i < n && p; ++i ) p = p->m_p2next;

    return p;
}

Paragraph*
Paragraph::get_next_visible() const
{
    Paragraph* next = m_p2next;

    while( next && !next->is_visible() )
    {
        next = next->m_p2next;
    }

    return next;
}

Paragraph*
Paragraph::get_parent() const
{
    if( get_heading_level() > VT::PS_SUBHDR ) // nothing can parent a subheading
    {
        for( Paragraph* prev = m_p2prev; prev; prev = prev->m_p2prev )
        {
            if( prev->is_header() ) // entry title is not a parent
                break;

            if( prev->is_parent_of( this ) )
                return prev;
        }
    }

    return nullptr;
}

Paragraph*
Paragraph::get_last() const
{
    Paragraph* p_last{ nullptr };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
    {
        if( p )
            p_last = p;
        else
            break;
    }

    return p_last;
}

Paragraph*
Paragraph::get_sub_last() const
{
    Paragraph* p_sub{ const_cast< Paragraph* >( this ) };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
    {
        if( this->is_parent_of( p ) )
            p_sub = p;
        else
            break;
    }

    return p_sub;
}

Paragraph*
Paragraph::get_sub_last_visible( bool F_force_1st_gen ) const
// Caution: returns itself in the absence of any visible sub
{
    Paragraph* p_sub{ const_cast< Paragraph* >( this ) };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
    {
        if( this->is_parent_of( p ) )
        {
            if( p->is_visible() && ( !F_force_1st_gen || p->get_parent() == this ) )
                p_sub = p;
            else
                continue;
        }
        else
            break;
    }

    return p_sub;
}
Paragraph*
Paragraph::get_sub_last_invisible() const
// Caution: returns itself in the absence of any visible sub
{
    Paragraph* p_sub{ const_cast< Paragraph* >( this ) };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
    {
        if( is_parent_of( p ) )
        {
            if( !p->is_visible() )
                p_sub = p;
            else
                continue;
        }
        else
            break;
    }

    return p_sub;
}

bool
Paragraph::is_last_in_host() const
{
    return( m_host ? ( this == m_host->get_paragraph_last() ) : false );
}

int
Paragraph::get_bgn_offset_in_host() const
{
    int offset{ 0 };
    for( Paragraph* para = m_p2prev; para; para = para->m_p2prev )
    {
        if( para->is_visible() )
            offset += para->get_size() + 1; // +1 is for the /n
    }

    return offset;
}
int
Paragraph::get_end_offset_in_host() const
{
    int offset{ get_size() };
    for( const Paragraph* para = m_p2prev; para; para = para->m_p2prev )
    {
        if( para->is_visible() )
            offset += para->get_size() + 1; // +1 is for the /n
    }

    return offset;
}

int
Paragraph::get_list_order() const
{
    int         order      { 0 };
    const int   indent_lvl { this->get_indent_level() };
    const int   headng_lvl { this->get_heading_level() };

    for( const Paragraph* para = this; para; para = para->m_p2prev )
    {
        const int indent_lvl_cur{ para->get_indent_level() };
        const int headng_lvl_cur{ para->get_heading_level() };
        if( indent_lvl_cur < indent_lvl || headng_lvl_cur < headng_lvl )
            break;
        else
        if( indent_lvl_cur == indent_lvl && headng_lvl_cur == headng_lvl &&
            para->get_para_type() == get_para_type() )
            ++order;
    }

    return order;
}
String
Paragraph::get_list_order_str( char separator, bool F_list_only ) const
{
    switch( get_list_type() )
    {
        default:
            if( F_list_only )
                return "";
            //else no break:
        case VT::PS_NUMBER:
            return STR::compose( get_list_order(), separator );
        case VT::PS_CLTTR:
            return STR::compose( char( 'A' + ( ( get_list_order() + 25 ) % 26 ) ), separator );
        case VT::PS_SLTTR:
            return STR::compose( char( 'a' + ( ( get_list_order() + 25 ) % 26 ) ), separator );
        case VT::PS_CROMAN:
            return STR::compose( STR::format_number_roman( get_list_order(), false ), separator );
        case VT::PS_SROMAN:
            return STR::compose( STR::format_number_roman( get_list_order(), true ), separator );
    }
}
String
Paragraph::get_list_order_full() const
{
    String order_str{ get_list_order_str( '-', false ) };

    for( const Paragraph* p = get_parent(); p; p = p->get_parent() )
        order_str.insert( 0, p->get_list_order_str( '.', false ) );

    return order_str;
}

Ustring
Paragraph::get_name() const
{
    //return STR::compose( m_host->get_name(), ":", m_order_in_host );
    return get_text(); // TODO: 3.2: find a better way of describing a paragraph
}

int
Paragraph::get_chain_length() const
{
    auto length { m_text.length() };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
        length += ( p->m_text.length() + 1 ); // +1 for the \n

    return length;
}
int
Paragraph::get_chain_para_count() const
{
    auto count { 1 };

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
        ++count;

    return count;
}

Color
Paragraph::get_color() const
{
    return( m_host ? m_host->get_color() : Color( "#FFFFFF" ) );
}

// TEXT
Ustring
Paragraph::get_text_stripped( int flags ) const
{
    ParserStripper parser;
    Ustring        str;

    if( flags & VT::TCT_CMPNT_INDENT )
        for( int i = 0; i < get_indent_level(); ++i ) str += "    ";

    str += parser.parse( this, flags );

    return str;
}

Ustring
Paragraph::get_text_decorated() const
{
    Ustring str;

    for( int i = get_indent_level(); i > 0; --i ) str += '\t';

    switch( get_list_type() )
    {
        case VT::PS_BULLET:   str += "* "; break;
        case VT::PS_NUMBER:
        case VT::PS_CLTTR:
        case VT::PS_SLTTR:
        case VT::PS_CROMAN:
        case VT::PS_SROMAN:   str += ( get_list_order_str() + ' ' ); break;
        case VT::PS_TODO:
        case VT::PS_PROGRS:
        case VT::PS_DONE:
        case VT::PS_CANCLD:   str += get_todo_status_as_text(); break;
    }

    str += m_text;

    return str;
}

Paragraph*
Paragraph::get_sub( UstringSize bgn, UstringSize end ) const
{
    Paragraph* para_sub { new Paragraph( m_host, m_text.substr( bgn, end - bgn ) ) };
    para_sub->m_style = m_style;

    for( auto format : m_formats )
    {
        // format at least intersects with the range:
        if( format->pos_bgn < end && format->pos_end > bgn )
        {
            auto format_new{ new HiddenFormat( *format ) };
            format_new->pos_bgn = format->pos_bgn > bgn ? format->pos_bgn - bgn : 0ul;
            format_new->pos_end = format->pos_end > end ? end - bgn : format->pos_end - bgn;
            para_sub->insert_format( format_new );
        }
    }

    return para_sub;
}

void
Paragraph::set_text( const Ustring& text, ParserBackGround* parser )
{
    m_text = text;
    if( parser ) parser->parse( this );
}

void
Paragraph::append( Paragraph* para, ParserBackGround* parser )
{
    const auto size_prev { m_text.size() };

    m_text += para->m_text;
    for( auto format : para->m_formats )
        add_format( format, size_prev );

    if( parser ) parser->parse( this );
}
void
Paragraph::append( const Ustring& text, ParserBackGround* parser )
{
    m_text += text;
    if( parser ) parser->parse( this );
}
HiddenFormat*
Paragraph::append( const Ustring& text, int type, const String& uri )
{
    const auto pos = m_text.size();
    m_text += text;
    return add_format( type, uri, pos, pos + text.length() );
}

void
Paragraph::insert_text( UstringSize pos, Paragraph* para, ParserBackGround* parser )
{
    m_text.insert( pos, para->m_text );
    update_formats( pos, 0, para->get_size() );
    for( auto format : para->m_formats )
        add_format( format, pos );

    if( parser ) parser->parse( this );
}

void
Paragraph::insert_text( UstringSize pos, const Ustring& text, ParserBackGround* parser )
{
    m_text.insert( pos, text );
    update_formats( pos, 0, text.size() );
    if( parser ) parser->parse( this );
}
std::tuple< UstringSize, UstringSize, UstringSize >
Paragraph::insert_text_with_spaces( UstringSize pos, Ustring text, ParserBackGround* parser,
                                    bool F_space_bgn_punct, bool F_space_end_punct )
{
    Wchar       c         { m_text[ pos ] };
    Wchar       ct        { text[ text.size() - 1 ] };
    UstringSize space_bgn { 0 };
    UstringSize space_end { 0 };

    if( !STR::is_char_space( ct ) && ( F_space_end_punct ? !STR::is_char_space( c )
                                                         : Glib::Unicode::isalnum( c ) ) )
    {
        text += " ";
        space_end = 1;
    }

    if( pos > 0 )
    {
        c = m_text[ pos - 1 ];
        ct = text[ 0 ];
        if( !STR::is_char_space( ct ) && ( F_space_bgn_punct ? !STR::is_char_space( c )
                                                             : Glib::Unicode::isalnum( c ) ) )
        {
            text.insert( 0, " " );
            space_bgn = 1;
        }
    }

    insert_text( pos, text, parser );
    return std::make_tuple( space_bgn, space_end, text.length() );
}
void
Paragraph::erase_text( UstringSize pos, UstringSize size, ParserBackGround* parser )
{
    m_text.erase( pos, size );
    update_formats( pos, size, 0 );
    if( parser ) parser->parse( this );
}
void
Paragraph::replace_text( UstringSize pos, UstringSize size, const Ustring& text,
                         ParserBackGround* parser )
{
    m_text.erase( pos, size );
    m_text.insert( pos, text );
    update_formats( pos, size, text.size() ); // erase + insert
    if( parser ) parser->parse( this );
}

void
Paragraph::change_letter_cases( int pb, int pe, LetterCase lc )
{
    if( pb != 0 )
    {
        if( lc == LetterCase::CASE_SENTENCE )
            pb = STR::find_sentence_start_backwards( m_text, pb );
        else
        if( lc == LetterCase::CASE_TITLE )
            pb = STR::find_word_start_backwards( m_text, pb );
    }

    const auto  size_old  { ( pe == -1 ? m_text.length() : pe ) - pb };
    Ustring     substring { m_text.substr( pb, size_old ) };

    switch( lc )
    {
        case LetterCase::CASE_SENTENCE: substring = STR::sentencecase( substring );  break;
        case LetterCase::CASE_TITLE:    substring = STR::titlecase( substring );     break;
        case LetterCase::CASE_LOWER:    substring = STR::lowercase( substring );     break;
        case LetterCase::CASE_UPPER:    substring = substring.uppercase();           break;
    }

    m_text.erase( pb, size_old );
    m_text.insert( pb, substring );
    update_formats( pb, size_old, substring.size() ); // erase + insert
}

void
Paragraph::inherit_style_from( const Paragraph* para, bool F_heading_level_too )
{
    if( para->m_style & VT::PS_TODO_GEN )
        set_list_type( VT::PS_TODO );
    else if( para->m_style & VT::PS_LIST_GEN )
        set_list_type( para->get_list_type() );
    set_indent_level( para->get_indent_level() );

    if( F_heading_level_too )
        set_heading_level( para->get_heading_level() );
}

void
Paragraph::set_expanded( bool F_expanded )
{
    DiaryElemDataSrc::set_expanded( F_expanded );

    for( Paragraph* p = m_p2next; p; p = p->m_p2next )
    {
        if( is_parent_of( p ) )
            p->reset_visibility();
        else
            break;
    }
}

bool
Paragraph::is_visible_recalculate() const
{
    Paragraph* parent = get_parent();

    if( !parent )
        return true;

    if( parent->is_expanded() )
        return parent->is_visible();
    else
        return false;
}

void
Paragraph::make_accessible()
{
    Paragraph* parent = get_parent();

    set_visible( true );

    if( !parent )
        return;

    if( !parent->is_visible() )
        parent->set_visible( true );

    parent->make_accessible();
}

void
Paragraph::set_tag( DEID id, Value value )
{
    m_tags[ id ] = value;
    m_tags_in_order.push_back( id );
}
void
Paragraph::set_tag( DEID id, Value v_real, Value v_planned )
{
    m_tags_planned[ id ] = v_planned;
    set_tag( id, v_real );
}

bool
Paragraph::has_tag( const Entry* tag ) const
{
    return( m_tags.find( tag->get_id() ) != m_tags.end() );
}

bool
Paragraph::has_tag_planned( const Entry* tag ) const
{
    return( m_tags_planned.find( tag->get_id() ) != m_tags_planned.end() );
}

bool
Paragraph::has_tag_broad( const Entry* tag ) const
{
    if( has_tag( tag ) ) return true;

    VecEntries&&                  descendant_tags{ tag->get_descendants() };
    std::list< const Paragraph* > paragraphs;

    for( const Paragraph* para = this; para; para = para->get_parent() )
        paragraphs.push_back( para );

    for( auto& t : descendant_tags )
        for( auto& para : paragraphs )
            if( para->has_tag( t ) ) return true;

    return false;
}

Value
Paragraph::get_tag_value( const Entry* tag, int& count ) const
{
    if( !tag || !has_tag( tag ) )
        return 0.0;
    else
    {
        count++;
        return m_tags.get_value_for_tag( tag->get_id() );
    }
}

Value
Paragraph::get_tag_value_planned( const Entry* tag, int& count ) const
{
    if( !tag || !has_tag( tag ) )
        return 0.0;
    else if( !has_tag_planned( tag ) )
        // return realized value as planned when planned value is omitted
        return m_tags.get_value_for_tag( tag->get_id() );
    else
    {
        count++;
        return m_tags_planned.get_value_for_tag( tag->get_id() );
    }
}

Value
Paragraph::get_tag_value_remaining( const Entry* tag, int& count ) const
{
    if( !tag || !has_tag_planned( tag ) )
        return 0.0;
    else
    {
        count++;
        return( m_tags_planned.get_value_for_tag( tag->get_id() ) -
                m_tags.get_value_for_tag( tag->get_id() ) );
    }
}

// double
// Paragraph::get_tag_value_completion( const Entry* tag ) const
// {
//     if( !m_host || !tag ) return 0.0;

//     int c{ 0 }; // dummy
//     double vp{ get_tag_value_planned( tag, c ) };

//     if( vp == 0.0 )
//         return 0.0;

//     return( get_tag_value( tag, c ) / vp );
// }

Entry*
Paragraph::get_sub_tag_first( const Entry* parent_tag ) const
{
    for( auto& tag_id : m_tags_in_order )
    {
        auto tag{ m_p2diary ? m_p2diary->get_entry_by_id( tag_id ) : nullptr };

        if( tag && tag->is_descendant_of( parent_tag ) )
            return tag;
    }

    return nullptr;
}
Entry*
Paragraph::get_sub_tag_last( const Entry* parent_tag ) const
{
    for( auto iter = m_tags_in_order.rbegin(); iter != m_tags_in_order.rend(); iter++ )
    {
        auto tag{ m_p2diary ? m_p2diary->get_entry_by_id( *iter ) : nullptr };

        if( tag && tag->is_descendant_of( parent_tag ) )
            return tag;
    }

    return nullptr;
}
Entry*
Paragraph::get_sub_tag_lowest( const Entry* parent ) const
{
    Entry* sub_tag_lowest{ nullptr };

    if( parent && m_p2diary )
    {
        for( auto& tag_id : m_tags_in_order )
        {
            auto sub_tag{ m_p2diary->get_entry_by_id( tag_id ) };
            if( sub_tag && sub_tag->get_parent() == parent )
            {
                if( sub_tag_lowest )
                {
                    if( sub_tag->get_list_order() < sub_tag_lowest->get_list_order() )
                        sub_tag_lowest = sub_tag;
                }
                else
                    sub_tag_lowest = sub_tag;
            }
        }
    }

    return sub_tag_lowest;
}
Entry*
Paragraph::get_sub_tag_highest( const Entry* parent ) const
{
    Entry* sub_tag_highest{ nullptr };

    if( parent && m_p2diary )
    {
        for( auto& tag_id : m_tags_in_order )
        {
            auto sub_tag{ m_p2diary->get_entry_by_id( tag_id ) };
            if( sub_tag && sub_tag->is_descendant_of( parent ) )
            {
                if( sub_tag_highest )
                {
                    if( sub_tag->get_list_order() > sub_tag_highest->get_list_order() )
                        sub_tag_highest = sub_tag;
                }
                else
                    sub_tag_highest = sub_tag;
            }
        }
    }

    return sub_tag_highest;
}
ListEntries
Paragraph::get_sub_tags( const Entry* parent ) const
{
    ListEntries sub_tags;

    if( parent && m_p2diary )
    {
        for( auto& tag_id : m_tags_in_order )
        {
            auto sub_tag{ m_p2diary->get_entry_by_id( tag_id ) };
            if( sub_tag && sub_tag->is_descendant_of( parent ) )
                sub_tags.push_back( sub_tag );
        }
    }

    return sub_tags;
}

double
Paragraph::get_completion() const
{
    if( !m_host ) return 0.0;

    const double wl     { get_workload() };
    const double cmpltd { get_completed() };

    if( wl == 0.0 )
        return( cmpltd > 0.0 ? 1.0 : 0.0 );
    else
        return( cmpltd / wl );
}

double
Paragraph::get_completed() const
{
    if( m_host == nullptr ) return 0.0;

    int c{ 0 }; // dummy
    Entry* tag_comp{ m_p2diary ? m_p2diary->get_completion_tag() : nullptr };

    if( tag_comp == nullptr )
        return 0.0;

    return get_tag_value( tag_comp, c );
}

double
Paragraph::get_workload() const
{
    if( m_host == nullptr ) return 0.0;

    int c{ 0 }; // dummy
    Entry* tag_comp{ m_p2diary ? m_p2diary->get_completion_tag() : nullptr };

    if( tag_comp == nullptr )
        return 0.0;

    return get_tag_value_planned( tag_comp, c );
}

DateV
Paragraph::get_date_broad( bool F_explicit ) const
{
    if( m_date != Date::NOT_SET )
        return m_date;

    Paragraph* pp{ get_parent() };
    if( pp )
    {
        const auto d { pp->get_date_broad( true ) };
        if( Date::is_set( d ) )
            return d;
    }

    if( !F_explicit && m_host )
        return m_host->get_date();

    return Date::NOT_SET;
}
DateV
Paragraph::get_date_finish_broad( bool F_explicit ) const
{
    if( Date::is_set( m_date_finish ) )
        return m_date_finish;

    Paragraph* pp{ get_parent() };
    if( pp )
    {
        const auto d { pp->get_date_finish_broad( true ) };
        if( Date::is_set( d ) )
            return d;
    }

    if( !F_explicit )
    {
        if( Date::is_set( m_date ) )
            return m_date;

        if( m_host )
            return m_host->get_date_finish();
    }

    return Date::NOT_SET;
}

void
Paragraph::set_list_type( int type )
{
    if( ( m_style & VT::PS_TODO_GEN ) && ( type & VT::PS_TODO_GEN ) &&
        m_p2diary && m_p2diary->has_completion_tag() &&
        has_tag( m_p2diary->get_completion_tag() ) )
    {
        const auto v{ get_completion() };
        if( type == VT::PS_CANCLD ||
            ( v == 0.0 && type != VT::PS_TODO ) ||
            ( v == 1.0 && type != VT::PS_DONE ) ||
            ( is_value_in_range_excl( v, 0.0,  1.0 ) && type != VT::PS_PROGRS ) )
            return;
    }

    m_style = ( ( m_style & ~( VT::PS_FLT_LIST ) ) | ( type & VT::PS_FLT_LIST ) );
}

void
Paragraph::set_para_type2( int type )
{
    if( type & VT::PS_LIST_GEN )
    {
        if( type == VT::PS_LIST_GEN )
            clear_list_type();
        else
            set_list_type( type );
    }

    if( type & VT::PS_HEADER_GEN )
    {
        if( type == VT::PS_HEADER_GEN )
            clear_heading_level();
        else
            set_heading_level( type );
    }
}

void
Paragraph::change_ordered_list_type()
{
    const int   lt        { get_list_type() };
    const bool  was_list  { is_list() };
    if( !( lt & VT::PS_ORDERED_GEN ) || lt == VT::PS_SROMAN )
        set_list_type( VT::PS_NUMBER );
    else
        set_list_type( lt + 0x10 );

    if( !was_list && get_indent_level() == 0 && get_heading_level() != VT::PS_SUBHDR )
        set_indent_level( 1 );
}

bool
Paragraph::set_indent_level( int lvl )
{
    if( lvl < 0 || lvl > VT::PS_INDENT_MAX )        return false;
    if( lvl == 0 && ( m_style & VT::PS_QUOTE ) )    return false;

    m_style = ( ( m_style & ~( VT::PS_FLT_INDENT ) ) |
                ( ( lvl << 24 ) & VT::PS_FLT_INDENT ) );

    return true;
}

void
update_para_style_after_after_indentation_change( Paragraph* p )
{
    if( ( p->m_style & VT::PS_LIST_GEN ) && !( p->m_style & VT::PS_TODO_GEN ) )
    {
        Paragraph* ps { p->get_prev_sibling() };
        Paragraph* ns { p->get_next_sibling() };

        if(      ps && ( ps->m_style & VT::PS_LIST_GEN ) )
            p->set_list_type( ps->get_list_type() );
        else if( ns && ( ns->m_style & VT::PS_LIST_GEN ) )
            p->set_list_type( ns->get_list_type() );
    }
}
void
Paragraph::indent()
{
    if( set_indent_level( get_indent_level() + 1 ) )
        update_para_style_after_after_indentation_change( this );
}
void
Paragraph::unindent()
{
    if( set_indent_level( get_indent_level() - 1 ) )
        update_para_style_after_after_indentation_change( this );
}

void
Paragraph::update_formats( StringSize pos, int s_del, int s_ins )
{
    const int        delta      { s_ins - s_del };
    const StringSize pos_end_d  { pos + s_del };
    const StringSize pos_end_i  { pos + s_ins };

    if( delta > 0 ) // insertion
    {
        for( auto format : m_formats )
        {
            if( pos_end_d <= format->pos_bgn ) format->pos_bgn += delta;
            if( pos_end_d < format->pos_end ||
                ( pos_end_d == format->pos_end && s_del > 0 ) ) format->pos_end += delta;
            // "pos_end_d == format->pos_end" makes the last format bleed into newly entered text
            // it is used on replace operations but not on inserts
        }
    }
    else
    if( delta < 0 ) // erasure
    {
        if( s_ins == 0 ) // only do this on erase (but not on replace)
            FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* format )
            {
                if( pos <= format->pos_bgn && pos_end_d >= format->pos_end )
                {
                    delete format;
                    return true;
                }
                return false;
            } );

        for( auto format : m_formats )
        {
            if( pos < format->pos_end && pos_end_i < format->pos_end )
            {
                if( pos_end_d >= format->pos_end ) format->pos_end = pos_end_i;
                else                               format->pos_end += delta;
               // if( pos < format->pos_end )
            }

            if( pos < format->pos_bgn && pos_end_i < format->pos_bgn )
            {
                if( pos_end_d <= format->pos_bgn ) format->pos_bgn += delta;
                else                               format->pos_bgn = pos_end_i;
            }
        }
    }
}

// IMAGE
R2Pixbuf
Paragraph::get_image( int max_w, const Pango::FontDescription& fd )
{
    if( !m_p2diary ) throw LIFEO::Error( "get_image() called on an orphan paragraph" );
    try
    {
        switch( m_style & VT::PS_FLT_IMAGE )
        {
            case VT::PS_IMAGE_FILE:
                return m_p2diary->get_image_file( m_uri,
                                                  max_w * ( m_image_size + 1 ) * 0.25 );
            case VT::PS_IMAGE_CHART:
                return m_p2diary->get_image_chart( m_uri,
                                                   max_w * ( m_image_size + 1 ) * 0.25,
                                                   fd );
            case VT::PS_IMAGE_TABLE:
                return m_p2diary->get_image_table( m_uri,
                                                   max_w,
                                                   fd,
                                                   m_style & VT::PS_IMAGE_EXPND );
            default:
                throw LIFEO::Error( "get_image() called on a non-image paragraph" );
        }
    }
    catch( ... )
    {
        throw LIFEO::Error( "Image not found" );
    }
}

// FORMATS
void
Paragraph::add_to_referred_entry( DEID id )
{
    if( m_p2diary )
    {
        auto entry{ m_p2diary->get_entry_by_id( id ) };

        if( entry )
            entry->add_referring_elem( this );
    }
}

void
Paragraph::remove_from_referred_entry( DEID id )
{
    if( m_p2diary )
    {
        auto entry{ m_p2diary->get_entry_by_id( id ) };

        if( entry )
            entry->remove_referring_elem( this );
    }
}
void
Paragraph::remove_from_referred_entries()
{
    if( m_p2diary )
    {
        for( auto& f : m_formats )
        {
            if( f->type & VT::HFT_F_REFERENCE )
            {
                auto entry{ m_p2diary->get_entry_by_id( f->ref_id ) };
                if( entry )
                    entry->remove_referring_elem( this );
            }
        }
    }
}

void
Paragraph::clear_formats()
{
    for( auto& format : m_formats )
        delete format;

    m_formats.clear();

    clear_tags();
}

HiddenFormat*
Paragraph::add_format( int type, const String& uri, UstringSize pos_bgn, UstringSize pos_end )
{
    UstringSize p_bgn_final{ pos_bgn }, p_end_final{ pos_end };

    if( type != VT::HFT_MATCH )
    {
        FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* format )
        {
            if( format->type == type &&
                format->pos_bgn <= pos_end && format->pos_end >= ( pos_bgn ) )
            {
                if( format->pos_bgn < p_bgn_final ) p_bgn_final = format->pos_bgn;
                if( format->pos_end > p_end_final ) p_end_final = format->pos_end;
                delete format;
                return true;
            }
            return false;

        } );
    }

    auto format_new{ new HiddenFormat( type, uri, p_bgn_final, p_end_final ) };
    insert_format( format_new );

    return format_new;
}

HiddenFormat*
Paragraph::add_format( HiddenFormat* format, int offset )
{
    auto format_new { new HiddenFormat( *format ) };
    format_new->pos_bgn += offset;
    format_new->pos_end += offset;

    if( format->type != VT::HFT_MATCH )
    {
        FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* f )
        {
            if( f->type == format_new->type &&
                f->pos_bgn <= format_new->pos_end &&
                f->pos_end >= ( format_new->pos_bgn ) )
            {
                if( f->pos_bgn < format_new->pos_bgn ) format_new->pos_bgn = f->pos_bgn;
                if( f->pos_end > format_new->pos_end ) format_new->pos_end = f->pos_end;
                delete f;
                return true;
            }
            return false;
        } );
    }

    insert_format( format_new );

    return format_new;
}

void
Paragraph::insert_format( HiddenFormat* f )
{
    for( auto&& it = m_formats.begin(); it != m_formats.end(); ++it )
    {
        if( ( *it )->pos_bgn > f->pos_bgn )
        {
            m_formats.insert( it, f );
            return;
        }
    }
    m_formats.insert( f );

    // set the ref_id for eval links:
    if( f->type == VT::HFT_LINK_EVAL )
        f->ref_id = m_id;
}

HiddenFormat*
Paragraph::add_format_tag( const Entry* tag, UstringSize pos_bgn )
{
    auto&& format{ add_format( VT::HFT_TAG, "", pos_bgn, pos_bgn + tag->get_name().length() ) };
    format->ref_id = tag->get_id();
    return format;
}

void
Paragraph::remove_format( int type, UstringSize pos_bgn, UstringSize pos_end )
// works both for individual formats & format classes per flags
{
    FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* format )
    {
        if( ( format->type == type ||
              // flag mode:
              ( !( type & VT::HFT_FILTER_CHARS ) && ( format->type & type ) ) ) &&
            format->pos_bgn < pos_end && format->pos_end > pos_bgn )
        {
            // if format extends in both directions, split:
            if( format->pos_bgn < pos_bgn && format->pos_end > pos_end )
            {
                insert_format( new HiddenFormat( type, format->uri, pos_end, format->pos_end ) );
                format->pos_end = pos_bgn;
            }
            // if rormat extends in one direction, trim:
            else if( format->pos_bgn < pos_bgn ) format->pos_end = pos_bgn;
            else if( format->pos_end > pos_end ) format->pos_bgn = pos_end;
            // if contained within the erase region, delete:
            else
            {
                delete format;
                return true;
            }
        }
        return false;
    } );
}

void
Paragraph::remove_format( HiddenFormat* format )
{
    FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* f )
    {
        if( f->type == format->type && f->pos_bgn == format->pos_bgn )
        {
            delete format;
            return true;
        }
        return false;
    } );
}

void
Paragraph::remove_onthefly_formats()
{
    FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* format )
    {
        if( format->type & VT::HFT_F_ONTHEFLY )
        {
            delete format;
            return true;
        }
        return false;
    } );
}

void
Paragraph::remove_formats_of_type( int t )
{
    FormattedText::erase_if( m_formats, [ & ]( HiddenFormat* format )
    {
        if( format->type == t )
        {
            delete format;
            return true;
        }
        return false;
    } );
}

HiddenFormat*
Paragraph::get_format_at( int type, UstringSize pos_bgn, UstringSize pos_end ) const
{
    for( auto& format : m_formats )
    {
        if( format->type == type && format->pos_bgn < pos_end && format->pos_end >= pos_bgn )
            return format;
    }
    return nullptr;
}

HiddenFormat*
Paragraph::get_format_oneof_at( const std::vector< int >& types,
                                UstringSize pos_bgn, UstringSize pos_end ) const
{
    for( auto& type : types )
    {
        for( auto format : m_formats )
            if( format->type == type && format->pos_bgn < pos_end && format->pos_end > pos_bgn )
                return format;
    }
    return nullptr;
}
HiddenFormat*
Paragraph::get_format_oneof_at( int flags, UstringSize pos ) const
{
    for( auto& format : m_formats )
    {
        if( ( format->type & flags ) && format->pos_bgn <= pos && format->pos_end >= pos )
            return format;
    }
    return nullptr;
}

int
Paragraph::predict_list_style_from_text()
{
    int ps{ VT::PS_PLAIN };
    bool F_has_mark{ false };
    for( UstringSize i = 0; i < m_text.length(); i++ )
    {
        const gunichar ch{ m_text[ i ] };
        if( i == 0 && Glib::Unicode::isalpha( ch ) )
            ps = ( Glib::Unicode::isupper( ch ) ? VT::PS_CLTTR : VT::PS_SLTTR );
        else
        if( ( i == 0 || ps == VT::PS_NUMBER ) && !F_has_mark && Glib::Unicode::isdigit( ch ) )
            ps = VT::PS_NUMBER;
        else
        if( i == 0 && ( ch == L'•' || ch == '*' || ch == '-' ) )
        {
            ps = VT::PS_BULLET;
            F_has_mark = true;
        }
        else
        if( !F_has_mark && ps != VT::PS_PLAIN && ( ch == '-' || ch == '.' || ch == ')' ) )
            F_has_mark = true;
        else
        if( F_has_mark && STR::is_char_space( ch ) )
        {
            set_list_type( ps );
            m_text = m_text.substr( ++i );

            for( auto format : m_formats )
            {
                format->pos_bgn -= i;
                format->pos_end -= i;
            }

            return ps;
        }
        else
            break;
    }
    return VT::PS_PLAIN;
}

int
Paragraph::predict_indent_from_text()
{
    if( !m_host ) return 0;

    int indent_level{ 0 };

    // match the indent level from previous paras of the same type:
    for( Paragraph* p = m_p2prev; p && p->m_order_in_host < m_order_in_host; p = p->m_p2prev )
    {
        if( p->get_para_type() == get_para_type() )
        {
            indent_level = p->get_indent_level();
            break;
        }
        // indent if different than the previous list paragraph
        // TODO: 3.1: we may as well need to outdent based on the parents of p
        else
        if( p->is_list() && p->get_indent_level() >= indent_level )
            indent_level = ( p->get_indent_level() + 1 );
    }

    // it is not beautiful the lists to hace 0 indent (questionable):
    if( indent_level == 0 && is_list() )
        indent_level = 1;

    if( indent_level != get_indent_level() )
        set_indent_level( indent_level );

    return indent_level;
}

void
Paragraph::join_with_next()
{
    if( !m_p2next ) return;

    // if this para is not the title paragraph:
    if( m_p2prev )
    {
        if( is_empty() ) // if this para is empty, inherit the next pararaph's style:
        {
            set_heading_level( m_p2next->get_heading_level() );

            if( !( m_style & VT::PS_FLT_LIST ) )
            {
                set_list_type( m_p2next->get_list_type() );
                set_indent_level( m_p2next->get_indent_level() );
            }

            set_alignment( m_p2next->get_alignment() );
            set_hrule( m_p2next->is_hrule() );

            if( m_uri.empty() )
                m_uri     = m_p2next->m_uri;

            m_image_size  = m_p2next->m_image_size;
        }
        else if( m_p2next->is_hrule() )
            set_hrule( true );
    }

    for( auto& format : m_p2next->m_formats )
    {
        auto format_copy = new HiddenFormat( *format );
        format_copy->pos_bgn += m_text.length();
        format_copy->pos_end += m_text.length();
        // not insert_format() as format_copy is guaranteed to come after the last existing format:
        m_formats.insert( format_copy );
    }

    set_expanded( m_p2next->is_expanded() );

    m_text += m_p2next->m_text;

    Paragraph* p_del { m_p2next };

    if( m_p2next->m_p2next )
        m_p2next->m_p2next->m_p2prev = this;
    m_p2next = m_p2next->m_p2next;

    delete p_del;
}
Paragraph*
Paragraph::split_at( UstringSize pos, ParserBackGround* parser )
{
    if( pos >= m_text.length() )
        return( new Paragraph( m_host, "" ) );

    Paragraph* para_new = new Paragraph( m_host, m_text.substr( pos ) );

    for( auto& format : m_formats )
    {
        if( format->pos_end > pos )
        {
            para_new->add_format( format->type,
                                  format->uri,
                                  ( format->pos_bgn > pos ? format->pos_bgn - pos : 0 ),
                                  format->pos_end - pos )->ref_id = format->ref_id;
        }
    }

    erase_text( pos, m_text.length() - pos, parser );

    return para_new;
}
