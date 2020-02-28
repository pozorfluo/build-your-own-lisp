#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_murmur3_nose.h"

#define BENCHMARK
#include <fcntl.h>
#include <unistd.h>

/*
 * ------------------------------------------------------------- DEBUG MACRO ---
 */
#ifdef BENCHMARK
#include <time.h>

#define LOOP_SIZE 10000000

#define START_BENCH(start) start = (float)clock() / CLOCKS_PER_SEC

#define STOP_BENCH(start, stop, diff, result)                                  \
	stop = (float)clock() / CLOCKS_PER_SEC;                                    \
	diff = stop - start;                                                       \
	printf("%.8f\n", diff);                                                    \
	result = diff;

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif
/**
 * Helps me follow malloc / free where needed
 */
#ifdef DEBUG_MALLOC

#include <malloc.h>

void *xmalloc(size_t size, const char *origin, const char *destination)
{
	void *return_pointer = malloc(size);
	if ((return_pointer == NULL) && !size) {
		return_pointer = malloc(1);
	}
	if ((return_pointer == NULL)) {
		printf(FG_RED REVERSE
		       "malloc failed, Out of memory I guess\n"
		       "\tcalled inside      : %s()\n"
		       "\ttried to allocate  : %lu bytes\n"
		       "\tfor                : %s\n" RESET,
		       origin,
		       size,
		       destination);
		exit(EXIT_FAILURE);
	}

	printf(FG_CYAN "\tmallocing %s %lu bytes inside %s()\n" RESET,
	       destination,
	       size,
	       origin);
	memset(return_pointer, 0xFB, size);
	return return_pointer;
}

void xfree(void *pointer, const char *pointer_name, const char *origin)
{
	size_t bytes_in_malloced_block = malloc_usable_size(pointer);
	printf(FG_BLUE "\tfreeing %s %lu bytes inside %s()\n" RESET,
	       pointer_name,
	       bytes_in_malloced_block,
	       origin);
	free(pointer);
}

#define XMALLOC(_size, _origin, _destination)                                  \
	xmalloc(_size, _origin, _destination)

#define XFREE(_pointer, _origin) xfree(_pointer, #_pointer, _origin)

#else
#define XMALLOC(_size, _origin, _destination) malloc(_size)
#define XFREE(_pointer, _origin) free(_pointer)
#endif

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(array) (sizeof(array) / sizeof((array)[0]))

//------------------------------------------------------------- DECLARATIONS ---
typedef struct KeyValuePair {
	// todo
	// - [ ] Study KeyValuePair data alignment
	size_t hash;
	char *key;
	void *value;
} KeyValuePair;

//----------------------------------------------------------- FILE SCOPE LUT ---
// first 200 prime numbers
static const uint_fast16_t prime_numbers[200] = {
    2,    3,    5,    7,    11,   13,   17,   19,   23,   29,   31,   37,
    41,   43,   47,   53,   59,   61,   67,   71,   73,   79,   83,   89,
    97,   101,  103,  107,  109,  113,  127,  131,  137,  139,  149,  151,
    157,  163,  167,  173,  179,  181,  191,  193,  197,  199,  211,  223,
    227,  229,  233,  239,  241,  251,  257,  263,  269,  271,  277,  281,
    283,  293,  307,  311,  313,  317,  331,  337,  347,  349,  353,  359,
    367,  373,  379,  383,  389,  397,  401,  409,  419,  421,  431,  433,
    439,  443,  449,  457,  461,  463,  467,  479,  487,  491,  499,  503,
    509,  521,  523,  541,  547,  557,  563,  569,  571,  577,  587,  593,
    599,  601,  607,  613,  617,  619,  631,  641,  643,  647,  653,  659,
    661,  673,  677,  683,  691,  701,  709,  719,  727,  733,  739,  743,
    751,  757,  761,  769,  773,  787,  797,  809,  811,  821,  823,  827,
    829,  839,  853,  857,  859,  863,  877,  881,  883,  887,  907,  911,
    919,  929,  937,  941,  947,  953,  967,  971,  977,  983,  991,  997,
    1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
    1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
};

// // first 8
// static const uint_fast32_t mersenne_primes[] = {
//     3,
//     7,
//     31,
//     127,
//     8191,
//     131071,
//     524287,
//     2147483647,
// };

// // first 34
// static const uint_fast64_t mersenne_numbers[] = {
//     0,          1,          3,          7,          15,        31,
//     63,         127,        255,        511,        1023,      2047,
//     4095,       8191,       16383,      32767,      65535,     131071,
//     262143,     524287,     1048575,    2097151,    4194303,   8388607,
//     16777215,   33554431,   67108863,   134217727,  268435455, 536870911,
//     1073741823, 2147483647, 4294967295, 8589934591,
// };
// as seen in cpp stl
static const size_t prime_sizes[62] = {
    /* 0  */ 5ul,
    /* 1  */ 11ul,
    /* 2  */ 23ul,
    /* 3  */ 47ul,
    /* 4  */ 97ul,
    /* 5  */ 199ul,
    /* 6  */ 409ul,
    /* 7  */ 823ul,
    /* 8  */ 1741ul,
    /* 9  */ 3469ul,
    /* 10 */ 6949ul,
    /* 11 */ 14033ul,
    /* 12 */ 28411ul,
    /* 13 */ 57557ul,
    /* 14 */ 116731ul,
    /* 15 */ 236897ul,
    /* 16 */ 480881ul,
    /* 17 */ 976369ul,
    /* 18 */ 1982627ul,
    /* 19 */ 4026031ul,
    /* 20 */ 8175383ul,
    /* 21 */ 16601593ul,
    /* 22 */ 33712729ul,
    /* 23 */ 68460391ul,
    /* 24 */ 139022417ul,
    /* 25 */ 282312799ul,
    /* 26 */ 573292817ul,
    /* 27 */ 1164186217ul,
    /* 28 */ 2364114217ul,
    /* 29 */ 4294967291ul,
    /* 30 */ 8589934583ull,
    /* 31 */ 17179869143ull,
    /* 32 */ 34359738337ull,
    /* 33 */ 68719476731ull,
    /* 34 */ 137438953447ull,
    /* 35 */ 274877906899ull,
    /* 36 */ 549755813881ull,
    /* 37 */ 1099511627689ull,
    /* 38 */ 2199023255531ull,
    /* 39 */ 4398046511093ull,
    /* 40 */ 8796093022151ull,
    /* 41 */ 17592186044399ull,
    /* 42 */ 35184372088777ull,
    /* 43 */ 70368744177643ull,
    /* 44 */ 140737488355213ull,
    /* 45 */ 281474976710597ull,
    /* 46 */ 562949953421231ull,
    /* 47 */ 1125899906842597ull,
    /* 48 */ 2251799813685119ull,
    /* 49 */ 4503599627370449ull,
    /* 50 */ 9007199254740881ull,
    /* 51 */ 18014398509481951ull,
    /* 52 */ 36028797018963913ull,
    /* 53 */ 72057594037927931ull,
    /* 54 */ 144115188075855859ull,
    /* 55 */ 288230376151711717ull,
    /* 56 */ 576460752303423433ull,
    /* 57 */ 1152921504606846883ull,
    /* 58 */ 2305843009213693951ull,
    /* 59 */ 4611686018427387847ull,
    /* 60 */ 9223372036854775783ull,
    /* 61 */ 18446744073709551557ull,
};
//----------------------------------------------------------------- Function ---
/**
 * Returns the number of digits for given size_t number of up to 20 digits
 */
size_t count_digits(const size_t n)
{
	if (n < 10000000000) {
		// 10 or less
		if (n < 100000) {
			// 5 or less
			if (n < 100) {
				// 1 or 2
				if (n < 10)
					return 1;
				else
					return 2;
			}
			else {
				// 3 or 4 or 5
				if (n < 1000)
					return 3;
				else {
					// 4 or 5
					if (n < 10000)
						return 4;
					else
						return 5;
				}
			}
		}
		else {
			// 6 or more
			if (n < 10000000) {
				// 6 or 7
				if (n < 1000000)
					return 6;
				else
					return 7;
			}
			else {
				// 8 to 10
				if (n < 100000000)
					return 8;
				else {
					// 9 or 10
					if (n < 1000000000)
						return 9;
					else
						return 10;
				}
			}
		}
	}
	else {
		// 11 or more
		if (n < 1000000000000000) {
			// 5 or less
			if (n < 1000000000000) {
				// 1 or 2
				if (n < 100000000000)
					return 11;
				else
					return 12;
			}
			else {
				// 3 or 4 or 5
				if (n < 10000000000000)
					return 13;
				else {
					// 4 or 5
					if (n < 100000000000000)
						return 14;
					else
						return 15;
				}
			}
		}
		else {
			// 6 or more
			if (n < 100000000000000000) {
				// 6 or 7
				if (n < 10000000000000000)
					return 16;
				else
					return 17;
			}
			else {
				// 8 to 10
				if (n < 1000000000000000000)
					return 18;
				else {
					// 9 or 10
					if (n < 10000000000000000000ull)
						return 19;
					else
						return 20;
				}
			}
		}
	}
}
//----------------------------------------------------------------- Function ---
/**
 * Return nearest next power of 2 for given integer
 */

//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n
 *   -> Remainder
 */
//----------------------------------------------------------------- Function ---
/**
 * Compute modulus division by 2^n - 1
 *   -> Remainder
 */
static size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
    __attribute__((const));

static size_t mod_2n1(uint_fast64_t dividend, const unsigned int n)
{
	const uint_fast64_t divisor = (1u << n) - 1;
	uint64_t remainder;
	// printf("%lu, %lu\n", dividend, divisor);

	for (remainder = dividend; dividend > divisor; dividend = remainder) {
		for (remainder = 0; dividend; dividend >>= n) {
			remainder += dividend & divisor;
		}
	}
	// printf("%lu, %lu, %lu\n", dividend, divisor, remainder);
	return remainder == divisor ? 0 : remainder;
}

/**
 * todo
 *   - [ ] Compare mask, shift byte by byte to char arithmetic
 *   - [ ] Replace % with a mod_2n1 version
 */
static size_t hash128_mod(const char *hash, const size_t divisor)
    __attribute__((pure));
static size_t hash128_mod(const char *hash, const size_t divisor)
{
	size_t remainder = 0;

	while (*hash) {
		switch (*hash) {
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			// remainder = (remainder * 16 + (*hash++ - 'a' + 10)) % divisor;
			remainder = (remainder * 16 + (*hash++ - 0x57)) % divisor;
			break;
		default:
			remainder = (remainder * 16 + (*hash++ - '0')) % divisor;
			break;
		}
	}
	return remainder;
}

//----------------------------------------------------------------- Function ---
/**
 * Returns a multiplicative style hash for given key
 *
 * see : Linear congruential generator
 */
size_t hash_multiplicative(const char *key, const size_t seed)
{
	size_t hash = seed;
	// const size_t length = strlen(key);

	// for (size_t i = 0; i < length; i++) {
	// 	hash = seed * hash + key[i];
	// }
	while (*key) {
		hash = seed * hash + *key++;
	}

	return hash;
}

//----------------------------------------------------------------- Function ---
/**
 * Dump and Toy with prime numbers lut and hash
 */
// static void dump_prime_luts(void)
// {
// 	//----------------------------------------------------------- as numbers
// 	puts("\nprime numbers LUT used in cpp stl\n");

// 	for (size_t i = 0; i < ARRAY_LENGTH(prime_sizes); i++) {
// 		printf("prime #%lu \t digits : %lu \t => %lu\n",
// 		       i,
// 		       count_digits(prime_sizes[i]),
// 		       prime_sizes[i]);
// 	}

// 	//----------------------------------------------------------- as strings
// 	puts("\nas strings :\n");
// 	char prime_string[21];

// 	for (size_t i = 0; i < ARRAY_LENGTH(prime_sizes); i++) {
// 		snprintf(prime_string,
// 		         count_digits(prime_sizes[i]) + 1,
// 		         "%lu",
// 		         prime_sizes[i]);
// 		printf("prime #%lu \t digits : %lu \t | hash_mult : %016lx \t => %s\n",
// 		       i,
// 		       strlen(prime_string),
// 		       hash_multiplicative(prime_string, prime_numbers[7]),
// 		       prime_string);
// 	}
// }

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
	// dump_prime_luts();

	//----------------------------------------------------------- hash tests
	const char *keys[] = {
	    "too",
	    "top",
	    "tor",
	    "tpp",
	    "a000",
	    "a001",
	    "a002",
	    "a003",
	    "a004",
	    "a005",
	    "a006",
	    "a007",
	    "a008",
	    "a009",
	    "a010",
	    "a",
	    "aa",
	    "aaa",
	    "add",
	    "sub",
	    "mul",
	    "div",
	    "mod",
	    "pow",
	    "max",
	    "min",
	    "+",
	    "-",
	    "*",
	    "/",
	    "%",
	    "^",
	    ">",
	    "<",
	    "head",
	    "tail",
	    "list",
	    "init",
	    "eval",
	    "join",
	    "cons",
	    "len",
	    "def",
	    "env",
	    "human being ",
	    "human being",
	    "adult",
	    "man",
	    "woman",
	    "child",
	    "boy",
	    "girl",
	    "thing ",
	    "cube",
	    "ball",
	    "mind",
	    "body",
	    "soul",
	    "will",
	    "reality",
	    "truth",
	    "consciousness",
	    "metaphysics",
	    "ethics",
	    "epistemology",
	    "aesthetics",
	    "life",
	    "freedom",
	    "job",
	    "biography",
	    "musician",
	    "explorer",
	    "scientist",
	    "inventor",
	    "writer",
	    "thinker",
	    "politician",
	    "Statesman",
	    "carpenter",
	    "driver",
	    "mathematician",
	    "book",
	    "bibliography",
	    "bread",
	    "coffee",
	    "maize",
	    "cotton",
	    "soya bean",
	    "sorghum",
	    "wheat",
	    "barley",
	    "oats",
	    "fruit",
	    "vegetable",
	    "tobacco",
	    "cheese",
	    "alcohol",
	    "tea",
	    "potato",
	    "water",
	    "beer",
	    "wine",
	    "Africa",
	    "Antarctica",
	    "Asia",
	    "Oceania",
	    "Europe",
	    "North America",
	    "South America",
	    "river",
	    "ocean",
	    "sea",
	    "volcano",
	    "canyon",
	    "reef",
	    "falls",
	    "lake",
	    "city",
	    "dinosaur",
	    "prehistory",
	    "archaeology",
	    "age",
	    "Renaissance",
	    "discovery",
	    "slavery",
	    "revolution",
	    "anarchy",
	    "communism",
	    "fascism",
	    "democracy",
	    "monarchy",
	    "dictatorship",
	    "nationalism",
	    "globalisation",
	    "socialism",
	    "liberalism",
	    "capitalism",
	    "imperialism",
	    "racism",
	    "feminism",
	    "diplomacy",
	    "Human rights",
	    "sexism",
	    "punishment",
	    "abortion",
	    "birth control",
	    "war",
	    "peace",
	    "islam",
	    "judaism",
	    "christianity",
	    "buddhism",
	    "hinduism",
	    "sikhism",
	    "god",
	    "spirit",
	    "atheism",
	    "agnosticism",
	    "humanism",
	    "theatre",
	    "television",
	    "radio",
	    "film",
	    "gambling",
	    "literature",
	    "dance",
	    "art",
	    "music ",
	    "pop",
	    "rock",
	    "traditional",
	    "classical",
	    "jazz",
	    "comics",
	    "games ",
	    "chess",
	    "go",
	    "checkers",
	    "backgammon",
	    "cards",
	    "dice",
	    "Heat",
	    "Electricity",
	    "Magnetism",
	    "Gravity",
	    "Mass",
	    "Force",
	    "Time",
	    "Length",
	    "Area",
	    "Velocity",
	    "Acceleration",
	    "Atom",
	    "Electron",
	    "Proton",
	    "Neutron",
	    "Photon",
	    "quantum",
	    "Chemical element",
	    "Periodic table",
	    "Chemical reaction",
	    "Molecule",
	    "Acid",
	    "Alkali",
	    "pH",
	    "Salt",
	    "Compound",
	    "Organic chemistry",
	    "bacterium",
	    "archaea",
	    "protist",
	    "fungus",
	    "plant ",
	    "tree",
	    "flower",
	    "animal ",
	    "bird",
	    "fish",
	    "horse",
	    "insect",
	    "mammal",
	    "reptile",
	    "snake",
	    "elephant",
	    "camel",
	    "cattle",
	    "sheep",
	    "dog",
	    "cat",
	    "lion",
	    "eagle",
	    "bear",
	    "dragon",
	    "geology",
	    "ecology",
	    "Solar system ",
	    "Sun",
	    "Earth ",
	    "Moon",
	    "Mercury",
	    "Venus",
	    "Mars",
	    "Jupiter",
	    "Saturn",
	    "Uranus",
	    "Neptune",
	    "Pluto",
	    "galaxy",
	    "Milky Way",
	    "Big Bang",
	    "rain",
	    "cloud",
	    "snow",
	    "hail",
	    "hurricane",
	    "salt",
	    "diamond",
	    "chalk",
	    "granite",
	    "flint",
	    "sandstone",
	    "quartz",
	    "gold",
	    "silver",
	    "iron",
	    "copper",
	    "zinc",
	    "tin",
	    "aluminium",
	    "bronze",
	    "brass",
	    "steel",
	    "benzene",
	    "kerosene",
	    "Agriculture",
	    "fire",
	    "Metallurgy",
	    "Writing",
	    "Alphabet",
	    "Ship",
	    "Sail",
	    "Inclined plane",
	    "Wheel",
	    "Pulley",
	    "Lever",
	    "Screw",
	    "Wedge",
	    "Weapon ",
	    "Gun",
	    "Axe",
	    "Sword",
	    "Longbow",
	    "Explosive",
	    "Gunpowder",
	    "Bicycle",
	    "Steam engine",
	    "Train",
	    "Automobile",
	    "Electronics",
	    "Electric motor",
	    "Radio",
	    "Television",
	    "Telephone",
	    "Aircraft",
	    "Computer",
	    "Laser",
	    "Internet",
	    "English",
	    "Mandarin",
	    "Cantonese",
	    "Arabic",
	    "German",
	    "Hindi",
	    "Urdu",
	    "Bahasa Indonesia",
	    "Russian",
	    "Spanish",
	    "French",
	    "Esperanto",
	    "Sanskrit",
	    "Latin",
	    "Greek",
	    "Japanese",
	    "Korean",
	    "Thai",
	    "Vietnamese",
	    "Portuguese",
	    "Italian",
	    "Dutch",
	    "pyramids",
	    "arch",
	    "dome",
	    "bridge",
	    "nail",
	    "number ",
	    "integer",
	    "geometry",
	    "algebra ",
	    "equation",
	    "Variable",
	    "Proof",
	    "Calculus",
	    "Trigonometry",
	    "Calendar",
	    "Set theory",
	    "Logic",
	    "heart",
	    "lungs",
	    "vagina",
	    "penis",
	    "kidneys",
	    "stomach",
	    "liver",
	    "spleen",
	    "intestines",
	    "skeleton",
	    "breast",
	    "skin",
	    "head ",
	    "eye",
	    "mouth",
	    "ear",
	    "brain",
	    "arm ",
	    "elbow",
	    "wrist",
	    "hand ",
	    "finger",
	    "thumb",
	    "leg ",
	    "knee",
	    "ankle",
	    "foot ",
	    "toe",
	    "pregnancy ",
	    "fetus",
	    "placenta",
	    "blindness",
	    "deafness",
	    "disease",
	    "illness",
	    "cancer",
	    "malnutrition",
	    "starvation",
	    "obesity",
	    "cholera",
	    "tuberculosis",
	    "AIDS",
	    "smallpox",
	    "malaria",
	    "dysentery",
	    "leprosy",
	    "virus",
	    "medication",
	    "vaccination",
	    "surgery",
	    "Soccer",
	    "Cricket",
	    "Baseball",
	    "American football",
	    "Athletics",
	    "Swimming",
	    "Earthquake",
	    "Volcano",
	    "Hurricane",
	    "Flood",
	    "Avalanche",
	    "meltdown",
	    "Tornado",
	    "Tsunami",
	    "Blue",
	    "Red",
	    "Green",
	    "Yellow",
	    "Orange",
	    "Black",
	    "White",
	    "January",
	    "February",
	    "March",
	    "April",
	    "May",
	    "June",
	    "July",
	    "August",
	    "September",
	    "October",
	    "November",
	    "December",
	    "Monday",
	    "Tuesday",
	    "Wednesday",
	    "Thursday",
	    "Friday",
	    "Saturday",
	    "Sunday",
	    "one",
	    "two",
	    "three",
	    "four",
	    "five",
	    "six",
	    "seven",
	    "eight",
	    "nine",
	    "ten",
	    "eleven",
	    "twelve",
	    "thirteen",
	    "fourteen",
	    "fifteen",
	    "sixteen",
	    "seventeen",
	    "eighteen",
	    "nineteen",
	    "twenty",
	    "thirty",
	    "forty",
	    "fifty",
	    "sixty",
	    "seventy",
	    "eighty",
	    "ninety",
	    "hundred",
	    "thousand",
	    "million",
	    "billion",
	    "meter",
	    "kilometer",
	    "litre",
	    "gram",
	    "kilogram",
	    "second",
	    "degree",
	    "to be",
	    "to begin",
	    "to give birth",
	    "to be born",
	    "to come",
	    "to count",
	    "to dance",
	    "to die",
	    "to dream",
	    "to drink",
	    "to eat",
	    "to forget",
	    "to get down",
	    "to get in",
	    "to get out",
	    "to get up",
	    "to give",
	    "to go",
	    "to go away",
	    "to hate",
	    "to have",
	    "to leave",
	    "to live",
	    "to love",
	    "to make",
	    "to marry",
	    "to rain",
	    "to read",
	    "to remember",
	    "to run",
	    "to sing",
	    "to sit",
	    "to sleep",
	    "to smoke",
	    "to start",
	    "to stay",
	    "to wish",
	    "to write",
	    "small",
	    "big",
	    "long",
	    "short",
	    "large",
	    "narrow",
	    "deep",
	    "swallow",
	    "fast",
	    "slow",
	    "high",
	    "low",
	    "expensive",
	    "cheap",
	    "young",
	    "aged",
	    "new",
	    "old",
	    "used",
	    "many",
	    "few",
	    "slowly",
	    "early",
	    "late",
	    "far",
	    "near",
	    "name",
	    "word",
	    "love",
	    "to know",
	    "egg",
	    "to fly",
	    "cold",
	};

	//------------------------------------------------------- table size
	// find first prime_size bigger than the key list length times load factor
	size_t size_index     = 0;
	size_t load_factor    = 2;
	uint_fast16_t seed    = prime_numbers[13];
	size_t minimum_length = ARRAY_LENGTH(keys) * load_factor;
	while (prime_sizes[size_index] < minimum_length) {
		size_index++;
	};
#define MAP_SIZE 8192

	KeyValuePair test_table[MAP_SIZE] = {{0, NULL, NULL}};

	puts("\nhash tests :\n");
	size_t hash;
	Hash128 hash128;
	uint_fast32_t hash32;
	size_t bucket_index_mm3;
	size_t collision = 0;
	size_t n         = 11;

	for (size_t i = 0; i < ARRAY_LENGTH(keys); i++) {
		hash128 = murmurhash3_x64(keys[i], strlen(keys[i]), seed);
		hash32  = murmurhash3_x86_32(keys[i], strlen(keys[i]), seed);
		hash    = hash_multiplicative(keys[i], seed);
		// bucket_index     = hash % MAP_SIZE;
		// bucket_index_mm3 = hash32 % 199;
		bucket_index_mm3 = mod_2n1(hash, n);
		bucket_index_mm3 = mod_2n1(hash128.hi, n);
		bucket_index_mm3 = mod_2n1(hash32, n);

		char hash_string[33];
		snprintf(hash_string, 33, "%016lx%016lx", hash128.hi, hash128.lo);
		bucket_index_mm3 = hash128_mod(hash_string, (size_t)((1 << n) - 1));

		printf(
		    "#%lu \t murmurhash3_x64 : %016lx %016lx x86_32 : %08lx    "
		    "bucket : %lu      \t => %s",
		    i,
		    hash128.hi,
		    hash128.lo,
		    hash32,
		    bucket_index_mm3,
		    keys[i]);

		if (test_table[bucket_index_mm3].key != NULL) {
			printf(" <== !!!! COLLISION !!!!\n");
			collision++;
		}
		else {
			putchar('\n');
			test_table[bucket_index_mm3].key =
			    XMALLOC(strlen(keys[i]) + 1,
			            "hash tests",
			            "test_table[bucket_index].key");
			strcpy(test_table[bucket_index_mm3].key, keys[i]);
			test_table[bucket_index_mm3].value = XMALLOC(
			    sizeof(i), "hash tests", "test_table[bucket_index].value");
			*((int *)test_table[bucket_index_mm3].value) = i;
		}
	}

	printf(
	    "\nsize_index : %lu"
	    "\nminimum_length : %lu"
	    "\nprime_sizes[size_index] : %lu"
	    "\nseed : %lu"
	    "\ntable size : %lu\n",
	    size_index,
	    minimum_length,
	    prime_sizes[size_index],
	    seed,
	    (size_t)((1 << n) - 1));

	printf("\n\ntotal collisions : %lu out of %lu keys\n\n",
	       collision,
	       ARRAY_LENGTH(keys));

	//------------------------------------------------- test hash128_mod
	char buffer[33];
	size_t failed = 0;
	size_t expected, result;

	size_t test_count = 10000;

	float start, stop, diff, bench_time;

	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t div = 1; div < test_count; div++) {
			snprintf(buffer, 33, "%016lx", i);
			expected = i % div;
			result   = hash128_mod(buffer, div);

			if (expected != result) {
				failed++;
			}
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("both   \t: %f \n", bench_time);

	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t div = 1; div < test_count; div++) {
			snprintf(buffer, 33, "%016lx", i);
			result   = hash128_mod(buffer, div);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("hash128_mod   \t: %f \n", bench_time);

	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t div = 1; div < test_count; div++) {
			snprintf(buffer, 33, "%016lx", i);
			expected = i % div;
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("i mod div   \t: %f \n", bench_time);


	printf("mod : %lu | %lu\n", expected, result);
	printf("\nhash128_mod total test failed : %lu\n", failed);
	// puts("\nhash table output :\n");
	// for (size_t i = 0; i < ARRAY_LENGTH(test_table); i++) {
	// 	if (test_table[i].key != NULL) {
	// 		printf("#%lu \t %s\t: %d\n",
	// 		       i,
	// 		       test_table[i].key,
	// 		       *((int *)test_table[i].value));
	// 	}
	// }

	//---------------------------------------------------------- cleanup
	for (size_t i = 0; i < ARRAY_LENGTH(test_table); i++) {
		if (test_table[i].key != NULL) {
			XFREE(test_table[i].key, "cleanup");
			XFREE((int *)test_table[i].value, "cleanup");
		}
	}

	return 0;
}