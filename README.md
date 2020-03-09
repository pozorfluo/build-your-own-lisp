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
> Based on the work and publications of the Abseil Team, Daniel Lemire,
> Peter Kankowski, Malte Skarupke.

Implement a hash map for lispy / LispEnv

#### Use
- open adressing
- linear probing, on chunks of metadata, once per find
- robin hood style entry shifting
- power of two advertised capacity
- overgrown actual capacity to account for probe chunk size
- structure of array layout for the buckets
- tabulation style hash function that yields necessary hash depth without 
  modulo

#### Do
- NOT store data inside the table itself, only pointers to data
- NOT store hash inside the table : the hash IS the index + metadata
- embed the pseudo-random hash function seed in the table
- take arbitrary size, c string style, '\0' terminated byte sequence as key
- take any pointer as value

#### Require
- user to allocate for the keys and values to insert in the table

#### Provide
- default allocator for keys, values delete operations
- ability for user to define custom allocator for keys, values emplace and 
  delete operations
- access to stats and verbose mode with -DDEBUG_HMAP compiler flag

### implementation WIP
- Split hash(key) into hash_index and hash_meta
  + hash_meta  is hash 7 least significant bits
  + hash_index is hash >> 7  

  | bit 7 | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0 |
  |-------|-------|-------|-------|-------|-------|-------|-------|
  |   0   |  hash |  hash |  hash |  hash |  hash |  hash |  hash |
  |   1   | state | state | state | state | state | state | state |

      MSB is 0 -> entry is OCCUPIED,     use 7 bits remaining as hash  
      MSB is 1 -> entry is NOT OCCUPIED, use 7 bits remaining as state

      EMPTY   = 0b10000000   -> -128
      DELETED = 0b11111110   -> -1
      MARKED  = 0b11111111   -> -2

      Think of OCCUPIED as anything like 0b0xxxxxxx
      aka 0 <= OCCUPIED < 128

```
         hash_index(hash) ─┐                    capacity + PROBE_LIMIT ─┐    
┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─ ─ ─ ─ ┬─┬─┬─┬─┬─┬─┐
│ │ │█│ │ │ │ │█│ │x│ │ │ │█│x│█│█│ │x│█│ │█│ │ │█│ │        │ │x│█│ │ │ │
└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─ ─ ─ ─ ┴─┴─┴─┴─┴─┴─┘
                           └─── ─ ─ ─ ───┘ probe length
┌─┐                       ┌─┐                    ┌─┐
│█│ occupied entry        │ │ empty entry        │x│ marked entry
└─┘                       └─┘                    └─┘
```



## `[todo]`
### Observe that :
  - any entry inside a probe can be 
      * empty
      * marked / deleted
      * occupied @home
      * occupied displaced

  Contrary to robin hood hashing distance from home is NOT tracked in 
  current implementation
 
Can one assert 

## `SNAIC`
![alt text][nvrstap]
- to do iz slow, to dunt is fastr
- nuthin last, best is fast  
- nevr fullow, best is first
- hi velucity = speed of fast + da wae
- wurst case scenario : long stories get short


[nvrstap]: https://img.shields.io/static/v1?label=NEVR%20STAP&message=OLWIZ%20FASTR&color=ff69b4&style=for-the-badge "SNAIC --halp"