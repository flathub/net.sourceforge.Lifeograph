#!/usr/bin/env python3

import os
import subprocess
import sys

datadir = sys.argv[1]
install_prefix = os.environ['MESON_INSTALL_PREFIX']
icondir = os.path.join(install_prefix, datadir, 'icons', 'hicolor')
mimedatabasedir = os.path.join(install_prefix, datadir, 'mime')


if not os.environ.get('DESTDIR'):
  print('Update icon cache...')
  subprocess.call(['gtk-update-icon-cache', '-f', '-t', icondir])
  print('Update MIME database...')
  subprocess.call(['update-mime-database', mimedatabasedir])
