---
layout: post
title: "BlackAlps 2026 - Badge Challenge"
author: cryptax
date: 2026-11-08
tags:
- BlackAlps
- CTF
- NFC
- token
- APDU
- GPT-2
- Torch
---

# Badge Challenge at [BlackAlps](https://blackalps.ch) 2026

At a booth, [Compass Security](https://www.compass-security.com/en/) was running an interesting CTF-like challenge. They were giving out NFC-programmable e-Ink badges such as these:

![](/images/compass-badge.jpg)

You'd go to their stand, get your photo taken (if you wanted) and have a local LLM on one of their laptops process the image. In my case, I added the prompt "put a crocodile on my shoulder" - spot it in the picture.

On the back of the badge, you'd find a QR code to a repository to handle their badge: [GitHub](https://github.com/CompassSecurity/nfcink)

![](/images/compass-back.jpg)

We know there is a flag 🚩 to find, but we don't have any other clue.

## Getting information from the GitHub repo

We learn the badge is a "4-colour e-ink NFC display badge (240 × 416 px, black / white / red / yellow)" (see `README` in the repo).

The badge speaks **ISO 14443-4**, because the `README` recommends "Any ISO 14443-4 compliant CCID reader" to program it. So, the badge **behaves like an ISO 14443-4 contactless smart card** where **NFC is the radio link**. 

`nfcink` is a tool to write the badges: set the photo, name, refresh:

- `python3 nfcink.py badge <photo> --name "Jane Doe"`
- `python3 nfcink.py refresh`

There is yet another interesting command, in the context of a challenge: `read-user-data`:

- `nfcink.py` -> `run_on_tag()` in `runner.py` -> `cmd_read_user_data()` in `protocol.py`

```python
def cmd_read_user_data(self, offset: int, length: int) -> bytes:
        """F0 D8 00 00 05 <offset BE 4B> <length 1B> -- read user data area.

        Reads `length` bytes (1..255 per APDU) starting at byte `offset`
        from the FMSC chip's 20 KB user data area. Per FMSC ESL manual
        section 7.14. The chip stores the literal ASCII "4_color Screen"
        at offset 0 by default (this is what `cmd_get_device_config` reads
        for the 4-color marker check).

        Caller must chunk reads longer than 255 bytes themselves.
        """
        if not 0 <= offset <= 0xFFFFFFFF:
            raise ValueError(f"offset out of 32-bit range: {offset}")
        if not 1 <= length <= 255:
            raise ValueError(f"length must be 1..255 per APDU, got {length}")
        apdu = (
            bytes([0xF0, 0xD8, 0x00, 0x00, 0x05])
            + offset.to_bytes(4, "big")
            + bytes([length])
        )
        return self._txb(apdu)
```

To read user data, we must send "F0 D8 00 00 05 <offset> <length>":

- Offset is written in Big Endian, and it's where we should start to read user data.
- Length of the data to read

## Reading user data from the badge

We can try and send `F0 D8 00 00 05 00 00 00 00 FF` (actually, there's something smarter to do if you read code comments, we'll see that after).

I don't have a CCID reader with me, so I use my Android smartphone and the "NFC Tools" application by NXP. I navigate to the menu for **Advanced commands**, which allows me to send custom APDUs.

![](/images/compass-advanced.jpg)
![](/images/compass-command00.jpg)

```
> Sending bytes: F0:D8:00:00:05:00:00:00:00:FF
> Result: 34:5F:63:6F:6C:6F:72:20:53:63:72:65:65:6E:FF:FF:53:74:61:67:65:20:31:20:66:6F:75:6E:64:21:20:41:6E:20:49:56:20:2B:20:63:69:70:68:65:72:74:65:78:74:20:61:72:65:20:68:69:64:69:6E:67:20:61:74:20:6F:66:66:73:65:74:20:30:78:30:34:31:35:2E:20:46:69:6E:64:20:74:68:65:20:6B:65:79:20:61:6E:64:20:6C:65:74:27:73:20:73:65:65:20:69:66:20:50:75:66:66:79:20:63:61:6E:20:68:65:6C:70:2E:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:90:00
```

There's ASCII text at the beginning:

```python
>>> s = '34:5F:63:6F:6C:6F:72:20:53:63:72:65:65:6E:FF:FF:53:74:61:67:65:20:31:20:66:6F:75:6E:64:21:20:41:6E:20:49:56:20:2B:20:63:69:70:68:65:72:74:65:78:74:20:61:72:65:20:68:69:64:69:6E:67:20:61:74:20:6F:66:66:73:65:74:20:30:78:30:34:31:35:2E:20:46:69:6E:64:20:74:68:65:20:6B:65:79:20:61:6E:64:20:6C:65:74:27:73:20:73:65:65:20:69:66:20:50:75:66:66:79:20:63:61:6E:20:68:65:6C:70:2E'
>>> bytes.fromhex(s.replace(':',''))
b"4_color Screen\xff\xffStage 1 found! An IV + ciphertext are hiding at offset 0x0415. Find the key and let's see if Puffy can help."
```

> If you read the comments, they explicitly say that at offset 0, we will find the literal "4_color Screen". So, we can directly asked only for what was *after* i.e offset 14 (0x0E): `F0 D8 00 00 05 00 00 00 0E FF`.

## Reading offset 0x0415

The badge tells us the next step: read offset 0x0415. So, that is `F0:D8:00:00:05:00:00:04:15:FF`.

![](/images/compass-0415ff.jpg)

The badge answers this:

```
43:14:D7:60:E9:8A:3B:E7:B3:E4:AE:CC:D8:AE:0A:D7:8E:6A:87:5C:9B:B1:42:B5:71:6F:71:33:71:46:0C:F2:CA:66:A5:DD:C7:58:B9:90:B1:C7:08:EE:EB:2C:1C:DC:E0:9F:F7:23:FB:B5:93:31:14:9F:C9:46:36:9B:29:0A:0A:6D:FE:36:4E:9B:6C:73:8B:E6:26:82:E9:91:D8:2B:8B:F8:ED:EA:D9:D7:12:49:24:DF:D2:8E:B5:B4:DF:4C:37:C7:80:CA:40:39:2B:E5:A4:6F:55:79:32:22:15:96:3A:51:16:FB:7C:D8:90:8E:6E:CB:B4:E8:B5:88:58:EE:24:15:16:2D:D8:9D:12:D2:64:DB:6A:40:B3:59:24:BD:2A:BE:5F:01:F3:00:60:70:59:E5:97:90:AE:CC:B5:D6:ED:C1:34:B8:EE:0F:96:A1:56:47:AE:7F:A7:2B:AF:93:48:97:8D:00:9B:D9:D0:2C:D3:ED:7B:A5:68:8D:01:45:01:C8:92:CE:00:80:1E:24:44:B9:D1:6E:2C:29:87:9C:1F:74:E0:79:08:B6:1F:2B:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:90:00
```

The previous message contains a hint "see if Puffy can help".
A pufferfish is also a blowfish.
So the algorithm is likely to be Blowfish.
The message also says "Stage 1 found! An IV + ciphertext are" + "Puffy"), so we know:

1. **IV**. 8 bytes for Blowfish. `43:14:D7:60:E9:8A:3B:E7`
2. **Ciphertext**. `B3:E4:AE:CC:D8:AE:0A:D7:8E:6A:87:5C:9B:B1:42:B5:71:6F:71:33:71:46:0C:F2:CA:66:A5:DD:C7:58:B9:90:B1:C7:08:EE:EB:2C:1C:DC:E0:9F:F7:23:FB:B5:93:31:14:9F:C9:46:36:9B:29:0A:0A:6D:FE:36:4E:9B:6C:73:8B:E6:26:82:E9:91:D8:2B:8B:F8:ED:EA:D9:D7:12:49:24:DF:D2:8E:B5:B4:DF:4C:37:C7:80:CA:40:39:2B:E5:A4:6F:55:79:32:22:15:96:3A:51:16:FB:7C:D8:90:8E:6E:CB:B4:E8:B5:88:58:EE:24:15:16:2D:D8:9D:12:D2:64:DB:6A:40:B3:59:24:BD:2A:BE:5F:01:F3:00:60:70:59:E5:97:90:AE:CC:B5:D6:ED:C1:34:B8:EE:0F:96:A1:56:47:AE:7F:A7:2B:AF:93:48:97:8D:00:9B:D9:D0:2C:D3:ED:7B:A5:68:8D:01:45:01:C8:92:CE:00:80:1E:24:44:B9:D1:6E:2C:29:87:9C:1F:74:E0:79:08:B6:1F:2B`. 208 bytes.
3. The remaining FF bytes are padding.

## Where is the key?

On the badge, we notice a strange bar code between the name and the company name. In a CTF challenge, barcodes are never there without reason 😀.

![](/images/compass-barcode.jpg)

On my smartphone, I have been unable to find an application that read that barcode. So I took a picture of it, and sent it to a LLM to be decoded.

```text
ASCII: _f\E:R)XDhw%/b
hex:   5F 66 5C 45 3A 52 29 58 44 68 77 25 2F 62
```

> This isn't a readable key, so I wasn't sure it would work, but it does: this is the Blowfish AES key.

## Decrypting the ciphertext

There is no difficulty in writing a script to decrypt the ciphertext now that we have all the parameters.

> My script was generated by a LLM. I'm unable to say if that's a smart move "because I know how to do it and it saves me a few minutes" or if it's silly "because I dont need a LLM for that"...

```python
#!/usr/bin/env python3
"""Decrypt the Stage 2 blob from the Black Alps 2026 e-ink NFC badge.

Requires pycryptodome:  pip install pycryptodome
"""

from Crypto.Cipher import Blowfish

# 216-byte blob read from the badge user-data area at offset 0x0415.
BLOB_HEX = (
    "4314d760e98a3be7b3e4aeccd8ae0a"
    "d78e6a875c9bb142b5716f713371460c"
    "f2ca66a5ddc758b990b1c708eeeb2c1c"
    "dce09ff723fbb59331149fc946369b29"
    "0a0a6dfe364e9b6c738be62682e991d8"
    "2b8bf8edead9d7124924dfd28eb5b4df"
    "4c37c780ca40392be5a46f5579322215"
    "963a5116fb7cd8908e6ecbb4e8b58858"
    "ee2415162dd89d12d264db6a40b35924"
    "bd2abe5f01f300607059e59790aeccb5"
    "d6edc134b8ee0f96a15647ae7fa72baf"
    "9348978d009bd9d02cd3ed7ba5688d01"
    "4501c892ce00801e2444b9d16e2c2987"
    "9c1f74e07908b61f2b"
)

# Key recovered from the barcode elsewhere on the badge.
# ASCII: _f\E:R)XDhw%/b
KEY_HEX = "5F665C453A522958446877252F62"

# The first 8 bytes are the IV; the rest is the ciphertext.
# 216 bytes = 8-byte IV + 208-byte ciphertext = 26 Blowfish blocks.
# "Puffy" in the Stage 1 message hints at Blowfish (pufferfish).


def main() -> None:
    blob = bytes.fromhex(BLOB_HEX)
    key = bytes.fromhex(KEY_HEX)
    iv, ciphertext = blob[:8], blob[8:]

    plaintext = Blowfish.new(key, Blowfish.MODE_CBC, iv=iv).decrypt(ciphertext)

    # PKCS#7-style padding: the Stage 2 plaintext ended with eight 0x08 bytes.
    plaintext = plaintext.rstrip(b"\x08")
    print(plaintext.decode("utf-8", errors="replace"))


if __name__ == "__main__":
    main()
```

The ciphertext decrypts to this message:

```
Stage 2 found! Find the 9 words: https://pastebin.com/xZ2gGyL6  https://pastebin.com/WDX12UkP  Send your name (cryptax), this string (a136c5a8), and the 9 words you found to blackalps26@securelogon.ch
```

We have completed stage 2, but there's a stage 3 😆

## Stage 3 : Pastebin and LLM

The first [pastebin link](https://pastebin.com/xZ2gGyL6) contains a long (boring) text obviously generated by a LLM:

```
Dale found the parking space he always used was taken, so he drove around the block again. When he came to a certain intersection and saw a woman in a dark coat standing nearby, he turned and looked. He stopped there to find her. He was sure she was his wife.
 
"That's right," he said, smiling. "She's my wife." A few minutes later, when he turned his head to look at her, she had turned back into the parking lot with a man.
```

The second [pastebin link](https://pastebin.com/WDX12UkP) contains a Python script which explains how the text was generated:

```python
from transformers import GPT2LMHeadModel, GPT2Tokenizer
import torch
 
tokenizer = GPT2Tokenizer.from_pretrained("gpt2")
model = GPT2LMHeadModel.from_pretrained("gpt2")
 
prompt = "Dale found the parking space he always used was taken, so he drove around the block again."
input_ids = tokenizer.encode(prompt, return_tensors="pt")
 
output = model.generate(
    input_ids,
    max_new_tokens=60,
    do_sample=True,
    temperature=0.9,
    top_k=40,
    top_p=0.95,
)
 
print(tokenizer.decode(output[0], skip_special_tokens=True))
```

### Attempt with Claude Sonnet 5.5

Claude finds 8 words without problem, but is uncertain the nineth word is Many (paragraph 18) or Day (paragraph 35).

![](/images/compass-claude.jpg)

I send an email to the organizers with a first attempt and the word `Day`: `cryptax a136c5a8 Dinner Work Pretty Wine Summer Cash Church Quiet Day`, but it is not correct.


### Attempt with DeepSeek V4.1 Flash

DeepSeek finds similar words, but seems to prefer `Many` to `Day`. 

![](/images/compass-deepseek.png)

Its criteria is that the words should be that **they are not stopwords, proper nouns or contradictions**.

I don't want to spam the organizers, so I go and chat with them. They tell me my LLM's criteria are not correct, and that I should better look at how the text is generated.

### Understanding the script

I am not an expert at using Python Hugging Face classes, so I ask Claude to explain the code line by line to me.

The code uses the GPT-2 model, which is a "tiny" model (according to DeepSeek 😆).
Models work on *tokens*, right? But, internally, models work on *numbers* (maths), not on words. So, the first step is to

1. Split the prompt (a seed sentence) into tokens
2. Map each token to its ID.

```python
model = GPT2LMHeadModel.from_pretrained("gpt2") 
prompt = "Dale found the parking space he always used was taken, so he drove around the block again."
input_ids = tokenizer.encode(prompt, return_tensors="pt")
```

Then, the model predicts the next token from those IDs. The generation parameters show that:

1. The model will generate max 60 tokens (for more, we need to loop)
2. The temperature is very high, so very random
3. The model only considers the 40 most likely next tokens

```python
output = model.generate(
    input_ids,
    max_new_tokens=60,
    do_sample=True,
    temperature=0.9,
    top_k=40,
    top_p=0.95,
)
```

Finally, actually, the model predicts a token ID, so the program converts that back to text:

```python
print(tokenizer.decode(output[0], skip_special_tokens=True)
```

So, basically, we have to find words in the next that don't look like they could have been generated by the model, i.e their probability to be select is far too low.

### Finding the 9 words

LLM creates a script which does computations on each tokens of the text:

1. Computes the negative log-likelihood `NLL(w) = -log P(w | all preceding text)`
2. Rank of the token (rank 0 is the most probable token, rank 1 is the second most probable token, rank 50,256 is the least probable)
3. Compute p = exp(-NLL) which is the probablility of a token at that position. So p=1.7e-06 means there's a chance of 0.00017% of having the token there.

Then, the script displays the tokens by order of appearance in the story. We get the following output:

![](/images/compass-script.png)

The correct solution is `cryptax a136c5a8 Dinner Work Pretty Wine Summer Cash Church Quiet October`

## Conclusion

> I liked the challenge very much. It involved a **device, NFC, APDUs, and LLMs**. Would have been perfect for [ph0wn](https://ph0wn.org)!
> I used LLMs to solve the challenge, but (1) they didn't solve it autonomously (good), and (2) it didn't spoil the fun of the challenge.

I unfortunately did not get lucky at the raffle for the prize, never mind.

Perhaps the only thing I would have liked in this challenge is a way to check the words I find are the correct one, beyond sending an email and waiting for the success message 🤣




## Appendix: Code to test the tokens

This code was entirely written by DeepSeek. I had to tell it exactly what to do though.

```python
#!/usr/bin/env python3
"""Score every word of a GPT-2 story with GPT-2: NLL, rank and p.

Reproduces the table "Scores of the 9" from notes.md:

    NLL(w)  = -log P(w | all preceding text)          (nats)
    rank(w) = #{ v in vocab : P(v) > P(w) }           (0 = model's top guess)
    p(w)    = exp(-NLL(w))

Only the *first* sub-token of each word is scored, because that is the
position at which the choice of that word is actually predicted (the
remaining sub-tokens are near-certain continuations).

Scoring uses rolling 1024-token windows with a 256-token stride.  Among
all windows that can score a token, the one giving the most left context
is kept, so every token gets ~768-1023 tokens of context (except tokens
near the very start of the document, which get everything available).

Under the sampler in the second pastebin (temperature=0.9, top_k=40,
top_p=0.95) rank >= 40 means the token had probability exactly zero of
being generated there.

Usage:
    python3 score_words.py --markdown --top 9 --min-nll 9.0
    python3 score_words.py --top 80 --context-words 2
    python3 score_words.py --tokens-out tokens.tsv
"""

from __future__ import annotations

import argparse
import math
import re
from dataclasses import dataclass
from pathlib import Path

import torch
from transformers import GPT2LMHeadModel, GPT2TokenizerFast

WORD_RE = re.compile(r"[A-Za-z][A-Za-z'’-]*")
CONTEXT_WORD_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9'’-]*")

HERE = Path(__file__).resolve().parent
DEFAULT_STORY = HERE / "story.txt"


@dataclass
class WordOccurrence:
    word: str
    char_start: int
    char_end: int
    token_index: int
    nll: float
    rank: int
    p: float

    @property
    def char_offset(self) -> int:
        return self.char_start


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(
        description="GPT-2 NLL / rank / p scorer (reproduces the Black Alps notes).",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    ap.add_argument("--story", type=Path, default=DEFAULT_STORY,
                    help="text file to score")
    ap.add_argument("--model", default="gpt2", help="HF model id or local path")
    ap.add_argument("--window", type=int, default=1024,
                    help="context window in tokens")
    ap.add_argument("--stride", type=int, default=256,
                    help="window stride in tokens")
    ap.add_argument("--top", type=int, default=40,
                    help="how many word occurrences / words to print")
    ap.add_argument("--context-words", type=int, default=3,
                    help="how many words of context to show on each side")
    ap.add_argument("--min-nll", type=float, default=None,
                    help="also list every word with worst-occurrence NLL >= this")
    ap.add_argument("--markdown", action="store_true",
                    help="print the per-word table as markdown")
    ap.add_argument("--tokens-out", type=Path, default=None,
                    help="optional TSV dump with one row per scorable token")
    ap.add_argument("--device", default="auto", choices=["auto", "cpu", "cuda"],
                    help="device to run the model on")
    return ap.parse_args()


def pick_device(name: str) -> torch.device:
    if name == "auto":
        return torch.device("cuda" if torch.cuda.is_available() else "cpu")
    return torch.device(name)


def score_tokens(
    input_ids: list[int],
    model: GPT2LMHeadModel,
    device: torch.device,
    window: int,
    stride: int,
) -> tuple[list[float | None], list[int | None], list[int]]:
    """Return per-token (NLL, rank, left-context length)."""
    n_tokens = len(input_ids)
    nll: list[float | None] = [None] * n_tokens
    rank: list[int | None] = [None] * n_tokens
    best_context = [-1] * n_tokens

    with torch.inference_mode():
        for start in range(0, max(n_tokens - 1, 0), stride):
            end = min(start + window, n_tokens)
            if end - start < 2:
                continue

            chunk = torch.tensor([input_ids[start:end]], device=device)
            logits = model(chunk).logits[0, :-1].float()     # (T-1, vocab)
            targets = chunk[0, 1:]                            # (T-1,)

            # NLL = logsumexp(logits) - logit[target], vectorised.
            log_z = torch.logsumexp(logits, dim=-1)
            target_logit = logits.gather(1, targets[:, None]).squeeze(1)
            nll_chunk = (log_z - target_logit).tolist()
            # Number of vocabulary items strictly more probable than target.
            rank_chunk = (logits > target_logit[:, None]).sum(dim=-1).tolist()

            for p, (cur_nll, cur_rank) in enumerate(zip(nll_chunk, rank_chunk)):
                global_idx = start + p + 1
                context = p + 1
                if context > best_context[global_idx]:
                    best_context[global_idx] = context
                    nll[global_idx] = cur_nll
                    rank[global_idx] = cur_rank

            del logits, targets, log_z, target_logit

    return nll, rank, best_context


def map_words(
    text: str,
    offsets: list[tuple[int, int]],
    nll: list[float | None],
    rank: list[int | None],
) -> list[WordOccurrence]:
    """Attach each word to the token containing its first character."""
    occurrences: list[WordOccurrence] = []
    for match in WORD_RE.finditer(text):
        wa, wb = match.span()
        token_index = next(
            (i for i, (a, b) in enumerate(offsets) if a <= wa < b),
            None,
        )
        if token_index is None or nll[token_index] is None:
            continue
        occurrences.append(
            WordOccurrence(
                word=match.group(0),
                char_start=wa,
                char_end=wb,
                token_index=token_index,
                nll=nll[token_index],
                rank=rank[token_index],  # type: ignore[arg-type]
                p=math.exp(-nll[token_index]),  # type: ignore[arg-type]
            )
        )
    return occurrences


def snippet(
    text: str,
    start: int,
    end: int,
    context_words: int = 3,
    mark: bool = False,
) -> str:
    """Show `context_words` words before/after the target, with ellipses."""
    pre = list(CONTEXT_WORD_RE.finditer(text, 0, start))[-context_words:] if context_words else []
    post = []
    if context_words:
        for match in CONTEXT_WORD_RE.finditer(text, end):
            post.append(match)
            if len(post) >= context_words:
                break

    lo = pre[0].start() if pre else start
    hi = post[-1].end() if post else end
    before = re.sub(r"\s+", " ", text[lo:start])
    after = re.sub(r"\s+", " ", text[end:hi])
    word = f"**{text[start:end]}**" if mark else text[start:end]
    body = f"{before}{word}{after}".strip()
    return f"{'…' if lo > 0 else ''}{body}{'…' if hi < len(text) else ''}"


def worst_per_word(occurrences: list[WordOccurrence]) -> list[tuple[WordOccurrence, int]]:
    """One entry per surface form, keeping its most surprising occurrence."""
    groups: dict[str, list[WordOccurrence]] = {}
    for occ in occurrences:
        groups.setdefault(occ.word, []).append(occ)
    rows = [(max(group, key=lambda o: o.nll), len(group)) for group in groups.values()]
    return sorted(rows, key=lambda row: row[0].nll, reverse=True)


def select_and_order(
    rows: list[tuple[WordOccurrence, int]], limit: int
) -> list[tuple[WordOccurrence, int]]:
    """Keep the `limit` most surprising rows, then show them in text order."""
    top = sorted(rows, key=lambda row: row[0].nll, reverse=True)[:limit]
    return sorted(top, key=lambda row: row[0].char_start)


def print_word_table(
    rows: list[tuple[WordOccurrence, int]], text: str, limit: int
) -> None:
    print(f"\n=== PER-WORD (worst occurrence, first-appearance order) ===")
    print(f"{'offset':>7}  {'NLL':>8}  {'rank':>7}  {'p':>9}  {'n':>2}  word")
    for occ, count in select_and_order(rows, limit):
        print(f"{occ.char_start:7d}  {occ.nll:8.3f}  {occ.rank:7d}  "
              f"{occ.p:9.1e}  {count:2d}  {occ.word}")


def print_occurrence_table(
    occurrences: list[WordOccurrence], text: str, limit: int, context_words: int
) -> None:
    top = sorted(occurrences, key=lambda o: o.nll, reverse=True)[:limit]
    print(f"\n=== WORD OCCURRENCES (first-appearance order) ===")
    print(f"{'offset':>7}  {'NLL':>8}  {'rank':>7}  {'p':>9}  word           context")
    for occ in sorted(top, key=lambda o: o.char_start):
        ctx = snippet(text, occ.char_start, occ.char_end, context_words)
        print(f"{occ.char_start:7d}  {occ.nll:8.3f}  {occ.rank:7d}  "
              f"{occ.p:9.1e}  {occ.word:<13}  {ctx}")


def print_markdown_table(
    rows: list[tuple[WordOccurrence, int]], text: str, limit: int, context_words: int
) -> None:
    print("\n| offset | word | NLL (nats) | rank | p | context |")
    print("|---|---|---|---|---|---|")
    for occ, _count in select_and_order(rows, limit):
        ctx = snippet(text, occ.char_start, occ.char_end, context_words, mark=True)
        ctx = ctx.replace("|", "\\|")
        print(f"| {occ.char_start} | {occ.word} | {occ.nll:.2f} | "
              f"{occ.rank} | {occ.p:.1e} | \"{ctx}\" |")


def write_token_tsv(
    path: Path,
    input_ids: list[int],
    offsets: list[tuple[int, int]],
    nll: list[float | None],
    rank: list[int | None],
    context: list[int],
    tokenizer: GPT2TokenizerFast,
    occurrences: list[WordOccurrence],
) -> None:
    first_token_word = {occ.token_index: occ.word for occ in occurrences}
    with path.open("w", encoding="utf-8") as fh:
        fh.write("index\ttoken\tchar_start\tchar_end\tcontext_tokens\tnll\trank\tp\tword\n")
        for i, tid in enumerate(input_ids):
            token = tokenizer.decode([tid])
            token = token.replace("\\", "\\\\").replace("\t", "\\t").replace("\n", "\\n")
            a, b = offsets[i]
            cur_nll = "" if nll[i] is None else f"{nll[i]:.6f}"
            cur_rank = "" if rank[i] is None else str(rank[i])
            cur_p = "" if nll[i] is None else f"{math.exp(-nll[i]):.6e}"
            cur_ctx = "" if nll[i] is None else str(context[i])
            word = first_token_word.get(i, "")
            fh.write(f"{i}\t{token}\t{a}\t{b}\t{cur_ctx}\t{cur_nll}\t{cur_rank}\t{cur_p}\t{word}\n")


def main() -> None:
    args = parse_args()

    text = args.story.read_text(encoding="utf-8")
    device = pick_device(args.device)
    print(f"story:  {args.story}")
    print(f"model:  {args.model}   device: {device}")

    tokenizer = GPT2TokenizerFast.from_pretrained(args.model)
    model = GPT2LMHeadModel.from_pretrained(args.model).to(device)
    model.eval()

    enc = tokenizer(
        text,
        return_tensors="pt",
        return_offsets_mapping=True,
        add_special_tokens=False,
    )
    input_ids = enc["input_ids"][0].tolist()
    offsets = [tuple(pair) for pair in enc["offset_mapping"][0].tolist()]

    print(f"tokens: {len(input_ids)}   chars: {len(text)}")
    print(f"window: {args.window}   stride: {args.stride}")

    nll, rank, context = score_tokens(input_ids, model, device, args.window, args.stride)
    scored = sum(1 for value in nll if value is not None)
    print(f"scored: {scored} / {len(input_ids)} tokens "
          f"(context {min(context[i] for i in range(len(context)) if nll[i] is not None)}"
          f"-{max(context)} tokens)")

    occurrences = map_words(text, offsets, nll, rank)
    words = worst_per_word(occurrences)

    if args.markdown:
        print_markdown_table(words, text, args.top, args.context_words)
    else:
        print_word_table(words, text, args.top)
        print_occurrence_table(occurrences, text, args.top, args.context_words)

    if args.min_nll is not None:
        hits = [(occ, count) for occ, count in words if occ.nll >= args.min_nll]
        hits.sort(key=lambda row: row[0].char_start)
        print(f"\n=== WORDS WITH NLL >= {args.min_nll} (first-appearance order) ===")
        print(" ".join(occ.word for occ, _ in hits))
        print(f"count: {len(hits)}")

    if args.tokens_out is not None:
        write_token_tsv(
            args.tokens_out, input_ids, offsets, nll, rank, context,
            tokenizer, occurrences,
        )
        print(f"\ntoken scores written to {args.tokens_out}")


if __name__ == "__main__":
    main()
```