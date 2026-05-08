---
layout: post
title: "Breach at SST 2"
author: cryptax
date: 2026-5-08
tags:
- THCon
- CTF
- 2026
- Forensics
- LUKS
---

## Don't forget to lock

This is the first challenge of the Forensics category.

```
We seized a suspect's computer and managed to capture a RAM dump before it was powered off, along with an encrypted disk. Your objective is to decrypt the drive.
```

We get a `chall.tar.gz`

### Solving the challenge

The tar.gz contains 2 files: an ELF and a raw:

```
drwxr-xr-x aurel/aurel       0 2026-03-09 16:46 files/
-rw-r--r-- aurel/aurel 1072693248 2026-03-09 16:46 files/disk.raw
-rw------- aurel/aurel 2264616099 2026-03-09 16:42 files/dump.elf
```

As the flag format is `THC{` or `THCON{`, we grep strings for `THC`:

```
$ strings dump.elf | grep THC
...
THCON{v1tl0ck3r_1n_MEm}
```

So easy, you don't need AI, right? 😜

