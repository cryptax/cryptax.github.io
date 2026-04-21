---
layout: post
title: "pycoin - Hack.lu 2021"
author: cryptax
date: 2021-11-04
tags:
- Hack.lu
- CTF
- 2021
- Python
- Bytecode
- Reverse
---

# Pycoin - Hack.lu 2021

This is what we know:

```
PYCOIN
Sold:  	92 times
Type:  	rev
Risk:  	Low
Seller:  	tunn3l

A friend gave me this and he says he can not reverse this... but this is just python? 
```

and we get a **.pyc file** and a  hint `flag[5] == "5" `.

I unfortunately did not solve this challenge on time for the CTF but found it interesting (I got stuck trying to disassemble with `dis` and did not know `xdis` did the work).

**Thanks to tunn3l, TheBadGod, crabbers and crazyman for their help elaborating this write-up.**


# Solution

There is [another write-up here](https://r3kapig.com/writeup/20211102-hacklu/#pycoin(low)), but I didn't find it detailed enough to understand how to solve the challenge. Hopefully a better attempt below! 

## Uncompyle

I used [uncompyle6](https://pypi.org/project/uncompyle6/) to transform the `.pyc` to `.py`: `uncompyle6 pycoin.pyc > uncompiled.py`. The resulting source code is still obscure:

```python
import marshal
marshalled =  b'\xe3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00.... '
exec(marshal.loads(marshalled))
```

## Marshal

[Marshal](https://docs.python.org/3/library/marshal.html) is an internal Python object serialization module.

If we have a look at the marshalled string, we see it is actually Python byte code from a file named `disassembly`. 

```python
>>> redata = marshal.loads(marshalled)
>>> print(redata)
<code object <module> at 0x7efedf31e9d0, file "<disassembly>", line 1>
```

By scanning the `marshalled` string more in details, we spot strings the program asks:

- please supply a valid key:
- valid key!
- invalid key :

and obviously there is MD5 hashing involved (spot md5, digest etc):

```
xda\x03md5z
\xda\x06digestZ\
...
```

## Disassembling Python bytecode

Unfortunately, the regular `dis` disassembler does not work:

```python
>>> dis.disassemble(redata)
  1           0 JUMP_FORWARD             2 (to 4)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "/usr/lib/python3.8/dis.py", line 369, in disassemble
    _disassemble_bytes(co.co_code, lasti, co.co_varnames, co.co_names,
  File "/usr/lib/python3.8/dis.py", line 401, in _disassemble_bytes
    for instr in _get_instructions_bytes(code, varnames, names,
  File "/usr/lib/python3.8/dis.py", line 340, in _get_instructions_bytes
    argval, argrepr = _get_name_info(arg, names)
  File "/usr/lib/python3.8/dis.py", line 304, in _get_name_info
    argval = name_list[name_index]
IndexError: tuple index out of range
```

The solution consists in using [xdis](https://github.com/rocky/python-xdis/) which handles python bytecode from multiple versions.

```python
>>> import xdis.std as dis
>>> dis.dis(redata)
  1:           0 JUMP_FORWARD         (to 4)
               2 LOAD_GLOBAL          (99)
         >>    4 LOAD_CONST           (0)
               6 LOAD_CONST           (('md5',))
               8 IMPORT_NAME          (hashlib)
              10 IMPORT_FROM          (md5)
```

## Understanding the byte code

We read the byte code step by step to understand what the program does.
It reads the user input, and stores it in a variable named `k`. Precisely the following bytecode would correspond to `k = str(input('please supply a valid key:')).encode()`.

```
              14 POP_TOP
              16 LOAD_NAME            (str)
              18 LOAD_NAME            (input)
              20 LOAD_CONST           ('please supply a valid key:')
              22 CALL_FUNCTION        1
              24 CALL_FUNCTION        1
              26 LOAD_METHOD          (encode)
 6:          28 CALL_METHOD          0
              30 STORE_NAME           (k)
```


Check the input length (`k`) is 16:

```
 
              32 LOAD_NAME            (len)
              34 LOAD_NAME            (k)
              36 CALL_FUNCTION        1
              38 LOAD_CONST           (16)
              40 COMPARE_OP           (==)
```

Check that `k[0] = 102`. This corresponds to character `f`.

```
            46 LOAD_NAME            (k)
              48 LOAD_CONST           (0)
              50 BINARY_SUBSCR

  6:          52 LOAD_CONST           (102)
              54 COMPARE_OP           (==)
```

Each time, if the comparison fails, we will fail with invalid key:

```
7:          42 EXTENDED_ARG         (256)
              44 JUMP_IF_FALSE_OR_POP (to 462)
```

Check that `k[1] = k[0] + 6`.

```
           60 LOAD_NAME            (k)
              62 LOAD_CONST           (1)
              64 BINARY_SUBSCR
              66 LOAD_NAME            (k)
              68 LOAD_CONST           (0)
              70 BINARY_SUBSCR
              72 LOAD_CONST           (6)

  6:          74 BINARY_ADD
              76 COMPARE_OP           (==)
```

As k[0] is `f`, we get k[1] = `l`. The format of Hack.Lu flags is `flag{....}`, so it makes sense.

The next bytecode checks that `k[2] = k[1] - k[0] + 91`. So, k[2]='a'.
Then, k[3] = 103 = 'g'.

The following bytecode checks that k[4] = k[11] * 3 - 42. 

```
            138 LOAD_NAME            (k)
             140 LOAD_CONST           (4)
             142 BINARY_SUBSCR

  6:         144 LOAD_NAME            (k)
             146 LOAD_CONST           (11)

 12:         148 BINARY_SUBSCR
             150 LOAD_CONST           (3)
             152 BINARY_MULTIPLY
             154 LOAD_CONST           (42)
             156 BINARY_SUBTRACT
             158 COMPARE_OP           (==)
```

As we know that k[4]='{', this means k[11] = (k[4] + 42)/3 = '7'

The next check is: `k[5] = sum(k)-1322`

```
            164 LOAD_NAME            (k)

  6:         166 LOAD_CONST           (5)
             168 BINARY_SUBSCR

 13:         170 LOAD_NAME            (sum)
             172 LOAD_NAME            (k)
             174 CALL_FUNCTION        1
             176 LOAD_CONST           (1322)
             178 BINARY_SUBTRACT
             180 COMPARE_OP           (==)

```

The next check is `k[6]  + k[7] + k[10] == 260`:

```
           186 LOAD_NAME            (k)
           188 LOAD_CONST           (6)
           190 BINARY_SUBSCR
           192 LOAD_NAME            (k)
           194 LOAD_CONST           (7)

  6:         196 BINARY_SUBSCR
             198 BINARY_ADD

 14:         200 LOAD_NAME            (k)
             202 LOAD_CONST           (10)
             204 BINARY_SUBSCR
             206 BINARY_ADD
             208 LOAD_CONST           (260)
             210 COMPARE_OP           (==)
```

Then, we have: `int(chr(k[7]) * 2) + 1 = k[9]`. In this formula, beware: `chr(k[7]) * 2` is the Python way to create a string of 2 characters `chr(k[7])`.

```
            216 LOAD_NAME            (int)
             218 LOAD_NAME            (chr)
             220 LOAD_NAME            (k)
             222 LOAD_CONST           (7)
             224 BINARY_SUBSCR
             226 CALL_FUNCTION        1
             228 LOAD_CONST           (2)

  6:         230 BINARY_MULTIPLY
             232 CALL_FUNCTION        1

 15:         234 LOAD_CONST           (1)
             236 BINARY_ADD
             238 LOAD_NAME            (k)
             240 LOAD_CONST           (9)
             242 BINARY_SUBSCR
             244 COMPARE_OP           (==)
```

We won't detail the other checks, but there are:

- int(chr(k[7]) * 2) + 1 = k[9]
- k[8] % 17 = 16
- k[9] = k[8] * 2
- md5(k[10] * b'a').digest()[0] - 1 = k[3]
- k[11] = 55
- k[12] = k[14] / 2 - 2
- k[13] = k[10] * k[8] % 32 * 2 - 1
- k[14] = (k[12] ^ k[9] ^ k[15]) * 3 - 23
- k[15] = 125 = '}'


## Solving the conditions and flag

First, we solve all conditions which directly provide characters.
This gives us `flag{....}` and `k[5]='5'` and `k[11]='7'`.

Then, we try to find possible values for k[8], knowing that `k[8] % 17 == 16`, and that k[9] depends on k[8] is probably an alphanumeric character. We are lucky, there is **only one possible value for k[8]**: '2'.

```python
def possible_k8():
    '''
    computes possible values for k[8] which provide alphanumeric value for k[9]
    given formulas:
    k[8] % 17 == 16
    and k[9] = k[8] * 2
    '''
    possible = []
    for k8 in range(ord('0'), ord('z')+1):
        if k8 % 17 == 16:
            k9 = k8 * 2
            if k9 <= ord('z'):
                possible.append(chr(k8))
            else:
                logger.debug("k8={} k9={} not possible".format(chr(k8), chr(k9)))

    logger.debug(possible)
    return possible
```

We immediately deduce k[9] = `d` from formula `k[9] = k[8] * 2`, and k[7] = `9` from `int(chr(k[7]) * 2) + 1 = k[9]`. In the last formula, note that the Python operation `chr(k[7])*2` returns `9`*2, i.e `99`.


Same, we try to guess possible values for k[10] knowing that `md5(k10 * b'a').digest()[0] - 1 = k[3]`. We already have k[3]. Assuming k[10] will be alphanumeric, we brute force each value and keep those that match the md5 formula. There is only 1 result. We get k[10] = 'e'.  From k[10], we automatically compute k[13] and k[6].

Then, there are 2 remaining characters to find: k[12] and k[14] which depend on each other.
Assuming k[12] will be alphanumeric, we compute k[14] with `(k[12] ^ k[9] ^ k[15]) * 3 - 23` and keep possible values.

In the end, there is only one possible solution: `flag{5f92de703d}`

## Full source code

```python
#!/usr/bin/env python3

import logging
from hashlib import md5

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
handler = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
handler.setLevel(logging.DEBUG)
logger.addHandler(handler)

def init():
    '''
    Sets the known characters for the flag
    '''
    k = list('flag{'+'x'*10+'}')
    k[11] = chr(55)
    k[5] = '5'
    logger.debug('len={} k={}'.format(len(k),''.join(k)))
    return k

def possible_k8():
    '''
    computes possible values for k[8] which provide alphanumeric value for k[9]
    given formulas:
    k[8] % 17 == 16
    and k[9] = k[8] * 2
    '''
    possible = []
    for k8 in range(ord('0'), ord('z')+1):
        if k8 % 17 == 16:
            k9 = k8 * 2
            if k9 <= ord('z'):
                possible.append(chr(k8))

    logger.debug("Possible k8={}".format(possible))
    return possible

def possible_k10(k3):
    '''
    computes possible values for k[10], knowing correct k[3]
    '''
    possible = []
    for k10 in range(ord('0'), ord('z')+1):
        computed_k3 = md5(k10 * b'a').digest()[0] - 1
        if k3 == computed_k3:
            possible.append(chr(k10))
    logger.debug("Possible k10={}".format(possible))
    return possible

def possible_k12(k9, k15):
    '''
    computes possible values for k[12], knowing correct k[9] and k[15]
    which give alphanumeric result for k[14]
    knowing formulas:
    k[14] = (k[12] ^ k[9] ^ k[15]) * 3 - 23
    k[12] = k[14] / 2 - 2
    '''
    possible = []
    for k12 in range(ord('0'), ord('z')+1):
        k14 = (k12 ^ k9 ^ k15) * 3 - 23
        if k14 >= ord('0') and k14 <= ord('z'):
            if k14 == (k12 + 2) *2:
                possible.append(chr(k12))
    logger.debug("Possible k12={}".format(possible))
    return possible
        
    

if __name__ == '__main__':
    k = init()
    
    possible = possible_k8()
    assert(len(possible) == 1), "we have several possible k8"
    k[8] = possible[0]
    logger.debug("k[8]={}".format(k[8]))

    # k[9] = k[8] * 2
    k[9] = chr(ord(k[8]) * 2)
    logger.debug("k[9]={}".format(k[9]))

    # int(chr(k[7]) * 2) + 1 = k[9]
    k[7] = str(ord(k[9])-1)[0]
    logger.debug("k[7]={}".format(k[7]))
    
    possible = possible_k10(ord(k[3]))
    assert(len(possible) == 1), "we have several possible k10"
    k[10] = possible[0]
    k[13] = chr( (((ord(k[10]) * ord(k[8])) % 32) * 2) - 1)
    logger.debug("k[13]={}".format(k[13]))

    #k[6]  + k[7] + k[10]  = 260
    k[6] = chr(260 - ord(k[7]) - ord(k[10]))
    logger.debug("k[6]={}".format(k[6]))

    possible = possible_k12(ord(k[9]), ord(k[15]))
    assert(len(possible) == 1), "we have several possible k12"
    k[12] = possible[0]
    k[14] = chr((ord(k[12]) + 2) * 2)
    logger.debug("k[14]={}".format(k[14]))

    logger.info("Solution: {}".format(''.join(k)))
```

