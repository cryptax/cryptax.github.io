---
layout: post
title: "THCon CTF 2021 - Crypto challenges"
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

# Baby sign

## The challenge

Nowadays even toddlers use rsa to sign: `nc server 11001` (where server was up during the CTF).

And providing `server.py`, which is the program which runs on the server.

```python
from Crypto.Util.number import getPrime, GCD, inverse, bytes_to_long
import os

class SecureSigner():
	def __init__(self):
		p = getPrime(512)
		q = getPrime(512)
		e = 0x10001
		phi = (p-1)*(q-1)
		while GCD(e,phi) != 1:
			p = getPrime(512)
			q = getPrime(512)
			phi = (p-1)*(q-1)

		self.d = inverse(e,phi)
		self.n = p * q
		self.e = e

	def sign(self, message):
		return pow(message,self.d,self.n)

	def verify(self, message, signature):
		return pow(signature,self.e,self.n) == message



def menu():
	print(
		"""
		1 - Sign an 8-bit integer
		2 - Execute command
		3 - Exit
		"""
		)
	choice = input("Choice: ")
	if choice == "1":
		try:
			m = int(input("Integer to sign: "))
			if 0 <= m < 256:
				print("Signature: {:d}".format(s.sign(m)))
			else:
				print("You can only sign 8-bit integers.")
		except:
			print("An error occured.")
			exit(1)
	elif choice == "2":
		try:
			cmd = input("Command: ")
			m = bytes_to_long(cmd.encode())
			signature = int(input("Signature: "))
			if s.verify(m,signature):
				os.system(cmd)
			else:
				print("Wrong signature.")
		except:
			print("An error occured.")
			exit(1)
	elif choice == "3":
		exit(0)
	else:
		print("Incorrect input.")
		exit(1)




if __name__ == '__main__':
	s = SecureSigner()

	print("Here are your parameters:\n - modulus n: {:d}\n - public exponent e: {:d}\n".format(s.n, s.e))
	
	while True:
		menu()
```

## Solving the challenge

The server is able to:

1. Sign integers below 256
2. Execute a Unix command whose signature is correct. The user must provide the command, and its signature.

For instance, we'd like to execute `ls` on the server, to see where the flag is located, and then `cat` the flag file. The issue is that `ls` transformed to an integer (`bytes_to_long('ls'.encode())`) is far above 256. So, *we need to find a way to get the signature of bigger integers...*

We are able to do that because the server performs raw RSA signature.
So, basically, let's say that if we want to sign integer `4`, this is actually `4^d mod n` (d being the private exponent), and this is `2^d * 2^d mod n`. So, we can also compute the signature of 4 (`s4`) by asking the signature of 2 (`s2`), and then `s4 = s2 * s2 mod n`. See [here](https://crypto.stackexchange.com/questions/20085/which-attacks-are-possible-against-raw-textbook-rsa)

## Find the shell command

The hardest part of the challenge is now to find a shell command whose prime factors are below 256.

I compute prime factors for various shell commands. There is a nice shell command `factor` for that by the way!

```bash
#!/bin/bash
for c in 'ls' 'sh' 'bash' 'll' ' sh'
do
    integer=$(printf "%d" 0x`echo -n "$c" | xxd -ps`)
    factors=$(factor $integer)
    echo "Command $c -> factors: $factors"
done
```

Unfortunately, neither `ls` nor `sh` work, because some of their factors are > 256. `ll` is close but not working. 

```
Command ls -> factors: 27763: 27763
Command sh -> factors: 29544: 2 2 2 3 1231
Command bash -> factors: 1650553704: 2 2 2 3 3 22924357
Command ll -> factors: 27756: 2 2 3 3 3 257
Command  sh -> factors: 2126696: 2 2 2 11 11 13 13 13
```

The command ` sh` (i.e starting with a space) should work.

## Executing the command

Once the command is found, we just need to apply the theory we identified earlier.

My program below must be run in parallel with the real one on the remote server. My program tells me what to ask to the real one:

- It asks me to provide the value `n` that the real server displays
- It asks to provide the result of various signatures of ints
- Based on those values, it computes the correct signature for the shell command.

```python
from Crypto.Util.number import bytes_to_long, isPrime

def factors(n):
        '''
        simplified function to find primes (won't work in all cases!)
        we are only interested in primes < 256
        In Shell, use # factor xxx
        '''
        factor = []
        primes = [2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251]
        current = n

        while not isPrime(current):
                for p in primes:
                        if current % p == 0:
                                current = current // p
                                factor.append(p)
                                break

        factor.append(current)

        return factor

cmd = ' sh'
cmd_int = bytes_to_long(cmd.encode())
factor_list = factors(cmd_int)

n = int(input("Please provide n: "))

sig = 1
for f in factor_list:
        print("Please ask signature of int {}".format(f))
        intermediate = int(input("value: "))
        sig = sig * intermediate

sig = sig % n
print("Please send command: '{}'".format(cmd))
print("Use this computed signature: {}".format(sig))
```

The signature is recognized (see below) and I get a shell on the remote server. And easily then the flag.

```
		1 - Sign an 8-bit integer
		2 - Execute command
		3 - Exit
		
Choice: 2
Command:  sh
Signature: 41609626228993531674308423452763833896469702200570002126534943100863455927692030907187524746966991634244450965040800407545526148474306900123580404483596334993024977357851683651502221718086925851882951517245197834174887191621924133861437811355658364899236723602859660221594479159243377647935842532691897847973
ls
flag.txt
server.py
cat flag.txt
THCon21{Textb00k_RS4_Mall3ab1l1ty}
```

The flag is **THCon21{Textb00k_RS4_Mall3ab1l1ty}**.
