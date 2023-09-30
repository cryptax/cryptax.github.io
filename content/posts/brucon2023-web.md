---
layout: post
title: "BruCON 2023 CTF Web and DNS challenges"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- Web
- DNS
---

This is a write-up for BruCON CTF Web and DNS challenges

## Web: Baby LFI

The description says: *"Local File Inclusions are so cool! Can you get the flag at /flag.txt?"* and leads to a web page which shows this piece of PHP code:

```php
<?php isset($_GET['file']) ? include_once($_GET['file']) : highlight_file('index.php') ?>
```

I tried http://URL/?file=flag.txt which returned several interesting warning messages.

```
Warning: include_once(flag.txt): Failed to open stream: No such file or directory in /var/www/html/index.php on line 1

Warning: include_once(): Failed opening 'flag.txt' for inclusion (include_path='.:/usr/local/lib/php') in /var/www/html/index.php on line 1
```

They tell me that the file `flag.txt` is not found in the same directory as `index.php`. Actually, the description said to try `/flag.txt`, so that's what I tried http://URL/?flag=/flag.txt and got the flag `FLAG={Th4tW4s34444sy!}`. Yes, that was easy indeed.

## Web: Beginner LFI

Very similar to Baby LFI, this one says *"That last one was a bit too easy. Let's see if you can get past my filters! Can you get the flag at /flag.txt?"*

I probably did not understand what additional security there was, because the same request, http://URL/?flag=/flag.txt flagged: `FLAG={R3curs1v3Ch3cksAr3N33ded!}`


## DNS: Do Not Search 1

This DNS challenge says: *"Do Not Search for the answer in brucon-ctf1.be."*. 

I immediately thought something would be hidden in a DNS answer. I tried `dig brucon-ctf1.be` which did not provide any useful information. 

I searched for "dns CTF" and found this interesting [writeup](https://ctftime.org/writeup/8874). I tried the `txt` command and it revealed the flag:

```
 dig brucon-ctf1.be txt

; <<>> DiG 9.18.12-0ubuntu0.22.04.3-Ubuntu <<>> brucon-ctf1.be txt
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 47012
;; flags: qr rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 65494
;; QUESTION SECTION:
;brucon-ctf1.be.			IN	TXT

;; ANSWER SECTION:
brucon-ctf1.be.		1121	IN	TXT	"TORFLAG:602C0CC369C134E25861F335EC1F290E"
```

The flag is `TORFLAG:602C0CC369C134E25861F335EC1F290E`. The flag format wasn't fixed in this CTF.

## DNS: Do Not Search 3

Same, but you need to search for `brucon-ctf3.be`. Actually the same solution worked again: `dig brucon-ctf3.be txt` and revealed the flag: `TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3` (without trailing dot at the end).

```
;; ANSWER SECTION:
brucon-ctf3.be.		13774	IN	CNAME	TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3.
```

## DNS: Do Not Search 2

Find information on `brucon-ctf2.be`, but this time `txt` did not provide the flag. This information was in the TXT record:

```
;; ANSWER SECTION:
brucon-ctf2.be.		7168	IN	TXT	"v=spf1 a mx ip4:185.182.56.120 a:spf.spamexperts.axc.nl ~all"
```

As this was about spam and mail MX record, I tried the MX record:

```
dig brucon-ctf2.be mx
...
;; QUESTION SECTION:
;brucon-ctf2.be.			IN	MX

;; ANSWER SECTION:
brucon-ctf2.be.		4303	IN	MX	20 TORFLAG.BC3C1A70072998AFA42892CF9F880DB4.
brucon-ctf2.be.		4303	IN	MX	10 mail.brucon-ctf2.be.
```

The flag did not have the trailing dot: `TORFLAG.BC3C1A70072998AFA42892CF9F880DB4`.




