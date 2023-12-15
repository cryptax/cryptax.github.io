---
layout: post
title: "Advent of Code 2023: What I learned"
author: cryptax
date: 2023-12-15
tags:
- Advent
- Code
- 2023
- Python
---

*Advent of Code* is a coding competition, with one new challenge per day during December. I started writing the core of my own solutions, then I realized this wouldn't be very interesting because (1) my own code is often very naive compared to the best ones, (2) the article was starting to get very long.

Instead, I will focus only on things I *learned* or *experienced* doing the challenges.
I've been doing my implementations in Python and in C, but also read other solutions (mostly posted on Reddit) and have often been amazed by one-liners or very performant implementations.

# Python

## Logging

Logging is important, especially when you need some form of debugging

```python
import logging

log_format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s" 
logging.basicConfig(format = log_format, level = logging.INFO)
logger = logging.getLogger()
```

Along with logging, I appreciate the f-strings notations where the variable to output is directly shown in the format string.

- [f-strings doc](http://cissandbox.bentley.edu/sandbox/wp-content/uploads/2022-02-10-Documentation-on-f-strings-Updated.pdf)

## Type hints

Type hints are very probably over-kill for Advent of Code challenges, but coming from C, I like to use type hints as much as possible. 

```python
def get_calibration_value(s: str) -> int:
```

- Type hint for an array of ints: `list[int]`
- Type hint for an array of arrays: `list[list[int]]`

## Enumerates

I wasn't using very much enumerates in Python. [They are very useful to simplify loops that need counters](https://realpython.com/python-enumerate/).

For example, in the following code, we automatically "map" the words to an incremented index:

```python
for i, num in enumerate('one two three four five six seven eight nine'.split()):
	print(i, num)
```	

## Lists

When you pass a list to a function in Python, it's the object that you provide. So, if the function modifies the list, the list will *remain modified outside the function*.

Example:

```
>>> a = [1,2,3,4]
>>> def modify(tab):
...     tab[0] = 'modified'
... 
>>> modify(a)
>>> a
['modified', 2, 3, 4]
```

If you don't want that to happen, you can copy the list with `mylist.copy()`.

## List comprehensions 

Reminder: it is possible to use conditions in list comprehensions.
For example, the following get the positions of `O`s:

```
>>> column = '....O...O'
>>> [i for i in range(len(column)) if column[i] == 'O']
[4, 8]
```

- [More on List comprehensions on Real Python](https://realpython.com/list-comprehension-python/)


## Tuples

The difference with a list is that *tuples* are **immutable**. 
You cannot *assign* to a tuple.

## Sets


A set does not have redundancies. 

So, to test that all elements of a table are zeroes, you can actually transform the list in a set and test: `set(tab) == {0}`

- Reminder: a set is defined with `{}`. A *dict* is a named set: `{ 'green' : True, 'blue' : False }`.


## Extended Slices

I had never used [**extended slices**](https://docs.python.org/release/2.3.5/whatsnew/section-slices.html) with a *third* argument. The syntax is exactly: `mylist[first:last:step]` which will produce mylist[first], mylist[first+step] ... until mylist[last] *not included*.

Example:

- Extracting elements with even indexes: `mylist[::2]`
- Reversing order: `mylist[::-1]`

This works with *lists, tupples and strings*.

Ref: [Understanding slice notation](https://www.30secondsofcode.org/python/s/slice-notation/)


## Strings

- Important: keep in mind strings are **immutable** in Python. If you want to modify letters in a string, a common way is to transform them in a list, edit the list and join back.
- `a[5:]` **works** even if there are less than 6 characters in the string! It will return `''`

## Map

[Map applies a *function* to elements of an iterable object](https://realpython.com/python-map-function/). If we have a line of numbers, the following works:

```python
nums = [int(i) for i in line.split(',') ]
```

But it also works with map:

```python
nums = list(map(int, l.split(',')))
```

If you need to rule out some values, consider using `filter`: `filter(function, iterable)`.

```python
groups_tab = list(filter(None, groups.split('.'))) # remove empty groups
```

## Regex

I often need to match a given syntax and return only the part that matches. For example: `Game 123` and I want to retrieve `123`. 

The way to do that is with a `search` and then use `group` on the match.

```python
def get_id(line: str) -> int:
    match = re.search(r'Game ([0-9]+)', line)
    return int(match.group(1))
```

Remember that `search` only returns the first occurence. If you need them all, consider using `findall`.

- Find 3 letters: `[A-Z]{3}`. This will match `ABC`, or `HSM` but not `H8T`.

## Reading a file

It's not difficult to read a file line by line in Python. Pay attention that the following includes the trailing `\n` at the end of each line.

```python
f = open('filename','r')
lines = f.readlines()
f.close()
```

If you don't want those `\n`, you can remove them or simpler:

```python
lines = open('filename','r').read().splitlines()
```

## Pointers

Pointers are essential to any C developper. There are no pointers in Python, but there are mutable objects and we can more or less simulate the mechanism.

In the following, `seed_to_soil` and `soil_to_fertilizer` are **lists**.
I use `tags` as a pointer to the right list.

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

	...
    array = tags[k]
```


> Lessons learned: math.lcm uses a variable number of arguments. This is marked as `lcm(* integers)`. In my case, I had a list of exit step counts for all A nodes, and wanted to supply this to lcm. The solution is very simply `lcm(*tab)`.  This will actually take each element of the tab and supply it as an argument to lcm.

## Unpacking operators

Functions such as `math.lcm` use a variable number of arguments:

```python
def lcm(*integers):
	...
```

If we have a list of integers to provide, we can use the **unpacking operator** `*`: `math.lcm(*tab)`. Actually, `*` works over any *iterable* object (even for example on strings themselves).

If you want a variable number of **named** arguments, use the unpacking operator `**`. This create a dict that the function iterates over.

[More on Real Python](https://realpython.com/python-kwargs-and-args/#passing-multiple-arguments-to-a-function)

## Zip 

[Good article from RealPython](https://realpython.com/python-zip-function/)

`zip` takes one or multiple iterables as input. It returns a sequence of tuples with one element of each iterable.

This is quite useful in Day 14 to convert a matrix organized by lines, into a matrix of columns.

```
>>> matrix = [ 'ABCDE', 'FGHIJ', 'KLMNO' ]
>>> list(zip(*matrix))
[('A', 'F', 'K'), ('B', 'G', 'L'), ('C', 'H', 'M'), ('D', 'I', 'N'), ('E', 'J', 'O')]
>>> list(map(''.join, list(zip(*matrix))))
['AFK', 'BGL', 'CHM', 'DIN', 'EJO']
```

Note that `zip` works without issue on iterables of different lengths. It will operate till the smallest iterable.

As mentioned in Real Python, `zip` is also particularly interesting when you want to loop over several iterables:

```
>>> letters = ['a', 'b', 'c']
>>> numbers = [0, 1, 2]
>>> for l, n in zip(letters, numbers):
...     print(f'Letter: {l}')
...     print(f'Number: {n}')
```

[Other good example in Real Python](https://realpython.com/python-zip-function/), using `zip` to unzip with the unpacking operator:

```
>>> pairs = [(1, 'a'), (2, 'b'), (3, 'c'), (4, 'd')]
>>> numbers, letters = zip(*pairs)
>>> numbers
(1, 2, 3, 4)
>>> letters
('a', 'b', 'c', 'd')
```

Note that it's possible to zip iterables of different lengths.

## Underscore

There are more [uses of underscore](https://www.datacamp.com/tutorial/role-underscore-python) that I had ever imagined.

Basically, in `[dict() for _ in range(256)]` the underscore acts as an "unnamed" variable. 

It can also used to ignore values:

```
a, _, b = (1,2,3)
a, *_,b = (1,2,3,4)
```


## Misc

- Read a string right to left: `reversed(s)`
- Is a digit: `isdigit()`
- Sum all elements of a list: `sum(thelist)`
- [Lambda functions](https://realpython.com/python-lambda/)
- To compute combinations, use `from itertools import combinations`
- Python 3.8 introduced `:=` which [*"assigns values as part of a larger expression"*](https://docs.python.org/3/whatsnew/3.8.html). In the following, `n` gets the value of `len(a)`.

```
if (n := len(a)) > 10:
    print(f"List is too long ({n} elements, expected <= 10)")
```




# Elegant solutions (SPOILER ALERT)

*Those are solutions I found on Internet. Not mine*


| Day         | Link  |
| ----------- | ----- |
| Day 1 Task 1 | [Awk often produces very short solutions](https://chaos.social/@root42/111505305360222320) |
| Day 3  | Bash one-liner: `cat input4.txt \| cut -d: -f2 \| tr -d '\|' \| xargs -I {} sh -c "echo {} \| tr ' ' '\n' \| sort \| uniq -d \| wc -l " \| awk '$1>0 {print 2^($1-1)}' \| paste -sd+ - \| bc`. The command `paste` merges lines of files |
| Day 4 Task 2 | `cat /tmp/input4.txt | cut -d: -f2 | tr -d '|' | xargs -I {} sh -c "echo {} | tr ' ' '\n' | sort | uniq -d | wc -l " | cat -n | xargs | awk '{ for (i = 1; i < NF; i = i + 2) { copies[$i] = $(i + 1); vals[$i] = 0 } } { for (i = 1; i <= NF/2; i++) { counter=0; for (j = i + 1; counter < copies[i] && copies[i] > 0; j++) { counter++; vals[j] = vals[j] + 1 + vals[i] } } } { for(i=1;i<=NF/2;i++) { print vals[i] } print NF/2 }' | paste -sd+ - | bc` |
| Day 5 Task 2 | [Super fast Perl implementation](https://topaz.github.io/paste/#XQAAAQCZBgAAAAAAAAA6nMjJFMpQu0F8vIUYE8mlPji8DyQkFpThIAFfXA1uKGMODfR8qSzU2hkIZRiADH0kfb2NH1c1SEcTc67ExT0Awc+mO45muq8KALPHQSSdwr/rgLF92VWY1pIMDxaXOl7f67pWJiSI05R5nN7Zz2urm2GQFNOFpWmSr5MYB0+ybOEtN+bt3Q6Ss2gI8+g1JCdXwZgoXUYOrJL7lcI0AVNk84SddSYeshj4gEIkOl4I2qfW78wJz1cVSiDoWmAuGtPTmbEIMOeOLhkmV6toZPRuTyWTo68eVPkHH2nQGnbnfV17q9XWAva+IDeVkeKhRYEQEx7BtGsOafxKfgqGCoPC5pfU+nFs9zXL6ZlH8rYSceOIljUjY309Ie+tIPC9ObtP6tU5XYeLMI4Ml2jHlmdJap5U4MWd6x1DC2EUN/lY+16+gqopbaQIzobbIueyzQ5EdgYJu+Sg3/Fy+2P1rFNDvMzoms7SIVVue44A2oXlg1iwz/6OP/FrmyzfjdLRiecA27L42dKdJX9qsP3F2iwwFwcWI+/Hdq+fJ3VkPNhXt7i6s0Vu1JqzuiznolUE7w1ufoeVLsymWUnuAnQGVZqdnMfCTlKlG2nSwVkgrEEnZXd0DpLayzZEjt93v4LXDIrAGRYmn4JOs1wyz6oyx1l8Sk27uLdH1lM0swq5+KmM8k5n0rc9gA4O9Nh2dw1YKmgTAEGCb3etFSWXs3EvWbDR6FXBLM3h/2Fg8wA=)
| Day 8 | [Blog post explaining the solution](https://advent-of-code.xavd.id/writeups/2023/day/8/) |
| Day 14 | [Video explaining the solution](https://www.youtube.com/watch?v=WCVOBKUNc38) |
| Day 15 | [This solution](https://topaz.github.io/paste/#XQAAAQDfAQAAAAAAAAAzHIoib6poHLpewxtGE3pTrRdzrponK6uH8J9SK+z3MPZ140TRLBu/IWrE4WwdeK2eATuamUeQIKu6RSuS/hh1AgNBsKNPQgVKQhKQjyrhIhpbyO5mmMFXxRukuGJYELN09QZHZyt3cyt1BS/5MBszPnK6vNMyrfI2FAT647NxZevHNnv3kQHxPtTtslxTfRfFStCDCljxdQQyPiOitKrzF2TE749eSXSBC2DXgDSbkgv0hPB7qo8YuEZ+g3V5WiG+iYMaJpqsK53FlhpIgQWCvlLuQFjyc/38RDk4oaKYZkzBhdHCpTs62DOyD1llR/BpMDdX+x9Ot36AAGJYaRiI82tWVi2234+ZvddY1S/z0h1pNOd7by30qQ7+u/tLgjtwoN+Yv2cejvsBBmXuPIFX54aI//ZMg7w=) uses the [match statement](https://docs.python.org/3/tutorial/controlflow.html#tut-match), which is like C's switch |

# C

I solved a few challenges using plain C (day 5, day 6, day 7 for instance). If you're not going to use some Python specificities, C does the work very well... and fast.


# Performance

Python is way slower than C. That's a fact. For example, this is the performance for similar implementations of Day 5 Task 2. The same with Python simply would not finish in a reasonable time.


| Implementation | Time to solution |
| -------------- | ---------------- |
| `gcc d5t2.c -o d5t2` | 7 min 43 |
| **Optimized**: `gcc -O3 ...` | 2 min 20 |
| Java | 3 min 30 |

However... it depends very much on the algorithm you choose and your implementation skills...

In Python, if a given function is called many times with the same arguments, it's a good idea to memoize it to speed up the result. This can be done with `from functools import cache` and the `@cache` decorator.

For example, [this Day 12 solution](https://www.reddit.com/r/adventofcode/comments/18ge41g/comment/kd221yp) caches the `recurse` function. Beware, caching works with simple types, it won't work with lists.

```python
from functools import cache

@cache
def recurse(lava, springs, result=0):
...
```

Also, if you're using recursivity intensively, you might need the optimization of `sys.setrecursionlimit(number)` - but that might not solve all your performance issues ;)
