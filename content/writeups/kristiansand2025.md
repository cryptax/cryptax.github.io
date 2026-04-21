---
layout: post
title: "BSides Kristiansand Badge CTF 2025"
author: cryptax
date: 2025-06-07
tags:
- BSides
- Kristiansand
- Norway
- CTF
- 2025
- RP2040
- Micropython
---

## Description

We are given a hardware badge with no particular instruction, but we know there are CTF challenges on it.

## Investigation

The front of the badge has Leds, no screen.
The back of the badge has a USB connector, a few exposed pins, a battery holder, and underneath a push button. Also, Morse code for numbers is shown on the back.

When we power on the badge, it blinks, and then nothing happens.

## Connecting the USB port

```
$ lsusb
...
Bus 003 Device 014: ID 2e8a:0005 MicroPython Board in FS mode
$ dmesg
...
[249846.706904] usb 3-3: new full-speed USB device number 14 using xhci_hcd
[249847.009429] usb 3-3: New USB device found, idVendor=2e8a, idProduct=0005, bcdDevice= 1.00
[249847.009438] usb 3-3: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[249847.009441] usb 3-3: Product: Board in FS mode
[249847.009443] usb 3-3: Manufacturer: MicroPython
[249847.009445] usb 3-3: SerialNumber: e6614103e7175d36
[249847.079980] cdc_acm 3-3:1.0: ttyACM0: USB ACM device

```

We connect to the serial: `picocom /dev/ttyACM0`. If we connect it sufficiently quickly at boot, we see the following messages:

![](/images/kristiansand25-connect-badge.png)


So, the device creates a WiFi AP, `DUCK-FF6FA7` in the case of my device, and the password is `00098552`. The device also probably initializes a web server (port 80) on 192.168.4.1 (one of the IP addresses listed on the message).

We can connect to that SSID, and head to the embedded website http://192.168.4.1

## Website
 
We are greeted with a CTF website. The welcome page gives us an Easy Flag, `DUCK_xxx`, that we capture on the flags page.

![](/images/kristiansand25-welcomepage.png)

There are several other flags to get: comms, credits etc.
I hack around a little on the comms page and see that pressing the button actually triggers an action on `/trigger_interface`. The credits page doesn't give me more valuable information.

```
Starting async server on 0.0.0.0:80...
GET / 200
PATH: duckLogo.png
GET /style.css 200
GET /static/duckLogo.png 200
GET /favicon.ico 404
GET /comms 200
PATH: duckLogo.png
GET /style.css 200
GET /static/duckLogo.png 200
GET /trigger_interface 200
```

## MicroPython REPL

As this is **RP2040**, I want to download the firmware. I try to with [`picotool`](https://github.com/raspberrypi/pico-sdk), but it tells me the chip is not in DFU mode, that I am on the *MicroPython* firmware.

So, easier, I decide to connect to the MicroPython REPL

- `picocom /dev/ttyACM0`
- then hit enter until you see the MicroPython prompt `>>>`

```
MicroPython v1.23.0 on 2024-06-02; Raspberry Pi Pico W with RP2040
Type "help()" for more information.
>>> 
```

I list files onboard:

```
>>> os.listdir()
['.micropico', '.vscode', 'apConfig.json', 'config.json', 'configManager.py', 'constants.py', 'contributors.json', 'encrypted_db.json', 'flagManager.py', 'helpers.py', 'images', 'js', 'ledManager.py', 'main.py', 'microdot', 'neopixel.py', 'networkManager.py', 'scanManager.py', 'serialManager.py', 'static', 'templates', 'tinydb', 'utemplate']

```


I decide to download all those files with [mpremote](https://pypi.org/project/mpremote/)

`mpremote connect /dev/ttyACM0 fs cp :main.py`

It works great, so I script it in Bash to download all the other files.

```bash
for i in apConfig.json config.json configManager.py constants.py contributors.json encrypted_db.json flagManager.py helpers.py ledManager.py networkManager.py scanManager.py serialManager.py; do  mpremote connect /dev/ttyACM0 fs cp :$i .; done
```

## Reading files

The `main.py` implements the website's routes. I am particularly interested in the file `flagManager.py`. Apparently, there is an encrypted database held in a JSON file. The encryption is AES ECB, and the key is hard coded.

```python
class FlagManager:
    def __init__(self, device_id):
        dbName = 'encrypted_db.json'
        encryption_key = b'donothardcodekey'
        self.encryption_cipher = ucryptolib.aes(encryption_key, 1)
        self.decryption_cipher = ucryptolib.aes(encryption_key, 1)
        self.device_name = device_id.replace("-", "")        
        self.db = None
        self.table = None
```

The entire JSON file is not encrypted, only some fields:

```json
{"flags": {"5": {"flag": "authorized", "data": "4f9889cd3a7dcf987e714abb980fbef
0f5c3509e463abb7fae95923ca45b8e61", "status": false}, "4": {"data": "572c732774
9a70ec0836fc40115fe7dfeaaa2ab9b981978a7f14ca4dc960289f", "flag": "firmware", "s
tatus": false}, "7": {"flag": "easy", "data": "e2e9ad760cacc78b00ea628732c052f8
", "status": true}, "6": {"data": 
...
```

In `flagManager`, the data of the flag simply needs to be un-hexlified:

```python
def decrypt(self, data):
        try:
            encrypted_bytes = ubinascii.unhexlify(data.encode())
            decrypted_data = self.decryption_cipher.decrypt(encrypted_bytes)
            return decrypted_data.rstrip(b'\x00').decode('utf-8')
```

So, it seems we can simply grab all flags by decrypting the JSON file. It's just a matter of converting micropython to python.

## Decrypt the database

The decryption is simple:

1. Unhexlify the string to convert it to bytes
2. Perform AES decryption. It's ECB mode.
3. Convert bytes to a string

```python
import json
from Crypto.Cipher import AES
import binascii

class DatabaseDecryptor:
    def __init__(self):
        self.encryption_key = b'donothardcodekey'
        
    def decrypt(self, hex_data):
        try:
            encrypted_bytes = binascii.unhexlify(hex_data)
            cipher = AES.new(self.encryption_key, AES.MODE_ECB)
            decrypted_data = cipher.decrypt(encrypted_bytes)
            # Remove null byte padding and decode to string
            return decrypted_data.rstrip(b'\x00').decode('utf-8')
        except Exception as e:
            return f"Decryption Error: {e}"
```

The rest consists in parsing the JSON and decrypting the encrypted fields:

```python
def main():
    # alternatively we could have read it from the file
    encrypted_db = {
        "flags": {
            "5": {"flag": "authorized", "data": "4f9889cd3a7dcf987e714abb980fbef0f5c3509e463abb7fae95923ca45b8e61", "status": False},
            "4": {"data": "572c7327749a70ec0836fc40115fe7dfeaaa2ab9b981978a7f14ca4dc960289f", "flag": "firmware", "status": False},
            "7": {"flag": "easy", "data": "e2e9ad760cacc78b00ea628732c052f8", "status": True},
            "6": {"data": "9adc46ef684b25d82d25e9d156734017cc2729855edd106b75d4e2103121f0dc", "flag": "respond", "status": False},
            "1": {"flag": "credits", "data": "e3134ab9c6617a8bca48b875f4c7cb80d85caf6f7d70a18890052315d9e668ef", "status": False},
            "3": {"flag": "secured", "data": "b19d92b7bf524794d07909c849604b2d796996288e58303d742247a4cc62a1a3", "status": False},
            "2": {"data": "9bf8741004fd4c065e42e9952e0aa6789a511d602ec5dc6a7732f1d90350baec", "flag": "comms", "status": False}
        }
    }
    
    decryptor = DatabaseDecryptor()
    
    print("Decrypting database entries:\n")
    print("=" * 60)
    
    # it's the flag entries we need to decrypt
    for entry_id, entry in encrypted_db["flags"].items():
        flag_name = entry["flag"]
        encrypted_data = entry["data"]
        status = entry["status"]
        decrypted_flag = decryptor.decrypt(encrypted_data)
        print(f"Flag: {flag_name}")
        print(f"Status: {'Captured' if status else 'Not Captured'}")
        print(f"Encrypted: {encrypted_data}")
        print(f"Decrypted: {decrypted_flag}")
        print("-" * 60)

if __name__ == "__main__":
    main()
```

## Solution

We run the program, and get all our flags.

```
$ python3 decryptit.py 
Decrypting database entries:

============================================================
Flag: authorized
Status: Not Captured
Encrypted: 4f9889cd3a7dcf987e714abb980fbef0f5c3509e463abb7fae95923ca45b8e61
Decrypted: DUCK_FF6FA7_4U7H0RI23D_5_77EE0
------------------------------------------------------------
Flag: firmware
Status: Not Captured
Encrypted: 572c7327749a70ec0836fc40115fe7dfeaaa2ab9b981978a7f14ca4dc960289f
Decrypted: DUCK_FIRMW4R3_4_56789
------------------------------------------------------------
Flag: easy
Status: Captured
Encrypted: e2e9ad760cacc78b00ea628732c052f8
Decrypted: DUCK_345Y_7_4612
------------------------------------------------------------
Flag: respond
Status: Not Captured
Encrypted: 9adc46ef684b25d82d25e9d156734017cc2729855edd106b75d4e2103121f0dc
Decrypted: DUCK_FF6FA7_R35P0ND_6_8493E
------------------------------------------------------------
Flag: credits
Status: Not Captured
Encrypted: e3134ab9c6617a8bca48b875f4c7cb80d85caf6f7d70a18890052315d9e668ef
Decrypted: DUCK_CR3DI75_1_01234
------------------------------------------------------------
Flag: secured
Status: Not Captured
Encrypted: b19d92b7bf524794d07909c849604b2d796996288e58303d742247a4cc62a1a3
Decrypted: DUCK_FF6FA7_53CUR3D_3_21A55
------------------------------------------------------------
Flag: comms
Status: Not Captured
Encrypted: 9bf8741004fd4c065e42e9952e0aa6789a511d602ec5dc6a7732f1d90350baec
Decrypted: DUCK_FF6FA7_C0MM5_2_1A5BF
------------------------------------------------------------
```

Note that this method is obviously faster than solving each challenge individually, but probably not as fun. I solved each of them in one step without even knowing what I was meant to do. 

I enter all these flags on the web interface, and get the deserved flags :)

![](/images/kristiansand25-captured-flags.png)
