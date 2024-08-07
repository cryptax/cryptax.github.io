---
layout: post
title: "Hashfun - Cyber Security Rumble CTF 2020"
author: cryptax
date: 2020-10-31
tags:
- CSR
- CTF
- 2020
- Crypto
- Python
---

# Hashfun

*"I guess there is no way to recover the flag"* and a Python program is provided:

```python
from secret import FLAG

def hashfun(msg):
    digest = []
    for i in range(len(msg) - 4):
        digest.append(ord(msg[i]) ^ ord(msg[i + 4]))
    return digest

print(hashfun(FLAG))
# [10, 30, 31, 62, 27, 9, 4, 0, 1, 1, 4, 4, 7, 13, 8, 12, 21, 28, 12, 6, 60]
```

## Understanding the algorithm

So this is an alleged hash function. The flag is not provided and part of the `FLAG` module. However, we have the resulting digest for the flag: `[10, 30, 31, 62, 27, 9, 4, 0, 1, 1, 4, 4, 7, 13, 8, 12, 21, 28, 12, 6, 60]`.

The digest consists in XORing a character with one 4 spaces later.

## The flaw

We know the flag begins with `CSR{` and ends with `}`. Actually, we just need to know it begins with `CSR{`. So, we can compute the next 4 characters of the flag. Indeed we have:

- C xor flag[4] = 10
- S xor flag[5] = 30
- R xor flag[6] = 31
- { xor flag[7] = 62

This gives us flag[4] = C xor 10 = I etc.

Then, again, we are able to compute the next 4 characters! E.g flag[4] xor flag[8] = 27,  so flag[8] = flag[4] xor 27 = I xor 27 = R.

Etc until the end of the hash. And we will be able to check our algorithm is correct if we recover '}' for the last character of the flag.

## Implementation

```python
thehash = [10, 30, 31, 62, 27, 9, 4, 0, 1, 1, 4, 4, 7, 13, 8, 12, 21, 28, 12, 6, 60]
prefix = 'CSR{'

flag = []
key = prefix

for i in range(0, len(thehash), 4):
    for j in range(0, 4):
        if i + j >= len(thehash):
            break
        flag.append(chr(ord(key[j]) ^ thehash[i+j]))

    if i + 4 >= len(thehash):
        break
    key = flag[i:i+4]

print("The flag is: {}{}".format(prefix, ''.join(flag)))   
```


The flag is: `CSR{IMMERDIESEMATHEMATIK}`
