---
tags: [ "linux", "fvwm", "alpine", "LVM", "pandoc", "docker" ]
---

# Who am I? :wave:

I am *Principal Security Researcher* at Fortinet, Fortiguard Labs. My current research interests are **mobile and IoT malware**. 

I am also organizing [Ph0wn CTF](https://ph0wn.org), a competition which focuses on ethical hacking of smart objects.

In a prior life, I used to implement *cryptographic algorithms and security protocols*.

# Preferred tools :hammer_and_pick:

- **Linux** Mint. I like *Debian* too.
- [**FVWM**](https://www.fvwm.org) Window Manager. It's old, but lightweight, fast and very customizable.
- [**Alpine**](https://en.wikipedia.org/wiki/Alpine_(email_client)) to read my emails. Same, it's old but lightweight and very easy to backup emails.
- **Emacs** is my favorite editor. I used *Visual Studio Code* for a couple of months... and came back to Emacs in the end.


Gosh, do you only like *old tools*? Hmm... often, but *not all the time*! :wink:

- **Docker** is a great invention!
- I love **Markdown**, and most of the time that's what I use to write documents, and then transform them using [Pandoc](https://pandoc.org). My research slides are written using direct *LaTeX / Beamer* though, especially because I need TikZ figures all the time.
- I used to like very much *ZFS* (on Solaris), but its support on Linux was problematic. Since then, I switched to **LVM** and like it very much.
- I never really used IRC and prefer **Discord**.
- I'm always on the look out for *new tools* which really help, as long as they are not too heavy. Feel free to share.


# Pico le Croco?

**Pico le Croco** is a fun character I invented and like to draw in various situations. 
You'll find him in several of my research slides. Pico has his own [website](https://pico.masdescrocodiles.fr).
You are free to use Pico's images under *CC-BY-SA 4.0*.


# This website

This website was built using [Hugo](https://gohugo.io) static website generator, and uses the [LoveIt](https://hugoloveit.com/) theme.

In `./layouts/index.html`, this line depends on your content sub-directory:

```
{{- $pages := where .Site.RegularPages "Type" "writeup" -}}
```
