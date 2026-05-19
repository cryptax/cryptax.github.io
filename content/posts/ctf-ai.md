---
layout: post
title: "CTFs and AI"
author: cryptax
date: 2026-05-19
draft: false
tags:
- AI
- CTF
- orchestration
- dead
---

# CTFs and Artificial Intelligence. Post Ph0wn 2026 Thoughts.

*Update May 19, 2026:*

- *Re-phrased first paragraph. In particular, replaced the title "Myth #1 CTF players hate AI [..]" with "CTF players hate AI [..]: True or False?" because I found my title was too biaised. It did not express fairly enough the reality.*
- *Added a paragraph on my (current) opinion.*
- *Added references to [foreman](https://github.com/palkeo/foreman) and [Kabir](https://kabir.au/blog/the-ctf-scene-is-dead)*

## My opinion, in short

The "problem" of AI in CTFs is very complex, with pros and cons...

> If you absolutely want my opinion, as of May 19, 2026, there you go: **I fear that CTFs are over** - like LAN parties were - but **I am such an optimistic person that I refuse to believe we won't find a solution, or a new sort of technical event which is fun and educative** like CTFs were.

- I "hate" when people tell me "it's dead": I want them to investigate solutions. I love when CTF organizers *try* something new, and even if their solution is not perfect, I support their attempts to "save" CTFs.
- I "hate" when people tell me "everything is fine, there is no problem, AI is just a new tool". No, AI is not "just a tool", it *thinks*. Acknowledge the problem! It's *okay* to have problems...
- I "hate" when people tell me "just adapt". Yeah, well, thanks, I'm trying to. Perhaps you should too!

### What will you do for Ph0wn CTF 2027?

Answer: **I really have NO idea yet.**

A couple of things we tried at Ph0wn 2026 worked out (prompt injection, air gapped challenges), while others showed limits (creation of beginner challenges, hardware challenges with serial/USB/simulator access).

I don't want to do Ph0wn 2027 "as in 2026" without any modification regarding AI, but I don't *know what to do yet*. I'm watching attempts from other CTFs (NorthSec, THCon, Sthack, CTE...) to see if anything works out. Not to mention that there are several organizers for Ph0wn, and we'll need to agree on a solution/mode.



## "CTF players hate AI because it spoils CTFs": True or False?

*Reality: it's complex, but honestly we can't conclude either way yet*.

![](/images/ctf2026-destroyed.png)

"AI has destroyed CTFs" - this kind of message is frequent on the net.
However, in a [feedback form of Ph0wn CTF 2026](https://github.com/ph0wn/writeups/tree/master/2026/feedback), **80% hold no grudge against AI and are either happy with it, or just want to adapt**:

- "I just want to win, with or without AI", 30%
- "It's a new way of solving challenges. It requires different skills. I still like CTFs.", 20%
- "I want to learn, so I look into only few challenges and take time to solve them and learn, with or without AI" 30%.

![](/images/ctf2026-povai.png)

*Learning about AI* is a strong motivation for players. On future types of challenges, **AI-based challenges were 70% of demands for the next edition** (prompt injection, MCP, data injection in LLM, creating agents, vibe coding...).

*Only 20%* express issues with AI:

- "When a challenge is solved easily by AI, it ruins the fun for me" ~10%
- "I don't use AI" ~10%

![](/images/ctf2026-flag.png)

It seems AI is rather an issue for **challenge creators** who

1. Hate to see their hard work be solved by AI in seconds,
2. Don't understand teams who solve without understanding the challenge,
3. Don't like to spend time hardening their challenge towards AI or (4) are unsure what's the best option to do so.

> Conclusion: the majority of public messages (on social networks, blog posts etc) condemn the use of AI in CTFs. But our anonymous survey of CTF players at Ph0wn 2026 show 80% of players are happy with using AI over CTF challenges. So, who should we believe? Is our survey reliable or not? Is it a difference between public voice and anonymous ones (see later)? As for challenge creators, yes, the majority are reluctant, partly discouraged or helpless faced to machines.


## Myth : "Solving with AI requires no skills"

*Reality: AI requires different skills*

People often show contempt towards players who solve with AI. A little like *Scripts Kiddies* for hackers.

In reality, while some challenges are solved with no glory, several challenges require (1) a good prompt, (2) human guidance, (3) a good AI setup with tailored agents, skills, scripts.

Ph0wn feedback demonstrates this point: in a question, the feedback form asked how much AI helped for each challenge.
The responses show very different results.

**In absolutely all challenges, we had some teams report "AI did not help", while other teams reported it helped**. Sometimes, the answers are opposites. For example, for Sploosh Kaboom 2 challenges, 1 team reported an entirely automatic solve with AI, while 3 teams said AI did not help at all.

![](/images/ctf2026-aihelp.png)

> Conclusion: **AI does not help all teams the same way**. It would be unfair to see AI does not give a strong advantage to teams, but it would be equally unfair to claim teams who flag with AI demonstrate no intelligence 💡 of their own.

## Myth : "CTFs are Pay to Win"

*Reality: very few CTF players will pay to win*

Some teams very probably use AI better than others, or have a better setup. Better setup... is that about AI subscription? Does a large AI subscription give unfair advantage to the team? For sure, it helps.

Ph0wn feedback forms tend to prove that most CTF players do not have costly setups: less than 30% have a paid account.

![](/images/ctf2026-aisetup.png)

*But* this is probably **different in more competitve CTFs** like Insomni'hack CTF, where it seemed that nearly all teams had access to a paid subscription, sometimes "unlimited plans". Discussing with a few teams though, I haven't heard yet of any CTF which purchases an AI plan **only for the CTFs**. Most of the time, they are re-using a subscription they have from their work, from a friend, or from access to in-house AI servers.

## There is a discrepancy between what CTF players say publicly and what they do

Publicly, CTF players "hate AI", "never use AI", "use it responsibly", "only for side questions" etc.
From organizers view or an external visitor, **AI is on all screens**, many (if not most) teams use it intensively.

![](/images/ctf2026-lie.png)

I think *it's not a blatant lie*, but more a cognitive biais: **we convince ourselves our brains did the hard part**, it's above us to humanly admit we were **lazy** and that it's the opposite, *AI did most of the job* (AI can't talk and say we lied). Lack of proof.





## Solutions to cope with AI in CTFs

| Solution                         | Type         | Comment                  |
| -------------------------------- | ------------ | ------------------------ |
| Ask not to use AI                | Organization | Some (many?) teams will cheat |
| No prizes to win                 | Organization | Less incentive |
| No ranking                       | Organization | Some teams want to use the ranking on their CVs |
| Provide laptops with no Internet | Organization | Cost + people like their own setup... |
| Submit writeup instead of flag   | Organization | AI is good at creating writeups too... |
| Detect AI-written writeups       | Technical    | Bypassable. Adding "human written parts", instructing AI to insert typos etc |
| Lightning talks of solutions     | Organization | AI can write the slides, human rehearses them. At least, doing so, they learn about the challenges |
| Ban teams with quick solves      | Technical    | Bypassable: teams will wait to flag + how do we tell the difference between AI and a genuine human genius? |
| Report use of AI when you flag a challenge | Organization / Technical | Implementable like the challenge feedback plugin in CTFd. Players don't have to say the truth + what about challenges where AI "helped a bit"?. Used at NorthSec 2026 |
| Separate scoreboards: AI no AI   | Organization / Technical | Teams can cheat |
| Send flag found by AI to a MCP server | Technical | Players can still submit their flag manually and not count as "found by AI" + what about challenges where AI "helped a bit" only? Used at NorthSec 2026. |
| No Internet                      | Technical    | Bypassed by 4G/5G |
| Faraday cage                     | Organization | Cost, size of the cage... |
| Watch participants screens       | Organization | When the cat doesn't watch, mice dance. |
| Live broadcast of 1 participant. All others watch.  | Organization | Very different from CTFs. Hackers don't like to share *all* their secrets |
| Block major AI websites from WiFi | Technical    | Only works for on-site CTFs which provide WiFi/LAN access to Internet. Even in this case, easily bypassable by 4G/5G, VPNs, SSH tunnels |
| Block AI traffic (e.g CloudFlare) | Technical    | Only works for challenges when participants access a service, server, container or remote host that runs on a host behind the filter/CloudFlare. Bypassed by VPNs or SSH tunnels |
| Catch flags from AI traffic      | Technical    | Only works for on-site CTFs which provide WiFi/LAN access. Requires SSL inspection |
| Detect AI User Agents            | Technical    | Only works for challenges with participants access a service, server or remote host. Used at FCSC 2026 |
| Redirect AI websites to fake one | Technical    | Bypassed by 4G/5G, VPNs, SSH tunnels |
| Inject prompt with fake flag     | Technical    | Needs to be fine tuned. Only works for a couple of months. |
| Use custom CTF interface         | Technical    | Limits use of on-the-shelf agents that download challenges from CTFd for example. But new ones can be created. |
| Shaming of teams who supply fake AI flag | Organization | Is it really ethical to shame? |
| Challenge instructions by video  | Technical    | File size |
| Inject prompt to convince AI not to solve | Technical | Needs to be fine tuned. Only works for a couple of months.  |
| Injecting a prompt that suggest something is illegal | Technical | See PagedOut magazine. Tried but abandonned for Ph0wn because we couldn't make it work |
| Inject false lead that slows AI  | Technical    | Does not affect all teams - Ph0wn 2026 "Ancient Stories", Hack10 |
| Insert easy fake flag and hope AI stops on it | Technical | Does not work all the time |
| Inject prompt in polyglot        | Technical  | Contemplated for Ph0wn 2026. Used at RITSEC 2026 |
| Insert misleading `AGENTS.md`, `CLAUDE.md` or `SKILL.md` | Technical    | Seen at Insomni'hack CTF 2026 |
| Require physical interaction with device | Technical | Beware SSH/Serial/USB access + simulators/containers. Used Ph0wn 2026 |
| Use custom virtual machines      | Technical | Does not necessarily stop AI... |
| Use air gapped challenges        | Technical | Works well, but hard constraint on challenges. Used at Ph0wn 2026 Minitel challenges |
| Protocols with no/little doc     | Technical | Only few such protocols! Partly used at Ph0wn 2026 |
| Unknown tricks / 0-days          | Technical | Transient state - Ph0wn 2026 "Flagged Pages" |
| Only 1 connection per team to challenge | Technical | AI can use the connection... |
| Challenges around optimizations e.g "create a binary with x/y/z below 200 bytes" | Technical | AI isn't so good at optimization yet (but it might improve). We contemplated this sort of challenge for Ph0wn 2026 but finally didn't play it. |
| Challenges with guessy parts | Technical | AI is usually better than humans to solve the guessy part. This does not refrain AI usage, but it opens up to different challenges |
| Wider scope challenges where there are many possibilities to investigate | Technical | AI is not scared by work, it will investigate possibilities. Humans can guide towards the best option. This does not refrain use of AI, but it opens up to a different category of challenges |


## Challenges categories which resist best to AI

- Air gapped challenges
- OSINT
- Reverse and Pwn

## What will we do for Ph0wn 2027?

Currently, **I do not know.** We are not taking any decision yet, we want to hear from other attempts in other CTFs, and pick up what worked the best.

After [our talk at THCon, with Damien Cauquil](https://thcon.party/events/#chain-of-thought), we had many valuable discussions:

- Some CTFs are going to test the non-competitive mode (no ranking, no scoreboard). I won't spoil which CTF that is 😉 , but I am very impatient to see how it goes.
- *THCon CTF* announced they have a few challenges intended to be "AI-resistant". For sure, it won't be perfect, but they may have come up with excellent ideas. CTF starts tomorrow.
- Collaborative challenges where a given team solves a first part, explains it to another team to continue second part...
- Blocking AI or not? People have different ideas on the questions, both being perfectly valid opinions. On one side, some participants say they no longer want to participate in CTFs where AI is permitted. On the other side, others say hackers have always had access to all tools, and AI is one of the tools to learn to use.


## References 📚

CTFs and AI talks:

- Damien "virtualabs" Cauquil, Chain of Thought Fucks everybody, Bière Sécu Rennes 9, [🔗](https://github.com/Biere-Secu-Rennes/BSR/blob/main/bsr9/bsr9-virtualabs-chain_of_thought_fs_everybody.pdf)
- A. Apvrille, D. Cauquil, *Chain of Thought Fucks* [🔗](https://github.com/cryptax/talks/tree/master/THCon-2026), [THCon](https://thcon.party/events/#chain-of-thought), May 2026.


Feedbacks from CTFs:

- 🔗 [Feedback from RITSEC](https://sylvie.fyi/posts/ritsec-2026/)
- 🔗 [Plans for NorthSec](https://res260.medium.com/northsec-ctf-et-les-agents-ia-0852b099045e)
- 🔗 [Feedback from HACK10](https://danisy-eisyraf-portfolio.super.site/blog-posts/how-i-make-ctf-challenges-harder-to-solve-with-ai)
- 🔗 [Retex Hack'in 2026](https://lololekik.com/fr/post/ai-vs-ctfs)
- 🔗 [CTF and AI at BSidesSF 2026](https://blog.includesecurity.com/2026/04/ctfs-in-the-ai-era/)
- [FEED26] Feedback from Ph0wn 2026 [🔗](https://github.com/ph0wn/writeups/tree/master/2026/feedback), in French.
- 🔗 [39C3, "Breaking BOTS: Cheating at Blue Team CTFs with AI Speed-Runs"](https://media.ccc.de/v/39c3-breaking-bots-cheating-at-blue-team-ctfs-with-ai-speed-runs)


User feedback:

- 🔗 [Krauq, "CTF is dying because of AI...?"](https://blog.krauq.com/post/ctf-is-dying-because-of-ai)
- 🔗 [Toot from Hoshino Lina](https://vt.social/@lina/116198976928184530)
- 🔗 [Post from Bl4sty](https://x.com/bl4sty/status/2031022899960693222)
- 🔗 [Kabir, the CTF scene is dead](https://kabir.au/blog/the-ctf-scene-is-dead)

Tools:

- CAI wins the NeuroGrid CTF: European-built CAI (CyberSecurity AI) sets a new global benchmark, November 25, 2025 [🔗](https://news.aliasrobotics.com/cai-wins-the-neurogrid-ctf-european-built-cai-cybersecurity-ai-sets-a-new-global-benchmark/)
- Yuwen Zou, Jia Liu, Wenjun Fan, CTFAgent: An LLM-powered Agent for CTF Challenge Solving, Journal of Information Security and Applications, [DOI 🔗](https://doi.org/10.1016/j.jisa.2025.104305)
- Palkeo, Foreman: a secure orchestrator for running short-lived interactive LLM sessions in sandboxed Incus containers, [🔗](https://github.com/palkeo/foreman) [blog](https://www.palkeo.com/en/blog/foreman.html)

CTF write-ups, solved with AI (at least parts):

- Cryptax, *Drone and secret message*, THCon CTF 2023 [🔗](https://cryptax.github.io/thcon2023-drone/)
- Cryptax, *MANUAL (crypto) - flagged with no skill*, Hack.Lu CTF 2025 [🔗](https://cryptax.github.io/hacklu2025/#manual-crypto---flagged-with-no-skill)
- Cryptax, *AI: Ask for It, Double Protection 1 and 2*, N0PS CTF 2024 [🔗](https://cryptax.github.io/nops2024-ai/)
- Cryptax, *Xmas Root Me CTF 2025* [🔗](https://cryptax.github.io/xmasrootme2025/)

Misc:

- CSAW'25, *Agentic Automated CTF*, July-October 2025 [🔗](https://www.csaw.io/agentic-automated-ctf)
- Peter Whiting, *Piracy as Proof of Personhood*, PagedOut #7, page 9 [🔗 ](https://pagedout.institute/download/PagedOut_007.pdf)
- Write-ups for challenges, Ph0wn Mag #3 [🔗](https://github.com/ph0wn/writeups/tree/master/ph0wnmag/issue-03)