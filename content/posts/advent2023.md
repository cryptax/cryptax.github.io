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

## Task 1 

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

## Task 2

In this second task, we have to handle numbers which can be spelt with letters like 'one'. Stupidly, I struggled on this one, handling incorrectly the parse from the right.

```python
#!/usr/bin/env python3
import re
import logging

log_format = "%(asctime)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.DEBUG)
logger = logging.getLogger()

def count_left(line: str) -> int:
    index = 0
    while index < len(line):
        logger.debug(f'index={index} {line[index:]}')
        if line[index].isdigit():
            return int(line[index])

        texts = [ 'zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine' ]
        for j in range(0, len(texts)):
            if line[index:].startswith(texts[j]):
                return j

        index = index + 1
    logger.error(f'left: we havent found anything: {line}')
    quit()


def count_right(line: str) -> int:
    index = len(line) - 1
    while index >= 0:
        logger.debug(f'index={index} {line[index:]}')
        if line[index].isdigit():
            return int(line[index])

        texts = [ 'zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine' ]
        for j in range(0, len(texts)):
            if line[index:].startswith(texts[j]):
                return j
        index = index -1
    logger.error(f'right: we havent found anything: {line}')
    quit()
    
        
def parse_file(filename='input.txt') -> int:
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    total = 0
    for line in lines:
        left = count_left(line.strip())
        right = count_right(line.strip())
        total = total + int(f'{left}{right}')
    logger.debug(total)
    return total

def test():
    test = ['two1nine', 'eighttwothree', 'abcone2threexyz', 'xtwone3four', '4nineeightseven2', 'zoneight234' ]
    expected = [ 29, 83, 13, 24, 42, 14 ]
    for t in range(0, len(test)):
        left = count_left(test[t])
        right = count_right(test[t])
        value = int(f'{left}{right}')
        if value != expected[t]:
            logger.error(f'Error for {t}: value={value}, expected={expected[t]}')
        else:
            logger.debug(f'[+] test #{t} ok')

if __name__ == '__main__':
    #test()
    logger.info(parse_file())
```


# Advent of Code - December 2nd

For this second day, we play a game with the elves where they put a given number of cubes red, blue and green in a bag. The game consists in several rounds where the elf picks up randomly cubes in the bag and tells you which color there are.

In the first task, we just had to say if the output was possible for 12 red cubes, 13 green and 14 blue.

In the second task, we have to say the minimum of cubes possible for each color in each game.

## Task 1

```python
#!/usr/bin/env python3
import logging
import re

log_format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.INFO)
logger = logging.getLogger()

def get_id(line: str) -> int:
    match = re.search(r'Game ([0-9]+)', line)
    return int(match.group(1))

def get_color_count(line: str, color: str) -> int:
    m = re.findall(f'([0-9]+) {color}', line)
    count = 0
    for i in m:
        count = count + int(i)
    return count

def parse_file(filename='input2.txt') -> int:
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    sum = 0
    for line in lines:
        id = get_id(line)
        # get that each round has a correct amount of cubes
        rounds = line.split(';')
        correct = True
        for r in rounds:
            red = get_color_count(r, 'red')
            blue = get_color_count(r, 'blue')
            green = get_color_count(r, 'green')
            if red >12 or green >13 or blue >14:
                logger.debug(f'id={id} incorrect round={r}')
                correct = False
                break
        if correct:
            sum = sum + id
    return sum

if __name__ == '__main__':
    logger.info(f'Total={parse_file()}')

```

## Task 2

```python
#!/usr/bin/env python3
import logging
import re

log_format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.INFO)
logger = logging.getLogger()

def get_id(line: str) -> int:
    match = re.search(r'Game ([0-9]+)', line)
    return int(match.group(1))

def get_color_count(line: str, color: str) -> int:
    m = re.findall(f'([0-9]+) {color}', line)
    count = 0
    for i in m:
        count = count + int(i)
    return count

def get_power(line: str) -> int:
    rounds = line.split(';')
    max_red = 0
    max_green = 0
    max_blue = 0
    for r in rounds:
        red = get_color_count(r, 'red')
        blue = get_color_count(r, 'blue')
        green = get_color_count(r, 'green')
        if red > max_red:
            max_red = red
        if blue > max_blue:
            max_blue = blue
        if green > max_green:
            max_green = green
    logger.debug(f'red={max_red} green={max_green} blue={max_blue}')
    return max_red * max_blue * max_green
    

def parse_file(filename='input2.txt') -> int:
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    sum = 0
    for line in lines:
        id = get_id(line)
        power = get_power(line)
        sum = sum + power
    return sum

if __name__ == '__main__':
    logger.info(f'Total={parse_file()}')
```
