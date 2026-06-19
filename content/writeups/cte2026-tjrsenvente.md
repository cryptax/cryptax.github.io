---
layout: post
title: CTE v2 (2026) - Toujours en Vente
author: cryptax
date: 2026-06-18
tags:
- CTE
- CTF
- 2026
- z3
- Reverse
- Rust
---

# CTE v2 (2026) - Toujours en Vente

This is a reverse engineering challenge created by *Miaou* for Capture The Evidence (CTE) in June 2026. We are given a binary `broker_tool`, and we need to find a URL to flag, e.g `https://u.rl/path`.

## Reconnaissance

The binary is an ELF x86-64, not stripped.
We run it:

```
===================================
   B U S I N E S S   P O R T A L   
===================================

Welcome, valued partner.
Remember: data is our business. And business is GOOD.

1. Submit Email Lists
2. Submit Leaked Photos
3. Submit Leaked Documents
4. Bug Report

q. Quit
Choose: 
```

The fourth menu requires a password, we'll concentrate on this one.

```
you need the password to unlock this menu

PRESS ENTER TO MENU
```

## Reverse engineering the binary with Ghidra

This binary was implemented in Rust. You'll spot plenty of Rust names: `__rust_begin_short_backtrace()`, `__rustcall {{closure}}()` etc.

> More? Read my talk on [reversing Rust with Radare2](https://youtu.be/XFiSfU-1E7I?si=dT9zWBx3PC5wH3hY) at r2con 2025.

Rust comes with lots of its own functions. Notice the namespace called `ToujoursEnVente`: that's where our application functions lie.

| Function name | Address | Description               |
| ------------- | ------- | ------------------------- |
| `ToujoursEnVente::main` | `0x119df0` | Infinite menu loop |
| `ToujoursEnVente::validate_password` | `0x118ec0` | Validates password for option 4 |
| `ToujoursEnVente::decrypt` | `0x11a120` | Decrypts a ciphertext |
| `ToujoursEnVente::bug_report` | `0x118150` | Called for option 4 |

The menu loop is handled by `ToujoursEnVente::main` and calls `bug_report()` when we select option 4.

![](/images/cte2026-tjrs-menuloop.png)

`bug_report` reads an environment variable `ADMIN_PWD` from which it validates a password:

```c
std::env::_var(&env_var_result,
                 "ADMIN_PWDlibrary/std/src/../../backtrace/src/symbolize/gimli/elf.rs",9);
if (env_var_result == 0) {
  validate_password(pwd_valid,pwd_cap,pwd_len);
``` 

This password is later used to decrypt a ciphertext:

```c
      lVar1 = CONCAT44(uStack_4c,decoded_output);
      data_ptr = (byte *)CONCAT44(uStack_44,uStack_48);
	  decrypt(&decrypt_buf,data_ptr,decoded_len);
```	  

## Understanding `validate_password`

The function is quite ugly to reverse: I'll use OpenCode + Ghidra MCP and a free model to make some sense out of it. Basically, the password is validated against a list of constraints, and each failed constraint produces a different error message.

The constraints are not easy to reverse. See for yourself, this is checking that bytes are not >= 0x80:

```
        00118ee0 48 8d 48 20     LEA        RCX,[RAX + 0x20]
        00118ee4 4c 39 f1        CMP        RCX,R14
        00118ee7 77 2b           JA         LAB_00118f14
        00118ee9 48 83 f8 e0     CMP        RAX,-0x20
        00118eed 0f 84 a7        JZ         LAB_0011909a
                 01 00 00
        00118ef3 f3 41 0f        MOVDQU     XMM0,xmmword ptr [R15 + RAX*0x1]
                 6f 04 07
        00118ef9 f3 41 0f        MOVDQU     XMM1,xmmword ptr [R15 + RAX*0x1 + 0x10]
                 6f 4c 07 10
        00118f00 66 0f eb c8     POR        XMM1,XMM0
        00118f04 66 0f d7 d1     PMOVMSKB   EDX,XMM1
        00118f08 48 89 c8        MOV        RAX,RCX
        00118f0b 85 d2           TEST       EDX,EDX
        00118f0d 74 d1           JZ         LAB_00118ee0
        00118f0f e9 86 01        JMP        LAB_0011909a
                 00 00
```

The error messages are obfuscated and cannot be found by a simple string search.

Constraints:

1. Each byte of the password must be between 0x20 and 0x7e
2. Password length must be >= 16, and a multiple of 3 and 4 (so it must be a multiple of 12).
3. The sum of all bytes must be divisible by 10
4. An exclamation mark must be in the middle of the password
5. Password must contain at least 1 lowercase, 1 uppercase, 1 digit and 1 character `!`
6. The password must contain the string `paste!s d3 na74 3 b4c4lh4u` as a subsequence, but not necessarily contiguous, i.e we can have `p`, then other bytes, then `aste!s` etc.

The string `paste!s d3 na74 3 b4c4lh4u` decodes from an encrypted table at 0x10a96c: `STRING[i] = ENC_TABLE[i] ^ (i + 0xc0)`.

![](/images/cte2026-tjrs-bacalhau.png)




## Finding the password with z3

The perfect solution to solve constraints is [Z3](https://www.microsoft.com/en-us/research/project/z3-3/): `pip install z3` for support in Python.

After a few attempts (this is solver number 4), my AI model wrote the perfect solver:

```python
#!/usr/bin/env python3
"""
Z3 solver for broker_tool password v4.
XOR is a SUBSEQUENCE match (26 bytes "paste!s d3 na74 3 b4c4lh4u").
'!' at position L/2, AFTER XOR completes.
L must be %12==0, L/2 > 25 → L > 50 → L >= 60.
"""
from z3 import *

XOR_TABLE = [
    0xb0, 0xa0, 0xb1, 0xb7, 0xa1, 0xe4, 0xb5, 0xe7,
    0xac, 0xfa, 0xea, 0xa5, 0xad, 0xfa, 0xfa, 0xef,
    0xe3, 0xf1, 0xb0, 0xe7, 0xb7, 0xe1, 0xba, 0xbf,
    0xec, 0xac
]
PREFIX = bytes(XOR_TABLE[i] ^ (i | 0xC0) for i in range(26))

def solve(L):
    print(f"\n=== Trying L={L} ===")
    s = Solver()
    pw = [BitVec(f"c{i}", 32) for i in range(L)]

    for i in range(L):
        s.add(pw[i] >= 0x20, pw[i] <= 0x7e)

    # XOR subsequence: PREFIX bytes must appear in order in the password
    # First char must be 'p' (XOR[0])
    s.add(pw[0] == PREFIX[0])

    # Track XOR subsequence progress
    # xp[i] = how many XOR bytes matched up to position i
    xp = [Int(f"xp_{i}") for i in range(L)]
    s.add(xp[0] == 1)  # pw[0] == PREFIX[0] guaranteed above
    for i in range(1, L):
        match_next = Or([And(xp[i-1] == k, pw[i] == PREFIX[k]) for k in range(26)])
        s.add(If(match_next, xp[i] == xp[i-1] + 1, xp[i] == xp[i-1]))
        s.add(xp[i] >= 0, xp[i] <= 26)

    # All 26 XOR bytes must be found
    s.add(xp[L-1] == 26)

    # '!' at middle position L/2
    mid = L // 2
    s.add(pw[mid] == ord('!'))

    # Character class requirements
    num_upper = Sum([If(And(pw[i] >= ord('A'), pw[i] <= ord('Z')), 1, 0) for i in range(L)])
    num_lower = Sum([If(And(pw[i] >= ord('a'), pw[i] <= ord('z')), 1, 0) for i in range(L)])
    num_digit = Sum([If(And(pw[i] >= ord('0'), pw[i] <= ord('9')), 1, 0) for i in range(L)])
    s.add(num_upper >= 5)
    s.add(num_lower >= 8)
    s.add(num_digit >= 1)

    # cStack_b0: password[i] == password[i+2] for i=0..L-3
    pair_matches = Sum([If(pw[i] == pw[i + 2], 1, 0) for i in range(L - 2)])
    s.add(pair_matches >= 7)

    # local_af: password[i] == password[i-1] + 1 for i=1..L-1
    inc_matches = Sum([If(pw[i] == pw[i - 1] + 1, 1, 0) for i in range(1, L)])
    s.add(inc_matches >= 6)

    # Sum % 10 == 0
    s.add(Sum(pw) % 10 == 0)

    print("Solving...")
    if s.check() == sat:
        m = s.model()
        vals = [m[pw[i]].as_long() for i in range(L)]
        pwd = ''.join(chr(v) for v in vals)

        # Verify
        upper = sum(1 for i in range(L) if chr(vals[i]).isupper())
        lower = sum(1 for i in range(L) if chr(vals[i]).islower())
        digit = sum(1 for i in range(L) if chr(vals[i]).isdigit())
        inc = sum(1 for i in range(1, L) if vals[i] == vals[i-1] + 1)
        pair = sum(1 for i in range(L-2) if vals[i] == vals[i+2])
        total = sum(vals)

        print(f"\nPassword (len={L}): {pwd!r}")
        print(f"Sum: {total}, %10 = {total % 10}")
        print(f"Upper: {upper} (≥5), Lower: {lower} (≥8), Digit: {digit} (≥1)")
        print(f"Inc: {inc} (≥6), Pair: {pair} (≥7)")
        print(f"Mid (pos{mid}): {pwd[mid]} (should be !)")

        # Check XOR subsequence
        xor_ok = True
        xor_idx = 0
        for i in range(L):
            if xor_idx < 26 and vals[i] == PREFIX[xor_idx]:
                xor_idx += 1
        xor_ok = (xor_idx == 26)
        print(f"XOR subsequence: {'OK' if xor_ok else f'FAIL (matched {xor_idx}/26)'}")

        return pwd
    else:
        print(f"UNSAT")
        return None

# Try various lengths
for L in [60, 72, 84, 96]:
    pwd = solve(L)
    if pwd:
        print(f"\n*** FOUND: {pwd!r} ***")
        break
```

Run it:

```
python3 solver4.py 

=== Trying L=60 ===
Solving...

Password (len=60): 'pa$_0f0F~}~12Q2Q23no3 t@  !|@x!SC@1ste!s d3 na74 3 b4c4lh4u*'
Sum: 4430, %10 = 0
Upper: 5 (≥5), Lower: 19 (≥8), Digit: 16 (≥1)
Inc: 6 (≥6), Pair: 7 (≥7)
Mid (pos30): ! (should be !)
XOR subsequence: OK

*** FOUND: 'pa$_0f0F~}~12Q2Q23no3 t@  !|@x!SC@1ste!s d3 na74 3 b4c4lh4u*' ***
```

The admin password is `'pa$_0f0F~}~12Q2Q23no3 t@  !|@x!SC@1ste!s d3 na74 3 b4c4lh4u*`. It's a bit strange. We do recognize the end of `pasteis` and `de nata e bacalhau`, but the beginning makes no sense. This had me doubt... but it was correct!

## Flag

Now, it's easy to flag. Launch `ADMIN_PWD='pa$_0f0F~}~12Q2Q23no3 t@  !|@x!SC@1ste!s d3 na74 3 b4c4lh4u*' ./broker_tool` and select option 4:

```
Choose: 4

Hey! So you pretend you are an admin and found a bug?

This is a serious software developed by serious roxxors from the 1337 team of brokers from Fantasmas De Redes!!
After all, we have found so much value in what others thought were useless!
We will track data from closed companies (maybe you worked there?) and extract value using their client listings.
Completed with dark web data, like people that wouldn't pay ransoms to recover their encrypted data
(all of this because they could not prevent 1337 hackers from entering their poorly protected systems).
And we did all this by our selves.

So.

Do you REALLY, *REALLY* think that you found a bug?
Prepare everything because we will want details and proofs that their IS a bug...
Be able to reproduce the bug, know what YOU did or what is wrong with YOUR machine (poor CPU architecture? not enough RAM? slow connection?).

WE WANT TO SEE IT.
If you dare showing it to us, just come to https://t.me/+k1JCrVoDHN5lYTBk and we will see.

Adeus!


PRESS ENTER TO MENU
```

Our flag is `https://t.me/+k1JCrVoDHN5lYTBk`. This is the Telegram group of a fake cybercriminal group, created for the CTE.

## Understanding `decrypt`, just for fun

The `decrypt` function is not a simple XOR. It uses SHA-256 with a counter to generate a keystream which is used as XOR-key.

```python
keystream = b''
for counter in range(64):   
    keystream += SHA-256(password || counter_BE32)

decrypted = ciphertext XOR keystream 
```

The ciphertext is located at 0x10b000

![](/images/cte2026-tjrs-ciphertext.png)

## Understanding where the error messages are - just for fun

Previously, we said that each missed constraint in `validate_password` would display an error message, except these messages were obfuscated. 

How are they obfuscated? This is of no use to flag the challenge, but I just wanted to know. Each error message consists in a static `XoredLiteral` struct with 3 fields:

1. data_ptr: XOR-ed obfuscated bytes
2. len
3. key: the XOR key (a single byte)

| PTR_DAT | data_ptr  | key | decrypted message |
|-----------------|------------------------------|---------|-------------------|
| `0x1611a8` | `0x10a9b2` | `0x27` | `there should be at least 1 symbol` |
| `0x1611c0` | `0x10a9d3` | `0xa0` | `! should be placed in the middle` |
| `0x1611d8` | `0x10a888` | `0x37` | `there should be at least 1 digit` |
| `0x1611f0` | `0x10aa21` | `0x14` | `more chars must be the increment of the previous char (ABC...)` |
| `0x161208` | `0x10aa5f` | `0xb7` | `more chars must be the same as the penultimate` |
| `0x161220` | `0x10aa8d` | `0xea` | `there are not enough upper case letter` |
| `0x161238` | `0x10aab4` | `0xf4` | `there are not enough lower case letters` |


## Conclusion

A lovely reverse challenge: I was very happy to use z3 + the decryption mechanism is interesting (not just a simple XOR as we often see).

> Contrary to the [official writeup](https://github.com/EternalBlueCTE2k26/CTEv2-Writeups/tree/main/GroupeOrganise/ToujoursEnVente), I did not use gdb.


-- Cryptax
