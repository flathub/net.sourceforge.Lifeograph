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


#include "../helpers.hpp"
#include "../lifeograph.hpp"
#include "widget_entrypicker.hpp"


using namespace LIFEO;


// COMPLETION POPOVER ==============================================================================
bool EntryPickerCompletion::s_F_offer_new{ false };
// this is always false for now but in the future there may be cases for true
// please always revert it to false after being done

EntryPickerCompletion::EntryPickerCompletion( Gtk::Widget& parent )
: m_W_parent( parent ), m_F_offer_new( s_F_offer_new )
{
    Gtk::Button*         B_add;
    // Gtk::ScrolledWindow* SW_list;

    m_builder       = Lifeograph::create_gui( Lifeograph::SHAREDIR + "/ui/widget_entry_picker.ui" );

    m_Po            = m_builder->get_widget< Gtk::Popover >( "Po_picker" );
    // SW_list         = m_builder->get_widget< Gtk::ScrolledWindow >( "SW_list" );
    m_LB            = m_builder->get_widget< Gtk::ListBox >( "LBx_list" );
    m_Bx_new_under  = m_builder->get_widget< Gtk::Box >( "Bx_new_under" );

    m_Po->set_parent( parent );

    m_LB->signal_row_activated().connect( [ this ]( Gtk::ListBoxRow* row )
            {
                Entry* entry{ m_items[ row->get_index() ] };

                if( entry ) // do nothing on the creat new line
                {
                    m_Sg_entry_activated.emit( entry );
                    popdown();
                }
            } );
    m_LB->signal_row_selected().connect( [ this ]( Gtk::ListBoxRow* row )
            {
                if( row )
                    m_Bx_new_under->set_visible( !m_items[ row->get_index() ] );
            } );

    if( !m_F_offer_new ) return;

    s_F_offer_new = false; // blocks infinite recursion
    m_WEP_new_under = Gtk::Builder::get_widget_derived< WidgetEntryPicker >( m_builder,
                                                                             "E_new_under" );
    m_WEP_new_under->set_name( "noncustom" );
    B_add           = m_builder->get_widget< Gtk::Button >( "B_add" );
    B_add->signal_clicked().connect( std::bind( &EntryPickerCompletion::create_new_entry, this ) );
}

void
EntryPickerCompletion::set_diary( Diary* diary )
{
    m_p2diary = diary;
    if( m_WEP_new_under )
        m_WEP_new_under->set_diary( m_p2diary );
}

EntryPickerCompletion::~EntryPickerCompletion()
{
    m_Po->unparent();
}

void
EntryPickerCompletion::add_item( Entry* entry )
{
    auto Bx_item{ Gtk::manage( new Gtk::Box( Gtk::Orientation::VERTICAL ) ) };
    auto L_item_name{ Gtk::manage( new Gtk::Label( entry->get_title_ancestral(),
                                                   Gtk::Align::START ) ) };
    auto L_item_info{ Gtk::manage( new Gtk::Label( entry->get_description(), Gtk::Align::START ) ) };

    L_item_name->set_margin_end( 15 ); // this makes it look better
    L_item_name->set_ellipsize( Pango::EllipsizeMode::START );
    L_item_name->set_use_markup( true );

    Pango::AttrList attrlist;
    auto&& attrscale{ Pango::Attribute::create_attr_scale( 0.8 ) };
    attrlist.insert( attrscale );
    L_item_info->set_attributes( attrlist );
    //L_item_info->set_sensitive( false );
    L_item_info->set_ellipsize( Pango::EllipsizeMode::END );

    Bx_item->append( *L_item_name );
    Bx_item->append( *L_item_info );

    m_LB->append( *Bx_item );

    m_items.push_back( entry );
}

void
EntryPickerCompletion::add_new_item()
{
    auto&& text{ Ustring::compose( _( "Create New Entry: %1" ),
                                   STR::compose( "<b>", Glib::Markup::escape_text( m_filter ),
                                                 "</b>" ) ) };
    auto   Bx_item{ Gtk::manage( new Gtk::Box( Gtk::Orientation::VERTICAL ) ) };
    auto   L_item_name{ Gtk::manage( new Gtk::Label( text, Gtk::Align::START ) ) };

    L_item_name->set_use_markup( true );

    Bx_item->append( *L_item_name );

    m_LB->append( *Bx_item );

    m_items.push_back( nullptr );
}

void
EntryPickerCompletion::clear()
{
    remove_all_children_from_LBx( m_LB );
    m_items.clear();
}

void
EntryPickerCompletion::popup( const Ustring& filter )
{
    Gdk::Rectangle rect{ 0, 0,
                         m_Po->get_parent()->get_width(),
                         m_Po->get_parent()->get_height() };

    popup( rect, filter );
}

void
EntryPickerCompletion::popup( const Gdk::Rectangle& rect, const Ustring& filter )
{
    update_list( filter );

    if( !m_LB->get_first_child() ) // means: if empty
        return;

    m_LB->select_row( * m_LB->get_row_at_index( 0 ) );
    m_Po->set_pointing_to( rect );
    m_Po->show();
}

void
EntryPickerCompletion::popdown()
{
    m_Po->hide();
}

struct FuncCmpEntriesByRefs
{
    bool operator()( Entry* const& l, Entry* const& r ) const
    {
        return( l->get_reference_count() > r->get_reference_count() );
    }
};

void
EntryPickerCompletion::update_list( const Ustring& filter )
{
    if( !m_p2diary )
        return;

    clear();

    m_filter = filter;

    const Ustring&&     filter_l{ STR::lowercase( filter ) };
    int                 count{ 0 };
    std::multiset< Entry*, FuncCmpEntriesByRefs >
                        entries_sorted;
    std::multiset< Entry*, FuncCmpEntriesByRefs >
                        entries_sorted_exact;


    for( Entry* e = m_p2diary->get_entry_1st(); e; e = e->get_next_straight() )
    {
        const Ustring&& entry_name_l{ STR::lowercase( e->get_name() ) };

        if( entry_name_l.find( filter_l ) != Ustring::npos )
        {
            if( entry_name_l == filter_l )
                entries_sorted_exact.insert( e );
            else
                entries_sorted.insert( e );
        }
    }

    for( Entry* e : entries_sorted_exact )
    {
        add_item( e );
        for( Entry* p2ec = e->get_child_1st(); p2ec; p2ec = p2ec->get_next() )
            add_item( p2ec );
    }

    for( Entry* e : entries_sorted )
    {
        if( ++count > 20 )
            break;
        add_item( e );
    }

    if( m_F_offer_new )
        add_new_item();
}

bool
EntryPickerCompletion::handle_key( guint keyval, bool F_ctrl, bool F_shift )
{
    if( !is_on_display() )
        return false;
    else if( F_ctrl )
    {
        if( keyval == GDK_KEY_Return )
        {
            create_new_entry();
            return true;
        }
    }
    else if( F_shift )
    {
        if( keyval == GDK_KEY_ISO_Left_Tab )
        {
            select_LB_item_prev( m_LB );
            return true;
        }
    }
    else
    {
        switch( keyval )
        {
            case GDK_KEY_Return:
            {
                auto e{ get_selected() };
                if( e )
                {
                    m_Sg_entry_activated.emit( e );
                    popdown();
                }
                else // i.e. create new entry is secected
                    m_WEP_new_under->grab_focus();
                return true;
            }
            case GDK_KEY_Escape:
                popdown();
                return true;
            case GDK_KEY_Tab:
            case GDK_KEY_Down:
                select_LB_item_next( m_LB );
                return true;
            case GDK_KEY_Up:
                select_LB_item_prev( m_LB );
                return true;
        }
    }

    return false;
}

Entry*
EntryPickerCompletion::get_selected() const
{
    auto row { m_LB->get_selected_row() };
    return( row ? m_items[ row->get_index() ] : nullptr );
}

void
EntryPickerCompletion::create_new_entry()
{
    auto entry { m_p2diary->create_entry( m_WEP_new_under->get_entry(),
                                          true,
                                          Date::get_today(),
                                          m_filter,
                                          m_WEP_new_under->get_entry()
                                                ? VT::ETS::INHERIT::I
                                                : VT::ETS::NAME_ONLY::I ) };
    if( entry )
    {
        m_Sg_entry_activated.emit( entry );
        popdown();
    }
}

// ENTRY PICKER WIDGET =============================================================================
WidgetEntryPicker::WidgetEntryPicker( BaseObjectType* o, const Glib::RefPtr<Gtk::Builder>& b )
: EntryClear( o, b )
{
    initialize();
}

WidgetEntryPicker::WidgetEntryPicker( bool F_editable, const String& icon_name )
{
    EntryPickerCompletion::s_F_offer_new = F_editable;

    if( not( icon_name.empty() ) )
        set_icon_from_icon_name( icon_name, Gtk::Entry::IconPosition::PRIMARY );

    initialize();
}

void
WidgetEntryPicker::initialize()
{
    m_completion = new EntryPickerCompletion( *this );

    if( get_icon_name( Gtk::Entry::IconPosition::PRIMARY ) == "" ) // if not set in the ui file
        set_icon_from_icon_name( "entry-16-symbolic", Gtk::Entry::IconPosition::PRIMARY );

    // does not seem to make a difference, why?
    set_icon_activatable( false, Gtk::Entry::IconPosition::PRIMARY );

    // SIGNALS
    m_completion->signal_entry_activated().connect(
            [ this ]( LIFEO::Entry* e ){ handle_entry_selected( e ); } );

    // we couldn't detect enter key in on_key_press_event:
    signal_activate().connect(
            [ this ]()
            {
                if( m_completion && m_completion->is_on_display() )
                    m_completion->handle_key( GDK_KEY_Return, false, false );
            } );

    // the following is necessary to ensure timely destruction/unparenting
    signal_destroy().connect(
            [ this ]()
            {
                if( m_completion )
                {
                    delete m_completion;
                    m_completion = nullptr;
                }
            } );

    // CONTROLLERS
    auto controller_key{ Gtk::EventControllerKey::create() };
    controller_key->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetEntryPicker::on_key_press_event ), false );
    add_controller( controller_key );

    m_drop_target = Gtk::DropTarget::create( LIFEO::Entry::GValue::value_type(),
                                             Gdk::DragAction::COPY );
    // m_drop_target->signal_motion().connect(
    //         sigc::mem_fun( *this, &EntryPickerCompletion::handle_drop_motion ), false );
    m_drop_target->signal_drop().connect(
            sigc::mem_fun( *this, &WidgetEntryPicker::handle_drop ), false );
    add_controller( m_drop_target );
}

void
WidgetEntryPicker::set_entry( LIFEO::Entry* entry )
{
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_p2entry = entry;
    Gtk::Entry::set_text( entry ? entry->get_name() : "" );
    set_tooltip_markup( entry ? entry->get_title_ancestral() : "" );
    remove_css_class( "error" );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

Entry*
WidgetEntryPicker::get_entry_create( Diary* diary )
{
    if( m_p2entry )
        return m_p2entry;

    if( !diary || get_text().empty() )
        return nullptr;

    return diary->create_entry( nullptr, false, Date::get_today(), get_text() );
}

void
WidgetEntryPicker::on_changed()
{
    if( Lifeograph::is_internal_operations_ongoing() )
        return;

    const Ustring text_new{ get_text() };
    bool flag_ok{ false };

    // empty
    if( text_new.empty() )
    {
        m_p2entry = nullptr;
        m_completion->popdown();
        flag_ok = true;
    }
    else
    {
        m_completion->popup( text_new );

        auto e{ m_completion->get_diary()->get_entry_by_name( text_new ) };
        if( e )
        {
            m_p2entry = e;
            flag_ok = true;
        }
        else
            m_p2entry = nullptr;
    }

    set_tooltip_markup( m_p2entry ? m_p2entry->get_title_ancestral() : "" );

    m_Sg_updated.emit( m_p2entry );

    if( flag_ok )
        remove_css_class( "error" );
    else
        add_css_class( "error" );

    EntryClear::on_changed();
}

bool
WidgetEntryPicker::on_key_press_event( guint keyval, guint, Gdk::ModifierType state )
{
    // if( ( state & ( Gdk::ModifierType::ALT_MASK|Gdk::ModifierType::SHIFT_MASK ) ) ==
    //     Gdk::ModifierType( 0 ) )
    {
        if( m_completion->handle_key( keyval, bool( state & Gdk::ModifierType::CONTROL_MASK ),
                                              bool( state & Gdk::ModifierType::SHIFT_MASK ) ) )
        {
            return true;
        }
    }
    return false;
}

void
WidgetEntryPicker::handle_entry_selected( LIFEO::Entry* entry )
{
    set_entry( entry );
    m_Sg_updated.emit( entry );
}

Gdk::DragAction
WidgetEntryPicker::handle_drop_motion( double x, double y )
// TODO: 3.1: gtkmm4: this function is probably no longer needed
{
    // if( Lifeograph::get_dragged_elem() != nullptr &&
    //     Lifeograph::get_dragged_elem()->get_type() >= DiaryElement::ET_ENTRY )
    {
        //drag_highlight();
        return Gdk::DragAction::COPY;
    }

    // drag_unhighlight();  // is called many times unnecessarily :(
    // context->drag_refuse( time );
    return Gdk::DragAction( 0 );
}

bool
WidgetEntryPicker::handle_drop( const Glib::ValueBase& value, double x, double y )
{
    if( G_VALUE_HOLDS( value.gobj(), LIFEO::Entry::GValue::value_type() ) )
    {
        LIFEO::Entry::GValue value_entry;
        value_entry.init( value.gobj() );

        handle_entry_selected( value_entry.get() );
        return true;
    }

    return false;
}

// SIMPLE POPOVER FOR SELECTING AN ELEMENT FROM A GROUP ============================================
VecDiaryElems   PoElemChooser::m_elems;
bool            PoElemChooser::m_F_para_mode;
Gtk::Popover*   PoElemChooser::m_Po { nullptr };
FuncVoidElem    PoElemChooser::handle_activate;

void
LIFEO::PoElemChooser::show( const SetDiaryElemsByName& elems,
                            std::function< void( DiaryElement* ) >&& select_handler,
                            Gtk::Widget& host,
                            const Gdk::Rectangle& rect )
{
    // clear the previsous values first:
    m_elems.clear();
    if( m_Po ) delete m_Po;
    m_Po = Gtk::make_managed< Gtk::Popover >();

    // new values:
    auto    SW          { Gtk::make_managed< Gtk::ScrolledWindow >() };
    auto    LV          { Gtk::make_managed< Gtk::ListView >() };
    auto    factory     { Gtk::SignalListItemFactory::create() };
    auto    stringlist  { Gtk::StringList::create() };
    int     size        { 0 };

    // fill the list model:
    for( auto& elem : elems )
    {
        if( ++size > 100 ) break; // limit to 100 elems

        stringlist->append( "" ); // just a placeholder TODO: 3.1: combine stringlist with m_elems
        m_elems.push_back( elem );
    }

    m_F_para_mode = ( size > 0 && m_elems[ 0 ]->get_type() == DiaryElement::ET_PARAGRAPH );

    handle_activate = select_handler;

    // set list model and selection model:
    auto selection_model { Gtk::SingleSelection::create( stringlist ) };
    selection_model->set_autoselect( false );
    // selection_model->set_can_unselect( true );
    LV->set_model( selection_model );
    LV->add_css_class( "data-table" ); // high density table
    LV->set_single_click_activate( true );
    LV->set_show_separators( true );

    factory->signal_setup().connect(
            []( const Glib::RefPtr< Gtk::ListItem >& list_item )
            {
                auto Bx       { Gtk::make_managed< Gtk::Box >( Gtk::Orientation::VERTICAL, 0 ) };
                auto L_entry  { Gtk::make_managed< Gtk::Label >() };

                L_entry->set_ellipsize( Pango::EllipsizeMode::MIDDLE );
                L_entry->set_halign( Gtk::Align::START );

                Bx->append( *L_entry );

                if( m_F_para_mode )
                {
                    auto L_para { Gtk::make_managed< Gtk::Label >() };

                    L_para->set_ellipsize( Pango::EllipsizeMode::END );
                    L_para->set_halign( Gtk::Align::START );

                    Bx->append( *L_para );
                }

                list_item->set_child( *Bx );
            } );

    factory->signal_bind().connect(
            []( const Glib::RefPtr< Gtk::ListItem >& list_item )
            {
                auto Bx       { dynamic_cast< Gtk::Box* >( list_item->get_child() ) };
                auto L_entry  { dynamic_cast< Gtk::Label* >( Bx->get_first_child() ) };
                auto elem     { m_elems[ list_item->get_position() ] };

                if( m_F_para_mode )
                {
                    auto L_para { dynamic_cast< Gtk::Label* >( L_entry->get_next_sibling() ) };
                    const auto para { dynamic_cast< Paragraph* >( elem ) };
                    L_entry->set_markup( para->m_host->get_title_ancestral() );
                    L_para->set_text( para->get_text() );
                }
                else
                {
                    const auto entry { dynamic_cast< Entry* >( elem ) };
                    L_entry->set_markup( entry->get_title_ancestral() );
                }

                // if( entry == AppWindow::p->UI_entry->get_cur_entry() ) menuitem->set_icon( Gio::Icon::create( "go-home-symbolic" ) );
            } );

    LV->set_factory( factory );
    LV->signal_activate().connect(
            []( guint i )
            {
                m_Po->hide();
                handle_activate( m_elems[ i ] );
            } );

    SW->set_propagate_natural_width( true );
    SW->set_propagate_natural_height( true );
    SW->set_child( *LV );
    m_Po->set_child( *SW );
    m_Po->signal_destroy().connect( [](){ m_Po->unparent(); } );
    m_Po->set_parent( host );
    m_Po->set_pointing_to( rect );
    m_Po->show();
}
