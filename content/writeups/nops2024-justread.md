---
layout: post
title: "Just Read and JoJo is Missing (NOPS CTF 2024)"
author: cryptax
date: 2024-06-03
tags:
- NOPS
- CTF
- 2024
- Reverse
- Radare2
- CyberChef
---

Those 2 challenges had already been flagged by a team mate when I started working on the CTF.
But, during the CTF, at a time when I was getting frustrated not to flag other challenges, I decided to do them again on my own lol.

Both challenges are very beginner friendly.


## Just Read

### Description

This challenge is in the *Reverse* category, with a simple description "Find a way to break this" and an executable `main` is provided.

```
main: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=d60942d0003ba2a102c7ef78d939c4a9232cf94a, for GNU/Linux 3.2.0, not stripped
```

### Solution with Radare2

I use Radare2 disassembler on the executable. I run the famous `aaa` command to have the disassembler analyze the binary. Then, I search for the main with `afl~main` and disassemble the main with `pdf`.

```
[0x00001060]> afl~main
0x00001149    4    575 main
[0x00001060]> s main
```

And there, the flag just flashes in front of my eyes character by character. 

![](/images/nops2024-justread.png)

We can check the flag:

```
$ ./main N0PS{cH4r_1s_8bits_1Nt}
Well done, you can validate with this flag!
```

## Jojo is missing

### Description

"We have received a message from Jojo, join our Discord server to read it: DISCORD-INVITE-LINK"

### Solution

So we simply had to go on the Discord server, in the announcement section, and find this message:

```Have you heard about Jojo?

Jojo has always been protecting the cyberworld, ensuring its peace, preserving its balance.

Jojo is also a numeric virtuoso, juggling with code just like a musician maestro would with his instrument. 

There is no one as good or as talented as Jojo.

The problem is that recently, Jojo disappeared. We have no idea where Jojo could be.

We need your help. Since Jojo vanished, the cyberworld has become a land of hackers, no one is safe anymore!

During our investigations, we managed to recover the last message sent by Jojo to us.

Here is the message:
49 66 20 61 6E 79 6F 6E 65 20 72 65 61 64 73 20 69 74 2C 20 49 20 61 6D 20 4A 6F 6A 6F 2E 20 49 20 68 61 76 65 20 62 65 65 6E 20 63 61 70 74 75 72 65 64 20 62 79 20 61 20 67 72 6F 75 70 20 63 61 6C 6C 65 64 20 4A 33 4A 75 4A 34 2E 20 50 6C 65 61 73 65 20 63 6F 6D 65 20 61 6E 64 20 73 61 76 65 20 6D 65 21 0A 4E 30 50 53 7B 4A 30 4A 30 5F 31 73 5F 6D 31 53 35 31 6E 47 21 7D
```

Throw the last few hexadecimal bytes in [CyberChef with the From Hex recipe](https://gchq.github.io/CyberChef/#recipe=From_Hex('Auto')&input=NDkgNjYgMjAgNjEgNkUgNzkgNkYgNkUgNjUgMjAgNzIgNjUgNjEgNjQgNzMgMjAgNjkgNzQgMkMgMjAgNDkgMjAgNjEgNkQgMjAgNEEgNkYgNkEgNkYgMkUgMjAgNDkgMjAgNjggNjEgNzYgNjUgMjAgNjIgNjUgNjUgNkUgMjAgNjMgNjEgNzAgNzQgNzUgNzIgNjUgNjQgMjAgNjIgNzkgMjAgNjEgMjAgNjcgNzIgNkYgNzUgNzAgMjAgNjMgNjEgNkMgNkMgNjUgNjQgMjAgNEEgMzMgNEEgNzUgNEEgMzQgMkUgMjAgNTAgNkMgNjUgNjEgNzMgNjUgMjAgNjMgNkYgNkQgNjUgMjAgNjEgNkUgNjQgMjAgNzMgNjEgNzYgNjUgMjAgNkQgNjUgMjEgMEEgNEUgMzAgNTAgNTMgN0IgNEEgMzAgNEEgMzAgNUYgMzEgNzMgNUYgNkQgMzEgNTMgMzUgMzEgNkUgNDcgMjEgN0Q) and you'll get the flag:

```
If anyone reads it, I am Jojo. I have been captured by a group called J3JuJ4. Please come and save me!
N0PS{J0J0_1s_m1S51nG!}
```



