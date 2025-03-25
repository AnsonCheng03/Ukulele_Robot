import subprocess
from signal import pause
import dbus
import dbus.exceptions
import dbus.mainloop.glib
import dbus.service

import array
try:
  from gi.repository import GObject
except ImportError:
  import gobject as GObject
import bt_gatt.advertising as advertising
import bt_gatt.gatt_server as gatt_server
import argparse


# Run setup commands before starting the server and ensure they complete successfully
def run_setup_command(command):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running command {' '.join(command)}: {result.stderr}")
        exit(1)
        
def unpair_all_devices():
  result = subprocess.run(["/usr/bin/btmgmt", "find"], stdout=subprocess.PIPE, text=True)
  for line in result.stdout.splitlines():
      if "dev" in line and "rpa" in line:  # or just check for MAC-like pattern
          parts = line.split()
          for part in parts:
              if ":" in part:  # likely a MAC address
                  run_setup_command(["/usr/bin/btmgmt", "unpair", part])


def start_bluetooth_server():
  run_setup_command(["/usr/bin/btmgmt", "power", "on"])
  run_setup_command(["/usr/bin/btmgmt", "connectable", "on"])
  run_setup_command(["/usr/bin/btmgmt", "pairable", "on"])
  run_setup_command(["/usr/bin/btmgmt", "discov", "on"])
  run_setup_command(["/usr/bin/btmgmt", "io-cap", "3"])
  run_setup_command(["/usr/bin/btmgmt", "le", "on"])
  run_setup_command(["/usr/bin/btmgmt", "bredr", "off"])
  run_setup_command(["/usr/bin/btmgmt", "advertising", "on"])
  run_setup_command(["/usr/bin/btmgmt", "name", "Guitar Robot"])
  unpair_all_devices()
  print("Setup commands completed successfully")


  # run_setup_command(["/usr/bin/hciconfig", "hci0", "sspmode", "disable"])

  parser = argparse.ArgumentParser()
  parser.add_argument('-a', '--adapter-name', type=str, help='Adapter name', default='')
  args = parser.parse_args()
  adapter_name = args.adapter_name

  dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
  bus = dbus.SystemBus()
  mainloop = GObject.MainLoop()

  advertising.advertising_main(mainloop, bus, adapter_name)
  gatt_server.gatt_server_main(mainloop, bus, adapter_name)
  mainloop.run()