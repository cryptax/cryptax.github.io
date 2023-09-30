---
layout: post
title: "BruCON 2023 CTF Forensics Challenge"
author: cryptax
date: 2023-09-29
tags:
- Brucon
- CTF
- 2023
- Regexp
---

## Forensics: Rexer

This is what the challenge says:

```
The pattern is the key.

\w*(3).+(T).*([s]).+(g).*([NT]r).*(u).+(L)\w+(Sm).*(N).*}$

asonV3l113UYnrvTfeJpsaeR}ZkJRvA{ThKgr730gnwWhYgVnNTSuhbpJ_BPn7Q8Q7mLnTrc_S3_p3zpFV8Fmm0hrUbvt_24upiqAt7ugXuhbpJ_BPn7Q8Q7mLq8qsSml4Sl4xmw1ON}

\2OREO\9{\8\6\4\4\7\1_\3\5\6\4\4\7\1}
```

The first pattern selects parts within the string (second line). Then, the third line re-assembles the selected groups to form the flag.

At first, I tried to use ChatGPT to help me out with the regexp. It didn't work out well and I used [Regex101](https://regex101.com).

![](/images/brucon2023_rexer.png)

This website is very convenient. See on the right how it tells me what are the letters in group `\1`, those in `\2`, those in `\3` etc.

Then, I had to re-assemble those characters from the 3rd line `\2OREO\9{\8\6\4\4\7\1_\3\5\6\4\4\7\1}` which gave the flag `TOREON{SmuggL3_sTruggL3}`.

I liked this challenge.

