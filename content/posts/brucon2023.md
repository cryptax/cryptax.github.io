---
layout: post
title: "BruCON 2023 CTF"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- stegano
- ICS
- PLC
- S7
---
# Web

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

# DNS

## Do Not Search 1

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

## Do Not Search 3

Same, but you need to search for `brucon-ctf3.be`. Actually the same solution worked again: `dig brucon-ctf3.be txt` and revealed the flag: `TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3` (without trailing dot at the end).

```
;; ANSWER SECTION:
brucon-ctf3.be.		13774	IN	CNAME	TORFLAG.ECCBC87E4B5CE2FE28308FD9F2A7BAF3.
```

## Do Not Search 2

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

# CTI

Those were several challenges on Critical Infrastructure. For all of them, we had a common pcap and smbp file

## Find the PDF

This challenge asked *"Find the PDF document file name within the network packet capture"*. 

We didn't even need to load the Pcap in Wireshark. Mere strings on the pcap and grepping on pdf revealed the name of the PDF. The flag was the name of the PDF.

```
$ strings Incident1.1.1.pcap | grep pdf
STOR Architecture Dossier-BruCON Brewery V2.0.pdf
MDTM 20230901120136 Architecture Dossier-BruCON Brewery V2.0.pdf
...
```

## Which subnet

*"Which subnet from the enterprise domain network exfiltrates the PDF document?"*

![](/images/brucon2023-ftp.png)

IP address 192.168.124.128 is getting the PDF via FTP. The FTP server is 192.168.170.215. We struggled a long time on this one, because we thought we had to supply `192.168.0.0/16` as answer, and it did not work.

In the end, I asked an organizer who made me understand he wasn't expecting a *technical representation* of the subnet but it's name in the network architecture diagram. This diagram was supplied in a PDF for the CTI Architecture challenges.

![](/images/brucon2023-marketing.png)

We see the PDF is exfiltrated from a workstation in the Marketing subnet. *Marketing* was the flag.

## Devices

*"How many devices did communicate with the PLC"*

Devices who communicate with a PLC use the ModBus protocol. We can filter such packets in Wireshark with the filter "mdtcp" (or we could have filtered communications to the PLC's IP address).

In ModBus we have communications between 192.168.174.242 and

- 192.168.170.240 (SCADA)
- 192.168.174.99 (PLC)
- 192.168.174.245 (PLC)

To be honest, when I flagged I hadn't understood this at all, and tried 1, then 2 and happily got the flag with answer `3`...

## Mac Addy

*"What is the MAC address of the PLC?"*

I solved this one by looking at the file `GRC_known_good.smbp` first. It's an XML file and we see this IP address:
```
          <IpAddress>192.168.174.242</IpAddress>
          <GatewayAddress>192.168.174.1</GatewayAddress>
```
So, then I looked for `192.168.174.242` in Wireshark and found its MAC address: `00:80:f4:0e:8d:13`. This was the flag.

# My opinion

Positive aspects:

- It is really interesting to have  a full document with network and architecture diagrams of what looks like a real beer brewery. It makes the challenges feel real and I appreciated the time the organizers took to prepare such a scenario
- I enjoyed the PLC challenges where we used and hacked into real PLCs. We don't have PLC-related challenges very often in CTFs (because it's difficult to set up) and I found that lovely.
- I was a bit reluctant on the CTI architecture challenges where we had to go and explain our solution to organizers to get the flag. I wasn't at ease with that at the beginning, but in the end, (1) organizers weren't trying to trap us and would give flags quite freely if they had heard more or less what they wanted, and (2) it helped work on challenges that were *real*. Working on paper, diagrams etc is also part of the job sometimes, so it's a good idea.
- Several challenges were very easy giving everybody the opportunity to flag at least a couple of times.

Negative aspects:

- There were perhaps too many of those very very very easy challenges.
- We need a flag format, and the flag format must be used for every challenge. We don't want to have to guess "is that the flag they are waiting for, or should I find something more?".
- Some descriptions were misleading, like the PLC challenges were requesting an UUID as flag... but error it wasn't a UUID.
- Too many points were attributed to the CTI Architecture challenges IMHO. The issue is that the borders of each challenge in that section weren't well defined. For instance, if you find an issue with RDP, you don't know if that's the first issue, the second or the third. Fortunately, organizers were cool on that.
- There were too many "unconventional" challenges (CTI, CTI Architecture) and not enough "standard" challenges in contrast. I did appreciate the unconventional ones though, just perhaps the ratio was a bit too much.
- Do not create challenges where the same solution can be applied to different challenges like DNS 1 and DNS 2, or Web 1 and Web 2. I believe this is a testing issue.








