from machine import Timer, SoftI2C
from time import sleep_ms
import ubluetooth

class BLE():
    def __init__(self):   
        self.name = "ESP32"
        self.ble = ubluetooth.BLE()
        self.ble.active(True)

        self.timer1 = Timer(0)
        self.timer2 = Timer(1)
        
        self.disconnected()
        self.ble.irq(self.ble_irq)
        self.register()
        self.advertiser()

    def connected(self):        
        self.timer1.deinit()
        self.timer2.deinit()
        print("Device connected")

    def disconnected(self):
        print("Device disconnected")

    def ble_irq(self, event, data):
        if event == 1:
            '''Central connected'''
            self.connected()
            print("Central connected")
        
        elif event == 2:
            '''Central disconnected'''
            self.advertiser()
            self.disconnected()
            print("Central disconnected")
        
        elif event == 3:
            '''New message received'''            
            buffer = self.ble.gatts_read(self.rx)
            message = buffer.decode('UTF-8').strip()
            print("Received message:", message)            
            if message == 'red_led':
                print('Toggling red_led')
                print('red_led state changed')
                ble.send('red_led' + str(1))  # Placeholder value for state
            if message == 'read_temp':
                print("Reading temperature")
                print(25.0)  # Placeholder for temperature reading
                ble.send(str(25.0))
            if message == 'read_hum':
                print("Reading humidity")
                print(50.0)  # Placeholder for humidity reading
                ble.send(str(50.0))
           
    def register(self):        
        # Nordic UART Service (NUS)
        NUS_UUID = '6E400001-B5A3-F393-E0A9-E50E24DCCA9E'
        RX_UUID = '6E400002-B5A3-F393-E0A9-E50E24DCCA9E'
        TX_UUID = '6E400003-B5A3-F393-E0A9-E50E24DCCA9E'
            
        BLE_NUS = ubluetooth.UUID(NUS_UUID)
        BLE_RX = (ubluetooth.UUID(RX_UUID), ubluetooth.FLAG_WRITE)
        BLE_TX = (ubluetooth.UUID(TX_UUID), ubluetooth.FLAG_NOTIFY)
            
        BLE_UART = (BLE_NUS, (BLE_TX, BLE_RX,))
        SERVICES = (BLE_UART, )
        ((self.tx, self.rx,), ) = self.ble.gatts_register_services(SERVICES)

    def send(self, data):
        self.ble.gatts_notify(0, self.tx, data + '\n')
        print("Sent data:", data)

    def advertiser(self):
        name = self.name.encode('UTF-8')  # Ensure name is properly encoded
        self.ble.gap_advertise(100, bytearray(b'\x02\x01\x02') + bytearray((len(name) + 1, 0x09)) + name)
        print("Advertising as", self.name)

# test
ble = BLE()
