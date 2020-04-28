# **&#955;ispy**
```
 > following http://www.buildyourownlisp.com/ tutorial
```
[![asciicast](https://asciinema.org/a/xUzkxNCyVGNhiKQoIAZdmF4Ms.png)](https://asciinema.org/a/xUzkxNCyVGNhiKQoIAZdmF4Ms?speed=2&theme=tango&size=medium&autoplay=1)

# from Abstraction to Compression
Modern higher level languages made it very friendly to get my journey 
started. Yet they also made it painfully obvious that the layers of 
abstraction they provide can turn against you. 

Everything is a blackbox when you don't have the faintest idea of what goes
on under the hood. At a surface level it indeed is more readable, but when 
it goes wrong, it doesn't helps me understand the problem, let alone 
anticipate it.

With that comes a certain level anxiety about everything you do. So you try 
recipes at first and quickly it turns into performing rituals. 

C is still quite high level, it is not completely out of reach, but it 
forces me to think about the implications of what I'm doing.

By getting a better picture of what goes on behind the curtains I hope to 
turn these abstraction blackboxes into compressions whose meaning is 
restored in context. 

`Less angsty voodoo, more internalized strategies`.

## build
```bash
make
```

## test
```bash
sudo apt install valgrind
pip install --user --requirement requirements.txt
make test
```
### ← playground/src
all the things I need to `explore` on the side.  

---
## lispy.c  
currently at bonus marks of chapter 11 of http://www.buildyourownlisp.com/  

---
## hmap.c
> Based on the work of the Abseil Team and publications of Daniel Lemire,
> Peter Kankowski, Malte Skarupke, Maurice Herlihy, Nir Shavit, 
> Moran Tzafrir.

Implement a hash map for lispy / LispEnv  
- [Thierry la Fronde hashing blurb](https://pozorfluo.github.io/simplon-tp/tp1/html/hash-sling.html)

---
## `SNAIC`
![alt text][nvrstap]
- to do iz slow, to dunt is fastr
- nuthin last, best is fast  
- nevr fullow, best is first
- hi velucity = speed of fast + da wae
- wurst case scenario : long stories get short


[nvrstap]: https://img.shields.io/static/v1?label=NEVR%20STAP&message=OLWIZ%20FASTR&color=ff69b4&style=for-the-badge "SNAIC --halp"