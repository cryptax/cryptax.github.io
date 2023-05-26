# Zeh - Cyber Security Rumble CTF 2020


# Zeh 

*"For the CSR we finally created a deutsche Programmiersprache! `nc chal.cybersecurityrumble.de 65123`" and the following C program is provided:


```C
#define wenn if
#define ansonsten else
#define Zeichen char
#define Zeiger *
#define Referenz &
#define Ausgabe(s) puts(s)
#define FormatAusgabe printf
#define FormatEingabe scanf
#define Zufall rand()
#define istgleich =
#define gleichbedeutend ==

nichts Hauptroutine(nichts) {
    Ganzzahl i istgleich Zufall;
    Ganzzahl k istgleich 13;
    Ganzzahl e;
    Ganzzahl Zeiger p istgleich Referenz i;

    FormatAusgabe("%d\n", i);
    fflush(stdout);
    FormatEingabe("%d %d", Referenz k, Referenz e);

    schleife(7)
        k istgleich bitrverschieb(Zeiger p, k % 3);

    k istgleich diskreteAddition(k, e);

    wenn(k gleichbedeutend 53225)
        Ausgabe(Fahne);
    ansonsten
        Ausgabe("War wohl nichts!");
}
```

## Removing the defines

It helps to know a little of German (Hauptroutine means main), but online translation is far enough.

The program is mainly "obfuscated" by the `#define` at the beginning. We replace them all by the official C command.

```C
#include <stdio.h>
#include <stdlib.h>

void main(void) {
  int i = rand();
  int k = 13;
  int e;
  int *p = &i;
  
  printf("%d\n", i, k, *p);
  fflush(stdout);
  scanf("%d %d", &k, &e);
  
  for (int i=7; i--;) {
    k = (*p) >> (k % 3);
  }

  k = k ^ e;

  if(k == 53225)
    puts("Fahne");
  else
    puts("War wohl nichts!");
}
```


## Where is the flag?

The flag is not provided in the C program. It is included in `#include "fahne.h"` (which means flag), and displayed if this condition is met:

```C
  if(k == 53225)
    puts("Fahne");
  else
...
```

## Getting the right numbers

So the program:

1. Gets a random number (note, they just call `rand()` so it will not be truly random, the same each time.
2. Read 2 integers (k and e)
3. Do some obscure computation on k, *p, i, e.
4. Display the flag is k is 53225

We need to work out a solution where we will get k 53225.
IMHO, the best way to understand what is happening is add a few `printf` to display the various values of k, *p, i and e.


```
i=1804289383 k=          13 *p=  1804289383
1804289383 1804289383
i=6 k=   902144691 *p=  1804289383 e=1804289383
i=5 k=  1804289383 *p=  1804289383 e=1804289383
i=4 k=   902144691 *p=  1804289383 e=1804289383
i=3 k=  1804289383 *p=  1804289383 e=1804289383
i=2 k=   902144691 *p=  1804289383 e=1804289383
i=1 k=  1804289383 *p=  1804289383 e=1804289383
i=0 k=   902144691 *p=  1804289383 e=1804289383
k xor e --> k=1582229460
Not good!
```

We can try again, and we confirm *i always gets the same value*.
We also note that k apparently only takes 2 different values: 902144691 and 1804289383.

The last computation is `k ^ e` and we want that to be 53225.
We can provide 2 integers to the program: k and e.
Let's use `k=i=1804289383`.
So, now we need to adapt `e`.

The last value for k is `902144691` (see above).
So, we compute `902144691 ^ 53225 = 902131034`.
Let's use that for e.

We try that on the CTF's server:

```
nc chal.cybersecurityrumble.de 65123
1804289383
1804289383 902131034
CSR{RUECKWARTSINGENEUREN}
```

Note we get the same i. Then we enter `k=1804289383` and our computed `e=902131034`. It works, we get the flag :)

