---
layout: post
title: "Capture The Evidence v2 (2026)"
author: cryptax
date: 2026-06-16
draft: false
tags:
- OSINT
- CTF
- Gendarmerie
- PACA
- Recap
- AI
- Tools
- Sherlock
- Evidence
- Proof
- Investigation
- lock-picking
- Web
- z3
- dumpor
---

# Capture The Evidence v2 - June 2026

French gendarmerie 🇫🇷 organized a special CTF called "Capture The Evidence" from June 5 to June 15, 2026.

I participated for the first time, with a team of 4 called *Eternal Green*. The name of our team is a play on words based on *Eternal Blue*, the organizing team where blue is the color of the gendarmerie, and *Green* in our case in reference to the (famous?) Pico le Croco 🐊 ([Ph0wn](https://ph0wn.org)'s mascot).

We had a *marvelous* time: the story was gripping, there were loads of challenges (90 😍) that unlocked progressively. Most of them were *OSINT*, which is not our strong point, but there were also *Web* challenges, *Reverse*, and even (digital) lock-picking!

We worked till the end, and are very proud to have achieved the *13th rank* on the scoreboard 🏅 . There were 87 teams with a non null score.

![](/images/cte2026-top3.png)
![](/images/cte2026-eternalgreen.png)

## Scenario

The very special thing at CTE is a *thrilling scenario*: very well done, lots of details, plausible facts and elements. This year's theme was on identity theft. A fictive character, Melanie Lefevre, studying journalism, gets her email, identity card and bank account stolen by cybercriminals. The impact for her is tremendous with over 20,000 euros of payments she never did, traffic fines in places she never went etc.

The French gendarmerie created a [intro video](https://www.youtube.com/shorts/ZOP-pttsNEI) and a [wrap-up](https://www.youtube.com/watch?v=XRka3q9UXBY). Watch them to get a glimpse at the professionalism of the CTE! The videos are really well done. The final one shows the victim, Melanie Lefevre, in motion and cybercriminal Miguel Santos, being caught.

> **Is a good scenario just smoke and mirrors, only meant to look impressive?** No, in the CTE, it provides thousands of details, elements, proofs which help progress through the investigation. It also helps understand this *could* be real, and what investigators might do in such cases. At [Virus Bulletin](https://www.virusbulletin.com/conference/vb2026/programme/) this year, I am presenting a cyber threat which affected an elderly citizen. He "only" suffered financial loss, but we see through this case it could easily have been far worse.

## Lock-picking

We all loved the beginning where we had to open a digital safe to access the CTFd, and judging from Discord comments, it was a general feeling.

![](/images/cte2026-code.png)

It was such an original idea. The code was hidden in ticks (sounds) in the [intro video](https://www.youtube.com/shorts/ZOP-pttsNEI). We patiently had to work out which numbers that equalled to, and reproduce the moves on the front page.

> **Did you use AI?** Yes. Of course. And I am pretty sure most teams did. *But* in this particular case, IMHO, **AI made the challenge more funny**. We still had to find the trick: parse all documents we were given, decide that in the intro video the ticks conveyed important meaning, grab the audio. AI just did the last step for us: listen to the audio and work out the number of ticks for us to open the safe. I'm not sure I would have enjoyed spending time on this myself, I'm glad the AI did it 😉

## OSINT is *not* for n00bs

In CTFs, I used to have mixed feelings about OSINT because, in my opinion, you mostly solve challenges by a few lucky (or unlucky) *Google dorks* or geographic searches with *Overpass Turbo*. Hmmm... that's the view of someone who's only scratching the surface!

After the CTE, and more experience with OSINT, I appreciate the skills and tools it requires. Yes, "luck"  always helps (for any challenge, even reverse!) but if you do **OSINT with method, tools and brains, you find more and more quickly**.

**What have I learned?**

- I used a lot [sherlock](https://github.com/sherlock-project/sherlock) (pre-installed on Exegol) and had less success with [maigret](https://github.com/soxoj/maigret/blob/main/README.md).
- I discovered [dumpor.io](https://dumpor.io/) which provides a general overview of several social media accounts for a given login.
- This [Gmail OSINT tool](https://gmail-osint.activetk.jp/) did wonders. I had never realized how easy it could be to access a user's calendar, google drive etc if adequate protection wasn't in place.

In terms of OSINT methodology,

- I learned (to my expense) it is important to **download all social accounts locally: do it once, it saves lots of search** time afterwards. There are certainly many tools to dump social accounts, but I could lay my hands on any that worked well in practice. For example, access to X accounts has significantly changed these last 5 years. So, I asked the AI to generate **scripts to download X, BlueSky, Wordpress and Diariste accounts**.
- **Maintain recaps on characters**. I usually had those recaps done by a low cost model, and would from time to time enhance them manually. Those recaps are **handy to share information with your team**, but also to feed in to AI for specific challenges and give context to the prompt. Finally, personally, they help get a clear overview of the scenario, see what's missing (e.g "oh? no Facebook account so far...") and get new ideas.

## An investigation

Actually, the CTE is more like a detective's investigation than pure, unrelated OSINT challenges. You receive a large collection of information all the time, and you have to decide what's important and what is not... while keeping in some place of your memory small details that might help you ascertain a source.

In the CTE, a single source of information is rarely sufficient to solve a challenge. Most of the time, you have to collaborate 2 or 3 information from different sources.

If you are a board game players, the CTE is similar to [Detective](https://iello.fr/jeux/detective/), or [Dossiers Criminels](https://dossierscriminels.com/products/dossiers-criminels-tragedie-a-mykonos).

## Is it still interesting with AI?

You probably heard [my position on AI in CTFs](./ctf-ai.md): while I'd hate to ban AI - as any other tool - it's true that it often spoils the fun of challenges. I've been thinking about solutions - so is the community - and I'm still thinking.

The CTE was a really good surprise on this account. **Artificial Intelligence helped speed up some potentially boring parts, but only seldom spoiled the fun of the challenges**. (Yes, it happened but only very occasionally).

AI is good at very specific tasks. Let me tell you where it helped me in the CTE:

- Search for an idea in a large quantity of data. The AI is better than me to turn this idea in an adequate `grep` with many words, or simply read and summarize text.
- Write specific scripts, e.g. download bluesky posts each with their date and title in a directory.
- Conduct reconnaissance on a Web challenge to find existing routes.
- Write a correct [z3 solver](https://www.microsoft.com/en-us/research/project/z3-3/) based on various constraints I provide.

But **AI is quite bad at larger tasks**, understanding how to assemble the puzzle. Many times it felt quite silly, testing randomly ideas that had little chance of working, looping on the same ideas etc. **I can't recall AI helping on a single OSINT challenge, apart creating useful scripts to help me grab the information I wanted.**

On reverse, I had it use a [Ghidra MCP](https://github.com/bethington/ghidra-mcp), and while it was good at explaining specifically this or that line, **it had no intuition at where to look at in the binary** (main, password comparison etc).

It did solve *a couple of Web challenges*, but never immediately. I'm notoriously unskilled at web challenges and usually avoid them in CTFs, so I can't say I was of a big help. Still, **I often asked my AI agent for recaps**: what it had already tried, what was its next steps, and that where I directed it, because, once more, surprisingly **it had little intuition of where to go when I seemed more obvious to me**. When it solved, at the end, I always asked it to generate a good writeup for me: I read it, and I must say I learned a lot about web challenges!

> **Why was AI not an issue in the CTF?** My analysis is that this CTF is very much about assembling information from multiple sources. While an AI is excellent at reading tons of information and summarizing it, models are apparently still poor at really understanding what they read and correlating facts.

## Write-ups, FAQ and Conclusion

Official write-ups for the CTF are available [here](https://github.com/EternalBlueCTE2k26/CTEv2-Writeups/). I might post a few selected writeups of my own on my blog afterwards too.

- *Which was your favorite challenge?* **Toujours en Vente**. **Le Vault** was excellent too. This is biased by the fact I love Reverse and Crypto challenges.
- *Which challenge did you struggle the most on?* **Hello World** and perhaps **Banque Root**. Ah ha! I'd even grumble and contest the solutions, but you'd be right to think this merely because I'm unhappy to have wasted so much time on them 😜
- *Which challenge looked most like a [Ph0wn](https://ph0wn.org) challenge*? **Point de chute** or **Le coffre fort**.
- *What is your favorite OSINT challenge*? Difficult to say! I like very much **Le Bailleur innocent**. I found **Banque Root** very interesting in terms of OSINT techniques.
- *Which character did you prefer*? I can't say. Of course, I'm on the side of the victim, Melanie. But Samir and Miguel are very interesting characters too.
- *Did you solve all challenges*? No!!! There were 90! Our team solved ~40+.


**Many many thanks to the organizing team for setting up this awesome CTF**. The scenario and the challenges were grasping. Many thanks also to the Support Team who kept answering (quickly) to our questions on Discord.

-- Cryptax