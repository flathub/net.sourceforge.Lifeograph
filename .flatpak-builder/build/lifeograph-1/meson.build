#    Copyright (C) 2013-2024 Ahmet Öztürk (aoz_2@yahoo.com)
#
#    This file is part of Lifeograph.
#
#    Lifeograph is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    Lifeograph is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with Lifeograph.  If not, see <http://www.gnu.org/licenses/>.

# PROJECT =====================================================================
project( 'net.sourceforge.Lifeograph', 'cpp', license : 'GPL3', version : '3.0',
         default_options : ['cpp_std=c++17'] ) # can DHAVECONFIG be added here?

compiler = meson.get_compiler( 'cpp' )

PACKAGE_NAME = meson.project_name()

# DEPENDENCIES ================================================================
dep_gtkmm = dependency( 'gtkmm-4.0', version : '>=4.10' )
dep_enchant = dependency( 'enchant-2' )
dep_shumate = dependency( 'shumate-1.0' )
dep_gcrypt = dependency( 'libgcrypt', required : false )
# below is taken from: https://github.com/GNOME/frogr/blob/master/meson.build
if not dep_gcrypt.found()
    message( 'gcrypt not found via pkg-config, using meson.find_library()...' )
    dep_gcrypt = compiler.find_library( 'gcrypt' )
endif
dep_threads = dependency( 'threads' )

dependency_list = [ dep_gtkmm, dep_enchant, dep_shumate, dep_gcrypt, dep_threads ]

# CONFIGURATION ===============================================================
INSTALL_PREFIX = get_option( 'prefix' ) #'/usr'
DEBUG_BUILD = get_option( 'buildtype' ).startswith( 'debug' ) ? 1 : 0
RUN_INSTALL_SCRIPT = get_option( 'run_install_script' )

# TODO: below can be simplified with meson
conf_data = configuration_data()
conf_data.set('PACKAGE_NAME', PACKAGE_NAME)
conf_data.set('SHAREDIR', INSTALL_PREFIX + '/share/' + PACKAGE_NAME)
conf_data.set('MANUALPATH', INSTALL_PREFIX + '/share/' + PACKAGE_NAME + '/diaries/Lifeograph Manual')
conf_data.set('EXAMPLEDIARYPATH', INSTALL_PREFIX + '/share/' + PACKAGE_NAME + '/diaries/example.diary')
conf_data.set('LOCALEDIR', INSTALL_PREFIX + '/share/locale')
conf_data.set('DEBUG_BUILD', DEBUG_BUILD)

configure_file( input : 'config.h.in',
                output : 'config.h',
                configuration : conf_data )
configuration_inc = include_directories('.')

add_global_arguments( '-DHAVE_CONFIG_H', language : 'cpp' )
add_global_arguments( '-DGDK_DISABLE_DEPRECATED', language : 'cpp' )
add_global_arguments( '-DGTK_DISABLE_DEPRECATED', language : 'cpp' )

# BUILD TIME STAMP ============================================================
run_command( 'create_time_build_time_header.sh',
             'lifeograph', '../src', meson.current_build_dir(), check : true )

# LOCALIZATION ================================================================
add_project_arguments( '-DGETTEXT_PACKAGE=' + PACKAGE_NAME, language: 'cpp')

subdir('po')

if build_machine.system() != 'windows'
  i18n.merge_file(
    input : 'net.sourceforge.Lifeograph.desktop.in',
    output : 'net.sourceforge.Lifeograph.desktop',
    type : 'desktop',
    po_dir : 'po',
    install : true,
    install_dir : join_paths( get_option( 'datadir' ), 'applications' )
  )

  i18n.merge_file(
    input : 'net.sourceforge.Lifeograph.metainfo.xml.in',
    output : 'net.sourceforge.Lifeograph.metainfo.xml',
    po_dir : 'po',
    install : true,
    install_dir : join_paths( get_option( 'datadir' ), 'metainfo' )
  )

  i18n.merge_file(
    input : 'x-lifeographdiary.xml.in',
    output : 'x-lifeographdiary.xml',
    po_dir : 'po',
    data_dirs: 'po/gettext',
    install : true,
    install_dir : join_paths( get_option( 'datadir' ), 'mime', 'packages' )
  )
endif

# SOURCES =====================================================================
sources = [
    'src/app_window.cpp',
    'src/diaryelements/diary.cpp',
    'src/diaryelements/diarydata.cpp',
    'src/diaryelements/entry.cpp',
    'src/diaryelements/filter.cpp',
    'src/diaryelements/paragraph.cpp',
    'src/diaryelements/tableelem.cpp',
    'src/dialogs/dialog_export.cpp',
    'src/dialogs/dialog_filter.cpp',
    'src/dialogs/dialog_gettext.cpp',
    'src/dialogs/dialog_password.cpp',
    'src/dialogs/dialog_preferences.cpp',
    'src/dialogs/dialog_smartadd.cpp',
    'src/dialogs/dialog_sync.cpp',
    'src/helpers.cpp',
    'src/lifeograph.cpp',
    'src/main.cpp',
    'src/parsers/parser_background.cpp',
    'src/parsers/parser_base.cpp',
    'src/parsers/parser_stripper.cpp',
    'src/parsers/parser_upgrader.cpp',
    'src/printing.cpp',
    'src/settings.cpp',
    'src/ui_diary.cpp',
    'src/ui_entry.cpp',
    'src/ui_extra.cpp',
    'src/ui_login.cpp',
    'src/undo.cpp',
    'src/widgets/calendar.cpp',
    'src/widgets/chart.cpp',
    'src/widgets/chart_surface.cpp',
    'src/widgets/po_entry.cpp',
    'src/widgets/table.cpp',
    'src/widgets/table_surface.cpp',
    'src/widgets/widget_chart.cpp',
    'src/widgets/widget_calendar.cpp',
    'src/widgets/widget_datepicker.cpp',
    'src/widgets/widget_entrylist.cpp',
    'src/widgets/widget_entrypicker.cpp',
    'src/widgets/widget_filter.cpp',
    'src/widgets/widget_filterpicker.cpp',
    'src/widgets/widget_map.cpp',
    'src/widgets/widget_table.cpp',
    'src/widgets/widget_textview.cpp',
    'src/widgets/widget_textviewedit.cpp',
    'src/widgets/widget_textviewpara.cpp',
    'src/widgets/widget_textviewsearch.cpp',
    'src/widgets/widget_textviewtheme.cpp' ]

# EXECUTABLE ==================================================================
if build_machine.system() == 'windows' and DEBUG_BUILD == 0
    executable( 'lifeograph', sources,
                dependencies : dependency_list,
                include_directories : configuration_inc,
                #link_args : '-Wl,-subsystem,windows',
                gui_app : true )
else
    executable( 'lifeograph', sources,
                dependencies : dependency_list,
                include_directories : configuration_inc,
                install : true )
endif

# INSTALL =====================================================================
install_man( 'lifeograph.1' )

#below item should no longer be needed
#install_data( 'icons/lifeograph-48.png', rename : 'lifeograph.png',
#              install_dir : join_paths( get_option( 'datadir' ),
#                                        'pixmaps' ) )

install_data( 'icons/lifeograph-256.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/256x256/apps' ) )
install_data( 'icons/lifeograph-128.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/128x128/apps' ) )
install_data( 'icons/lifeograph-64.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/64x64/apps' ) )
install_data( 'icons/lifeograph-48.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/48x48/apps' ) )
install_data( 'icons/lifeograph-32.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/32x32/apps' ) )
install_data( 'icons/lifeograph-24.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/24x24/apps' ) )
install_data( 'icons/lifeograph-22.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/22x22/apps' ) )
install_data( 'icons/lifeograph-16.png', rename : 'net.sourceforge.Lifeograph.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/16x16/apps' ) )

install_data( 'icons/lifeograph.svg', rename : 'net.sourceforge.Lifeograph.svg',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/scalable/apps' ) )
install_data( 'icons/lifeograph-symbolic.svg', rename : 'net.sourceforge.Lifeograph-symbolic.svg',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/symbolic/apps' ) )

# FILE ICONS
install_data( 'icons/diary-16.png', rename : 'application-x-lifeographdiary.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/16x16/mimetypes' ) )
install_data( 'icons/diary-32.png', rename : 'application-x-lifeographdiary.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/32x32/mimetypes' ) )
install_data( 'icons/diary-48.png', rename : 'application-x-lifeographdiary.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/48x48/mimetypes' ) )
install_data( 'icons/diary-64.png', rename : 'application-x-lifeographdiary.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/64x64/mimetypes' ) )
install_data( 'icons/diary-128.png', rename : 'application-x-lifeographdiary.png',
              install_dir : join_paths( get_option( 'datadir' ),
                                        'icons/hicolor/128x128/mimetypes' ) )

# FILES SPECIAL TO APP:
install_subdir( 'icons/installed',
                strip_directory : true,
                install_dir : join_paths( get_option( 'datadir' ),
                                          meson.project_name(),
                                          'icons' ) )
install_subdir( 'ui',
                install_dir : join_paths( get_option( 'datadir' ),
                                          meson.project_name() ) )
install_subdir( 'diaries',
                install_dir : join_paths( get_option( 'datadir' ),
                                          meson.project_name() ) )
install_subdir( 'backgrounds',
                install_dir : join_paths( get_option( 'datadir' ),
                                          meson.project_name() ) )

if build_machine.system() != 'windows' #and RUN_INSTALL_SCRIPT == true
    meson.add_install_script( 'meson_post_install.py', get_option( 'datadir' ) )
endif

