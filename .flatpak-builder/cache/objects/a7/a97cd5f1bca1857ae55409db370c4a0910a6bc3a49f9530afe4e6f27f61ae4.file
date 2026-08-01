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


#ifndef LIFEOGRAPH_DIALOG_EXPORT_HEADER
#define LIFEOGRAPH_DIALOG_EXPORT_HEADER

// TODO: 3.1: rename this file to cover all of its current functions


#include <gtkmm.h>

#include "../helpers.hpp"
#include "../diaryelements/diary.hpp"
#include "../widgets/widget_filterpicker.hpp"
#include "glibmm/refptr.h"
#include "gtkmm/filedialog.h"


namespace LIFEO
{

// FILE SAVE DIALOGS ===============================================================================
// class DialogSaveGeneric
// {
//     public:
//                                     DialogSaveGeneric( const Ustring& );
//                                     // DialogSaveGeneric( BaseObjectType*,
//                                     //                    const Glib::RefPtr< Gtk::Builder >& );

//         std::string                 get_filename( const String& = "" );

//         Glib::RefPtr< Gtk::FileDialog > m_Dlg;
// };

struct FileOpenFunction
{
    FileOpenFunction( const FuncVoidString& sf ) : p2func( sf ) {}
    const FuncVoidString& p2func;
    int m;
};

class DialogDiaryFile //: public DialogSaveGeneric
{
	public:
        static void                 open( const Ustring&, const String&, const FuncVoidString& );
        static void                 handle_open_result( Glib::RefPtr< Gio::AsyncResult >&,
                                                        const Glib::RefPtr< Gtk::FileDialog >&,
                                                        const FuncVoidString& );

        static void                 save( const Ustring&, const String&,
                                          const String&, const FuncVoidString& );
        static void                 handle_save_result( Glib::RefPtr< Gio::AsyncResult >&,
                                                        const Glib::RefPtr< Gtk::FileDialog >&,
                                                        const FuncVoidString& );

	protected:
};

// FILE OPEN DIALOG ================================================================================
/*class DialogOpenDiary : public Gtk::FileChooserDialog
{
    public:
                                    DialogOpenDiary();

        // FILTERING
        static void                 init_filters();

        static Glib::RefPtr< Gtk::FileFilter >
                                    filter_any;
        static Glib::RefPtr< Gtk::FileFilter >
                                    filter_diary;

    protected:
        void                        handle_selection_changed();

        static bool                 F_filters_created;
};*/

// EXPORTERS =======================================================================================
class Diarysaver
{
    public:
        virtual Gtk::Widget*        draw_options()  { return nullptr; }
        virtual bool                is_ready()      { return true; }
        virtual void                save() = 0;

        const Ustring               m_name;
        const Filter*               m_p2filter      { nullptr };
        SignalVoid                  m_Sg_updated;

    protected:
                                    Diarysaver( const Ustring&, Diary* );
        virtual                     ~Diarysaver() {}

        Diary*                      m_p2diary;
};

class ExporterLifeograph : public Diarysaver
{
    public:
        explicit                    ExporterLifeograph( Diary* );

        Gtk::Widget*                draw_options() override;
        bool                        is_ready() override;
        void                        save() override;
        static void                 save2( const String&, const String&, const Filter*, Diary* );

    private:
        Gtk::Grid*                  m_G_contents;
        EntryClear*                 m_E_password_1;
        EntryClear*                 m_E_password_2;

};

class ExporterText : public Diarysaver
{
    public:
        explicit                    ExporterText( Diary* );

        void                        save() override;
};

// EXPORT DIALOG ===================================================================================
class DialogExport : public Gtk::Window //DialogEvent
{
    public:
                                    DialogExport( BaseObjectType*,
                                                  const Glib::RefPtr< Gtk::Builder >& );

        void                        add_saver( Diarysaver* );

    protected:
        void                        on_show() override;
        void                        handle_go();

        void                        handle_save_type_changed();
        void                        handle_readiness_changed();

        WidgetFilterPicker*         m_WFP_contents;
        Gtk::ComboBoxText*          m_CB_type;
        Gtk::Button*                m_B_export;
        Gtk::Box*                   m_Bx_contents;
        Gtk::Widget*                m_p2W_options;

        std::vector< Diarysaver* >  m_savers;
        Diarysaver*                 m_saver_cur;

        bool                        m_F_saver_ready;

    private:

};

} // end of namespace LIFEO

#endif
