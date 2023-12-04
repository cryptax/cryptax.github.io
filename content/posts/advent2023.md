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

## 1.1

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

## 1.2

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

## 2.1

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

## 2.2

The minimum number of cubes for a given color consists in the *maximum* of cubes across all rounds of a given line. So, basically, I implemented a function that split each round of a line (using `line.split(';')`) and kept the maximum values for each count of cubes for a given line.

```python
for r in rounds:
  red = get_color_count(r, 'red')
  blue = get_color_count(r, 'blue')
  green = get_color_count(r, 'green')
  if red > max_red:
    max_red = red
```

# Advent of Code - December 3rd

## 3.1

We get a file with numbers and miscellaneous characters. We must add all numbers who have a misc characters just next to the numbers, let that be left, right, up, down or in diagonal. (`.` does not count as a character).

What a pain!

1. Pay attention to the fact the character may be close to a number in the middle. For example, `123` counts because a star is below number 2.

```
...123...
....*....
```

2. I had to debug my program, and that was very painful. I created a better/longer test that the one provided by the website.

```
467*......
.....+12..
5..3..78..
+.......*.
........65
.12.......
*......99.
.7..44...*
........5.
...12...5.
.*789*....
....1901*.
.23..2....
```

In my implementation, I chose to represent the input file as a single one-dimension array, concatenating all lines, and then use `% LINE_LEN` operators.
The first task is to find line length:

```python
f = open(filename, 'r')
data = f.read()
f.close()
m = re.search('\n', data)
line_len = m.span()[0]
```

Then, basically, what I did is :

1. Spot all numbers in a line
2. For each of the numbers, check if there's a character nearby
3. Add value

To spot all numbers in a line, I used a convenient regex: 

```python
line = data[index:index+LINE_LEN]
numbers = re.findall('[0-9]+', line)
```

Finding the position of each number in the data is more complicated, and was my major bug. I used `re.search()` initially. But this is a **bad idea**. Suppose your line has `123.....2`. First, you search for `123`: no problem. Then, we search for `2`: issue, because `2` is also part of `123`...
I ended up coding the routine myself, where:

- line is a the buffer of characters for that line
- number is the string representing the number to search for
- offset is the index to start the search in the line, because we're not going to search from the beginning all the time.

```python
def get_number_pos(line: str, number: str, offset: int) -> int:
    for i in range(offset, len(line)):
        if line[i:].startswith(number):
            return i
    logger.error(f'Should not happen: line={line} number={number}')
    quit()
```

Then, the idea is to note that the array of numbers returned by `re.findall()` is ordered: the numbers which appear first in the line are shown first.
So, I'm going to search for the number and keep incrementing the position in the line:

```python
    p = 0
    for n in numbers:
        # get the position of each number
        p = get_number_pos(line, n, p)
        logger.debug(f'Processing {n} at {index+p}-{index+p+len(n)}')
        for pos in range(index+p, index+p+len(n)):
            if has_nearby_character(data, pos, LINE_LEN):
                count = count + int(n)
                logger.info(f'Counting {n}')
                break
        p = p + len(n)
```

Finally, of course, you need to code a routine that tells you if there's a nearby character. It's a big long, but not too difficult:

```python
def has_nearby_character(data: str, index: int, LINE_LEN: int) -> bool:
    assert(data[index].isdigit()), f'data[{index}]={data[index]} is not a digit'
    # do we have a chararacter left
    if index > 0 and index % LINE_LEN != 0:
        if is_character(data, index -1):
            return True
	...
    # diagonal down right
    if index + LINE_LEN < len(data) and index % LINE_LEN != LINE_LEN -1:
        if is_character(data, index + LINE_LEN + 1):
            return True

    logger.debug(f'No nearby char for index={index}')
    return False
```

To summarize, despite a simple description, this exercise was painful to implement and to debug (and not very interesting IMHO).

## 3.2

In this new task, you have to multiple numbers which are around a star `*`, whether those numbers are above, below, left, right or diagonally. The task is slightly less difficult than the first one, the main difficulty consisting in correctly implementing how to search for a number at a given position.

My solution consists in:

1. Finding all stars of a given line

```python
    position = []
    # find all starts in a line
    for i in range(index, index+LINE_LEN):
        if data[i] == '*':
            position.append(i)
```

2. For each star, finding if it's surrounded by a digit. 

```python
    # do we have a number left
    if index > 0 and index % LINE_LEN != 0 and data[index-1].isdigit():
		...
	# do we have a chararacter right
    if index % LINE_LEN != LINE_LEN - 1 and data[index+1].isdigit():
		...
```

3. If it's surrounded by a digit, then get the surrounding number. The number may appear in a "window" of 5 characters around that digit + you need to handle correctly beginning and ends of lines. This is the most difficult part IMHO.


4. If we found a pair of surrounding numbers, then multiply them to get the gear value


This is the code to get the surrounding number:

```python
def get_number(data: str, offset: int, LINE_LEN: int) -> int:
	'''
	when we call this function, offset contains the index 
	of a number which is surrounding a star. Always good 
	to check data[offset] is really a digit, if not
	there's a bug for sure
	'''
    assert(data[offset].isdigit()), f'data[{offset}] is not a digit!'

	'''
	we'll want to ensure we search for the surrounding number 
	on a given line so, we compute the current line
	'''
	offset_line = offset // LINE_LEN
	# the idea is to have at the end the number 
	# between begin_offset and end_offset
	end_offset = offset
   
	# we're going to search character by character 
	# if the number spans to the right
	max_offset = offset+3
	if max_offset // LINE_LEN != offset_line:
        # different line, we need to handle the last offset to search
        max_offset = ((offset_line+1) * LINE_LEN) - 1

	for i in range(offset, max_offset+1):
		if not data[i].isdigit():
            end_offset = i-1
            break
        end_offset = i
		
	'''
	we have the end offset of the number, now, we need to 
	find the beginning offset. It's the same procedure except
	we go backwards
	'''
	min_offset = offset-3
	begin_offset = offset
	if min_offset // LINE_LEN != offset_line:
        min_offset = offset_line*LINE_LEN

	for j in range(offset, min_offset-1, -1):
        logger.debug(f'Searching before: data[{j}]={data[j]}')
        if not data[j].isdigit():
            begin_offset = j+1
            break
        begin_offset = j

	return int(data[begin_offset: end_offset+1])
```



