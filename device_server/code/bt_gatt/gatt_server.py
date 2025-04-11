import dbus
import functools
import bt_gatt.adapters as adapters
from bt_gatt.constants import BLUEZ_SERVICE_NAME, GATT_MANAGER_IFACE, DBUS_OM_IFACE
from bt_gatt.motor_service import MotorService
from bt_gatt.application import Application

def register_app_cb():
    print('GATT application registered')

def register_app_error_cb(mainloop, error):
    print('Failed to register application: ' + str(error))
    mainloop.quit()

def gatt_server_main(mainloop, bus, adapter_name):
    adapter = adapters.find_adapter(bus, GATT_MANAGER_IFACE, adapter_name)
    if not adapter:
        raise Exception('GattManager1 interface not found')

    service_manager = dbus.Interface(
            bus.get_object(BLUEZ_SERVICE_NAME, adapter),
            GATT_MANAGER_IFACE)

    app = Application(bus)

    service_manager.RegisterApplication(app.get_path(), {},
                                    reply_handler=register_app_cb,
                                    error_handler=functools.partial(register_app_error_cb, mainloop))
