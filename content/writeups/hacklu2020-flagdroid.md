---
layout: post
title: "Flagdroid - Hack.Lu CTF 2020"
author: cryptax
date: 2020-10-28
tags:
- hack.lu
- CTF
- 2020
- Android
---


# Flagdroid

- 147 solves
- 153 points
- Easy
- Category: Reverse
- Author: m0ney
- Text: "This app won't let me in without a secret message. Can you do me a favor and find out what it is?"
- Download file: an APK



## Analysis of the app


We decompile the application. The validation of the flag is done when you press the button check:

```java
public void onClick(View arg10) {
                TextView tvWrong = (TextView)MainActivity.this.findViewById(0x7F0700EC);  // id:textViewWrong
                TextView tvCorrect = (TextView)MainActivity.this.findViewById(0x7F0700EB);  // id:textViewCorrect
                String secretInput = ((EditText)MainActivity.this.findViewById(0x7F0700C4)).getText().toString();  // id:secretInput
                Matcher match = Pattern.compile("flag\\{(.*)\\}").matcher(secretInput);
                if(match.find()) {
                    String[] flagcore = match.group().replace("flag{", "").replace("}", "").split("_");
                    if(flagcore.length == 4 && ((MainActivity.this.checkSplit1(flagcore[0])) && (MainActivity.this.checkSplit2(flagcore[1])) && (MainActivity.this.checkSplit3(flagcore[2])) && (MainActivity.this.checkSplit4(flagcore[3])))) {
                        tvWrong.setVisibility(4);
                        tvCorrect.setVisibility(0);
                        return;
                    }
                }
```

The flag is surrounded by `flag{...}` and each part is separated by a `_`. There are 4 different parts. Each part is validated by a method `checkSplitX()` where X is the number of the split.

## First part

It is a base64 string `dEg0VA==`: `tH4T`

## Second part

The second part is created from an algorithm that:

- checks the length is 9
- performs a translation on the character
- XORs the result with a fixed key `hack.lu20`

To do the decode, we must do it the other way:

```java
    public static String createSplit2() {
        try {
	    String s = new String("\u001fTT:\u001f5\u00f1HG");
            char[] v8 = s.toCharArray();
            byte[] key = "hack.lu20".getBytes("UTF-8");

	    int v3;
            for(v3 = 0; v3 < 9; ++v3) {
		v8[v3] = (char)(v8[v3] ^ key[v3]);
                v8[v3] = (char)(v8[v3] - v3);
            }

	    System.out.println("Result: "+ String.valueOf(v8));
	    return new String(v8);
        }
        catch(UnsupportedEncodingException unused_ex) {
	    System.out.println("Exception");
        }
	return null;
    }
```

The main difficulty is to copy paste the Unicode string correctly...

Result: `w45N-T~so`

## Third part

We know the part consists of 8 characters. The first four are `h4rd`, possibly with a different case.

The last 4 characters are not given, but we know the MD5 of the string must be `6d90ca30c5de200fe9f671abb2dd704e`.

We search for the string on [MD5 reverse](https://md5.gromweb.com/?md5=6d90ca30c5de200fe9f671abb2dd704e) but it is not known

So, we brute force it.

```python
import hashlib

prefix = 'h4rd'

for a in ['h', 'H']:
    for b in ['r', 'R']:
        for c in ['d', 'D']:
            for i in range(ord('-'), ord('~')+1):
                for j in range(ord('-'), ord('~')+1):
                    for k in range(ord('-'), ord('~')+1):
                        for l in range(ord('-'), ord('~')+1):
                            s = a + '4' + b + c + chr(i) + chr(j) + chr(k) + chr(l)
                            print("Testing: {0}".format(s))
                            value = hashlib.md5(bytes(s,'utf-8')).hexdigest()
                            if value == "6d90ca30c5de200fe9f671abb2dd704e":
                                print("FOUND: {0}".format(s))
                                quit()
```


Result:

```
Testing: h4rd~hue
Testing: h4rd~huf
Testing: h4rd~hug
Testing: h4rd~huh
FOUND: h4rd~huh
```

## Fourth part

The last part of the flag is given by a native function `stringFromJNI`.
We write a Frida hook for that:

```javascript
'use strict';

console.log("[*] INSIDE native-lib.js");

global.run = function () {
    console.log("[debug] global.run: Java.available="+Java.available);
    
    Java.perform(function () {

	var mainClass = Java.use("lu.hack.Flagdroid.MainActivity");
	mainClass.stringFromJNI.implementation = function() {
	    console.log("[*] Hooking");
	    var ret = this.stringFromJNI();
	    console.log("ret="+ret);
	    return ret;
	}

	mainClass.checkSplit1.implementation = function(b) {
	    var ret = this.stringFromJNI();
	    console.log("ret="+ret);
	    return this.checkSplit1(b);
	}
	    
	
	console.log("[*] loaded hooks - v4");
    });
};
```

Then, we launch the Frida server on the smartphone, launch the app, and on the laptop, launch Frida client: `frida -U -l native-lib.js -n lu.hack.Flagdroid`

```
[*] Hooking
ret=0r~w4S-1t?8)
```

## Final solution

`flag{tH4T_w45N-T~so_h4rd~huh_0r~w4S-1t?8)}`
