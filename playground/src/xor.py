entry = 0x7FC66B580010
head = 0x5586267F2080
tail = 0x55862759E700

bin(head ^ tail)
hex(head ^ tail)
(head ^ tail) ^ head == tail
(head ^ tail) ^ tail == head


# force odd
for n in range(100):
    n |= 1
    print(n)

bin(11400714819323198486)
bin(11400714819323198485)
bin(11400714819323198485 ^ 11400714819323198486)
bin(11400714819323198486 ^ 10)
11400714819323198486 & 1
11400714819323198485

import math

int(2 ** 16 / ((1 + math.sqrt(5)) / 2))
int(2 ** 32 / ((1 + math.sqrt(5)) / 2))
int(2 ** 64 / ((1 + math.sqrt(5)) / 2))
int(2 ** 128 / ((1 + math.sqrt(5)) / 2))


# fib hash
for key in range(100):
    (key * 11400714819323198485) >> 54


fimur_hash = [
    8613972110907996638,
    15358817489599691552,
    3656918988786498657,
    10401764430870904227,
    17146609988017651429,
    5444711761147035686,
    12189556917607727464,
    487658412734448298,
    7232503855452193771,
    13977349419385324845,
    2275450786074068591,
    9020296620023908272,
    15765141844109363442,
    4063243553980254771,
    10808088789751040885,
    17552934303797120183,
    5851035703310487032,
    12595881237862518586,
    893982664156516476,
    14383673675207217919,
    9426620913003140482,
    16171466353514682052,
    4469567715245758469,
    11214413148748618055,
]

murmur3_x64 = [
    1584096894626881991315877671151210123721,
    54336250343350542782176903032023148998,
    855158039471493497310317094450549862077,
    1039127549861262650713404564393879311435,
    93136154732261704556933925320311062615,
    74312781558375054611426997378294798738,
    1807475194205470988210194190753005636218,
    1619970959750794755710948967737182030885,
    128772317988171237053664783826596130169,
    1550580235102978885114018615696532709555,
    1154421219551708742117692178646737305277,
    70824873152156146165017857262847459992,
    86718167958849630914322687166258693395,
    5145779186172661644318506030192103308,
    1112946414600426880511747594572229236485,
    82534992099906578798463907845345498967,
    413297339436388227811327427340427008806,
    1814992898938886923516075192548260744819,
    540449371664412957912350958338390321963,
    38622926961033010357625350269841354667,
    56363050618590338737050388589839385361,
    86807990082632946749662700404111625938,
    162281206466706151118429615108459332167,
    170811608674138578702126145761342232142,
]

# for h in fimurhash:
#     bin(h)

def hamming(a, b, bitdepth):
    distance = 0
    for i in range(bitdepth - 1, -1, -1):
        bit_a = (a >> i) & 1
        bit_b = (b >> i) & 1
        distance += not(bit_a == bit_b)
    return distance


for i, hash in enumerate(fimur_hash):
    distance = hamming(hash, fimur_hash[i-1], 64)
    print(f"distance  [{i}]<->[{i-1}] \t {distance}/64")

for i, hash in enumerate(murmur3_x64):
    distance = hamming(hash, murmur3_x64[i-1], 128)
    print(f"distance  [{i}]<->[{i-1}] \t {distance}/128")


# fimurhash
# distance  [0]<->[-1]     =28/64
# distance  [1]<->[0]      =35/64
# distance  [2]<->[1]      =31/64
# distance  [3]<->[2]      =26/64
# distance  [4]<->[3]      =33/64
# distance  [5]<->[4]      =37/64
# distance  [6]<->[5]      =24/64
# distance  [7]<->[6]      =34/64
# distance  [8]<->[7]      =27/64
# distance  [9]<->[8]      =28/64
# distance  [10]<->[9]     =32/64
# distance  [11]<->[10]    =32/64
# distance  [12]<->[11]    =33/64
# distance  [13]<->[12]    =23/64
# distance  [14]<->[13]    =28/64
# distance  [15]<->[14]    =29/64
# distance  [16]<->[15]    =27/64
# distance  [17]<->[16]    =34/64
# distance  [18]<->[17]    =32/64
# distance  [19]<->[18]    =29/64
# distance  [20]<->[19]    =29/64
# distance  [21]<->[20]    =31/64
# distance  [22]<->[21]    =32/64
# distance  [23]<->[22]    =24/64

# murmur3_x64
# distance  [0]<->[-1]     =67/128
# distance  [1]<->[0]      =55/128
# distance  [2]<->[1]      =66/128
# distance  [3]<->[2]      =62/128
# distance  [4]<->[3]      =59/128
# distance  [5]<->[4]      =63/128
# distance  [6]<->[5]      =66/128
# distance  [7]<->[6]      =62/128
# distance  [8]<->[7]      =65/128
# distance  [9]<->[8]      =64/128
# distance  [10]<->[9]     =70/128
# distance  [11]<->[10]    =65/128
# distance  [12]<->[11]    =66/128
# distance  [13]<->[12]    =67/128
# distance  [14]<->[13]    =59/128
# distance  [15]<->[14]    =55/128
# distance  [16]<->[15]    =67/128
# distance  [17]<->[16]    =64/128
# distance  [18]<->[17]    =65/128
# distance  [19]<->[18]    =61/128
# distance  [20]<->[19]    =66/128
# distance  [21]<->[20]    =58/128
# distance  [22]<->[21]    =63/128
# distance  [23]<->[22]    =65/128



def fimurhash(number):
    hash = (31 * 11400714819323198485 * (number << 15)) & 0xFFFFFFFFFFFFFFFF
    hash = ((hash << 7) | (hash >> (32 - 7)) ) & 0xFFFFFFFFFFFFFFFF
    return hash

def fimurhalt(number):
    hash = (31 * 102334155 * (number << 15)) & 0xFFFFFFFFFFFFFFFF
    hash = ((hash << 7) | (hash >> (32 - 7)) ) & 0xFFFFFFFFFFFFFFFF
    return hash

mean = 0
pool = 10000
for i in range(pool):
    hash_a = fimurhash(i)
    hash_b = fimurhash(i + 1)
    distance = hamming(hash_a, hash_b, 64)
    mean += distance/64
    print(f"fimurhash [{i}]<->[{i+1}] \t {distance/64}     \t {(hash_a & 0xFFFFFFFFFFFFFFFF)>> (64 - 14)} ")

print(f"mean = {mean/pool}")

for i in range(0, 544, 34):
    hash_a = fimurhash(i)
    hash_b = fimurhash(i + 1)
    distance = hamming(hash_a, hash_b, 64)
    print(f"fimurhash [{i}]<->[{i+1}] \t {distance}/64 = {distance/64}")

mean = 0
pool = 10000
for i in range(pool):
    hash_a = fimurhalt(i)
    hash_b = fimurhalt(i + 1)
    distance = hamming(hash_a, hash_b, 64)
    mean += distance/64
    print(f"fimurhalt [{i}]<->[{i+1}] \t {distance/64}     \t {(hash_a & 0xFFFFFFFFFFFFFFFF)>> (64 - 14)} ")

print(f"mean = {mean/pool}")

for i in range(0, 544, 34):
    hash_a = fimurhalt(i)
    hash_b = fimurhalt(i + 1)
    distance = hamming(hash_a, hash_b, 64)
    print(f"fimurhalt [{i}]<->[{i+1}] \t {distance}/64 = {distance/64}")



pool = 1000
for i in range(pool):
    print(f"{fimurhash(i) >> 54} \t: "
    f"{fimurhash(fimurhash(i >> 54) + i) >> 54} \t: "
    f"{fimurhash(fimurhash(i >> 54) ^ i) >> 54}")

# for i in range(pool):
#     print(f"{fimurhash(i)} \t: "
#     f"{fimurhash(fimurhash(i) + i)} \t: "
#     f"{fimurhash(fimurhash(i) ^ i)}")
    

def fimurhash_seed(number, seed):
    hash = (seed * 11400714819323198485 * (number << 15)) & 0xFFFFFFFFFFFFFFFF
    hash = ((hash << 7) | (hash >> (32 - 7)) ) & 0xFFFFFFFFFFFFFFFF
    return hash

# for i in range(0, 545, 34):
#     print(f"{i} ; "
#         f"{fimurhash_seed(i, 31) >> 54} ; "
#     f"{fimurhash_seed(i, 31 + fimurhash_seed(i, 31)) >> 54} ; "
#     f"{fimurhash_seed(i, 31 ^ fimurhash_seed(i, 31)) >> 54}")


# nested hash
n = 14
top_n = 0
seed = 31 << 7
collisions = 0
entry_count = (2**(n-1))

hmap = [[] for n in range(2**top_n)]
for i in range(1, entry_count):
    hash = fimurhash_seed(i, seed)
    top_level = hash >> (64 - top_n)
    # nested_seed =fimurhash_seed(i ^ hash, seed + hash) >> (64 - n + top_n)
    # nested_hash = fimurhash_seed(i, (seed * hash) & 0xFFFFFFFFFFFFFFFF )
    # nested_hash = fimurhash_seed(i, seed << 7)
    nested_hash = fimurhash_seed(i, seed)
    nested_level = nested_hash >> (64 - n + top_n)
    print(f"{i} : { hash } : { nested_hash } : {top_level} : {nested_level}")
    hmap[top_level].append(nested_level)


for i, top_level in enumerate(hmap):
    entries = len(top_level)
    unique = len(set(top_level))
    collisions += entries-unique
    print(f"toplevel[{i}] -> {unique}/{entries} : {entries-unique} collisions")

print(f"{collisions} / {entry_count} : {collisions/entry_count}")



# for i in range(100):
#     print(f"{i} : { fimurhash_seed(i, 31) >> 54}")
# def fimurhash(number):
#     hash = 31 * 11400714819323198486 * (number << 15)
#     hash = (hash << 7) | (hash >> (32 - 7))
#     return hash

# def fimurhash(number):
#     hash = 31 * 62831853071 * (number << 15)
#     hash = (hash << 7) | (hash >> (32 - 7))
#     return hash

# def fimurhash(number):
#     hash = 31 * 102334155 * (number << 15)
#     hash = (hash << 7) | (hash >> (32 - 7))
#     return hash

# def fimurhash(number):
#     hash = 31 * 102334155 * (number << 16)
#     hash = (hash << 8) | (hash >> (32 - 8))
#     return hash

# def fimurhash(number):
#     hash = 31 * 11400714819323198486 * (number << 16)
#     hash = (hash << 8) | (hash >> (32 - 8))
#     return hash

# def fimurhash64(number):
#     hash = 31 * 11400714819323198485 * (number << 31)
#     hash = (hash << 15) | (hash >> (64 - 19))
#     return hash

# def merxorhash64(number, n):
#     hash = 31 * ((2**n)-1 ) * (number << 31)
#     hash = (hash << 15) | (hash >> (64 - 19))
#     return hash

# import string
# for char in string.ascii_lowercase:


# mean = 0
# for i in range(pool):
#     hash_a = fimurhash64(i)
#     hash_b = fimurhash64(i + 1)
#     distance = hamming(hash_a, hash_b, 64)
#     mean += distance/64
#     print(f"fimurhash64 [{i}]<->[{i+1}] \t {distance}/64 = {distance/64}")

# print(f"mean = {mean/pool}")

# mean = 0
# for i in range(pool):
#     hash_a = merxorhash64(i, 61)
#     hash_b = merxorhash64(i + 1, 61)
#     distance = hamming(hash_a, hash_b, 64)
#     mean += distance/64
#     print(f"merxorhash64 [{i}]<->[{i+1}] \t {distance}/64 = {distance/64}")

# print(f"mean = {mean/pool}")

# mean = 0
# for i in range(pool):
#     hash_a = merxorhash64(i, 31)
#     hash_b = merxorhash64(i + 1, 31)
#     distance = hamming(hash_a, hash_b, 64)
#     mean += distance/64
#     print(f"merxorhash64 [{i}]<->[{i+1}] \t {distance}/64 = {distance/64}")

# print(f"mean = {mean/pool}")




