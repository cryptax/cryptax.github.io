---
layout: post
title: "Auvergn'hack 2026"
author: cryptax
date: 2026-06-01
draft: false
tags:
- Auvergnhack
- 2026
- conference
- landlock
- CTF
- telemetry
- post-quantum
- cryptography
- VPN
---
# Back from Auvergn'hack 2026

[*Auvergn'hack*](https://www.auvergnhack.fr/) is a recent, local security conference in *Clermont Ferrand* (France). It is rather small: a single day and a single track, but it was a great discovery with an **excellent ratio of interesting talks**.

Yes, when you're a "senior" with "20+ years of experience" (a kinder way of saying you're old), you've already seen quite a lot. You don't say it too often not to sound old and grumpy, but you are often disappointed with talks because they tell a story so similar to one you've already heard in the past. You end up finding more value in *networking* (this is fine, it's a valid reason for joining conferences).

At Auvergn'hack, **on the 4 conferences I followed in the afternoon, 3 were deeply interesting**. This is the *best ratio I've had at conferences in ages*. Hey, only 4 conferences, you're wondering? Well, in the morning, I was busy with my own workshop and couldn't attend any. In the afternoon, I only missed one (needed to purchase a toothbrush lol). 

## The best talks

![](/assets/images/auvergnhack26-landlock.png)

1. **Landlock**, a Linux security module that lets applications sandbox themselves and restrict access to files, directories and sockets [:(fa-solid fa-file-lines):](https://blog.sekoia.io/leveraging-landlock-telemetry-for-linux-detection-engineering/). It's rather lightweight and for example an excellent solution to block against path traversal. In practice, I'm not entirely convinced this will be widely adopted and efficient, because you have to implement Landlock calls in your code, it doesn't control access to memory etc. Nevertheless, it was happy to learn about the feature and see it in action.

2. **Post quantum cryptography**. [Pascal Lafourcade](https://www.eyrolles.com/Sciences/Livre/cryptographie-post-quantique-9782100843510/) is a fabulous teacher: his presentations are clear, and focus on important points. I like cryptography, I've been following progress of quantum cryptography. I enjoyed being refreshed on the topic, including learning about [isogenies](https://en.wikipedia.org/wiki/Isogeny). His talk convinced me that, at least, I'll have to dig in post quantum crypto algorithms in the next 5 years. 

3. **Secure network at home, for cyber threat analysts**. How do you protect yourself when you're a cyber threat analyst working from home? As an investigator, how can you obscure the trail to avoid disclosing personal information? The speaker presented his home network setup - very professional - with segmentation, multiple virtual networks, automatic rotation of VPNs etc. The talk was amazing.

## The venue, the atmosphere

![](/assets/images/auvergnhack26-venue.png)

The conference took place at the PIC. It's a massive timber building with an open, airy design, plenty of natural light, and a friendly atmosphere. There are conference rooms, coworking areas, and several good-quality fast-casual dining options.

After the conference, attendees usually gathered in the terrasse for an informal drink and chat. The staff was easily available and helpful.

## Reverse with AI workshop

I gave my workshop in the morning, with 20-25 persons in the room. It's never easy to cover AI because things evolve so fast that by the time you present something, you're quite sure something has changed. In my case, for example, [OpenCode](https://opencode.ai) had stopped offering free access to MiniMax LLM, but fortunately all labs worked fine with Big Pickle (free as well) or other LLMs. I also feared the setup of [Ghidra MCP](https://github.com/bethington/ghidra-mcp) for participants : it's not "that" difficult, but a bit tricky, you've got to follow instructions to the letter. Again, all went well. I was very happy with the audience, they asked many questions, understood very well the goal of exercises. It was an excellent experience from my point of view!

## Auvergn'hack CTF

Hmmm... Maybe we shouldn't talk about it :P
There was a severe infrastructure issue, and the CTF started ~5 hours late. This kind of undermined our motivation (if I have time, I'll however post a writeup about a crypto challenge), but at least, we had an excellent barbecue and my CTF team was fabulous, I learned about Java champions, Auvergne, Saint Nectaire and patisserie :)

-- Cryptax

