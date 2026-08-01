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

#include <gtkmm/application.h>

#include "lifeograph.hpp"


int
main( int argc, char* argv[] )
{
    using namespace LIFEO;

    // INIT PATHS
#ifdef _WIN32
    const String bindir { HELPERS::get_exec_path() };
    Lifeograph::BINPATH     = bindir + "\\bin\\lifeograph.exe";
    Lifeograph::SHAREDIR    = bindir + "\\share\\" + PACKAGE;
    Lifeograph::MANUALPATH  = Lifeograph::SHAREDIR + "\\diaries\\Lifeograph Manual";
    Lifeograph::EXAMPLEDIARYPATH = Lifeograph::SHAREDIR + "\\diaries\\example.diary";
    Lifeograph::LOCALEDIR   = bindir + "\\share\\locale";

    PRINT_DEBUG( "SHAREDIR: ", Lifeograph::SHAREDIR );
#else
    Lifeograph::BINPATH          = argv[ 0 ];
    Lifeograph::SHAREDIR         = DEF_SHAREDIR;
    Lifeograph::MANUALPATH       = DEF_MANUALPATH;
    Lifeograph::EXAMPLEDIARYPATH = DEF_EXAMPLEDIARYPATH;
    Lifeograph::LOCALEDIR        = DEF_LOCALEDIR;
#endif

    // LIBGETTEXT REQUIREMENTS
    setlocale( LC_ALL, "" );
    bindtextdomain( PACKAGE, Lifeograph::LOCALEDIR.c_str() );
#ifdef _WIN32
    bind_textdomain_codeset( PACKAGE, "UTF-8" );
#endif
    textdomain( PACKAGE );

    // APPLICATION
    Glib::RefPtr< LIFEO::Lifeograph > app = Lifeograph::create();

    return app->run( argc, argv );
}
