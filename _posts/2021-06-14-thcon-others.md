---
layout: post
title: "RSA Internal Attacker - THCon 2021"
author: cryptax
tags:
- THCon
- CTF
- 2021
- Crypto
---

# RSA Internal Attacker - THCon 2021

## Challenge description

```
I've found this rsa implementation, it is quite strange. I have a public/private key and I've also intercepted a ciphertext but infortunately it was not for me, so I can't read it. But I'am really curious, can you decrypt it ? :)
```

2 files are provided:

```python
#!/usr/bin/env python3

from Crypto.Util.number import getPrime, inverse, bytes_to_long
import random
from math import gcd

def init():
    p = getPrime(1024)
    q = getPrime(1024)
    return p, q

def new_user(p, q):
    phi = (p - 1) * (q - 1)
    while True:
        e = random.randint(2, 100000)
        if gcd(e, phi) == 1:
            break
    d = inverse(e, phi)
    return e, d

def encrypt(m, e, n):
    return pow(m, e, n)


p, q = init()
n = p * q
e_a, d_a = new_user(p, q)
e_b, d_b = new_user(p, q)

FLAG = b"THC2021{??????????????????????????????????????}"

c = encrypt(bytes_to_long(FLAG), e_b, n)

print(f"The public modulus : {hex(n)}")
print(f"Your key pair : ({hex(e_a)}, {hex(d_a)})")
print(f"Your boss public key : {hex(e_b)}")
print(f"Intercepted message : {hex(c)}")
```

output.txt:

```
The public modulus : 0x8d926c44899930f8f3fc3ea04cb9dfa7eb309b6d8e932b531007c4d8479e1dd227365087feeced8f854b1b54cc947182ee2241fe526c758e630b44e0c196ce8dc0995124f94755b0601d3454f89f178db2ffb3adeafcac2f49b656aace2acdb63afcd62a8847aadc55ca2452dff8c65ea5bfcfe03411f3b63a2bc4b244126259e2e845c68f8c1cd2d275bd2e344d35da542503c72f153ded14f766efecdfc98605e6963c4b1a7197de9e56b4b61ca1ab648265e6775819935a005a089eff04c27083d385e8d73ebf56b47f875c5fa9984e026914e1cbfc02205e75d02dc0da392700b536bf0fc8decd043736441e69fecc696b2127589f2ac9700e30c4dc88ef
Your key pair : (0x8095, 0x21a31ccbce8117f468e9c26e3a404659d585ea166606c85ff192466b8dd4d9650b110addef35e3effdf8cb8710235cf5843e688e977be0d32842e0b4fa493f451ad8d77d35672696cf4373eaa0c0093a6a0baa348f790fc466be559bd90e788505b795026df6e991f6e8769565e06f472a445676e2c99240eccab25cd44433e8a083e66912c7a81c81c190470188c699c1a24dac441956b46aa364623f2c78c4ffca49e89f8a6f6edc51140e744f80a968fa80901fc91b88d491829b334542fd3ef460ddfa9a729d981b0ae9fa12bd0901c919972020b5f9e661b34a914fff85732e45718a2d216018507e7406aed4543096df76ca6fcfa4ab5dd21a84f162fd)
Your boss public key : 0x22bb
Intercepted message : 0x2118ee5b546b529c6b8d8fba1638f046006d7de2c10571d179af958f65d223a9a78df91daa5913f39f97d47681e1e10b8c58b6b462caf1fd56c683129ea732927cf55a06441cde5b743d00582569c9bbf43dab3d7b46ddbf03b358ca6ee075bafcc06165efa8592474bf78732dec4433502579338f2b925a922e74704cf19f7dff414a451fbc24b4ace4a9d8a072fb4259ebc8452941eb9f100f1df0cf19d5718088867a17d52d1c3f1fd5f92c9b9c55cbe528fbfd130879c14bde651a9e402f50b851c753e5915882b02a1136b43e015c6d4fd07e48aa05be08e9faf533a763f21d29a9b7fe8f355a8ffcbf11dc96b1069df4e302a3b310ecf39f25300bb375
```

## Solving the challenge

Reading `chall.py`, we see the program creates 2 RSA key pairs, for "me" and my boss, using the **same modulus** (n).

By definition, RSA key pairs are created such as `e*d = 1 mod phi`:

- e is the public exponent
- d the corresponding private exponent
- phi is (p-1)(q-1) - actually we don't care so much to solve the exercise, although it's important for RSA! And p and q are 2 primes such that n = p*q (same we don't care).

So, `e*d` is a multiple of phi + 1. This means we have an integer `k`, such that `e*d = k *phi + 1`.

For our boss, we have only the public exponent (e_b). But if we have managed to compute phi, then compute the private exponent of our boss (d_b) is simple. Again, `e *d = 1 mod phi`, so `d = 1/e mod phi`, i.e d is the modular inverse of e (modulus phi).

And once we know the private exponent, then it is game over to decrypt the intercepted message, we just use the standard RSA formula: `plaintext = ciphertext ^ d mod n`.

So, **the challenge boils down on finding phi**. Actually, we can easily compute `k * phi`, but we have no idea what value `k` (an integer) has. So, we'll just try and check the decrypted ciphertext looks like a flag. All flags at THCon begin with `THCon21{`, so we'll check this. (IMHO, `chall.py` is misleading here as it mentions `THC2021{`. This should have been changed).

In the end, we are very lucky, the solution arrives nearly immediately. I would have let the program run 5 minutes at most, before trying a more intelligent approach (common modulus attack etc).

```python
#!/usr/bin/env python3

from Crypto.Util.number import inverse, long_to_bytes

# ----------------------
n = int(0x8d926c44899930f8f3fc3ea04cb9dfa7eb309b6d8e932b531007c4d8479e1dd227365087feeced8f854b1b54cc947182ee2241fe526c758e630b44e0c196ce8dc0995124f94755b0601d3454f89f178db2ffb3adeafcac2f49b656aace2acdb63afcd62a8847aadc55ca2452dff8c65ea5bfcfe03411f3b63a2bc4b244126259e2e845c68f8c1cd2d275bd2e344d35da542503c72f153ded14f766efecdfc98605e6963c4b1a7197de9e56b4b61ca1ab648265e6775819935a005a089eff04c27083d385e8d73ebf56b47f875c5fa9984e026914e1cbfc02205e75d02dc0da392700b536bf0fc8decd043736441e69fecc696b2127589f2ac9700e30c4dc88ef)
e_a = int(0x8095)
d_a = int(0x21a31ccbce8117f468e9c26e3a404659d585ea166606c85ff192466b8dd4d9650b110addef35e3effdf8cb8710235cf5843e688e977be0d32842e0b4fa493f451ad8d77d35672696cf4373eaa0c0093a6a0baa348f790fc466be559bd90e788505b795026df6e991f6e8769565e06f472a445676e2c99240eccab25cd44433e8a083e66912c7a81c81c190470188c699c1a24dac441956b46aa364623f2c78c4ffca49e89f8a6f6edc51140e744f80a968fa80901fc91b88d491829b334542fd3ef460ddfa9a729d981b0ae9fa12bd0901c919972020b5f9e661b34a914fff85732e45718a2d216018507e7406aed4543096df76ca6fcfa4ab5dd21a84f162fd)
e_b = int(0x22bb)
ciphertext = int(0x2118ee5b546b529c6b8d8fba1638f046006d7de2c10571d179af958f65d223a9a78df91daa5913f39f97d47681e1e10b8c58b6b462caf1fd56c683129ea732927cf55a06441cde5b743d00582569c9bbf43dab3d7b46ddbf03b358ca6ee075bafcc06165efa8592474bf78732dec4433502579338f2b925a922e74704cf19f7dff414a451fbc24b4ace4a9d8a072fb4259ebc8452941eb9f100f1df0cf19d5718088867a17d52d1c3f1fd5f92c9b9c55cbe528fbfd130879c14bde651a9e402f50b851c753e5915882b02a1136b43e015c6d4fd07e48aa05be08e9faf533a763f21d29a9b7fe8f355a8ffcbf11dc96b1069df4e302a3b310ecf39f25300bb375)

print("n={}".format(n))
print("e_a={}".format(e_a))
print("d_a={}".format(d_a))
print("e_b={}".format(e_b))

# ----------------------
# e * d = 1 mod phi
# so, e * d = k * phi + 1
found = False
k = 1
kphi = (e_a * d_a) - 1

while not found:
    # compute possible phi 
    phi = ((e_a * d_a) - 1) // k

    # d = 1/e mod phi
    d_b = inverse(e_b, phi)
    
    plaintext = pow(ciphertext,d_b, n)
    flag = long_to_bytes(plaintext)
    if flag.startswith(b'THCon21'):
        found = True
        print(flag)
        print("k={}".format(k))
        quit()

    k = k + 1
```

The flag is `THCon21{coMm0n_m0duLus_wh1th_int3rn4l_aTt4ck3r}`.

NB. I didn't flag this during the CTF, as I only played sporadically. A pity! ;-)





