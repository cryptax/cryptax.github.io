---
layout: post
title: "TUCTF 2019"
author: cryptax
date: 2019-12-01
tags:
- TU
- CTF
- 2019
---


I tried **TU CTF**.

I only played 1 or 2 hours and flagged quite a lot: this is an easy CTF, good for beginners. See some very quick write-ups below.

Unfortunately I can't remember the titles of the challenges.

## Challenge 1



We have a binary to decompile. I use Ghidra and locate a strange function called `thisone`

![](/images/tuctf19-thisone.jpg)

The `printFlag` function decodes the flag.

![](/images/tuctf19-printflag.jpg)

I write a program to mimic the decoding:

```c
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
  char encrypted [] = "\\PJ\\fC|)L0LTw@Yt@;Twmq0Lw|qw@w2$a@0;w|)@awmLL|Tw|)LwZL2lhhL0k
";
  char decrypted[40];
  int i = 0;
  int len = strlen(encrypted);
  
  memset(decrypted, 0x00, 40);
  strcpy(decrypted, encrypted);

  printf("encrypted=%s\n", encrypted);
  printf("len=%d\n",len);

  while (i<len) {
    decrypted[i] = ((((decrypted[i] ^ 0x0f) - 0x1d) * 8) % 0x5f) + ' ';
    i++;
  }

  printf("%s\n", decrypted);
  
}
```

I get the flag: `TUCTF{7h3r35_4lw4y5_m0r3_70_4_b1n4ry_7h4n_m3375_7h3_d3bu663r}`

## Challenge 2

We got an object file `run.o`. We decompile it using Ghidra.

![](/images/tuctf19-runobject.png)

The function `checkPassword` explains how the password is validated. There is an XOR with 0xaa.

I write the following code:

```c
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
  char encrypted [] = { 0xfd,0xff,0xd3,0xfd,0xd9,0xa3,0x93,0x35,0x89,0x39,0xb1,0x3d,0x3b,0xbf,0x8d,0x3d,0x3b,0x37,0x35,0x89,0x3f,0xeb,0x35,0x89,0xeb,0x91,0xb1,0x33,0x3d,0x83,0x37,0x89,0x39,0xeb,0x3b,0x85,0x37,0x3f,0xeb,0x99,0x8d,0x3d,0x39,0xaf, 0x00 };
  char pass[0x2c];

  int len = 0x2c;
  int i = 0;

  int encrypted_len = strlen(encrypted);

  printf("len=%d\n", encrypted_len);
  printf("expected=%d\n", len);

  while (i<len)  {
    pass[i] = ((encrypted[i] >> 1) ^ 0xaa);
    printf("i=%d character=%c hex=%x \n", i, pass[i], pass[i]);
    
    i++;
  }

  printf("%s\n", pass);
  return 0;
  
}
```

and get the flag: `TUCTF{c0n6r47ul4710n5_0n_br34k1n6_7h15_fl46}`
