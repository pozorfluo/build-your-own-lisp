# hm->capacity * 2 - HMAP_PROBE_LENGTH

probe = 32
c = 2 + probe
for n in range (1, 16):
	c = c * 2 - probe
	print(c)

c = 2 + probe
for n in range (1, 16):
	c =( c - probe ) * 2 + probe
	print(c)

# m.n15o="<#>wiq2a
#  > fill1K
# hmap->top : 1000
# |><| 0.000115 s
#  > m.n15o="<#>wiq2a
# hashes to : 498
# Looking for m.n15o="<#>wiq2a -> found @ 501
#                 -> value : 557
# Removing entry !