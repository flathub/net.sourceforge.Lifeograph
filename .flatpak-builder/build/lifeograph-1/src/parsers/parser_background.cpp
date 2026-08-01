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


#include "parser_background.hpp"
#include "../diaryelements/diary.hpp"


using namespace LIFEO;

void
ParserBackGround::reset( UstringSize bgn, UstringSize end )
{
    Paragraph* para_cur = m_parser_p2para_cur;
    ParserBase::reset( bgn, end );
    m_parser_p2para_cur = para_cur; // restore after ParserBase::reset()

    m_parser_p2para_cur->set_todo_status_forced( false );
    m_parser_p2para_cur->clear_tags();
    m_parser_p2para_cur->remove_onthefly_formats();
    m_parser_p2para_cur->remove_from_referred_entries();
    m_parser_p2para_cur->set_date( Date::NOT_SET );
    m_parser_p2para_cur->set_date_finish( Date::NOT_SET );
}

void
ParserBackGround::parse( Paragraph* para )
{
    m_parser_p2para_cur = para;
    m_p2diary = ( para->get_diary() ? para->get_diary() : Diary::d );

    ParserBase::parse( 0, para->get_size() );
}

void
ParserBackGround::process_paragraph()
{
    const auto&& F_todo_para{ bool( m_parser_p2para_cur->get_para_type() & VT::PS_TODO_GEN ) };

    // HIDDEN FORMATS
    for( auto format : m_parser_p2para_cur->m_formats )
    {
        switch( format->type )
        {
            case VT::HFT_TAG:
            {
                auto tag            { m_p2diary->get_entry_by_id( format->ref_id ) };
                auto host_theme     { m_parser_p2para_cur->m_host->get_theme() };
                if( !tag ||
                    tag->get_name() != m_parser_p2para_cur->get_substr( format->pos_bgn,
                                                                        format->pos_end ) )
                {
                    format->uri = host_theme->color_link_broken.to_string();
                    format->ref_id = DEID_UNSET;
                }
                else
                {
                    format->uri = midtone( tag->get_color(),
                                           host_theme->color_text, 0.4 ).to_string();
                    m_parser_p2para_cur->set_tag( tag->get_id(), 1.0 );
                    m_p2format_tag_cur = format;

                    m_parser_p2para_cur->add_to_referred_entry( format->ref_id );
                }
                break;
            }
            case VT::HFT_TAG_VALUE:
            {
            // BEAWARE that the last reference overrides previous ones within a paragraph:
                if( format->var_i >= int( format->pos_bgn ) ) // has planned value
                {
                    const Value v_real{ STR::get_d( get_substr( format->pos_bgn,
                                                                format->var_i ) ) };
                    const Value v_plan{ STR::get_d( get_substr( format->var_i + 1,
                                                                format->pos_end ) ) };
                    m_parser_p2para_cur->set_tag( m_p2format_tag_cur->ref_id, v_real, v_plan );
                }
                else
                {
                    const Value v_real{ STR::get_d( get_substr( format->pos_bgn,
                                                                format->pos_end ) ) };
                    m_parser_p2para_cur->set_tag( m_p2format_tag_cur->ref_id, v_real );
                }

                auto completion_tag { m_p2diary->get_completion_tag() };
                if( completion_tag && completion_tag->get_id() == m_p2format_tag_cur->ref_id )
                {
                    if( F_todo_para )
                    {
                        const auto c{ m_parser_p2para_cur->get_completion() };
                        if( c == 1.0 )
                            m_parser_p2para_cur->set_list_type( VT::PS_DONE );
                        else
                        if( c == 0.0 )
                            m_parser_p2para_cur->set_list_type( VT::PS_TODO );
                        else
                            m_parser_p2para_cur->set_list_type( VT::PS_PROGRS );
                    }

                    m_parser_p2para_cur->set_todo_status_forced( true );
                }
                break;
            }
            case VT::HFT_LINK_ID:
                m_parser_p2para_cur->add_to_referred_entry( format->ref_id );
                break;
            case VT::HFT_DATE:
                // use date only when not in comment:
                if( !m_parser_p2para_cur->get_format_at( VT::HFT_COMMENT, format->pos_bgn ) )
                {
                    if( format->var_i == -1 ) // explicitly end date
                        m_parser_p2para_cur->set_date_finish( format->ref_id );
                    else
                        m_parser_p2para_cur->add_date( format->ref_id );
                }
                break;
        }
    }
}

void
ParserBackGround::apply_comment()
{
    m_parser_p2para_cur->add_format( VT::HFT_COMMENT, "", m_recipe_cur->m_pos_bgn,
                                                          m_parser_pos_cur + 1 );
}

void
ParserBackGround::apply_time()
{
    m_parser_p2para_cur->add_time( m_date_last );
    m_parser_p2para_cur->add_format( VT::HFT_TIME, "", m_recipe_cur->m_pos_bgn,
                                                       m_recipe_cur->m_pos_mid );
    m_parser_p2para_cur->add_format( VT::HFT_TIME_MS, "", m_recipe_cur->m_pos_mid - 3,
                                                          m_recipe_cur->m_pos_mid );
}

void
ParserBackGround::apply_link()
{
    switch( m_recipe_cur->m_id )
    {
        case RID_DATE:
        {
            auto format{ m_parser_p2para_cur->add_format( VT::HFT_DATE,
                                                          "",
                                                          m_recipe_cur->m_pos_bgn,
                                                          m_parser_pos_cur + 1 ) };
            format->ref_id = m_date_last;
            if( m_recipe_cur->m_pos_bgn > 2 &&
                get_char_at( m_recipe_cur->m_pos_bgn - 1 ) == '.' &&
                get_char_at( m_recipe_cur->m_pos_bgn - 2 ) == '.' &&
                get_char_at( m_recipe_cur->m_pos_bgn - 3 ) == '.' )
                {
                    format->var_i = -1; // end mark
                    m_parser_p2para_cur->add_format( VT::HFT_DATE_ELLIPSIS,
                                                     "",
                                                     m_recipe_cur->m_pos_bgn - 3,
                                                     m_recipe_cur->m_pos_bgn );
                }
            else
            if( int( m_parser_pos_cur ) < m_parser_p2para_cur->get_size() &&
                get_char_at( m_parser_pos_cur + 1 ) == '.' &&
                get_char_at( m_parser_pos_cur + 2 ) == '.' &&
                get_char_at( m_parser_pos_cur + 3 ) == '.' )
                {
                    m_parser_p2para_cur->add_format( VT::HFT_DATE_ELLIPSIS,
                                                     "",
                                                     m_parser_pos_cur + 1,
                                                     m_parser_pos_cur + 4 );
                }
        break;
        }
        case RID_LINK_AT:
        {
            const auto&& uri{ "mailto:" + m_parser_p2para_cur->get_substr( m_recipe_cur->m_pos_bgn,
                                                                           m_parser_pos_cur ) };
            m_parser_p2para_cur->add_format( VT::HFT_LINK_ONTHEFLY,
                                             uri,
                                             m_recipe_cur->m_pos_bgn,
                                             m_parser_pos_cur );
            break;
        }
        case RID_URI:
        {
            const auto&& uri{ m_parser_p2para_cur->get_substr( m_recipe_cur->m_pos_bgn,
                                                               m_parser_pos_cur ) };
            m_parser_p2para_cur->add_format( VT::HFT_LINK_ONTHEFLY,
                                             uri,
                                             m_recipe_cur->m_pos_bgn,
                                             m_parser_pos_cur );
            break;
        }
    }
}

void
ParserBackGround::apply_inline_tag_value_nmbr()
{
    if( m_p2format_tag_cur )
    {
        m_parser_p2para_cur->add_format( VT::HFT_TAG_VALUE,
                                         "",
                                         m_recipe_cur->m_pos_bgn,
                                         m_parser_pos_last_digit + 1 )->
                var_i = m_parser_pos_extra_1;
    }
}
