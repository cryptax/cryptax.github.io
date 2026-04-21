---
layout: post
title: "Notes (Insomni'hack CTF 2024)"
author: cryptax
date: 2024-04-27
tags:
- Insomnihack
- CTF
- 2024
- android
- crypto
---

In this challenge at Insomni'hack CTF, you get an *Android application* (apk) to analyze. Had to look at that, obviously :wink:.
The challenge was solved as team work with 2 other team mates from *Soudure au Beurre*.

*Update April 20, 2024: PKCS7 and PKCS5 are basically the same. Corrected the explanation.*

## Understanding the Android application

In this challenge, you get an Android application. I decompile it to find a pretty simple application with 2 classes: `MainActivity` and `Part1`.

The main activity asks for credentials to login. Those are hard-coded in the application:

```java
  private boolean isValidCredentials(String s, String s1) {
                return (s.equals("admin")) && (s1.equals("fbG6pXd0wm1cz"));
            }
```

![](/images/ins24-login.png)

This launches an other activity, `Part1`. It also sends the password as extra data, but actually this is not used, perhaps just there to confuse the player.

```java
if(this.isValidCredentials(s, s1)) {
  Intent intent0 = new Intent(MainActivity.this, Part1.class);
  intent0.putExtra("password", s1);
  MainActivity.this.startActivity(intent0);
  return;
}
```

`Part1` activity reads an encrypted note from a file in the assets and displays it.

```java
String s = Part1.getJSONData(this, "storage.json");
try {
  s1 = new JSONObject(s).getString("note");
}
```

The content of `storage.json` is the following. It hints that AES-256 is used, which will be confirmed later.

```json
{
  "AES-Type": "256",
  "note": "uS0D11dq3RM9QimRWfXcewwQdoxYwrZRNUGT205pDfQ="
}
```

The end-user must enter a *passphrase* to decrypt the note. In the following, `s1` is the encrypted note and `s` is the passphrase.

```java
public void onClick(View view0) {
  String s = Part1.this.editTextPassphrase.getText().toString().trim();
  if(!s.isEmpty()) {
  try {
    ((TextView)Part1.this.findViewById(id.textViewFLAG2)).setText(this.decrypt(s1, s));
	...
```	

![](/images/ins24-note.png)


The decryption algorithm is based on AES-ECB + an initial Base64 decoding. Note there are several iterations of AES.

```java
private String decrypt(String s, String s1) throws Exception {
	SecretKeySpec secretKeySpec0 = new SecretKeySpec(s1.getBytes(), "AES/ECB/PKCS7Padding");
    byte[] arr_b = Base64.decode(s, 0);
    Cipher cipher0 = Cipher.getInstance("AES/ECB/NoPadding");
    cipher0.init(2, secretKeySpec0);
    for(int v = 0; v < 313370; ++v) {
        arr_b = cipher0.doFinal(arr_b);
    }

	Cipher cipher1 = Cipher.getInstance("AES/ECB/PKCS7Padding");
    cipher1.init(2, secretKeySpec0);
    return new String(cipher1.doFinal(arr_b), "UTF-8");
}
```

If we perform Base64 decoding on the secret note, we have 32 bytes. As hinted previously ("AES-256"), we are going to need a 256-bit key (32 bytes).

```
>>> import base64
>>> note = 'uS0D11dq3RM9QimRWfXcewwQdoxYwrZRNUGT205pDfQ='
>>> len(base64.b64decode(note))
32
```


## Analysis

So, we must find a 32-byte AES key which correctly decrypts the encrypted note. 
We search in the app for a hidden key, or hints for the key, but there are none.
The encrypted note is likely to be the flag, thus to begin with `INS{`. Or, we might have an extra step of MD5 hash, because the application includes the following function - but it is never called (strange).

```java
private String generateFlag(String s) {
  StringBuffer stringBuffer0;
  try {
    MessageDigest messageDigest0 = MessageDigest.getInstance("MD5");
	messageDigest0.update(s.getBytes());
	byte[] arr_b = messageDigest0.digest();
	stringBuffer0 = new StringBuffer();
	for(int v = 0; v < arr_b.length; ++v) {
	  stringBuffer0.append(String.format("%02X", ((byte)arr_b[v])));
	}
  }
  catch(NoSuchAlgorithmException noSuchAlgorithmException0) {
	noSuchAlgorithmException0.printStackTrace();
	return "";
  }
  return "FLAG1: INS{" + stringBuffer0.toString() + "}";
}
```	

Even if the decrypted node directly decrypts to the flag, knowing the first 4 characters `INS{` is going to be insufficient to find the 32 bytes...

After a while, the organizers publish a hint saying "have you heard about rockyou?". Rockyou is a famous password list, so this suggests we need to *bruteforce* the password. 

## Implementing the solution

We download `rockyou` password list and only keep 32-byte passwords. Actually, this is **new** to me. I thought that `rockyou` only included or obvious passwords (`123456`, `admin`...), I wouldn't have thought it included *32-byte long passwords*, but yes, there are a couple. In particular, one of them catches my eye: `letsyouupdateyourfunnotesandmore` because it includes the word `notes`.
I try it, bad luck it's not the correct one.

So, I go on and implement a bruteforce program. I use Java to be able to re-use code from the application. There are a few things to adapt. In Java, Base64 does not come from `android.util.Base64`, and decoding is performed via a *decoder*: `Base64.getDecoder().decode(s)`. Also, there are 2 surprises:

1. The Android code builds the AES key with `SecretKeySpec secretKeySpec0 = new SecretKeySpec(s1.getBytes(), "AES/ECB/PKCS7Padding");`. This does not work in "pure" Java, padding and block mode should not be specified in the `SecretKeySpec`. It's a bit surprising this works on Android. 	`SecretKeySpec sks = new SecretKeySpec(s1.getBytes("UTF-8"), "AES");`

2. Java complains it does not have any PKCS7 padding provider. But [PKCS7 is basically the same as PKCS5](https://en.wikipedia.org/wiki/Padding_(cryptography)#:~:text=PKCS%235%20padding%20is%20identical,number%20a%20byte%20can%20contain.), so we can use PKCS5... actually we don't even padding in this particular case because the encrypted note is 32-byte long.... 

I compile the program, and run it on `rockyou32.txt`. The solution is output in the blink of the eye:

```
$ java DecryptNotes 
Reading rockyou32.txt
passphrase=letsyouupdateyourfunNotesandmore decrypted notes=INS{H4PPY_H4CK1N6}
Solution: INS{H4PPY_H4CK1N6}
```

## Opinion

I love any *Android* based challenge, and I was happy to see one at Insomni'hack. Moreover, this one required decompilation of the application, not some other Android trick. I love decompiling Android apps :) I was however a bit disappointed the solution involved *bruteforcing*. IMHO, bruteforce is for the weak :smile:. 

I wonder why the MD5 function was included.


## Appendix: Code

```java
import java.io.*;
import java.security.NoSuchAlgorithmException;
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.BadPaddingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

public class DecryptNotes {
    public static final String ENCRYPTED_NOTE = "uS0D11dq3RM9QimRWfXcewwQdoxYwrZRNUGT205pDfQ=";
    
    public static String decrypt(String s, String s1) throws IllegalBlockSizeException, UnsupportedEncodingException, NoSuchPaddingException, BadPaddingException, InvalidKeyException, NoSuchAlgorithmException {
	SecretKeySpec sks = new SecretKeySpec(s1.getBytes("UTF-8"), "AES");
	byte[] decoded_bytes = Base64.getDecoder().decode(s);
	Cipher cipher0 = Cipher.getInstance("AES/ECB/NoPadding");
	cipher0.init(2, sks);
	for(int v = 0; v < 313370; ++v) {
	    decoded_bytes = cipher0.doFinal(decoded_bytes);
	}

	Cipher cipher1 = Cipher.getInstance("AES/ECB/PKCS5Padding");
	cipher1.init(2, sks);
	return new String(cipher1.doFinal(decoded_bytes), "UTF-8");
    }

    public static String bruteforce(String filename) {
	System.out.println("Reading "+filename);
	try ( BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            String line;
            while ((line = reader.readLine()) != null) {
		try {
		    String decrypted = DecryptNotes.decrypt(DecryptNotes.ENCRYPTED_NOTE, line);
		    System.out.println("passphrase="+line+" decrypted notes="+decrypted);
		    return decrypted;
		}
		catch(NoSuchPaddingException e) {
		    System.out.println("NoSuchPaddingException: "+e.getMessage());
		}
		catch (IllegalBlockSizeException e) {
		}
		catch (BadPaddingException e) {
		}
		catch (InvalidKeyException e) {
		    System.out.println("InvalidKeyException: "+e.getMessage()+" passphrase="+line);
		    e.printStackTrace();
		}
		catch (NoSuchAlgorithmException e) {
		    System.out.println("NoSuchAlgorithmException: "+e.getMessage());
		    e.printStackTrace();
		}
		catch (UnsupportedEncodingException e) {
		    System.out.println("UnsupportedEncodingException: "+e.getMessage());
		}
            }
        } catch (IOException e) {
	    System.out.println("IOException: "+e.getMessage());
        }
	return null;
    }

    public static void main(String args[]) {
	String filename;
	if (args.length != 1)
	    filename = new String("rockyou32.txt");
	else
	    filename = args[1];

	System.out.println("Solution: "+DecryptNotes.bruteforce(filename));
	
    }
}
```

         
