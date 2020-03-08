import random
import string

# bin(random.getrandbits(64))

# tab hashing
n = 11
seed = 31
key = 'hellp'


def fimurhash(key, seed, n):
    hash = seed
    for c in key:
        c_int = ord(c)
        hash *= (11400714819323198485 * (c_int << 15)) & 0xFFFFFFFFFFFFFFFF
        hash = ((hash << 7) | (hash >> (32 - 7)) ) & 0xFFFFFFFFFFFFFFFF
    return (hash >> (64 - n))

    
hmap = [random.getrandbits(n + 7) for r in range(256)]

def tab_hash(key):
    hash = 0
    i = 0
    for c in key:
        c_int = ord(c)
        hash_partial = hmap[c_int] ^ (c_int << i)
        hash ^= hash_partial
        i += 1
        # print(f"{c} : {hash_partial} : {hash}")
        # i+= hmap[i & 0xFF]
    return hash

def tab_gash(key):
    hash = 0
    # i = 0
    for c in key:
        c_int = ord(c)
        hash_partial = hmap[(c_int + hash) & 0xFF] ^ c_int
        hash ^= hash_partial
        # print(f"{c} : {hash_partial} : {hash}")
        # i+= hmap[i & 0xFF]
    return hash

def bad_hash(key):
    hash = 0
    for c in key:
        c_int = ord(c)
        hash_partial = hmap[c_int] ^ c_int
        hash ^= hash_partial
        # print(f"{c} : {hash_partial} : {hash}")
    return hash

# generate random keys
# def random_key(size=6, charset=string.digits):
# def random_key(size=6, charset=string.ascii_letters + string.digits):
def random_key(size=6, charset="abc"):
    return ''.join(random.choices(charset, k=size))

# test drive tab hashing
for i in range(2**(n-1)):
    # key = random_key(random.randint(1, 10))
    key = random_key(8)
    t_hash = tab_gash(key)
    fimur_hash = fimurhash(key, seed, n)
    # mixed_hash = tab_hash(str(fimur_hash))
    # print(f"{key} ; {t_hash} ; {fimur_hash} ; {mixed_hash}")
    print(f"{key} ; {t_hash} ; {fimur_hash}")
