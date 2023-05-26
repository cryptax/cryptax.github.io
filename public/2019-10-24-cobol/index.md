# Cobol OTP - Hack.Lu CTF 2019


# Cobol OTP

Category: Crypto
Author: midao
Level: easy
Solves: 104

Thanks to R. who actually flagged it :)

## Description

To save the future you have to look at the past. Someone from the inside sent you an access code to a bank account with a lot of money. Can you handle the past and decrypt the code to save the future?

## Attached file

A zip is provided with:

- otp.cob: a program in Cobol
- out

```
Enter your message to encrypt:
ï¿½ï¿½ï¿½y;dhuï¿½ï¿½Fï¿½]UjhCï¿½ï¿½-ï¿½1T`h&ï¿½ï¿½Fï¿½1*T{_ï¿½ï¿½ï¿½p02J
```


## Understanding the Cobol program

We understand the Cobol program is performing XOR encryption using a key provided in a keyfille.

```cobol
       file-control.
           select key-file assign to 'key.txt'
```

We don't have the key file of course, but we know that flags begin with `flag{...}`.

This means that the data in the key data will be at most 50 characters.
```
       fd key-file.
       01 key-data pic x(50).
```

This allocates two variables: ws-flag which contains 1 characters and ws-key with at most 50. Then we have ws-xor-len composed of at most 1 digit (so 0 to 9), and ws-ctr, a counter, one digit as well.

```
       01 ws-flag pic x(1).
       01 ws-key pic x(50).
       ...
       01 ws-xor-len pic 9(1) value 1.
       77 ws-ctr pic 9(1).

```

In Cobol, the program instructions begin at `procedure division`.
We read the key file.

```
       procedure division.
           open input key-file.
           read key-file into ws-key end-read.
```

Then, we ask for the message to encrypt, and initialize the counter to 1.

```
           display 'Enter your message to encrypt:'.
           move 1 to ws-ctr.
```

Then, we loop 50 times (because the message has 50 characters) to encrypt each character.
By the way, we check that indeed our encrypted message has 50 characters (actually 49 + \n).

For each character, we read one character of the message to encrypt and put it in ws-parse.
We also copy it on ws-flag which is the variable which will hold the encrypted character.
Then, we call XOR on ws-flag using the i-th character of the key (using the counter ws-ctr).
Finally we increment the counter.

```
perform 50 times
               call 'getchar' end-call
               move return-code to ws-parse
               move ws-parse to ws-flag

               call 'CBL_XOR' using ws-key(ws-ctr:1) ws-flag by value
               ws-xor-len end-call

               display ws-flag with no advancing
               add 1 to ws-ctr end-add
           end-perform.
```

## Compililng Cobol

If you want to try the program, on Linux you can compile it using `open-cobol` package:

```
$  cobc -free -x -o hack_otp otp.cob
```
 
## Solution

We know that the first few characters of the encrypted message must correspond to plaintext `flag{`. As the *encryption* algorithm is XOR, we can recover the first few bytes of the key:

```
>>> hex(0xa6 ^ ord('f'))
'0xc0'
>>> hex(0xd2 ^ ord('l'))
'0xbe'
>>> hex(0x13 ^ ord('a'))
'0x72'
>>> hex(0x96 ^ ord('g'))
'0xf1'
>>> hex(0x79 ^ ord('{'))
'0x2'
```

So, the first bytes of the key are 0xc0, 0xbe, 0x72, 0xf1, 0x02.

We set this as a key, and decrypt the entire message in a Python interpreter:

```
>>> from itertools import izip, cycle
>>> import string
>>> def xor(message,key):
...     return ''.join(chr(ord(x) ^ ord(y)) for (x,y) in izip(message, cycle(key)))
... 
>>> key='\xc0\xber\xf1\x02'
>>> message='\xa6\xd2\x13\x96y;\x10dhu\x9f\xddF\x9f]\x17UjhC\x8f\x8c-\x921\x07T`h&\x9f\xcdF\x871*T{\x04_\xa6\xeb\x06\xa4p0\x112J'
>>> xor(message,key)
'flag{\xfb\xae\x16\x99w_c4n_\xd7\xeb\x18\x99AO2_c3\xc7\xea\x12\x99$_s4v3\xea\xea\t\xf5]fUtUr\xf0\xaf@\xbb'
```

It doesn't work yet, because we miss characters for the key. We have only set 5 bytes but obviously it is not enough.

We notice in the middle that we have something that prints out correctly: `fUtUr`, so this means that at some point we were correctly aligned with the key once again. As the message is 50 characters long, the key is likely to be a divider of 50. If 5 is not enough, then 10 probably.

Slightly before `fUtUr`, we also see `s4v3`. We guess that in between we should have `_th3_` for `s4v3_th3_fUtUr`.

With that, we recover the rest of the key:

```python
beginning_key='\xc0\xber\xf1\x02'
guess=xor(ciphertext[35:40], '_th3_')
key = beginning_key+guess
```

And then, we can decrypt the message with the correct 10 character long key and get the flag: `flag{N0w_u_c4n_buy_CO2_c3rts_&_s4v3_th3_fUtUrE1!}`

Here is the Python program that solves the challenge

```python
from itertools import izip, cycle
import string

def xor(message,key):
    return ''.join(chr(ord(x) ^ ord(y)) for (x,y) in izip(message, cycle(key)))

beginning_key='\xc0\xber\xf1\x02'

# this is the hex contained in the file named out
ciphertext='\xa6\xd2\x13\x96y;\x10dhu\x9f\xddF\x9f]\x17UjhC\x8f\x8c-\x921\x07T`h&\x9f\xcdF\x871*T{\x04_\xa6\xeb\x06\xa4p0\x112J'

partial=xor(ciphertext,beginning_key)
print partial

# guess rest of key
guess=xor(ciphertext[35:40], '_th3_')
key = beginning_key+guess

# decrypt
print xor(ciphertext, key)
```




