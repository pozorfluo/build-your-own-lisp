#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define BENCHMARK

#ifdef BENCHMARK
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define LOOP_SIZE 10000000

#define START_BENCH(start) start = (float)clock() / CLOCKS_PER_SEC

#define STOP_BENCH(start, stop, diff, result)                                  \
	stop   = (float)clock() / CLOCKS_PER_SEC;                                  \
	diff   = stop - start;                                                     \
	result = diff;

#else
#define LOOP_SIZE 0
#define START_BENCH(start)
#define STOP_BENCH(start, stop, diff, result)
#define BENCH(expression, loop, result)
#endif /* BENCHMARK */

//------------------------------------------------------------------- MACROS ---
#define ARRAY_LENGTH(_array) (sizeof(_array) / sizeof((_array)[0]))

size_t hash_tab_goto(const unsigned char *key)
    __attribute__((const));
size_t hash_tab(const unsigned char *key)
    __attribute__((const, always_inline));

size_t hash_tab_goto(const unsigned char *key)
{
#define NEXT_BYTE() goto *jump_table[key[i++]]
#define XOR_HASH(_char)                                                        \
	hash ^= xor_table[_char] ^ _char;                                          \
	NEXT_BYTE();

	static const size_t xor_table[] = {
	    27971, 29696, 1025,  26659, 62932, 5107,  35958, 44509, 34357, 7372,
	    12308, 36243, 62261, 65251, 39383, 10080, 28288, 11518, 1323,  44124,
	    52132, 23168, 3596,  31582, 8167,  36443, 11626, 18430, 30798, 43965,
	    59727, 1268,  28298, 35204, 51329, 27156, 58003, 29983, 8709,  47190,
	    60865, 53205, 51095, 62274, 14082, 12782, 62190, 39013, 15442, 30154,
	    30941, 2339,  42072, 39864, 45305, 52485, 49671, 19860, 55841, 28916,
	    35175, 2534,  35942, 28144, 23848, 29273, 59281, 9056,  30103, 60243,
	    25134, 22411, 13879, 57006, 12427, 32343, 4009,  37415, 45652, 51638,
	    18637, 8093,  11142, 7979,  34882, 59976, 42164, 59067, 8225,  53147,
	    13070, 29442, 10300, 51391, 16478, 27106, 7805,  54803, 55782, 64995,
	    17263, 27761, 25926, 48941, 3350,  3286,  44653, 27656, 8271,  45953,
	    10756, 39484, 64531, 6395,  8859,  62236, 23993, 28374, 40716, 44295,
	    43328, 36200, 18531, 6098,  1233,  4770,  2583,  6643,  10674, 36756,
	    11180, 25383, 34628, 59497, 48477, 47740};
	/*, 60115, 17503, 28003, 10726,
	57784, 29011, 22565, 32690, 35743, 16689, 4870,  21481, 3928,  59612,
	51349, 30690, 23197, 33206, 55257, 44558, 16565, 51897, 17954, 28538,
	2155,  60808, 13606, 34417, 2366,  34448, 16632, 9374,  48957, 50496,
	41252, 26248, 52014, 53996, 5376,  61881, 51902, 50329, 46143, 51791,
	24067, 37453, 32080, 41273, 45460, 63236, 42166, 18393, 40321, 55987,
	62124, 57274, 29204, 10995, 5691,  12962, 15814, 174,   22691, 52354,
	14838, 43033, 37401, 33756, 40194, 53728, 40672, 49446, 37287, 1115,
	48396, 26441, 4633,  37936, 35133, 31609, 49588, 48314, 26113, 8879,
	2437,  65176, 46792, 40390, 1328,  64648, 56361, 1794,  8309,  56310,
	7282,  21274, 52611, 1793,  65286, 39254, 3863,  15019, 44753, 60445,
	44700, 8710,  61930, 33656, 63895, 34451, 42755, 4603,  45978, 14290,
	1180,  24418, 38082, 64862, 64087, 50749};*/

	static const void *jump_table[] = {&&NUL,
	                                   &&SOH,
	                                   &&STX,
	                                   &&ETX,
	                                   &&EOT,
	                                   &&ENQ,
	                                   &&ACK,
	                                   &&BEL,
	                                   &&BS,
	                                   &&HT,
	                                   &&LF,
	                                   &&VT,
	                                   &&FF,
	                                   &&CR,
	                                   &&SO,
	                                   &&SI,
	                                   &&DLE,
	                                   &&DC1,
	                                   &&DC2,
	                                   &&DC3,
	                                   &&DC4,
	                                   &&NAK,
	                                   &&SYN,
	                                   &&ETB,
	                                   &&CAN,
	                                   &&EM,
	                                   &&SUB,
	                                   &&ESC,
	                                   &&FS,
	                                   &&GS,
	                                   &&RS,
	                                   &&US,
	                                   &&space,
	                                   &&exclamation_mak,
	                                   &&double_quote,
	                                   &&hash,
	                                   &&dollar,
	                                   &&percent,
	                                   &&ampersand,
	                                   &&single_quote,
	                                   &&open_parenthesis,
	                                   &&closing_parenthesis,
	                                   &&asterisk,
	                                   &&plus,
	                                   &&comma,
	                                   &&hyphen,
	                                   &&period,
	                                   &&slash,
	                                   &&zero,
	                                   &&one,
	                                   &&two,
	                                   &&three,
	                                   &&four,
	                                   &&five,
	                                   &&six,
	                                   &&seven,
	                                   &&eight,
	                                   &&nine,
	                                   &&colon,
	                                   &&semicolon,
	                                   &&less_than,
	                                   &&equals,
	                                   &&greater_than,
	                                   &&question_mark,
	                                   &&at,
	                                   &&A,
	                                   &&B,
	                                   &&C,
	                                   &&D,
	                                   &&E,
	                                   &&F,
	                                   &&G,
	                                   &&H,
	                                   &&I,
	                                   &&J,
	                                   &&K,
	                                   &&L,
	                                   &&M,
	                                   &&N,
	                                   &&O,
	                                   &&P,
	                                   &&Q,
	                                   &&R,
	                                   &&S,
	                                   &&T,
	                                   &&U,
	                                   &&V,
	                                   &&W,
	                                   &&X,
	                                   &&Y,
	                                   &&Z,
	                                   &&opening_bracket,
	                                   &&backslash,
	                                   &&closing_bracket,
	                                   &&caret,
	                                   &&underscore,
	                                   &&grave_accent,
	                                   &&a,
	                                   &&b,
	                                   &&c,
	                                   &&d,
	                                   &&e,
	                                   &&f,
	                                   &&g,
	                                   &&h,
	                                   &&i,
	                                   &&j,
	                                   &&k,
	                                   &&l,
	                                   &&m,
	                                   &&n,
	                                   &&o,
	                                   &&p,
	                                   &&q,
	                                   &&r,
	                                   &&s,
	                                   &&t,
	                                   &&u,
	                                   &&v,
	                                   &&w,
	                                   &&x,
	                                   &&y,
	                                   &&z,
	                                   &&opening_brace,
	                                   &&vertical_bar,
	                                   &&closing_brace,
	                                   &&tilde,
	                                   &&del};

	int i       = 0;
	size_t hash = 0;

	NEXT_BYTE();
	while (1) {
	NUL:
		return hash;
	SOH:
		XOR_HASH(0x01);
	STX:
		XOR_HASH(0x02);
	ETX:
		XOR_HASH(0x03);
	EOT:
		XOR_HASH(0x04);
	ENQ:
		XOR_HASH(0x05);
	ACK:
		XOR_HASH(0x06);
	BEL:
		XOR_HASH(0x07);
	BS:
		XOR_HASH(0x08);
	HT:
		XOR_HASH(0x09);
	LF:
		XOR_HASH(0x0A);
	VT:
		XOR_HASH(0x0B);
	FF:
		XOR_HASH(0x0C);
	CR:
		XOR_HASH(0x0D);
	SO:
		XOR_HASH(0x0E);
	SI:
		XOR_HASH(0x0F);
	DLE:
		XOR_HASH(0x10);
	DC1:
		XOR_HASH(0x11);
	DC2:
		XOR_HASH(0x12);
	DC3:
		XOR_HASH(0x13);
	DC4:
		XOR_HASH(0x14);
	NAK:
		XOR_HASH(0x15);
	SYN:
		XOR_HASH(0x16);
	ETB:
		XOR_HASH(0x17);
	CAN:
		XOR_HASH(0x18);
	EM:
		XOR_HASH(0x19);
	SUB:
		XOR_HASH(0x1A);
	ESC:
		XOR_HASH(0x1B);
	FS:
		XOR_HASH(0x1C);
	GS:
		XOR_HASH(0x1D);
	RS:
		XOR_HASH(0x1E);
	US:
		XOR_HASH(0x1F);
	space:
		XOR_HASH(0x20);
	exclamation_mak:
		XOR_HASH(0x21);
	double_quote:
		XOR_HASH(0x22);
	hash:
		XOR_HASH(0x23);
	dollar:
		XOR_HASH(0x24);
	percent:
		XOR_HASH(0x25);
	ampersand:
		XOR_HASH(0x26);
	single_quote:
		XOR_HASH(0x27);
	open_parenthesis:
		XOR_HASH(0x28);
	closing_parenthesis:
		XOR_HASH(0x29);
	asterisk:
		XOR_HASH(0x2A);
	plus:
		XOR_HASH(0x2B);
	comma:
		XOR_HASH(0x2C);
	hyphen:
		XOR_HASH(0x2D);
	period:
		XOR_HASH(0x2E);
	slash:
		XOR_HASH(0x2F);
	zero:
		XOR_HASH(0x30);
	one:
		XOR_HASH(0x31);
	two:
		XOR_HASH(0x32);
	three:
		XOR_HASH(0x33);
	four:
		XOR_HASH(0x34);
	five:
		XOR_HASH(0x35);
	six:
		XOR_HASH(0x36);
	seven:
		XOR_HASH(0x37);
	eight:
		XOR_HASH(0x38);
	nine:
		XOR_HASH(0x39);
	colon:
		XOR_HASH(0x3A);
	semicolon:
		XOR_HASH(0x3B);
	less_than:
		XOR_HASH(0x3C);
	equals:
		XOR_HASH(0x3D);
	greater_than:
		XOR_HASH(0x3E);
	question_mark:
		XOR_HASH(0x3F);
	at:
		XOR_HASH(0x40);
	A:
		XOR_HASH(0x41);
	B:
		XOR_HASH(0x42);
	C:
		XOR_HASH(0x43);
	D:
		XOR_HASH(0x44);
	E:
		XOR_HASH(0x45);
	F:
		XOR_HASH(0x46);
	G:
		XOR_HASH(0x47);
	H:
		XOR_HASH(0x48);
	I:
		XOR_HASH(0x49);
	J:
		XOR_HASH(0x4A);
	K:
		XOR_HASH(0x4B);
	L:
		XOR_HASH(0x4C);
	M:
		XOR_HASH(0x4D);
	N:
		XOR_HASH(0x4E);
	O:
		XOR_HASH(0x4F);
	P:
		XOR_HASH(0x50);
	Q:
		XOR_HASH(0x51);
	R:
		XOR_HASH(0x52);
	S:
		XOR_HASH(0x53);
	T:
		XOR_HASH(0x54);
	U:
		XOR_HASH(0x55);
	V:
		XOR_HASH(0x56);
	W:
		XOR_HASH(0x57);
	X:
		XOR_HASH(0x58);
	Y:
		XOR_HASH(0x59);
	Z:
		XOR_HASH(0x5A);
	opening_bracket:
		XOR_HASH(0x5B);
	backslash:
		XOR_HASH(0x5C);
	closing_bracket:
		XOR_HASH(0x5D);
	caret:
		XOR_HASH(0x5E);
	underscore:
		XOR_HASH(0x5F);
	grave_accent:
		XOR_HASH(0x60);
	a:
		XOR_HASH(0x61);
	b:
		XOR_HASH(0x62);
	c:
		XOR_HASH(0x63);
	d:
		XOR_HASH(0x64);
	e:
		XOR_HASH(0x65);
	f:
		XOR_HASH(0x66);
	g:
		XOR_HASH(0x67);
	h:
		XOR_HASH(0x68);
	i:
		XOR_HASH(0x69);
	j:
		XOR_HASH(0x6A);
	k:
		XOR_HASH(0x6B);
	l:
		XOR_HASH(0x6C);
	m:
		XOR_HASH(0x6D);
	n:
		XOR_HASH(0x6E);
	o:
		XOR_HASH(0x6F);
	p:
		XOR_HASH(0x70);
	q:
		XOR_HASH(0x71);
	r:
		XOR_HASH(0x72);
	s:
		XOR_HASH(0x73);
	t:
		XOR_HASH(0x74);
	u:
		XOR_HASH(0x75);
	v:
		XOR_HASH(0x76);
	w:
		XOR_HASH(0x77);
	x:
		XOR_HASH(0x78);
	y:
		XOR_HASH(0x79);
	z:
		XOR_HASH(0x7A);
	opening_brace:
		XOR_HASH(0x7B);
	vertical_bar:
		XOR_HASH(0x7C);
	closing_brace:
		XOR_HASH(0x7D);
	tilde:
		XOR_HASH(0x7E);
	del:
		XOR_HASH(0x7F);
	}
}

inline size_t hash_tab(const unsigned char *key)
{
	static const size_t xor_table[] = {
	    27971, 29696, 1025,  26659, 62932, 5107,  35958, 44509, 34357, 7372,
	    12308, 36243, 62261, 65251, 39383, 10080, 28288, 11518, 1323,  44124,
	    52132, 23168, 3596,  31582, 8167,  36443, 11626, 18430, 30798, 43965,
	    59727, 1268,  28298, 35204, 51329, 27156, 58003, 29983, 8709,  47190,
	    60865, 53205, 51095, 62274, 14082, 12782, 62190, 39013, 15442, 30154,
	    30941, 2339,  42072, 39864, 45305, 52485, 49671, 19860, 55841, 28916,
	    35175, 2534,  35942, 28144, 23848, 29273, 59281, 9056,  30103, 60243,
	    25134, 22411, 13879, 57006, 12427, 32343, 4009,  37415, 45652, 51638,
	    18637, 8093,  11142, 7979,  34882, 59976, 42164, 59067, 8225,  53147,
	    13070, 29442, 10300, 51391, 16478, 27106, 7805,  54803, 55782, 64995,
	    17263, 27761, 25926, 48941, 3350,  3286,  44653, 27656, 8271,  45953,
	    10756, 39484, 64531, 6395,  8859,  62236, 23993, 28374, 40716, 44295,
	    43328, 36200, 18531, 6098,  1233,  4770,  2583,  6643,  10674, 36756,
	    11180, 25383, 34628, 59497, 48477, 47740};
	/*, 60115, 17503, 28003, 10726,
	57784, 29011, 22565, 32690, 35743, 16689, 4870,  21481, 3928,  59612,
	51349, 30690, 23197, 33206, 55257, 44558, 16565, 51897, 17954, 28538,
	2155,  60808, 13606, 34417, 2366,  34448, 16632, 9374,  48957, 50496,
	41252, 26248, 52014, 53996, 5376,  61881, 51902, 50329, 46143, 51791,
	24067, 37453, 32080, 41273, 45460, 63236, 42166, 18393, 40321, 55987,
	62124, 57274, 29204, 10995, 5691,  12962, 15814, 174,   22691, 52354,
	14838, 43033, 37401, 33756, 40194, 53728, 40672, 49446, 37287, 1115,
	48396, 26441, 4633,  37936, 35133, 31609, 49588, 48314, 26113, 8879,
	2437,  65176, 46792, 40390, 1328,  64648, 56361, 1794,  8309,  56310,
	7282,  21274, 52611, 1793,  65286, 39254, 3863,  15019, 44753, 60445,
	44700, 8710,  61930, 33656, 63895, 34451, 42755, 4603,  45978, 14290,
	1180,  24418, 38082, 64862, 64087, 50749};*/

	size_t hash = 0;

	while (*key) {
		hash ^= xor_table[*key] ^ *key;
		key++;
	}

	return hash;
}

//--------------------------------------------------------------------- MAIN ---
// int main(void)
// {

// 	char key[256];

// 	// unsigned char code[] = "abd\1\2\3";
// 	for (;;) {

// 		fputs("\x1b[102m > \x1b[0m", stdout);
// 		fgets(key, 255, stdin);

// 		// if ((strcmp(key, "exit")) == 0) {
// 		// 	break;
// 		// }
// 		// scanf("%s", key);
// 		printf("%lu\n", hash_tab((unsigned char *)key));
// 	}

// 	return 0;
// }

//--------------------------------------------------------------------- MAIN ---
int main(void)
{
//------------------------------------------------------------ setup
#define KEYPOOL_SIZE 4096
	char random_keys[KEYPOOL_SIZE] = {'\0'};
	int r;

	for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
		r = rand() % 32;
		// Replace with a-z for interval [0,25]
		// Leave '\0' for interval [26,32]
		if (r < 26) {
			random_keys[i] = (char)(r + 0x61);
			putchar(random_keys[i]);
		}
	}

	puts("\nhash tests :\n");
	size_t hash;
	// uint_fast32_t hash32;
	// Hash128 hash128;

	// uint_fast16_t seed = 41;

	size_t test_count = 100000;

	float start, stop, diff, bench_time;

	//---------------------------------------------------------- bench A
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab_goto((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab_goto   \t: %f \n", bench_time);
	printf("%016lx\n", hash);

	//---------------------------------------------------------- bench B
	START_BENCH(start);
	for (size_t i = 0; i < test_count; i++) {
		for (size_t i = 0; i < KEYPOOL_SIZE; i++) {
			hash = hash_tab((unsigned char *)&random_keys[i]);
		}
	}
	STOP_BENCH(start, stop, diff, bench_time);
	printf("bench hash_tab   \t: %f \n", bench_time);
	printf("%016lx\n", hash);
	//---------------------------------------------------------- cleanup

	return 0;
}