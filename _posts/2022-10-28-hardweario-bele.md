---
layout: post
title: "BeLE - Hardwear.io 2022"
author: cryptax
tags:
- Hardwear.io
- CTF
- 2022
- BLE
---

# BeLE - Hardwear.io CTF 2022

The challenge is named "BeLE" and says there are several BLE devices + provides a Bluetooth PCAP file.

## PCAP file

The PCAP file is small with only 5 packets.

![PCAP file provided in the description](/images/bele-2022-pcap.png)

1. The first packet is a **BLE Read Request** to read the BLE characteristic with handle 0x2a

`02 10 00 07 00 03 00 04 00 0a 2a 00`

2. The second packet is a **BLE Read Response**. The characteristic has value `f3 dd 42 04 de ed c3 fc`

```
0000   02 10 20 0d 00 09 00 04 00 0b f3 dd 42 04 de ed
0010   c3 fc
```

3. The third packet is a **BLE Write Command** on handle 0x48. We wrote value `05 55 d7 37 b7 ad 8a 17 a1 a8 b1 be c4 70 f9 fe`

```
0000   02 10 00 17 00 13 00 04 00 52 48 00 05 55 d7 37
0010   b7 ad 8a 17 a1 a8 b1 be c4 70 f9 fe
```

4. The fourth packet is a BLE Read Request to read handle 0x39

`02 10 00 07 00 03 00 04 00 0a 39 00`

5. The fifth packet is the BLE Read Response. It apparently contained the flag, but was redacted. The answer is `Flag_Here!`.

```
0000   02 10 20 10 00 0c 00 04 00 0b 46 6c 61 67 5f 48
0010   65 72 65 21 21
```

## Investigating BLE devices

To discover BLE devices, there are many tools like `bluetoothctl`, `gatttool` or simply a Bluetooth scanner on smartphone.

We quickly spot the 2 BLE devices for the challenges, for example with `bluetoothctl` and command `scan on`

```
[NEW] Device 58:BF:25:9F:B9:36 BeLE Device #02
[NEW] Device 78:E3:6D:1A:FF:52 BeLE Device #01
```

We pick one of the devices, pair (`pair 58:BF:25:9F:B9:36`) and connect (`connect 58:BF:25:9F:B9:36`). Then, we list the services and characteristics.

The first few services (and their characteristics) are standard, and hold the device's name. They are not interesting to solve the challenge.

```
[NEW] Primary Service (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0001
	00001801-0000-1000-8000-00805f9b34fb
	Generic Attribute Profile
[NEW] Characteristic (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0001/char0002
	00002a05-0000-1000-8000-00805f9b34fb
	Service Changed
[NEW] Descriptor (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0001/char0002/desc0004
	00002902-0000-1000-8000-00805f9b34fb
	Client Characteristic Configuration
```    

The services which are specific to the devices hold characteristics at handle 0x29, 0x38 and 0x47.
This always 1 less than the handles to actually read/write to. This is because in BLE the *value* of a characteristic is held in its own handle.

```
[NEW] Primary Service (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0028
	f0248d4c-261a-11ed-861d-0242ac120002
	Vendor specific
[NEW] Characteristic (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0028/char0029
	f65a7230-261a-11ed-861d-0242ac120002
	Vendor specific
[NEW] Primary Service (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0037
	3ef82f7c-2114-4a08-9ead-85fce4757fca
	Vendor specific
[NEW] Characteristic (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0037/char0038
	84af9714-264c-11ed-a261-0242ac120002
	Vendor specific
[NEW] Primary Service (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0046
	4fafc201-1fb5-459e-8fcc-c5c9c331914b
	Vendor specific
[NEW] Characteristic (Handle 0x0000)
	/org/bluez/hci0/dev_58_BF_25_9F_B9_36/service0046/char0047
	beb5483e-36e1-4688-b7f5-ea07361b26a8
	Vendor specific
```

Note we can get the same results `gatttool` or a smartphone BLE scanner. Actually, the results are often easier to read.

```
sudo gatttool -b 58:BF:25:9F:B9:36 -I --sec-level=high
[58:BF:25:9F:B9:36][LE]> connect
...
[58:BF:25:9F:B9:36][LE]> characteristics
handle: 0x0002, char properties: 0x20, char value handle: 0x0003, uuid: 00002a05-0000-1000-8000-00805f9b34fb
handle: 0x0015, char properties: 0x02, char value handle: 0x0016, uuid: 00002a00-0000-1000-8000-00805f9b34fb
handle: 0x0017, char properties: 0x02, char value handle: 0x0018, uuid: 00002a01-0000-1000-8000-00805f9b34fb
handle: 0x0019, char properties: 0x02, char value handle: 0x001a, uuid: 00002aa6-0000-1000-8000-00805f9b34fb
handle: 0x0029, char properties: 0x02, char value handle: 0x002a, uuid: f65a7230-261a-11ed-861d-0242ac120002
handle: 0x0038, char properties: 0x02, char value handle: 0x0039, uuid: 84af9714-264c-11ed-a261-0242ac120002
handle: 0x0047, char properties: 0x08, char value handle: 0x0048, uuid: beb5483e-36e1-4688-b7f5-ea07361b26a8
```

## Attempts

We can read on handle 0x2a and 0x39, but can only write on handle 0x48. We attempt to replay the value of the pcap.

```
[58:BF:25:9F:B9:36][LE]> char-read-hnd 0x002a
Characteristic value/descriptor: 66 13 e1 f5 2c 98 28 eb 
[58:BF:25:9F:B9:36][LE]> char-write-req 0x0048 "0555d737b7ad8a17a1a8b1bec470f9fe"
Characteristic value was written successfully
[58:BF:25:9F:B9:36][LE]> char-read-hnd 0x0039
Characteristic value/descriptor: 4e 6f 70 20 21 
```

In handle 0x39, we read `4e 6f 70 20 21` which is ASCII for `Nop !`. So, our input is incorrect and does not yield the flag.

We quickly notice that each time we write to 0x48, a *new value* is generated in 0x2a.
So, probably this is a challenge / response mechanism:

1. We are expected to read a **challenge nonce** on handle 0x2a
2. We are expected to write the corresponding **challenge response** on handle 0x48
3. If our response is correct, handle 0x39 will probably yield the flag.

## Relation between challenge and response

The nonce is 8 bytes long. Too big to brute force and attempt numerous connections and hope to fall back on the nonce of the PCAP for which we have the correct answer.

The expected answers is 16 bytes long.

At this point, we went to have a look at the BLE devices. They are ESP32 devices.
So, how could an ESP32 generate easily a 16-byte response? Encryption... Hash... SH1? too long. MD5 produces the right output length.

```python
import hashlib

m = hashlib.md5()
m.update(b'\xf3\xdd\x42\x04\xde\xed\xc3\xfc')
m.hexdigest()
```

The output is `fef970c4beb1a8a1178aadb737d75505`.
This is exactly bytes `05 55 d7 37 b7 ad 8a 17 a1 a8 b1 be c4 70 f9 fe` but in the reverse order!

## Flag

We connect once again to a BLE device and read the nonce:

```
[58:BF:25:9F:B9:36][LE]>  char-read-hnd 0x002a
Characteristic value/descriptor: 62 58 66 59 0d 9c 3f 3e
```

Then we compute its MD5 value

```python
m = hashlib.md5()
m.update(b'\x62\x58\x66\x59\x0d\x9c\x3f\x3e')
m.hexdigest()
```

The output is `df5e543a2cbc43508be6d23dc06fa213`. If we reverse the order, it is `13a26fc03dd2e68b5043bc2c3a545edf`.

We write this response to the device:

```
[58:BF:25:9F:B9:36][LE]> char-write-req 0x0048 "13a26fc03dd2e68b5043bc2c3a545edf
Characteristic value was written successfully
```

Then we read the result on handle 0x39:

```
[58:BF:25:9F:B9:36][LE]> char-read-hnd 0x0039
Characteristic value/descriptor: 46 4c 41 47 7b 4d 79 5f 43 68 34 72 5f 73 33 6e 64 5f 34 5f 63 68 34 72 5f 34 72 72 34 79 21 7d
```

```python
f = '46 4c 41 47 7b 4d 79 5f 43 68 34 72 5f 73 33 6e 64 5f 34 5f 63 68 34 72 5f 34 72 72 34 79 21 7d'
bytes.fromhex(f.replace(' ',''))
```

The answer is : `FLAG{My_Ch4r_s3nd_4_ch4r_4rr4y!}`. We've got the flag :)

## Troubles I didn't mention

This challenge was relatively simple but took me more time than expected for several reasons:

1. At first, I wrongly extracted the payload of the BLE Write Command and thought we were sending `52 48 00 05 55 d7 37
b7 ad 8a 17 a1 a8 b1 be c4 70 f9 fe`. This is 19 bytes, and does not correspond to any known hash or encryption result length. I realized then my mistake, removed the 3 heading bytes (which actually contain the reference to handle 0x48) and kept the remaining 16 bytes. 

2. I didn't immediately recognize the MD5 digest. Maybe it took me 5 minutes to see it was reversed. Although it was pretty obvious.

3. In the end, I was sure I had understood the correct challenge/response mechanism but faced many problems to connect back to the BLE device. Fortunately, I have worked with BLE before and know they are touchy, so I patiently tried again and again, checked I was not paired, asked the organizers to reboot the devices. At some point, I was able to connect with my smartphone, but it's not easy to enter 16 bytes and I did it wrong and did not flag. I was still unable to connect from my laptop. I finally decided to reboot it, and it worked straight away afterwards: connect, get nonce, send response, collect flag :)