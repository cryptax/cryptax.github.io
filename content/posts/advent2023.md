---
layout: post
title: "Advent of Code 2023"
author: cryptax
date: 2023-12-02
tags:
- Advent
- Code
- 2023
- Python
---

*Advent of Code* is a coding competition, with one new challenge per day during December.

**SPOILER ALERT** in this post!

# Advent of Code - December 1st

We are asked to produce a program which takes the first digit of each line and the last:

```
1abc2 -> 12
pqr3stu8vwx -> 38
a1b2c3d4e5f -> 15
treb7uchet -> 77
```

My solution in Python. It's certainly not the best solution, but it works:

```python
#!/usr/bin/env python3
import logging

log_format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.INFO)
logger = logging.getLogger()

def get_calibration_value(s: str) -> int:
    length = len(s)
    for c in s:
        if c.isdigit():
            value = c
            break

    for c in reversed(s):
        if c.isdigit():
            value = value+c
            break
    logger.debug(f'input={s} value={value}')
    return int(value)

def test():
    test = ['1abc2', 'pqr3stu8vwx', 'a1b2c3d4e5f', 'treb7uchet' ]
    expected = [ 12, 38, 15, 77 ]
    for t in range(0, len(test)):
        value = get_calibration_value(test[t])
        if value != expected[t]:
            logger.error(f'Error for {t}: value={value}, expected={expected[t]}')
        else:
            logger.debug(f'[+] test #{t} ok')

def parse_file(filename='input.txt') -> int:
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    total = 0
    for line in lines:
        value = get_calibration_value(line)
        total = total + value
    logger.debug(total)
    return total

if __name__ == '__main__':
    test()
    logger.info(f'Total={parse_file()}')
```

I read on [Mastodon a much more elegant solution with *awk*](https://chaos.social/@root42/111505305360222320)

```awk
awk '{gsub(/[a-z]*/,""); num = substr($0,0,1)substr($0,length,1); sum += num } END {print sum}' < input.txt
```

#
