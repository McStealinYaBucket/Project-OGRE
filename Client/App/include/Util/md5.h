/* Util/md5.h -- October 2013 reconstruction.
   RFC 1321 MD5 with the OpenSSL-style MD5_Init/MD5_Update/MD5_Final entry
   points, as used by RBX::MD5HasherImpl. The algorithm is Alexander
   Peslyak's (Solar Designer) public-domain implementation; this text is a
   restatement, not a copy.

   2013 evidence: MD5HasherImpl keeps the context at +0x04 and its result
   string at +0x9c (PLAYER ~MD5HasherImpl 0x00652bc0, create 0x00652bf0), so
   sizeof(MD5_CTX) is 0x98: two count words, four state words, a 64-byte
   input buffer and a 16-word decoded block. The three entry points are
   PLAYER 0x00855d30 (Init), 0x00855d60 (Update), 0x00855e10 (Final); the
   transform is 0x00855370. Dropped: the 2016 header's HAVE_OPENSSL switch. */
#ifndef RBX_UTIL_MD5_H
#define RBX_UTIL_MD5_H

typedef unsigned int MD5_u32plus; // any unsigned type of at least 32 bits

typedef struct
{
	MD5_u32plus lo, hi;          // message length in bytes, 61 bits split low/high
	MD5_u32plus a, b, c, d;      // chaining state
	unsigned char buffer[64];    // pending partial block
	MD5_u32plus block[16];       // current block as little-endian words
} MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif

void MD5_Init(MD5_CTX* ctx);
void MD5_Update(MD5_CTX* ctx, void* data, unsigned long size);
void MD5_Final(unsigned char* result, MD5_CTX* ctx);

#ifdef __cplusplus
}
#endif

#endif
