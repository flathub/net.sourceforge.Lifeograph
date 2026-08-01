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


#include "../lifeograph.hpp"
#include "../app_window.hpp"
#include "dialog_sync.hpp"
#include "dialog_password.hpp"


using namespace LIFEO;

constexpr char COLOR_IGNORE[]      = "#cccccc";
constexpr char COLOR_IGNORE_FG[]   = "#666666";
constexpr char COLOR_OVERWRITE[]   = "#d0e0bb";
constexpr char COLOR_ADD[]         = "#ccffcc";
constexpr char COLOR_DELETED[]     = "#ffcccc";
constexpr char COLOR_CHANGED[]     = "#ccccff";

DialogSync::DialogSync( BaseObjectType* cobject, const Glib::RefPtr< Gtk::Builder >& builder )
:   Gtk::Window( cobject ), /* m_dialog_password( nullptr ),  */m_diary_r( nullptr )
{
    Gtk::CellRendererPixbuf* CRP_icon{ nullptr };
    Gtk::CellRendererText* CRT_name{ nullptr };
    Gtk::CellRendererCombo* CRC_action{ nullptr };
    Gtk::TreeViewColumn* TVC_name{ nullptr };
    Gtk::Button* B_ignore{ nullptr };

    Gtk::CellRendererText* CRT_comp_l{ nullptr };
    Gtk::CellRendererText* CRT_comp_r{ nullptr };
    Gtk::TreeViewColumn* TVC_comp_l{ nullptr };
    Gtk::TreeViewColumn* TVC_comp_r{ nullptr };

    try
    {
        m_TS_content = TreeStoreImport::create();
        m_B_go              = builder->get_widget< Gtk::Button >( "B_sync_go" );
        m_B_cancel          = builder->get_widget< Gtk::Button >( "B_sync_cancel" );
        m_B_unlock          = builder->get_widget< Gtk::Button >( "B_sync_unlock" );
        m_IB_sync           = builder->get_widget< Gtk::InfoBar >( "IB_sync" );
        m_L_info            = builder->get_widget< Gtk::Label >( "L_sync_info" );
        m_FCB_diary         = Gtk::Builder::get_widget_derived< FileChooserButton >(
                                    builder, "FB_sync_diary" );
        m_WEP_widget        = Gtk::Builder::get_widget_derived< WidgetEntryPicker >(
                                    builder, "E_sync_add_tag" );
        m_RB_changed_ignore = builder->get_widget< Gtk::CheckButton >( "RB_sync_changed_ignore" );
        m_RB_changed_ovrwrt = builder->get_widget< Gtk::CheckButton >( "RB_sync_changed_overwrite" );
        m_RB_changed_add    = builder->get_widget< Gtk::CheckButton >( "RB_sync_changed_add" );
        m_RB_new_ignore     = builder->get_widget< Gtk::CheckButton >( "RB_sync_new_ignore" );
        m_RB_new_add        = builder->get_widget< Gtk::CheckButton >( "RB_sync_new_add" );
        m_B_apply_scheme    = builder->get_widget< Gtk::Button >( "B_sync_apply_scheme" );

        m_TV_contents       = builder->get_widget< Gtk::TreeView >( "TV_sync_content" );
        m_P_contents        = builder->get_widget< Gtk::Paned >( "P_sync" );
        m_Bx_operations     = builder->get_widget< Gtk::Box >( "Bx_sync_operations" );
        m_TV_compare        = builder->get_widget< Gtk::TreeView>( "TV_sync_compare" );
        m_Sw_similar        = builder->get_widget< Gtk::Switch >( "Sw_sync_similar" );
        B_ignore            = builder->get_widget< Gtk::Button >( "B_sync_ignore" );
        m_B_ow              = builder->get_widget< Gtk::Button >( "B_sync_overwrite" );
        m_B_add             = builder->get_widget< Gtk::Button >( "B_sync_add" );

        CRP_icon = Gtk::manage( new Gtk::CellRendererPixbuf );
        CRT_name = Gtk::manage( new Gtk::CellRendererText );
        CRC_action = Gtk::manage( new Gtk::CellRendererCombo );
        TVC_name = Gtk::manage( new Gtk::TreeViewColumn( _( "Name" ) ) );
        m_TVC_status = Gtk::manage( new Gtk::TreeViewColumn( _( "Status" ),
                                                             m_TS_content->colrec.status ) );
        m_TVC_action = Gtk::manage( new Gtk::TreeViewColumn( _( "Action" ) ) );

        TVC_comp_l = Gtk::manage( new Gtk::TreeViewColumn( _( "Local" ) ) );
        TVC_comp_r = Gtk::manage( new Gtk::TreeViewColumn( _( "Remote" ) ) );
        CRT_comp_l = Gtk::manage( new Gtk::CellRendererText );
        CRT_comp_r = Gtk::manage( new Gtk::CellRendererText );
    }
    catch( ... )
    {
        throw LIFEO::Error( "Failed to create sync dialog" );
    }

    // ACTION COMBOBOX
    m_LS_action_ow = Gtk::ListStore::create( m_colrec_combo );
    auto row = *( m_LS_action_ow->append() );
    row[ m_colrec_combo.name ] = STR0/SI::SI_IGNORE;
    row = *( m_LS_action_ow->append() );
    row[ m_colrec_combo.name ] = STR0/SI::OVERWRITE;

    m_LS_action_add = Gtk::ListStore::create( m_colrec_combo );
    row = *( m_LS_action_add->append() );
    row[ m_colrec_combo.name ] = STR0/SI::SI_IGNORE;
    row = *( m_LS_action_add->append() );
    row[ m_colrec_combo.name ] = STR0/SI::ADD;

    m_LS_action_add_ow = Gtk::ListStore::create( m_colrec_combo );
    row = *( m_LS_action_add_ow->append() );
    row[ m_colrec_combo.name ] = STR0/SI::SI_IGNORE;
    row = *( m_LS_action_add_ow->append() );
    row[ m_colrec_combo.name ] = STR0/SI::ADD;
    row = *( m_LS_action_add_ow->append() );
    row[ m_colrec_combo.name ] = STR0/SI::OVERWRITE;

    // TREEVIEW CONTENT
    m_TV_contents->set_model( m_TS_content );

    // not at all sure about the below approach to sizing
    TVC_name->set_sizing( Gtk::TreeViewColumn::Sizing::FIXED );
    m_TVC_status->set_sizing( Gtk::TreeViewColumn::Sizing::FIXED );
    m_TVC_action->set_sizing( Gtk::TreeViewColumn::Sizing::FIXED );
    m_TVC_status->set_fixed_width( 110 );
    m_TVC_action->set_fixed_width( 110 );

    m_TVC_status->set_sort_indicator( true );
    m_TVC_action->set_sort_indicator( true );
    m_TV_contents->append_column( *TVC_name );
    m_TV_contents->append_column( *m_TVC_status );
    m_TV_contents->append_column( *m_TVC_action );

    // NAME COLUMN
    CRT_name->property_ellipsize() = Pango::EllipsizeMode::END;
    TVC_name->pack_start( *CRP_icon, false );
    TVC_name->pack_start( *CRT_name );
    TVC_name->add_attribute( CRP_icon->property_pixbuf(), m_TS_content->colrec.icon );
    TVC_name->add_attribute( CRT_name->property_markup(), m_TS_content->colrec.name );
    TVC_name->set_expand( true );

    // STATUS COLUMN
    m_TVC_status->set_cell_data_func( * m_TV_contents->get_column_cell_renderer( 1 ),
                                      sigc::mem_fun( *this, &DialogSync::set_cell_color_s ) );

    // ACTION COLUMN
    m_TVC_action->pack_start( *CRC_action, false );
    m_TVC_action->add_attribute( CRC_action->property_text(), m_TS_content->colrec.action );
    m_TVC_action->add_attribute( CRC_action->property_model(), m_TS_content->colrec.action_combo );
    CRC_action->property_text_column() = 0;
    CRC_action->property_editable() = true;
    CRC_action->property_has_entry() = false;
    CRC_action->signal_edited().connect(
            sigc::mem_fun( *this, &DialogSync::handle_TV_action_edited ) );
    m_TVC_action->set_cell_data_func( * m_TV_contents->get_column_cell_renderer( 2 ),
                                      sigc::mem_fun( *this, &DialogSync::set_cell_color_a ) );

    // TREEVIEW COMPARE
    CRT_comp_l->property_wrap_mode() = Pango::WrapMode::WORD;
    CRT_comp_r->property_wrap_mode() = Pango::WrapMode::WORD;
    CRT_comp_l->property_wrap_width() = 300;
    CRT_comp_r->property_wrap_width() = 300;
    m_LS_compare = Gtk::ListStore::create( m_CR_compare );
    m_TV_compare->set_model( m_LS_compare );
    m_TV_compare->append_column( _( "Name" ), m_CR_compare.name );
    TVC_comp_l->pack_start( *CRT_comp_l );
    TVC_comp_r->pack_start( *CRT_comp_r );
    TVC_comp_l->add_attribute( CRT_comp_l->property_text(), m_CR_compare.str_l );
    TVC_comp_r->add_attribute( CRT_comp_r->property_text(), m_CR_compare.str_r );
    TVC_comp_l->set_expand( true );
    TVC_comp_r->set_expand( true );
    m_TV_compare->append_column( *TVC_comp_l );
    m_TV_compare->append_column( *TVC_comp_r );
    TVC_comp_l->set_cell_data_func( * m_TV_compare->get_column_cell_renderer( 1 ),
                                      sigc::mem_fun( *this, &DialogSync::set_cell_color_comp_l ) );
    TVC_comp_r->set_cell_data_func( * m_TV_compare->get_column_cell_renderer( 2 ),
                                      sigc::mem_fun( *this, &DialogSync::set_cell_color_comp_r ) );

    // BUTTONS
    auto css_provider{ Gtk::CssProvider::create() };
    css_provider->load_from_data(
            STR::compose(
                    "button.sync-ignore { color: #000000; background: ", COLOR_IGNORE, "; }\n"
                    "button.sync-overwr { color: #000000; background: ", COLOR_OVERWRITE, "; }\n"
                    "button.sync-add { color: #000000; background: ", COLOR_ADD, "; }" ) );
    Gtk::StyleProvider::add_provider_for_display( Gdk::Display::get_default(),
                                                  css_provider,
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

    m_FCB_diary->add_diary_file_filters();
    m_FCB_diary->set_info_text( _( "Click to select a diary file..." ) );

    m_WEP_widget->set_diary( Diary::d );

    // SIGNALS
    m_B_apply_scheme->signal_clicked().connect(
            sigc::mem_fun( *this, &DialogSync::update_maps ) );

    m_TV_contents->get_selection()->signal_changed().connect(
            sigc::mem_fun( *this, &DialogSync::show_details ) );

    m_AG = Gio::SimpleActionGroup::create();
    m_A_show_accepted = m_AG->add_action_bool(
            "show_accepted",
            [ this ](){ toggle_filter( FILTER_SHOW_ACCEPTED, m_A_show_accepted ); },
            true );
    m_A_show_ignored = m_AG->add_action_bool(
            "show_ignored",
            [ this ](){ toggle_filter( FILTER_SHOW_IGNORED, m_A_show_ignored ); },
            true );
    m_A_show_new = m_AG->add_action_bool(
            "show_new",
            [ this ](){ toggle_filter( FILTER_SHOW_NEW, m_A_show_new ); },
            true );
    m_A_show_changed = m_AG->add_action_bool(
            "show_changed",
            [ this ](){ toggle_filter( FILTER_SHOW_CHANGED, m_A_show_changed ); },
            true );
    m_A_only_show_different = m_AG->add_action_bool(
            "only_show_different",
            [ this ](){ toggle_filter( FILTER_ONLY_SHOW_DIFFERENT, m_A_only_show_different ); },
            true );
    insert_action_group( "sync", m_AG );

    B_ignore->signal_clicked().connect(
            sigc::bind( sigc::mem_fun( *this, &DialogSync::change_action ), SI::SI_IGNORE ) );
    m_B_add->signal_clicked().connect(
            sigc::bind( sigc::mem_fun( *this, &DialogSync::change_action ), SI::ADD ) );
    m_B_ow->signal_clicked().connect(
            sigc::bind( sigc::mem_fun( *this, &DialogSync::change_action ), SI::OVERWRITE ) );

    m_FCB_diary->signal_clicked().connect( [&](){ set_not_ready(); } );
    m_FCB_diary->m_signal_file_set.connect(
            sigc::mem_fun( *this, &DialogSync::open_remote_diary ) );
    m_B_unlock->signal_clicked().connect( [&](){ ask_for_password(); } );

    m_B_go->signal_clicked().connect(
            [ this ]()
            {
                go_synchonize();
                AppWindow::p->UI_diary->update_entry_list();
                AppWindow::p->UI_extra->refresh_after_sync();
            } );
    m_B_cancel->signal_clicked().connect( [ this ](){ hide(); } );

    m_IB_sync->signal_response().connect( [ this ]( int ){ m_IB_sync->set_visible( false ); } );
}

void
DialogSync::on_show()
{
    Gtk::Window::on_show();
    set_not_ready();

    m_Bx_operations->set_visible( false );

    m_FCB_diary->set_uri( "" );
    m_B_unlock->hide();
    m_WEP_widget->clear();

    m_FCB_diary->grab_focus();
}

void
DialogSync::open_remote_diary( const String& uri )
{
    if( m_diary_r != nullptr )
        delete m_diary_r;

    m_diary_r = new Diary;

    if( m_diary_r->set_path( uri, Diary::SPT_READ_ONLY ) == SUCCESS )
    {
        if( m_diary_r->read_header() == SUCCESS )
        {
            if( m_diary_r->is_encrypted() )
                // Glib::signal_idle().connect_once(
                //     sigc::mem_fun( *this, &DialogSync::ask_for_password ) );
                // ask_for_password();
                // TODO: 3.2: popover cannot be shown in a Filechooser event any more...
                // ...so we use an intermediary unlock button to tringger the password popover
                m_B_unlock->set_visible();
            else
                open_remote_diary2();
        }
        else
        {
            set_not_ready( _( "Selected file does not seem to be a valid diary" ) );
        }
    }
    else
        set_not_ready( _( "Selected file cannot be read" ) );
}

void
DialogSync::open_remote_diary2()
{
    switch( m_diary_r->read_body() )
    {
        case SUCCESS:
            open_remote_diary3();
            break;
        case WRONG_PASSWORD:
            open_remote_diary( m_diary_r->get_uri() );
            break;
        default:
            set_not_ready( _( "Selected file cannot be read" ) );
            break;
    }
}

void
DialogSync::open_remote_diary3()
{
    DialogPassword::finish( m_diary_r->get_uri() );
    m_B_unlock->hide();
    m_P_contents->set_position( get_height() * 0.55 );
    m_RB_changed_ignore->set_active( true );
    m_RB_new_ignore->set_active( true );

    m_A_show_changed->change_state( true );
    m_A_show_new->change_state( true );
    m_A_show_accepted->change_state( true );
    m_A_show_ignored->change_state( true );
    m_A_only_show_different->change_state( false );
    m_content_filter = FILTER_DEFAULT;

    update_maps();

    if( m_F_identical )
        set_not_ready( "Diaries are identical" );
    else
    {
        set_ready();
        m_L_info->set_text(_( "Please select what to import/sync and press Go" ) );
    }
}

void
DialogSync::ask_for_password()
{
    auto            gphrect { m_B_unlock->compute_bounds( *this ) };
    Gdk::Rectangle  rect    { int( gphrect->get_x() ),      int( gphrect->get_y() ),
                              int( gphrect->get_width() ),  int( gphrect->get_height() ) };
    DialogPassword::launch( DialogPassword::OT_OPEN,
                            m_diary_r, &rect, this,
                            sigc::mem_fun( *this, &DialogSync::open_remote_diary2 ),
                            [ this ]() { m_B_unlock->hide(); m_FCB_diary->set_uri( "" ); } );
}

void
DialogSync::update_maps()
{
    PRINT_DEBUG( "DialogSync::update_maps()" );

    m_comparison_map.clear();
    m_action_map.clear();
    m_F_identical = true;

    for( auto kv_elem : m_diary_r->m_ids )
    {
        if( kv_elem.second->get_type() == DiaryElement::ET_PARAGRAPH ||
            kv_elem.second->get_id() < DEID_MIN ) continue;
        // IDs below DEID_MIN designate standard elements

        const DiaryElement* elem_l;
        SI cmp_res { Diary::d->compare_foreign_elem( kv_elem.second, elem_l ) };
        m_comparison_map[ kv_elem.first ] = cmp_res;

        if( cmp_res == SI::CHANGED )
        {
            if( m_RB_changed_ignore->get_active() )
                m_action_map[ kv_elem.first ] = SI::SI_IGNORE;
            else if( m_RB_changed_ovrwrt->get_active() )
                m_action_map[ kv_elem.first ] = SI::OVERWRITE;
            else //if( m_RB_changed_add->get_active() )
                m_action_map[ kv_elem.first ] = ( elem_l->is_entry() ? SI::ADD : SI::OVERWRITE );

            if( m_F_identical )
                m_F_identical = false;
        }
        else if( cmp_res == SI::NEW )
        {
            if( m_RB_new_ignore->get_active() )
                m_action_map[ kv_elem.first ] = SI::SI_IGNORE;
            // m_RB_new_add is active:
            else if( elem_l->get_type() == DiaryElement::ET_DIARY )
                m_action_map[ kv_elem.first ] = SI::OVERWRITE;
            else
                m_action_map[ kv_elem.first ] = SI::ADD;

            if( m_F_identical )
                m_F_identical = false;
        }
        else
            m_action_map[ kv_elem.first ] = SI::SI_IGNORE;
    }

    populate_content();
}

void
DialogSync::show_details()
{
    if( !is_ready() )
        return;

    m_LS_compare->clear();
    m_Bx_operations->set_visible( false );
    m_B_ow->set_visible( false );
    m_B_add->set_visible( false );

    auto&&              iter    { m_TV_contents->get_selection()->get_selected() };

    if( !iter )
        return;

    Gtk::TreeRow        row     { *iter };
    const DiaryElement* elem_r  { row[ m_TS_content->colrec.ptr ] };
    const SI            si      { row[ m_TS_content->colrec.comp_res ] };

    if( elem_r == nullptr || si == SI::INTACT )
        return;

    m_Bx_operations->set_visible( true );

    switch( elem_r->get_type() )
    {
        case DiaryElement::ET_DIARY:
            m_B_add->set_visible( false );
            m_B_ow->set_visible( si != SI::INTACT );
            break;
        case DiaryElement::ET_ENTRY:
            m_B_add->set_visible( true );
            m_B_ow->set_visible( si == SI::CHANGED );
            break;
        default:
            m_B_add->set_visible( si == SI::NEW );
            m_B_ow->set_visible( si == SI::CHANGED );
            break;
    }

    const DiaryElement* elem_l { Diary::d->get_corresponding_elem( elem_r ) };

    auto add_item = [ & ]( const SI id, const Ustring& str_l, const Ustring& str_r, SI result )
    {
        row = * ( m_LS_compare->append() );

        row[ m_CR_compare.name ] = STR0/id;
        row[ m_CR_compare.str_l ] = str_l;
        row[ m_CR_compare.str_r ] = str_r;
        row[ m_CR_compare.result ] = result;
    };

    const SKVVec&&  props_l { elem_l ? elem_l->get_as_skvvec() : SKVVec() };
    const SKVVec&&  props_r { elem_r->get_as_skvvec() };
    unsigned        i_l { 0 }, i_r { 0 };

    while( i_l < props_l.size() || i_r < props_r.size() )
    {
        // PAST THE END OF THE LOCAL ITEMS - ADD AS A DELETED ITEM
        if( i_l >= props_l.size() )
        {
            add_item( props_r[ i_r ].first, "", props_r[ i_r ].second, SI::DELETED );
            i_r++;
        }
        // PAST THE END OF THE REMOTE ITEMS - ADD AS A NEW ITEM
        else
        if( i_r >= props_r.size() )
        {
            add_item( props_l[ i_l ].first, props_l[ i_l ].second, "", SI::NEW );
            i_l++;
        }
        // DIFFERENT TYPE OF ITEM - NEW OR DELETED ITEM DEPENDING ON THE COMPARSION OF ORDERS
        else
        if( props_l[ i_l ].first != props_r[ i_r ].first )
        {
            if( props_l[ i_l ].first < props_r[ i_r ].first )
            {
                add_item( props_l[ i_l ].first, props_l[ i_l ].second, "", SI::NEW );
                i_l++;
            }
            else
            {
                add_item( props_r[ i_r ].first, "", props_r[ i_r ].second, SI::DELETED );
                i_r++;
            }
        }
        // DIFFERENT ITEM VALUES
        else
        if( props_l[ i_l ].second != props_r[ i_r ].second )
        {
            int j_l{ -1 }, j_r{ -1 };
            // LOOK DOWN IN LOCAL ITEMS TO SEE IF THERE IS A MATCH.
            // if so, that will mean new item in local
            for( int i = 1; i_l + i < props_l.size(); i++ )
                if( props_l[ i_l + i ].first == props_r[ i_r ].first &&
                    props_l[ i_l + i ].second == props_r[ i_r ].second )
                {
                    j_l = i;
                    break;
                }
            // LOOK DOWN IN REMOTE ITEMS TO SEE IF THERE IS A MATCH.
            // if so, that will mean deleted item in local
            for( int i = 1; i_r + i < props_r.size(); i++ )
                if( props_l[ i_l ].first == props_r[ i_r + i ].first &&
                    props_l[ i_l ].second == props_r[ i_r + i ].second )
                {
                    j_r = i;
                    break;
                }

            if( j_l > 0 && ( j_r < 0 || j_l <= j_r ) )
            {
                add_item( props_l[ i_l ].first, props_l[ i_l ].second, "", SI::NEW );
                i_l++;
            }
            else
            if( j_r > 0 && ( j_l < 0 || j_r <= j_l ) )
            {
                add_item( props_r[ i_r ].first, "", props_r[ i_r ].second, SI::DELETED );
                i_r++;
            }
            else
            {
                add_item( props_l[ i_l ].first, props_l[ i_l ].second,
                          props_r[ i_r ].second, SI::CHANGED );
                i_l++;
                i_r++;
            }
        }
        else
        {
            if( !( m_content_filter & FILTER_ONLY_SHOW_DIFFERENT ) &&
                !props_l[ i_l ].second.empty() )
                add_item( props_l[ i_l ].first, props_l[ i_l ].second,
                          props_r[ i_r ].second, SI::INTACT );
            i_l++;
            i_r++;
        }
    }
}

void
DialogSync::handle_TV_action_edited( const Ustring& path_str, const Ustring& new_text )
{
    Gtk::TreePath path( path_str );
    auto iter = m_TS_content->get_iter( path );
    if( iter )
    {
        auto row = *iter;
        DiaryElement* elem{ row[ m_TS_content->colrec.ptr ] };
        SI action{ new_text == STR0/SI::ADD ? SI::ADD :
                ( new_text == STR0/SI::SI_IGNORE ? SI::SI_IGNORE : SI::OVERWRITE ) };
        if( elem )
        {
            m_action_map[ elem->get_id() ] = action;

            if( m_Sw_similar->get_active() )
            {
                for( auto& kv : m_action_map )
                {
                    DiaryElement* elem_i{ m_diary_r->get_element( kv.first ) };
                    if( elem_i->get_type() == elem->get_type() &&
                        m_comparison_map[ kv.first ] == m_comparison_map[ elem->get_id() ] )
                        m_action_map[ kv.first ] = action;
                }
            }
        }

        populate_content();
    }
}

void
DialogSync::toggle_filter( int changed_filter, R2Action& action )
{
    if( !m_diary_r )
        return;

    if( m_content_filter & changed_filter )
    {
        m_content_filter -= changed_filter;
        action->change_state( false );
    }
    else
    {
        m_content_filter |= changed_filter;
        action->change_state( true );
    }

    if( changed_filter == FILTER_ONLY_SHOW_DIFFERENT )
        show_details();
    else
        populate_content();
}

void
DialogSync::change_action( SI si )
{
    auto treesel{ m_TV_contents->get_selection() };
    if( treesel->count_selected_rows() < 1 )
        return;

    handle_TV_action_edited( m_TS_content->get_path( treesel->get_selected() ).to_string(),
                             STR0/si );
}

void
DialogSync::populate_content()
{
    auto add_header = [ & ]( int header_type )
    {
        Gtk::TreeRow row{ * m_TS_content->append() };
        row[ m_TS_content->colrec.ptr ] = nullptr;
        //row[ m_TS_content->colrec.icon ] = icon;
        row[ m_TS_content->colrec.header_type ] = header_type;
        row[ m_TS_content->colrec.comp_res ] = SI::_VOID_;
        return row;
    };

    auto update_header = [ & ]( Gtk::TreeRow& row, const Ustring& name, const int size )
    {
        if( row.children().empty() )
            m_TS_content->erase( row.get_iter() );
        else
            row[ m_TS_content->colrec.name ] = Glib::ustring::compose( "<b>%1</b> (%2/%3)",
                                                                       name,
                                                                       row.children().size(),
                                                                       size );
    };

    auto get_action_combo = [ & ]( DiaryElement* elem, SI si )
    {
        switch( elem->get_type() )
        {
            case DiaryElement::ET_DIARY:
                return( m_LS_action_ow );
            case DiaryElement::ET_ENTRY:
                return( si == SI::NEW ? m_LS_action_add : m_LS_action_add_ow );
            default:
                return( si == SI::NEW ? m_LS_action_add : m_LS_action_ow );
        }
    };

    auto add_elem = [ & ]( DiaryElement* elem_r, Gtk::TreeRow& parent )
    {
        SI cmp_res{ m_comparison_map[ elem_r->get_id() ] };
        SI action{ m_action_map[ elem_r->get_id() ] };

        if( cmp_res == SI::INTACT ||
            ( cmp_res == SI::NEW && !( m_content_filter & FILTER_SHOW_NEW ) ) ||
            ( cmp_res == SI::CHANGED && !( m_content_filter & FILTER_SHOW_CHANGED ) ) ||
            ( action == SI::SI_IGNORE && !( m_content_filter & FILTER_SHOW_IGNORED ) ) ||
            ( action != SI::SI_IGNORE && !( m_content_filter & FILTER_SHOW_ACCEPTED ) ) )
            return;

        Gtk::TreeRow row = *( m_TS_content->append( parent.children() ) );

        row[ m_TS_content->colrec.name ] = elem_r->get_type() == DiaryElement::ET_DIARY ?
                                                    _( "Options" ) : elem_r->get_list_str();
        row[ m_TS_content->colrec.ptr ] = elem_r;
        row[ m_TS_content->colrec.icon ] = elem_r->get_icon();
        row[ m_TS_content->colrec.status ] = STR0/cmp_res;
        row[ m_TS_content->colrec.action ] = STR0/action;
        row[ m_TS_content->colrec.action_combo ] = get_action_combo( elem_r, cmp_res );
        row[ m_TS_content->colrec.header_type ] = 0; //TreeStoreImport::HT_NONE;
        row[ m_TS_content->colrec.comp_res ] = cmp_res;
    };

    Gtk::TreePath path_top, path_btm;
    m_TV_contents->get_visible_range( path_top, path_btm );

    m_TV_contents->unset_model();
    m_TS_content->clear();

    // ELEMENTS
    auto&& row_header_diary{ add_header( TreeStoreImport::HT_DIARY ) };
    auto&& row_header_entries{ add_header( TreeStoreImport::HT_ENTRIES ) };
    auto&& row_header_themes{ add_header( TreeStoreImport::HT_THEMES ) };
    auto&& row_header_filters{ add_header( TreeStoreImport::HT_FILTERS ) };
    auto&& row_header_charts{ add_header( TreeStoreImport::HT_CHARTS ) };
    auto&& row_header_tables{ add_header( TreeStoreImport::HT_TABLES ) };
    for( auto& kv_elem : m_action_map )
    {
        DiaryElement* elem{ m_diary_r->get_element( kv_elem.first ) };
        switch( elem->get_type() )
        {
            case DiaryElement::ET_ENTRY:
                add_elem( elem, row_header_entries );
                break;
            case DiaryElement::ET_THEME:
                add_elem( elem, row_header_themes );
                break;
            case DiaryElement::ET_FILTER:
                add_elem( elem, row_header_filters );
                break;
            case DiaryElement::ET_CHART:
                add_elem( elem, row_header_charts );
                break;
            case DiaryElement::ET_TABLE:
                add_elem( elem, row_header_tables );
                break;
            case DiaryElement::ET_DIARY:
                add_elem( elem, row_header_diary );
                break;
            default:
                break;
        }
    }
    update_header( row_header_diary, _( "DIARY" ), 1 );
    update_header( row_header_entries, _( "ENTRIES" ), m_diary_r->m_entries.size() );
    update_header( row_header_themes, _( "THEMES" ), m_diary_r->m_themes.size() );
    update_header( row_header_filters, _( "FILTERS" ), m_diary_r->m_filters.size() );
    update_header( row_header_charts, _( "CHARTS" ), m_diary_r->m_charts.size() );
    update_header( row_header_tables, _( "TABLES" ), m_diary_r->m_tables.size() );

    m_TV_contents->set_model( m_TS_content );
    m_TV_contents->expand_all();

    if( path_top.empty() == false )
        m_TV_contents->scroll_to_row( path_top, 0.0 );
}

void
DialogSync::set_cell_color_s( Gtk::CellRenderer* cell, const Gtk::TreeModel::const_iterator& iter )
{
    Color color_b{ COLOR_IGNORE };
    Color color_f{ "#000000" };

    switch( ( * iter )[ m_TS_content->colrec.comp_res ]  )
    {
        case SI::CHANGED:
            color_b.set( "#ffcc66" );
            color_f.set( "#664422" );
            break;
        case SI::NEW:
            color_b.set( "#66cc66" );
            color_f.set( "#446644" );
            break;
        case SI::DELETED:
            color_b.set( "#cc6666" );
            color_f.set( "#664444" );
            break;
        default:
            break;
    }

    cell->property_cell_background_rgba() = color_b;
    dynamic_cast< Gtk::CellRendererText* >( cell )->property_foreground_rgba() = color_f;
}

void
DialogSync::set_cell_color_a( Gtk::CellRenderer* cell, const Gtk::TreeModel::const_iterator& iter )
{
    DiaryElement* elem{ ( * iter )[ m_TS_content->colrec.ptr ] };

    if( elem != nullptr )
    {
        switch( m_action_map[ elem->get_id() ] )
        {
            case SI::ADD:
                cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_ADD );
                break;
            case SI::OVERWRITE:
                cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_OVERWRITE );
                break;
            default:
                cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_IGNORE );
                break;
        }
    }
    else
        cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_IGNORE );

    dynamic_cast< Gtk::CellRendererText* >( cell )->property_foreground_rgba() =
            Gdk::RGBA( "#000000" );
}

void
DialogSync::set_cell_color_comp_l( Gtk::CellRenderer* cell,
                                   const Gtk::TreeModel::const_iterator& iter )
{
    Gdk::RGBA   color_f{ "#000000" };

    switch( ( * iter )[ m_CR_compare.result ] )
    {
        case SI::NEW:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_ADD );
            break;
        case SI::CHANGED:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_CHANGED );
            break;
        case SI::DELETED:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_DELETED );
            break;
        default:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_IGNORE );
            color_f.set( COLOR_IGNORE_FG );
            break;
    }

    dynamic_cast< Gtk::CellRendererText* >( cell )->property_foreground_rgba() = color_f;
}

void
DialogSync::set_cell_color_comp_r( Gtk::CellRenderer* cell,
                                   const Gtk::TreeModel::const_iterator& iter )
{
    Gdk::RGBA   color_f{ "#000000" };

    switch( ( * iter )[ m_CR_compare.result ] )
    {
        case SI::NEW:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_DELETED );
            break;
        case SI::CHANGED:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_CHANGED );
            break;
        case SI::DELETED:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_ADD );
            break;
        default:
            cell->property_cell_background_rgba() = Gdk::RGBA( COLOR_IGNORE );
            color_f.set( COLOR_IGNORE_FG );
            break;
    }

    dynamic_cast< Gtk::CellRendererText* >( cell )->property_foreground_rgba() = color_f;
}

inline bool
DialogSync::is_ready()
{
    return( not( m_F_identical ) && m_flag_diary_is_ready );
}

void
DialogSync::set_ready()
{
    m_B_apply_scheme->set_sensitive( true );
    m_B_go->set_sensitive( true );
    m_flag_diary_is_ready = true;
}

void
DialogSync::set_not_ready( const Ustring& message )
{
    m_flag_diary_is_ready = false;

    m_B_go->set_sensitive( false );
    m_B_apply_scheme->set_sensitive( false );

    m_LS_compare->clear();
    m_TS_content->clear();
    m_action_map.clear();

    if( m_diary_r )
    {
        delete m_diary_r;
        m_diary_r = nullptr;
    }

    if( ! m_IB_sync->is_visible() )
        m_IB_sync->set_visible( true );

    m_L_info->set_text( message );
}

void
DialogSync::go_synchonize()
{
    // prepare the new tag
    Entry*  tag_all{ m_WEP_widget->get_entry_create( Diary::d ) };
    SI      action;

    // THEMES
    for( auto& kv_theme : m_diary_r->m_themes )
    {
        action = m_action_map[ kv_theme.second->get_id() ];
        if( action != SI::SI_IGNORE )
            Diary::d->import_theme( kv_theme.second, action == SI::ADD );
    }

    // FILTERS
    for( auto& kv_filter : m_diary_r->m_filters )
    {
        action = m_action_map[ kv_filter.second->get_id() ];
        if( action != SI::SI_IGNORE )
            Diary::d->import_filter( kv_filter.second, action == SI::ADD );
    }

    // CHARTS
    for( auto& kv_chart : m_diary_r->m_charts )
    {
        action = m_action_map[ kv_chart.second->get_id() ];
        if( action != SI::SI_IGNORE )
            Diary::d->import_chart( kv_chart.second, action == SI::ADD );
    }

    // TABLES
    for( auto& kv_table : m_diary_r->m_tables )
    {
        action = m_action_map[ kv_table.second->get_id() ];
        if( action != SI::SI_IGNORE )
            Diary::d->import_table( kv_table.second, action == SI::ADD );
    }

    // ENTRYIES
    for( Entry* e = m_diary_r->get_entry_1st(); e; e = e->get_next_straight() )
    {
        action = m_action_map[ e->get_id() ];
        if( action != SI::SI_IGNORE )
            Diary::d->import_entry( e, tag_all, action == SI::ADD );
    }

    // DIARY OPTIONS --comes last to be able to use impported elements
    if( m_action_map[ m_diary_r->get_id() ] != SI::SI_IGNORE )
        Diary::d->synchronize_options( m_diary_r );

    set_not_ready();

    hide();
}
