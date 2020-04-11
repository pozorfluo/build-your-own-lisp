# check gcc define's
# see https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
# see https://askubuntu.com/questions/420981/how-do-i-save-terminal-output-to-a-file
gcc -dM -E - </dev/null > ./.tmp/gcc_defines.txt

gcc -dumpmachine

gcc -v

# see https://github.com/magnumripper/JohnTheRipper/wiki/Show-all-predefined-macros-for-your-compiler
#  alias gccmacros='gcc -dM -E -x c /dev/null'
gcc -dM -E -x c /dev/null | rg "MMX|SSE|AVX|XOP"
gcc -dM -E -x c -march=native /dev/null | rg "MMX|SSE|AVX|XOP"
# gcc -dM -E -x c -march=knl /dev/null | rg "MMX|SSE|AVX|XOP"



# __SSE2__
_mm_set1_epi8
_mm_cmpeq_epi8
_mm_cmpgt_epi8
_mm_movemask_epi8
_mm_cmpeq_epi8
_mm_store_si128

# Load 128-bits of integer data from memory into dst. 
#mem_addr does not need to be aligned on any particular boundary.
_mm_loadu_si128


# __SSE3__
# Load 128-bits of integer data from unaligned memory into dst. 
# This intrinsic may perform better than _mm_loadu_si128 when the
# data crosses a cache line boundary.
_mm_lddqu_si128

# __AVX__ 
_mm256_set_epi8
_mm256_cmpeq_epi8
_mm256_cmpgt_epi8
_mm256_movemask_epi8
_mm256_cmpeq_epi8
_mm256_store_si256
_mm256_loadu_si256
_mm256_lddqu_si256

# Other
# with si128
_bit_scan_forward
# with si256
_bit_scan_forward64


#ifdef __AVX__

#else 

#endif /* __AVX__ */