/* App/util/md5.c -- October 2013 reconstruction.
 *
 * MD5 (RFC 1321) behind MD5_Init / MD5_Update / MD5_Final, the routines
 * RBX::MD5HasherImpl calls (see Util/md5.h for the 2013 addresses and the
 * 0x98-byte context MD5HasherImpl embeds). The method is Alexander Peslyak's
 * (Solar Designer) public-domain implementation: the transform is fully
 * unrolled (PLAYER 0x00855370 is 2493 bytes, one block of 64 steps per loop
 * pass), message words are decoded byte by byte into ctx->block in round 1
 * (VC9 defines no __i386__, so no unaligned-load shortcut) and read back from
 * it in rounds 2-4, and the bit count is kept as a byte count split at bit 29.
 * The expressions below keep that method's evaluation order; the text,
 * naming and layout are this project's.
 */

#include <string.h>

#include "Util/md5.h"

/* Round functions; F and G in the form that needs no AND-NOT. */
#define MD5_F(x, y, z) ((z) ^ ((x) & ((y) ^ (z))))
#define MD5_G(x, y, z) ((y) ^ ((z) & ((x) ^ (y))))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | ~(z)))

/* Message word n of the current block: decoded on first use, cached after. */
#define WORD_LOAD(n) \
	(ctx->block[(n)] = (MD5_u32plus)p[(n) * 4] | \
		((MD5_u32plus)p[(n) * 4 + 1] << 8) | \
		((MD5_u32plus)p[(n) * 4 + 2] << 16) | \
		((MD5_u32plus)p[(n) * 4 + 3] << 24))
#define WORD(n) (ctx->block[(n)])

/* One step: w += fn(x, y, z) + m + k; w = rotl(w, r); w += x. */
#define OP(fn, w, x, y, z, m, k, r) \
	(w) += fn((x), (y), (z)) + (m) + (k); \
	(w) = (((w) << (r)) | (((w) & 0xffffffff) >> (32 - (r)))); \
	(w) += (x);

/* Runs the compression function over len bytes (a multiple of 64) at src.
   Leaves the length counters alone; returns the first byte not consumed. */
static void* md5_blocks(MD5_CTX* ctx, void* src, unsigned long len)
{
	unsigned char* p;
	MD5_u32plus a, b, c, d;
	MD5_u32plus a0, b0, c0, d0;

	p = (unsigned char*)src;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		a0 = a; b0 = b; c0 = c; d0 = d;

		/* round 1: words in order, shifts 7 12 17 22 */
		OP(MD5_F, a, b, c, d, WORD_LOAD(0),  0xD76AA478, 7)
		OP(MD5_F, d, a, b, c, WORD_LOAD(1),  0xE8C7B756, 12)
		OP(MD5_F, c, d, a, b, WORD_LOAD(2),  0x242070DB, 17)
		OP(MD5_F, b, c, d, a, WORD_LOAD(3),  0xC1BDCEEE, 22)
		OP(MD5_F, a, b, c, d, WORD_LOAD(4),  0xF57C0FAF, 7)
		OP(MD5_F, d, a, b, c, WORD_LOAD(5),  0x4787C62A, 12)
		OP(MD5_F, c, d, a, b, WORD_LOAD(6),  0xA8304613, 17)
		OP(MD5_F, b, c, d, a, WORD_LOAD(7),  0xFD469501, 22)
		OP(MD5_F, a, b, c, d, WORD_LOAD(8),  0x698098D8, 7)
		OP(MD5_F, d, a, b, c, WORD_LOAD(9),  0x8B44F7AF, 12)
		OP(MD5_F, c, d, a, b, WORD_LOAD(10), 0xFFFF5BB1, 17)
		OP(MD5_F, b, c, d, a, WORD_LOAD(11), 0x895CD7BE, 22)
		OP(MD5_F, a, b, c, d, WORD_LOAD(12), 0x6B901122, 7)
		OP(MD5_F, d, a, b, c, WORD_LOAD(13), 0xFD987193, 12)
		OP(MD5_F, c, d, a, b, WORD_LOAD(14), 0xA679438E, 17)
		OP(MD5_F, b, c, d, a, WORD_LOAD(15), 0x49B40821, 22)

		/* round 2: word (1 + 5i) mod 16, shifts 5 9 14 20 */
		OP(MD5_G, a, b, c, d, WORD(1),  0xF61E2562, 5)
		OP(MD5_G, d, a, b, c, WORD(6),  0xC040B340, 9)
		OP(MD5_G, c, d, a, b, WORD(11), 0x265E5A51, 14)
		OP(MD5_G, b, c, d, a, WORD(0),  0xE9B6C7AA, 20)
		OP(MD5_G, a, b, c, d, WORD(5),  0xD62F105D, 5)
		OP(MD5_G, d, a, b, c, WORD(10), 0x02441453, 9)
		OP(MD5_G, c, d, a, b, WORD(15), 0xD8A1E681, 14)
		OP(MD5_G, b, c, d, a, WORD(4),  0xE7D3FBC8, 20)
		OP(MD5_G, a, b, c, d, WORD(9),  0x21E1CDE6, 5)
		OP(MD5_G, d, a, b, c, WORD(14), 0xC33707D6, 9)
		OP(MD5_G, c, d, a, b, WORD(3),  0xF4D50D87, 14)
		OP(MD5_G, b, c, d, a, WORD(8),  0x455A14ED, 20)
		OP(MD5_G, a, b, c, d, WORD(13), 0xA9E3E905, 5)
		OP(MD5_G, d, a, b, c, WORD(2),  0xFCEFA3F8, 9)
		OP(MD5_G, c, d, a, b, WORD(7),  0x676F02D9, 14)
		OP(MD5_G, b, c, d, a, WORD(12), 0x8D2A4C8A, 20)

		/* round 3: word (5 + 3i) mod 16, shifts 4 11 16 23 */
		OP(MD5_H, a, b, c, d, WORD(5),  0xFFFA3942, 4)
		OP(MD5_H, d, a, b, c, WORD(8),  0x8771F681, 11)
		OP(MD5_H, c, d, a, b, WORD(11), 0x6D9D6122, 16)
		OP(MD5_H, b, c, d, a, WORD(14), 0xFDE5380C, 23)
		OP(MD5_H, a, b, c, d, WORD(1),  0xA4BEEA44, 4)
		OP(MD5_H, d, a, b, c, WORD(4),  0x4BDECFA9, 11)
		OP(MD5_H, c, d, a, b, WORD(7),  0xF6BB4B60, 16)
		OP(MD5_H, b, c, d, a, WORD(10), 0xBEBFBC70, 23)
		OP(MD5_H, a, b, c, d, WORD(13), 0x289B7EC6, 4)
		OP(MD5_H, d, a, b, c, WORD(0),  0xEAA127FA, 11)
		OP(MD5_H, c, d, a, b, WORD(3),  0xD4EF3085, 16)
		OP(MD5_H, b, c, d, a, WORD(6),  0x04881D05, 23)
		OP(MD5_H, a, b, c, d, WORD(9),  0xD9D4D039, 4)
		OP(MD5_H, d, a, b, c, WORD(12), 0xE6DB99E5, 11)
		OP(MD5_H, c, d, a, b, WORD(15), 0x1FA27CF8, 16)
		OP(MD5_H, b, c, d, a, WORD(2),  0xC4AC5665, 23)

		/* round 4: word 7i mod 16, shifts 6 10 15 21 */
		OP(MD5_I, a, b, c, d, WORD(0),  0xF4292244, 6)
		OP(MD5_I, d, a, b, c, WORD(7),  0x432AFF97, 10)
		OP(MD5_I, c, d, a, b, WORD(14), 0xAB9423A7, 15)
		OP(MD5_I, b, c, d, a, WORD(5),  0xFC93A039, 21)
		OP(MD5_I, a, b, c, d, WORD(12), 0x655B59C3, 6)
		OP(MD5_I, d, a, b, c, WORD(3),  0x8F0CCC92, 10)
		OP(MD5_I, c, d, a, b, WORD(10), 0xFFEFF47D, 15)
		OP(MD5_I, b, c, d, a, WORD(1),  0x85845DD1, 21)
		OP(MD5_I, a, b, c, d, WORD(8),  0x6FA87E4F, 6)
		OP(MD5_I, d, a, b, c, WORD(15), 0xFE2CE6E0, 10)
		OP(MD5_I, c, d, a, b, WORD(6),  0xA3014314, 15)
		OP(MD5_I, b, c, d, a, WORD(13), 0x4E0811A1, 21)
		OP(MD5_I, a, b, c, d, WORD(4),  0xF7537E82, 6)
		OP(MD5_I, d, a, b, c, WORD(11), 0xBD3AF235, 10)
		OP(MD5_I, c, d, a, b, WORD(2),  0x2AD7D2BB, 15)
		OP(MD5_I, b, c, d, a, WORD(9),  0xEB86D391, 21)

		a += a0; b += b0; c += c0; d += d0;
		p += 64;
	} while (len -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;
	return p;
}

void MD5_Init(MD5_CTX* ctx)
{
	/* RFC 1321 initial chaining values */
	ctx->a = 0x67452301;
	ctx->b = 0xEFCDAB89;
	ctx->c = 0x98BADCFE;
	ctx->d = 0x10325476;
	ctx->lo = 0;
	ctx->hi = 0;
}

void MD5_Update(MD5_CTX* ctx, void* data, unsigned long size)
{
	MD5_u32plus oldLo;
	unsigned long have, room;

	/* 29 low bits in lo, the rest carried into hi */
	oldLo = ctx->lo;
	if ((ctx->lo = (oldLo + size) & 0x1fffffff) < oldLo)
		ctx->hi++;
	ctx->hi += size >> 29;

	have = oldLo & 0x3f;
	if (have) {
		room = 64 - have;
		if (size < room) {
			memcpy(&ctx->buffer[have], data, size);
			return;
		}
		/* top up the pending block and compress it */
		memcpy(&ctx->buffer[have], data, room);
		data = (unsigned char*)data + room;
		size -= room;
		md5_blocks(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = md5_blocks(ctx, data, size & ~(unsigned long)0x3f);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

/* Little-endian store of a 32-bit word into dst[0..3]. */
#define PUT_LE32(dst, v) \
	(dst)[0] = (v); \
	(dst)[1] = (v) >> 8; \
	(dst)[2] = (v) >> 16; \
	(dst)[3] = (v) >> 24;

void MD5_Final(unsigned char* result, MD5_CTX* ctx)
{
	unsigned long have, room;

	/* 0x80 terminator, zero fill, then the 64-bit bit count */
	have = ctx->lo & 0x3f;
	ctx->buffer[have++] = 0x80;
	room = 64 - have;

	if (room < 8) {
		/* no space for the length: finish this block, start another */
		memset(&ctx->buffer[have], 0, room);
		md5_blocks(ctx, ctx->buffer, 64);
		have = 0;
		room = 64;
	}
	memset(&ctx->buffer[have], 0, room - 8);

	ctx->lo <<= 3;
	PUT_LE32(&ctx->buffer[56], ctx->lo)
	PUT_LE32(&ctx->buffer[60], ctx->hi)

	md5_blocks(ctx, ctx->buffer, 64);

	PUT_LE32(&result[0], ctx->a)
	PUT_LE32(&result[4], ctx->b)
	PUT_LE32(&result[8], ctx->c)
	PUT_LE32(&result[12], ctx->d)

	/* wipe the state */
	memset(ctx, 0, sizeof(*ctx));
}
