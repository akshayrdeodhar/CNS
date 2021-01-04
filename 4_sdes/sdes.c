#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* S-DES permutations */

typedef uint8_t u8;

typedef uint32_t u32;

static const u8 IPlen = 8;
static const u8 IP[] = {1, 5, 2, 0, 3, 7, 4, 6};

static const u8 invIPlen = 8;
static const u8 invIP[] = {3, 0, 2, 4, 6, 1, 7, 5};

static const u8 n_rounds = 2;

static const u8 P8len = 8;
static const u8 P8[] = {5, 2, 6, 3, 7, 4, 9, 8};

static const u8 P10len = 10;
static const u8 P10[] = {2, 4, 1, 6, 3, 9, 0, 8, 7, 5};

static const u8 EPlen = 8;
static const u8 EP[] = {3, 0, 1, 2, 1, 2, 3, 0};

/* rotate x, an input of m bits, n bits to the left */
/* note: the macro expects safe inputs (having the specified number of bits */
/* this might "seem" a rightshift- but in the "abstract" representation, the
 * leftmost bit, is bit 0, which is actually the LSB */
u32 rotate_l(const u32 x, const u8 m, const u8 n) {
		return ((1 << m) - 1) & ((x >> n) | (x << (m - n)));
}

/* append nl LSBs of l to nr LSBs or r, with l at the MSBs in result */
/* note: the macro expects safe inputs (having the specified number of bits */
u32 append(const u32 l, const u8 nl, const u32 r, const u8 nr) {
		return (r << nr) | l;
}

/* extract n bits starting from bit l from x */
u32 getbits(const u32 x, const u8 l, const u8 n) {
		u32 mask = ((1 << n) - 1) << l; /* [l:(l + n)] 1 bits */
		return (x & mask) >> l;
}

u32 permute_bits(const u32 in, const u8 *permutation, const u8 oplen) { 
		int i;
		u32 op = 0;
		u32 temp;
		for (i = 0; i < oplen; i++) {
				temp = (in & (1 << permutation[i])) ? (1 << i) : 0;
				op |= temp;
				/* permutation[i]th bit of in placed in ith position of out */
		}
		return op;
}

void printbits(u32 x, int n) {
		u32 one_bit = 1;
		char this_bit;
		for (int i = 0; i < n; i++) {
				this_bit = (one_bit & x) ? '1' : '0';
				putchar(this_bit);
				one_bit <<= 1;
		}
}


u8 get_round_subkey(u32 key, u8 round_number) {

		/* round 0 entry is dummy */

		/* this is the prefix sum of the rotation array, so that the
		 * "effective" rotation can be used directly */
		static const u8 shift_routine[] = {0, 1, 3};

		u32 p10_x = permute_bits(key, P10, P10len);

		u32 left_bits, right_bits, rotated_left, rotated_right, combination, final;

		left_bits = getbits(p10_x, 0, 5);

		right_bits = getbits(p10_x, 5, 5);

		rotated_left = rotate_l(left_bits, 5, shift_routine[round_number]);

		rotated_right = rotate_l(right_bits, 5, shift_routine[round_number]);

		combination = append(rotated_left, 5, rotated_right, 5);

		final = permute_bits(combination, P8, P8len);

		return final;

		/* return permute_bits(
						append(
								rotate_l(
										getbits(
												p10_x, 
												0,
												5),
										5,
										shift_routine[round_number]),
								5,
								rotate_l(
										getbits(
												p10_x, 
												5, 
												5),
										5,
										shift_routine[round_number]),
								5),
						P8,
						P8len);
		*/
}

void debugprint(char *string, u32 val, u8 bits) {
		printf("%s: ", string); printbits(val, bits); printf("\n");
}

/* takes 4 bits as input
 * expands to 8 bit
 * divides into 2 4 bit parts
 * uses S-boxes to convert them to 2 bits each
 * permutes the 4 bit sequence obtained
 * returns result 
 * */
u8 T(u8 x, u32 key, u8 round) {
		u8 permuted, boxin, constructed, row, col, key_combined, k_i;

		const static u8 S[2][4][4] = {
				/* S[0] */
				{
					{1, 0, 3, 2},
					{3, 2, 1, 0},
					{0, 2, 1, 3},
					{3, 1, 3, 2}
				},
				/* S[1] */
				{
					{0, 1, 2, 3},
					{2, 0, 1, 3},
					{3, 0, 1, 0},
					{2, 1, 0, 3}
				}
		};

		const static u8 P4[] = {1, 3, 2, 0};
		const static u8 P4len = 4;

		/* permute bits of x to get an 8-bit string */
		permuted = permute_bits(x, EP, EPlen);

		k_i = get_round_subkey(key, round);

		key_combined = permuted ^ k_i;


		constructed = 0;

		for (int i = 0; i < 2; i++) {

				/* select 4 bits from boxin */
				boxin = getbits(key_combined, i * 4, 4);

				row = (getbits(boxin, 0, 1) << 1) | getbits(boxin, 3, 1);

				col = (getbits(boxin, 1, 1) << 1) | getbits(boxin, 2, 1);

				/* row is chosen based on bits 0 and 3 */

				/* choose proper S-box, get substitution result, append to
				 * current constructed result */
				constructed = append(constructed, 2, S[i][row][col], 2);
		}

		u8 p4_x = permute_bits(constructed, P4, P4len);


		return p4_x;
}

u8 pi_operation(u8 in, u32 key, u8 round) {

		u8 x, xdash;

		/* first 4 bits */
		x = getbits(in, 0, 4);

		/* last 4 bits */
		xdash = getbits(in, 4, 4);

		/* (X + Ti(X'), X') */
		return append(
						x ^ T(xdash, key, round),
						4,
						xdash,
						4);
}

u8 sdes_encrypt(u8 in, u32 key) {

		u8 permuted, first_pi, shifted, second_pi, unpermuted;

		permuted = permute_bits(in, IP, IPlen);

		first_pi = pi_operation(permuted, key, 1);

		shifted = rotate_l(first_pi, 8, 4);

		second_pi = pi_operation(shifted, key, 2);

		unpermuted = permute_bits(second_pi, invIP, invIPlen);

		return unpermuted;
}

u8 sdes_decrypt(u8 in, u32 key) {

		u8 permuted, second_pi, shifted, first_pi, unpermuted;

		permuted = permute_bits(in, IP, IPlen);

		second_pi = pi_operation(permuted, key, 2);

		shifted = rotate_l(second_pi, 8, 4);

		first_pi = pi_operation(shifted, key, 1);

		unpermuted = permute_bits(first_pi, invIP, invIPlen);

		return unpermuted;
}

/* based on:
 * A SIMPLIFIED DATA ENCRYPTION STANDARD ALGORITHM 
 * Edward. F. Schaefer */

int main(int argc, char *argv[]) {
		if (argc != 5) {
				fprintf(stderr, "usage: sdes <mode> <key> <input_file> <output_file>\n");
				return EINVAL;
		}

		u32 key = atoi(argv[1]) % 1024;
		u8 in, op, de;

		key = atoi(argv[2])% 256;
		int fi, fo;

		char mode = argv[1][0];

		/* verify for all possible combinations */
		/* for (int j = 0; j < 1024; j++) {

			for (int i = 0; i < 256; i++) {
					key = j;
					in = i;
					op = sdes_encrypt(in, key);

					de = sdes_decrypt(op, key);

					if (in != de) {
							printf("wrong: "); printbits(in, 8); printf("\tseen is: "); printbits(de, 8); printf("\n");
					}
			}

		} */

		if ((fi = open(argv[3], O_RDONLY)) == -1) {
				perror(argv[3]);
				return errno;
		}

		if ((fo = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) == -1) {
				perror(argv[4]);
				return errno;
		}

		while(read(fi, &in, 1) == 1) {
				if (mode == 'e') {
						op = sdes_encrypt(in, key);
				}
				else if (mode == 'd') {
						op = sdes_decrypt(in, key);
				}
				else {
						break;
				}
				if (write(fo, &op, 1) != 1) {
					perror("write");
					return errno;
				}
		}

		close(fi);
		close(fo);

		return 0;
}
