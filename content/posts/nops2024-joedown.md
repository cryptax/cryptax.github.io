---
layout: post
title: "Joe Dohn (NOPS CTF 2024)"
author: cryptax
date: 2024-06-03
tags:
- NOPS
- CTF
- 2024
- Discord
- Pwn
---

## Description


This challenge from the *Misc* category was saying: "Is it me or there is a strange guy on the discord server ?"

## Solution

I went on the Discord server of the CTF, and found a guy named "Joe Dohn". I sent him a message, and immediately he replied by inviting me to a private Discord server. Actually, Joe is not real person but a bot.

![](/images/nops2024-joeserver.png)

This private server has 2 channels: a general channel, and one dedicated to conversation with me.
Quickly, on the dedicated channel, Joe gives me the list of commands he understands:

![](/images/nops2024-allcommands.png)

Whenever you issue one of these commands, the result appears in the general channel.

- `!roll` rolls a dice
- `!toss` tosses a coin
- `!cat` displays a cat image
- `!fox` displays a fox image

![](/images/nops2024-rolltoss.png)

I struggled there for quite a long time, investigating several possibilities:

- Was I meant to send the commands in a given order?
- Was I meant to display a cat only if the dice number was even?
- Was I meant to find a combination where I'd have an odd number and Tail?
- Was there something hidden in the cat/fox images?
- Etc

Whenever a team got the flag, a mention would be written in the general channel, so when that happened, I inspected what commands that person had issued. It didn't tell me much... 

A team mate noticed that some teams flagged when nothing had appeared on the general channel for a long time. So, we deduced that we had rather something to do on the private dedicated channel.

There was a `!send_secret` command, but of course Joe would tell his secret. 

![](/images/nops2024-joenotallowed.png)

I started by looking for potentially hidden commands: `/help`, `!code`, `!say secret`, `!flag` `!help toss`. I also imagined influencing a command: `!roll 1`, `!roll 5`. It was quite amusing, because at some point I thought I had an algorithm there: it would display the "opposite" number: `!roll 1` would show 5, `!roll 5` would give 1 etc... but of course then I noticed it didn't work at all and that my alleged algorithm was completely random.

I also sent several messages with the `!send_msg` command. It was clear Joe got the message but didn't care. 

![](/images/nops2024-joemsg.png)

At some point, in the same vein as arguments, I finally got the idea to chain commands, and I tried `!send_msg !send_secret cryptax`. And I received the flag :)

![](/images/nops2024-joeflag.png)


Though not very technical, this challenge was *fun*, and I could have thought of the solution faster.
