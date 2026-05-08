---
layout: post
title: "THCon 2026 - Forensics Challenges"
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

## Breach at SST 2

This is the *third* challenge of the Forensics category, but strangely I found the solution to this one before the second (although they are meant to be linked...).

In this challenge, we get a 2G bootable drive `scavos.img`.

```
S.N.A.F.U. agents intercepted Viktor Crypt during a meeting with his accomplice. They both fled, but Viktor dropped his bootable drive in the rush.

Boot it up and find out what he was up to inside the SST Dynamics factory network.

You can find the drive here

N.B.: flag format : THCON{...}
```

### Mounting the disk

`fdisk` shows the image has 3 partitions:

```
Device      Boot   Start     End Sectors  Size Id Type
scavos.img1 *       2048  206847  204800  100M  c W95 FAT32 (LBA)
scavos.img2       206848 3500031 3293184  1.6G 83 Linux
scavos.img3      3500032 4194303  694272  339M 83 Linux
```

We assign a loop device, and mount the partitions:

```
$ sudo losetup --find --show -P scavos.img
/dev/loop27
$ sudo mkdir -p /tmp/scavos /tmp/scavos2 /tmp/scavos3
$ sudo mount /dev/loop27p1 /tmp/scavos
$ sudo mount /dev/loop27p2 /tmp/scavos2
```

We are unable to mount the 3rd partition, because we discover it's an encrypted LUKS container:

```
$ sudo mount /dev/loop27p3 /tmp/scavos3/
mount: /tmp/scavos3: unknown filesystem type 'crypto_LUKS'.
dmesg(1) may have more information after failed mount system call.
```

To open the vault, we need a passphrase but we don't have that...

```
sudo cryptsetup luksOpen /dev/loop27p3 scavos3
Enter passphrase for /dev/loop27p3: 
```

### Finding the passphrase

The first partition has nothing interesting:

```
$ ls
config-6.12.79-0-virt  modloop-virt  System.map-6.12.79-0-virt
initramfs-virt         syslinux      vmlinuz-virt
```

The second partition has loads of information:

```
$ ls
bin   dev  home  lost+found  mnt  proc  run   srv  tmp  var
boot  etc  lib   media       opt  root  sbin  sys  usr
```

In `/var/log/auth.log`, we see repeated mounts of a LUKS vault on /dev/sda3.

```
grep luks auth.log 
Mar 10 08:14:33 scavos sudo[1500]: crypt : TTY=tty1 ; PWD=/home/crypt ; USER=root ; COMMAND=/sbin/cryptsetup luksOpen /dev/sda3 vault
Mar 10 08:31:08 scavos sudo[1512]: crypt : TTY=tty1 ; PWD=/home/crypt ; USER=root ; COMMAND=/sbin/cryptsetup luksClose vault
Mar 12 19:30:10 scavos sudo[1499]: crypt : TTY=tty1 ; PWD=/home/crypt ; USER=root ; COMMAND=/sbin/cryptsetup luksOpen /dev/sda3 vault
Mar 12 19:45:31 scavos sudo[1509]: crypt : TTY=tty1 ; PWD=/home/crypt ; USER=root ; COMMAND=/sbin/cryptsetup luksClose vault
```

In `/home/crypt/.local/share/weechat/logs/irc.xss-mesh.#ops.weechatlog`, we have WeeChat IRC log which mentions a REST API with a persistant store, and says the passphrase is in 5G traffic.

```
2125-02-13 16:42:10  CryptShadow  btw I poked around the robots' REST API while I was in there
2125-02-13 16:42:25  CryptShadow  they expose a /api/v1/memory endpoint, key/value persistent store
2125-02-13 16:42:40  CryptShadow  probably for state checkpoints between reboots
2125-02-13 16:43:08  CryptShadow  I can write whatever I want in there. nobody monitors it
2125-02-13 16:43:22  D1m1tr1      free cloud storage on enemy infrastructure, I love it
...
2125-02-28 22:08:30  CryptShadow  on my bootable drive, encrypted partition
2125-02-28 22:08:42  D1m1tr1      password?
2125-02-28 22:08:55  CryptShadow  I'll send it over the 5G channel, not here
2125-02-28 22:09:08  CryptShadow  you know how to read the capture
2125-03-02 10:31:30  CryptShadow  yeah, in the 5G traffic. you'll find it
```

In `/home/crypt/recon/robot_observations.txt`, we learn the company SST Dynamics has robots that communicate over a private 5G network + details:

```
...
Robots communicate over a private 5G standalone network.
The gNB antenna is on the factory rooftop, north-east sector.
Band n78 (3.5 GHz). Captured the signal with the SDR from
the adjacent building's parking lot.

UEs (robots) register to the network with encrypted SUCIs.
Protection scheme ECIES profile A (secp256r1 curve).
Managed to grab the Home Network private key from the
UDM server when I got access to the internal network.
=> see 5g_capture/sst_hn_privkey.pem
```



### Analyzing the PCAP

The pcap is located in `/home/crypt/recon/5g_capture/sst_north_sector.pcap`.
It's a 5G capture.

The PCAP contains two distinct layers of traffic:

| Traffic Type | Protocol Stack | Description |
|---|---|---|
| Control plane | NGAP / SCTP → NAS 5GS | Robot (UE) registrations with SUCI-concealed identities |
| User plane | GTP-U / UDP → IP → TCP → HTTP | Robot REST API communications with central controller |

In 5G standalone networks, UEs (robots here) never transmit their real identity (SUPI/IMSI) in the clear. Instead they send a SUCI — an ECIES-encrypted version of the MSIN portion. Only the Home Network UDM, holding the private key, can decrypt it. This privacy mechanism makes passive interception of UE identities impossible without the HN private key. 

The **5G NAS registration** traffic uses SUCI (Subscription Concealed Identifier) to hide each robot's SUPI (IMSI). The SUCIs are encrypted with ECIES (profile A, secp256r1), and Viktor even grabbed the home network private key from the UDM server (`/etc/open5gs/hnet/curve.key`) to decrypt them. 

The **GTP user plane** carries actual robot application traffic: HTTP/JSON REST API calls between the robots (10.0.3.x) and the central controller (10.0.2.1:80). This is unencrypted HTTP inside GTP tunnels.


The key insight is that `tshark --export-objects` can see through the GTP encapsulation and reassemble HTTP objects from the user plane. So, we extract objects from the PCAP:

```
cd /home/crypt/recon/5g_capture/
tshark -r sst_north_sector.pcap --export-objects http,./objects
```

The exported objects are voluminous:

```
...
(8)
5(9)
5g-aka-confirmation
5g-aka-confirmation(1)
5g-aka-confirmation(10)
5g-aka-confirmation(11)
...
```

Among the robot API calls, we are interested in the `store`:

```
s | grep store
store
store(1)
store(2)
store(3)
```

Recall this conversation on WeeChat IRC:

```
2125-02-13 16:42:10  CryptShadow  btw I poked around the robots' REST API while I was in there
2125-02-13 16:42:25  CryptShadow  they expose a /api/v1/memory endpoint, key/value persistent store
2125-02-13 16:42:40  CryptShadow  probably for state checkpoints between reboots
2125-02-13 16:43:08  CryptShadow  I can write whatever I want in there. nobody monitors it
2125-02-13 16:43:22  D1m1tr1      free cloud storage on enemy infrastructure, I love it
```

```
cat store*
{"key": "viktor_notes", "value": "remember to buy coffee for dimitri"}{"stored":true,"key":"viktor_notes"}{"key": "vault_key", "value": "d1m1tr1_0w3s_m3_c0ff33"}{"stored":true,"key":"vault_key"}
```

We have the vault key: `d1m1tr1_0w3s_m3_c0ff33`.

### Mounting the LUKS

```
$ sudo cryptsetup luksOpen /dev/loop27p3 scavos3
Enter passphrase for /dev/loop27p3: 
$ sudo mount /dev/mapper/scavos3 ./scavos3
$ cd ./scavos3 && ls
flag.txt  intercept.wav  lost+found  README_DIMITRI.txt  sigdb  vault_note.txt
$ cat ./flag.txt 
THCON{h0p3_y0u_gr4bb3d_c0ff33_f0r_th3_n3xt_st3p}
```

