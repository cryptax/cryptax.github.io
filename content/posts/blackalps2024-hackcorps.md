---
layout: post
title: "BlackAlps 2024 CTF HackCorps Warmup"
author: cryptax
date: 2024-11-10
tags:
- BlackAlps
- CTF
- 2024
---

# Description

Serge the accountant inadvertently opened an infected file on his workstation. After investigation by the SOC team, the file was found to contain malware designed to extract confidential information. The SOC team asked you to help them investigate the incident. The SOC has found a folder (C:\exfiltr) but cannot recover its contents.

+ we get a file `exfiltr.7z`

# Solution

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
