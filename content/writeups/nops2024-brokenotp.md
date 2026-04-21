---
layout: post
title: "Broken OTP (NOPS CTF 2024)"
author: cryptax
date: 2024-06-03
tags:
- NOPS
- CTF
- 2024
- Crypto
- OTP
- Python
- obfuscation
- builtin
- XOR
---

## Description

This challenge has a very short description: "i heard OTP it the most secure encryption algorithm ever" and supplied a **Python** source file. Because that code only worked on **Python 3.12**, a **Dockerfile** was also supplied.

## De-obfuscating the source code

The source code is intentionally *obfuscated*. 


```python
import random

secret = 'XXXXXXXXXXXXXXXXXXXX'

PINK = 118
RED = 101
YELLOW = 97
GREEN = 108
BLACK = __builtins__
PURPLE = dir
e = getattr(BLACK, bytes([RED, PINK, YELLOW, GREEN]).decode())
g = e(''.__dir__()[4].strip('_')[:7])
b = g(BLACK, PURPLE(BLACK)[92])
i = g(BLACK, PURPLE(BLACK)[120])
...
```

### getattr and __builtins__

The values `PINK`, `RED`, `YELLOW` and `GREEN` are just the ASCII codes for the letters `v`,`e`,`a`,`l`. The construction `bytes([RED, PINK, YELLOW, GREEN]).decode()` will simply make the bytes `eval`.

The Python interpreter has several [built-in functions](https://docs.python.org/3/library/functions.html), such as `getattr` and `eval`. We read that `getattr` returns "the value of the named attribute of the supplied object".

Let's inspect again `getattr(BLACK, bytes([RED, PINK, YELLOW, GREEN]).decode())`.

```python
getattr(BLACK, bytes([RED, PINK, YELLOW, GREEN]).decode())
getattr(__builtins__, bytes([101, 118, 97, 108]).decode()) # replace variables
getattr(__builtins__, b'eval'.decode()) # replace ASCII codes
getattr(__builtins__, 'eval') # decode() transforms bytes to string
__builtins__.eval
```

The obfuscated line boils down to getting the `eval()` builtin function. [Eval is an important Python function](https://docs.python.org/3/library/functions.html#eval) which evaluates Python expressions.

This is `getattr(__builtins__, bytes(['e','v','a','l']).decode())`

### `__dir__` and dir

The [dir() function](https://docs.python.org/3/library/functions.html#dir) returns "the list of names in the current local scope. [..] If the object has a method named __dir__)_, this method will be called and must return the list of attributes."

So, if we do `''.__dir__()`, `''` is of type `str` and we call its `__dir__()` function. This will return all attributes of the `str` class. Actually this returns:

```python
>>> ''.__dir__()
['__new__', '__repr__', '__hash__', '__str__', '__getattribute__', '__lt__', '__le__', '__eq__', '__ne__', '__gt__', '__ge__', '__iter__', '__mod__', '__rmod__', '__len__', '__getitem__', '__add__', '__mul__', '__rmul__', '__contains__', 'encode', 'replace', 'split', 'rsplit', 'join', 'capitalize', 'casefold', 'title', 'center', 'count', 'expandtabs', 'find', 'partition', 'index', 'ljust', 'lower', 'lstrip', 'rfind', 'rindex', 'rjust', 'rstrip', 'rpartition', 'splitlines', 'strip', 'swapcase', 'translate', 'upper', 'startswith', 'endswith', 'removeprefix', 'removesuffix', 'isascii', 'islower', 'isupper', 'istitle', 'isspace', 'isdecimal', 'isdigit', 'isnumeric', 'isalpha', 'isalnum', 'isidentifier', 'isprintable', 'zfill', 'format', 'format_map', '__format__', 'maketrans', '__sizeof__', '__getnewargs__', '__doc__', '__setattr__', '__delattr__', '__init__', '__reduce_ex__', '__reduce__', '__subclasshook__', '__init_subclass__', '__dir__', '__class__']
```

The fifth element of the output (arrays start at 0 in Python) is `__getattribute__`.

```python
g = e(''.__dir__()[4].strip('_')[:7])
g = e('__getattribute__'.strip('_')[:7])
g = e('getattribute'[:7])
g = e('getattr') # e is the builtin eval function
g = eval('getattr') # g will be the getattr function
```

Similarly `b = g(BLACK, PURPLE(BLACK)[92])` is transformed as follows:

```python
b = g(BLACK, PURPLE(BLACK)[92]) # replace each variable
b = getattr(__builtins__, dir(__builtins__)[92])
```

Now, this line **depends on your Python version** because Python builtin function vary slightly from one version to another. The challenge was meant to run on **Python 3.12** and there, the 93th attribute of __builtins__ is `bytes`.

### Use the interpreter

Let's use the interpreter to solve the remaining obfuscation

```python
>>> import random
>>> secret = 'XXXXXXXXXXXXXXXXXXXX'
>>> BLACK = __builtins__
>>> PURPLE = dir
>>> e = eval
>>> g = getattr
>>> b = g(BLACK, PURPLE(BLACK)[92])
>>> i = g(BLACK, PURPLE(BLACK)[120])
>>> t = ['74696d65', '72616e646f6d', '5f5f696d706f72745f5f', '726f756e64', '73656564']
>>> d = lambda x: b.fromhex(x).decode()
>>> fb = g(i, PURPLE(i)[-6])
>>> _i = lambda x: e(d(t[2]))(x)
>>> i
<class 'int'>
>>> fb
<built-in method from_bytes of type object at 0x7f58cc426b80>
>>> [ bytes.fromhex(x) for x in t ]
[b'time', b'random', b'__import__', b'round', b'seed']
```

`i` is the integer class. `fb` is the [`from_bytes()` function of the `int` class](https://docs.python.org/3/library/stdtypes.html). `t` is an array of hexadecimal ASCII characters which form known Python modules and function names.

`_i` is lambda function that does `eval('__import__')(x)`, i.e it gets the module provided as argument.


### s and r

The last two obfuscated lines require a little of attention:

```python
s = lambda: g(BLACK,d(t[3]))(g(_i(d(t[0])), d(t[0]))()) + fb(secret.encode())
r = g(_i(d(t[1])), d(t[4]))
```

We work out what `s` is doing step by step:

```python
s = lambda: getattr(__builtins__, 'round')(getattr(_i(d(t[0])), 'time')()) + int.from_bytes(secret.encode())
s = lambda: round(getattr(_i(d(t[0])), 'time')()) + int.from_bytes(secret.encode())
s = lambda: round(getattr(_i('time'), 'time')()) + int.from_bytes(secret.encode())
s = lambda: round(time.time()) + int.from_bytes(secret.encode())
```
It gets the current time, rounds it up, and adds the flag transformed into an integer.

As expected, `r` is going to deal with randomness:

```python
r = getattr(_i('random'), 'seed')
r = random.seed()
```

We can check it with the interpreter: `r` is the `seed()` function of the random module.

```python
>>> r
<bound method Random.seed of <random.Random object at 0x55a96b067650>>
```

r = g(_i(d(t[1])), d(t[4]))


```python
>>> t = ['74696d65', '72616e646f6d', '5f5f696d706f72745f5f', '726f756e64', '73656564']
```

## OTP Algorithm

The remaining of the Python source code is as follows:

```python
def kg(l):
    return bytes([random.randint(0,255) for i in range(l)])

def c(p):
    k = kg(len(p))
    return bytes([k[i] ^ p[i] for i in range(len(p))]).hex()

if __name__ == '__main__':
    r(s())
    print("Welcome to our encryption service.")
    choice = input("Choose between:\n1. Encrypt your message.\n2. Get the encrypted secret.\nEnter your choice: ")
    match choice:
        case "1":
            message = input("Please enter the message you wish to encrypt: ")
            print(f"Your encrypted message is: {c(message.encode())}")
        case "2":
            print(f"The secret is: {c(secret.encode())}")
        case _:
            print("Invalid option!")
```			

Once we have understood what `r` and `s` do, the algorithm is quite simple to understand:

- `kg` is a key generation function. It generates a random byte string of a desired length.
- `c` is the XOR algorithm: first we generate a one-time key of the same length as the input, then we XOR this generated key with the input.

The XOR algorithm is "perfect" in cryptography, as long as the key is never re-used.
The provided implementation generates a different key for each input... as long as the random number generator provides different output. The issue is with the *seed*. The implementation seeds the RNG `r(s())` with:

1. The time. This is not a good seed on computers because it changes only every second.
2. The secret. 

So, let's suppose we manage to call the OTP system twice during the *same second*. In a first thread, we have it compute a message of our own, and in a second thread, we ask it to compute the secret (those are the 2 choices of the program).

Both computations occur during the same second, so they share the same *seed* `s()`.
This means that in both threads, `kg(l)` is going to output the *same one-time key*, let's call it OTP.

In the first thread, let's call our message M. The program will output M ^ OTP.
In the second thread, the program will output flag ^ OTP.

We know M, so we can compute (M ^ OTP) ^ M = OTP.
This gives us *OTP*.
Now, we can compute (flag ^ OTP) ^ OTP = flag.
This yields the *flag*.

## Implementation

I wrote the program to call the OTP system twice in Bash, using `expect`. The message I choose to encrypt can be anything, it just needs to be the same length as the secret. By running the program once before, we get an output of 38 hexadecimal characters, this means there are 19 characters, so we choose a message of 19 characters too.

```bash
#!/bin/bash

encrypt_message() {
    expect <<EOF
    spawn ./sc nopsctf-broken-otp.chals.io
    expect "Welcome to our encryption service."
    expect "Choose between:"
    expect "Enter your choice:"
    send "1\r"
    expect "Enter the message you want to encrypt:"
    send "SENDMETHEFLAGINEEDI\r"
    expect eof
EOF
}

get_encrypted_secret() {
    expect <<EOF
    spawn ./sc nopsctf-broken-otp.chals.io
    expect "Welcome to our encryption service."
    expect "Choose between:"
    expect "Enter your choice:"
    send "2\r"
    expect eof
EOF
}

# Run the functions in parallel
encrypt_message &
get_encrypted_secret &

# Wait for both background jobs to complete
wait
```

I ran this program on the system:

```
$ ./automate.sh
spawn ./sc nopsctf-broken-otp.chals.io
spawn ./sc nopsctf-broken-otp.chals.io
(connected to nopsctf-broken-otp.chals.io:443 and reading from stdin)
(connected to nopsctf-broken-otp.chals.io:443 and reading from stdin)
Welcome to our encryption service.
Choose between:
1. Encrypt your message.
2. Get the encrypted secret.
Enter your choice: Welcome to our encryption service.
Choose between:
1. Encrypt your message.
2. Get the encrypted secret.
Enter your choice: 1
2
Please enter the message you wish to encrypt: The secret is: 315c160529fe70163557a8c5f91e2aab31e29c
SENDMETHEFLAGINEEDI
Your encrypted message is: 2c2908121f8b500e2f7ad7dde125579b4195a8
```

Then I wrote a very simple Python program to compute the OTP and finally the flag:

```python
motp = '2c2908121f8b500e2f7ad7dde125579b4195a8'
kotp = '315c160529fe70163557a8c5f91e2aab31e29c'

m = 'SENDMETHEFLAGINEEDI'

# MSG^OTP ^MSG
otp = bytes(x ^ y for x, y in zip(bytes.fromhex(motp), m.encode()))
# KEY ^OTP)
^OTP 
f = bytes(x ^ y for x, y in zip(bytes.fromhex(kotp), otp))
print(f)
```

## Happy End

The output is `N0PS{0tP_k3Y_r3u53}`.

I enjoyed this challenge because we had to *de-obfuscate*, *think* and *code*. I also enjoyed it even more than we solved it as a team: I had worked out most of the challenge, but somehow messed up the final XORs to recover the OTP and the flag (??!!) and fortunately, a team mate with a fresh mind helped me find the error.
