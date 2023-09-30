---
layout: post
title: "BruCON 2023 CTF Steganography challenges"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- stegano
---

## Hidden in Brucon

*"It appears a flag has hidden itself inside the brucon logo, can you retrieve it?"*
A file named `brucon.jpg` is provided.

The solution was very easy:

```
$ file brucon.jpg 
brucon.jpg: JPEG image data, JFIF standard 1.01, aspect ratio, density 1x1, segment length 16, comment: "TOREON{4lw4Ys_CHeck_MEt4Dat4}", progressive, precision 8, 842x595, components 3
```

Flag: `TOREON{4lw4Ys_CHeck_MEt4Dat4}`

## Hidden inside the House

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

## You Are Significant

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

## One Call Away

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



