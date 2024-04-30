import requests
import json


def get_firmware(device, serial, fw_type):
    data = {"device": device,
            "action": fw_type,
            "serial": serial}

    req = requests.post("https://app.storz-bickel.com/firmware", data=data)
    return req.text

def get_firmware_bootloader(device, serial):
    return get_firmware(device, serial, "firmwareBootloader")

def get_firmware_application(device, serial):
    return get_firmware(device, serial, "firmwareApplication")


def main():
    fw = get_firmware_bootloader("Venty", "VY4DT3MR")
    fw_json = json.loads(fw)[0]
    assert fw_json["valid"] == 1
    with open("firmware.bin", "wb") as f:
        f.write(bytes.fromhex(fw_json['firmware']))

if __name__ == '__main__':
    main()

