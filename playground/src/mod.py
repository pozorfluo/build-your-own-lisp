# n % m == n & (m - 1)

# ...where m is a power of 2.

# For example:

# 22 % 8 == 22 - 16 == 6

#          Dec   Bin
#        -----   -----
#           22 = 10110
#            8 = 01000
#        8 - 1 = 00111
# 22 & (8 - 1) =   10110
#                & 00111
#                -------
#            6 =   00110


n = 13
div = 1 << n

hex(0x0ECFDC8D586EDA764E8330B24E5397A5 % div)


hex((((0x0ECFDC8D586EDA76 % div) << 64) + 0x4E8330B24E5397A5) % div)

num = 0x0ECFDC8D586EDA764E8330B24E5397A5
hex(num % div)
hex(num & (div - 1))

num0 = (num & 0xFFFFFFFF000000000000000000000000) >> 96
num1 = (num & 0x00000000FFFFFFFF0000000000000000) >> 64
num2 = (num & 0x0000000000000000FFFFFFFF00000000) >> 32
num3 = num & 0x000000000000000000000000FFFFFFFF


hex(num0 & (div - 1))
hex(num1 & (div - 1))
hex(num2 & (div - 1))
hex(num3 & (div - 1))

hex(
    (
        (
            (((((num0 & (div - 1)) << 32) + num1) & (div - 1) << 32) + num2)
            & (div - 1) << 32
        )
        + num3
    )
    & (div - 1)
)

num0 = (num & 0xFFFFFFFF000000000000000000000000) >> 96
num1 = (num & 0x00000000FFFFFFFF0000000000000000) >> 64
num2 = (num & 0x0000000000000000FFFFFFFF00000000) >> 32
num3 = num & 0x000000000000000000000000FFFFFFFF

(((((num0 % div) + num1) % div ) + num2) + num3) % div

((num0 % div) << 32 + num1)
((((((((((num0 % div) << 32) + num1) % div) << 32) + num2) % div) << 32 ) + num3) % div)

hex(0xbb236c61 % 8191)
hex(0x8c1fdef2 % 8191)
hex(0xde9ee3d1 % 8191)
hex(0x72f01557 % 8191)
hex(0xbb236c618c1fdef2de9ee3d172f01557 % 8191)
hex(0x0cd7b043c708958f068b783a56ec805b % 8191)