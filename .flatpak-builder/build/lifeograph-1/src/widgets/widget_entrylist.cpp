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
#include "../ui_entry.hpp"
#include "widget_entrylist.hpp"


using namespace LIFEO;


// WIDGET ENTRY LIST ===============================================================================
WidgetEntryList::WidgetEntryList( BaseObjectType* cobject, const Glib::RefPtr< Gtk::Builder >& )
:   Gtk::ListView( cobject )
{
    // Add the factory for the ListView's single column.
    auto factory = Gtk::SignalListItemFactory::create();

    factory->signal_setup().connect( sigc::mem_fun( *this, &WidgetEntryList::handle_setup_row ) );
    factory->signal_bind().connect( sigc::mem_fun( *this, &WidgetEntryList::handle_bind_row ) );
    factory->signal_unbind().connect( sigc::mem_fun( *this, &WidgetEntryList::handle_unbind_row ) );

    set_factory( factory );

    // SIGNALS
    signal_activate().connect( sigc::mem_fun( *this, &WidgetEntryList::handle_row_activated ) );

    // CONTROLLERS
    m_controller_key = Gtk::EventControllerKey::create();
    m_controller_key->signal_key_pressed().connect(
            sigc::mem_fun( *this, &WidgetEntryList::handle_key_press ), false );
    add_controller( m_controller_key );
}

Glib::RefPtr< Gio::ListModel >
WidgetEntryList::create_model( const Glib::RefPtr< Glib::ObjectBase >& item )
{
    const auto result { Gio::ListStore< GObjectEntry >::create() };
    auto       gentry { std::dynamic_pointer_cast< GObjectEntry >( item ) };
    auto       entry  { gentry ? gentry->m_p2entry->get_child_1st() : Diary::d->get_entry_1st() };

    for( ; entry; entry = entry->get_next() )
    {
        if( entry->is_filtered_out_completely() != 'o' ) // visible or has_visible descendant
            result->append( GObjectEntry::create( entry ) );
    }

    // If result is empty, it's a leaf in the tree, i.e. an item without children.
    // Returning an empty RefPtr (not a RefPtr with an empty StringList)
    // signals that the item is not expandable.
    return( result->get_n_items() > 0 ? result : Glib::RefPtr< Gio::ListModel >() );
}

void
WidgetEntryList::handle_setup_row( const Glib::RefPtr< Gtk::ListItem >& list_item )
{
    auto expander = Gtk::make_managed< Gtk::TreeExpander >();
    auto box      = Gtk::make_managed< Gtk::Box >( Gtk::Orientation::HORIZONTAL, 5 );
    auto image    = Gtk::make_managed< Gtk::Image >();
    auto label    = Gtk::make_managed< Gtk::Label >();
    auto I_fav    = Gtk::make_managed< Gtk::Image >( Lifeograph::icons->favorite_16 );

    // TODO: 3.1: we may add this as an option on v3.1:
    // Pango::AttrList attrs;
    // auto&&          attrscale{ Pango::Attribute::create_attr_scale( 0.9 ) };
    // attrs.insert( attrscale );

    expander->set_margin_start( 8 ); // to avoid interference with the Paned handle. is this a Gtk bug?
    label->set_halign( Gtk::Align::START );
    label->set_hexpand( true );
    label->set_ellipsize( Pango::EllipsizeMode::MIDDLE );
    // label->set_attributes( attrs );
    box->set_margin_end( 5 );
    box->append( *image );
    box->append( *label );
    box->append( *I_fav );
    expander->set_child( *box );
    list_item->set_child( *expander );
}

void
WidgetEntryList::handle_bind_row( const Glib::RefPtr< Gtk::ListItem >& list_item )
{
    // When TreeListModel::property_passthrough() is false, ListItem::get_item()
    // is a TreeListRow. TreeExpander needs the TreeListRow.
    auto row      { std::dynamic_pointer_cast< Gtk::TreeListRow >( list_item->get_item() ) };
    if( !row ) return;
    auto gentry   { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
    if( !gentry ) return;
    auto expander { dynamic_cast< Gtk::TreeExpander* >( list_item->get_child() ) };
    if( !expander ) return;

    expander->set_list_row( row );
    // expander->set_indent_for_icon( true ); // this is the default it seems

    gentry->m_p2entry->set_list_order( row->get_position() );

    // note that when an entry is collapsed, its sub-entries are also collapsed automatically
    if( gentry->m_p2entry->get_child_1st() )
    {
        gentry->m_connection_expand = row->property_expanded()
            .signal_changed().connect( std::bind( &WidgetEntryList::handle_expand_changed,
                                                  this,
                                                  row ) );
    }

    // WIDGET CONTENTS
    gentry->m_Bx = dynamic_cast< Gtk::Box* >( expander->get_child() );
    gentry->m_I = dynamic_cast< Gtk::Image* >( gentry->m_Bx->get_first_child() );
    gentry->m_I->set( gentry->m_p2entry->get_icon() );
    gentry->m_L = dynamic_cast< Gtk::Label* >( gentry->m_I->get_next_sibling() );
    gentry->m_L->set_markup( gentry->m_p2entry->get_list_str() );
    gentry->m_L->set_tooltip_markup( gentry->m_p2entry->get_info_str() );
    if( gentry->m_p2entry->is_trashed() )
        gentry->m_L->set_sensitive( false );
    gentry->m_I_fav = dynamic_cast< Gtk::Image* >( gentry->m_Bx->get_last_child() );
    gentry->m_I_fav->set_visible( gentry->m_p2entry->is_favorite() );

    // CONTROLLERS
    auto gesture_click = Gtk::GestureClick::create();
    gesture_click->set_button( 3 ); // right click
    gesture_click->signal_released().connect(
            sigc::bind( sigc::mem_fun( *this, &WidgetEntryList::handle_menu_click ), gentry ),
            false );
    expander->add_controller( gesture_click );

    // DRAG & DROP
    if( !m_F_editable ) return;

    auto drag_source { Gtk::DragSource::create() };
    drag_source->signal_prepare().connect(
        sigc::bind( sigc::mem_fun( *this, &WidgetEntryList::handle_drag_prepare ),
                    drag_source, gentry ),
        false );
    drag_source->signal_drag_end().connect(
            [ this ]( const Glib::RefPtr< Gdk::Drag >&, bool )
            {
                m_id_dragged_entry = DEID_UNSET;
                m_drop_position = DropPosition::NONE;
                queue_draw();
            } );
    expander->add_controller( drag_source );

    auto drop_target { Gtk::DropTarget::create( G_TYPE_INVALID,
                                                Gdk::DragAction::COPY|Gdk::DragAction::MOVE ) };

    drop_target->set_gtypes( { Glib::Value< Ustring >::value_type(),
                               Theme::GValue::value_type(),
                               Entry::GValue::value_type(),
                               EntrySelection::GValue::value_type() } );
    drop_target->signal_motion().connect(
            sigc::bind( sigc::mem_fun( *this, &WidgetEntryList::handle_drop_motion ),
                        drop_target, expander, gentry->m_p2entry ),
            false );
    drop_target->signal_drop().connect(
            sigc::bind( sigc::mem_fun( *this, &WidgetEntryList::handle_drop ),
                        gentry->m_p2entry ),
            false );
    expander->add_controller( drop_target );
}
void
WidgetEntryList::handle_unbind_row( const Glib::RefPtr< Gtk::ListItem >& list_item )
{
    auto row    { std::dynamic_pointer_cast< Gtk::TreeListRow >( list_item->get_item() ) };
    if( !row ) return;
    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };

    if( gentry )
        gentry->m_connection_expand.disconnect();
}

void
WidgetEntryList::size_allocate_vfunc( int w, int h, int b )
{
    Gtk::ListView::size_allocate_vfunc( w, h, b );
    AppWindow::p->UI_diary->update_PoEntry_size();
}

void
WidgetEntryList::set_editable()
{
    m_F_editable = true;
    AppWindow::p->UI_diary->update_entry_list(); // rebinds the rows
}

void
WidgetEntryList::clear_and_disable()
{
    m_F_editable = false;
    set_model( Gtk::SingleSelection::create() );
}

void
WidgetEntryList::populate()
{
    Lifeograph::START_INTERNAL_OPERATIONS();

    clear_cut_entries(); // reset the cut entries at updates

    // create the root model:
    auto root = create_model();

    if( !root )
    {
        m_selection_model = Gtk::NoSelection::create();
        // we handle this separately because empty list case creates problems with multi model
        PRINT_DEBUG( "NO ITEMS IN THE LIST" );
    }
    else
    {
        // set list model and selection model:
        // passthrough must be false when Gtk::TreeExpander is used in the view
        m_TLM = Gtk::TreeListModel::create( root,
                                            sigc::mem_fun( *this, &WidgetEntryList::create_model ),
                                            false, /* passthrough */
                                            false  /* autoexpand */ );

        m_selection_model = Gtk::MultiSelection::create( m_TLM );

        // update expansion states of the rows by expanding the needed rows:
        for( unsigned int i = 0; i < m_TLM->property_n_items(); ++i )
        {
            auto row    { m_TLM->get_row( i ) };
            if( !row ) continue;
            auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
            if( !gentry ) continue;

            if( gentry->m_p2entry->is_expanded() )
                row->set_expanded( true );
        }
    }

    // the following is the standard way but does not fire when the active entry...
    // ...is a child of a collapsed parent and the parent is selected:
    // m_selection_model->signal_selection_changed().connect(
    //         sigc::mem_fun( *this, &WidgetEntryList::handle_selection_changed
    //         ) );

    auto gesture_click { Gtk::GestureClick::create() };
    gesture_click->set_button( 1 );
    gesture_click->signal_released().connect(
                [ this ]( int, double, double ) { handle_selection_changed(); } );

    add_controller( gesture_click );

    set_model( m_selection_model );

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetEntryList::select( Entry* p2entry )
{
    m_selected_entries.clear();

    Lifeograph::START_INTERNAL_OPERATIONS();

    if( p2entry )
    {
        p2entry = p2entry->get_first_exposed_upwards();
        const auto order { p2entry->get_list_order() };
        if( order >= 0 )
        {
            get_model()->select_item( order, true );
            m_selected_entries.insert( p2entry );
        }
    }
    else
        get_model()->unselect_all();

    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetEntryList::scroll_to_entry( const Entry* p2entry )
{
    const int order { p2entry->get_list_order() };

    Lifeograph::START_INTERNAL_OPERATIONS();
    scroll_to( order < 6 ? 0 : ( order - 5 ), Gtk::ListScrollFlags::NONE, {} );
    // NOTE: we cannot use ListScrollFlags::SELECT due to the overscrolling trick we use
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetEntryList::present_entry_hard( const Entry* p2entry )
{
    bool F_expasion_done { false };
    // expand recursively:
    for( Entry* p2ep = p2entry->get_parent(); p2ep; p2ep = p2ep->get_parent() )
    {
        if( p2ep->is_expanded() ) // no need to go further
            break;
        else
        {
            F_expasion_done = true;
            p2ep->set_expanded( true );
        }
    }

    if( F_expasion_done )
        populate();

    scroll_to_and_select_entry( p2entry );
}

void
WidgetEntryList::present_entry( const Entry* p2entry )
{
    scroll_to_and_select_entry( p2entry->get_first_exposed_upwards() );
}

void
WidgetEntryList::expand_entry( const Entry* p2entry ) // this function does not do much
{
    Lifeograph::START_INTERNAL_OPERATIONS();
    m_TLM->get_row( p2entry->get_list_order() )->set_expanded( true );
    Lifeograph::FINISH_INTERNAL_OPERATIONS();
}

void
WidgetEntryList::refresh_row( const Entry* entry )
{
    if( Lifeograph::is_internal_operations_ongoing() || !m_TLM || !entry->is_exposed_in_list() )
        return;

    auto row    { m_TLM->get_row( entry->get_list_order() ) };
    if( !row ) return;
    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
    if( !gentry ) return;

    gentry->m_I->set( entry->get_icon() );
    gentry->m_L->set_markup( entry->get_list_str() );
    gentry->m_L->set_sensitive( !entry->is_trashed() );
    gentry->m_I_fav->set_visible( entry->is_favorite() );
}
// simpler version of the above:
void
WidgetEntryList::refresh_row_label( const Entry* entry )
{
    if( Lifeograph::is_internal_operations_ongoing() || !m_TLM || !entry->is_exposed_in_list() )
        return;

    auto row    { m_TLM->get_row( entry->get_list_order() ) };
    if( !row ) return;
    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
    if( !gentry ) return;

    gentry->m_L->set_markup( entry->get_list_str() );
    gentry->m_L->set_sensitive( !entry->is_trashed() );
}

void
WidgetEntryList::go_up()
{
    if( m_selected_entries.empty() ) return;

    Entry* entry { *m_selected_entries.begin() };

    if( entry == nullptr || entry->get_list_order() < 1 )
        return;

    auto row    { m_TLM->get_row( entry->get_list_order() - 1 ) };
    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };

    if( gentry )
        select( gentry->m_p2entry );
}

void
WidgetEntryList::go_down()
{
    if( m_selected_entries.empty() ) return;

    Entry* entry { *m_selected_entries.begin() };

    if( entry == nullptr || entry->get_list_order() > int( m_TLM->property_n_items() - 2 ) )
        return;

    auto row    { m_TLM->get_row( entry->get_list_order() + 1 ) };
    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };

    if( gentry )
        select( gentry->m_p2entry );
}

void
WidgetEntryList::toggle_selected_collapsed()
{
    if( m_selected_entries.empty() ) return;

    auto row { m_TLM->get_row( ( *m_selected_entries.begin() )->get_list_order() ) };
    if( !row ) return;

    row->set_expanded( !row->get_expanded() );
}

void
WidgetEntryList::handle_menu_click( int n_press, double x, double y,
                                    const Glib::RefPtr< GObjectEntry >& gentry )
{
    // auto event { m_gesture_click->get_last_event( m_gesture_click->get_current_sequence() ) };
    // if( !event->triggers_context_menu() ) return;

    // const bool F_ctrl_or_shift_pressed{
    //         bool( m_gesture_click->get_current_event()->get_modifier_state() &
    //               ( Gdk::ModifierType::CONTROL_MASK | Gdk::ModifierType::SHIFT_MASK) ) };

    // if( !F_ctrl_or_shift_pressed )

    auto pt { gentry->m_Bx->compute_point( *AppWindow::p, Gdk::Graphene::Point( x, y ) ) };

    if( !get_model()->is_selected( gentry->m_p2entry->get_list_order() ) )
        get_model()->select_item( gentry->m_p2entry->get_list_order(), true );

    AppWindow::p->UI_diary->show_Po_entry( gentry->m_p2entry,
                                           Gdk::Rectangle( pt->get_x(), pt->get_y(), 1, 1 ) );
}

bool
WidgetEntryList::handle_key_press( guint keyval, guint, Gdk::ModifierType state )
{
    if( keyval == GDK_KEY_v || keyval == GDK_KEY_V )
    {
        if( !m_F_editable || m_cut_entries.empty() ) return false;

        switch( int( state ) )
        {
            case int( Gdk::ModifierType::ALT_MASK ):
                m_drop_position = DropPosition::INTO;
                break;
            case int( Gdk::ModifierType::CONTROL_MASK ):
                m_drop_position = DropPosition::AFTER;
                break;
            case HELPERS::CTRL_SHIFT_MASK:
                m_drop_position = DropPosition::BEFORE;
                break;
            default: return false;
        }

        Entry* entry_receiving { *m_selected_entries.begin() };
        entry_receiving->get_diary()->move_entries( &m_cut_entries, entry_receiving,
                                                    m_drop_position );

        clear_cut_entries();

        // AppWindow::p->UI_entry->refresh_extra_info();
        AppWindow::p->UI_diary->update_entry_list();

        return true;
    }

    if( state == Gdk::ModifierType::CONTROL_MASK )
    {
        switch( keyval )
        {
            case GDK_KEY_x:
            case GDK_KEY_X:
                if( !m_F_editable ) return false;
                mark_selected_entries_cut();
                return true;
            case GDK_KEY_Return:
                toggle_selected_collapsed();
                return true;
        }
    }
    else if( state == Gdk::ModifierType( 0 ) )
    {
        if( keyval == GDK_KEY_Up )
        {
            // go_up();
            m_F_inhibit_show_entry = true;
            // return true;
            return false;
        }
        else if( keyval == GDK_KEY_Down )
        {
            // go_down();
            m_F_inhibit_show_entry = true;
            // return true;
            return false;
        }
    }

    return false;
}

// DRAG & DROP
// void
// WidgetEntryList::handle_drag_begin( const Glib::RefPtr< Gdk::Drag >& drag,
//                                     const Glib::RefPtr< Gtk::DragSource >& drag_source,
//                                     const Glib::RefPtr< GObjectEntry >& gentry )
// {
// }

Glib::RefPtr< Gdk::ContentProvider >
WidgetEntryList::handle_drag_prepare( double x, double y,
                                      const Glib::RefPtr< Gtk::DragSource >& drag_source,
                                      const Glib::RefPtr< GObjectEntry >& gentry )
{
    // multiple selection case:
    if( m_selected_entries.size() > 1 && m_selected_entries.has_entry( gentry->m_p2entry ) )
    {
        EntrySelection::GValue value_multi;
        value_multi.init( value_multi.value_type() );
        value_multi.set( &m_selected_entries );

        drag_source->set_icon( lookup_default_icon( "entry_plus-16", 16 ), 0, 0 );
        m_id_dragged_entry = DEID_MULTIPLE;

        return Gdk::ContentProvider::create( value_multi );
    }
    else
    {
        LIFEO::Entry::GValue value_entry;
        value_entry.init( value_entry.value_type() );
        value_entry.set( gentry->m_p2entry );

        auto widget_paintable { Gtk::WidgetPaintable::create( *gentry->m_Bx ) };
        drag_source->set_icon( widget_paintable, 0, 0 );
        m_id_dragged_entry = gentry->m_p2entry->get_id();

        return Gdk::ContentProvider::create( value_entry );
    }
}

// void
// WidgetEntryList::handle_drag_end( const Glib::RefPtr< Gdk::Drag >& drag, bool )
// {
//     PRINT_DEBUG( "WidgetEntryList::on_drag_end()" );
// }

Gdk::DragAction
WidgetEntryList::handle_drop_motion( double x, double y,
                                     Glib::RefPtr< Gtk::DropTarget >& drop_target,
                                     Gtk::TreeExpander* expander,
                                     Entry* target_entry )
{
    auto pt       { expander->compute_point( *this, Gdk::Graphene::Point( x, 0.0 ) ) };
    auto pt_2     { expander->compute_point( *this, Gdk::Graphene::Point( 0.0, 0.0 ) ) };
    auto dp_new   { DropPosition::NONE };
    auto formats  { drop_target->get_formats() };
    auto action   { Gdk::DragAction::COPY };

    // disable dropping onto itself:
    if( m_id_dragged_entry == target_entry->get_id() )
    {
        PRINT_DEBUG( "contains entry");
        action = Gdk::DragAction::MOVE;
        return Gdk::DragAction( 0 );
    }
    else if( m_id_dragged_entry == DEID_MULTIPLE )
    {
        PRINT_DEBUG( "contains multiple entries");
        action = Gdk::DragAction::MOVE;
        if( m_selected_entries.find( target_entry ) != m_selected_entries.end() )
            return Gdk::DragAction( 0 );
    }

    if( m_line_height < 1 )
        m_line_height = expander->get_height();

    m_drop_coord_x = pt_2->get_x();

    // EVALUATE THE HOVERED ELEMENT
    if     ( pt->get_x() < OFFSET_DROP_INTO || m_id_dragged_entry == DEID_UNSET )
    {
        dp_new = DropPosition::INTO;
        m_drop_coord_y = pt->get_y();
        m_drop_rect_h = m_line_height;
    }
    else if( y < m_line_height * 0.5 )
    {
        dp_new = DropPosition::BEFORE;
        m_drop_coord_y = pt->get_y();
        m_drop_rect_h = 4.0;
    }
    else
    {
        dp_new = DropPosition::AFTER;
        m_drop_coord_y = pt->get_y() + m_line_height - 3;
        m_drop_rect_h = 4.0;
    }

    if( dp_new != m_drop_position )
    {
        m_drop_position = dp_new;
        queue_draw();
    }

    // ACTION TYPE
    return action;
}

void
WidgetEntryList::mark_selected_entries_cut()
{
    clear_cut_entries();

    for( Entry* e : m_selected_entries )
    {
        e->set_status_flag( ES::CUT, true );
        m_cut_entries.insert( e );
        refresh_row_label( e );
    }
}
void
WidgetEntryList::clear_cut_entries()
{
    for( Entry* e : m_cut_entries )
    {
        e->set_status_flag( ES::CUT, false );
        m_cut_entries.insert( e );
        refresh_row_label( e );
    }

    m_cut_entries.clear();
}

bool
WidgetEntryList::handle_drop( const Glib::ValueBase& value, double x, double y,
                              Entry* p2entry_rcvn )
{
    bool F_ret_val { true };

    // text onto entry:
    if( G_VALUE_HOLDS( value.gobj(), Glib::Value< Ustring >::value_type() ) )
    {
        Glib::Value< Ustring > value_ustring;
        value_ustring.init( value.gobj() );

        if( m_drop_position == DropPosition::INTO )
        {
            p2entry_rcvn->insert_text( Ustring::npos, value_ustring.get(), false );
            refresh_row( p2entry_rcvn );
        }
        else
        {
            // p2entry_rcvn first used for style inheritance:
            Entry* entry_new { Diary::d->create_entry( p2entry_rcvn, true, Date::get_today(),
                                                       value_ustring.get(),
                                                       VT::ETS::INHERIT::I ) };
            // actual position is set in the next step:
            p2entry_rcvn->get_diary()->move_entry( entry_new, p2entry_rcvn, m_drop_position );
            AppWindow::p->UI_diary->update_entry_list( p2entry_rcvn );
        }
    }
    else if( G_VALUE_HOLDS( value.gobj(), Theme::GValue::value_type() ) )
    {
        Theme::GValue value_theme;
        value_theme.init( value.gobj() );

        p2entry_rcvn->set_theme( value_theme.get() );
        if( AppWindow::p->UI_entry->is_cur_entry( p2entry_rcvn ) )
            AppWindow::p->UI_entry->refresh_theme();
    }
    else if( G_VALUE_HOLDS( value.gobj(), LIFEO::Entry::GValue::value_type() ) )
    {
        LIFEO::Entry::GValue  value_entry;
        value_entry.init( value.gobj() );
        LIFEO::Entry*         p2entry_drpd  { value_entry.get() };

        p2entry_rcvn->get_diary()->move_entry( p2entry_drpd, p2entry_rcvn, m_drop_position );

        // AppWindow::p->UI_entry->refresh_extra_info();
        AppWindow::p->UI_diary->update_entry_list( p2entry_drpd );
    }
    else if( G_VALUE_HOLDS( value.gobj(), LIFEO::EntrySelection::GValue::value_type() ) )
    {
        // no need to go down the the following road as we already possess the selected entries
        // LIFEO::EntrySelection::GValue value_multi;
        // value_multi.init( value.gobj() );
        // LIFEO::EntrySelection*        p2entries   { value_multi.get() };

        p2entry_rcvn->get_diary()->move_entries( &m_selected_entries, p2entry_rcvn,
                                                 m_drop_position );

        // AppWindow::p->UI_entry->refresh_extra_info();
        AppWindow::p->UI_diary->update_entry_list( p2entry_rcvn );
    }
    else
    {
        F_ret_val = false;
    }

    m_drop_position = DropPosition::NONE;
    m_id_dragged_entry = DEID_UNSET;
    return F_ret_val;
}

void
WidgetEntryList::handle_selection_changed( /*guint pos, guint n_items*/ )
{
    // NOTE: we did not use the pos & n_items as it turned out to be quite hard
    // to keep an uptodate list of selected items

    if( Lifeograph::is_internal_operations_ongoing() ) return;

    m_selected_entries.clear();

    auto&& selected_entries { m_selection_model->get_selection() };
    for( auto it : *selected_entries )
    {
        auto row    { m_TLM->get_row( it ) };
        if( !row ) continue;
        auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
        if( !gentry ) continue;

        m_selected_entries.insert( gentry->m_p2entry );
    }

    // show the entry if it is the only one selected:
    if( m_selected_entries.size() == 1 && !m_F_inhibit_show_entry )
    {
        Entry* entry { *m_selected_entries.begin() };
        AppWindow::p->UI_entry->show( entry );
        AppWindow::p->UI_extra->set_entry( entry );
    }
    m_F_inhibit_show_entry = false;
}

void
WidgetEntryList::snapshot_vfunc( const Glib::RefPtr< Gtk::Snapshot >& snapshot )
{
    const static std::vector< double > dash_pattern { 7.0, 7.0 };

    Gtk::ListView::snapshot_vfunc( snapshot );

    if( m_drop_position != DropPosition::NONE || m_id_dragged_entry != DEID_UNSET )
    {
        const int       w       { get_width() };
        const int       h       { get_height() };
        auto            cr      { snapshot->append_cairo( Gdk::Rectangle( 0, 0, w, h ) ) };

        if( m_drop_position != DropPosition::NONE )
        {
            Gdk::Cairo::set_source_rgba( cr, Gdk::RGBA( "#FF333366" ) );
            // cr->set_line_join( Cairo::Context::LineJoin::ROUND );

            cr->rectangle( m_drop_coord_x, m_drop_coord_y, w - m_drop_coord_x, m_drop_rect_h );

            cr->fill();
        }

        if( m_id_dragged_entry != DEID_UNSET ) // only true for entries
        {
            Gdk::Cairo::set_source_rgba( cr, Gdk::RGBA( "#CC3333AA" ) );
            cr->set_line_width( 2.0 );
            cr->set_dash( dash_pattern, 0.0 );
            cr->move_to( OFFSET_DROP_INTO, 0.0 );
            cr->rel_line_to( 0.0, h );
            cr->stroke();
        }
    }
}

void
WidgetEntryList::do_for_each_selected_entry( const std::function< void( Entry* ) >& slot )
{
    for( Entry* entry : m_selected_entries )
    {
        slot( entry );
    }
}

void
WidgetEntryList::handle_expand_changed( const Glib::RefPtr< Gtk::TreeListRow >& row )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };
    if( !gentry ) return;
    gentry->m_p2entry->set_expanded( row->get_expanded() );
    if( !row->get_expanded() )
    {
        gentry->m_p2entry->do_for_each_descendant( []( Entry* e ) { e->set_list_order( -1 ); } );
        if( AppWindow::p->UI_entry->get_cur_entry()->is_descendant_of( gentry->m_p2entry ) )
            Glib::signal_idle().connect_once(
                    [ this ](){ select( AppWindow::p->UI_entry->get_cur_entry() ); } );
    }

    PRINT_DEBUG( "entry [", gentry->m_p2entry->get_name(),
                 "] new expand state: ", row->get_expanded() );
}

void
WidgetEntryList::handle_row_activated( guint pos )
{
    if( Lifeograph::is_internal_operations_ongoing() ) return;

    PRINT_DEBUG( "Activated entry no: ", pos );

    // auto row { m_TLM->get_row( pos ) };
    // if( !row ) return;
    // auto gentry { std::dynamic_pointer_cast< GObjectEntry >( row->get_item() ) };

    // if( gentry )
    if( m_selected_entries.size() == 1 )
    {
        Entry* entry { *m_selected_entries.begin() };

        if( !AppWindow::p->UI_entry->is_cur_entry( entry ) )
        {
            AppWindow::p->UI_entry->show( /*gentry->m_p2*/entry );
            AppWindow::p->UI_extra->set_entry( /*gentry->m_p2*/entry );
        }
    }
}

