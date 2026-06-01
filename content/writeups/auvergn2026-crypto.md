---
layout: post
title: "Auvergn'hack 2026: Crypto challenge"
author: cryptax
date: 2026-06-01
tags:
- Auvergn'hack
- CTF
- 2026
- Crypto
- ChaCha20
- HMAC
- Nonce
---

## Auvergn'hack 2026 CTF: Crypto - Saki

There was a crypto challenge at Auvergn'hack. I can't remember exactly the title (I think it was "Saki" or something). I did not preserve the description either, but the only important files that were provided were:

- an encrypted message (`message.txt`):

```json
{"h": "g1oatFTHSYsUH377iZQSuesUM/t+pFXRrwCrNW8v8Lw=", "n": "WQq7B4XEueM=", "m": "2gFQlO5+YZFnqaar7QEGpu3/f/2WdbJEWPnVjNuNs2dXyHUi7/8="}
```

- a verification file (`v.txt`), with a single word: `Monkey`
- a Python script (`script.py`) to encrypt or decrypt messages

**Goal**: we need to decrypt `message.txt`

### Code analysis

The `decrypt` function in `script.py` shows we are supposed to have a key file, `key.txt`. But **we don't have that**.


```python
def decrypt():
    ct = input("message> ")
    h_v = input("key verification> ")
    nonce = input("nonce> ")
    key = read_f("key.txt")
    if verify(b64decode(h_v),key):
        cipher_d = ChaCha20.new(key=pad(key,32),nonce=b64decode(nonce))
        print(cipher_d.decrypt(b64decode(ct)))
```

The `gen_verification` function explains the format of `message.txt`:

- `h` is a base64 encoding of the keyed HMAC-SHA256 of file `v.txt` with the key
- `n` is a base64 encoded nonce
- `m` is a base64 encoded ciphertext

```python
def gen_verification(ciphertext, nonce, key):
    v = read_f("v.txt") 
    h = HMAC.new(key, v, digestmod=SHA256)
    nonce = b64encode(nonce).decode("utf-8")
    ct = b64encode(ciphertext).decode("utf-8")
    h_v = b64encode(h.digest()).decode("utf-8")
    print(json.dumps({'h': h_v, 'n': nonce, 'm': ct}))
```

Consequently, to decrypt the message, we theoretically need to launch `script.py` and provide:

| Script prompt | Explanation              | Value |
| ------------- | ------------------------ | ----- |
| message> | ciphertext (`m`) from `message.txt` | `2gFQlO5+YZFnqaar7QEGpu3/f/2WdbJEWPnVjNuNs2dXyHUi7/8=` |
| key verification> | `h` field of `message.txt` | `g1oatFTHSYsUH377iZQSuesUM/t+pFXRrwCrNW8v8Lw=` |
| nonce> | `n` field of `message.txt` | `WQq7B4XEueM=` |

The "only" issue is that we don't have the **key**...

### A word about ChaCha20

ChaCha20 is a stream cipher. From a *key* and a *nonce*, a random key stream is generated, and XORed against the plaintext (or the ciphertext for decryption).

As it's XOR, the keystream must never be re-used. That's what the nonce is for: the nonce is unique, and ensure the key can be re-used, as long as the nonce is never re-used.

Typically, ChaCha20 is used along with an HMAC or Poly1305 mechanism, to guarantee the ciphertext hasn't been *modified* by an attacker (ChaCha20 is an encryption algorithm, it only protects confidentiality). To do so, in addition to the *ciphertext*, people usually produce a *tag* which is the HMAC of the ciphertext with the key. If the *tag* is verified, this ensures that the ciphertext hasn't been modified.

In the CTF challenge's implementation, that's not what happens. We are **not** doing a HMAC of the ciphertext but of a short "verification message". The verification message (`v.txt` = "Monkey") is known. So, if the `HMAC(key, v.txt)` is correct, we know the *key* is correct. This acts as a key validation procedure. Strange... but the puzzle solves in the next paragraph.

### The trick with the key


Two observations stand out:

1. In the `decrypt` function, we notice the **key is padded to 32 bytes**: `cipher_d = ChaCha20.new(key=pad(key,32),nonce=b64decode(nonce))` ... which means that **the key is very probably shorter**!

2. We know `h` and `v`, i.e we know that: `b64encode(HMAC.new(key, "Monkey", digestmod=SHA256)) = g1oatFTHSYsUH377iZQSuesUM/t+pFXRrwCrNW8v8Lw=`

So, we are going to **bruteforce** the key with this property. We'll download a known password list, and test their HMAC value until we find the right one.

Will this work? It's not totally guaranteed (our password list may be too short + we're not exactly certain how long the key will be), but there are strong hints this is the way to go:

- There's isn't any other way to solve the challenge, so the key is likely to be short for the challenge to be do-able.
- It explains the usual use of `h`, which HMAC the key with `v`, only to help us verify the key is correct.

### Key recovery

I wanted to bruteforce script to be fast, so I opted for an implementation in C.
The implementation is not tricky, a perfect job for AI. It uses OpenSSL EVP. 

```c
// For each word in wordlist:
HMAC(EVP_sha256(), line, klen,
     V_MSG, V_LEN,  // "Monkey", 6 bytes
     hmac_out, &hmac_len);

if (memcmp(hmac_out, TARGET_HMAC, 32) == 0) {
    printf("[+] KEY FOUND: \"%.*s\"\n", klen, line);
    return 0;
}
```

I downloaded a password list from [SecList](https://github.com/danielmiessler/SecLists/blob/master/Passwords/Common-Credentials/100k-most-used-passwords-NCSC.txt) and ran the program:

```
$ gcc -O2 -o crack crack.c -lcrypto
$ ./crack /usr/share/wordlists/100k-most-used-passwords-NCSC.txt
[*] Brute-forcing HMAC-SHA256(key, "Monkey") against known hash...
[+] KEY FOUND after N attempts: "cheekymonkey" (len=12)
```

NB. The program returns very fast. Probably, an implementation in C wasn't necessary and Python would have worked.

### Decrypting the message

The next step is trivial: we have the key, the ciphertext, the nonce. We simply need to decrypt.

```python
import json
from base64 import b64decode
from Crypto.Cipher import ChaCha20
from Crypto.Util.Padding import pad

with open("message.txt") as f:
    data = json.load(f)

key = b"cheekymonkey"
h_v = b64decode(data["h"])
nonce = b64decode(data["n"])
ct = b64decode(data["m"])

cipher = ChaCha20.new(key=pad(key, 32), nonce=nonce)
plaintext = cipher.decrypt(ct)
print(f"Decrypted: {plaintext.decode()}")
```

No need for AI for this piece of code :P

```python
python3 mydecrypt.py
Decrypted: ZiTF{23135dbf5cdb57b80d18d506dc0b0a21}
```

Flag: `ZiTF{23135dbf5cdb57b80d18d506dc0b0a21}`

### Conclusion

An interesting relatively easy crypto challenge around ChaCha20. I was perhaps slightly disappointed the solution relied on bruteforcing (not very "noble"), but the fact was smartly hinted by the unusual use of the verification message.

This writeup demonstrates what I call a "reasonable" use of AI: we solve the challenge with "assistance", but still entirely understand the concept. In particular, we use AI to generate an implementation (in C) which is not "difficult", but a bit boring (moreover, actually, C was overkill and Python would have worked).

### Full script to find the key

The script was generated by AI (I can't remember which LLM I used).
I would have far quicker to generate it in Python (even by hand), but I wanted to ensure this would run quick, so I preferred an implementation in C.

```c
/*
 * crack.c - Brute-force key for ChaCha20/HMAC-SHA256 CTF challenge
 *
 * The script uses:
 *   HMAC-SHA256(key=key, msg="Monkey") -> published as h
 *   ChaCha20(key=PKCS7_pad(key,32), nonce=n) -> published as m
 *
 * Since the key is PKCS7-padded to 32 bytes, it must be < 32 bytes.
 * We brute-force candidate keys from a wordlist, compute HMAC-SHA256,
 * compare with the known h, then decrypt with ChaCha20 on match.
 *
 * Usage:
 *   ./crack <wordlist>
 *   ./crack /usr/share/wordlists/words
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

/* ---- Known values from message.txt (base64-decoded) ---- */

/* h = base64("g1oatFTHSYsUH377iZQSuesUM/t+pFXRrwCrNW8v8Lw=") */
static const uint8_t TARGET_HMAC[32] = {
    0x83,0x5a,0x1a,0xb4,0x54,0xc7,0x49,0x8b,
    0x14,0x1f,0x7e,0xfb,0x89,0x94,0x12,0xb9,
    0xeb,0x14,0x33,0xfb,0x7e,0xa4,0x55,0xd1,
    0xaf,0x00,0xab,0x35,0x6f,0x2f,0xf0,0xbc
};

/* n = base64("WQq7B4XEueM=") */
static const uint8_t NONCE[8] = {
    0x59,0x0a,0xbb,0x07,0x85,0xc4,0xb9,0xe3
};

/* m = base64("2gFQlO5+YZFnqaar7QEGpu3/f/2WdbJEWPnVjNuNs2dXyHUi7/8=") */
static const uint8_t CIPHERTEXT[] = {
    0xda,0x01,0x50,0x94,0xee,0x7e,0x61,0x91,
    0x67,0xa9,0xa6,0xab,0xed,0x01,0x06,0xa6,
    0xed,0xff,0x7f,0xfd,0x96,0x75,0xb2,0x44,
    0x58,0xf9,0xd5,0x8c,0xdb,0x8d,0xb3,0x67,
    0x57,0xc8,0x75,0x22,0xed,0xff
};
static const size_t CT_LEN = sizeof(CIPHERTEXT);

/* v.txt content */
static const uint8_t V_MSG[]  = "Monkey";
static const size_t  V_LEN    = 6;

/* ---- PKCS7 pad key to 32 bytes ---- */
static void pkcs7_pad(const uint8_t *key, size_t klen, uint8_t *out) {
    size_t pad = 32 - klen;
    memcpy(out, key, klen);
    memset(out + klen, (uint8_t)pad, pad);
}

/* ---- Original ChaCha20 (8-byte nonce) implementation ---- */
/* OpenSSL's EVP_chacha20() is IETF variant (RFC 8439, 12-byte nonce).
   PyCryptodome with 8-byte nonce uses the original Bernstein variant,
   where state positions 12-13 = nonce, position 14 = counter (swapped vs IETF). */

static inline uint32_t rotr32(uint32_t v, int n) {
    return (v >> n) | (v << (32 - n));
}

static inline uint32_t rotl32(uint32_t v, int n) {
    return (v << n) | (v >> (32 - n));
}

static inline void le32_pack(uint8_t *buf, uint32_t val) {
    buf[0] = (uint8_t)(val);
    buf[1] = (uint8_t)(val >> 8);
    buf[2] = (uint8_t)(val >> 16);
    buf[3] = (uint8_t)(val >> 24);
}

static inline uint32_t le32_unpack(const uint8_t *buf) {
    return (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
           ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
}

static void chacha20_quarterround(uint32_t *x) {
    x[0] += x[1]; x[3] ^= x[0]; x[3] = rotl32(x[3], 16);
    x[2] += x[3]; x[1] ^= x[2]; x[1] = rotl32(x[1], 12);
    x[0] += x[1]; x[3] ^= x[0]; x[3] = rotl32(x[3], 8);
    x[2] += x[3]; x[1] ^= x[2]; x[1] = rotl32(x[1], 7);
}

static void chacha20_block(uint32_t state[16], uint8_t out[64]) {
    uint32_t w[16];
    memcpy(w, state, sizeof(w));
    for (int i = 10; i; i--) {
        /* Column rounds */
        chacha20_quarterround(&w[0]);
        chacha20_quarterround(&w[4]);
        chacha20_quarterround(&w[8]);
        chacha20_quarterround(&w[12]);
        /* Diagonal rounds */
        chacha20_quarterround(&w[0]);
        chacha20_quarterround(&w[5]);
        chacha20_quarterround(&w[10]);
        chacha20_quarterround(&w[15]);
    }
    for (int i = 0; i < 16; i++) {
        state[i] += w[i];
        le32_pack(out + i * 4, state[i]);
    }
}

static int chacha20_decrypt(const uint8_t *padded_key,
                            const uint8_t *nonce8,
                            const uint8_t *ct, size_t ct_len,
                            uint8_t *out) {
    /*
     * Original Bernstein ChaCha20 state (8-byte nonce):
     *   pos 0-3:    constants "expand 32-byte k" (little-endian)
     *   pos 4-11:   key (32 bytes = 8 x uint32 LE)
     *   pos 12-13:  nonce (8 bytes = 2 x uint32 LE)
     *   pos 14:     counter (starts at 0)
     *   pos 15:     0
     */
    uint32_t state[16] = {
        0x61707865, 0x3320646e, 0x79622d32, 0x6b206574,
        le32_unpack(padded_key + 0),   le32_unpack(padded_key + 4),
        le32_unpack(padded_key + 8),   le32_unpack(padded_key + 12),
        le32_unpack(padded_key + 16),  le32_unpack(padded_key + 20),
        le32_unpack(padded_key + 24),  le32_unpack(padded_key + 28),
        le32_unpack(nonce8 + 0),       le32_unpack(nonce8 + 4),
        0,  /* counter */
        0   /* spare */
    };

    size_t offset = 0;
    uint32_t counter = 0;
    while (offset < ct_len) {
        state[14] = counter++;
        uint8_t keystream[64];
        chacha20_block(state, keystream);
        size_t chunk = 64;
        if (offset + chunk > ct_len) chunk = ct_len - offset;
        for (size_t i = 0; i < chunk; i++)
            out[offset + i] = ct[offset + i] ^ keystream[i];
        offset += chunk;
    }
    return (int)ct_len;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <wordlist>\n", argv[0]);
        return 1;
    }

    FILE *wl = fopen(argv[1], "r");
    if (!wl) {
        perror("fopen wordlist");
        return 1;
    }

    char line[256];
    uint8_t hmac_out[32];
    unsigned int hmac_len = 32;
    uint8_t padded_key[32];
    uint8_t plaintext[256];
    long count = 0;

    printf("[*] Brute-forcing HMAC-SHA256(key, \"Monkey\") against known hash...\n");
    printf("[*] Target: ");
    for (int i = 0; i < 32; i++) printf("%02x", TARGET_HMAC[i]);
    printf("\n\n");

    while (fgets(line, sizeof(line), wl)) {
        /* Strip newline */
        size_t klen = strlen(line);
        while (klen > 0 && (line[klen-1] == '\n' || line[klen-1] == '\r'))
            klen--;
        if (klen == 0 || klen >= 32) continue;  /* must be < 32 for PKCS7 to pad */

        count++;

        /* Compute HMAC-SHA256(key=line[:klen], msg="Monkey") */
        HMAC(EVP_sha256(),
             line, (int)klen,
             V_MSG, V_LEN,
             hmac_out, &hmac_len);

        if (memcmp(hmac_out, TARGET_HMAC, 32) == 0) {
            printf("[+] KEY FOUND after %ld attempts: \"%.*s\" (len=%zu)\n",
                   count, (int)klen, line, klen);
            fclose(wl);
            return 0;
        }
    }

    fclose(wl);
    printf("[-] Key not found in wordlist after %ld attempts.\n", count);
    return 1;
}
```