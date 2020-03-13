hex(127)
bin(0x7f)


((0b01111111 ^ 0b01011000) ^ 0b01011000 ) == 0b01111111
((0b01111111 ^ 0b01011000) ^ 0b01111111 ) == 0b01011000



# hit a bucket where h2 and neighbourhood are encoded in the meta_byte
# retrieve neighbourhood map with a xor h2 on the meta_byte

2**8
h2 = 0b00011100 # 0x1c
distance = 7
bin(h2)
bin(distance)
bin(distance^h2)

# problem is you do NOT know that the initial bucket you hit is @home
# your xored meta_byte is useless because your h2 is NOT necessarily a key
# to get the distance

