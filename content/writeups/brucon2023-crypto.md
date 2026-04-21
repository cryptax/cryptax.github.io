---
layout: post
title: "BruCON 2023 CTF Crypto Challenge"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- Crypto
---

# Crypto: Dragon's Crib

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





