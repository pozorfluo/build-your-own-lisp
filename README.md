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

#### Use
- open adressing
- linear probing, on chunks of metadata, once per find operation
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
- resize if probing fails to find an eligible spot where to insert
- alloc backing arrays by chunks on creation and resize only
- NOT use a linked list of entries to iterate through the table
- iterate through backing arrays, skip empty by probing metadata chunks
- touch keys, values as late as possible, work with metadata

#### Require
- user to allocate for the keys and values and pass pointer to insert 
  in the table

#### Provide
- default allocator for keys, values delete operations
- ability for user to define custom allocator for keys, values emplace and 
  delete operations
- access to stats and verbose mode with -DDEBUG_HMAP compiler flag

### implementation WIP

#### vocabulary

bucket : a slot in the table backing array. It can contain an entry. Its 
         state is tracked in a metadata byte 'shared' with the entry it may 
         hold.

entry : a key, value pointer pair and 7 bits of secondary hash stored in a
        metadata byte.

#### metadata
Use 1 byte of metadata for each bucket
  - Use 1 bit (MSB) to switch the way the remaining bits are read
  - Use 7 remaining bits to encode state or secondary hash

  - Split hash(key)
    + Take hash 7 least significant bits  
    + Store it in the metadata byte
    + Shift hash >> 7 to index in table backing arrays
      * Think of this value as the @home of entry


```
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

Use another 1 byte of metadata for each bucket
  - Keep track of distance from @home
```


#### buckets
```
         hash_index(hash) ─┐                   capacity + probe length ─┐    
┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─ ─ ─ ─ ┬─┬─┬─┬─┬─┬─┐
│ │ │█│ │ │ │ │█│ │x│ │ │ │█│x│█│█│ │x│█│ │█│ │ │█│ │        │ │x│█│ │ │ │
└─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─ ─ ─ ─ ┴─┴─┴─┴─┴─┴─┘
                           └─── ─ ─ ─ ───┘ probe length
┌─┐                       ┌─┐                    ┌─┐
│█│ occupied bucket       │ │ empty bucket       │x│ marked bucket
└─┘                       └─┘                    └─┘
```

## `[todo]`
### Observe that :
  - any entry inside a probe can be 
      * empty
      * marked / deleted
      * occupied @home
      * occupied displaced

  - @home means the entry is in the bucket it exactly hashes to
  - displaced means the entry is some distance away from @home
  - probing can happen from any bucket
  - think of probing as a sliding window of probe length
  - each bucket can appear in up to (probe length) different probe origins
  - entries unrelated to the current bucket of concern will be found inside
    probe length
  - unrelated entries have different @home


### Possible trade-offs
|          | space                           | time                        |
|----------|---------------------------------|-----------------------------|
| swapping | keep track of distance to @home | recompute hash              |



### Questions
  - How likely are false positive when probing ?
    + Is it low enough that matches can be thought of related to 
      @home bucket ?
  - How bad is it to probe beyond metadata allocated space ?
    + Is it ok if buckets is a single arena of memory, allocated in one
      piece, knowing that what immediately follows is part of the struct ?
    + Provided it is 'ok' and does not break things, how usable would a 
      probe be over partially garbage data ?
    + At what point does it become easier to just check boundaries or wrap
      to the beginning of the table ?

## `SNAIC`
![alt text][nvrstap]
- to do iz slow, to dunt is fastr
- nuthin last, best is fast  
- nevr fullow, best is first
- hi velucity = speed of fast + da wae
- wurst case scenario : long stories get short


[nvrstap]: https://img.shields.io/static/v1?label=NEVR%20STAP&message=OLWIZ%20FASTR&color=ff69b4&style=for-the-badge "SNAIC --halp"