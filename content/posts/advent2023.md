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

*Advent of Code* is a coding competition, with one new challenge per day during December. I'll update this post every time I have time to solve some challenges.

As it would be a long (and boring?) post to include all the details of my code, I'll just highlight the important points or things I learned doing it. **SPOILER ALERT** though: don't read this post if you're doing the challenge yourself.

# Advent of Code - December 1st

## Task 1 

We are asked to produce a program which takes the first digit of each line and the last:

```
1abc2 -> 12
pqr3stu8vwx -> 38
a1b2c3d4e5f -> 15
treb7uchet -> 77
```

This is not very difficult, I wrote it in Python, I just paid attention to the fact I wanted *neat code*, if possible re-usable, and not a *quick and dirty* solution.

So, first of all, we want logs, so as to debug the code when necessary:

```python
import logging

log_format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.INFO)
logger = logging.getLogger()
```

Also, I used Python type hints. In a small project such as this one, it's over-kill, but I like the idea in recent Python program because it clarifies input arguments.

```python
def get_calibration_value(s: str) -> int:
```

There are many ways to solve this task, and at second thought, I didn't use the best one. Basically, what I do is parse each line, first from left to right until I find a digit (`isdigit()`), and then from right to left using `reversed(s)` and `isdigit()`.

I coded a test function to verify my calibration values matched the expected ones. Certainly not "necessary", but IMHO a good thing to do.

Finally, I parsed each line of the input file using `readlines()`.


I read on [Mastodon a much more elegant solution with *awk*](https://chaos.social/@root42/111505305360222320)

```awk
awk '{gsub(/[a-z]*/,""); num = substr($0,0,1)substr($0,length,1); sum += num } END {print sum}' < input.txt
```

## Task 2

In this second task, we have to handle numbers which can be spelt with letters like 'one'. Stupidly, I struggled on this one:

- Pay attention that something like `oneight` must be transformed as `1ight`
- And **also** do it correctly from right to left: `oneight` is now transformed `on8`

The test values contained `sixteen` but if you `grep` the input file, notice every number was been `one` and `nine`. So, we can simplify the algorithm.

From left to right, the idea is to stop as soon as you find a digit, or if `line[pos:]` starts with one of the texts between `one` and `nine`. You need to parse manually the line and increment a position index (`pos`). If you spot a text, be sure to increment the index by the length of the text.

From right to left, the idea is exactly the same, except you're going to parse the line from the last character to the beginning.

On Internet, I found a very short solution in Python. It works, it's short, but it's ugly to read:

```python
f = lambda str, dir: min((str[::dir].find(num[::dir])%99, i) for i, num in enumerate(
    '1 2 3 4 5 6 7 8 9 one two three four five six seven eight nine'.split()))[1]%9+1

print(sum(10*f(x, 1) + f(x, -1) for x in open('input.txt')))
```

# Advent of Code - December 2nd

For this second day, we play a game with the elves where they put a given number of cubes red, blue and green in a bag. The game consists in several rounds where the elf picks up randomly cubes in the bag and tells you which color there are.

In the first task, we just had to say if the output was possible for 12 red cubes, 13 green and 14 blue.

In the second task, we have to say the minimum of cubes possible for each color in each game.

## Task 1

To retrieve the ID of the game, I used a simple regex:

```python
def get_id(line: str) -> int:
    match = re.search(r'Game ([0-9]+)', line)
    return int(match.group(1))
```

To count the number of cubes of a given color on a given line, we need to find *all* matches, so it's a `findall`:

```python
def get_color_count(line: str, color: str) -> int:
    m = re.findall(f'([0-9]+) {color}', line)
    count = 0
    for i in m:
        count = count + int(i)
    return count
```

Then, we parse each line of the file. A game is subdivided in what I called "rounds". We check that we don't have more 12 red, 13 green and 14 blue in each round.

```python
        for r in rounds:
            red = get_color_count(r, 'red')
            blue = get_color_count(r, 'blue')
            green = get_color_count(r, 'green')
            if red >12 or green >13 or blue >14:
                logger.debug(f'id={id} incorrect round={r}')
                correct = False
                break
```

## Task 2

The minimum number of cubes for a given color consists in the *maximum* of cubes across all rounds of a given line. So, basically, I implemented a function that split each round of a line (using `line.split(';')`) and kept the maximum values for each count of cubes for a given line.

```python
for r in rounds:
  red = get_color_count(r, 'red')
  blue = get_color_count(r, 'blue')
  green = get_color_count(r, 'green')
  if red > max_red:
    max_red = red
```
