---
layout: post
title: "XMas Root Me 2025"
author: cryptax
date: 2026-01-01
draft: true
tags:
- rootme
- 2025
- ctf
- AI
---

# Xmas Root Me CTF 2025

## X-Mas Assistant Day 1

### Description of the challenge

"Meet your X-Mas Assistant! It will help you throughout this entire month of CTF, providing a brand-new way to interact with CTFd through a "futuristic" Chatbot UI. This assistant talks directly to the CTFd API for you! Try it now! Can you exploit its MCP quirks and uncover the flag?

This is not a web challenge.Meet your X-Mas Assistant! It will help you throughout this entire month of CTF, providing a brand-new way to interact with CTFd through a "futuristic" Chatbot UI. This assistant talks directly to the CTFd API for you! Try it now! Can you exploit its MCP quirks and uncover the flag?

This is not a web challenge."

We get a link to our own instance, which is a web interface to an AI.

### Reconnaissance

I download the HTML page, and don't see anything hidden  in it. We see the expects entry points are `/mcp/chat` and `/mcp/reset`.

```
await fetch('/mcp/reset', { method: 'POST' });
const response = await fetch('/mcp/chat', {
```

I start asking a few questions to the AI, like "How are you?", and it tells me it can help me access the CTFd interface. 

### Functionalities

![](/images/xmas2025-day1-listfunc.png)

The functionalites are real. If I ask to list users, I do indeed get the current list of users on the Xmas root CTF.

Being admin is always important so I list them, and recognize a few names of known admins (hi!).

![](/images/xmas2025-day1-getadmins.png)

### Solve

To get the flag, we need to know the challenge id. 

> I initially tried 1 but couldn't retrieve its flag whatever admin username I supplied. This is because 1 is not a valid/active challenge (being a CTFd admin myself, this happens very often, you create test challenges they get ID 1 and then you delete them, so actually your real challenges start later).

So, I list challenges to retrieve the ID of the challenge I want (day 1). We see it's challenge 11.

![](/images/xmas2025-day1-getchall.png)

Then, I just get the flag for it. You don't get the flag if you're not admin. But you actually don't need to authenticate to be admin, just claim you are one, that's the flaw.

![](/images/xmas2025-day1-getflag.png)
![](/images/xmas2025-day1-getflag2.png)
![](/images/xmas2025-day1-getflag3.png)

> Simple but nice challenge, I liked it!

