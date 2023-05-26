---
layout: post
title: "Shakti CTF 2020"
author: cryptax
tags:
- Shakticon
- CTF
- 2020
---

The overall intent for Shakti CTF is for *beginners*, learn what is a CTF and get women interested in the topic. Therefore, the "easy" 50 points challenges are *really very very easy*, and 100-point challenges are still *really easy  (far easier than baby challenges at Hack.Lu CTF ;-). Everybody has to start one day ;P so I'm okay with this rating as long as it remains consistent. Globally, this was the case, with the exception of a few challenges under/over rated.

| Challenges      | My opinion on the rating      |
| ------------------- | --------------------------------------- |
| Easy Encoding | Slightly more difficult than other [ultra] baby challenges |
| PYthn | Appropriate rating |
| Biscuits | Very easy but consistent rating  |
| AuthEN | Very easy but consistent rating  |
| Ancient Warfare | Very easy but consistent rating  |
| Not that easy | Overrated |
| Connect | Very easy but consistent rating  |
| Damez | Overrated! Come on! |
| Shark on Wire | Very easy but consistent rating  |
| 3, 2, 1... Go | Very easy but consistent rating  |


# Easy Encoding (Crypto, 50 points)

```
Joan knows this is breakable. Do you know how?

01001110 01111010 01001101 00110010 01001111 01000100 01011001 01111000 01001110 01101101 01001001 00110011 01001110 01000100 01011001 00110101 01001110 01101010 01001101 00110011 01001110 01000100 01011001 00110010 01001110 00110010 01001001 00110001 01001110 01111010 01001101 00110000 01001110 01111010 01001001 00110010 01011010 01000100 01010101 00110001 01001110 01111010 01000001 00110001 01011010 01101010 01010001 01111010 01001110 01101010 01100111 01111010 01001110 01000100 01011010 01101010 01001110 01101101 01001101 01111010 01001101 01111010 01011010 01101100 01001110 01101010 01100011 01111010 01001101 01111010 01100100 01101011
```

We transform each 8-bit sequence in a character:

```python
result = []
encoding = '01001110 01111010 01001101 00110010 01001111 01000100 01011001 01111000 01001110 01101101 01001001 00110011 01001110 0100010
0 01011001 00110101 01001110 01101010 01001101 00110011 01001110 01000100 01011001 00110010 01001110 00110010 01001001 00110001 01001110
 01111010 01001101 00110000 01001110 01111010 01001001 00110010 01011010 01000100 01010101 00110001 01001110 01111010 01000001 00110001 
01011010 01101010 01010001 01111010 01001110 01101010 01100111 01111010 01001110 01000100 01011010 01101010 01001110 01101101 01001101 0
1111010 01001101 01111010 01011010 01101100 01001110 01101010 01100011 01111010 01001101 01111010 01100100 01101011'

for c in encoding.split(' '):
    result.append(chr(int(c,2)))
    
print("Encoded string: {0}".format(''.join(result)))
```

The result is `NzM2ODYxNmI3NDY5NjM3NDY2N2I1NzM0NzI2ZDU1NzA1ZjQzNjgzNDZjNmMzMzZlNjczMzdk`. This is not yet the flag, but it is good because only ASCII characters, we are probably close to the solution.

Encoding. Lets' try Base 64 decoding.

```python
import base64

unb64 = base64.b64decode(''.join(result))
print("Flag: {0}".format(bytes.fromhex(unb64.decode('utf-8'))))
```

We get the flag **shaktictf{W4rmUp_Ch4ll3ng3}**.

# PYthn (Reversing, 100 points)

Challenge description only says: `Familiar with python?` and provides a Python source file.

The source file has 3 functions: `Fun`, `FuN` and `fuN`. I immediately rename those to make the reading easier. Also, actually, `fuN` is never called. We only have `k=FuN(Fun(X))`, so I rename `Fun` as `step1` and `FuN` will be `step2`.

The idea of the program is to take some input, feed it in a custom algorithm (step1 then step2) and compare the value to an expected hash. If the digests are equal, then the input is the flag (need to surround with `shaktictf{...}`). 

Let's try to inverse the algorithm. `step1` does an XOR which will work both ways. Nothing to do. `step2` does `+i+5` and `+4`. Let's try the inverse: `-i-5` and `-4`.

```python
Z=[]
k=[]
thehash="K78m)hm=|cwsXhbH}uq5w4sJbPrw6"

def unstep2(inp):
    for i in range(len(inp)):
        if(i<11):
            Z.append(chr(ord(inp[i])-i-5))
        else:
            Z.append(chr(ord(inp[i])-4))
    print("[unstep2] returns: ", ''.join(Z))
    return(''.join(Z))

def step1(inp):
    st=[]
    for i in range (len(inp)):
        st.append(chr(ord(inp[i])^1))
    print("[step1] returns: ", ''.join(st))
    return(''.join(st))

a=unstep2(step1(thehash))
print(step1(unstep2(thehash)))
```

This returns:

```
[step1] returns:  J69l(il<}bvrYicI|tp4v5rKcQsv7
[unstep2] returns:  E02d_a0pTgnUe_Expl0r1nG_Mor3
[unstep2] returns:  E02d_a0pTgnUe_Expl0r1nG_Mor3F11e ^b1oUhoTd^Dyqm1s0oF^Lns2
[step1] returns:  D13e^`1qUfoTd^Dyqm1s0oF^Lns2G00d!_c0nTinUe_Expl0r1nG_Mor3
D13e^`1qUfoTd^Dyqm1s0oF^Lns2G00d!_c0nTinUe_Expl0r1nG_Mor3
```

And there we see it: `G00d!_c0nTinUe_Expl0r1nG_Mor3`. The flag is `shaktictf{G00d!_c0nTinUe_Expl0r1nG_Mor3}`.

I didn't really understand the algorithm here, but this solution works ;-)

# Not that easy (Forensics, 200 points)

```
We have intercepted the communication between two criminals and we found that they had shared a secret information. Can you find out the secret?
```

A PCAP file is provided. We open it with wireshark and notice this:

![](/images/shakticon-notthateasy-wireshark.png)

A PNG file is transmitted. We save it to a file and open it: it is a QR code.

![](/images/shakticon-notthateasy-qrcode.png)

We scan the QR code with a smartphone app, it provides the flag. **This challenge was clearly overrated!**


# AuthEN (Web, 50 points)

The challenge redirects you to a web page. If you have a look at the source code of the page, you see:

![](/images/shaktictf-authen.png)

So, we need to login as `admin` and the password corresponds to `String.fromCharCode(...)` where the values are ASCII.

```python3
password = [115, 104, 97, 107, 116, 105, 99, 116, 102, 123, 98, 51, 121, 48, 110, 100, 95, 112, 117, 114, 51, 95, 99, 52, 108, 99, 117, 
108, 97, 116, 105, 48, 110, 115, 125]
print(''.join([chr(i) for i in password]))
```

Password (and flag): `shaktictf{b3y0nd_pur3_c4lculati0ns}`

# Biscuits (Web, 50 points)

The challenge says : "Ada Lovelace used to love eating french biscuits during her work" and the challenge's title is "Biscuits". For a web challenge, we immediately think: **Cookies**.

The challenge redirects to a web server. We query the page using **curl** and requesting *cookies*:

```
curl -c cookies.txt http://34.72.245.53/Web/Biscuits/
SORRY NO FLAG FOR U :P<!DOCTYPE html>
<html>
<head>
	<title>
		
	</title>
</head>
<body>
	Help Ada Remember her theories!!
	<br>
	<h2>
	SHE NEEDS YOUR HELP HACKER
</h2>


</body>
</html>
```

Then simply read the cookie file:

```
$ cat cookies.txt 
# Netscape HTTP Cookie File
# https://curl.haxx.se/docs/http-cookies.html
# This file was generated by libcurl! Edit at your own risk.

34.72.245.53	FALSE	/Web/Biscuits/	FALSE	0	THE_FLAG_IS	shaktictf%7Bc00k13s_m4k3_phr3n0l0gy%26m3sm3r15m_3asy%7D
34.72.245.53	FALSE	/	FALSE	0	PHPSESSID	ir75n4a9vik8ng9clmucths17i
```


# Ancient Warfare (Crypto, 50 points)

The challenge description is:

```
Do you know how people from Caeser's time used to send encrypted messages?

Try to get the flag : funxgvpgs{byq3e_1f_a0g_nyj4lf_gu3_o3gg3e!}
```

Caesar is known to have used rotational ciphers. We are going to use `rot13` which is the best known on computers:

```
echo 'funxgvpgs{byq3e_1f_a0g_nyj4lf_gu3_o3gg3e!}' | rot13
shaktictf{old3r_1s_n0t_alw4ys_th3_b3tt3r!}
```

# Connect (Pwn, 50 points)

```
Your adventure begins here to help the renowned Computer Scientist Kathleen Booth to get across the challenges and win the race. Cross the gates and enter into the arena!

Connect via nc 34.72.218.129 1111
```

The challenge provides an ELF executable `chall`.
We run `strings` on the executable and notice these:

```
You have successfully connected to our service!
To get your flag, please enter the appropriate bash commands.
/bin/sh
```

It looks like the challenge is simply running a shell!
We try, and then list files, see there is `flag.txt` and read it:

```
$ nc 34.72.218.129 1111
You have successfully connected to our service!
To get your flag, please enter the appropriate bash commands.
ls
chall
flag.txt
run.sh
cat flag.txt
shaktictf{w3lc0me_t0_th3_ar3na_c0mrade}
```
