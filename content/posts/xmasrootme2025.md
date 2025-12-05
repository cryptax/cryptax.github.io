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

## Community Gift Project - Day 4

### Description 

We are given a ZIP file with a Docker environment: 

- `docker-compose.yml`: builds the container for port 1337
- `Dockerfile`: basic Flask environment with gunicorn which copies `app.py`, `factory.py` and `templates/`
- `requirements.txt`: Python dependencies with Flask and Gunicorn.
- `templates/index.j2`: Jinja 2 template for HTML website with some javascript
- `app.py` and `factory.py`

![](/images/xmas2025-day4-website.png)

### app.py

`app.py` implements a simple Flask application with only 2 routes: / (GET) and /help_santa (POST). 

Each time you help Santa, a global `work_counter` is incremented:

```python
@app.route('/help_santa', methods=['POST'])
def help_santa():
    global work_counter
    with mutex:
        if work_counter < REQUIRED_WORK:
            flag_room.work()
            work_counter += 1
    return redirect(url_for('index'))
```

The goal is to reach an enormous value, which will then display the flag:

```python
REQUIRED_WORK = 1337133713371337133713371337133713371337
...
counter = work_counter
        if counter == REQUIRED_WORK:
            gift_state = 'Gift complete: ' + int(''.join(str(b) for b in flag_room.gift_state), 2).to_bytes(24, 'little').decode()
```

Naturally, the difficulty is that `REQUIRED_WORK` is really huge, no way we're going to POST that many help_santas, even with a script.

### factory.py

The flag is decoded from a bit field state, which is altered each time work is performed:

```python
class LeFlagSynthesisRoom:
    def __init__(self):
        self.instructions = [192, 191, 190, 189, 187, 183, 178, 174, 173, 171, 170, 167, 166, 165, 162, 160, 159, 158, 157, 155, 149, 148, 147, 146, 143, 139, 137, 135, 131, 130, 123, 119, 117, 116, 115, 113, 111, 110, 109, 108, 106, 105, 102, 100, 99, 94, 93, 90, 89, 85, 81, 75, 74, 73, 72, 71, 70, 69, 68, 67, 65, 64, 63, 60, 58, 57, 55, 54, 51, 50, 47, 45, 44, 41, 40, 39, 38, 37, 32, 30, 29, 25, 24, 23, 22, 20, 19, 18, 16, 14, 12, 10, 9, 7, 5, 4, 3, 2]
        self.gift_state =  [0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1]

def work(self):
        s = 0
        for t in self.instructions:
            s ^= self.gift_state[t-1]
        self.gift_state = [s] + self.gift_state[:-1]
```

### Cycle

The gift state is in GF(2)^192 (that's the length of the gift state table).
This group has a *finite number of elements*, and each element depends on the previous one, so after a while, we know we're going to fall back to a state we have already explored, and then all next ones will be predictable. 
In other words, we have a *cycle*.

We can try and explore the cycle length: 

```python
seen = {}
state = gift[:]
i = 0
while tuple(state) not in seen:
    seen[tuple(state)] = i
    state = next_value(state)
    i += 1
```	

Unfortunately, it's very long, so we need to find another trick.

### Linearity

The key observation is that the work() function is linear over GF(2): it only does XORs and shifts. 

This means that each update of gift_state can be written as a matrix multiplication `x_next = M * x` where M is a 192×192 binary matrix. 

Once we see it that way, we don’t need to simulate billions of steps anymore. Instead, we can jump ahead by computing an exponentiation of M:

