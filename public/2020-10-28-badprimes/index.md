# Bad Primes - Hack.Lu CTF 2020


# Bad Primes

- 86 solves
- Easy
- Category: Crypto
- Author: midao
- Description : "I thought I understood the RSA primitive but apparently I generated the primitive's primes too primitively... "
- 193 points

# Understanding the issue

The challenge comes with Python code which implements RSA encryption/decryption.

```python
#!/usr/bin/env python2
import binascii


# https://en.wikibooks.org/wiki/Algorithm_Implementation/Mathematics/Extended_Euclidean_algorithm
def xgcd(a, b):
    x0, x1, y0, y1 = 0, 1, 1, 0
    while a != 0:
        (q, a), b = divmod(b, a), a
        y0, y1 = y1, y0 - q * y1
        x0, x1 = x1, x0 - q * x1
    return b, x0, y0


# https://en.wikibooks.org/wiki/Algorithm_Implementation/Mathematics/Extended_Euclidean_algorithm
def modinv(a, m):
    g, x, y = xgcd(a, m)
    if g != 1:
        return None
    else:
        return x % m


n = 3283820208958447696987943374117448908009765357285654693385347327161990683145362435055078968569512096812028089118865534433123727617331619214412173257331161
p = 34387544593670505224894952205499074005031928791959611454481093888481277920639
q = 95494466027181231798633086231116363926111790946014452380632032637864163116199
e = 65537

# flag = "flag{...}"
# flag = int(binascii.hexlify(flag), 16)
# flag = pow(flag, e, n)
flag = 2152534604028570372634288477962037445130495144236447333908131330331177601915631781056255815304219841064038378099612028528380520661613873180982330559507116
d = modinv(e, (p - 1) * (q - 1))
if d == None:
    print "definitely too primitive..."
else:
    print pow(flag, d, n)
```

When we run the program, we get `definitely too primitive`, because d cannot be inversed.
Let's try and understand why.


The 4 integers for RSA are provided: `n, p, q and e`:

- n is the modulus. `n=p*q`. We check this is true:

```python
print "Je verifie n"
myn = p * q
if myn == n:
    print "n is correct"
else:
    print "n is not correct"
```

- p and q are 2 prime numbers. We [check they are prime online](https://www.alpertron.com.ar/ECM.HTM).

- e is the public exponent, and value `65537` is extremly common. No problem with that.

We are unable to compute the private exponent (d) which is part the the decryption key (d,n).
The issue comes from the fact that unfortunately e is not prime with `(p-1) * (q-1)`.

In that case, [this page](https://crypto.stackexchange.com/questions/12255/in-rsa-why-is-it-important-to-choose-e-so-that-it-is-coprime-to-%CF%86n) explains that in that case it is impossible to decrypt the message in a unique way. I.e **There are several possible decrypted messages**

# Solution

We are going to list *all possible decrypted messages* and keep the one which looks like a flag `flag{...}`.

How can we get the decrypted messages when we cannot inverse e? [The solution comes from this page](https://crypto.stackexchange.com/questions/81949/how-to-compute-m-value-from-rsa-if-phin-is-not-relative-prime-with-the-e/81966#81966). The algorithm explains how to compute possible plaintexts when e is no prime with (p-1)(q-1).

We implement this algorithm.

```python
# xgcd returns a triplet. The GCD is g, the first element of the triplet
g, x0, y0 = xgcd(p-1, q-1)

# computing lambda
l_lambda=(p-1)*(q-1) / g

# we arbitraly select k = 2, could work with another value
k = 2

# don't forget to use pow() and not **
L = pow(k, l_lambda/e, n)

# we check L is not 1, if that was the case, we'd need to use another k - see algo
assert L != 1

# Now we are able to compute d
d = modinv(e, l_lambda/e)
print "d=",d

# Compute the various solutions and stop when it's the flag
for i in range(0,e):
    ret = pow(pow(flag, d, n) * pow(L, i,n), 1, n)
    print "Result: ", ret

    # Transform the integer to a string
    v = hex(ret).replace('0x','').replace('L','')
    if len(v) %2 != 0:
        # Odd string length: that's because we have something like 0xa at the beginning
	# instead of 0x0a. We need to add the 0.
        v = '0'+v
    s = binascii.unhexlify(v)
    print "ASCII: ", s[0:20]
    
    if s[0:4] == 'flag':
        print "FOUND"
        print s
        quit()
```

We get the flag:

```
...
Result:  20954479300763094611764878148492851443855269259657142449817554915858952171015595866731757955253353010257834410311051564402462417563162248845880201671549
ASCII:  flag{thanks_so_much_
FOUND
flag{thanks_so_much_for_helping_me_with_these_primitive_primes}

