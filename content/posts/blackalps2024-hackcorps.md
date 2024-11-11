---
layout: post
title: "BlackAlps 2024 CTF HackCorps Warmup"
author: cryptax
date: 2024-11-10
tags:
- BlackAlps
- CTF
- 2024
- Docker
- Namespace
---

# HackCorps Warm Up

## Description

Serge the accountant inadvertently opened an infected file on his workstation. After investigation by the SOC team, the file was found to contain malware designed to extract confidential information. The SOC team asked you to help them investigate the incident. The SOC has found a folder (C:\exfiltr) but cannot recover its contents.

+ we get a file `exfiltr.7z`

## Solution

We unzip the file. There are 3 directories named 0, 1 and 2. Each of them contains text files.
For example this is `./0/1.txt`:

```
...
hn0Fr35h4KRmKQog8P43vML8RX230SGt6OzfiZy3lOb2+YrEjg7ZyXVweX0clW9AQ2+iB9iKUuWXkXO/Gbf/s/xhfaq8kkdN157FBk2z0ux/vYdAN9E+8LTvSdri2+yrXZQwG/M5neh6fJUQV54TH54alkLv0SGGijbFvQQvbJQNO89UjRBR3JbPJ22/kdHuD70e/ugQGufFK0vtGVrIDV79KR2sqfwQP3Jj06wa8e5Ts1/YoPginX8k4BjsEPfO253/xg1607uw9dXre131SU17W+uoaf7nfLah4WfPJUQJd6yYl6waH/6uYrTH3WNt3lvo8E3
```

We quickly recognize it is Base64 encoded. If we decode, the file is part of a PNG image:

```
$ file 1.unb64.png 
1.unb64.png: PNG image data, 1027 x 772, 8-bit/color RGBA, non-interlaced
```

So, we concatenate all text files of the directory (`cat 1.txt 2.txt 3.txt | base64 -d > total.png`) and we get a complete PNG. The image of directory `0` shows a Windows login screen.
In directory `1`, a user named *serge* is trying to login.

In directory `2`, we get the flag.

![](/images/blackalps24-corpsflag.png]

## Conclusion

It was easy, but I'm always in favor for a few easy challenges :)

# Docker challenges: Hidden Secrets

## Description

Hidden secrets in docker containers. There are three flags. The flags are not dependent to each other. Docker Secrets 1: Find and submit the FLAG_FILE

Network ports in scope (to save you an nmap):

HTTP on 80/tcp
SSH on 22/tcp

- docker-secrets.ctf.blackalps.ch

## Solution for flag 1

*This challenge was solved by a team mate, FdlSifu, and I helped a little.*

After an unconclusive run of Dirbuster on the website (`docker run --rm hypnza/dirbuster -u http://docker-secrets.ctf.blackalps.ch`), we finally identified a hidden paths, http://docker-secrets.ctf.blackalps.ch/flag, which suggested http://docker-secrets.ctf.blackalps.ch/docker-compose.yml

This is the docker compose file:

```
services:
  docker-secret:
    image: blackalps2024/docker-secret
    restart: unless-stopped
    build: 
      context: .
      dockerfile_inline: |
        FROM alpine:3.20.2 AS builder
        WORKDIR /
        RUN wget https://ziglang.org/download/0.13.0/zig-linux-x86_64-0.13.0.tar.xz && \
            tar xf zig-linux-x86_64-0.13.0.tar.xz
        WORKDIR /build
        COPY . .
        RUN cp src/main.zig docker-compose.* build.zig build.zig.zon src/www && \
            /zig-linux-x86_64-0.13.0/zig build 

        FROM scratch
        WORKDIR /
        COPY --from=builder /build/zig-out/bin/docker-secret .
        CMD [ "/docker-secret" ]
    read_only: true
    hostname: ${FLAG_HOSTNAME}
    ports:
      - "80:8080"
    secrets:
      - flag
  helper:
    image: blackalps2024/docker-helper
    restart: unless-stopped
    build:
      context: .
      dockerfile_inline: |
        FROM alpine:3.20.2
        WORKDIR /
        RUN apk add --no-cache openssh-server && \
            ssh-keygen -A && \
            echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
            echo "root:PleaseLetMeIn" | chpasswd
        COPY ./motd /etc/motd
        CMD [ "/usr/sbin/sshd", "-D", "-e" ]
    read_only: true
    pid: "service:docker-secret"
    ports:
      - "22:22"
    depends_on:
      - docker-secret

secrets:
  flag:
    file: ${FLAG_FILE}
```

First, we notice the root password `PleaseLetMeIn` to log onto the host with SSH. This will have us log in the `docker-helper` container, while the secret flag file is in another container, `docker-secret`.

We log in:

```
 ssh -o PubkeyAuthentication=no -o PasswordAuthentication=yes root@10.159.0.37
root@10.159.0.37's password: 
Welcome to Alpine!

This is standard Alpine in docker with the following setup:

* OpenSSH server running
* Docker instance running in read-only mode
* The challenge does *not* require a docker escape

Enjoy!
75a91491c843:~#
```

We see the `docker-secret` runs with PID 1. So, we investigate `/proc/1/` and find `/proc/1/root/run/secrets/flag`:

75a91491c843:/proc/1/root/run/secrets# cat flag 
BA24{m0uNt_n@m3spaCe_rOck5}

## Solution for flag 2

The Docker challenges have another flag, they say it is related to the *hostname*. So, we need to find the hostname of the docker secret container.

The `nsenter` is particularly useful to inspect namespaces and other containers. So, once we are logged in the docker-helper container, we try to inspect the docker container with PID 1 (`-t 1`). Option `-u` preserves the container's hostname and various other environment variables. Option `-i` executes a command. So, we launch a shell for the other container and read its hostname.

```
75a91491c843:~# nsenter -t 1 -u -i sh
~ # hostname
BA24{D1d_U_Kn0w_@bout_UTS_n@Mspac3}
```

## Conclusion

Interesting challenge! I knew Docker containers were not secure towards one another, but had never experienced with that.









