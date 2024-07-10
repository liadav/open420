
import asyncio
import argparse
from bleak import BleakScanner, BleakClient
from bleak.exc import BleakError

SERVICE_UUID_QVAP = "00000000-5354-4f52-5a26-4249434b454c"
SERVICE_UUID_QVAP1 = "00001800-0000-1000-8000-00805f9b34fb"
CHARACTERISTIC_UUID = "00000001-5354-4f52-5a26-4249434b454c"
CHARACTERISTIC_UUID_SERIAL = "00002a00-0000-1000-8000-00805f9b34fb"

COMMANDS = {
    "INIT_CONNECTION": 0x02,
    "SETUP_INITIAL_PARAMS": 0x01,
    "PREPARE_DEVICE": 0x04,
    "ADDITIONAL_SETUP": 0x05,
    "FINAL_SETUP": 0x06,
    "BOOTLOADER_UPDATE": 0x30
}

APPLICATION_FIRMWARE_MASK_APPLICATION = 1
APPLICATION_FIRMWARE_MASK_INVALID_APPLICATION = 16
APPLICATION_FIRMWARE_MASK_INVALID_BOOTLOADER = 32

BIT_SETTINGS_UNIT = 1 << 0
BIT_SETTINGS_SETPOINT_REACHED = 1 << 1
BIT_SETTINGS_FACTORY_RESET = 1 << 2
BIT_SETTINGS_ECOMODE_CHARGE = 1 << 3
BIT_SETTINGS_BUTTON_CHANGED_FILLING_CHAMBER = 1 << 4
BIT_SETTINGS_ECOMODE_VOLTAGE = 1 << 5
BIT_SETTINGS_BOOST_VISUALIZATION = 1 << 6

class QVAPDevice:
    def __init__(self, address, service_uuid, service_uuid1, characteristic_uuid, characteristic_uuid_serial):
        self.address = address
        self.service_uuid = service_uuid
        self.service_uuid1 = service_uuid1
        self.characteristic_uuid = characteristic_uuid
        self.characteristic_uuid_serial = characteristic_uuid_serial
        self.client = None
        self.notifications_enabled = False
        self.primary_service = None
        self.serial_service = None
        self.characteristic = None
        self.serial_characteristic = None
        self.application_firmware = 0
        
        # Parameters to store
        self.current_temp = None
        self.set_temp = None
        self.boost_temp = None
        self.superboost_temp = None
        self.battery_level = None
        self.auto_shutoff = None
        self.heater_mode = None
        self.charger_status = None
        self.settings_flags = None

    async def connect(self):
        async with BleakClient(self.address) as client:
            self.client = client
            await self.client.connect()
            await self.discover_services()
            await self.enable_notifications()

            await self.init_connection()
            await asyncio.sleep(0.5)

            await self.setup_initial_params()
            await asyncio.sleep(0.5)
            await self.prepare_device()
            await asyncio.sleep(0.5)
            await self.additional_setup()
            await asyncio.sleep(0.5)
            await self.final_setup()
            await asyncio.sleep(0.5)

            if self.serial_service:
                await self.read_serial_number()

            await self.final_stage()
            await asyncio.sleep(0.5)
            await self.start_periodic_updates()

    async def discover_services(self):
        services = await self.client.get_services()
        self.primary_service = services.get_service(self.service_uuid)
        self.serial_service = services.get_service(self.service_uuid1) if self.service_uuid1 in services else None
        self.characteristic = self.primary_service.get_characteristic(self.characteristic_uuid)
        if self.serial_service:
            self.serial_characteristic = self.serial_service.get_characteristic(self.characteristic_uuid_serial)

    async def enable_notifications(self):
        await self.client.start_notify(self.characteristic, self.notification_handler)
        self.notifications_enabled = True

    async def init_connection(self):
        await self.send_command(COMMANDS["INIT_CONNECTION"])

    async def setup_initial_params(self):
        await self.send_command(COMMANDS["SETUP_INITIAL_PARAMS"])

    async def prepare_device(self):
        await self.send_command(COMMANDS["PREPARE_DEVICE"])

    async def additional_setup(self):
        await self.send_command(COMMANDS["ADDITIONAL_SETUP"])

    async def final_setup(self):
        await self.send_command(COMMANDS["FINAL_SETUP"])

    async def read_serial_number(self):
        if "read" in self.serial_characteristic.properties:
            serial_number = await self.client.read_gatt_char(self.serial_characteristic)
            print("Serial Number:", serial_number.decode("utf-8"))
        else:
            print("Serial number characteristic does not support reading.")

    async def final_stage(self):
        buffer = bytearray(20)
        buffer[1] = 6
        if self.application_firmware & APPLICATION_FIRMWARE_MASK_APPLICATION:
            print("Firmware application valid. Starting periodic updates.")
            buffer[0] = 0x30  # BOOTLOADER_UPDATE
        else:
            buffer[0] = 0x01  # SETUP_INITIAL_PARAMS
        await self.client.write_gatt_char(self.characteristic, buffer)

    async def start_periodic_updates(self):
        while self.notifications_enabled:
            await self.send_command(COMMANDS["PREPARE_DEVICE"])
            await asyncio.sleep(4)  # Periodic interval
            await self.send_command(COMMANDS["SETUP_INITIAL_PARAMS"], [0])
            await asyncio.sleep(4)  # Periodic interval

    async def send_command(self, command, data=[]):
        buffer = bytearray([command] + data + [0] * (20 - len(data) - 1))
        await self.client.write_gatt_char(self.characteristic, buffer)

    async def send_periodic_command(self):
        await self.send_command(COMMANDS["PREPARE_DEVICE"])

    def notification_handler(self, sender, data):
        print(f"Notification from {sender}: {data}")
        asyncio.create_task(self.parse_response(data))

    async def parse_response(self, response):
        command = response[0]
        if command == 2:  # Command that updates the application firmware status
            self.application_firmware = response[1]
            firmware_version = response[2:8].decode("utf-8")
            bootloader_version = response[11:17].decode("utf-8")
            print(f"Application Firmware: {firmware_version}, Bootloader: {bootloader_version}")
            # Handle further firmware version comparison and update
        elif command in {1, 48}:
            if self.application_firmware & APPLICATION_FIRMWARE_MASK_APPLICATION and command != 48:
                self.current_temp = (response[3] << 8) + response[2]
                self.set_temp = (response[5] << 8) + response[4]
                self.boost_temp = response[6]
                self.superboost_temp = response[7]
                self.battery_level = response[8]
                self.auto_shutoff = (response[10] << 8) + response[9]
                self.heater_mode = response[11]
                self.charger_status = response[13]
                self.settings_flags = response[14]

                print(f"Current Temp: {self.current_temp}, Set Temp: {self.set_temp}, Boost Temp: {self.boost_temp}, Superboost Temp: {self.superboost_temp}, Battery: {self.battery_level}, Auto-shutoff: {self.auto_shutoff}, Heater Mode: {self.heater_mode}, Charger: {self.charger_status}, Settings: {self.settings_flags}")

            else:
                print("_______cmd0x01 - bootloader")
                # Handle bootloader update steps
                status = response[1]
                if status == 1:
                    self.data_idx_qvap += 1
                    if self.data_idx_qvap >= self.page_size_qvap / self.data_size_firmware_per_packet:
                        print("page write start request")
                    else:
                        print("page data write request")
                    await self.write_page_data_sequence_qvap()
                elif status == 2:
                    self.page_idx_qvap += 1
                    self.data_idx_qvap = 0
                    print(f"page {self.page_idx_qvap} write done")
                    if self.page_idx_qvap < self.binary_number_of_pages:
                        await self.write_page_data_sequence_qvap()
                    else:
                        await asyncio.sleep(0.2)
                        await self.start_qvap_application(True)
                elif status == 3:
                    pass
                elif status == 4:
                    pass
                elif status == 5:
                    pass
                elif status == 6:
                    connection_interval = response[2]
                    print(f"   connection interval {connection_interval}")
                    if connection_interval < 25:
                        print("BLE connection interval too small; Firmware upgrade will fail")
                elif status == 8:
                    print("  decrypt data done")
                    buffer = bytearray(3)
                    buffer[0] = 0x30 if self.update_bootloader else 0x01
                    buffer[1] = 2
                    buffer[2] = self.page_idx_qvap
                    self.timeout_ble_response_qvap = asyncio.create_task(self.handle_timeout_ble_response_missing_qvap(0.75))
                    await self.client.write_gatt_char(self.characteristic, buffer)
                elif status == 34:
                    print("Erase failed: please reload and retry.\n")
                elif status == 19:
                    print("Validation failed: please reload and retry.\n")
                    if self.nb_retries_validation < 1:
                        await self.start_qvap_application(True)
                        self.nb_retries_validation += 1
                elif status == 51:
                    print("Validation failed (mode): please reload and retry.\n")
                elif status == 35:
                    print("Validation failed: please reload and retry.\n")
                elif status == 82:
                    print("VersionMajor failed: please reload and retry.\n")
                elif status == 98:
                    print("VersionMinor failed: please reload and retry.\n")
                else:
                    print(f"   undefined status {status} - {response[2]}")
        elif command == 3:
            err_code = response[1]
            err_category = response[2]
            if err_category == 4:
                print("Start analysis")
            else:
                print("Device analysis successful")
                if response[3] < 9:
                    print("Display brightness reduced")
                if response[4] & 1:
                    print("Charge limit activated")
                if not response[5] & 1:
                    print("Boost visualization disabled")
                if response[6] & 1:
                    print("Charge optimization activated")
                if not response[7] & 1:
                    print("Vibration disabled")
        elif command == 4:
            if len(response) >= 20:
                heater_runtime_minutes = int.from_bytes(response[1:4], byteorder='little')
                battery_charging_time_minutes = int.from_bytes(response[4:7], byteorder='little')
                print(f"Heater Runtime: {heater_runtime_minutes} minutes, Battery Charging Time: {battery_charging_time_minutes} minutes")
        elif command == 5:
            if len(response) >= 17:
                device_prefix = response[15:17].decode("utf-8")
                device_name = response[9:15].decode("utf-8")
                print(f"Device Prefix: {device_prefix}, Device Name: {device_name}")
        elif command == 6:
            if len(response) >= 5:
                brightness = response[2]
                vibration = response[5]
                print(f"Brightness: {brightness}, Vibration: {'Enabled' if vibration else 'Disabled'}")
        else:
            print(f"Default command {command}")

    async def write_page_data_sequence_qvap(self):
        # Implement the write page data sequence logic here
        pass

    async def start_qvap_application(self, repeat):
        # Implement the start QVAP application logic here
        pass

    async def handle_timeout_ble_response_missing_qvap(self, timeout):
        # Implement the timeout handler logic here
        pass

async def discover_devices():
    devices = await BleakScanner.discover()
    print("Found the following devices:")
    for i, device in enumerate(devices):
        print(f"{i}: {device.name} - {device.address}")
    return devices

async def main(address=None):
    if address is None:
        devices = await discover_devices()
        if not devices:
            print("No devices found. Exiting.")
            return

        index = int(input("Select the device you want to connect to by entering its index: "))
        if index < 0 or index >= len(devices):
            print("Invalid index. Exiting.")
            return

        selected_device = devices[index]
        address = selected_device.address
        print(f"Connecting to {selected_device.name} at {selected_device.address}")

    qvap = QVAPDevice(address, SERVICE_UUID_QVAP, SERVICE_UUID_QVAP1, CHARACTERISTIC_UUID, CHARACTERISTIC_UUID_SERIAL)
    await qvap.connect()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="QVAP Device Connection Script")
    parser.add_argument('--address', type=str, help='Manually enter the device address')
    args = parser.parse_args()

    asyncio.run(main(args.address))
