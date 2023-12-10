---
layout: post
title: "Advent of Code 2023"
author: cryptax
date: 2023-12-07
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

We get a file with numbers and miscellaneous characters. We must add all numbers who have a misc characters just next to the numbers, let that be left, right, up, down or in diagonal. Character `.` does not count as a character).

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

> Lessons Learned: it's documented, but pay attention: `re.search()` only returns the first occurence.

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

# Advent of Code - December 4

## 4.1

The task consists in help count points for a game. There are winning numbers (like lotery), and on the other side, cards with numbers that the elf owns. We have to count the amount of winning numbers the elf has. The score for the game is then doubled for each subsequent winning number e.g 1 winning number = 1 point, 2 winning = 2 points, 3 winning = 4 points etc.

This task is *way* easier than on December 3 and I coded it rather quickly.

Lines are formatted as such:

```
Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
```

For each line, we want to retrieve the winning numbers (on the left) and the cards the elf has (on the right). For that, I spot the position of `:` (beginning of winning numbers) and `|` (end of winning numbers, beginning of cards of the elf).

I create an array using `split()`. As there are sometimes multiple spaces that would generate empty entries in the array (`['86', '', '6']`), I use a Python trick to remove the spaces: `' '.join(array.split())`: this splits the array, then re-assembles each entry with a single space.

Then, basically, the task is completed.

```python
def process(filename='input4.txt') -> int:
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()

    total = 0
    for line in lines:
        begin = line.find(':') + 1
        separator = line.find('|')
        winning = ' '.join(line[begin:separator].split()).split(' ')
        cards = ' '.join(line[separator+1:].replace('\n','').split()).split(' ')
        logger.debug(f'winning={winning}')
        logger.debug(f'cards={cards}')

        count = 0
        for w in winning:
            if w in cards:
                count = count + 1
        if count > 0:
            total = total + 2**(count -1)
            logger.debug(f'count={count} value={2**(count-1)} total={total}')
    logger.info(f'Total={total}')
```

### Other solutions

I saw a few other solutions on the net. Overall, I'm pretty happy with mine which is both short and easy to understand. 

To count the number of winning numbers, I read a solution which uses `set.intersection`.

In **Bash**, this one-liner solution is fantastic:

```bash

cat input4.txt | cut -d: -f2 | tr -d '|' | xargs -I {} sh -c "echo {} | tr ' ' '\n' | sort | uniq -d | wc -l " | awk '$1>0 {print 2^($1-1)}' | paste -sd+ - | bc
```

I didn't know command `paste`: it merges lines of files.


## 4.2

### Description

This is the description of the task:

>you win copies of the scratchcards below the winning card equal to the number of matches. So, if card 10 were to have 5 matching numbers, you would win one copy each of cards 11, 12, 13, 14, and 15.
>Copies of scratchcards are scored like normal scratchcards and have the same card number as the card they copied. So, if you win a copy of card 10 and it has 5 matching numbers, it would then win a copy of the same cards that the original card 10 won: cards 11, 12, 13, 14, and 15. This process repeats until none of the copies cause you to win any more cards. (Cards will never make you copy a card past the end of the table.)

### Example

```
Card 1: 41 48 83 86 17 | 83 86  6 31 17  9 48 53
Card 2: 13 32 20 16 61 | 61 30 68 82 17 32 24 19
Card 3:  1 21 53 59 44 | 69 82 63 72 16 21 14  1
Card 4: 41 92 73 84 69 | 59 84 76 51 58  5 54 83
Card 5: 87 83 26 28 32 | 88 30 70 12 93 22 82 36
Card 6: 31 18 13 56 72 | 74 77 10 23 35 67 36 11
```

- Card 1 has four matching numbers, so you win one copy each of the next four cards: cards 2, 3, 4, and 5.
- Your original card 2 has two matching numbers, so you win one copy each of cards 3 and 4.
- Your copy of card 2 also wins one copy each of cards 3 and 4.
- Your four instances of card 3 (one original and three copies) have two matching numbers, so you win four copies each of cards 4 and 5.
- Your eight instances of card 4 (one original and seven copies) have one matching number, so you win eight copies of card 5.
- Your fourteen instances of card 5 (one original and thirteen copies) have no matching numbers and win no more cards.
- Your one instance of card 6 (one original) has no matching numbers and wins no more cards.
- Once all of the originals and copies have been processed, you end up with 1 instance of card 1, 2 instances of card 2, 4 instances of card 3, 8 instances of card 4, 14 instances of card 5, and 1 instance of card 6. In total, this example pile of scratchcards causes you to ultimately have 30 scratchcards!

### Solution

This second task is a bit more complex than the first one but still easier than the day before :)
The complexity is not to forget to process new cards you won.
I thought I'd have to use recursivity, but it didn't turn out to be necessary.

I chose to keep an array of cards I had. At the beginning, I have exactly 1 card of each.

```python
cards = [ 1 ] * len(lines)
```

We re-use the way to get the array of winning numbers, numbers and to count the number of wins.
This time, I made a function to count wins:

```python
def count_wins(winning: list[str], numbers: list[str]) -> int:
    count = 0
    for w in winning:
            if w in numbers:
                count = count + 1
    return count
```	

Then, we need to implement the amount of cards we win. When we have several copies of the current card, we are going to win n=number of copies cards for the next cards:

```python
count = count_wins(winning, numbers)
logger.debug(f'Card {card_no} has {count} wins')
for i in range(0, count):
    if card_no + i < len(lines):
        logger.debug(f'Winning card {card_no+i+1}')
        cards[card_no+i] = cards[card_no+i] + cards[card_no-1]
```

Then, we merely have to count all cards we have:

```python
total = 0
for c in cards:
    total = total + c
```	

> Lessons learned: type hinting for an array is `list[int]` for example. If you need to copy an array and copies have a different life, then use `a.copy()`. I actually didn't have to use that.

### Other solutions

There's another fabulous Bash one-liner for this task that I found on Internet:

```
cat /tmp/input4.txt | cut -d: -f2 | tr -d '|' | xargs -I {} sh -c "echo {} | tr ' ' '\n' | sort | uniq -d | wc -l " | cat -n | xargs | awk '{ for (i = 1; i < NF; i = i + 2) { copies[$i] = $(i + 1); vals[$i] = 0 } } { for (i = 1; i <= NF/2; i++) { counter=0; for (j = i + 1; counter < copies[i] && copies[i] > 0; j++) { counter++; vals[j] = vals[j] + 1 + vals[i] } } } { for(i=1;i<=NF/2;i++) { print vals[i] } print NF/2 }' | paste -sd+ - | bc
```
# Advent of Code - December 5th

## 5.1

The description of the day's task is long, but the task in itself isn't too difficult.

The first step is to read the input file:

- Read the seed line, which is handled differently from the other maps.
- Be able to detect the beginning of a map
- Be able to read a "dst src len" range

The implementation is quite appropriate with Python.

### Read seed line

```python
def get_seed_list(line: str) -> list[int]:
    assert(line.startswith('seeds:')), f'this is not a seed line: {line}'
    seeds = [ int(x) for x in line[len('seeds: '):].split(' ') ]
    logger.debug(f'seeds={seeds}')
    return seeds
```	

### Reading the maps

I'm quite happy with my implementation which basically uses "pointers" on various map arrays: `array` is the pointer, and it will point at `seed_to_soil`, `soil_to_fertilizer` etc depending on the context.

We have a few empty lines that I just skip with `if not lines[...].isdigit()`.

```python
def get_maps(lines: list[str]):
    array = []
    tags = { 'seed-to-soil' : seed_to_soil,
             'soil-to-fertilizer' : soil_to_fertilizer,
             'fertilizer-to-water' : fertilizer_to_water,
             'water-to-light' : water_to_light,
             'light-to-temperature' : light_to_temperature,
             'temperature-to-humidity' : temperature_to_humidity,
             'humidity-to-location' : humidity_to_location }
    
    for line_num in range(1, len(lines)):
        for k in tags.keys():
            if lines[line_num].startswith(k):
                array = tags[k]
                break
        if not lines[line_num][0].isdigit():
            next
        else:
            tab = lines[line_num].strip().split(' ')
            array.append([ int(x) for x in tab ])
```

### Computing destination value

This is straight forward. I'll just keep in mind the type hint for a list of lists is for example `list[list[int]]` :)

```python
def get_dst(map : list[list[int]], value: int) -> int:
    for m in map:
        logger.debug(f'dst={m[0]} src={m[1]} len={m[2]}')
        if value >= m[1]  and value < (m[1] + m[2]):
            dst = m[0] + value - m[1]
            logger.debug(f'src_value={value} -> dst_value={dst}')
            return dst
    return value
```	

### Putting it together

Finally, you just need to compute the location for each seed and keep the minimum location

```python
    for s in seeds:
        soil = get_dst(seed_to_soil, s)
        fertilizer = get_dst(soil_to_fertilizer, soil)
        water = get_dst(fertilizer_to_water, fertilizer)
        light = get_dst(water_to_light, water)
        temperature = get_dst(light_to_temperature, light)
        humidity = get_dst(temperature_to_humidity, temperature)
        location = get_dst(humidity_to_location, humidity)
        logger.info(f'seed={s} is at location={location}')
        if location < min_location:
            min_location = location
```

## 5.2

The difference in this second task is that the seed line should be considered as `basenum len`, i.e you'll have seed `basenum`, `basenum +1` ... until you have len elements.

This is a very small algorithmic change... but with the real input file, there is a *huge* difference: we are going to have **huge ranges** of seeds to test.

```
seeds: 3082872446 316680412 2769223903 74043323 
```

Impossible to memorize all seeds in an array: too much memory. That's not a big issue, because we don't really need to store the array, a loop parsing each seed is fine.

The biggest issue is that there's still lots of seeds to compute the location for.
Python is not performance driven: I could have modified the algorithm to use threads, or to compute seed range intersections.
Instead, I just decided to use an another programming language and fell back to C.

My implementation follows the same logic. See the [full source code at the end of the page](#d5t2_code). 

It runs, unoptimized, on my host on 7 minutes 43. I'm disappointed, I read a similar implementation on Java (but using Hash maps) which runs in ~3 minutes.

| Implementation | Time to solution |
| -------------- | ---------------- |
| `gcc d5t2.c -o d5t2` | 7 min 43 |
| **Optimized**: `gcc -O3 ...` | 2 min 20 |
| Java | 3 min 30 |

> Lessons Learned: (1) I already knew that, but Python is really bad for performances. (2) Compiler optimizations have a strong impact on performances too.

Alternatives: [this implementation in Perl runs **super fast**](https://topaz.github.io/paste/#XQAAAQCZBgAAAAAAAAA6nMjJFMpQu0F8vIUYE8mlPji8DyQkFpThIAFfXA1uKGMODfR8qSzU2hkIZRiADH0kfb2NH1c1SEcTc67ExT0Awc+mO45muq8KALPHQSSdwr/rgLF92VWY1pIMDxaXOl7f67pWJiSI05R5nN7Zz2urm2GQFNOFpWmSr5MYB0+ybOEtN+bt3Q6Ss2gI8+g1JCdXwZgoXUYOrJL7lcI0AVNk84SddSYeshj4gEIkOl4I2qfW78wJz1cVSiDoWmAuGtPTmbEIMOeOLhkmV6toZPRuTyWTo68eVPkHH2nQGnbnfV17q9XWAva+IDeVkeKhRYEQEx7BtGsOafxKfgqGCoPC5pfU+nFs9zXL6ZlH8rYSceOIljUjY309Ie+tIPC9ObtP6tU5XYeLMI4Ml2jHlmdJap5U4MWd6x1DC2EUN/lY+16+gqopbaQIzobbIueyzQ5EdgYJu+Sg3/Fy+2P1rFNDvMzoms7SIVVue44A2oXlg1iwz/6OP/FrmyzfjdLRiecA27L42dKdJX9qsP3F2iwwFwcWI+/Hdq+fJ3VkPNhXt7i6s0Vu1JqzuiznolUE7w1ufoeVLsymWUnuAnQGVZqdnMfCTlKlG2nSwVkgrEEnZXd0DpLayzZEjt93v4LXDIrAGRYmn4JOs1wyz6oyx1l8Sk27uLdH1lM0swq5+KmM8k5n0rc9gA4O9Nh2dw1YKmgTAEGCb3etFSWXs3EvWbDR6FXBLM3h/2Fg8wA=)

```perl

my $ranges = \@seedsRanges;
foreach my $chunk (@chunks) {
    my ($type, $map) = @$chunk;
    my @nextRanges;
    foreach my $line (@$map) {
        last unless @$ranges;
        my ($dest, $src, $len) = @$line;
        my @nopeRanges;
        foreach my $range (@$ranges) {
            my ($pos, $plen) = @$range;
            if ($pos + $plen < $src) {
                push @nopeRanges, [$pos, $plen];
            } elsif ($pos >= $src + $len) {
                push @nopeRanges, [$pos, $plen];
            } else {
                my $start = max($pos, $src);
                my $end = min($pos + $plen, $src + $len);
                my $newPos = $dest + $start - $src;
                my $newLen = $end - $start;
                push @nextRanges, [$newPos, $newLen];
                if ($pos < $src) {
                    push @nopeRanges, [$pos, $src - $pos];
                }
                if ($pos + $plen > $src + $len) {
                    push @nopeRanges, [$src + $len, $pos + $plen - $src - $len];
                }
            }
        }
        $ranges = \@nopeRanges;
    }
    push @nextRanges, @$ranges;
    $ranges = \@nextRanges;
}

my $minStart = min map { $_->[0] } @$ranges;
print "Min start: $minStart\n";
```
# December 6 - Race with charger

## 6.1

This task is really simple. You must compute how many times you can beat a race record.
A race consists of a given duration. If you hold the charge button during x ms, you'll have a speed of x mm per ms. But of course, the time you spend charging (at the beginning) is time you can't move.

So, the formula to compute the distance is `hold*(duration-hold)`.

You get an input with several race durations and their corresponding record.
You must compute how many times you can beat the record, and multiply those values.

As the input is small, I hard coded it:
```c
struct race {
  int duration;
  int distance;
};

struct race input[] = { { 47, 282 }, {70, 1079}, {75, 1147}, {66, 1062}};
```

I brute forced all hold lengths, removing 0 (if you don't hold, you don't move, no way to win), same if you hold during nearly all the time. Obviously the best distances are going to be somewhere in the middle.

```c
#define COMPUTE_DISTANCE(hold, duration) (hold*(duration-hold))

int beat_record() {
  int race_index;
  int result = 1;
  for (race_index = 0; race_index < 4; race_index++) {
    int hold;
    int beat_record = 0;
    for (hold = 1; hold < input[race_index].duration -1; hold++) {
      if (COMPUTE_DISTANCE(hold, input[race_index].duration) > input[race_index].distance)
	beat_record++;
    }
    printf("race[%d]: %d ways to beat record\n", race_index, beat_record);
    result = result * beat_record;
  }
  return result;
}
```

On Internet, several solved the equation where

```
hold * (duration - hold) = distance
-hold^2 + duration*hold - distance = 0
```

The solution is:

1. Compute  `d = sqrt(time^2 - 4 * distance)`
2. Range where we beat: `(time -d)/2 < hold < (time + d)/2`

As the brute force program works immediately (no lag), I feel personally this is overkill ;)

## 6.2

This second task is nearly the same thing expect there is a single race with large values:

```c
struct race input = { 47707566, 282107911471062 };
```

We do the same, except we're going to use `unsigned long long int` (actually we could have used smaller).

My initial solution did not work. I couldn't see the solution. After a while, I noticed my input was bad ;-(

The only goal of this second task is to have people deal with long integers.

# December 7 - Poker

Today, it's implementation of a Poker game. You have to rank hands, and then produce a result which is rank * bid for each hand.

## 7.1

I implemented the task in C, but it would have worked well in Python too.
My algorithm uses 3 "tricks".

1. To detect a hand's type (one pair, two pairs, full house etc), we can actually count the number of similar cards for each of the 5 cards, and then sum all counts. The sum will be 5 for 5 different cards, 7 for one pair, 9 for 2 pairs etc.

```c
int  get_type(const char *cards) {
 int i, j;
  int total = 0;
  int count[5] = { 1, 1, 1, 1, 1};
  for (i = 0; i< 5; i++) {
    for (j = 0; j <5; j++) {
      if (i !=j && cards[i] == cards[j]
	count[i] = count[i] + 1;
    }
  }

  for (i=0; i<5; i++) {
    total = total + count[i];
  }
  
  return total;
}
```

2. I do not implement hand comparison, but decided to implement a strict hand value algorithm. If we want it to match priorities, we have to basically represent the hand in a different base. There are 13 different card values, from 2 (value=0) to Ace (value=12), so we'll express the hand in base 13. The highest power is for hand type (one pair, two pairs etc). Then the first card, then the second, down to the last card on the right.

```c
double get_value(const char *cards) {
  int hand_type;
  int value;
  int i;

  hand_type = get_type(cards);
  value = 0;
  for (i=0; i<5; i++) {
    value = value + get_card_value(cards[i]) * pow(13, 4-i);
  }
  value += hand_type * pow(13, 5);
  return value;
}
```

3. The rank of hand can simply be computed by counting the number of other handsthat have a lower value that the current one.

```c
void rank_hands() {
  int i,j;
  for (i=0; i<nb; i++) {
    hands[i].rank = 1;
    for (j=0; j<nb; j++) {
      if (hands[j].value < hands[i].value)
	hands[i].rank = hands[i].rank + 1;
    }
  }
}
```

The function `get_card_value()` simply maps a card to its value: 2 has value 0, up to Ace with value 12.

Then, basically, the work is done:

1. Read each line (hand) and get the cards and the bid
2. Compute the value of the hand
3. Compute winnings: sum of all rank * bid for each hand.

## 7.2

The second task introduces the notion of Joker. Letter `J` is no longer for `Jack` but for `Joker` and can replace any other card. This can help achieve a hand of higher type, but to balance that the value of the Jack is set to 0.

Computing the type of a hand is more complex. I chose to compute the type of each possible hand by replacing the Joker with one of the other cards.

We have to take into account to special hands:

- The case where all cards are Jokers. This is the highest type.
- The case where there are no Jokers at all: we use the previous hand type algorithm.

```c
int get_type(const char* cards){
  int i, j, k, total;
  int max = 0;
  
  if (memcmp(cards, "JJJJJ", 5) == 0)
    return FIVEKIND;
  
  for (i=0; i<5; i++) {
    if (cards[i] == 'J') {
      for (j=0; j<5; j++) {
	if (i != j && cards[j] != 'J') {
	  /* replace jokers by a non-joker existing card */
	  char replace_by = cards[j];
	  char joker_hand[5];
	  memcpy(joker_hand, cards, 5);
	  for (k=0; k<5; k++) {
	    if (joker_hand[k] == 'J') {
		joker_hand[k] = replace_by;
	      }
	  }
	  total = get_raw_type(joker_hand);
	  if (total > max) {
	    max = total;
	  }
	}
      }
    }
  }
  if (max == 0) {
    /* we had no joker at all */
    max = get_raw_type(cards);
  }
  return max;
}
```

The implementation of the computation of hand value changes slightly because J now means 0, but overall the idea is the same.

#  December 8 - Graph

The day's task consists in walking in a graph.

## 8.1

We start at a node 'AAA' and need to know how many steps there are to reach 'ZZZ'.
The description of the task was not very clear to me, I somehow thought we had to make our own way in the graph, but that's not the case: we are meant to follow instructions written as the first line of the input file. Those instructions say when to turn left or right. If by the end of the instructions we haven't found the exit, then we loop on intrusctions.

Reading the file is easy:

1. The instructions are the first line.
2. Then, there is an empty line
3. The graph starts on the 3rd line and consists of the node and where Left or Right leads to. We can easily read each line with a regexp: `re.findall(r"[A-Z]{3}"`. Note that if you want to input the test file, you'll have to add digit characters to the regexp.
4. We populate the graph (a dict) with a tuple: `graph[tag] = (left, right)`

Then, we basically follow the instructions of the graph until we find the exit:

```python
def follow(instructions: str, graph):
    step = 0
    node = 'AAA'
    while node != 'ZZZ':
        if instructions[step % len(instructions)] == 'L':
            node = graph[node][0]
        elif instructions[step % len(instructions)] == 'R':
            node = graph[node][1]
        else:
            logger.error(f"Bad instructions: step={step}")
            quit()
        step = step + 1
```

## 8.2

In this second step, we start at nodes that finish with an 'A' and want to count steps for all nodes to have a trailing 'Z' (simultaneoulsy). I modified the Python program and coded that naively, but it takes "forever".

To solve the task, I used 2 hints:

1. I was told that C or any other language wouldn't solve the issue.
2. I was told the graph was *very special* and read this excellent [blog post](https://advent-of-code.xavd.id/writeups/2023/day/8/) which explains the solution.

I adapted the solution to my own code and got the result in 0.1 second :)

> Lessons learned: math.lcm uses a variable number of arguments. This is marked as `lcm(* integers)`. In my case, I had a list of exit step counts for all A nodes, and wanted to supply this to lcm. The solution is very simply `lcm(*tab)`.  This will actually take each element of the tab and supply it as an argument to lcm.



# Appendix

## Full code of day 5 task 2 {#d5t2_code}

```c
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

// -------------- TYPES ----------------
struct pair {
  unsigned long base;
  unsigned long range;
};

struct mapping {
  long int dst;
  long int src;
  long int len;
};

#define LEVEL LOG_INFO

typedef enum {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR } LOG_LEVEL;

// --------------- GLOBALS -------------
#define MAP_LEN 100

struct pair seed_list[30];
struct mapping seed_to_soil[MAP_LEN];
struct mapping soil_to_fertilizer[MAP_LEN];
struct mapping fertilizer_to_water[MAP_LEN];
struct mapping water_to_light[MAP_LEN];
struct mapping light_to_temperature[MAP_LEN];
struct mapping temperature_to_humidity[MAP_LEN];
struct mapping humidity_to_location[MAP_LEN];
struct mapping *map = NULL;
int *map_len = NULL;
int nb_seed = 0, nb2soil = 0, nb2fertilizer = 0, nb2water = 0, nb2light = 0, nb2temp = 0, nb2hum = 0, nb2loc = 0;

#define LOGMSG(lvl, fmt, ...) logger(lvl, __func__, fmt, __VA_ARGS__);

// ---------------- LOGS -----------------------
void logger(LOG_LEVEL lvl, const char *funcname, char *fmt, ...) {
  if (lvl >= LEVEL) {
    const char *level_str;
    switch (LEVEL) {
    case LOG_DEBUG:
      level_str = "DEBUG";
      break;
    case LOG_INFO:
      level_str = "INFO";
      break;
    case LOG_WARNING:
      level_str = "WARNING";
      break;
    case LOG_ERROR:
      level_str = "ERROR";
    default:
      level_str = "UNKNOWN";
    }
    va_list arglist;
    va_start( arglist, fmt );
    printf("[%s][%10s()]: ", level_str, funcname);
    vprintf( fmt, arglist );
    va_end( arglist );
  }
}

// ---------------- SEED PAIRS ---------------
void parse_seed_list(struct pair *list, int nb) {
  int i;
  for (i=0; i<nb;i++) {
    LOGMSG(LOG_DEBUG, "base=%20ld range=%20ld\n", list[i].base, list[i].range);
  }
}
 
int get_seed_list(char *line, struct pair *seed_list, int max) {
  char *token = NULL;
  int nb = 0;
  unsigned long base, range;

  assert(seed_list != NULL);

  /* initialize token split */
  token = strtok(&line[7], " ");
  assert(token != NULL);
  base = atol(token);

  while ((token = strtok(NULL, " ")) != NULL) {
    if (base == 0) {
      base = atol(token);
    } else {
      assert(nb <= max);
      seed_list[nb].range = atol(token);
      seed_list[nb].base = base;
      base = 0;
      nb++;
    }
  }
  
  return nb;
}

// -------------------- MAPS ------------------
long int get_dst(struct mapping *map, int max, long value) {
  int i;
  assert(value >= 0);
  for (i=0; i< max; i++) {
    if (value >= map[i].src && value < (map[i].src + map[i].len)) {
      long int dst = map[i].dst + value - map[i].src;
      if (dst < 0) {
	LOGMSG(LOG_ERROR, "dst=%ld src=%ld len=%ld value=%ld -> dst=%ld\n", map[i].dst, map[i].src, map[i].len, dst);
	assert(dst >= 0);
      }
      return dst;
    }
  }
  LOGMSG(LOG_DEBUG, "Value not found=%ld - mapping to same value\n", value);
  return value;
}

int read_mapping(struct mapping *array, int index, char *line) {
  if (isdigit(line[0]) == 0) {
    return -1;
  }
  char *token = strtok(line, " ");
  assert(token != NULL);
  array[index].dst = atol(token);
  token = strtok(NULL, " ");
  array[index].src = atol(token);
  token = strtok(NULL, " ");
  array[index].len = atol(token);
  LOGMSG(LOG_DEBUG, "dst=%15ld src=%15ld len=%15ld\n", array[index].dst, array[index].src, array[index].len);
  return 1;
}

int parse_map(char *line) {
  const char *tags[] = { "seed-to-soil",
    "soil-to-fertilizer",
    "fertilizer-to-water",
    "water-to-light",
    "light-to-temperature",
    "temperature-to-humidity",
    "humidity-to-location" };
  struct mapping *maps[] = { &seed_to_soil[0],
    &soil_to_fertilizer[0],
    &fertilizer_to_water[0],
    &water_to_light[0],
    &light_to_temperature[0],
    &temperature_to_humidity[0],
    &humidity_to_location[0] };
  int *lens[] = { &nb2soil,
    &nb2fertilizer,
    &nb2water,
    &nb2light,
    &nb2temp,
    &nb2hum,
    &nb2loc };
  int i;

  for (i = 0; i<7; i++) {
    if (memcmp(tags[i], line, strlen(tags[i])) == 0) {
      LOGMSG(LOG_DEBUG, "Map is %s\n", tags[i]);
      map = maps[i];
      map_len = lens[i];
      return 1;
    }
  }
  return 0;
}

void parse_file(char *filename) {
  FILE *fp = NULL;
  char *line = NULL;
  ssize_t read;
  size_t len;

  /* parse the file */
  fp = fopen(filename, "r");
  while ((read = getline(&line, &len, fp)) != -1) {
    /* remove \n */
    line[strcspn(line, "\n")] = 0;
    
    if (memcmp("seeds: ", line, 7) == 0) {
      /* process seed line */
      nb_seed = get_seed_list(line, seed_list, 30);
      parse_seed_list(seed_list, nb_seed);
    } else if (parse_map(line) == 0) {
      /* this is a line with dst src len */
      if (map != NULL) {
	assert(*map_len < MAP_LEN);
	read_mapping(map, *map_len, line);
	(*map_len)++;
      }
    }
  }
  fclose(fp);
  if (line) free(line);
}


void process(char *filename) {
  LOGMSG(LOG_INFO, "Parsing %s\n", filename);
  parse_file(filename);

  /* get location */
  int i = 0;
  long j;
  long min_location = -1;
  for (i=0; i<nb_seed; i++) {
    for (j=seed_list[i].base; j<seed_list[i].base+seed_list[i].range; j++) {
      long value = get_dst(seed_to_soil, nb2soil, j);
      value = get_dst(soil_to_fertilizer, nb2fertilizer, value);
      value = get_dst(fertilizer_to_water, nb2water, value);
      value = get_dst(water_to_light, nb2light, value);
      value = get_dst(light_to_temperature, nb2temp, value);
      value = get_dst(temperature_to_humidity, nb2hum, value);
      value = get_dst(humidity_to_location, nb2loc, value);
      LOGMSG(LOG_DEBUG, "seed=%15ld --> location=%15ld\n", j, value);
      if (min_location == -1 || value < min_location) {
	min_location = value;
	LOGMSG(LOG_DEBUG, "Updating min_location: %ld\n", min_location);
      }
    }
  }
  LOGMSG(LOG_INFO, "Minimum location=%ld\n", min_location);
}

int main(int argc, char **argv) {
  if (argc == 2) {
    process(argv[1]);
  } else {
    process("input5.txt");
  }
}
```


