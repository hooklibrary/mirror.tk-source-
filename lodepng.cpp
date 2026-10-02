/*
LodePNG version 20180819

Copyright (c) 2005-2018 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
The manual and changelog are in the header file "lodepng.h"
Rename this file to lodepng.cpp to use it for C++, or to lodepng.c to use it for C.
*/

#include "lodepng.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

const char* LODEPNG_VERSION_STRING = "20180819";

/*
This source file is built up in the following large parts. The code sections
with the "LODEPNG_COMPILE_" #defines divide this up further in an intermixed way.
-Tools for C and common code for PNG and Zlib
-C Code for Zlib (huffman, deflate, ...)
-C Code for PNG (file format chunks, adam7, PNG filters, color conversions, ...)
-The C++ wrapper around all of the above
*/

/*The malloc, realloc and free functions defined here with "lodepng_" in front
of the name, so that you can easily change them to others related to your
platform if needed. Everything else in the code calls these. Pass
-DLODEPNG_NO_COMPILE_ALLOCATORS to the compiler, or comment out
#define LODEPNG_COMPILE_ALLOCATORS in the header, to disable the ones here and
define them in your own project's source files without needing to change
lodepng source code. Don't forget to remove "static" if you copypaste them
from here.*/

#ifdef LODEPNG_COMPILE_ALLOCATORS
static void* lodepng_malloc(size_t size)
{
#ifdef LODEPNG_MAX_ALLOC
	if (size > LODEPNG_MAX_ALLOC) return 0;
#endif
	return malloc(size);
}

static void* lodepng_realloc(void* ptr, size_t new_size)
{
#ifdef LODEPNG_MAX_ALLOC
	if (new_size > LODEPNG_MAX_ALLOC) return 0;
#endif
	return realloc(ptr, new_size);
}

static void lodepng_free(void* ptr)
{
	free(ptr);
}
#else /*LODEPNG_COMPILE_ALLOCATORS*/
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif /*LODEPNG_COMPILE_ALLOCATORS*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // Tools for C, and common code for PNG and Zlib.                       // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#define LODEPNG_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LODEPNG_MIN(a, b) (((a) < (b)) ? (a) : (b))

/*
Often in case of an error a value is assigned to a variable and then it breaks
out of a loop (to go to the cleanup phase of a function). This macro does that.
It makes the error handling code shorter and more readable.

Example: if(!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83);
*/
#define CERROR_BREAK(errorvar, code)\
{\
  errorvar = code;\
  break;\
}

/*version of CERROR_BREAK that assumes the common case where the error variable is named "error"*/
#define ERROR_BREAK(code) CERROR_BREAK(error, code)

/*Set error var to the error code, and return it.*/
#define CERROR_RETURN_ERROR(errorvar, code)\
{\
  errorvar = code;\
  return code;\
}

/*Try the code, if it returns error, also return the error.*/
#define CERROR_TRY_RETURN(call)\
{\
  unsigned error = call;\
  if(error) return error;\
}

/*Set error var to the error code, and return from the void function.*/
#define CERROR_RETURN(errorvar, code)\
{\
  errorvar = code;\
  return;\
}

/*
About uivector, ucvector and string:
-All of them wrap dynamic arrays or text strings in a similar way.
-LodePNG was originally written in C++. The vectors replace the std::vectors that were used in the C++ version.
-The string tools are made to avoid problems with compilers that declare things like strncat as deprecated.
-They're not used in the interface, only internally in this file as static functions.
-As with many other structs in this file, the init and cleanup functions serve as ctor and dtor.
*/

#ifdef LODEPNG_COMPILE_ZLIB
/*dynamic vector of unsigned ints*/
typedef struct uivector
{
	unsigned* data;
	size_t size; /*size in number of unsigned longs*/
	size_t allocsize; /*allocated size in bytes*/
} uivector;

static void uivector_cleanup(void* p)
{
	((uivector*)p)->size = ((uivector*)p)->allocsize = 0;
	lodepng_free(((uivector*)p)->data);
	((uivector*)p)->data = NULL;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_reserve(uivector* p, size_t allocsize)
{
	if (allocsize > p->allocsize)
	{
		size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
		void* data = lodepng_realloc(p->data, newsize);
		if (data)
		{
			p->allocsize = newsize;
			p->data = (unsigned*)data;
		}
		else return 0; /*error: not enough memory*/
	}
	return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_resize(uivector* p, size_t size)
{
	if (!uivector_reserve(p, size * sizeof(unsigned))) return 0;
	p->size = size;
	return 1; /*success*/
}

/*resize and give all new elements the value*/
static unsigned uivector_resizev(uivector* p, size_t size, unsigned value)
{
	size_t oldsize = p->size, i;
	if (!uivector_resize(p, size)) return 0;
	for (i = oldsize; i < size; ++i) p->data[i] = value;
	return 1;
}

static void uivector_init(uivector* p)
{
	p->data = NULL;
	p->size = p->allocsize = 0;
}

#ifdef LODEPNG_COMPILE_ENCODER
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_push_back(uivector* p, unsigned c)
{
	if (!uivector_resize(p, p->size + 1)) return 0;
	p->data[p->size - 1] = c;
	return 1;
}
#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_ZLIB*/

/* /////////////////////////////////////////////////////////////////////////// */

/*dynamic vector of unsigned chars*/
typedef struct ucvector
{
	unsigned char* data;
	size_t size; /*used size*/
	size_t allocsize; /*allocated size*/
} ucvector;

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_reserve(ucvector* p, size_t allocsize)
{
	if (allocsize > p->allocsize)
	{
		size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
		void* data = lodepng_realloc(p->data, newsize);
		if (data)
		{
			p->allocsize = newsize;
			p->data = (unsigned char*)data;
		}
		else return 0; /*error: not enough memory*/
	}
	return 1;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_resize(ucvector* p, size_t size)
{
	if (!ucvector_reserve(p, size * sizeof(unsigned char))) return 0;
	p->size = size;
	return 1; /*success*/
}

#ifdef LODEPNG_COMPILE_PNG

static void ucvector_cleanup(void* p)
{
	((ucvector*)p)->size = ((ucvector*)p)->allocsize = 0;
	lodepng_free(((ucvector*)p)->data);
	((ucvector*)p)->data = NULL;
}

static void ucvector_init(ucvector* p)
{
	p->data = NULL;
	p->size = p->allocsize = 0;
}
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ZLIB
/*you can both convert from vector to buffer&size and vica versa. If you use
init_buffer to take over a buffer and size, it is not needed to use cleanup*/
static void ucvector_init_buffer(ucvector* p, unsigned char* buffer, size_t size)
{
	p->data = buffer;
	p->allocsize = p->size = size;
}
#endif /*LODEPNG_COMPILE_ZLIB*/

#if (defined(LODEPNG_COMPILE_PNG) && defined(LODEPNG_COMPILE_ANCILLARY_CHUNKS)) || defined(LODEPNG_COMPILE_ENCODER)
/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_push_back(ucvector* p, unsigned char c)
{
	if (!ucvector_resize(p, p->size + 1)) return 0;
	p->data[p->size - 1] = c;
	return 1;
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/


/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

/*free string pointer and set it to NULL*/
static void string_cleanup(char** out)
{
	lodepng_free(*out);
	*out = NULL;
}

/* dynamically allocates a new string with a copy of the null terminated input text */
static char* alloc_string(const char* in)
{
	size_t insize = strlen(in);
	char* out = (char*)lodepng_malloc(insize + 1);
	if (out)
	{
		size_t i;
		for (i = 0; i != insize; ++i)
		{
			out[i] = in[i];
		}
		out[i] = 0;
	}
	return out;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#endif /*LODEPNG_COMPILE_PNG*/

/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_read32bitInt(const unsigned char* buffer)
{
	return (unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

#if defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)
/*buffer must have at least 4 allocated bytes available*/
static void lodepng_set32bitInt(unsigned char* buffer, unsigned value)
{
	buffer[0] = (unsigned char)((value >> 24) & 0xff);
	buffer[1] = (unsigned char)((value >> 16) & 0xff);
	buffer[2] = (unsigned char)((value >> 8) & 0xff);
	buffer[3] = (unsigned char)((value) & 0xff);
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/

#ifdef LODEPNG_COMPILE_ENCODER
static void lodepng_add32bitInt(ucvector* buffer, unsigned value)
{
	ucvector_resize(buffer, buffer->size + 4); /*todo: give error if resize failed*/
	lodepng_set32bitInt(&buffer->data[buffer->size - 4], value);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / File IO                                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DISK

/* returns negative value on error. This should be pure C compatible, so no fstat. */
static long lodepng_filesize(const char* filename)
{
	FILE* file;
	long size;
	file = fopen(filename, "rb");
	if (!file) return -1;

	if (fseek(file, 0, SEEK_END) != 0)
	{
		fclose(file);
		return -1;
	}

	size = ftell(file);
	/* It may give LONG_MAX as directory size, this is invalid for us. */
	if (size == LONG_MAX) size = -1;

	fclose(file);
	return size;
}

/* load file into buffer that already has the correct allocated size. Returns error code.*/
static unsigned lodepng_buffer_file(unsigned char* out, size_t size, const char* filename)
{
	FILE* file;
	size_t readsize;
	file = fopen(filename, "rb");
	if (!file) return 78;

	readsize = fread(out, 1, size, file);
	fclose(file);

	if (readsize != size) return 78;
	return 0;
}

unsigned lodepng_load_file(unsigned char** out, size_t* outsize, const char* filename)
{
	long size = lodepng_filesize(filename);
	if (size < 0) return 78;
	*outsize = (size_t)size;

	*out = (unsigned char*)lodepng_malloc((size_t)size);
	if (!(*out) && size > 0) return 83; /*the above malloc failed*/

	return lodepng_buffer_file(*out, (size_t)size, filename);
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned lodepng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename)
{
	FILE* file;
	file = fopen(filename, "wb");
	if (!file) return 79;
	fwrite((char*)buffer, 1, buffersize, file);
	fclose(file);
	return 0;
}

#endif /*LODEPNG_COMPILE_DISK*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of common code and tools. Begin of Zlib related code.            // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_ENCODER
/*TODO: this ignores potential out of memory errors*/
#define addBitToStream(/*size_t**/ bitpointer, /*ucvector**/ bitstream, /*unsigned char*/ bit)\
{\
  /*add a new byte at the end*/\
  if(((*bitpointer) & 7) == 0) ucvector_push_back(bitstream, (unsigned char)0);\
  /*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/\
  (bitstream->data[bitstream->size - 1]) |= (bit << ((*bitpointer) & 0x7));\
  ++(*bitpointer);\
}

static void addBitsToStream(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
	size_t i;
	for (i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> i) & 1));
}

static void addBitsToStreamReversed(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
	size_t i;
	for (i = 0; i != nbits; ++i) addBitToStream(bitpointer, bitstream, (unsigned char)((value >> (nbits - 1 - i)) & 1));
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

#define READBIT(bitpointer, bitstream) ((bitstream[bitpointer >> 3] >> (bitpointer & 0x7)) & (unsigned char)1)

static unsigned char readBitFromStream(size_t* bitpointer, const unsigned char* bitstream)
{
	unsigned char result = (unsigned char)(READBIT(*bitpointer, bitstream));
	++(*bitpointer);
	return result;
}

static unsigned readBitsFromStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
	unsigned result = 0, i;
	for (i = 0; i != nbits; ++i)
	{
		result += ((unsigned)READBIT(*bitpointer, bitstream)) << i;
		++(*bitpointer);
	}
	return result;
}
#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflate - Huffman                                                      / */
/* ////////////////////////////////////////////////////////////////////////// */

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define NUM_DEFLATE_CODE_SYMBOLS 288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define NUM_DISTANCE_SYMBOLS 32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define NUM_CODE_LENGTH_CODES 19

/*the base lengths represented by codes 257-285*/
static const unsigned LENGTHBASE[29]
= { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
67, 83, 99, 115, 131, 163, 195, 227, 258 };

/*the extra bits used by codes 257-285 (added to base length)*/
static const unsigned LENGTHEXTRA[29]
= { 0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
4,  4,  4,   4,   5,   5,   5,   5,   0 };

/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)*/
static const unsigned DISTANCEBASE[30]
= { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };

/*the extra bits of backwards distances (added to base)*/
static const unsigned DISTANCEEXTRA[30]
= { 0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,
8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13 };

/*the order in which "code length alphabet code lengths" are stored, out of this
the huffman tree of the dynamic huffman tree lengths is generated*/
static const unsigned CLCL_ORDER[NUM_CODE_LENGTH_CODES]
= { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

/* ////////////////////////////////////////////////////////////////////////// */

/*
Huffman tree struct, containing multiple representations of the tree
*/
typedef struct HuffmanTree
{
	unsigned* tree2d;
	unsigned* tree1d;
	unsigned* lengths; /*the lengths of the codes of the 1d-tree*/
	unsigned maxbitlen; /*maximum number of bits a single code can get*/
	unsigned numcodes; /*number of symbols in the alphabet = number of codes*/
} HuffmanTree;

/*function used for debug purposes to draw the tree in ascii art with C++*/
/*
static void HuffmanTree_draw(HuffmanTree* tree)
{
std::cout << "tree. length: " << tree->numcodes << " maxbitlen: " << tree->maxbitlen << std::endl;
for(size_t i = 0; i != tree->tree1d.size; ++i)
{
if(tree->lengths.data[i])
std::cout << i << " " << tree->tree1d.data[i] << " " << tree->lengths.data[i] << std::endl;
}
std::cout << std::endl;
}*/

static void HuffmanTree_init(HuffmanTree* tree)
{
	tree->tree2d = 0;
	tree->tree1d = 0;
	tree->lengths = 0;
}

static void HuffmanTree_cleanup(HuffmanTree* tree)
{
	lodepng_free(tree->tree2d);
	lodepng_free(tree->tree1d);
	lodepng_free(tree->lengths);
}

/*the tree representation used by the decoder. return value is error*/
static unsigned HuffmanTree_make2DTree(HuffmanTree* tree)
{
	unsigned nodefilled = 0; /*up to which node it is filled*/
	unsigned treepos = 0; /*position in the tree (1 of the numcodes columns)*/
	unsigned n, i;

	tree->tree2d = (unsigned*)lodepng_malloc(tree->numcodes * 2 * sizeof(unsigned));
	if (!tree->tree2d) return 83; /*alloc fail*/

								  /*
								  convert tree1d[] to tree2d[][]. In the 2D array, a value of 32767 means
								  uninited, a value >= numcodes is an address to another bit, a value < numcodes
								  is a code. The 2 rows are the 2 possible bit values (0 or 1), there are as
								  many columns as codes - 1.
								  A good huffman tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
								  Here, the internal nodes are stored (what their 0 and 1 option point to).
								  There is only memory for such good tree currently, if there are more nodes
								  (due to too long length codes), error 55 will happen
								  */
	for (n = 0; n < tree->numcodes * 2; ++n)
	{
		tree->tree2d[n] = 32767; /*32767 here means the tree2d isn't filled there yet*/
	}

	for (n = 0; n < tree->numcodes; ++n) /*the codes*/
	{
		for (i = 0; i != tree->lengths[n]; ++i) /*the bits for this code*/
		{
			unsigned char bit = (unsigned char)((tree->tree1d[n] >> (tree->lengths[n] - i - 1)) & 1);
			/*oversubscribed, see comment in lodepng_error_text*/
			if (treepos > 2147483647 || treepos + 2 > tree->numcodes) return 55;
			if (tree->tree2d[2 * treepos + bit] == 32767) /*not yet filled in*/
			{
				if (i + 1 == tree->lengths[n]) /*last bit*/
				{
					tree->tree2d[2 * treepos + bit] = n; /*put the current code in it*/
					treepos = 0;
				}
				else
				{
					/*put address of the next step in here, first that address has to be found of course
					(it's just nodefilled + 1)...*/
					++nodefilled;
					/*addresses encoded with numcodes added to it*/
					tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;
					treepos = nodefilled;
				}
			}
			else treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
		}
	}

	for (n = 0; n < tree->numcodes * 2; ++n)
	{
		if (tree->tree2d[n] == 32767) tree->tree2d[n] = 0; /*remove possible remaining 32767's*/
	}

	return 0;
}

/*
Second step for the ...makeFromLengths and ...makeFromFrequencies functions.
numcodes, lengths and maxbitlen must already be filled in correctly. return
value is error.
*/
static unsigned HuffmanTree_makeFromLengths2(HuffmanTree* tree)
{
	uivector blcount;
	uivector nextcode;
	unsigned error = 0;
	unsigned bits, n;

	uivector_init(&blcount);
	uivector_init(&nextcode);

	tree->tree1d = (unsigned*)lodepng_malloc(tree->numcodes * sizeof(unsigned));
	if (!tree->tree1d) error = 83; /*alloc fail*/

	if (!uivector_resizev(&blcount, tree->maxbitlen + 1, 0)
		|| !uivector_resizev(&nextcode, tree->maxbitlen + 1, 0))
		error = 83; /*alloc fail*/

	if (!error)
	{
		/*step 1: count number of instances of each code length*/
		for (bits = 0; bits != tree->numcodes; ++bits) ++blcount.data[tree->lengths[bits]];
		/*step 2: generate the nextcode values*/
		for (bits = 1; bits <= tree->maxbitlen; ++bits)
		{
			nextcode.data[bits] = (nextcode.data[bits - 1] + blcount.data[bits - 1]) << 1;
		}
		/*step 3: generate all the codes*/
		for (n = 0; n != tree->numcodes; ++n)
		{
			if (tree->lengths[n] != 0) tree->tree1d[n] = nextcode.data[tree->lengths[n]]++;
		}
	}

	uivector_cleanup(&blcount);
	uivector_cleanup(&nextcode);

	if (!error) return HuffmanTree_make2DTree(tree);
	else return error;
}

/*
given the code lengths (as stored in the PNG file), generate the tree as defined
by Deflate. maxbitlen is the maximum bits that a code in the tree can have.
return value is error.
*/
static unsigned HuffmanTree_makeFromLengths(HuffmanTree* tree, const unsigned* bitlen,
	size_t numcodes, unsigned maxbitlen)
{
	unsigned i;
	tree->lengths = (unsigned*)lodepng_malloc(numcodes * sizeof(unsigned));
	if (!tree->lengths) return 83; /*alloc fail*/
	for (i = 0; i != numcodes; ++i) tree->lengths[i] = bitlen[i];
	tree->numcodes = (unsigned)numcodes; /*number of symbols*/
	tree->maxbitlen = maxbitlen;
	return HuffmanTree_makeFromLengths2(tree);
}

#ifdef LODEPNG_COMPILE_ENCODER

/*BPM: Boundary Package Merge, see "A Fast and Space-Economical Algorithm for Length-Limited Coding",
Jyrki Katajainen, Alistair Moffat, Andrew Turpin, 1995.*/

/*chain node for boundary package merge*/
typedef struct BPMNode
{
	int weight; /*the sum of all weights in this chain*/
	unsigned index; /*index of this leaf node (called "count" in the paper)*/
	struct BPMNode* tail; /*the next nodes in this chain (null if last)*/
	int in_use;
} BPMNode;

/*lists of chains*/
typedef struct BPMLists
{
	/*memory pool*/
	unsigned memsize;
	BPMNode* memory;
	unsigned numfree;
	unsigned nextfree;
	BPMNode** freelist;
	/*two heads of lookahead chains per list*/
	unsigned listsize;
	BPMNode** chains0;
	BPMNode** chains1;
} BPMLists;

/*creates a new chain node with the given parameters, from the memory in the lists */
static BPMNode* bpmnode_create(BPMLists* lists, int weight, unsigned index, BPMNode* tail)
{
	unsigned i;
	BPMNode* result;

	/*memory full, so garbage collect*/
	if (lists->nextfree >= lists->numfree)
	{
		/*mark only those that are in use*/
		for (i = 0; i != lists->memsize; ++i) lists->memory[i].in_use = 0;
		for (i = 0; i != lists->listsize; ++i)
		{
			BPMNode* node;
			for (node = lists->chains0[i]; node != 0; node = node->tail) node->in_use = 1;
			for (node = lists->chains1[i]; node != 0; node = node->tail) node->in_use = 1;
		}
		/*collect those that are free*/
		lists->numfree = 0;
		for (i = 0; i != lists->memsize; ++i)
		{
			if (!lists->memory[i].in_use) lists->freelist[lists->numfree++] = &lists->memory[i];
		}
		lists->nextfree = 0;
	}

	result = lists->freelist[lists->nextfree++];
	result->weight = weight;
	result->index = index;
	result->tail = tail;
	return result;
}

/*sort the leaves with stable mergesort*/
static void bpmnode_sort(BPMNode* leaves, size_t num)
{
	BPMNode* mem = (BPMNode*)lodepng_malloc(sizeof(*leaves) * num);
	size_t width, counter = 0;
	for (width = 1; width < num; width *= 2)
	{
		BPMNode* a = (counter & 1) ? mem : leaves;
		BPMNode* b = (counter & 1) ? leaves : mem;
		size_t p;
		for (p = 0; p < num; p += 2 * width)
		{
			size_t q = (p + width > num) ? num : (p + width);
			size_t r = (p + 2 * width > num) ? num : (p + 2 * width);
			size_t i = p, j = q, k;
			for (k = p; k < r; k++)
			{
				if (i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
				else b[k] = a[j++];
			}
		}
		counter++;
	}
	if (counter & 1) memcpy(leaves, mem, sizeof(*leaves) * num);
	lodepng_free(mem);
}

/*Boundary Package Merge step, numpresent is the amount of leaves, and c is the current chain.*/
static void boundaryPM(BPMLists* lists, BPMNode* leaves, size_t numpresent, int c, int num)
{
	unsigned lastindex = lists->chains1[c]->index;

	if (c == 0)
	{
		if (lastindex >= numpresent) return;
		lists->chains0[c] = lists->chains1[c];
		lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, 0);
	}
	else
	{
		/*sum of the weights of the head nodes of the previous lookahead chains.*/
		int sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
		lists->chains0[c] = lists->chains1[c];
		if (lastindex < numpresent && sum > leaves[lastindex].weight)
		{
			lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, lists->chains1[c]->tail);
			return;
		}
		lists->chains1[c] = bpmnode_create(lists, sum, lastindex, lists->chains1[c - 1]);
		/*in the end we are only interested in the chain of the last list, so no
		need to recurse if we're at the last one (this gives measurable speedup)*/
		if (num + 1 < (int)(2 * numpresent - 2))
		{
			boundaryPM(lists, leaves, numpresent, c - 1, num);
			boundaryPM(lists, leaves, numpresent, c - 1, num);
		}
	}
}

unsigned lodepng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
	size_t numcodes, unsigned maxbitlen)
{
	unsigned error = 0;
	unsigned i;
	size_t numpresent = 0; /*number of symbols with non-zero frequency*/
	BPMNode* leaves; /*the symbols, only those with > 0 frequency*/

	if (numcodes == 0) return 80; /*error: a tree of 0 symbols is not supposed to be made*/
	if ((1u << maxbitlen) < (unsigned)numcodes) return 80; /*error: represent all symbols*/

	leaves = (BPMNode*)lodepng_malloc(numcodes * sizeof(*leaves));
	if (!leaves) return 83; /*alloc fail*/

	for (i = 0; i != numcodes; ++i)
	{
		if (frequencies[i] > 0)
		{
			leaves[numpresent].weight = (int)frequencies[i];
			leaves[numpresent].index = i;
			++numpresent;
		}
	}

	for (i = 0; i != numcodes; ++i) lengths[i] = 0;

	/*ensure at least two present symbols. There should be at least one symbol
	according to RFC 1951 section 3.2.7. Some decoders incorrectly require two. To
	make these work as well ensure there are at least two symbols. The
	Package-Merge code below also doesn't work correctly if there's only one
	symbol, it'd give it the theoritical 0 bits but in practice zlib wants 1 bit*/
	if (numpresent == 0)
	{
		lengths[0] = lengths[1] = 1; /*note that for RFC 1951 section 3.2.7, only lengths[0] = 1 is needed*/
	}
	else if (numpresent == 1)
	{
		lengths[leaves[0].index] = 1;
		lengths[leaves[0].index == 0 ? 1 : 0] = 1;
	}
	else
	{
		BPMLists lists;
		BPMNode* node;

		bpmnode_sort(leaves, numpresent);

		lists.listsize = maxbitlen;
		lists.memsize = 2 * maxbitlen * (maxbitlen + 1);
		lists.nextfree = 0;
		lists.numfree = lists.memsize;
		lists.memory = (BPMNode*)lodepng_malloc(lists.memsize * sizeof(*lists.memory));
		lists.freelist = (BPMNode**)lodepng_malloc(lists.memsize * sizeof(BPMNode*));
		lists.chains0 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
		lists.chains1 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
		if (!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83; /*alloc fail*/

		if (!error)
		{
			for (i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

			bpmnode_create(&lists, leaves[0].weight, 1, 0);
			bpmnode_create(&lists, leaves[1].weight, 2, 0);

			for (i = 0; i != lists.listsize; ++i)
			{
				lists.chains0[i] = &lists.memory[0];
				lists.chains1[i] = &lists.memory[1];
			}

			/*each boundaryPM call adds one chain to the last list, and we need 2 * numpresent - 2 chains.*/
			for (i = 2; i != 2 * numpresent - 2; ++i) boundaryPM(&lists, leaves, numpresent, (int)maxbitlen - 1, (int)i);

			for (node = lists.chains1[maxbitlen - 1]; node; node = node->tail)
			{
				for (i = 0; i != node->index; ++i) ++lengths[leaves[i].index];
			}
		}

		lodepng_free(lists.memory);
		lodepng_free(lists.freelist);
		lodepng_free(lists.chains0);
		lodepng_free(lists.chains1);
	}

	lodepng_free(leaves);
	return error;
}

/*Create the Huffman tree given the symbol frequencies*/
static unsigned HuffmanTree_makeFromFrequencies(HuffmanTree* tree, const unsigned* frequencies,
	size_t mincodes, size_t numcodes, unsigned maxbitlen)
{
	unsigned error = 0;
	while (!frequencies[numcodes - 1] && numcodes > mincodes) --numcodes; /*trim zeroes*/
	tree->maxbitlen = maxbitlen;
	tree->numcodes = (unsigned)numcodes; /*number of symbols*/
	tree->lengths = (unsigned*)lodepng_realloc(tree->lengths, numcodes * sizeof(unsigned));
	if (!tree->lengths) return 83; /*alloc fail*/
								   /*initialize all lengths to 0*/
	memset(tree->lengths, 0, numcodes * sizeof(unsigned));

	error = lodepng_huffman_code_lengths(tree->lengths, frequencies, numcodes, maxbitlen);
	if (!error) error = HuffmanTree_makeFromLengths2(tree);
	return error;
}

static unsigned HuffmanTree_getCode(const HuffmanTree* tree, unsigned index)
{
	return tree->tree1d[index];
}

static unsigned HuffmanTree_getLength(const HuffmanTree* tree, unsigned index)
{
	return tree->lengths[index];
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*get the literal and length code tree of a deflated block with fixed tree, as per the deflate specification*/
static unsigned generateFixedLitLenTree(HuffmanTree* tree)
{
	unsigned i, error = 0;
	unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
	if (!bitlen) return 83; /*alloc fail*/

							/*288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused*/
	for (i = 0; i <= 143; ++i) bitlen[i] = 8;
	for (i = 144; i <= 255; ++i) bitlen[i] = 9;
	for (i = 256; i <= 279; ++i) bitlen[i] = 7;
	for (i = 280; i <= 287; ++i) bitlen[i] = 8;

	error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15);

	lodepng_free(bitlen);
	return error;
}

/*get the distance code tree of a deflated block with fixed tree, as specified in the deflate specification*/
static unsigned generateFixedDistanceTree(HuffmanTree* tree)
{
	unsigned i, error = 0;
	unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
	if (!bitlen) return 83; /*alloc fail*/

							/*there are 32 distance codes, but 30-31 are unused*/
	for (i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen[i] = 5;
	error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15);

	lodepng_free(bitlen);
	return error;
}

#ifdef LODEPNG_COMPILE_DECODER

/*
returns the code, or (unsigned)(-1) if error happened
inbitlength is the length of the complete buffer, in bits (so its byte length times 8)
*/
static unsigned huffmanDecodeSymbol(const unsigned char* in, size_t* bp,
	const HuffmanTree* codetree, size_t inbitlength)
{
	unsigned treepos = 0, ct;
	for (;;)
	{
		if (*bp >= inbitlength) return (unsigned)(-1); /*error: end of input memory reached without endcode*/
													   /*
													   decode the symbol from the tree. The "readBitFromStream" code is inlined in
													   the expression below because this is the biggest bottleneck while decoding
													   */
		ct = codetree->tree2d[(treepos << 1) + READBIT(*bp, in)];
		++(*bp);
		if (ct < codetree->numcodes) return ct; /*the symbol is decoded, return it*/
		else treepos = ct - codetree->numcodes; /*symbol not yet decoded, instead move tree position*/

		if (treepos >= codetree->numcodes) return (unsigned)(-1); /*error: it appeared outside the codetree*/
	}
}
#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Inflator (Decompressor)                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

/*get the tree of a deflated block with fixed tree, as specified in the deflate specification*/
static void getTreeInflateFixed(HuffmanTree* tree_ll, HuffmanTree* tree_d)
{
	/*TODO: check for out of memory errors*/
	generateFixedLitLenTree(tree_ll);
	generateFixedDistanceTree(tree_d);
}

/*get the tree of a deflated block with dynamic tree, the tree itself is also Huffman compressed with a known tree*/
static unsigned getTreeInflateDynamic(HuffmanTree* tree_ll, HuffmanTree* tree_d,
	const unsigned char* in, size_t* bp, size_t inlength)
{
	/*make sure that length values that aren't filled in will be 0, or a wrong tree will be generated*/
	unsigned error = 0;
	unsigned n, HLIT, HDIST, HCLEN, i;
	size_t inbitlength = inlength * 8;

	/*see comments in deflateDynamic for explanation of the context and these variables, it is analogous*/
	unsigned* bitlen_ll = 0; /*lit,len code lengths*/
	unsigned* bitlen_d = 0; /*dist code lengths*/
							/*code length code lengths ("clcl"), the bit lengths of the huffman tree used to compress bitlen_ll and bitlen_d*/
	unsigned* bitlen_cl = 0;
	HuffmanTree tree_cl; /*the code tree for code length codes (the huffman tree for compressed huffman trees)*/

	if ((*bp) + 14 > (inlength << 3)) return 49; /*error: the bit pointer is or will go past the memory*/

												 /*number of literal/length codes + 257. Unlike the spec, the value 257 is added to it here already*/
	HLIT = readBitsFromStream(bp, in, 5) + 257;
	/*number of distance codes. Unlike the spec, the value 1 is added to it here already*/
	HDIST = readBitsFromStream(bp, in, 5) + 1;
	/*number of code length codes. Unlike the spec, the value 4 is added to it here already*/
	HCLEN = readBitsFromStream(bp, in, 4) + 4;

	if ((*bp) + HCLEN * 3 > (inlength << 3)) return 50; /*error: the bit pointer is or will go past the memory*/

	HuffmanTree_init(&tree_cl);

	while (!error)
	{
		/*read the code length codes out of 3 * (amount of code length codes) bits*/

		bitlen_cl = (unsigned*)lodepng_malloc(NUM_CODE_LENGTH_CODES * sizeof(unsigned));
		if (!bitlen_cl) ERROR_BREAK(83 /*alloc fail*/);

		for (i = 0; i != NUM_CODE_LENGTH_CODES; ++i)
		{
			if (i < HCLEN) bitlen_cl[CLCL_ORDER[i]] = readBitsFromStream(bp, in, 3);
			else bitlen_cl[CLCL_ORDER[i]] = 0; /*if not, it must stay 0*/
		}

		error = HuffmanTree_makeFromLengths(&tree_cl, bitlen_cl, NUM_CODE_LENGTH_CODES, 7);
		if (error) break;

		/*now we can use this tree to read the lengths for the tree that this function will return*/
		bitlen_ll = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
		bitlen_d = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
		if (!bitlen_ll || !bitlen_d) ERROR_BREAK(83 /*alloc fail*/);
		for (i = 0; i != NUM_DEFLATE_CODE_SYMBOLS; ++i) bitlen_ll[i] = 0;
		for (i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen_d[i] = 0;

		/*i is the current symbol we're reading in the part that contains the code lengths of lit/len and dist codes*/
		i = 0;
		while (i < HLIT + HDIST)
		{
			unsigned code = huffmanDecodeSymbol(in, bp, &tree_cl, inbitlength);
			if (code <= 15) /*a length code*/
			{
				if (i < HLIT) bitlen_ll[i] = code;
				else bitlen_d[i - HLIT] = code;
				++i;
			}
			else if (code == 16) /*repeat previous*/
			{
				unsigned replength = 3; /*read in the 2 bits that indicate repeat length (3-6)*/
				unsigned value; /*set value to the previous code*/

				if (i == 0) ERROR_BREAK(54); /*can't repeat previous if i is 0*/

				if ((*bp + 2) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
				replength += readBitsFromStream(bp, in, 2);

				if (i < HLIT + 1) value = bitlen_ll[i - 1];
				else value = bitlen_d[i - HLIT - 1];
				/*repeat this value in the next lengths*/
				for (n = 0; n < replength; ++n)
				{
					if (i >= HLIT + HDIST) ERROR_BREAK(13); /*error: i is larger than the amount of codes*/
					if (i < HLIT) bitlen_ll[i] = value;
					else bitlen_d[i - HLIT] = value;
					++i;
				}
			}
			else if (code == 17) /*repeat "0" 3-10 times*/
			{
				unsigned replength = 3; /*read in the bits that indicate repeat length*/
				if ((*bp + 3) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
				replength += readBitsFromStream(bp, in, 3);

				/*repeat this value in the next lengths*/
				for (n = 0; n < replength; ++n)
				{
					if (i >= HLIT + HDIST) ERROR_BREAK(14); /*error: i is larger than the amount of codes*/

					if (i < HLIT) bitlen_ll[i] = 0;
					else bitlen_d[i - HLIT] = 0;
					++i;
				}
			}
			else if (code == 18) /*repeat "0" 11-138 times*/
			{
				unsigned replength = 11; /*read in the bits that indicate repeat length*/
				if ((*bp + 7) > inbitlength) ERROR_BREAK(50); /*error, bit pointer jumps past memory*/
				replength += readBitsFromStream(bp, in, 7);

				/*repeat this value in the next lengths*/
				for (n = 0; n < replength; ++n)
				{
					if (i >= HLIT + HDIST) ERROR_BREAK(15); /*error: i is larger than the amount of codes*/

					if (i < HLIT) bitlen_ll[i] = 0;
					else bitlen_d[i - HLIT] = 0;
					++i;
				}
			}
			else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
			{
				if (code == (unsigned)(-1))
				{
					/*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
					(10=no endcode, 11=wrong jump outside of tree)*/
					error = (*bp) > inbitlength ? 10 : 11;
				}
				else error = 16; /*unexisting code, this can never happen*/
				break;
			}
		}
		if (error) break;

		if (bitlen_ll[256] == 0) ERROR_BREAK(64); /*the length of the end code 256 must be larger than 0*/

												  /*now we've finally got HLIT and HDIST, so generate the code trees, and the function is done*/
		error = HuffmanTree_makeFromLengths(tree_ll, bitlen_ll, NUM_DEFLATE_CODE_SYMBOLS, 15);
		if (error) break;
		error = HuffmanTree_makeFromLengths(tree_d, bitlen_d, NUM_DISTANCE_SYMBOLS, 15);

		break; /*end of error-while*/
	}

	lodepng_free(bitlen_cl);
	lodepng_free(bitlen_ll);
	lodepng_free(bitlen_d);
	HuffmanTree_cleanup(&tree_cl);

	return error;
}

/*inflate a block with dynamic of fixed Huffman tree*/
static unsigned inflateHuffmanBlock(ucvector* out, const unsigned char* in, size_t* bp,
	size_t* pos, size_t inlength, unsigned btype)
{
	unsigned error = 0;
	HuffmanTree tree_ll; /*the huffman tree for literal and length codes*/
	HuffmanTree tree_d; /*the huffman tree for distance codes*/
	size_t inbitlength = inlength * 8;

	HuffmanTree_init(&tree_ll);
	HuffmanTree_init(&tree_d);

	if (btype == 1) getTreeInflateFixed(&tree_ll, &tree_d);
	else if (btype == 2) error = getTreeInflateDynamic(&tree_ll, &tree_d, in, bp, inlength);

	while (!error) /*decode all symbols until end reached, breaks at end code*/
	{
		/*code_ll is literal, length or end code*/
		unsigned code_ll = huffmanDecodeSymbol(in, bp, &tree_ll, inbitlength);
		if (code_ll <= 255) /*literal symbol*/
		{
			/*ucvector_push_back would do the same, but for some reason the two lines below run 10% faster*/
			if (!ucvector_resize(out, (*pos) + 1)) ERROR_BREAK(83 /*alloc fail*/);
			out->data[*pos] = (unsigned char)code_ll;
			++(*pos);
		}
		else if (code_ll >= FIRST_LENGTH_CODE_INDEX && code_ll <= LAST_LENGTH_CODE_INDEX) /*length code*/
		{
			unsigned code_d, distance;
			unsigned numextrabits_l, numextrabits_d; /*extra bits for length and distance*/
			size_t start, forward, backward, length;

			/*part 1: get length base*/
			length = LENGTHBASE[code_ll - FIRST_LENGTH_CODE_INDEX];

			/*part 2: get extra bits and add the value of that to length*/
			numextrabits_l = LENGTHEXTRA[code_ll - FIRST_LENGTH_CODE_INDEX];
			if ((*bp + numextrabits_l) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
			length += readBitsFromStream(bp, in, numextrabits_l);

			/*part 3: get distance code*/
			code_d = huffmanDecodeSymbol(in, bp, &tree_d, inbitlength);
			if (code_d > 29)
			{
				if (code_d == (unsigned)(-1)) /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
				{
					/*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
					(10=no endcode, 11=wrong jump outside of tree)*/
					error = (*bp) > inlength * 8 ? 10 : 11;
				}
				else error = 18; /*error: invalid distance code (30-31 are never used)*/
				break;
			}
			distance = DISTANCEBASE[code_d];

			/*part 4: get extra bits from distance*/
			numextrabits_d = DISTANCEEXTRA[code_d];
			if ((*bp + numextrabits_d) > inbitlength) ERROR_BREAK(51); /*error, bit pointer will jump past memory*/
			distance += readBitsFromStream(bp, in, numextrabits_d);

			/*part 5: fill in all the out[n] values based on the length and dist*/
			start = (*pos);
			if (distance > start) ERROR_BREAK(52); /*too long backward distance*/
			backward = start - distance;

			if (!ucvector_resize(out, (*pos) + length)) ERROR_BREAK(83 /*alloc fail*/);
			if (distance < length) {
				for (forward = 0; forward < length; ++forward)
				{
					out->data[(*pos)++] = out->data[backward++];
				}
			}
			else {
				memcpy(out->data + *pos, out->data + backward, length);
				*pos += length;
			}
		}
		else if (code_ll == 256)
		{
			break; /*end code, break the loop*/
		}
		else /*if(code == (unsigned)(-1))*/ /*huffmanDecodeSymbol returns (unsigned)(-1) in case of error*/
		{
			/*return error code 10 or 11 depending on the situation that happened in huffmanDecodeSymbol
			(10=no endcode, 11=wrong jump outside of tree)*/
			error = ((*bp) > inlength * 8) ? 10 : 11;
			break;
		}
	}

	HuffmanTree_cleanup(&tree_ll);
	HuffmanTree_cleanup(&tree_d);

	return error;
}

static unsigned inflateNoCompression(ucvector* out, const unsigned char* in, size_t* bp, size_t* pos, size_t inlength)
{
	size_t p;
	unsigned LEN, NLEN, n, error = 0;

	/*go to first boundary of byte*/
	while (((*bp) & 0x7) != 0) ++(*bp);
	p = (*bp) / 8; /*byte position*/

				   /*read LEN (2 bytes) and NLEN (2 bytes)*/
	if (p + 4 >= inlength) return 52; /*error, bit pointer will jump past memory*/
	LEN = in[p] + 256u * in[p + 1]; p += 2;
	NLEN = in[p] + 256u * in[p + 1]; p += 2;

	/*check if 16-bit NLEN is really the one's complement of LEN*/
	if (LEN + NLEN != 65535) return 21; /*error: NLEN is not one's complement of LEN*/

	if (!ucvector_resize(out, (*pos) + LEN)) return 83; /*alloc fail*/

														/*read the literal data: LEN bytes are now stored in the out buffer*/
	if (p + LEN > inlength) return 23; /*error: reading outside of in buffer*/
	for (n = 0; n < LEN; ++n) out->data[(*pos)++] = in[p++];

	(*bp) = p * 8;

	return error;
}

static unsigned lodepng_inflatev(ucvector* out,
	const unsigned char* in, size_t insize,
	const LodePNGDecompressSettings* settings)
{
	/*bit pointer in the "in" data, current byte is bp >> 3, current bit is bp & 0x7 (from lsb to msb of the byte)*/
	size_t bp = 0;
	unsigned BFINAL = 0;
	size_t pos = 0; /*byte position in the out buffer*/
	unsigned error = 0;

	(void)settings;

	while (!BFINAL)
	{
		unsigned BTYPE;
		if (bp + 2 >= insize * 8) return 52; /*error, bit pointer will jump past memory*/
		BFINAL = readBitFromStream(&bp, in);
		BTYPE = 1u * readBitFromStream(&bp, in);
		BTYPE += 2u * readBitFromStream(&bp, in);

		if (BTYPE == 3) return 20; /*error: invalid BTYPE*/
		else if (BTYPE == 0) error = inflateNoCompression(out, in, &bp, &pos, insize); /*no compression*/
		else error = inflateHuffmanBlock(out, in, &bp, &pos, insize, BTYPE); /*compression, BTYPE 01 or 10*/

		if (error) return error;
	}

	return error;
}

unsigned lodepng_inflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGDecompressSettings* settings)
{
	unsigned error;
	ucvector v;
	ucvector_init_buffer(&v, *out, *outsize);
	error = lodepng_inflatev(&v, in, insize, settings);
	*out = v.data;
	*outsize = v.size;
	return error;
}

static unsigned inflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGDecompressSettings* settings)
{
	if (settings->custom_inflate)
	{
		return settings->custom_inflate(out, outsize, in, insize, settings);
	}
	else
	{
		return lodepng_inflate(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflator (Compressor)                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */

static const size_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

/*bitlen is the size in bits of the code*/
static void addHuffmanSymbol(size_t* bp, ucvector* compressed, unsigned code, unsigned bitlen)
{
	addBitsToStreamReversed(bp, compressed, code, bitlen);
}

/*search the index in the array, that has the largest value smaller than or equal to the given value,
given array must be sorted (if no value is smaller, it returns the size of the given array)*/
static size_t searchCodeIndex(const unsigned* array, size_t array_size, size_t value)
{
	/*binary search (only small gain over linear). TODO: use CPU log2 instruction for getting symbols instead*/
	size_t left = 1;
	size_t right = array_size - 1;

	while (left <= right) {
		size_t mid = (left + right) >> 1;
		if (array[mid] >= value) right = mid - 1;
		else left = mid + 1;
	}
	if (left >= array_size || array[left] > value) left--;
	return left;
}

static void addLengthDistance(uivector* values, size_t length, size_t distance)
{
	/*values in encoded vector are those used by deflate:
	0-255: literal bytes
	256: end
	257-285: length/distance pair (length code, followed by extra length bits, distance code, extra distance bits)
	286-287: invalid*/

	unsigned length_code = (unsigned)searchCodeIndex(LENGTHBASE, 29, length);
	unsigned extra_length = (unsigned)(length - LENGTHBASE[length_code]);
	unsigned dist_code = (unsigned)searchCodeIndex(DISTANCEBASE, 30, distance);
	unsigned extra_distance = (unsigned)(distance - DISTANCEBASE[dist_code]);

	uivector_push_back(values, length_code + FIRST_LENGTH_CODE_INDEX);
	uivector_push_back(values, extra_length);
	uivector_push_back(values, dist_code);
	uivector_push_back(values, extra_distance);
}

/*3 bytes of data get encoded into two bytes. The hash cannot use more than 3
bytes as input because 3 is the minimum match length for deflate*/
static const unsigned HASH_NUM_VALUES = 65536;
static const unsigned HASH_BIT_MASK = 65535; /*HASH_NUM_VALUES - 1, but C90 does not like that as initializer*/

typedef struct Hash
{
	int* head; /*hash value to head circular pos - can be outdated if went around window*/
			   /*circular pos to prev circular pos*/
	unsigned short* chain;
	int* val; /*circular pos to hash value*/

			  /*TODO: do this not only for zeros but for any repeated byte. However for PNG
			  it's always going to be the zeros that dominate, so not important for PNG*/
	int* headz; /*similar to head, but for chainz*/
	unsigned short* chainz; /*those with same amount of zeros*/
	unsigned short* zeros; /*length of zeros streak, used as a second hash chain*/
} Hash;

static unsigned hash_init(Hash* hash, unsigned windowsize)
{
	unsigned i;
	hash->head = (int*)lodepng_malloc(sizeof(int) * HASH_NUM_VALUES);
	hash->val = (int*)lodepng_malloc(sizeof(int) * windowsize);
	hash->chain = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

	hash->zeros = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);
	hash->headz = (int*)lodepng_malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
	hash->chainz = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

	if (!hash->head || !hash->chain || !hash->val || !hash->headz || !hash->chainz || !hash->zeros)
	{
		return 83; /*alloc fail*/
	}

	/*initialize hash table*/
	for (i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->val[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->chain[i] = i; /*same value as index indicates uninitialized*/

	for (i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->chainz[i] = i; /*same value as index indicates uninitialized*/

	return 0;
}

static void hash_cleanup(Hash* hash)
{
	lodepng_free(hash->head);
	lodepng_free(hash->val);
	lodepng_free(hash->chain);

	lodepng_free(hash->zeros);
	lodepng_free(hash->headz);
	lodepng_free(hash->chainz);
}



static unsigned getHash(const unsigned char* data, size_t size, size_t pos)
{
	unsigned result = 0;
	if (pos + 2 < size)
	{
		/*A simple shift and xor hash is used. Since the data of PNGs is dominated
		by zeroes due to the filters, a better hash does not have a significant
		effect on speed in traversing the chain, and causes more time spend on
		calculating the hash.*/
		result ^= (unsigned)(data[pos + 0] << 0u);
		result ^= (unsigned)(data[pos + 1] << 4u);
		result ^= (unsigned)(data[pos + 2] << 8u);
	}
	else {
		size_t amount, i;
		if (pos >= size) return 0;
		amount = size - pos;
		for (i = 0; i != amount; ++i) result ^= (unsigned)(data[pos + i] << (i * 8u));
	}
	return result & HASH_BIT_MASK;
}

static unsigned countZeros(const unsigned char* data, size_t size, size_t pos)
{
	const unsigned char* start = data + pos;
	const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
	if (end > data + size) end = data + size;
	data = start;
	while (data != end && *data == 0) ++data;
	/*subtracting two addresses returned as 32-bit number (max value is MAX_SUPPORTED_DEFLATE_LENGTH)*/
	return (unsigned)(data - start);
}

/*wpos = pos & (windowsize - 1)*/
static void updateHashChain(Hash* hash, size_t wpos, unsigned hashval, unsigned short numzeros)
{
	hash->val[wpos] = (int)hashval;
	if (hash->head[hashval] != -1) hash->chain[wpos] = hash->head[hashval];
	hash->head[hashval] = (int)wpos;

	hash->zeros[wpos] = numzeros;
	if (hash->headz[numzeros] != -1) hash->chainz[wpos] = hash->headz[numzeros];
	hash->headz[numzeros] = (int)wpos;
}

/*
LZ77-encode the data. Return value is error code. The input are raw bytes, the output
is in the form of unsigned integers with codes representing for example literal bytes, or
length/distance pairs.
It uses a hash table technique to let it encode faster. When doing LZ77 encoding, a
sliding window (of windowsize) is used, and all past bytes in that window can be used as
the "dictionary". A brute force search through all possible distances would be slow, and
this hash technique is one out of several ways to speed this up.
*/
static unsigned encodeLZ77(uivector* out, Hash* hash,
	const unsigned char* in, size_t inpos, size_t insize, unsigned windowsize,
	unsigned minmatch, unsigned nicematch, unsigned lazymatching)
{
	size_t pos;
	unsigned i, error = 0;
	/*for large window lengths, assume the user wants no compression loss. Otherwise, max hash chain length speedup.*/
	unsigned maxchainlength = windowsize >= 8192 ? windowsize : windowsize / 8;
	unsigned maxlazymatch = windowsize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

	unsigned usezeros = 1; /*not sure if setting it to false for windowsize < 8192 is better or worse*/
	unsigned numzeros = 0;

	unsigned offset; /*the offset represents the distance in LZ77 terminology*/
	unsigned length;
	unsigned lazy = 0;
	unsigned lazylength = 0, lazyoffset = 0;
	unsigned hashval;
	unsigned current_offset, current_length;
	unsigned prev_offset;
	const unsigned char *lastptr, *foreptr, *backptr;
	unsigned hashpos;

	if (windowsize == 0 || windowsize > 32768) return 60; /*error: windowsize smaller/larger than allowed*/
	if ((windowsize & (windowsize - 1)) != 0) return 90; /*error: must be power of two*/

	if (nicematch > MAX_SUPPORTED_DEFLATE_LENGTH) nicematch = MAX_SUPPORTED_DEFLATE_LENGTH;

	for (pos = inpos; pos < insize; ++pos)
	{
		size_t wpos = pos & (windowsize - 1); /*position for in 'circular' hash buffers*/
		unsigned chainlength = 0;

		hashval = getHash(in, insize, pos);

		if (usezeros && hashval == 0)
		{
			if (numzeros == 0) numzeros = countZeros(in, insize, pos);
			else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
		}
		else
		{
			numzeros = 0;
		}

		updateHashChain(hash, wpos, hashval, numzeros);

		/*the length and offset found for the current position*/
		length = 0;
		offset = 0;

		hashpos = hash->chain[wpos];

		lastptr = &in[insize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? insize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

		/*search for the longest string*/
		prev_offset = 0;
		for (;;)
		{
			if (chainlength++ >= maxchainlength) break;
			current_offset = (unsigned)(hashpos <= wpos ? wpos - hashpos : wpos - hashpos + windowsize);

			if (current_offset < prev_offset) break; /*stop when went completely around the circular buffer*/
			prev_offset = current_offset;
			if (current_offset > 0)
			{
				/*test the next characters*/
				foreptr = &in[pos];
				backptr = &in[pos - current_offset];

				/*common case in PNGs is lots of zeros. Quickly skip over them as a speedup*/
				if (numzeros >= 3)
				{
					unsigned skip = hash->zeros[hashpos];
					if (skip > numzeros) skip = numzeros;
					backptr += skip;
					foreptr += skip;
				}

				while (foreptr != lastptr && *backptr == *foreptr) /*maximum supported length by deflate is max length*/
				{
					++backptr;
					++foreptr;
				}
				current_length = (unsigned)(foreptr - &in[pos]);

				if (current_length > length)
				{
					length = current_length; /*the longest length*/
					offset = current_offset; /*the offset that is related to this longest length*/
											 /*jump out once a length of max length is found (speed gain). This also jumps
											 out if length is MAX_SUPPORTED_DEFLATE_LENGTH*/
					if (current_length >= nicematch) break;
				}
			}

			if (hashpos == hash->chain[hashpos]) break;

			if (numzeros >= 3 && length > numzeros)
			{
				hashpos = hash->chainz[hashpos];
				if (hash->zeros[hashpos] != numzeros) break;
			}
			else
			{
				hashpos = hash->chain[hashpos];
				/*outdated hash value, happens if particular value was not encountered in whole last window*/
				if (hash->val[hashpos] != (int)hashval) break;
			}
		}

		if (lazymatching)
		{
			if (!lazy && length >= 3 && length <= maxlazymatch && length < MAX_SUPPORTED_DEFLATE_LENGTH)
			{
				lazy = 1;
				lazylength = length;
				lazyoffset = offset;
				continue; /*try the next byte*/
			}
			if (lazy)
			{
				lazy = 0;
				if (pos == 0) ERROR_BREAK(81);
				if (length > lazylength + 1)
				{
					/*push the previous character as literal*/
					if (!uivector_push_back(out, in[pos - 1])) ERROR_BREAK(83 /*alloc fail*/);
				}
				else
				{
					length = lazylength;
					offset = lazyoffset;
					hash->head[hashval] = -1; /*the same hashchain update will be done, this ensures no wrong alteration*/
					hash->headz[numzeros] = -1; /*idem*/
					--pos;
				}
			}
		}
		if (length >= 3 && offset > windowsize) ERROR_BREAK(86 /*too big (or overflown negative) offset*/);

		/*encode it as length/distance pair or literal value*/
		if (length < 3) /*only lengths of 3 or higher are supported as length/distance pair*/
		{
			if (!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
		}
		else if (length < minmatch || (length == 3 && offset > 4096))
		{
			/*compensate for the fact that longer offsets have more extra bits, a
			length of only 3 may be not worth it then*/
			if (!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
		}
		else
		{
			addLengthDistance(out, length, offset);
			for (i = 1; i < length; ++i)
			{
				++pos;
				wpos = pos & (windowsize - 1);
				hashval = getHash(in, insize, pos);
				if (usezeros && hashval == 0)
				{
					if (numzeros == 0) numzeros = countZeros(in, insize, pos);
					else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
				}
				else
				{
					numzeros = 0;
				}
				updateHashChain(hash, wpos, hashval, numzeros);
			}
		}
	} /*end of the loop through each character of input*/

	return error;
}

/* /////////////////////////////////////////////////////////////////////////// */

static unsigned deflateNoCompression(ucvector* out, const unsigned char* data, size_t datasize)
{
	/*non compressed deflate block data: 1 bit BFINAL,2 bits BTYPE,(5 bits): it jumps to start of next byte,
	2 bytes LEN, 2 bytes NLEN, LEN bytes literal DATA*/

	size_t i, j, numdeflateblocks = (datasize + 65534) / 65535;
	unsigned datapos = 0;
	for (i = 0; i != numdeflateblocks; ++i)
	{
		unsigned BFINAL, BTYPE, LEN, NLEN;
		unsigned char firstbyte;

		BFINAL = (i == numdeflateblocks - 1);
		BTYPE = 0;

		firstbyte = (unsigned char)(BFINAL + ((BTYPE & 1) << 1) + ((BTYPE & 2) << 1));
		ucvector_push_back(out, firstbyte);

		LEN = 65535;
		if (datasize - datapos < 65535) LEN = (unsigned)datasize - datapos;
		NLEN = 65535 - LEN;

		ucvector_push_back(out, (unsigned char)(LEN & 255));
		ucvector_push_back(out, (unsigned char)(LEN >> 8));
		ucvector_push_back(out, (unsigned char)(NLEN & 255));
		ucvector_push_back(out, (unsigned char)(NLEN >> 8));

		/*Decompressed data*/
		for (j = 0; j < 65535 && datapos < datasize; ++j)
		{
			ucvector_push_back(out, data[datapos++]);
		}
	}

	return 0;
}

/*
write the lz77-encoded data, which has lit, len and dist codes, to compressed stream using huffman trees.
tree_ll: the tree for lit and len codes.
tree_d: the tree for distance codes.
*/
static void writeLZ77data(size_t* bp, ucvector* out, const uivector* lz77_encoded,
	const HuffmanTree* tree_ll, const HuffmanTree* tree_d)
{
	size_t i = 0;
	for (i = 0; i != lz77_encoded->size; ++i)
	{
		unsigned val = lz77_encoded->data[i];
		addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_ll, val), HuffmanTree_getLength(tree_ll, val));
		if (val > 256) /*for a length code, 3 more things have to be added*/
		{
			unsigned length_index = val - FIRST_LENGTH_CODE_INDEX;
			unsigned n_length_extra_bits = LENGTHEXTRA[length_index];
			unsigned length_extra_bits = lz77_encoded->data[++i];

			unsigned distance_code = lz77_encoded->data[++i];

			unsigned distance_index = distance_code;
			unsigned n_distance_extra_bits = DISTANCEEXTRA[distance_index];
			unsigned distance_extra_bits = lz77_encoded->data[++i];

			addBitsToStream(bp, out, length_extra_bits, n_length_extra_bits);
			addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_d, distance_code),
				HuffmanTree_getLength(tree_d, distance_code));
			addBitsToStream(bp, out, distance_extra_bits, n_distance_extra_bits);
		}
	}
}

/*Deflate for a block of type "dynamic", that is, with freely, optimally, created huffman trees*/
static unsigned deflateDynamic(ucvector* out, size_t* bp, Hash* hash,
	const unsigned char* data, size_t datapos, size_t dataend,
	const LodePNGCompressSettings* settings, unsigned final)
{
	unsigned error = 0;

	/*
	A block is compressed as follows: The PNG data is lz77 encoded, resulting in
	literal bytes and length/distance pairs. This is then huffman compressed with
	two huffman trees. One huffman tree is used for the lit and len values ("ll"),
	another huffman tree is used for the dist values ("d"). These two trees are
	stored using their code lengths, and to compress even more these code lengths
	are also run-length encoded and huffman compressed. This gives a huffman tree
	of code lengths "cl". The code lenghts used to describe this third tree are
	the code length code lengths ("clcl").
	*/

	/*The lz77 encoded data, represented with integers since there will also be length and distance codes in it*/
	uivector lz77_encoded;
	HuffmanTree tree_ll; /*tree for lit,len values*/
	HuffmanTree tree_d; /*tree for distance codes*/
	HuffmanTree tree_cl; /*tree for encoding the code lengths representing tree_ll and tree_d*/
	uivector frequencies_ll; /*frequency of lit,len codes*/
	uivector frequencies_d; /*frequency of dist codes*/
	uivector frequencies_cl; /*frequency of code length codes*/
	uivector bitlen_lld; /*lit,len,dist code lenghts (int bits), literally (without repeat codes).*/
	uivector bitlen_lld_e; /*bitlen_lld encoded with repeat codes (this is a rudemtary run length compression)*/
						   /*bitlen_cl is the code length code lengths ("clcl"). The bit lengths of codes to represent tree_cl
						   (these are written as is in the file, it would be crazy to compress these using yet another huffman
						   tree that needs to be represented by yet another set of code lengths)*/
	uivector bitlen_cl;
	size_t datasize = dataend - datapos;

	/*
	Due to the huffman compression of huffman tree representations ("two levels"), there are some anologies:
	bitlen_lld is to tree_cl what data is to tree_ll and tree_d.
	bitlen_lld_e is to bitlen_lld what lz77_encoded is to data.
	bitlen_cl is to bitlen_lld_e what bitlen_lld is to lz77_encoded.
	*/

	unsigned BFINAL = final;
	size_t numcodes_ll, numcodes_d, i;
	unsigned HLIT, HDIST, HCLEN;

	uivector_init(&lz77_encoded);
	HuffmanTree_init(&tree_ll);
	HuffmanTree_init(&tree_d);
	HuffmanTree_init(&tree_cl);
	uivector_init(&frequencies_ll);
	uivector_init(&frequencies_d);
	uivector_init(&frequencies_cl);
	uivector_init(&bitlen_lld);
	uivector_init(&bitlen_lld_e);
	uivector_init(&bitlen_cl);

	/*This while loop never loops due to a break at the end, it is here to
	allow breaking out of it to the cleanup phase on error conditions.*/
	while (!error)
	{
		if (settings->use_lz77)
		{
			error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
				settings->minmatch, settings->nicematch, settings->lazymatching);
			if (error) break;
		}
		else
		{
			if (!uivector_resize(&lz77_encoded, datasize)) ERROR_BREAK(83 /*alloc fail*/);
			for (i = datapos; i < dataend; ++i) lz77_encoded.data[i - datapos] = data[i]; /*no LZ77, but still will be Huffman compressed*/
		}

		if (!uivector_resizev(&frequencies_ll, 286, 0)) ERROR_BREAK(83 /*alloc fail*/);
		if (!uivector_resizev(&frequencies_d, 30, 0)) ERROR_BREAK(83 /*alloc fail*/);

		/*Count the frequencies of lit, len and dist codes*/
		for (i = 0; i != lz77_encoded.size; ++i)
		{
			unsigned symbol = lz77_encoded.data[i];
			++frequencies_ll.data[symbol];
			if (symbol > 256)
			{
				unsigned dist = lz77_encoded.data[i + 2];
				++frequencies_d.data[dist];
				i += 3;
			}
		}
		frequencies_ll.data[256] = 1; /*there will be exactly 1 end code, at the end of the block*/

									  /*Make both huffman trees, one for the lit and len codes, one for the dist codes*/
		error = HuffmanTree_makeFromFrequencies(&tree_ll, frequencies_ll.data, 257, frequencies_ll.size, 15);
		if (error) break;
		/*2, not 1, is chosen for mincodes: some buggy PNG decoders require at least 2 symbols in the dist tree*/
		error = HuffmanTree_makeFromFrequencies(&tree_d, frequencies_d.data, 2, frequencies_d.size, 15);
		if (error) break;

		numcodes_ll = tree_ll.numcodes; if (numcodes_ll > 286) numcodes_ll = 286;
		numcodes_d = tree_d.numcodes; if (numcodes_d > 30) numcodes_d = 30;
		/*store the code lengths of both generated trees in bitlen_lld*/
		for (i = 0; i != numcodes_ll; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_ll, (unsigned)i));
		for (i = 0; i != numcodes_d; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_d, (unsigned)i));

		/*run-length compress bitlen_ldd into bitlen_lld_e by using repeat codes 16 (copy length 3-6 times),
		17 (3-10 zeroes), 18 (11-138 zeroes)*/
		for (i = 0; i != (unsigned)bitlen_lld.size; ++i)
		{
			unsigned j = 0; /*amount of repititions*/
			while (i + j + 1 < (unsigned)bitlen_lld.size && bitlen_lld.data[i + j + 1] == bitlen_lld.data[i]) ++j;

			if (bitlen_lld.data[i] == 0 && j >= 2) /*repeat code for zeroes*/
			{
				++j; /*include the first zero*/
				if (j <= 10) /*repeat code 17 supports max 10 zeroes*/
				{
					uivector_push_back(&bitlen_lld_e, 17);
					uivector_push_back(&bitlen_lld_e, j - 3);
				}
				else /*repeat code 18 supports max 138 zeroes*/
				{
					if (j > 138) j = 138;
					uivector_push_back(&bitlen_lld_e, 18);
					uivector_push_back(&bitlen_lld_e, j - 11);
				}
				i += (j - 1);
			}
			else if (j >= 3) /*repeat code for value other than zero*/
			{
				size_t k;
				unsigned num = j / 6, rest = j % 6;
				uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
				for (k = 0; k < num; ++k)
				{
					uivector_push_back(&bitlen_lld_e, 16);
					uivector_push_back(&bitlen_lld_e, 6 - 3);
				}
				if (rest >= 3)
				{
					uivector_push_back(&bitlen_lld_e, 16);
					uivector_push_back(&bitlen_lld_e, rest - 3);
				}
				else j -= rest;
				i += j;
			}
			else /*too short to benefit from repeat code*/
			{
				uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
			}
		}

		/*generate tree_cl, the huffmantree of huffmantrees*/

		if (!uivector_resizev(&frequencies_cl, NUM_CODE_LENGTH_CODES, 0)) ERROR_BREAK(83 /*alloc fail*/);
		for (i = 0; i != bitlen_lld_e.size; ++i)
		{
			++frequencies_cl.data[bitlen_lld_e.data[i]];
			/*after a repeat code come the bits that specify the number of repetitions,
			those don't need to be in the frequencies_cl calculation*/
			if (bitlen_lld_e.data[i] >= 16) ++i;
		}

		error = HuffmanTree_makeFromFrequencies(&tree_cl, frequencies_cl.data,
			frequencies_cl.size, frequencies_cl.size, 7);
		if (error) break;

		if (!uivector_resize(&bitlen_cl, tree_cl.numcodes)) ERROR_BREAK(83 /*alloc fail*/);
		for (i = 0; i != tree_cl.numcodes; ++i)
		{
			/*lenghts of code length tree is in the order as specified by deflate*/
			bitlen_cl.data[i] = HuffmanTree_getLength(&tree_cl, CLCL_ORDER[i]);
		}
		while (bitlen_cl.data[bitlen_cl.size - 1] == 0 && bitlen_cl.size > 4)
		{
			/*remove zeros at the end, but minimum size must be 4*/
			if (!uivector_resize(&bitlen_cl, bitlen_cl.size - 1)) ERROR_BREAK(83 /*alloc fail*/);
		}
		if (error) break;

		/*
		Write everything into the output

		After the BFINAL and BTYPE, the dynamic block consists out of the following:
		- 5 bits HLIT, 5 bits HDIST, 4 bits HCLEN
		- (HCLEN+4)*3 bits code lengths of code length alphabet
		- HLIT + 257 code lenghts of lit/length alphabet (encoded using the code length
		alphabet, + possible repetition codes 16, 17, 18)
		- HDIST + 1 code lengths of distance alphabet (encoded using the code length
		alphabet, + possible repetition codes 16, 17, 18)
		- compressed data
		- 256 (end code)
		*/

		/*Write block type*/
		addBitToStream(bp, out, BFINAL);
		addBitToStream(bp, out, 0); /*first bit of BTYPE "dynamic"*/
		addBitToStream(bp, out, 1); /*second bit of BTYPE "dynamic"*/

									/*write the HLIT, HDIST and HCLEN values*/
		HLIT = (unsigned)(numcodes_ll - 257);
		HDIST = (unsigned)(numcodes_d - 1);
		HCLEN = (unsigned)bitlen_cl.size - 4;
		/*trim zeroes for HCLEN. HLIT and HDIST were already trimmed at tree creation*/
		while (!bitlen_cl.data[HCLEN + 4 - 1] && HCLEN > 0) --HCLEN;
		addBitsToStream(bp, out, HLIT, 5);
		addBitsToStream(bp, out, HDIST, 5);
		addBitsToStream(bp, out, HCLEN, 4);

		/*write the code lenghts of the code length alphabet*/
		for (i = 0; i != HCLEN + 4; ++i) addBitsToStream(bp, out, bitlen_cl.data[i], 3);

		/*write the lenghts of the lit/len AND the dist alphabet*/
		for (i = 0; i != bitlen_lld_e.size; ++i)
		{
			addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_cl, bitlen_lld_e.data[i]),
				HuffmanTree_getLength(&tree_cl, bitlen_lld_e.data[i]));
			/*extra bits of repeat codes*/
			if (bitlen_lld_e.data[i] == 16) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 2);
			else if (bitlen_lld_e.data[i] == 17) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 3);
			else if (bitlen_lld_e.data[i] == 18) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 7);
		}

		/*write the compressed data symbols*/
		writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
		/*error: the length of the end code 256 must be larger than 0*/
		if (HuffmanTree_getLength(&tree_ll, 256) == 0) ERROR_BREAK(64);

		/*write the end code*/
		addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

		break; /*end of error-while*/
	}

	/*cleanup*/
	uivector_cleanup(&lz77_encoded);
	HuffmanTree_cleanup(&tree_ll);
	HuffmanTree_cleanup(&tree_d);
	HuffmanTree_cleanup(&tree_cl);
	uivector_cleanup(&frequencies_ll);
	uivector_cleanup(&frequencies_d);
	uivector_cleanup(&frequencies_cl);
	uivector_cleanup(&bitlen_lld_e);
	uivector_cleanup(&bitlen_lld);
	uivector_cleanup(&bitlen_cl);

	return error;
}

static unsigned deflateFixed(ucvector* out, size_t* bp, Hash* hash,
	const unsigned char* data,
	size_t datapos, size_t dataend,
	const LodePNGCompressSettings* settings, unsigned final)
{
	HuffmanTree tree_ll; /*tree for literal values and length codes*/
	HuffmanTree tree_d; /*tree for distance codes*/

	unsigned BFINAL = final;
	unsigned error = 0;
	size_t i;

	HuffmanTree_init(&tree_ll);
	HuffmanTree_init(&tree_d);

	generateFixedLitLenTree(&tree_ll);
	generateFixedDistanceTree(&tree_d);

	addBitToStream(bp, out, BFINAL);
	addBitToStream(bp, out, 1); /*first bit of BTYPE*/
	addBitToStream(bp, out, 0); /*second bit of BTYPE*/

	if (settings->use_lz77) /*LZ77 encoded*/
	{
		uivector lz77_encoded;
		uivector_init(&lz77_encoded);
		error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
			settings->minmatch, settings->nicematch, settings->lazymatching);
		if (!error) writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
		uivector_cleanup(&lz77_encoded);
	}
	else /*no LZ77, but still will be Huffman compressed*/
	{
		for (i = datapos; i < dataend; ++i)
		{
			addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, data[i]), HuffmanTree_getLength(&tree_ll, data[i]));
		}
	}
	/*add END code*/
	if (!error) addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

	/*cleanup*/
	HuffmanTree_cleanup(&tree_ll);
	HuffmanTree_cleanup(&tree_d);

	return error;
}

static unsigned lodepng_deflatev(ucvector* out, const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings)
{
	unsigned error = 0;
	size_t i, blocksize, numdeflateblocks;
	size_t bp = 0; /*the bit pointer*/
	Hash hash;

	if (settings->btype > 2) return 61;
	else if (settings->btype == 0) return deflateNoCompression(out, in, insize);
	else if (settings->btype == 1) blocksize = insize;
	else /*if(settings->btype == 2)*/
	{
		/*on PNGs, deflate blocks of 65-262k seem to give most dense encoding*/
		blocksize = insize / 8 + 8;
		if (blocksize < 65536) blocksize = 65536;
		if (blocksize > 262144) blocksize = 262144;
	}

	numdeflateblocks = (insize + blocksize - 1) / blocksize;
	if (numdeflateblocks == 0) numdeflateblocks = 1;

	error = hash_init(&hash, settings->windowsize);
	if (error) return error;

	for (i = 0; i != numdeflateblocks && !error; ++i)
	{
		unsigned final = (i == numdeflateblocks - 1);
		size_t start = i * blocksize;
		size_t end = start + blocksize;
		if (end > insize) end = insize;

		if (settings->btype == 1) error = deflateFixed(out, &bp, &hash, in, start, end, settings, final);
		else if (settings->btype == 2) error = deflateDynamic(out, &bp, &hash, in, start, end, settings, final);
	}

	hash_cleanup(&hash);

	return error;
}

unsigned lodepng_deflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings)
{
	unsigned error;
	ucvector v;
	ucvector_init_buffer(&v, *out, *outsize);
	error = lodepng_deflatev(&v, in, insize, settings);
	*out = v.data;
	*outsize = v.size;
	return error;
}

static unsigned deflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings)
{
	if (settings->custom_deflate)
	{
		return settings->custom_deflate(out, outsize, in, insize, settings);
	}
	else
	{
		return lodepng_deflate(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Adler32                                                                  */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned update_adler32(unsigned adler, const unsigned char* data, unsigned len)
{
	unsigned s1 = adler & 0xffff;
	unsigned s2 = (adler >> 16) & 0xffff;

	while (len > 0)
	{
		/*at least 5552 sums can be done before the sums overflow, saving a lot of module divisions*/
		unsigned amount = len > 5552 ? 5552 : len;
		len -= amount;
		while (amount > 0)
		{
			s1 += (*data++);
			s2 += s1;
			--amount;
		}
		s1 %= 65521;
		s2 %= 65521;
	}

	return (s2 << 16) | s1;
}

/*Return the adler32 of the bytes data[0..len-1]*/
static unsigned adler32(const unsigned char* data, unsigned len)
{
	return update_adler32(1L, data, len);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Zlib                                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_DECODER

unsigned lodepng_zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGDecompressSettings* settings)
{
	unsigned error = 0;
	unsigned CM, CINFO, FDICT;

	if (insize < 2) return 53; /*error, size of zlib data too small*/
							   /*read information from zlib header*/
	if ((in[0] * 256 + in[1]) % 31 != 0)
	{
		/*error: 256 * in[0] + in[1] must be a multiple of 31, the FCHECK value is supposed to be made that way*/
		return 24;
	}

	CM = in[0] & 15;
	CINFO = (in[0] >> 4) & 15;
	/*FCHECK = in[1] & 31;*/ /*FCHECK is already tested above*/
	FDICT = (in[1] >> 5) & 1;
	/*FLEVEL = (in[1] >> 6) & 3;*/ /*FLEVEL is not used here*/

	if (CM != 8 || CINFO > 7)
	{
		/*error: only compression method 8: inflate with sliding window of 32k is supported by the PNG spec*/
		return 25;
	}
	if (FDICT != 0)
	{
		/*error: the specification of PNG says about the zlib stream:
		"The additional flags shall not specify a preset dictionary."*/
		return 26;
	}

	error = inflate(out, outsize, in + 2, insize - 2, settings);
	if (error) return error;

	if (!settings->ignore_adler32)
	{
		unsigned ADLER32 = lodepng_read32bitInt(&in[insize - 4]);
		unsigned checksum = adler32(*out, (unsigned)(*outsize));
		if (checksum != ADLER32) return 58; /*error, adler checksum not correct, data must be corrupted*/
	}

	return 0; /*no error*/
}

static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGDecompressSettings* settings)
{
	if (settings->custom_zlib)
	{
		return settings->custom_zlib(out, outsize, in, insize, settings);
	}
	else
	{
		return lodepng_zlib_decompress(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

unsigned lodepng_zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGCompressSettings* settings)
{
	/*initially, *out must be NULL and outsize 0, if you just give some random *out
	that's pointing to a non allocated buffer, this'll crash*/
	ucvector outv;
	size_t i;
	unsigned error;
	unsigned char* deflatedata = 0;
	size_t deflatesize = 0;

	/*zlib data: 1 byte CMF (CM+CINFO), 1 byte FLG, deflate data, 4 byte ADLER32 checksum of the Decompressed data*/
	unsigned CMF = 120; /*0b01111000: CM 8, CINFO 7. With CINFO 7, any window size up to 32768 can be used.*/
	unsigned FLEVEL = 0;
	unsigned FDICT = 0;
	unsigned CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
	unsigned FCHECK = 31 - CMFFLG % 31;
	CMFFLG += FCHECK;

	/*ucvector-controlled version of the output buffer, for dynamic array*/
	ucvector_init_buffer(&outv, *out, *outsize);

	ucvector_push_back(&outv, (unsigned char)(CMFFLG >> 8));
	ucvector_push_back(&outv, (unsigned char)(CMFFLG & 255));

	error = deflate(&deflatedata, &deflatesize, in, insize, settings);

	if (!error)
	{
		unsigned ADLER32 = adler32(in, (unsigned)insize);
		for (i = 0; i != deflatesize; ++i) ucvector_push_back(&outv, deflatedata[i]);
		lodepng_free(deflatedata);
		lodepng_add32bitInt(&outv, ADLER32);
	}

	*out = outv.data;
	*outsize = outv.size;

	return error;
}

/* compress using the default or custom zlib function */
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGCompressSettings* settings)
{
	if (settings->custom_zlib)
	{
		return settings->custom_zlib(out, outsize, in, insize, settings);
	}
	else
	{
		return lodepng_zlib_compress(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_ENCODER*/

#else /*no LODEPNG_COMPILE_ZLIB*/

#ifdef LODEPNG_COMPILE_DECODER
static unsigned zlib_decompress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGDecompressSettings* settings)
{
	if (!settings->custom_zlib) return 87; /*no custom zlib function provided */
	return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGCompressSettings* settings)
{
	if (!settings->custom_zlib) return 87; /*no custom zlib function provided */
	return settings->custom_zlib(out, outsize, in, insize, settings);
}
#endif /*LODEPNG_COMPILE_ENCODER*/

#endif /*LODEPNG_COMPILE_ZLIB*/

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ENCODER

/*this is a good tradeoff between speed and compression ratio*/
#define DEFAULT_WINDOWSIZE 2048

void lodepng_compress_settings_init(LodePNGCompressSettings* settings)
{
	/*compress with dynamic huffman tree (not in the mathematical sense, just not the predefined one)*/
	settings->btype = 2;
	settings->use_lz77 = 1;
	settings->windowsize = DEFAULT_WINDOWSIZE;
	settings->minmatch = 3;
	settings->nicematch = 128;
	settings->lazymatching = 1;

	settings->custom_zlib = 0;
	settings->custom_deflate = 0;
	settings->custom_context = 0;
}

const LodePNGCompressSettings lodepng_default_compress_settings = { 2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0 };


#endif /*LODEPNG_COMPILE_ENCODER*/

#ifdef LODEPNG_COMPILE_DECODER

void lodepng_decompress_settings_init(LodePNGDecompressSettings* settings)
{
	settings->ignore_adler32 = 0;

	settings->custom_zlib = 0;
	settings->custom_inflate = 0;
	settings->custom_context = 0;
}

const LodePNGDecompressSettings lodepng_default_decompress_settings = { 0, 0, 0, 0 };

#endif /*LODEPNG_COMPILE_DECODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of Zlib related code. Begin of PNG related code.                 // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG

/* ////////////////////////////////////////////////////////////////////////// */
/* / CRC32                                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */


#ifndef LODEPNG_NO_COMPILE_CRC
/* CRC polynomial: 0xedb88320 */
static unsigned lodepng_crc32_table[256] = {
	0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
	249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
	498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
	325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
	997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
	901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
	651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
	671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
	1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
	2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
	1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
	1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
	1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
	1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
	1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
	1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
	3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
	3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
	4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
	4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
	3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
	3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
	3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
	3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
	2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
	2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
	2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
	2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
	2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
	2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
	3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
	3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

/*Return the CRC of the bytes buf[0..len-1].*/
unsigned lodepng_crc32(const unsigned char* data, size_t length)
{
	unsigned r = 0xffffffffu;
	size_t i;
	for (i = 0; i < length; ++i)
	{
		r = lodepng_crc32_table[(r ^ data[i]) & 0xff] ^ (r >> 8);
	}
	return r ^ 0xffffffffu;
}
#else /* !LODEPNG_NO_COMPILE_CRC */
unsigned lodepng_crc32(const unsigned char* data, size_t length);
#endif /* !LODEPNG_NO_COMPILE_CRC */

/* ////////////////////////////////////////////////////////////////////////// */
/* / Reading and writing single bits and bytes from/to stream for LodePNG   / */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned char readBitFromReversedStream(size_t* bitpointer, const unsigned char* bitstream)
{
	unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
	++(*bitpointer);
	return result;
}

static unsigned readBitsFromReversedStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
	unsigned result = 0;
	size_t i;
	for (i = 0; i < nbits; ++i)
	{
		result <<= 1;
		result |= (unsigned)readBitFromReversedStream(bitpointer, bitstream);
	}
	return result;
}

#ifdef LODEPNG_COMPILE_DECODER
static void setBitOfReversedStream0(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
	/*the current bit in bitstream must be 0 for this to work*/
	if (bit)
	{
		/*earlier bit of huffman code is in a lesser significant bit of an earlier byte*/
		bitstream[(*bitpointer) >> 3] |= (bit << (7 - ((*bitpointer) & 0x7)));
	}
	++(*bitpointer);
}
#endif /*LODEPNG_COMPILE_DECODER*/

static void setBitOfReversedStream(size_t* bitpointer, unsigned char* bitstream, unsigned char bit)
{
	/*the current bit in bitstream may be 0 or 1 for this to work*/
	if (bit == 0) bitstream[(*bitpointer) >> 3] &= (unsigned char)(~(1 << (7 - ((*bitpointer) & 0x7))));
	else         bitstream[(*bitpointer) >> 3] |= (1 << (7 - ((*bitpointer) & 0x7)));
	++(*bitpointer);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG chunks                                                             / */
/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_chunk_length(const unsigned char* chunk)
{
	return lodepng_read32bitInt(&chunk[0]);
}

void lodepng_chunk_type(char type[5], const unsigned char* chunk)
{
	unsigned i;
	for (i = 0; i != 4; ++i) type[i] = (char)chunk[4 + i];
	type[4] = 0; /*null termination char*/
}

unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type)
{
	if (strlen(type) != 4) return 0;
	return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

unsigned char lodepng_chunk_ancillary(const unsigned char* chunk)
{
	return((chunk[4] & 32) != 0);
}

unsigned char lodepng_chunk_private(const unsigned char* chunk)
{
	return((chunk[6] & 32) != 0);
}

unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk)
{
	return((chunk[7] & 32) != 0);
}

unsigned char* lodepng_chunk_data(unsigned char* chunk)
{
	return &chunk[8];
}

const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk)
{
	return &chunk[8];
}

unsigned lodepng_chunk_check_crc(const unsigned char* chunk)
{
	unsigned length = lodepng_chunk_length(chunk);
	unsigned CRC = lodepng_read32bitInt(&chunk[length + 8]);
	/*the CRC is taken of the data and the 4 chunk type letters, not the length*/
	unsigned checksum = lodepng_crc32(&chunk[4], length + 4);
	if (CRC != checksum) return 1;
	else return 0;
}

void lodepng_chunk_generate_crc(unsigned char* chunk)
{
	unsigned length = lodepng_chunk_length(chunk);
	unsigned CRC = lodepng_crc32(&chunk[4], length + 4);
	lodepng_set32bitInt(chunk + 8 + length, CRC);
}

unsigned char* lodepng_chunk_next(unsigned char* chunk)
{
	unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
	return chunk + total_chunk_length;
}

const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk)
{
	unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
	return chunk + total_chunk_length;
}

unsigned lodepng_chunk_append(unsigned char** out, size_t* outlength, const unsigned char* chunk)
{
	unsigned i;
	unsigned total_chunk_length = lodepng_chunk_length(chunk) + 12;
	unsigned char *chunk_start, *new_buffer;
	size_t new_length = (*outlength) + total_chunk_length;
	if (new_length < total_chunk_length || new_length < (*outlength)) return 77; /*integer overflow happened*/

	new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
	if (!new_buffer) return 83; /*alloc fail*/
	(*out) = new_buffer;
	(*outlength) = new_length;
	chunk_start = &(*out)[new_length - total_chunk_length];

	for (i = 0; i != total_chunk_length; ++i) chunk_start[i] = chunk[i];

	return 0;
}

unsigned lodepng_chunk_create(unsigned char** out, size_t* outlength, unsigned length,
	const char* type, const unsigned char* data)
{
	unsigned i;
	unsigned char *chunk, *new_buffer;
	size_t new_length = (*outlength) + length + 12;
	if (new_length < length + 12 || new_length < (*outlength)) return 77; /*integer overflow happened*/
	new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
	if (!new_buffer) return 83; /*alloc fail*/
	(*out) = new_buffer;
	(*outlength) = new_length;
	chunk = &(*out)[(*outlength) - length - 12];

	/*1: length*/
	lodepng_set32bitInt(chunk, (unsigned)length);

	/*2: chunk name (4 letters)*/
	chunk[4] = (unsigned char)type[0];
	chunk[5] = (unsigned char)type[1];
	chunk[6] = (unsigned char)type[2];
	chunk[7] = (unsigned char)type[3];

	/*3: the data*/
	for (i = 0; i != length; ++i) chunk[8 + i] = data[i];

	/*4: CRC (of the chunkname characters and the data)*/
	lodepng_chunk_generate_crc(chunk);

	return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Color types and such                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */

/*return type is a LodePNG error code*/
static unsigned checkColorValidity(LodePNGColorType colortype, unsigned bd) /*bd = bitdepth*/
{
	switch (colortype)
	{
	case 0: if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break; /*grey*/
	case 2: if (!(bd == 8 || bd == 16)) return 37; break; /*RGB*/
	case 3: if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8)) return 37; break; /*palette*/
	case 4: if (!(bd == 8 || bd == 16)) return 37; break; /*grey + alpha*/
	case 6: if (!(bd == 8 || bd == 16)) return 37; break; /*RGBA*/
	default: return 31;
	}
	return 0; /*allowed color type / bits combination*/
}

static unsigned getNumColorChannels(LodePNGColorType colortype)
{
	switch (colortype)
	{
	case 0: return 1; /*grey*/
	case 2: return 3; /*RGB*/
	case 3: return 1; /*palette*/
	case 4: return 2; /*grey + alpha*/
	case 6: return 4; /*RGBA*/
	}
	return 0; /*unexisting color type*/
}

static unsigned lodepng_get_bpp_lct(LodePNGColorType colortype, unsigned bitdepth)
{
	/*bits per pixel is amount of channels * bits per channel*/
	return getNumColorChannels(colortype) * bitdepth;
}

/* ////////////////////////////////////////////////////////////////////////// */

void lodepng_color_mode_init(LodePNGColorMode* info)
{
	info->key_defined = 0;
	info->key_r = info->key_g = info->key_b = 0;
	info->colortype = LCT_RGBA;
	info->bitdepth = 8;
	info->palette = 0;
	info->palettesize = 0;
}

void lodepng_color_mode_cleanup(LodePNGColorMode* info)
{
	lodepng_palette_clear(info);
}

unsigned lodepng_color_mode_copy(LodePNGColorMode* dest, const LodePNGColorMode* source)
{
	size_t i;
	lodepng_color_mode_cleanup(dest);
	*dest = *source;
	if (source->palette)
	{
		dest->palette = (unsigned char*)lodepng_malloc(1024);
		if (!dest->palette && source->palettesize) return 83; /*alloc fail*/
		for (i = 0; i != source->palettesize * 4; ++i) dest->palette[i] = source->palette[i];
	}
	return 0;
}

static int lodepng_color_mode_equal(const LodePNGColorMode* a, const LodePNGColorMode* b)
{
	size_t i;
	if (a->colortype != b->colortype) return 0;
	if (a->bitdepth != b->bitdepth) return 0;
	if (a->key_defined != b->key_defined) return 0;
	if (a->key_defined)
	{
		if (a->key_r != b->key_r) return 0;
		if (a->key_g != b->key_g) return 0;
		if (a->key_b != b->key_b) return 0;
	}
	if (a->palettesize != b->palettesize) return 0;
	for (i = 0; i != a->palettesize * 4; ++i)
	{
		if (a->palette[i] != b->palette[i]) return 0;
	}
	return 1;
}

#ifdef LODEPNG_COMPILE_ENCODER
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/* Makes a temporary LodePNGColorMode that does not need cleanup (no palette) */
static LodePNGColorMode lodepng_color_mode_make(LodePNGColorType colortype, unsigned bitdepth)
{
	LodePNGColorMode result;
	lodepng_color_mode_init(&result);
	result.colortype = colortype;
	result.bitdepth = bitdepth;
	return result;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#endif /*LODEPNG_COMPILE_ENCODER*/

void lodepng_palette_clear(LodePNGColorMode* info)
{
	if (info->palette) lodepng_free(info->palette);
	info->palette = 0;
	info->palettesize = 0;
}

unsigned lodepng_palette_add(LodePNGColorMode* info,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	unsigned char* data;
	/*the same resize technique as C++ std::vectors is used, and here it's made so that for a palette with
	the max of 256 colors, it'll have the exact alloc size*/
	if (!info->palette) /*allocate palette if empty*/
	{
		/*room for 256 colors with 4 bytes each*/
		data = (unsigned char*)lodepng_realloc(info->palette, 1024);
		if (!data) return 83; /*alloc fail*/
		else info->palette = data;
	}
	info->palette[4 * info->palettesize + 0] = r;
	info->palette[4 * info->palettesize + 1] = g;
	info->palette[4 * info->palettesize + 2] = b;
	info->palette[4 * info->palettesize + 3] = a;
	++info->palettesize;
	return 0;
}

/*calculate bits per pixel out of colortype and bitdepth*/
unsigned lodepng_get_bpp(const LodePNGColorMode* info)
{
	return lodepng_get_bpp_lct(info->colortype, info->bitdepth);
}

unsigned lodepng_get_channels(const LodePNGColorMode* info)
{
	return getNumColorChannels(info->colortype);
}

unsigned lodepng_is_greyscale_type(const LodePNGColorMode* info)
{
	return info->colortype == LCT_GREY || info->colortype == LCT_GREY_ALPHA;
}

unsigned lodepng_is_alpha_type(const LodePNGColorMode* info)
{
	return (info->colortype & 4) != 0; /*4 or 6*/
}

unsigned lodepng_is_palette_type(const LodePNGColorMode* info)
{
	return info->colortype == LCT_PALETTE;
}

unsigned lodepng_has_palette_alpha(const LodePNGColorMode* info)
{
	size_t i;
	for (i = 0; i != info->palettesize; ++i)
	{
		if (info->palette[i * 4 + 3] < 255) return 1;
	}
	return 0;
}

unsigned lodepng_can_have_alpha(const LodePNGColorMode* info)
{
	return info->key_defined
		|| lodepng_is_alpha_type(info)
		|| lodepng_has_palette_alpha(info);
}

size_t lodepng_get_raw_size_lct(unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
	size_t bpp = lodepng_get_bpp_lct(colortype, bitdepth);
	size_t n = (size_t)w * (size_t)h;
	return ((n / 8) * bpp) + ((n & 7) * bpp + 7) / 8;
}

size_t lodepng_get_raw_size(unsigned w, unsigned h, const LodePNGColorMode* color)
{
	return lodepng_get_raw_size_lct(w, h, color->colortype, color->bitdepth);
}


#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_DECODER

/*in an idat chunk, each scanline is a multiple of 8 bits, unlike the lodepng output buffer,
and in addition has one extra byte per line: the filter byte. So this gives a larger
result than lodepng_get_raw_size. */
static size_t lodepng_get_raw_size_idat(unsigned w, unsigned h, const LodePNGColorMode* color)
{
	size_t bpp = lodepng_get_bpp(color);
	/* + 1 for the filter byte, and possibly plus padding bits per line */
	size_t line = ((size_t)(w / 8) * bpp) + 1 + ((w & 7) * bpp + 7) / 8;
	return (size_t)h * line;
}

/* Safely check if multiplying two integers will overflow (no undefined
behavior, compiler removing the code, etc...) and output result. */
static int lodepng_mulofl(size_t a, size_t b, size_t* result)
{
	*result = a * b; /* Unsigned multiplication is well defined and safe in C90 */
	return (a != 0 && *result / a != b);
}

/* Safely check if adding two integers will overflow (no undefined
behavior, compiler removing the code, etc...) and output result. */
static int lodepng_addofl(size_t a, size_t b, size_t* result)
{
	*result = a + b; /* Unsigned addition is well defined and safe in C90 */
	return *result < a;
}

/*Safely checks whether size_t overflow can be caused due to amount of pixels.
This check is overcautious rather than precise. If this check indicates no overflow,
you can safely compute in a size_t (but not an unsigned):
-(size_t)w * (size_t)h * 8
-amount of bytes in IDAT (including filter, padding and Adam7 bytes)
-amount of bytes in raw color model
Returns 1 if overflow possible, 0 if not.
*/
static int lodepng_pixel_overflow(unsigned w, unsigned h,
	const LodePNGColorMode* pngcolor, const LodePNGColorMode* rawcolor)
{
	size_t bpp = LODEPNG_MAX(lodepng_get_bpp(pngcolor), lodepng_get_bpp(rawcolor));
	size_t numpixels, total;
	size_t line; /* bytes per line in worst case */

	if (lodepng_mulofl((size_t)w, (size_t)h, &numpixels)) return 1;
	if (lodepng_mulofl(numpixels, 8, &total)) return 1; /* bit pointer with 8-bit color, or 8 bytes per channel color */

														/* Bytes per scanline with the expression "(w / 8) * bpp) + ((w & 7) * bpp + 7) / 8" */
	if (lodepng_mulofl((size_t)(w / 8), bpp, &line)) return 1;
	if (lodepng_addofl(line, ((w & 7) * bpp + 7) / 8, &line)) return 1;

	if (lodepng_addofl(line, 5, &line)) return 1; /* 5 bytes overhead per line: 1 filterbyte, 4 for Adam7 worst case */
	if (lodepng_mulofl(line, h, &total)) return 1; /* Total bytes in worst case */

	return 0; /* no overflow */
}
#endif /*LODEPNG_COMPILE_DECODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static void LodePNGUnknownChunks_init(LodePNGInfo* info)
{
	unsigned i;
	for (i = 0; i != 3; ++i) info->unknown_chunks_data[i] = 0;
	for (i = 0; i != 3; ++i) info->unknown_chunks_size[i] = 0;
}

static void LodePNGUnknownChunks_cleanup(LodePNGInfo* info)
{
	unsigned i;
	for (i = 0; i != 3; ++i) lodepng_free(info->unknown_chunks_data[i]);
}

static unsigned LodePNGUnknownChunks_copy(LodePNGInfo* dest, const LodePNGInfo* src)
{
	unsigned i;

	LodePNGUnknownChunks_cleanup(dest);

	for (i = 0; i != 3; ++i)
	{
		size_t j;
		dest->unknown_chunks_size[i] = src->unknown_chunks_size[i];
		dest->unknown_chunks_data[i] = (unsigned char*)lodepng_malloc(src->unknown_chunks_size[i]);
		if (!dest->unknown_chunks_data[i] && dest->unknown_chunks_size[i]) return 83; /*alloc fail*/
		for (j = 0; j < src->unknown_chunks_size[i]; ++j)
		{
			dest->unknown_chunks_data[i][j] = src->unknown_chunks_data[i][j];
		}
	}

	return 0;
}

/******************************************************************************/

static void LodePNGText_init(LodePNGInfo* info)
{
	info->text_num = 0;
	info->text_keys = NULL;
	info->text_strings = NULL;
}

static void LodePNGText_cleanup(LodePNGInfo* info)
{
	size_t i;
	for (i = 0; i != info->text_num; ++i)
	{
		string_cleanup(&info->text_keys[i]);
		string_cleanup(&info->text_strings[i]);
	}
	lodepng_free(info->text_keys);
	lodepng_free(info->text_strings);
}

static unsigned LodePNGText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
	size_t i = 0;
	dest->text_keys = 0;
	dest->text_strings = 0;
	dest->text_num = 0;
	for (i = 0; i != source->text_num; ++i)
	{
		CERROR_TRY_RETURN(lodepng_add_text(dest, source->text_keys[i], source->text_strings[i]));
	}
	return 0;
}

void lodepng_clear_text(LodePNGInfo* info)
{
	LodePNGText_cleanup(info);
}

unsigned lodepng_add_text(LodePNGInfo* info, const char* key, const char* str)
{
	char** new_keys = (char**)(lodepng_realloc(info->text_keys, sizeof(char*) * (info->text_num + 1)));
	char** new_strings = (char**)(lodepng_realloc(info->text_strings, sizeof(char*) * (info->text_num + 1)));
	if (!new_keys || !new_strings)
	{
		lodepng_free(new_keys);
		lodepng_free(new_strings);
		return 83; /*alloc fail*/
	}

	++info->text_num;
	info->text_keys = new_keys;
	info->text_strings = new_strings;

	info->text_keys[info->text_num - 1] = alloc_string(key);
	info->text_strings[info->text_num - 1] = alloc_string(str);

	return 0;
}

/******************************************************************************/

static void LodePNGIText_init(LodePNGInfo* info)
{
	info->itext_num = 0;
	info->itext_keys = NULL;
	info->itext_langtags = NULL;
	info->itext_transkeys = NULL;
	info->itext_strings = NULL;
}

static void LodePNGIText_cleanup(LodePNGInfo* info)
{
	size_t i;
	for (i = 0; i != info->itext_num; ++i)
	{
		string_cleanup(&info->itext_keys[i]);
		string_cleanup(&info->itext_langtags[i]);
		string_cleanup(&info->itext_transkeys[i]);
		string_cleanup(&info->itext_strings[i]);
	}
	lodepng_free(info->itext_keys);
	lodepng_free(info->itext_langtags);
	lodepng_free(info->itext_transkeys);
	lodepng_free(info->itext_strings);
}

static unsigned LodePNGIText_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
	size_t i = 0;
	dest->itext_keys = 0;
	dest->itext_langtags = 0;
	dest->itext_transkeys = 0;
	dest->itext_strings = 0;
	dest->itext_num = 0;
	for (i = 0; i != source->itext_num; ++i)
	{
		CERROR_TRY_RETURN(lodepng_add_itext(dest, source->itext_keys[i], source->itext_langtags[i],
			source->itext_transkeys[i], source->itext_strings[i]));
	}
	return 0;
}

void lodepng_clear_itext(LodePNGInfo* info)
{
	LodePNGIText_cleanup(info);
}

unsigned lodepng_add_itext(LodePNGInfo* info, const char* key, const char* langtag,
	const char* transkey, const char* str)
{
	char** new_keys = (char**)(lodepng_realloc(info->itext_keys, sizeof(char*) * (info->itext_num + 1)));
	char** new_langtags = (char**)(lodepng_realloc(info->itext_langtags, sizeof(char*) * (info->itext_num + 1)));
	char** new_transkeys = (char**)(lodepng_realloc(info->itext_transkeys, sizeof(char*) * (info->itext_num + 1)));
	char** new_strings = (char**)(lodepng_realloc(info->itext_strings, sizeof(char*) * (info->itext_num + 1)));
	if (!new_keys || !new_langtags || !new_transkeys || !new_strings)
	{
		lodepng_free(new_keys);
		lodepng_free(new_langtags);
		lodepng_free(new_transkeys);
		lodepng_free(new_strings);
		return 83; /*alloc fail*/
	}

	++info->itext_num;
	info->itext_keys = new_keys;
	info->itext_langtags = new_langtags;
	info->itext_transkeys = new_transkeys;
	info->itext_strings = new_strings;

	info->itext_keys[info->itext_num - 1] = alloc_string(key);
	info->itext_langtags[info->itext_num - 1] = alloc_string(langtag);
	info->itext_transkeys[info->itext_num - 1] = alloc_string(transkey);
	info->itext_strings[info->itext_num - 1] = alloc_string(str);

	return 0;
}

/* same as set but does not delete */
static unsigned lodepng_assign_icc(LodePNGInfo* info, const char* name, const unsigned char* profile, unsigned profile_size)
{
	info->iccp_name = alloc_string(name);
	info->iccp_profile = (unsigned char*)lodepng_malloc(profile_size);

	if (!info->iccp_name || !info->iccp_profile) return 83; /*alloc fail*/

	memcpy(info->iccp_profile, profile, profile_size);
	info->iccp_profile_size = profile_size;

	return 0; /*ok*/
}

unsigned lodepng_set_icc(LodePNGInfo* info, const char* name, const unsigned char* profile, unsigned profile_size)
{
	if (info->iccp_name) lodepng_clear_icc(info);

	return lodepng_assign_icc(info, name, profile, profile_size);
}

void lodepng_clear_icc(LodePNGInfo* info)
{
	string_cleanup(&info->iccp_name);
	lodepng_free(info->iccp_profile);
	info->iccp_profile = NULL;
	info->iccp_profile_size = 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

void lodepng_info_init(LodePNGInfo* info)
{
	lodepng_color_mode_init(&info->color);
	info->interlace_method = 0;
	info->compression_method = 0;
	info->filter_method = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	info->background_defined = 0;
	info->background_r = info->background_g = info->background_b = 0;

	LodePNGText_init(info);
	LodePNGIText_init(info);

	info->time_defined = 0;
	info->phys_defined = 0;

	info->gama_defined = 0;
	info->chrm_defined = 0;
	info->srgb_defined = 0;
	info->iccp_defined = 0;
	info->iccp_name = NULL;
	info->iccp_profile = NULL;

	LodePNGUnknownChunks_init(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

void lodepng_info_cleanup(LodePNGInfo* info)
{
	lodepng_color_mode_cleanup(&info->color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	LodePNGText_cleanup(info);
	LodePNGIText_cleanup(info);

	lodepng_clear_icc(info);

	LodePNGUnknownChunks_cleanup(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

unsigned lodepng_info_copy(LodePNGInfo* dest, const LodePNGInfo* source)
{
	lodepng_info_cleanup(dest);
	*dest = *source;
	lodepng_color_mode_init(&dest->color);
	CERROR_TRY_RETURN(lodepng_color_mode_copy(&dest->color, &source->color));

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	CERROR_TRY_RETURN(LodePNGText_copy(dest, source));
	CERROR_TRY_RETURN(LodePNGIText_copy(dest, source));
	if (source->iccp_defined)
	{
		CERROR_TRY_RETURN(lodepng_assign_icc(dest, source->iccp_name, source->iccp_profile, source->iccp_profile_size));
	}

	LodePNGUnknownChunks_init(dest);
	CERROR_TRY_RETURN(LodePNGUnknownChunks_copy(dest, source));
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */

/*index: bitgroup index, bits: bitgroup size(1, 2 or 4), in: bitgroup value, out: octet array to add bits to*/
static void addColorBits(unsigned char* out, size_t index, unsigned bits, unsigned in)
{
	unsigned m = bits == 1 ? 7 : bits == 2 ? 3 : 1; /*8 / bits - 1*/
													/*p = the partial index in the byte, e.g. with 4 palettebits it is 0 for first half or 1 for second half*/
	unsigned p = index & m;
	in &= (1u << bits) - 1u; /*filter out any other bits of the input value*/
	in = in << (bits * (m - p));
	if (p == 0) out[index * bits / 8] = in;
	else out[index * bits / 8] |= in;
}

typedef struct ColorTree ColorTree;

/*
One node of a color tree
This is the data structure used to count the number of unique colors and to get a palette
index for a color. It's like an octree, but because the alpha channel is used too, each
node has 16 instead of 8 children.
*/
struct ColorTree
{
	ColorTree* children[16]; /*up to 16 pointers to ColorTree of next level*/
	int index; /*the payload. Only has a meaningful value if this is in the last level*/
};

static void color_tree_init(ColorTree* tree)
{
	int i;
	for (i = 0; i != 16; ++i) tree->children[i] = 0;
	tree->index = -1;
}

static void color_tree_cleanup(ColorTree* tree)
{
	int i;
	for (i = 0; i != 16; ++i)
	{
		if (tree->children[i])
		{
			color_tree_cleanup(tree->children[i]);
			lodepng_free(tree->children[i]);
		}
	}
}

/*returns -1 if color not present, its index otherwise*/
static int color_tree_get(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	int bit = 0;
	for (bit = 0; bit < 8; ++bit)
	{
		int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
		if (!tree->children[i]) return -1;
		else tree = tree->children[i];
	}
	return tree ? tree->index : -1;
}

#ifdef LODEPNG_COMPILE_ENCODER
static int color_tree_has(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return color_tree_get(tree, r, g, b, a) >= 0;
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*color is not allowed to already exist.
Index should be >= 0 (it's signed to be compatible with using -1 for "doesn't exist")*/
static void color_tree_add(ColorTree* tree,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned index)
{
	int bit;
	for (bit = 0; bit < 8; ++bit)
	{
		int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
		if (!tree->children[i])
		{
			tree->children[i] = (ColorTree*)lodepng_malloc(sizeof(ColorTree));
			color_tree_init(tree->children[i]);
		}
		tree = tree->children[i];
	}
	tree->index = (int)index;
}

/*put a pixel, given its RGBA color, into image of any color type*/
static unsigned rgba8ToPixel(unsigned char* out, size_t i,
	const LodePNGColorMode* mode, ColorTree* tree /*for palette*/,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if (mode->colortype == LCT_GREY)
	{
		unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
		if (mode->bitdepth == 8) out[i] = grey;
		else if (mode->bitdepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = grey;
		else
		{
			/*take the most significant bits of grey*/
			grey = (grey >> (8 - mode->bitdepth)) & ((1 << mode->bitdepth) - 1);
			addColorBits(out, i, mode->bitdepth, grey);
		}
	}
	else if (mode->colortype == LCT_RGB)
	{
		if (mode->bitdepth == 8)
		{
			out[i * 3 + 0] = r;
			out[i * 3 + 1] = g;
			out[i * 3 + 2] = b;
		}
		else
		{
			out[i * 6 + 0] = out[i * 6 + 1] = r;
			out[i * 6 + 2] = out[i * 6 + 3] = g;
			out[i * 6 + 4] = out[i * 6 + 5] = b;
		}
	}
	else if (mode->colortype == LCT_PALETTE)
	{
		int index = color_tree_get(tree, r, g, b, a);
		if (index < 0) return 82; /*color not in palette*/
		if (mode->bitdepth == 8) out[i] = index;
		else addColorBits(out, i, mode->bitdepth, (unsigned)index);
	}
	else if (mode->colortype == LCT_GREY_ALPHA)
	{
		unsigned char grey = r; /*((unsigned short)r + g + b) / 3*/;
		if (mode->bitdepth == 8)
		{
			out[i * 2 + 0] = grey;
			out[i * 2 + 1] = a;
		}
		else if (mode->bitdepth == 16)
		{
			out[i * 4 + 0] = out[i * 4 + 1] = grey;
			out[i * 4 + 2] = out[i * 4 + 3] = a;
		}
	}
	else if (mode->colortype == LCT_RGBA)
	{
		if (mode->bitdepth == 8)
		{
			out[i * 4 + 0] = r;
			out[i * 4 + 1] = g;
			out[i * 4 + 2] = b;
			out[i * 4 + 3] = a;
		}
		else
		{
			out[i * 8 + 0] = out[i * 8 + 1] = r;
			out[i * 8 + 2] = out[i * 8 + 3] = g;
			out[i * 8 + 4] = out[i * 8 + 5] = b;
			out[i * 8 + 6] = out[i * 8 + 7] = a;
		}
	}

	return 0; /*no error*/
}

/*put a pixel, given its RGBA16 color, into image of any color 16-bitdepth type*/
static void rgba16ToPixel(unsigned char* out, size_t i,
	const LodePNGColorMode* mode,
	unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
	if (mode->colortype == LCT_GREY)
	{
		unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
		out[i * 2 + 0] = (grey >> 8) & 255;
		out[i * 2 + 1] = grey & 255;
	}
	else if (mode->colortype == LCT_RGB)
	{
		out[i * 6 + 0] = (r >> 8) & 255;
		out[i * 6 + 1] = r & 255;
		out[i * 6 + 2] = (g >> 8) & 255;
		out[i * 6 + 3] = g & 255;
		out[i * 6 + 4] = (b >> 8) & 255;
		out[i * 6 + 5] = b & 255;
	}
	else if (mode->colortype == LCT_GREY_ALPHA)
	{
		unsigned short grey = r; /*((unsigned)r + g + b) / 3*/;
		out[i * 4 + 0] = (grey >> 8) & 255;
		out[i * 4 + 1] = grey & 255;
		out[i * 4 + 2] = (a >> 8) & 255;
		out[i * 4 + 3] = a & 255;
	}
	else if (mode->colortype == LCT_RGBA)
	{
		out[i * 8 + 0] = (r >> 8) & 255;
		out[i * 8 + 1] = r & 255;
		out[i * 8 + 2] = (g >> 8) & 255;
		out[i * 8 + 3] = g & 255;
		out[i * 8 + 4] = (b >> 8) & 255;
		out[i * 8 + 5] = b & 255;
		out[i * 8 + 6] = (a >> 8) & 255;
		out[i * 8 + 7] = a & 255;
	}
}

/*Get RGBA8 color of pixel with index i (y * width + x) from the raw image with given color type.*/
static void getPixelColorRGBA8(unsigned char* r, unsigned char* g,
	unsigned char* b, unsigned char* a,
	const unsigned char* in, size_t i,
	const LodePNGColorMode* mode)
{
	if (mode->colortype == LCT_GREY)
	{
		if (mode->bitdepth == 8)
		{
			*r = *g = *b = in[i];
			if (mode->key_defined && *r == mode->key_r) *a = 0;
			else *a = 255;
		}
		else if (mode->bitdepth == 16)
		{
			*r = *g = *b = in[i * 2 + 0];
			if (mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
			else *a = 255;
		}
		else
		{
			unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
			size_t j = i * mode->bitdepth;
			unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
			*r = *g = *b = (value * 255) / highest;
			if (mode->key_defined && value == mode->key_r) *a = 0;
			else *a = 255;
		}
	}
	else if (mode->colortype == LCT_RGB)
	{
		if (mode->bitdepth == 8)
		{
			*r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
			if (mode->key_defined && *r == mode->key_r && *g == mode->key_g && *b == mode->key_b) *a = 0;
			else *a = 255;
		}
		else
		{
			*r = in[i * 6 + 0];
			*g = in[i * 6 + 2];
			*b = in[i * 6 + 4];
			if (mode->key_defined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
				&& 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
				&& 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
			else *a = 255;
		}
	}
	else if (mode->colortype == LCT_PALETTE)
	{
		unsigned index;
		if (mode->bitdepth == 8) index = in[i];
		else
		{
			size_t j = i * mode->bitdepth;
			index = readBitsFromReversedStream(&j, in, mode->bitdepth);
		}

		if (index >= mode->palettesize)
		{
			/*This is an error according to the PNG spec, but common PNG decoders make it black instead.
			Done here too, slightly faster due to no error handling needed.*/
			*r = *g = *b = 0;
			*a = 255;
		}
		else
		{
			*r = mode->palette[index * 4 + 0];
			*g = mode->palette[index * 4 + 1];
			*b = mode->palette[index * 4 + 2];
			*a = mode->palette[index * 4 + 3];
		}
	}
	else if (mode->colortype == LCT_GREY_ALPHA)
	{
		if (mode->bitdepth == 8)
		{
			*r = *g = *b = in[i * 2 + 0];
			*a = in[i * 2 + 1];
		}
		else
		{
			*r = *g = *b = in[i * 4 + 0];
			*a = in[i * 4 + 2];
		}
	}
	else if (mode->colortype == LCT_RGBA)
	{
		if (mode->bitdepth == 8)
		{
			*r = in[i * 4 + 0];
			*g = in[i * 4 + 1];
			*b = in[i * 4 + 2];
			*a = in[i * 4 + 3];
		}
		else
		{
			*r = in[i * 8 + 0];
			*g = in[i * 8 + 2];
			*b = in[i * 8 + 4];
			*a = in[i * 8 + 6];
		}
	}
}

/*Similar to getPixelColorRGBA8, but with all the for loops inside of the color
mode test cases, optimized to convert the colors much faster, when converting
to RGBA or RGB with 8 bit per cannel. buffer must be RGBA or RGB output with
enough memory, if has_alpha is true the output is RGBA. mode has the color mode
of the input buffer.*/
static void getPixelColorsRGBA8(unsigned char* buffer, size_t numpixels,
	unsigned has_alpha, const unsigned char* in,
	const LodePNGColorMode* mode)
{
	unsigned num_channels = has_alpha ? 4 : 3;
	size_t i;
	if (mode->colortype == LCT_GREY)
	{
		if (mode->bitdepth == 8)
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = buffer[1] = buffer[2] = in[i];
				if (has_alpha) buffer[3] = mode->key_defined && in[i] == mode->key_r ? 0 : 255;
			}
		}
		else if (mode->bitdepth == 16)
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = buffer[1] = buffer[2] = in[i * 2];
				if (has_alpha) buffer[3] = mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r ? 0 : 255;
			}
		}
		else
		{
			unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
			size_t j = 0;
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
				buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
				if (has_alpha) buffer[3] = mode->key_defined && value == mode->key_r ? 0 : 255;
			}
		}
	}
	else if (mode->colortype == LCT_RGB)
	{
		if (mode->bitdepth == 8)
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = in[i * 3 + 0];
				buffer[1] = in[i * 3 + 1];
				buffer[2] = in[i * 3 + 2];
				if (has_alpha) buffer[3] = mode->key_defined && buffer[0] == mode->key_r
					&& buffer[1] == mode->key_g && buffer[2] == mode->key_b ? 0 : 255;
			}
		}
		else
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = in[i * 6 + 0];
				buffer[1] = in[i * 6 + 2];
				buffer[2] = in[i * 6 + 4];
				if (has_alpha) buffer[3] = mode->key_defined
					&& 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
					&& 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
					&& 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b ? 0 : 255;
			}
		}
	}
	else if (mode->colortype == LCT_PALETTE)
	{
		unsigned index;
		size_t j = 0;
		for (i = 0; i != numpixels; ++i, buffer += num_channels)
		{
			if (mode->bitdepth == 8) index = in[i];
			else index = readBitsFromReversedStream(&j, in, mode->bitdepth);

			if (index >= mode->palettesize)
			{
				/*This is an error according to the PNG spec, but most PNG decoders make it black instead.
				Done here too, slightly faster due to no error handling needed.*/
				buffer[0] = buffer[1] = buffer[2] = 0;
				if (has_alpha) buffer[3] = 255;
			}
			else
			{
				buffer[0] = mode->palette[index * 4 + 0];
				buffer[1] = mode->palette[index * 4 + 1];
				buffer[2] = mode->palette[index * 4 + 2];
				if (has_alpha) buffer[3] = mode->palette[index * 4 + 3];
			}
		}
	}
	else if (mode->colortype == LCT_GREY_ALPHA)
	{
		if (mode->bitdepth == 8)
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
				if (has_alpha) buffer[3] = in[i * 2 + 1];
			}
		}
		else
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
				if (has_alpha) buffer[3] = in[i * 4 + 2];
			}
		}
	}
	else if (mode->colortype == LCT_RGBA)
	{
		if (mode->bitdepth == 8)
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = in[i * 4 + 0];
				buffer[1] = in[i * 4 + 1];
				buffer[2] = in[i * 4 + 2];
				if (has_alpha) buffer[3] = in[i * 4 + 3];
			}
		}
		else
		{
			for (i = 0; i != numpixels; ++i, buffer += num_channels)
			{
				buffer[0] = in[i * 8 + 0];
				buffer[1] = in[i * 8 + 2];
				buffer[2] = in[i * 8 + 4];
				if (has_alpha) buffer[3] = in[i * 8 + 6];
			}
		}
	}
}

/*Get RGBA16 color of pixel with index i (y * width + x) from the raw image with
given color type, but the given color type must be 16-bit itself.*/
static void getPixelColorRGBA16(unsigned short* r, unsigned short* g, unsigned short* b, unsigned short* a,
	const unsigned char* in, size_t i, const LodePNGColorMode* mode)
{
	if (mode->colortype == LCT_GREY)
	{
		*r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
		if (mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
		else *a = 65535;
	}
	else if (mode->colortype == LCT_RGB)
	{
		*r = 256u * in[i * 6 + 0] + in[i * 6 + 1];
		*g = 256u * in[i * 6 + 2] + in[i * 6 + 3];
		*b = 256u * in[i * 6 + 4] + in[i * 6 + 5];
		if (mode->key_defined
			&& 256u * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
			&& 256u * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
			&& 256u * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
		else *a = 65535;
	}
	else if (mode->colortype == LCT_GREY_ALPHA)
	{
		*r = *g = *b = 256u * in[i * 4 + 0] + in[i * 4 + 1];
		*a = 256u * in[i * 4 + 2] + in[i * 4 + 3];
	}
	else if (mode->colortype == LCT_RGBA)
	{
		*r = 256u * in[i * 8 + 0] + in[i * 8 + 1];
		*g = 256u * in[i * 8 + 2] + in[i * 8 + 3];
		*b = 256u * in[i * 8 + 4] + in[i * 8 + 5];
		*a = 256u * in[i * 8 + 6] + in[i * 8 + 7];
	}
}

unsigned lodepng_convert(unsigned char* out, const unsigned char* in,
	const LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in,
	unsigned w, unsigned h)
{
	size_t i;
	ColorTree tree;
	size_t numpixels = (size_t)w * (size_t)h;
	unsigned error = 0;

	if (lodepng_color_mode_equal(mode_out, mode_in))
	{
		size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
		for (i = 0; i != numbytes; ++i) out[i] = in[i];
		return 0;
	}

	if (mode_out->colortype == LCT_PALETTE)
	{
		size_t palettesize = mode_out->palettesize;
		const unsigned char* palette = mode_out->palette;
		size_t palsize = (size_t)1u << mode_out->bitdepth;
		/*if the user specified output palette but did not give the values, assume
		they want the values of the input color type (assuming that one is palette).
		Note that we never create a new palette ourselves.*/
		if (palettesize == 0)
		{
			palettesize = mode_in->palettesize;
			palette = mode_in->palette;
			/*if the input was also palette with same bitdepth, then the color types are also
			equal, so copy literally. This to preserve the exact indices that were in the PNG
			even in case there are duplicate colors in the palette.*/
			if (mode_in->colortype == LCT_PALETTE && mode_in->bitdepth == mode_out->bitdepth)
			{
				size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
				for (i = 0; i != numbytes; ++i) out[i] = in[i];
				return 0;
			}
		}
		if (palettesize < palsize) palsize = palettesize;
		color_tree_init(&tree);
		for (i = 0; i != palsize; ++i)
		{
			const unsigned char* p = &palette[i * 4];
			color_tree_add(&tree, p[0], p[1], p[2], p[3], (unsigned)i);
		}
	}

	if (mode_in->bitdepth == 16 && mode_out->bitdepth == 16)
	{
		for (i = 0; i != numpixels; ++i)
		{
			unsigned short r = 0, g = 0, b = 0, a = 0;
			getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
			rgba16ToPixel(out, i, mode_out, r, g, b, a);
		}
	}
	else if (mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGBA)
	{
		getPixelColorsRGBA8(out, numpixels, 1, in, mode_in);
	}
	else if (mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGB)
	{
		getPixelColorsRGBA8(out, numpixels, 0, in, mode_in);
	}
	else
	{
		unsigned char r = 0, g = 0, b = 0, a = 0;
		for (i = 0; i != numpixels; ++i)
		{
			getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
			error = rgba8ToPixel(out, i, mode_out, &tree, r, g, b, a);
			if (error) break;
		}
	}

	if (mode_out->colortype == LCT_PALETTE)
	{
		color_tree_cleanup(&tree);
	}

	return error;
}


/* Converts a single rgb color without alpha from one type to another, color bits truncated to
their bitdepth. In case of single channel (grey or palette), only the r channel is used. Slow
function, do not use to process all pixels of an image. Alpha channel not supported on purpose:
this is for bKGD, supporting alpha may prevent it from finding a color in the palette, from the
specification it looks like bKGD should ignore the alpha values of the palette since it can use
any palette index but doesn't have an alpha channel. Idem with ignoring color key. */
unsigned lodepng_convert_rgb(
	unsigned* r_out, unsigned* g_out, unsigned* b_out,
	unsigned r_in, unsigned g_in, unsigned b_in,
	const LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in)
{
	unsigned r = 0, g = 0, b = 0;
	unsigned mul = 65535 / ((1 << mode_in->bitdepth) - 1); /*65535, 21845, 4369, 257, 1*/
	unsigned shift = 16 - mode_out->bitdepth;

	if (mode_in->colortype == LCT_GREY || mode_in->colortype == LCT_GREY_ALPHA)
	{
		r = g = b = r_in * mul;
	}
	else if (mode_in->colortype == LCT_RGB || mode_in->colortype == LCT_RGBA)
	{
		r = r_in * mul;
		g = g_in * mul;
		b = b_in * mul;
	}
	else if (mode_in->colortype == LCT_PALETTE)
	{
		if (r_in >= mode_in->palettesize) return 82;
		r = mode_in->palette[r_in * 4 + 0] * 257;
		g = mode_in->palette[r_in * 4 + 1] * 257;
		b = mode_in->palette[r_in * 4 + 2] * 257;
	}
	else
	{
		return 31;
	}

	/* now convert to output format */
	if (mode_out->colortype == LCT_GREY || mode_out->colortype == LCT_GREY_ALPHA)
	{
		*r_out = r >> shift;
	}
	else if (mode_out->colortype == LCT_RGB || mode_out->colortype == LCT_RGBA)
	{
		*r_out = r >> shift;
		*g_out = g >> shift;
		*b_out = b >> shift;
	}
	else if (mode_out->colortype == LCT_PALETTE)
	{
		unsigned i;
		/* a 16-bit color cannot be in the palette */
		if ((r >> 8) != (r & 255) || (g >> 8) != (g & 255) || (b >> 8) != (b & 255)) return 82;
		for (i = 0; i < mode_out->palettesize; i++) {
			unsigned j = i * 4;
			if ((r >> 8) == mode_out->palette[j + 0] && (g >> 8) == mode_out->palette[j + 1] &&
				(b >> 8) == mode_out->palette[j + 2])
			{
				*r_out = i;
				return 0;
			}
		}
		return 82;
	}
	else
	{
		return 31;
	}

	return 0;
}

#ifdef LODEPNG_COMPILE_ENCODER

void lodepng_color_profile_init(LodePNGColorProfile* profile)
{
	profile->colored = 0;
	profile->key = 0;
	profile->key_r = profile->key_g = profile->key_b = 0;
	profile->alpha = 0;
	profile->numcolors = 0;
	profile->bits = 1;
	profile->numpixels = 0;
}

/*function used for debug purposes with C++*/
/*void printColorProfile(LodePNGColorProfile* p)
{
std::cout << "colored: " << (int)p->colored << ", ";
std::cout << "key: " << (int)p->key << ", ";
std::cout << "key_r: " << (int)p->key_r << ", ";
std::cout << "key_g: " << (int)p->key_g << ", ";
std::cout << "key_b: " << (int)p->key_b << ", ";
std::cout << "alpha: " << (int)p->alpha << ", ";
std::cout << "numcolors: " << (int)p->numcolors << ", ";
std::cout << "bits: " << (int)p->bits << std::endl;
}*/

/*Returns how many bits needed to represent given value (max 8 bit)*/
static unsigned getValueRequiredBits(unsigned char value)
{
	if (value == 0 || value == 255) return 1;
	/*The scaling of 2-bit and 4-bit values uses multiples of 85 and 17*/
	if (value % 17 == 0) return value % 85 == 0 ? 2 : 4;
	return 8;
}

/*profile must already have been inited.
It's ok to set some parameters of profile to done already.*/
unsigned lodepng_get_color_profile(LodePNGColorProfile* profile,
	const unsigned char* in, unsigned w, unsigned h,
	const LodePNGColorMode* mode_in)
{
	unsigned error = 0;
	size_t i;
	ColorTree tree;
	size_t numpixels = (size_t)w * (size_t)h;

	/* mark things as done already if it would be impossible to have a more expensive case */
	unsigned colored_done = lodepng_is_greyscale_type(mode_in) ? 1 : 0;
	unsigned alpha_done = lodepng_can_have_alpha(mode_in) ? 0 : 1;
	unsigned numcolors_done = 0;
	unsigned bpp = lodepng_get_bpp(mode_in);
	unsigned bits_done = (profile->bits == 1 && bpp == 1) ? 1 : 0;
	unsigned sixteen = 0; /* whether the input image is 16 bit */
	unsigned maxnumcolors = 257;
	if (bpp <= 8) maxnumcolors = LODEPNG_MIN(257, profile->numcolors + (1 << bpp));

	profile->numpixels += numpixels;

	color_tree_init(&tree);

	/*If the profile was already filled in from previous data, fill its palette in tree
	and mark things as done already if we know they are the most expensive case already*/
	if (profile->alpha) alpha_done = 1;
	if (profile->colored) colored_done = 1;
	if (profile->bits == 16) numcolors_done = 1;
	if (profile->bits >= bpp) bits_done = 1;
	if (profile->numcolors >= maxnumcolors) numcolors_done = 1;

	if (!numcolors_done)
	{
		for (i = 0; i < profile->numcolors; i++)
		{
			const unsigned char* color = &profile->palette[i * 4];
			color_tree_add(&tree, color[0], color[1], color[2], color[3], i);
		}
	}

	/*Check if the 16-bit input is truly 16-bit*/
	if (mode_in->bitdepth == 16 && !sixteen)
	{
		unsigned short r, g, b, a;
		for (i = 0; i != numpixels; ++i)
		{
			getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
			if ((r & 255) != ((r >> 8) & 255) || (g & 255) != ((g >> 8) & 255) ||
				(b & 255) != ((b >> 8) & 255) || (a & 255) != ((a >> 8) & 255)) /*first and second byte differ*/
			{
				profile->bits = 16;
				sixteen = 1;
				bits_done = 1;
				numcolors_done = 1; /*counting colors no longer useful, palette doesn't support 16-bit*/
				break;
			}
		}
	}

	if (sixteen)
	{
		unsigned short r = 0, g = 0, b = 0, a = 0;

		for (i = 0; i != numpixels; ++i)
		{
			getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);

			if (!colored_done && (r != g || r != b))
			{
				profile->colored = 1;
				colored_done = 1;
			}

			if (!alpha_done)
			{
				unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
				if (a != 65535 && (a != 0 || (profile->key && !matchkey)))
				{
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
				}
				else if (a == 0 && !profile->alpha && !profile->key)
				{
					profile->key = 1;
					profile->key_r = r;
					profile->key_g = g;
					profile->key_b = b;
				}
				else if (a == 65535 && profile->key && matchkey)
				{
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
				}
			}
			if (alpha_done && numcolors_done && colored_done && bits_done) break;
		}

		if (profile->key && !profile->alpha)
		{
			for (i = 0; i != numpixels; ++i)
			{
				getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
				if (a != 0 && r == profile->key_r && g == profile->key_g && b == profile->key_b)
				{
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
				}
			}
		}
	}
	else /* < 16-bit */
	{
		unsigned char r = 0, g = 0, b = 0, a = 0;
		for (i = 0; i != numpixels; ++i)
		{
			getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);

			if (!bits_done && profile->bits < 8)
			{
				/*only r is checked, < 8 bits is only relevant for greyscale*/
				unsigned bits = getValueRequiredBits(r);
				if (bits > profile->bits) profile->bits = bits;
			}
			bits_done = (profile->bits >= bpp);

			if (!colored_done && (r != g || r != b))
			{
				profile->colored = 1;
				colored_done = 1;
				if (profile->bits < 8) profile->bits = 8; /*PNG has no colored modes with less than 8-bit per channel*/
			}

			if (!alpha_done)
			{
				unsigned matchkey = (r == profile->key_r && g == profile->key_g && b == profile->key_b);
				if (a != 255 && (a != 0 || (profile->key && !matchkey)))
				{
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
					if (profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
				else if (a == 0 && !profile->alpha && !profile->key)
				{
					profile->key = 1;
					profile->key_r = r;
					profile->key_g = g;
					profile->key_b = b;
				}
				else if (a == 255 && profile->key && matchkey)
				{
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
					if (profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
			}

			if (!numcolors_done)
			{
				if (!color_tree_has(&tree, r, g, b, a))
				{
					color_tree_add(&tree, r, g, b, a, profile->numcolors);
					if (profile->numcolors < 256)
					{
						unsigned char* p = profile->palette;
						unsigned n = profile->numcolors;
						p[n * 4 + 0] = r;
						p[n * 4 + 1] = g;
						p[n * 4 + 2] = b;
						p[n * 4 + 3] = a;
					}
					++profile->numcolors;
					numcolors_done = profile->numcolors >= maxnumcolors;
				}
			}

			if (alpha_done && numcolors_done && colored_done && bits_done) break;
		}

		if (profile->key && !profile->alpha)
		{
			for (i = 0; i != numpixels; ++i)
			{
				getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
				if (a != 0 && r == profile->key_r && g == profile->key_g && b == profile->key_b)
				{
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					profile->alpha = 1;
					profile->key = 0;
					alpha_done = 1;
					if (profile->bits < 8) profile->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
			}
		}

		/*make the profile's key always 16-bit for consistency - repeat each byte twice*/
		profile->key_r += (profile->key_r << 8);
		profile->key_g += (profile->key_g << 8);
		profile->key_b += (profile->key_b << 8);
	}

	color_tree_cleanup(&tree);
	return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*Adds a single color to the color profile. The profile must already have been inited. The color must be given as 16-bit
(with 2 bytes repeating for 8-bit and 65535 for opaque alpha channel). This function is expensive, do not call it for
all pixels of an image but only for a few additional values. */
static unsigned lodepng_color_profile_add(LodePNGColorProfile* profile,
	unsigned r, unsigned g, unsigned b, unsigned a)
{
	unsigned error = 0;
	unsigned char image[8];
	LodePNGColorMode mode;
	lodepng_color_mode_init(&mode);
	image[0] = r >> 8; image[1] = r; image[2] = g >> 8; image[3] = g;
	image[4] = b >> 8; image[5] = b; image[6] = a >> 8; image[7] = a;
	mode.bitdepth = 16;
	mode.colortype = LCT_RGBA;
	error = lodepng_get_color_profile(profile, image, 1, 1, &mode);
	lodepng_color_mode_cleanup(&mode);
	return error;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*Autochoose color model given the computed profile. mode_in is to copy palette order from
when relevant.*/
static unsigned auto_choose_color_from_profile(LodePNGColorMode* mode_out,
	const LodePNGColorMode* mode_in,
	const LodePNGColorProfile* prof)
{
	unsigned error = 0;
	unsigned palettebits, palette_ok;
	size_t i, n;
	size_t numpixels = prof->numpixels;

	unsigned alpha = prof->alpha;
	unsigned key = prof->key;
	unsigned bits = prof->bits;

	mode_out->key_defined = 0;

	if (key && numpixels <= 16)
	{
		alpha = 1; /*too few pixels to justify tRNS chunk overhead*/
		key = 0;
		if (bits < 8) bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
	}
	n = prof->numcolors;
	palettebits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
	palette_ok = n <= 256 && bits <= 8;
	if (numpixels < n * 2) palette_ok = 0; /*don't add palette overhead if image has only a few pixels*/
	if (!prof->colored && bits <= palettebits) palette_ok = 0; /*grey is less overhead*/

	if (palette_ok)
	{
		const unsigned char* p = prof->palette;
		lodepng_palette_clear(mode_out); /*remove potential earlier palette*/
		for (i = 0; i != prof->numcolors; ++i)
		{
			error = lodepng_palette_add(mode_out, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
			if (error) break;
		}

		mode_out->colortype = LCT_PALETTE;
		mode_out->bitdepth = palettebits;

		if (mode_in->colortype == LCT_PALETTE && mode_in->palettesize >= mode_out->palettesize
			&& mode_in->bitdepth == mode_out->bitdepth)
		{
			/*If input should have same palette colors, keep original to preserve its order and prevent conversion*/
			lodepng_color_mode_cleanup(mode_out);
			lodepng_color_mode_copy(mode_out, mode_in);
		}
	}
	else /*8-bit or 16-bit per channel*/
	{
		mode_out->bitdepth = bits;
		mode_out->colortype = alpha ? (prof->colored ? LCT_RGBA : LCT_GREY_ALPHA)
			: (prof->colored ? LCT_RGB : LCT_GREY);

		if (key)
		{
			unsigned mask = (1u << mode_out->bitdepth) - 1u; /*profile always uses 16-bit, mask converts it*/
			mode_out->key_r = prof->key_r & mask;
			mode_out->key_g = prof->key_g & mask;
			mode_out->key_b = prof->key_b & mask;
			mode_out->key_defined = 1;
		}
	}

	return error;
}

/*Automatically chooses color type that gives smallest amount of bits in the
output image, e.g. grey if there are only greyscale pixels, palette if there
are less than 256 colors, color key if only single transparent color, ...
Updates values of mode with a potentially smaller color model. mode_out should
contain the user chosen color model, but will be overwritten with the new chosen one.*/
unsigned lodepng_auto_choose_color(LodePNGColorMode* mode_out,
	const unsigned char* image, unsigned w, unsigned h,
	const LodePNGColorMode* mode_in)
{
	unsigned error = 0;
	LodePNGColorProfile prof;
	lodepng_color_profile_init(&prof);
	error = lodepng_get_color_profile(&prof, image, w, h, mode_in);
	if (error) return error;
	return auto_choose_color_from_profile(mode_out, mode_in, &prof);
}

#endif /* #ifdef LODEPNG_COMPILE_ENCODER */

/*
Paeth predicter, used by PNG filter type 4
The parameters are of type short, but should come from unsigned chars, the shorts
are only needed to make the paeth calculation correct.
*/
static unsigned char paethPredictor(short a, short b, short c)
{
	short pa = abs(b - c);
	short pb = abs(a - c);
	short pc = abs(a + b - c - c);

	if (pc < pa && pc < pb) return (unsigned char)c;
	else if (pb < pa) return (unsigned char)b;
	else return (unsigned char)a;
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; /*x start values*/
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; /*y start values*/
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; /*x delta values*/
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; /*y delta values*/

															 /*
															 Outputs various dimensions and positions in the image related to the Adam7 reduced images.
															 passw: output containing the width of the 7 passes
															 passh: output containing the height of the 7 passes
															 filter_passstart: output containing the index of the start and end of each
															 reduced image with filter bytes
															 padded_passstart output containing the index of the start and end of each
															 reduced image when without filter bytes but with padded scanlines
															 passstart: output containing the index of the start and end of each reduced
															 image without padding between scanlines, but still padding between the images
															 w, h: width and height of non-interlaced image
															 bpp: bits per pixel
															 "padded" is only relevant if bpp is less than 8 and a scanline or image does not
															 end at a full byte
															 */
static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8],
	size_t padded_passstart[8], size_t passstart[8], unsigned w, unsigned h, unsigned bpp)
{
	/*the passstart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass*/
	unsigned i;

	/*calculate width and height in pixels of each pass*/
	for (i = 0; i != 7; ++i)
	{
		passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
		passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
		if (passw[i] == 0) passh[i] = 0;
		if (passh[i] == 0) passw[i] = 0;
	}

	filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
	for (i = 0; i != 7; ++i)
	{
		/*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
		filter_passstart[i + 1] = filter_passstart[i]
			+ ((passw[i] && passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0);
		/*bits padded if needed to fill full byte at end of each scanline*/
		padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7) / 8);
		/*only padded at end of reduced image*/
		passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7) / 8;
	}
}

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Decoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*read the information from the header and store it in the LodePNGInfo. return value is error*/
unsigned lodepng_inspect(unsigned* w, unsigned* h, LodePNGState* state,
	const unsigned char* in, size_t insize)
{
	LodePNGInfo* info = &state->info_png;
	if (insize == 0 || in == 0)
	{
		CERROR_RETURN_ERROR(state->error, 48); /*error: the given data is empty*/
	}
	if (insize < 33)
	{
		CERROR_RETURN_ERROR(state->error, 27); /*error: the data length is smaller than the length of a PNG header*/
	}

	/*when decoding a new PNG image, make sure all parameters created after previous decoding are reset*/
	lodepng_info_cleanup(info);
	lodepng_info_init(info);

	if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71
		|| in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10)
	{
		CERROR_RETURN_ERROR(state->error, 28); /*error: the first 8 bytes are not the correct PNG signature*/
	}
	if (lodepng_chunk_length(in + 8) != 13)
	{
		CERROR_RETURN_ERROR(state->error, 94); /*error: header size must be 13 bytes*/
	}
	if (!lodepng_chunk_type_equals(in + 8, "IHDR"))
	{
		CERROR_RETURN_ERROR(state->error, 29); /*error: it doesn't start with a IHDR chunk!*/
	}

	/*read the values given in the header*/
	*w = lodepng_read32bitInt(&in[16]);
	*h = lodepng_read32bitInt(&in[20]);
	info->color.bitdepth = in[24];
	info->color.colortype = (LodePNGColorType)in[25];
	info->compression_method = in[26];
	info->filter_method = in[27];
	info->interlace_method = in[28];

	if (*w == 0 || *h == 0)
	{
		CERROR_RETURN_ERROR(state->error, 93);
	}

	if (!state->decoder.ignore_crc)
	{
		unsigned CRC = lodepng_read32bitInt(&in[29]);
		unsigned checksum = lodepng_crc32(&in[12], 17);
		if (CRC != checksum)
		{
			CERROR_RETURN_ERROR(state->error, 57); /*invalid CRC*/
		}
	}

	/*error: only compression method 0 is allowed in the specification*/
	if (info->compression_method != 0) CERROR_RETURN_ERROR(state->error, 32);
	/*error: only filter method 0 is allowed in the specification*/
	if (info->filter_method != 0) CERROR_RETURN_ERROR(state->error, 33);
	/*error: only interlace methods 0 and 1 exist in the specification*/
	if (info->interlace_method > 1) CERROR_RETURN_ERROR(state->error, 34);

	state->error = checkColorValidity(info->color.colortype, info->color.bitdepth);
	return state->error;
}

static unsigned unfilterScanline(unsigned char* recon, const unsigned char* scanline, const unsigned char* precon,
	size_t bytewidth, unsigned char filterType, size_t length)
{
	/*
	For PNG filter method 0
	unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte,
	the filter works byte per byte (bytewidth = 1)
	precon is the previous unfiltered scanline, recon the result, scanline the current one
	the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
	recon and scanline MAY be the same memory address! precon must be disjoint.
	*/

	size_t i;
	switch (filterType)
	{
	case 0:
		for (i = 0; i != length; ++i) recon[i] = scanline[i];
		break;
	case 1:
		for (i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
		for (i = bytewidth; i < length; ++i) recon[i] = scanline[i] + recon[i - bytewidth];
		break;
	case 2:
		if (precon)
		{
			for (i = 0; i != length; ++i) recon[i] = scanline[i] + precon[i];
		}
		else
		{
			for (i = 0; i != length; ++i) recon[i] = scanline[i];
		}
		break;
	case 3:
		if (precon)
		{
			for (i = 0; i != bytewidth; ++i) recon[i] = scanline[i] + (precon[i] >> 1);
			for (i = bytewidth; i < length; ++i) recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) >> 1);
		}
		else
		{
			for (i = 0; i != bytewidth; ++i) recon[i] = scanline[i];
			for (i = bytewidth; i < length; ++i) recon[i] = scanline[i] + (recon[i - bytewidth] >> 1);
		}
		break;
	case 4:
		if (precon)
		{
			for (i = 0; i != bytewidth; ++i)
			{
				recon[i] = (scanline[i] + precon[i]); /*paethPredictor(0, precon[i], 0) is always precon[i]*/
			}
			for (i = bytewidth; i < length; ++i)
			{
				recon[i] = (scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
			}
		}
		else
		{
			for (i = 0; i != bytewidth; ++i)
			{
				recon[i] = scanline[i];
			}
			for (i = bytewidth; i < length; ++i)
			{
				/*paethPredictor(recon[i - bytewidth], 0, 0) is always recon[i - bytewidth]*/
				recon[i] = (scanline[i] + recon[i - bytewidth]);
			}
		}
		break;
	default: return 36; /*error: unexisting filter type given*/
	}
	return 0;
}

static unsigned unfilter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
	/*
	For PNG filter method 0
	this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 seven times)
	out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
	w and h are image dimensions or dimensions of reduced image, bpp is bits per pixel
	in and out are allowed to be the same memory address (but aren't the same size since in has the extra filter bytes)
	*/

	unsigned y;
	unsigned char* prevline = 0;

	/*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
	size_t bytewidth = (bpp + 7) / 8;
	size_t linebytes = (w * bpp + 7) / 8;

	for (y = 0; y < h; ++y)
	{
		size_t outindex = linebytes * y;
		size_t inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
		unsigned char filterType = in[inindex];

		CERROR_TRY_RETURN(unfilterScanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes));

		prevline = &out[outindex];
	}

	return 0;
}

/*
in: Adam7 interlaced image, with no padding bits between scanlines, but between
reduced images so that each reduced image starts at a byte.
out: the same pixels, but re-ordered so that they're now a non-interlaced image with size w*h
bpp: bits per pixel
out has the following size in bits: w * h * bpp.
in is possibly bigger due to padding bits between reduced images.
out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation
(because that's likely a little bit faster)
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_deinterlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
	unsigned passw[7], passh[7];
	size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned i;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	if (bpp >= 8)
	{
		for (i = 0; i != 7; ++i)
		{
			unsigned x, y, b;
			size_t bytewidth = bpp / 8;
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x)
				{
					size_t pixelinstart = passstart[i] + (y * passw[i] + x) * bytewidth;
					size_t pixeloutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
					for (b = 0; b < bytewidth; ++b)
					{
						out[pixeloutstart + b] = in[pixelinstart + b];
					}
				}
		}
	}
	else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
	{
		for (i = 0; i != 7; ++i)
		{
			unsigned x, y, b;
			unsigned ilinebits = bpp * passw[i];
			unsigned olinebits = bpp * w;
			size_t obp, ibp; /*bit pointers (for out and in buffer)*/
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x)
				{
					ibp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
					obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
					for (b = 0; b < bpp; ++b)
					{
						unsigned char bit = readBitFromReversedStream(&ibp, in);
						/*note that this function assumes the out buffer is completely 0, use setBitOfReversedStream otherwise*/
						setBitOfReversedStream0(&obp, out, bit);
					}
				}
		}
	}
}

static void removePaddingBits(unsigned char* out, const unsigned char* in,
	size_t olinebits, size_t ilinebits, unsigned h)
{
	/*
	After filtering there are still padding bits if scanlines have non multiple of 8 bit amounts. They need
	to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers
	for the Adam7 code, the color convert code and the output to the user.
	in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must
	have >= ilinebits*h bits, out must have >= olinebits*h bits, olinebits must be <= ilinebits
	also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
	only useful if (ilinebits - olinebits) is a value in the range 1..7
	*/
	unsigned y;
	size_t diff = ilinebits - olinebits;
	size_t ibp = 0, obp = 0; /*input and output bit pointers*/
	for (y = 0; y < h; ++y)
	{
		size_t x;
		for (x = 0; x < olinebits; ++x)
		{
			unsigned char bit = readBitFromReversedStream(&ibp, in);
			setBitOfReversedStream(&obp, out, bit);
		}
		ibp += diff;
	}
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from
the IDAT chunks (with filter index bytes and possible padding bits)
return value is error*/
static unsigned postProcessScanlines(unsigned char* out, unsigned char* in,
	unsigned w, unsigned h, const LodePNGInfo* info_png)
{
	/*
	This function converts the filtered-padded-interlaced data into pure 2D image buffer with the PNG's colortype.
	Steps:
	*) if no Adam7: 1) unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
	*) if adam7: 1) 7x unfilter 2) 7x remove padding bits 3) Adam7_deinterlace
	NOTE: the in buffer will be overwritten with intermediate data!
	*/
	unsigned bpp = lodepng_get_bpp(&info_png->color);
	if (bpp == 0) return 31; /*error: invalid colortype*/

	if (info_png->interlace_method == 0)
	{
		if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
		{
			CERROR_TRY_RETURN(unfilter(in, in, w, h, bpp));
			removePaddingBits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
		}
		/*we can immediately filter into the out buffer, no other steps needed*/
		else CERROR_TRY_RETURN(unfilter(out, in, w, h, bpp));
	}
	else /*interlace_method is 1 (Adam7)*/
	{
		unsigned passw[7], passh[7]; size_t filter_passstart[8], padded_passstart[8], passstart[8];
		unsigned i;

		Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

		for (i = 0; i != 7; ++i)
		{
			CERROR_TRY_RETURN(unfilter(&in[padded_passstart[i]], &in[filter_passstart[i]], passw[i], passh[i], bpp));
			/*TODO: possible efficiency improvement: if in this reduced image the bits fit nicely in 1 scanline,
			move bytes instead of bits or move not at all*/
			if (bpp < 8)
			{
				/*remove padding bits in scanlines; after this there still may be padding
				bits between the different reduced images: each reduced image still starts nicely at a byte*/
				removePaddingBits(&in[passstart[i]], &in[padded_passstart[i]], passw[i] * bpp,
					((passw[i] * bpp + 7) / 8) * 8, passh[i]);
			}
		}

		Adam7_deinterlace(out, in, w, h, bpp);
	}

	return 0;
}

static unsigned readChunk_PLTE(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
	unsigned pos = 0, i;
	if (color->palette) lodepng_free(color->palette);
	color->palettesize = chunkLength / 3;
	color->palette = (unsigned char*)lodepng_malloc(4 * color->palettesize);
	if (!color->palette && color->palettesize)
	{
		color->palettesize = 0;
		return 83; /*alloc fail*/
	}
	if (color->palettesize > 256) return 38; /*error: palette too big*/

	for (i = 0; i != color->palettesize; ++i)
	{
		color->palette[4 * i + 0] = data[pos++]; /*R*/
		color->palette[4 * i + 1] = data[pos++]; /*G*/
		color->palette[4 * i + 2] = data[pos++]; /*B*/
		color->palette[4 * i + 3] = 255; /*alpha*/
	}

	return 0; /* OK */
}

static unsigned readChunk_tRNS(LodePNGColorMode* color, const unsigned char* data, size_t chunkLength)
{
	unsigned i;
	if (color->colortype == LCT_PALETTE)
	{
		/*error: more alpha values given than there are palette entries*/
		if (chunkLength > color->palettesize) return 39;

		for (i = 0; i != chunkLength; ++i) color->palette[4 * i + 3] = data[i];
	}
	else if (color->colortype == LCT_GREY)
	{
		/*error: this chunk must be 2 bytes for greyscale image*/
		if (chunkLength != 2) return 30;

		color->key_defined = 1;
		color->key_r = color->key_g = color->key_b = 256u * data[0] + data[1];
	}
	else if (color->colortype == LCT_RGB)
	{
		/*error: this chunk must be 6 bytes for RGB image*/
		if (chunkLength != 6) return 41;

		color->key_defined = 1;
		color->key_r = 256u * data[0] + data[1];
		color->key_g = 256u * data[2] + data[3];
		color->key_b = 256u * data[4] + data[5];
	}
	else return 42; /*error: tRNS chunk not allowed for other color models*/

	return 0; /* OK */
}


#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*background color chunk (bKGD)*/
static unsigned readChunk_bKGD(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (info->color.colortype == LCT_PALETTE)
	{
		/*error: this chunk must be 1 byte for indexed color image*/
		if (chunkLength != 1) return 43;

		/*error: invalid palette index, or maybe this chunk appeared before PLTE*/
		if (data[0] >= info->color.palettesize) return 103;

		info->background_defined = 1;
		info->background_r = info->background_g = info->background_b = data[0];
	}
	else if (info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
	{
		/*error: this chunk must be 2 bytes for greyscale image*/
		if (chunkLength != 2) return 44;

		/*the values are truncated to bitdepth in the PNG file*/
		info->background_defined = 1;
		info->background_r = info->background_g = info->background_b = 256u * data[0] + data[1];
	}
	else if (info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
	{
		/*error: this chunk must be 6 bytes for greyscale image*/
		if (chunkLength != 6) return 45;

		/*the values are truncated to bitdepth in the PNG file*/
		info->background_defined = 1;
		info->background_r = 256u * data[0] + data[1];
		info->background_g = 256u * data[2] + data[3];
		info->background_b = 256u * data[4] + data[5];
	}

	return 0; /* OK */
}

/*text chunk (tEXt)*/
static unsigned readChunk_tEXt(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	unsigned error = 0;
	char *key = 0, *str = 0;
	unsigned i;

	while (!error) /*not really a while loop, only used to break on error*/
	{
		unsigned length, string2_begin;

		length = 0;
		while (length < chunkLength && data[length] != 0) ++length;
		/*even though it's not allowed by the standard, no error is thrown if
		there's no null termination char, if the text is empty*/
		if (length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

		key = (char*)lodepng_malloc(length + 1);
		if (!key) CERROR_BREAK(error, 83); /*alloc fail*/

		key[length] = 0;
		for (i = 0; i != length; ++i) key[i] = (char)data[i];

		string2_begin = length + 1; /*skip keyword null terminator*/

		length = (unsigned)(chunkLength < string2_begin ? 0 : chunkLength - string2_begin);
		str = (char*)lodepng_malloc(length + 1);
		if (!str) CERROR_BREAK(error, 83); /*alloc fail*/

		str[length] = 0;
		for (i = 0; i != length; ++i) str[i] = (char)data[string2_begin + i];

		error = lodepng_add_text(info, key, str);

		break;
	}

	lodepng_free(key);
	lodepng_free(str);

	return error;
}

/*compressed text chunk (zTXt)*/
static unsigned readChunk_zTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
	const unsigned char* data, size_t chunkLength)
{
	unsigned error = 0;
	unsigned i;

	unsigned length, string2_begin;
	char *key = 0;
	ucvector decoded;

	ucvector_init(&decoded);

	while (!error) /*not really a while loop, only used to break on error*/
	{
		for (length = 0; length < chunkLength && data[length] != 0; ++length);
		if (length + 2 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/
		if (length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

		key = (char*)lodepng_malloc(length + 1);
		if (!key) CERROR_BREAK(error, 83); /*alloc fail*/

		key[length] = 0;
		for (i = 0; i != length; ++i) key[i] = (char)data[i];

		if (data[length + 1] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

		string2_begin = length + 2;
		if (string2_begin > chunkLength) CERROR_BREAK(error, 75); /*no null termination, corrupt?*/

		length = (unsigned)chunkLength - string2_begin;
		/*will fail if zlib error, e.g. if length is too small*/
		error = zlib_decompress(&decoded.data, &decoded.size,
			(unsigned char*)(&data[string2_begin]),
			length, zlibsettings);
		if (error) break;
		ucvector_push_back(&decoded, 0);

		error = lodepng_add_text(info, key, (char*)decoded.data);

		break;
	}

	lodepng_free(key);
	ucvector_cleanup(&decoded);

	return error;
}

/*international text chunk (iTXt)*/
static unsigned readChunk_iTXt(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
	const unsigned char* data, size_t chunkLength)
{
	unsigned error = 0;
	unsigned i;

	unsigned length, begin, compressed;
	char *key = 0, *langtag = 0, *transkey = 0;
	ucvector decoded;
	ucvector_init(&decoded); /* TODO: only use in case of compressed text */

	while (!error) /*not really a while loop, only used to break on error*/
	{
		/*Quick check if the chunk length isn't too small. Even without check
		it'd still fail with other error checks below if it's too short. This just gives a different error code.*/
		if (chunkLength < 5) CERROR_BREAK(error, 30); /*iTXt chunk too short*/

													  /*read the key*/
		for (length = 0; length < chunkLength && data[length] != 0; ++length);
		if (length + 3 >= chunkLength) CERROR_BREAK(error, 75); /*no null termination char, corrupt?*/
		if (length < 1 || length > 79) CERROR_BREAK(error, 89); /*keyword too short or long*/

		key = (char*)lodepng_malloc(length + 1);
		if (!key) CERROR_BREAK(error, 83); /*alloc fail*/

		key[length] = 0;
		for (i = 0; i != length; ++i) key[i] = (char)data[i];

		/*read the compression method*/
		compressed = data[length + 1];
		if (data[length + 2] != 0) CERROR_BREAK(error, 72); /*the 0 byte indicating compression must be 0*/

															/*even though it's not allowed by the standard, no error is thrown if
															there's no null termination char, if the text is empty for the next 3 texts*/

															/*read the langtag*/
		begin = length + 3;
		length = 0;
		for (i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

		langtag = (char*)lodepng_malloc(length + 1);
		if (!langtag) CERROR_BREAK(error, 83); /*alloc fail*/

		langtag[length] = 0;
		for (i = 0; i != length; ++i) langtag[i] = (char)data[begin + i];

		/*read the transkey*/
		begin += length + 1;
		length = 0;
		for (i = begin; i < chunkLength && data[i] != 0; ++i) ++length;

		transkey = (char*)lodepng_malloc(length + 1);
		if (!transkey) CERROR_BREAK(error, 83); /*alloc fail*/

		transkey[length] = 0;
		for (i = 0; i != length; ++i) transkey[i] = (char)data[begin + i];

		/*read the actual text*/
		begin += length + 1;

		length = (unsigned)chunkLength < begin ? 0 : (unsigned)chunkLength - begin;

		if (compressed)
		{
			/*will fail if zlib error, e.g. if length is too small*/
			error = zlib_decompress(&decoded.data, &decoded.size,
				(unsigned char*)(&data[begin]),
				length, zlibsettings);
			if (error) break;
			if (decoded.allocsize < decoded.size) decoded.allocsize = decoded.size;
			ucvector_push_back(&decoded, 0);
		}
		else
		{
			if (!ucvector_resize(&decoded, length + 1)) CERROR_BREAK(error, 83 /*alloc fail*/);

			decoded.data[length] = 0;
			for (i = 0; i != length; ++i) decoded.data[i] = data[begin + i];
		}

		error = lodepng_add_itext(info, key, langtag, transkey, (char*)decoded.data);

		break;
	}

	lodepng_free(key);
	lodepng_free(langtag);
	lodepng_free(transkey);
	ucvector_cleanup(&decoded);

	return error;
}

static unsigned readChunk_tIME(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (chunkLength != 7) return 73; /*invalid tIME chunk size*/

	info->time_defined = 1;
	info->time.year = 256u * data[0] + data[1];
	info->time.month = data[2];
	info->time.day = data[3];
	info->time.hour = data[4];
	info->time.minute = data[5];
	info->time.second = data[6];

	return 0; /* OK */
}

static unsigned readChunk_pHYs(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (chunkLength != 9) return 74; /*invalid pHYs chunk size*/

	info->phys_defined = 1;
	info->phys_x = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
	info->phys_y = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
	info->phys_unit = data[8];

	return 0; /* OK */
}

static unsigned readChunk_gAMA(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (chunkLength != 4) return 96; /*invalid gAMA chunk size*/

	info->gama_defined = 1;
	info->gama_gamma = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];

	return 0; /* OK */
}

static unsigned readChunk_cHRM(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (chunkLength != 32) return 97; /*invalid cHRM chunk size*/

	info->chrm_defined = 1;
	info->chrm_white_x = 16777216u * data[0] + 65536u * data[1] + 256u * data[2] + data[3];
	info->chrm_white_y = 16777216u * data[4] + 65536u * data[5] + 256u * data[6] + data[7];
	info->chrm_red_x = 16777216u * data[8] + 65536u * data[9] + 256u * data[10] + data[11];
	info->chrm_red_y = 16777216u * data[12] + 65536u * data[13] + 256u * data[14] + data[15];
	info->chrm_green_x = 16777216u * data[16] + 65536u * data[17] + 256u * data[18] + data[19];
	info->chrm_green_y = 16777216u * data[20] + 65536u * data[21] + 256u * data[22] + data[23];
	info->chrm_blue_x = 16777216u * data[24] + 65536u * data[25] + 256u * data[26] + data[27];
	info->chrm_blue_y = 16777216u * data[28] + 65536u * data[29] + 256u * data[30] + data[31];

	return 0; /* OK */
}

static unsigned readChunk_sRGB(LodePNGInfo* info, const unsigned char* data, size_t chunkLength)
{
	if (chunkLength != 1) return 98; /*invalid sRGB chunk size (this one is never ignored)*/

	info->srgb_defined = 1;
	info->srgb_intent = data[0];

	return 0; /* OK */
}

static unsigned readChunk_iCCP(LodePNGInfo* info, const LodePNGDecompressSettings* zlibsettings,
	const unsigned char* data, size_t chunkLength)
{
	unsigned error = 0;
	unsigned i;

	unsigned length, string2_begin;
	ucvector decoded;

	info->iccp_defined = 1;
	if (info->iccp_name) lodepng_clear_icc(info);

	for (length = 0; length < chunkLength && data[length] != 0; ++length);
	if (length + 2 >= chunkLength) return 75; /*no null termination, corrupt?*/
	if (length < 1 || length > 79) return 89; /*keyword too short or long*/

	info->iccp_name = (char*)lodepng_malloc(length + 1);
	if (!info->iccp_name) return 83; /*alloc fail*/

	info->iccp_name[length] = 0;
	for (i = 0; i != length; ++i) info->iccp_name[i] = (char)data[i];

	if (data[length + 1] != 0) return 72; /*the 0 byte indicating compression must be 0*/

	string2_begin = length + 2;
	if (string2_begin > chunkLength) return 75; /*no null termination, corrupt?*/

	length = (unsigned)chunkLength - string2_begin;
	ucvector_init(&decoded);
	error = zlib_decompress(&decoded.data, &decoded.size,
		(unsigned char*)(&data[string2_begin]),
		length, zlibsettings);
	if (!error) {
		info->iccp_profile_size = decoded.size;
		info->iccp_profile = (unsigned char*)lodepng_malloc(decoded.size);
		if (info->iccp_profile) {
			memcpy(info->iccp_profile, decoded.data, decoded.size);
		}
		else {
			error = 83; /* alloc fail */
		}
	}
	ucvector_cleanup(&decoded);
	return error;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
static void decodeGeneric(unsigned char** out, unsigned* w, unsigned* h,
	LodePNGState* state,
	const unsigned char* in, size_t insize)
{
	unsigned char IEND = 0;
	const unsigned char* chunk;
	size_t i;
	ucvector idat; /*the data from idat chunks*/
	ucvector scanlines;
	size_t predict;
	size_t outsize = 0;

	/*for unknown chunk order*/
	unsigned unknown = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	unsigned critical_pos = 1; /*1 = after IHDR, 2 = after PLTE, 3 = after IDAT*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

							   /*provide some proper output values if error will happen*/
	*out = 0;

	state->error = lodepng_inspect(w, h, state, in, insize); /*reads header and resets other parameters in state->info_png*/
	if (state->error) return;

	if (lodepng_pixel_overflow(*w, *h, &state->info_png.color, &state->info_raw))
	{
		CERROR_RETURN(state->error, 92); /*overflow possible due to amount of pixels*/
	}

	ucvector_init(&idat);
	chunk = &in[33]; /*first byte of the first chunk after the header*/

					 /*loop through the chunks, ignoring unknown chunks and stopping at IEND chunk.
					 IDAT data is put at the start of the in buffer*/
	while (!IEND && !state->error)
	{
		unsigned chunkLength;
		const unsigned char* data; /*the data in the chunk*/

								   /*error: size of the in buffer too small to contain next chunk*/
		if ((size_t)((chunk - in) + 12) > insize || chunk < in)
		{
			if (state->decoder.ignore_end) break; /*other errors may still happen though*/
			CERROR_BREAK(state->error, 30);
		}

		/*length of the data of the chunk, excluding the length bytes, chunk type and CRC bytes*/
		chunkLength = lodepng_chunk_length(chunk);
		/*error: chunk length larger than the max PNG chunk size*/
		if (chunkLength > 2147483647)
		{
			if (state->decoder.ignore_end) break; /*other errors may still happen though*/
			CERROR_BREAK(state->error, 63);
		}

		if ((size_t)((chunk - in) + chunkLength + 12) > insize || (chunk + chunkLength + 12) < in)
		{
			CERROR_BREAK(state->error, 64); /*error: size of the in buffer too small to contain next chunk*/
		}

		data = lodepng_chunk_data_const(chunk);

		unknown = 0;

		/*IDAT chunk, containing compressed image data*/
		if (lodepng_chunk_type_equals(chunk, "IDAT"))
		{
			size_t oldsize = idat.size;
			size_t newsize;
			if (lodepng_addofl(oldsize, chunkLength, &newsize)) CERROR_BREAK(state->error, 95);
			if (!ucvector_resize(&idat, newsize)) CERROR_BREAK(state->error, 83 /*alloc fail*/);
			for (i = 0; i != chunkLength; ++i) idat.data[oldsize + i] = data[i];
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
			critical_pos = 3;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		}
		/*IEND chunk*/
		else if (lodepng_chunk_type_equals(chunk, "IEND"))
		{
			IEND = 1;
		}
		/*palette chunk (PLTE)*/
		else if (lodepng_chunk_type_equals(chunk, "PLTE"))
		{
			state->error = readChunk_PLTE(&state->info_png.color, data, chunkLength);
			if (state->error) break;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
			critical_pos = 2;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		}
		/*palette transparency chunk (tRNS). Even though this one is an ancillary chunk , it is still compiled
		in without 'LODEPNG_COMPILE_ANCILLARY_CHUNKS' because it contains essential color information that
		affects the alpha channel of pixels. */
		else if (lodepng_chunk_type_equals(chunk, "tRNS"))
		{
			state->error = readChunk_tRNS(&state->info_png.color, data, chunkLength);
			if (state->error) break;
		}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*background color chunk (bKGD)*/
		else if (lodepng_chunk_type_equals(chunk, "bKGD"))
		{
			state->error = readChunk_bKGD(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		/*text chunk (tEXt)*/
		else if (lodepng_chunk_type_equals(chunk, "tEXt"))
		{
			if (state->decoder.read_text_chunks)
			{
				state->error = readChunk_tEXt(&state->info_png, data, chunkLength);
				if (state->error) break;
			}
		}
		/*compressed text chunk (zTXt)*/
		else if (lodepng_chunk_type_equals(chunk, "zTXt"))
		{
			if (state->decoder.read_text_chunks)
			{
				state->error = readChunk_zTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
				if (state->error) break;
			}
		}
		/*international text chunk (iTXt)*/
		else if (lodepng_chunk_type_equals(chunk, "iTXt"))
		{
			if (state->decoder.read_text_chunks)
			{
				state->error = readChunk_iTXt(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
				if (state->error) break;
			}
		}
		else if (lodepng_chunk_type_equals(chunk, "tIME"))
		{
			state->error = readChunk_tIME(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		else if (lodepng_chunk_type_equals(chunk, "pHYs"))
		{
			state->error = readChunk_pHYs(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		else if (lodepng_chunk_type_equals(chunk, "gAMA"))
		{
			state->error = readChunk_gAMA(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		else if (lodepng_chunk_type_equals(chunk, "cHRM"))
		{
			state->error = readChunk_cHRM(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		else if (lodepng_chunk_type_equals(chunk, "sRGB"))
		{
			state->error = readChunk_sRGB(&state->info_png, data, chunkLength);
			if (state->error) break;
		}
		else if (lodepng_chunk_type_equals(chunk, "iCCP"))
		{
			state->error = readChunk_iCCP(&state->info_png, &state->decoder.zlibsettings, data, chunkLength);
			if (state->error) break;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		else /*it's not an implemented chunk type, so ignore it: skip over the data*/
		{
			/*error: unknown critical chunk (5th bit of first byte of chunk type is 0)*/
			if (!state->decoder.ignore_critical && !lodepng_chunk_ancillary(chunk))
			{
				CERROR_BREAK(state->error, 69);
			}

			unknown = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
			if (state->decoder.remember_unknown_chunks)
			{
				state->error = lodepng_chunk_append(&state->info_png.unknown_chunks_data[critical_pos - 1],
					&state->info_png.unknown_chunks_size[critical_pos - 1], chunk);
				if (state->error) break;
			}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		}

		if (!state->decoder.ignore_crc && !unknown) /*check CRC if wanted, only on known chunk types*/
		{
			if (lodepng_chunk_check_crc(chunk)) CERROR_BREAK(state->error, 57); /*invalid CRC*/
		}

		if (!IEND) chunk = lodepng_chunk_next_const(chunk);
	}

	ucvector_init(&scanlines);
	/*predict output size, to allocate exact size for output buffer to avoid more dynamic allocation.
	If the decompressed size does not match the prediction, the image must be corrupt.*/
	if (state->info_png.interlace_method == 0)
	{
		predict = lodepng_get_raw_size_idat(*w, *h, &state->info_png.color);
	}
	else
	{
		/*Adam-7 interlaced: predicted size is the sum of the 7 sub-images sizes*/
		const LodePNGColorMode* color = &state->info_png.color;
		predict = 0;
		predict += lodepng_get_raw_size_idat((*w + 7) >> 3, (*h + 7) >> 3, color);
		if (*w > 4) predict += lodepng_get_raw_size_idat((*w + 3) >> 3, (*h + 7) >> 3, color);
		predict += lodepng_get_raw_size_idat((*w + 3) >> 2, (*h + 3) >> 3, color);
		if (*w > 2) predict += lodepng_get_raw_size_idat((*w + 1) >> 2, (*h + 3) >> 2, color);
		predict += lodepng_get_raw_size_idat((*w + 1) >> 1, (*h + 1) >> 2, color);
		if (*w > 1) predict += lodepng_get_raw_size_idat((*w + 0) >> 1, (*h + 1) >> 1, color);
		predict += lodepng_get_raw_size_idat((*w + 0), (*h + 0) >> 1, color);
	}
	if (!state->error && !ucvector_reserve(&scanlines, predict)) state->error = 83; /*alloc fail*/
	if (!state->error)
	{
		state->error = zlib_decompress(&scanlines.data, &scanlines.size, idat.data,
			idat.size, &state->decoder.zlibsettings);
		if (!state->error && scanlines.size != predict) state->error = 91; /*decompressed size doesn't match prediction*/
	}
	ucvector_cleanup(&idat);

	if (!state->error)
	{
		outsize = lodepng_get_raw_size(*w, *h, &state->info_png.color);
		*out = (unsigned char*)lodepng_malloc(outsize);
		if (!*out) state->error = 83; /*alloc fail*/
	}
	if (!state->error)
	{
		for (i = 0; i < outsize; i++) (*out)[i] = 0;
		state->error = postProcessScanlines(*out, scanlines.data, *w, *h, &state->info_png);
	}
	ucvector_cleanup(&scanlines);
}

unsigned lodepng_decode(unsigned char** out, unsigned* w, unsigned* h,
	LodePNGState* state,
	const unsigned char* in, size_t insize)
{
	*out = 0;
	decodeGeneric(out, w, h, state, in, insize);
	if (state->error) return state->error;
	if (!state->decoder.color_convert || lodepng_color_mode_equal(&state->info_raw, &state->info_png.color))
	{
		/*same color type, no copying or converting of data needed*/
		/*store the info_png color settings on the info_raw so that the info_raw still reflects what colortype
		the raw image has to the end user*/
		if (!state->decoder.color_convert)
		{
			state->error = lodepng_color_mode_copy(&state->info_raw, &state->info_png.color);
			if (state->error) return state->error;
		}
	}
	else
	{
		/*color conversion needed; sort of copy of the data*/
		unsigned char* data = *out;
		size_t outsize;

		/*TODO: check if this works according to the statement in the documentation: "The converter can convert
		from greyscale input color type, to 8-bit greyscale or greyscale with alpha"*/
		if (!(state->info_raw.colortype == LCT_RGB || state->info_raw.colortype == LCT_RGBA)
			&& !(state->info_raw.bitdepth == 8))
		{
			return 56; /*unsupported color mode conversion*/
		}

		outsize = lodepng_get_raw_size(*w, *h, &state->info_raw);
		*out = (unsigned char*)lodepng_malloc(outsize);
		if (!(*out))
		{
			state->error = 83; /*alloc fail*/
		}
		else state->error = lodepng_convert(*out, data, &state->info_raw,
			&state->info_png.color, *w, *h);
		lodepng_free(data);
	}
	return state->error;
}

unsigned lodepng_decode_memory(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in,
	size_t insize, LodePNGColorType colortype, unsigned bitdepth)
{
	unsigned error;
	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = colortype;
	state.info_raw.bitdepth = bitdepth;
	error = lodepng_decode(out, w, h, &state, in, insize);
	lodepng_state_cleanup(&state);
	return error;
}

unsigned lodepng_decode32(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
	return lodepng_decode_memory(out, w, h, in, insize, LCT_RGBA, 8);
}

unsigned lodepng_decode24(unsigned char** out, unsigned* w, unsigned* h, const unsigned char* in, size_t insize)
{
	return lodepng_decode_memory(out, w, h, in, insize, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_decode_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename,
	LodePNGColorType colortype, unsigned bitdepth)
{
	unsigned char* buffer = 0;
	size_t buffersize;
	unsigned error;
	error = lodepng_load_file(&buffer, &buffersize, filename);
	if (!error) error = lodepng_decode_memory(out, w, h, buffer, buffersize, colortype, bitdepth);
	lodepng_free(buffer);
	return error;
}

unsigned lodepng_decode32_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
	return lodepng_decode_file(out, w, h, filename, LCT_RGBA, 8);
}

unsigned lodepng_decode24_file(unsigned char** out, unsigned* w, unsigned* h, const char* filename)
{
	return lodepng_decode_file(out, w, h, filename, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_decoder_settings_init(LodePNGDecoderSettings* settings)
{
	settings->color_convert = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	settings->read_text_chunks = 1;
	settings->remember_unknown_chunks = 0;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	settings->ignore_crc = 0;
	settings->ignore_critical = 0;
	settings->ignore_end = 0;
	lodepng_decompress_settings_init(&settings->zlibsettings);
}

#endif /*LODEPNG_COMPILE_DECODER*/

#if defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER)

void lodepng_state_init(LodePNGState* state)
{
#ifdef LODEPNG_COMPILE_DECODER
	lodepng_decoder_settings_init(&state->decoder);
#endif /*LODEPNG_COMPILE_DECODER*/
#ifdef LODEPNG_COMPILE_ENCODER
	lodepng_encoder_settings_init(&state->encoder);
#endif /*LODEPNG_COMPILE_ENCODER*/
	lodepng_color_mode_init(&state->info_raw);
	lodepng_info_init(&state->info_png);
	state->error = 1;
}

void lodepng_state_cleanup(LodePNGState* state)
{
	lodepng_color_mode_cleanup(&state->info_raw);
	lodepng_info_cleanup(&state->info_png);
}

void lodepng_state_copy(LodePNGState* dest, const LodePNGState* source)
{
	lodepng_state_cleanup(dest);
	*dest = *source;
	lodepng_color_mode_init(&dest->info_raw);
	lodepng_info_init(&dest->info_png);
	dest->error = lodepng_color_mode_copy(&dest->info_raw, &source->info_raw); if (dest->error) return;
	dest->error = lodepng_info_copy(&dest->info_png, &source->info_png); if (dest->error) return;
}

#endif /* defined(LODEPNG_COMPILE_DECODER) || defined(LODEPNG_COMPILE_ENCODER) */

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Encoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*chunkName must be string of 4 characters*/
static unsigned addChunk(ucvector* out, const char* chunkName, const unsigned char* data, size_t length)
{
	CERROR_TRY_RETURN(lodepng_chunk_create(&out->data, &out->size, (unsigned)length, chunkName, data));
	out->allocsize = out->size; /*fix the allocsize again*/
	return 0;
}

static void writeSignature(ucvector* out)
{
	/*8 bytes PNG signature, aka the magic bytes*/
	ucvector_push_back(out, 137);
	ucvector_push_back(out, 80);
	ucvector_push_back(out, 78);
	ucvector_push_back(out, 71);
	ucvector_push_back(out, 13);
	ucvector_push_back(out, 10);
	ucvector_push_back(out, 26);
	ucvector_push_back(out, 10);
}

static unsigned addChunk_IHDR(ucvector* out, unsigned w, unsigned h,
	LodePNGColorType colortype, unsigned bitdepth, unsigned interlace_method)
{
	unsigned error = 0;
	ucvector header;
	ucvector_init(&header);

	lodepng_add32bitInt(&header, w); /*width*/
	lodepng_add32bitInt(&header, h); /*height*/
	ucvector_push_back(&header, (unsigned char)bitdepth); /*bit depth*/
	ucvector_push_back(&header, (unsigned char)colortype); /*color type*/
	ucvector_push_back(&header, 0); /*compression method*/
	ucvector_push_back(&header, 0); /*filter method*/
	ucvector_push_back(&header, interlace_method); /*interlace method*/

	error = addChunk(out, "IHDR", header.data, header.size);
	ucvector_cleanup(&header);

	return error;
}

static unsigned addChunk_PLTE(ucvector* out, const LodePNGColorMode* info)
{
	unsigned error = 0;
	size_t i;
	ucvector PLTE;
	ucvector_init(&PLTE);
	for (i = 0; i != info->palettesize * 4; ++i)
	{
		/*add all channels except alpha channel*/
		if (i % 4 != 3) ucvector_push_back(&PLTE, info->palette[i]);
	}
	error = addChunk(out, "PLTE", PLTE.data, PLTE.size);
	ucvector_cleanup(&PLTE);

	return error;
}

static unsigned addChunk_tRNS(ucvector* out, const LodePNGColorMode* info)
{
	unsigned error = 0;
	size_t i;
	ucvector tRNS;
	ucvector_init(&tRNS);
	if (info->colortype == LCT_PALETTE)
	{
		size_t amount = info->palettesize;
		/*the tail of palette values that all have 255 as alpha, does not have to be encoded*/
		for (i = info->palettesize; i != 0; --i)
		{
			if (info->palette[4 * (i - 1) + 3] == 255) --amount;
			else break;
		}
		/*add only alpha channel*/
		for (i = 0; i != amount; ++i) ucvector_push_back(&tRNS, info->palette[4 * i + 3]);
	}
	else if (info->colortype == LCT_GREY)
	{
		if (info->key_defined)
		{
			ucvector_push_back(&tRNS, (unsigned char)(info->key_r >> 8));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_r & 255));
		}
	}
	else if (info->colortype == LCT_RGB)
	{
		if (info->key_defined)
		{
			ucvector_push_back(&tRNS, (unsigned char)(info->key_r >> 8));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_r & 255));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_g >> 8));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_g & 255));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_b >> 8));
			ucvector_push_back(&tRNS, (unsigned char)(info->key_b & 255));
		}
	}

	error = addChunk(out, "tRNS", tRNS.data, tRNS.size);
	ucvector_cleanup(&tRNS);

	return error;
}

static unsigned addChunk_IDAT(ucvector* out, const unsigned char* data, size_t datasize,
	LodePNGCompressSettings* zlibsettings)
{
	ucvector zlibdata;
	unsigned error = 0;

	/*compress with the Zlib compressor*/
	ucvector_init(&zlibdata);
	error = zlib_compress(&zlibdata.data, &zlibdata.size, data, datasize, zlibsettings);
	if (!error) error = addChunk(out, "IDAT", zlibdata.data, zlibdata.size);
	ucvector_cleanup(&zlibdata);

	return error;
}

static unsigned addChunk_IEND(ucvector* out)
{
	unsigned error = 0;
	error = addChunk(out, "IEND", 0, 0);
	return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static unsigned addChunk_tEXt(ucvector* out, const char* keyword, const char* textstring)
{
	unsigned error = 0;
	size_t i;
	ucvector text;
	ucvector_init(&text);
	for (i = 0; keyword[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)keyword[i]);
	if (i < 1 || i > 79) return 89; /*error: invalid keyword size*/
	ucvector_push_back(&text, 0); /*0 termination char*/
	for (i = 0; textstring[i] != 0; ++i) ucvector_push_back(&text, (unsigned char)textstring[i]);
	error = addChunk(out, "tEXt", text.data, text.size);
	ucvector_cleanup(&text);

	return error;
}

static unsigned addChunk_zTXt(ucvector* out, const char* keyword, const char* textstring,
	LodePNGCompressSettings* zlibsettings)
{
	unsigned error = 0;
	ucvector data, compressed;
	size_t i, textsize = strlen(textstring);

	ucvector_init(&data);
	ucvector_init(&compressed);
	for (i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
	if (i < 1 || i > 79) return 89; /*error: invalid keyword size*/
	ucvector_push_back(&data, 0); /*0 termination char*/
	ucvector_push_back(&data, 0); /*compression method: 0*/

	error = zlib_compress(&compressed.data, &compressed.size,
		(unsigned char*)textstring, textsize, zlibsettings);
	if (!error)
	{
		for (i = 0; i != compressed.size; ++i) ucvector_push_back(&data, compressed.data[i]);
		error = addChunk(out, "zTXt", data.data, data.size);
	}

	ucvector_cleanup(&compressed);
	ucvector_cleanup(&data);
	return error;
}

static unsigned addChunk_iTXt(ucvector* out, unsigned compressed, const char* keyword, const char* langtag,
	const char* transkey, const char* textstring, LodePNGCompressSettings* zlibsettings)
{
	unsigned error = 0;
	ucvector data;
	size_t i, textsize = strlen(textstring);

	ucvector_init(&data);

	for (i = 0; keyword[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)keyword[i]);
	if (i < 1 || i > 79) return 89; /*error: invalid keyword size*/
	ucvector_push_back(&data, 0); /*null termination char*/
	ucvector_push_back(&data, compressed ? 1 : 0); /*compression flag*/
	ucvector_push_back(&data, 0); /*compression method*/
	for (i = 0; langtag[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)langtag[i]);
	ucvector_push_back(&data, 0); /*null termination char*/
	for (i = 0; transkey[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)transkey[i]);
	ucvector_push_back(&data, 0); /*null termination char*/

	if (compressed)
	{
		ucvector compressed_data;
		ucvector_init(&compressed_data);
		error = zlib_compress(&compressed_data.data, &compressed_data.size,
			(unsigned char*)textstring, textsize, zlibsettings);
		if (!error)
		{
			for (i = 0; i != compressed_data.size; ++i) ucvector_push_back(&data, compressed_data.data[i]);
		}
		ucvector_cleanup(&compressed_data);
	}
	else /*not compressed*/
	{
		for (i = 0; textstring[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)textstring[i]);
	}

	if (!error) error = addChunk(out, "iTXt", data.data, data.size);
	ucvector_cleanup(&data);
	return error;
}

static unsigned addChunk_bKGD(ucvector* out, const LodePNGInfo* info)
{
	unsigned error = 0;
	ucvector bKGD;
	ucvector_init(&bKGD);
	if (info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA)
	{
		ucvector_push_back(&bKGD, (unsigned char)(info->background_r >> 8));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255));
	}
	else if (info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA)
	{
		ucvector_push_back(&bKGD, (unsigned char)(info->background_r >> 8));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_g >> 8));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_g & 255));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_b >> 8));
		ucvector_push_back(&bKGD, (unsigned char)(info->background_b & 255));
	}
	else if (info->color.colortype == LCT_PALETTE)
	{
		ucvector_push_back(&bKGD, (unsigned char)(info->background_r & 255)); /*palette index*/
	}

	error = addChunk(out, "bKGD", bKGD.data, bKGD.size);
	ucvector_cleanup(&bKGD);

	return error;
}

static unsigned addChunk_tIME(ucvector* out, const LodePNGTime* time)
{
	unsigned error = 0;
	unsigned char* data = (unsigned char*)lodepng_malloc(7);
	if (!data) return 83; /*alloc fail*/
	data[0] = (unsigned char)(time->year >> 8);
	data[1] = (unsigned char)(time->year & 255);
	data[2] = (unsigned char)time->month;
	data[3] = (unsigned char)time->day;
	data[4] = (unsigned char)time->hour;
	data[5] = (unsigned char)time->minute;
	data[6] = (unsigned char)time->second;
	error = addChunk(out, "tIME", data, 7);
	lodepng_free(data);
	return error;
}

static unsigned addChunk_pHYs(ucvector* out, const LodePNGInfo* info)
{
	unsigned error = 0;
	ucvector data;
	ucvector_init(&data);

	lodepng_add32bitInt(&data, info->phys_x);
	lodepng_add32bitInt(&data, info->phys_y);
	ucvector_push_back(&data, info->phys_unit);

	error = addChunk(out, "pHYs", data.data, data.size);
	ucvector_cleanup(&data);

	return error;
}

static unsigned addChunk_gAMA(ucvector* out, const LodePNGInfo* info)
{
	unsigned error = 0;
	ucvector data;
	ucvector_init(&data);

	lodepng_add32bitInt(&data, info->gama_gamma);

	error = addChunk(out, "gAMA", data.data, data.size);
	ucvector_cleanup(&data);

	return error;
}

static unsigned addChunk_cHRM(ucvector* out, const LodePNGInfo* info)
{
	unsigned error = 0;
	ucvector data;
	ucvector_init(&data);

	lodepng_add32bitInt(&data, info->chrm_white_x);
	lodepng_add32bitInt(&data, info->chrm_white_y);
	lodepng_add32bitInt(&data, info->chrm_red_x);
	lodepng_add32bitInt(&data, info->chrm_red_y);
	lodepng_add32bitInt(&data, info->chrm_green_x);
	lodepng_add32bitInt(&data, info->chrm_green_y);
	lodepng_add32bitInt(&data, info->chrm_blue_x);
	lodepng_add32bitInt(&data, info->chrm_blue_y);

	error = addChunk(out, "cHRM", data.data, data.size);
	ucvector_cleanup(&data);

	return error;
}

static unsigned addChunk_sRGB(ucvector* out, const LodePNGInfo* info)
{
	unsigned char data = info->srgb_intent;
	return addChunk(out, "sRGB", &data, 1);
}

static unsigned addChunk_iCCP(ucvector* out, const LodePNGInfo* info, LodePNGCompressSettings* zlibsettings)
{
	unsigned error = 0;
	ucvector data, compressed;
	size_t i;

	ucvector_init(&data);
	ucvector_init(&compressed);
	for (i = 0; info->iccp_name[i] != 0; ++i) ucvector_push_back(&data, (unsigned char)info->iccp_name[i]);
	if (i < 1 || i > 79) return 89; /*error: invalid keyword size*/
	ucvector_push_back(&data, 0); /*0 termination char*/
	ucvector_push_back(&data, 0); /*compression method: 0*/

	error = zlib_compress(&compressed.data, &compressed.size,
		info->iccp_profile, info->iccp_profile_size, zlibsettings);
	if (!error)
	{
		for (i = 0; i != compressed.size; ++i) ucvector_push_back(&data, compressed.data[i]);
		error = addChunk(out, "iCCP", data.data, data.size);
	}

	ucvector_cleanup(&compressed);
	ucvector_cleanup(&data);
	return error;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

static void filterScanline(unsigned char* out, const unsigned char* scanline, const unsigned char* prevline,
	size_t length, size_t bytewidth, unsigned char filterType)
{
	size_t i;
	switch (filterType)
	{
	case 0: /*None*/
		for (i = 0; i != length; ++i) out[i] = scanline[i];
		break;
	case 1: /*Sub*/
		for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
		for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - scanline[i - bytewidth];
		break;
	case 2: /*Up*/
		if (prevline)
		{
			for (i = 0; i != length; ++i) out[i] = scanline[i] - prevline[i];
		}
		else
		{
			for (i = 0; i != length; ++i) out[i] = scanline[i];
		}
		break;
	case 3: /*Average*/
		if (prevline)
		{
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i] - (prevline[i] >> 1);
			for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) >> 1);
		}
		else
		{
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
			for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - (scanline[i - bytewidth] >> 1);
		}
		break;
	case 4: /*Paeth*/
		if (prevline)
		{
			/*paethPredictor(0, prevline[i], 0) is always prevline[i]*/
			for (i = 0; i != bytewidth; ++i) out[i] = (scanline[i] - prevline[i]);
			for (i = bytewidth; i < length; ++i)
			{
				out[i] = (scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
			}
		}
		else
		{
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
			/*paethPredictor(scanline[i - bytewidth], 0, 0) is always scanline[i - bytewidth]*/
			for (i = bytewidth; i < length; ++i) out[i] = (scanline[i] - scanline[i - bytewidth]);
		}
		break;
	default: return; /*unexisting filter type given*/
	}
}

/* log2 approximation. A slight bit faster than std::log. */
static float flog2(float f)
{
	float result = 0;
	while (f > 32) { result += 4; f /= 16; }
	while (f > 2) { ++result; f /= 2; }
	return result + 1.442695f * (f * f * f / 3 - 3 * f * f / 2 + 3 * f - 1.83333f);
}

static unsigned filter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h,
	const LodePNGColorMode* info, const LodePNGEncoderSettings* settings)
{
	/*
	For PNG filter method 0
	out must be a buffer with as size: h + (w * h * bpp + 7) / 8, because there are
	the scanlines with 1 extra byte per scanline
	*/

	unsigned bpp = lodepng_get_bpp(info);
	/*the width of a scanline in bytes, not including the filter type*/
	size_t linebytes = (w * bpp + 7) / 8;
	/*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
	size_t bytewidth = (bpp + 7) / 8;
	const unsigned char* prevline = 0;
	unsigned x, y;
	unsigned error = 0;
	LodePNGFilterStrategy strategy = settings->filter_strategy;

	/*
	There is a heuristic called the minimum sum of absolute differences heuristic, suggested by the PNG standard:
	*  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e.
	use fixed filtering, with the filter None).
	* (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is
	not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply
	all five filters and select the filter that produces the smallest sum of absolute values per row.
	This heuristic is used if filter strategy is LFS_MINSUM and filter_palette_zero is true.

	If filter_palette_zero is true and filter_strategy is not LFS_MINSUM, the above heuristic is followed,
	but for "the other case", whatever strategy filter_strategy is set to instead of the minimum sum
	heuristic is used.
	*/
	if (settings->filter_palette_zero &&
		(info->colortype == LCT_PALETTE || info->bitdepth < 8)) strategy = LFS_ZERO;

	if (bpp == 0) return 31; /*error: invalid color type*/

	if (strategy == LFS_ZERO)
	{
		for (y = 0; y != h; ++y)
		{
			size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
			size_t inindex = linebytes * y;
			out[outindex] = 0; /*filter type byte*/
			filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, 0);
			prevline = &in[inindex];
		}
	}
	else if (strategy == LFS_MINSUM)
	{
		/*adaptive filtering*/
		size_t sum[5];
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		size_t smallest = 0;
		unsigned char type, bestType = 0;

		for (type = 0; type != 5; ++type)
		{
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) return 83; /*alloc fail*/
		}

		if (!error)
		{
			for (y = 0; y != h; ++y)
			{
				/*try the 5 filter types*/
				for (type = 0; type != 5; ++type)
				{
					filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);

					/*calculate the sum of the result*/
					sum[type] = 0;
					if (type == 0)
					{
						for (x = 0; x != linebytes; ++x) sum[type] += (unsigned char)(attempt[type][x]);
					}
					else
					{
						for (x = 0; x != linebytes; ++x)
						{
							/*For differences, each byte should be treated as signed, values above 127 are negative
							(converted to signed char). Filtertype 0 isn't a difference though, so use unsigned there.
							This means filtertype 0 is almost never chosen, but that is justified.*/
							unsigned char s = attempt[type][x];
							sum[type] += s < 128 ? s : (255U - s);
						}
					}

					/*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
					if (type == 0 || sum[type] < smallest)
					{
						bestType = type;
						smallest = sum[type];
					}
				}

				prevline = &in[y * linebytes];

				/*now fill the out values*/
				out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
				for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
			}
		}

		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else if (strategy == LFS_ENTROPY)
	{
		float sum[5];
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		float smallest = 0;
		unsigned type, bestType = 0;
		unsigned count[256];

		for (type = 0; type != 5; ++type)
		{
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) return 83; /*alloc fail*/
		}

		for (y = 0; y != h; ++y)
		{
			/*try the 5 filter types*/
			for (type = 0; type != 5; ++type)
			{
				filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
				for (x = 0; x != 256; ++x) count[x] = 0;
				for (x = 0; x != linebytes; ++x) ++count[attempt[type][x]];
				++count[type]; /*the filter type itself is part of the scanline*/
				sum[type] = 0;
				for (x = 0; x != 256; ++x)
				{
					float p = count[x] / (float)(linebytes + 1);
					sum[type] += count[x] == 0 ? 0 : flog2(1 / p) * p;
				}
				/*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
				if (type == 0 || sum[type] < smallest)
				{
					bestType = type;
					smallest = sum[type];
				}
			}

			prevline = &in[y * linebytes];

			/*now fill the out values*/
			out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
			for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
		}

		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else if (strategy == LFS_PREDEFINED)
	{
		for (y = 0; y != h; ++y)
		{
			size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
			size_t inindex = linebytes * y;
			unsigned char type = settings->predefined_filters[y];
			out[outindex] = type; /*filter type byte*/
			filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, type);
			prevline = &in[inindex];
		}
	}
	else if (strategy == LFS_BRUTE_FORCE)
	{
		/*brute force filter chooser.
		deflate the scanline after every filter attempt to see which one deflates best.
		This is very slow and gives only slightly smaller, sometimes even larger, result*/
		size_t size[5];
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		size_t smallest = 0;
		unsigned type = 0, bestType = 0;
		unsigned char* dummy;
		LodePNGCompressSettings zlibsettings = settings->zlibsettings;
		/*use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose,
		to simulate the true case where the tree is the same for the whole image. Sometimes it gives
		better result with dynamic tree anyway. Using the fixed tree sometimes gives worse, but in rare
		cases better compression. It does make this a bit less slow, so it's worth doing this.*/
		zlibsettings.btype = 1;
		/*a custom encoder likely doesn't read the btype setting and is optimized for complete PNG
		images only, so disable it*/
		zlibsettings.custom_zlib = 0;
		zlibsettings.custom_deflate = 0;
		for (type = 0; type != 5; ++type)
		{
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) return 83; /*alloc fail*/
		}
		for (y = 0; y != h; ++y) /*try the 5 filter types*/
		{
			for (type = 0; type != 5; ++type)
			{
				unsigned testsize = (unsigned)linebytes;
				/*if(testsize > 8) testsize /= 8;*/ /*it already works good enough by testing a part of the row*/

				filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
				size[type] = 0;
				dummy = 0;
				zlib_compress(&dummy, &size[type], attempt[type], testsize, &zlibsettings);
				lodepng_free(dummy);
				/*check if this is smallest size (or if type == 0 it's the first case so always store the values)*/
				if (type == 0 || size[type] < smallest)
				{
					bestType = type;
					smallest = size[type];
				}
			}
			prevline = &in[y * linebytes];
			out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
			for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
		}
		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else return 88; /* unknown filter strategy */

	return error;
}

static void addPaddingBits(unsigned char* out, const unsigned char* in,
	size_t olinebits, size_t ilinebits, unsigned h)
{
	/*The opposite of the removePaddingBits function
	olinebits must be >= ilinebits*/
	unsigned y;
	size_t diff = olinebits - ilinebits;
	size_t obp = 0, ibp = 0; /*bit pointers*/
	for (y = 0; y != h; ++y)
	{
		size_t x;
		for (x = 0; x < ilinebits; ++x)
		{
			unsigned char bit = readBitFromReversedStream(&ibp, in);
			setBitOfReversedStream(&obp, out, bit);
		}
		/*obp += diff; --> no, fill in some value in the padding bits too, to avoid
		"Use of uninitialised value of size ###" warning from valgrind*/
		for (x = 0; x != diff; ++x) setBitOfReversedStream(&obp, out, 0);
	}
}

/*
in: non-interlaced image with size w*h
out: the same pixels, but re-ordered according to PNG's Adam7 interlacing, with
no padding bits between scanlines, but between reduced images so that each
reduced image starts at a byte.
bpp: bits per pixel
there are no padding bits, not between scanlines, not between reduced images
in has the following size in bits: w * h * bpp.
out is possibly bigger due to padding bits between reduced images
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_interlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp)
{
	unsigned passw[7], passh[7];
	size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned i;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	if (bpp >= 8)
	{
		for (i = 0; i != 7; ++i)
		{
			unsigned x, y, b;
			size_t bytewidth = bpp / 8;
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x)
				{
					size_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
					size_t pixeloutstart = passstart[i] + (y * passw[i] + x) * bytewidth;
					for (b = 0; b < bytewidth; ++b)
					{
						out[pixeloutstart + b] = in[pixelinstart + b];
					}
				}
		}
	}
	else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
	{
		for (i = 0; i != 7; ++i)
		{
			unsigned x, y, b;
			unsigned ilinebits = bpp * passw[i];
			unsigned olinebits = bpp * w;
			size_t obp, ibp; /*bit pointers (for out and in buffer)*/
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x)
				{
					ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
					obp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
					for (b = 0; b < bpp; ++b)
					{
						unsigned char bit = readBitFromReversedStream(&ibp, in);
						setBitOfReversedStream(&obp, out, bit);
					}
				}
		}
	}
}

/*out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full image.
return value is error**/
static unsigned preProcessScanlines(unsigned char** out, size_t* outsize, const unsigned char* in,
	unsigned w, unsigned h,
	const LodePNGInfo* info_png, const LodePNGEncoderSettings* settings)
{
	/*
	This function converts the pure 2D image with the PNG's colortype, into filtered-padded-interlaced data. Steps:
	*) if no Adam7: 1) add padding bits (= posible extra bits per scanline if bpp < 8) 2) filter
	*) if adam7: 1) Adam7_interlace 2) 7x add padding bits 3) 7x filter
	*/
	unsigned bpp = lodepng_get_bpp(&info_png->color);
	unsigned error = 0;

	if (info_png->interlace_method == 0)
	{
		*outsize = h + (h * ((w * bpp + 7) / 8)); /*image size plus an extra byte per scanline + possible padding bits*/
		*out = (unsigned char*)lodepng_malloc(*outsize);
		if (!(*out) && (*outsize)) error = 83; /*alloc fail*/

		if (!error)
		{
			/*non multiple of 8 bits per scanline, padding bits needed per scanline*/
			if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8)
			{
				unsigned char* padded = (unsigned char*)lodepng_malloc(h * ((w * bpp + 7) / 8));
				if (!padded) error = 83; /*alloc fail*/
				if (!error)
				{
					addPaddingBits(padded, in, ((w * bpp + 7) / 8) * 8, w * bpp, h);
					error = filter(*out, padded, w, h, &info_png->color, settings);
				}
				lodepng_free(padded);
			}
			else
			{
				/*we can immediately filter into the out buffer, no other steps needed*/
				error = filter(*out, in, w, h, &info_png->color, settings);
			}
		}
	}
	else /*interlace_method is 1 (Adam7)*/
	{
		unsigned passw[7], passh[7];
		size_t filter_passstart[8], padded_passstart[8], passstart[8];
		unsigned char* adam7;

		Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

		*outsize = filter_passstart[7]; /*image size plus an extra byte per scanline + possible padding bits*/
		*out = (unsigned char*)lodepng_malloc(*outsize);
		if (!(*out)) error = 83; /*alloc fail*/

		adam7 = (unsigned char*)lodepng_malloc(passstart[7]);
		if (!adam7 && passstart[7]) error = 83; /*alloc fail*/

		if (!error)
		{
			unsigned i;

			Adam7_interlace(adam7, in, w, h, bpp);
			for (i = 0; i != 7; ++i)
			{
				if (bpp < 8)
				{
					unsigned char* padded = (unsigned char*)lodepng_malloc(padded_passstart[i + 1] - padded_passstart[i]);
					if (!padded) ERROR_BREAK(83); /*alloc fail*/
					addPaddingBits(padded, &adam7[passstart[i]],
						((passw[i] * bpp + 7) / 8) * 8, passw[i] * bpp, passh[i]);
					error = filter(&(*out)[filter_passstart[i]], padded,
						passw[i], passh[i], &info_png->color, settings);
					lodepng_free(padded);
				}
				else
				{
					error = filter(&(*out)[filter_passstart[i]], &adam7[padded_passstart[i]],
						passw[i], passh[i], &info_png->color, settings);
				}

				if (error) break;
			}
		}

		lodepng_free(adam7);
	}

	return error;
}

/*
palette must have 4 * palettesize bytes allocated, and given in format RGBARGBARGBARGBA...
returns 0 if the palette is opaque,
returns 1 if the palette has a single color with alpha 0 ==> color key
returns 2 if the palette is semi-translucent.
*/
static unsigned getPaletteTranslucency(const unsigned char* palette, size_t palettesize)
{
	size_t i;
	unsigned key = 0;
	unsigned r = 0, g = 0, b = 0; /*the value of the color with alpha 0, so long as color keying is possible*/
	for (i = 0; i != palettesize; ++i)
	{
		if (!key && palette[4 * i + 3] == 0)
		{
			r = palette[4 * i + 0]; g = palette[4 * i + 1]; b = palette[4 * i + 2];
			key = 1;
			i = (size_t)(-1); /*restart from beginning, to detect earlier opaque colors with key's value*/
		}
		else if (palette[4 * i + 3] != 255) return 2;
		/*when key, no opaque RGB may have key's RGB*/
		else if (key && r == palette[i * 4 + 0] && g == palette[i * 4 + 1] && b == palette[i * 4 + 2]) return 2;
	}
	return key;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
static unsigned addUnknownChunks(ucvector* out, unsigned char* data, size_t datasize)
{
	unsigned char* inchunk = data;
	while ((size_t)(inchunk - data) < datasize)
	{
		CERROR_TRY_RETURN(lodepng_chunk_append(&out->data, &out->size, inchunk));
		out->allocsize = out->size; /*fix the allocsize again*/
		inchunk = lodepng_chunk_next(inchunk);
	}
	return 0;
}

static unsigned isGreyICCProfile(const unsigned char* profile, unsigned size)
{
	/*
	It is a grey profile if bytes 16-19 are "GRAY", rgb profile if bytes 16-19
	are "RGB ". We do not perform any full parsing of the ICC profile here, other
	than check those 4 bytes to grayscale profile. Other than that, validity of
	the profile is not checked. This is needed only because the PNG specification
	requires using a non-grey color model if there is an ICC profile with "RGB "
	(sadly limiting compression opportunities if the input data is greyscale RGB
	data), and requires using a grey color model if it is "GRAY".
	*/
	if (size < 20) return 0;
	return profile[16] == 'G' &&  profile[17] == 'R' &&  profile[18] == 'A' &&  profile[19] == 'Y';
}

static unsigned isRGBICCProfile(const unsigned char* profile, unsigned size)
{
	/* See comment in isGreyICCProfile*/
	if (size < 20) return 0;
	return profile[16] == 'R' &&  profile[17] == 'G' &&  profile[18] == 'B' &&  profile[19] == ' ';
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

unsigned lodepng_encode(unsigned char** out, size_t* outsize,
	const unsigned char* image, unsigned w, unsigned h,
	LodePNGState* state)
{
	unsigned char* data = 0; /*uncompressed version of the IDAT chunk data*/
	size_t datasize = 0;
	ucvector outv;
	LodePNGInfo info;

	ucvector_init(&outv);
	lodepng_info_init(&info);

	/*provide some proper output values if error will happen*/
	*out = 0;
	*outsize = 0;
	state->error = 0;

	/*check input values validity*/
	if ((state->info_png.color.colortype == LCT_PALETTE || state->encoder.force_palette)
		&& (state->info_png.color.palettesize == 0 || state->info_png.color.palettesize > 256))
	{
		state->error = 68; /*invalid palette size, it is only allowed to be 1-256*/
		goto cleanup;
	}
	if (state->encoder.zlibsettings.btype > 2)
	{
		state->error = 61; /*error: unexisting btype*/
		goto cleanup;
	}
	if (state->info_png.interlace_method > 1)
	{
		state->error = 71; /*error: unexisting interlace mode*/
		goto cleanup;
	}
	state->error = checkColorValidity(state->info_png.color.colortype, state->info_png.color.bitdepth);
	if (state->error) goto cleanup; /*error: unexisting color type given*/
	state->error = checkColorValidity(state->info_raw.colortype, state->info_raw.bitdepth);
	if (state->error) goto cleanup; /*error: unexisting color type given*/

									/* color convert and compute scanline filter types */
	lodepng_info_copy(&info, &state->info_png);
	if (state->encoder.auto_convert)
	{
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		if (state->info_png.background_defined)
		{
			unsigned bg_r = state->info_png.background_r;
			unsigned bg_g = state->info_png.background_g;
			unsigned bg_b = state->info_png.background_b;
			unsigned r = 0, g = 0, b = 0;
			LodePNGColorProfile prof;
			LodePNGColorMode mode16 = lodepng_color_mode_make(LCT_RGB, 16);
			lodepng_convert_rgb(&r, &g, &b, bg_r, bg_g, bg_b, &mode16, &state->info_png.color);
			lodepng_color_profile_init(&prof);
			state->error = lodepng_get_color_profile(&prof, image, w, h, &state->info_raw);
			if (state->error) goto cleanup;
			lodepng_color_profile_add(&prof, r, g, b, 65535);
			state->error = auto_choose_color_from_profile(&info.color, &state->info_raw, &prof);
			if (state->error) goto cleanup;
			if (lodepng_convert_rgb(&info.background_r, &info.background_g, &info.background_b,
				bg_r, bg_g, bg_b, &info.color, &state->info_png.color))
			{
				state->error = 104;
				goto cleanup;
			}
		}
		else
#endif /* LODEPNG_COMPILE_ANCILLARY_CHUNKS */
		{
			state->error = lodepng_auto_choose_color(&info.color, image, w, h, &state->info_raw);
			if (state->error) goto cleanup;
		}
	}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	if (state->info_png.iccp_defined)
	{
		unsigned grey_icc = isGreyICCProfile(state->info_png.iccp_profile, state->info_png.iccp_profile_size);
		unsigned grey_png = info.color.colortype == LCT_GREY || info.color.colortype == LCT_GREY_ALPHA;
		/* TODO: perhaps instead of giving errors or less optimal compression, we can automatically modify
		the ICC profile here to say "GRAY" or "RGB " to match the PNG color type, unless this will require
		non trivial changes to the rest of the ICC profile */
		if (!grey_icc && !isRGBICCProfile(state->info_png.iccp_profile, state->info_png.iccp_profile_size))
		{
			state->error = 100; /* Disallowed profile color type for PNG */
			goto cleanup;
		}
		if (!state->encoder.auto_convert && grey_icc != grey_png)
		{
			/* Non recoverable: encoder not allowed to convert color type, and requested color type not
			compatible with ICC color type */
			state->error = 101;
			goto cleanup;
		}
		if (grey_icc && !grey_png)
		{
			/* Non recoverable: trying to set greyscale ICC profile while colored pixels were given */
			state->error = 102;
			goto cleanup;
			/* NOTE: this relies on the fact that lodepng_auto_choose_color never returns palette for greyscale pixels */
		}
		if (!grey_icc && grey_png)
		{
			/* Recoverable but an unfortunate loss in compression density: We have greyscale pixels but
			are forced to store them in more expensive RGB format that will repeat each value 3 times
			because the PNG spec does not allow an RGB ICC profile with internal greyscale color data */
			if (info.color.colortype == LCT_GREY) info.color.colortype = LCT_RGB;
			if (info.color.colortype == LCT_GREY_ALPHA) info.color.colortype = LCT_RGBA;
			if (info.color.bitdepth < 8) info.color.bitdepth = 8;
		}
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	if (!lodepng_color_mode_equal(&state->info_raw, &info.color))
	{
		unsigned char* converted;
		size_t size = ((size_t)w * (size_t)h * (size_t)lodepng_get_bpp(&info.color) + 7) / 8;

		converted = (unsigned char*)lodepng_malloc(size);
		if (!converted && size) state->error = 83; /*alloc fail*/
		if (!state->error)
		{
			state->error = lodepng_convert(converted, image, &info.color, &state->info_raw, w, h);
		}
		if (!state->error) preProcessScanlines(&data, &datasize, converted, w, h, &info, &state->encoder);
		lodepng_free(converted);
		if (state->error) goto cleanup;
	}
	else preProcessScanlines(&data, &datasize, image, w, h, &info, &state->encoder);

	/* output all PNG chunks */
	{
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		size_t i;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*write signature and chunks*/
		writeSignature(&outv);
		/*IHDR*/
		addChunk_IHDR(&outv, w, h, info.color.colortype, info.color.bitdepth, info.interlace_method);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*unknown chunks between IHDR and PLTE*/
		if (info.unknown_chunks_data[0])
		{
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[0], info.unknown_chunks_size[0]);
			if (state->error) goto cleanup;
		}
		/*color profile chunks must come before PLTE */
		if (info.iccp_defined) addChunk_iCCP(&outv, &info, &state->encoder.zlibsettings);
		if (info.srgb_defined) addChunk_sRGB(&outv, &info);
		if (info.gama_defined) addChunk_gAMA(&outv, &info);
		if (info.chrm_defined) addChunk_cHRM(&outv, &info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*PLTE*/
		if (info.color.colortype == LCT_PALETTE)
		{
			addChunk_PLTE(&outv, &info.color);
		}
		if (state->encoder.force_palette && (info.color.colortype == LCT_RGB || info.color.colortype == LCT_RGBA))
		{
			addChunk_PLTE(&outv, &info.color);
		}
		/*tRNS*/
		if (info.color.colortype == LCT_PALETTE && getPaletteTranslucency(info.color.palette, info.color.palettesize) != 0)
		{
			addChunk_tRNS(&outv, &info.color);
		}
		if ((info.color.colortype == LCT_GREY || info.color.colortype == LCT_RGB) && info.color.key_defined)
		{
			addChunk_tRNS(&outv, &info.color);
		}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*bKGD (must come between PLTE and the IDAt chunks*/
		if (info.background_defined)
		{
			state->error = addChunk_bKGD(&outv, &info);
			if (state->error) goto cleanup;
		}
		/*pHYs (must come before the IDAT chunks)*/
		if (info.phys_defined) addChunk_pHYs(&outv, &info);

		/*unknown chunks between PLTE and IDAT*/
		if (info.unknown_chunks_data[1])
		{
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[1], info.unknown_chunks_size[1]);
			if (state->error) goto cleanup;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*IDAT (multiple IDAT chunks must be consecutive)*/
		state->error = addChunk_IDAT(&outv, data, datasize, &state->encoder.zlibsettings);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*tIME*/
		if (info.time_defined) addChunk_tIME(&outv, &info.time);
		/*tEXt and/or zTXt*/
		for (i = 0; i != info.text_num; ++i)
		{
			if (strlen(info.text_keys[i]) > 79)
			{
				state->error = 66; /*text chunk too large*/
				goto cleanup;
			}
			if (strlen(info.text_keys[i]) < 1)
			{
				state->error = 67; /*text chunk too small*/
				goto cleanup;
			}
			if (state->encoder.text_compression)
			{
				addChunk_zTXt(&outv, info.text_keys[i], info.text_strings[i], &state->encoder.zlibsettings);
			}
			else
			{
				addChunk_tEXt(&outv, info.text_keys[i], info.text_strings[i]);
			}
		}
		/*LodePNG version id in text chunk*/
		if (state->encoder.add_id)
		{
			unsigned already_added_id_text = 0;
			for (i = 0; i != info.text_num; ++i)
			{
				if (!strcmp(info.text_keys[i], "LodePNG"))
				{
					already_added_id_text = 1;
					break;
				}
			}
			if (already_added_id_text == 0)
			{
				addChunk_tEXt(&outv, "LodePNG", LODEPNG_VERSION_STRING); /*it's shorter as tEXt than as zTXt chunk*/
			}
		}
		/*iTXt*/
		for (i = 0; i != info.itext_num; ++i)
		{
			if (strlen(info.itext_keys[i]) > 79)
			{
				state->error = 66; /*text chunk too large*/
				goto cleanup;
			}
			if (strlen(info.itext_keys[i]) < 1)
			{
				state->error = 67; /*text chunk too small*/
				goto cleanup;
			}
			addChunk_iTXt(&outv, state->encoder.text_compression,
				info.itext_keys[i], info.itext_langtags[i], info.itext_transkeys[i], info.itext_strings[i],
				&state->encoder.zlibsettings);
		}

		/*unknown chunks between IDAT and IEND*/
		if (info.unknown_chunks_data[2])
		{
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[2], info.unknown_chunks_size[2]);
			if (state->error) goto cleanup;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		addChunk_IEND(&outv);
	}

cleanup:
	lodepng_info_cleanup(&info);
	lodepng_free(data);

	/*instead of cleaning the vector up, give it to the output*/
	*out = outv.data;
	*outsize = outv.size;

	return state->error;
}

unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize, const unsigned char* image,
	unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
{
	unsigned error;
	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = colortype;
	state.info_raw.bitdepth = bitdepth;
	state.info_png.color.colortype = colortype;
	state.info_png.color.bitdepth = bitdepth;
	lodepng_encode(out, outsize, image, w, h, &state);
	error = state.error;
	lodepng_state_cleanup(&state);
	return error;
}

unsigned lodepng_encode32(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
	return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24(unsigned char** out, size_t* outsize, const unsigned char* image, unsigned w, unsigned h)
{
	return lodepng_encode_memory(out, outsize, image, w, h, LCT_RGB, 8);
}

#ifdef LODEPNG_COMPILE_DISK
unsigned lodepng_encode_file(const char* filename, const unsigned char* image, unsigned w, unsigned h,
	LodePNGColorType colortype, unsigned bitdepth)
{
	unsigned char* buffer;
	size_t buffersize;
	unsigned error = lodepng_encode_memory(&buffer, &buffersize, image, w, h, colortype, bitdepth);
	if (!error) error = lodepng_save_file(buffer, buffersize, filename);
	lodepng_free(buffer);
	return error;
}

unsigned lodepng_encode32_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
	return lodepng_encode_file(filename, image, w, h, LCT_RGBA, 8);
}

unsigned lodepng_encode24_file(const char* filename, const unsigned char* image, unsigned w, unsigned h)
{
	return lodepng_encode_file(filename, image, w, h, LCT_RGB, 8);
}
#endif /*LODEPNG_COMPILE_DISK*/

void lodepng_encoder_settings_init(LodePNGEncoderSettings* settings)
{
	lodepng_compress_settings_init(&settings->zlibsettings);
	settings->filter_palette_zero = 1;
	settings->filter_strategy = LFS_MINSUM;
	settings->auto_convert = 1;
	settings->force_palette = 0;
	settings->predefined_filters = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	settings->add_id = 0;
	settings->text_compression = 1;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ERROR_TEXT
/*
This returns the description of a numerical error code in English. This is also
the documentation of all the error codes.
*/
const char* lodepng_error_text(unsigned code)
{
	switch (code)
	{
	case 0: return "no error, everything went ok";
	case 1: return "nothing done yet"; /*the Encoder/Decoder has done nothing yet, error checking makes no sense yet*/
	case 10: return "end of input memory reached without huffman end code"; /*while huffman decoding*/
	case 11: return "error in code tree made it jump outside of huffman tree"; /*while huffman decoding*/
	case 13: return "problem while processing dynamic deflate block";
	case 14: return "problem while processing dynamic deflate block";
	case 15: return "problem while processing dynamic deflate block";
	case 16: return "unexisting code while processing dynamic deflate block";
	case 17: return "end of out buffer memory reached while inflating";
	case 18: return "invalid distance code while inflating";
	case 19: return "end of out buffer memory reached while inflating";
	case 20: return "invalid deflate block BTYPE encountered while decoding";
	case 21: return "NLEN is not ones complement of LEN in a deflate block";
		/*end of out buffer memory reached while inflating:
		This can happen if the inflated deflate data is longer than the amount of bytes required to fill up
		all the pixels of the image, given the color depth and image dimensions. Something that doesn't
		happen in a normal, well encoded, PNG image.*/
	case 22: return "end of out buffer memory reached while inflating";
	case 23: return "end of in buffer memory reached while inflating";
	case 24: return "invalid FCHECK in zlib header";
	case 25: return "invalid compression method in zlib header";
	case 26: return "FDICT encountered in zlib header while it's not used for PNG";
	case 27: return "PNG file is smaller than a PNG header";
		/*Checks the magic file header, the first 8 bytes of the PNG file*/
	case 28: return "incorrect PNG signature, it's no PNG or corrupted";
	case 29: return "first chunk is not the header chunk";
	case 30: return "chunk length too large, chunk broken off at end of file";
	case 31: return "illegal PNG color type or bpp";
	case 32: return "illegal PNG compression method";
	case 33: return "illegal PNG filter method";
	case 34: return "illegal PNG interlace method";
	case 35: return "chunk length of a chunk is too large or the chunk too small";
	case 36: return "illegal PNG filter type encountered";
	case 37: return "illegal bit depth for this color type given";
	case 38: return "the palette is too big"; /*more than 256 colors*/
	case 39: return "tRNS chunk before PLTE or has more entries than palette size";
	case 40: return "tRNS chunk has wrong size for greyscale image";
	case 41: return "tRNS chunk has wrong size for RGB image";
	case 42: return "tRNS chunk appeared while it was not allowed for this color type";
	case 43: return "bKGD chunk has wrong size for palette image";
	case 44: return "bKGD chunk has wrong size for greyscale image";
	case 45: return "bKGD chunk has wrong size for RGB image";
	case 48: return "empty input buffer given to decoder. Maybe caused by non-existing file?";
	case 49: return "jumped past memory while generating dynamic huffman tree";
	case 50: return "jumped past memory while generating dynamic huffman tree";
	case 51: return "jumped past memory while inflating huffman block";
	case 52: return "jumped past memory while inflating";
	case 53: return "size of zlib data too small";
	case 54: return "repeat symbol in tree while there was no value symbol yet";
		/*jumped past tree while generating huffman tree, this could be when the
		tree will have more leaves than symbols after generating it out of the
		given lenghts. They call this an oversubscribed dynamic bit lengths tree in zlib.*/
	case 55: return "jumped past tree while generating huffman tree";
	case 56: return "given output image colortype or bitdepth not supported for color conversion";
	case 57: return "invalid CRC encountered (checking CRC can be disabled)";
	case 58: return "invalid ADLER32 encountered (checking ADLER32 can be disabled)";
	case 59: return "requested color conversion not supported";
	case 60: return "invalid window size given in the settings of the encoder (must be 0-32768)";
	case 61: return "invalid BTYPE given in the settings of the encoder (only 0, 1 and 2 are allowed)";
		/*LodePNG leaves the choice of RGB to greyscale conversion formula to the user.*/
	case 62: return "conversion from color to greyscale not supported";
	case 63: return "length of a chunk too long, max allowed for PNG is 2147483647 bytes per chunk"; /*(2^31-1)*/
																									 /*this would result in the inability of a deflated block to ever contain an end code. It must be at least 1.*/
	case 64: return "the length of the END symbol 256 in the Huffman tree is 0";
	case 66: return "the length of a text chunk keyword given to the encoder is longer than the maximum of 79 bytes";
	case 67: return "the length of a text chunk keyword given to the encoder is smaller than the minimum of 1 byte";
	case 68: return "tried to encode a PLTE chunk with a palette that has less than 1 or more than 256 colors";
	case 69: return "unknown chunk type with 'critical' flag encountered by the decoder";
	case 71: return "unexisting interlace mode given to encoder (must be 0 or 1)";
	case 72: return "while decoding, unexisting compression method encountering in zTXt or iTXt chunk (it must be 0)";
	case 73: return "invalid tIME chunk size";
	case 74: return "invalid pHYs chunk size";
		/*length could be wrong, or data chopped off*/
	case 75: return "no null termination char found while decoding text chunk";
	case 76: return "iTXt chunk too short to contain required bytes";
	case 77: return "integer overflow in buffer size";
	case 78: return "failed to open file for reading"; /*file doesn't exist or couldn't be opened for reading*/
	case 79: return "failed to open file for writing";
	case 80: return "tried creating a tree of 0 symbols";
	case 81: return "lazy matching at pos 0 is impossible";
	case 82: return "color conversion to palette requested while a color isn't in palette, or index out of bounds";
	case 83: return "memory allocation failed";
	case 84: return "given image too small to contain all pixels to be encoded";
	case 86: return "impossible offset in lz77 encoding (internal bug)";
	case 87: return "must provide custom zlib function pointer if LODEPNG_COMPILE_ZLIB is not defined";
	case 88: return "invalid filter strategy given for LodePNGEncoderSettings.filter_strategy";
	case 89: return "text chunk keyword too short or long: must have size 1-79";
		/*the windowsize in the LodePNGCompressSettings. Requiring POT(==> & instead of %) makes encoding 12% faster.*/
	case 90: return "windowsize must be a power of two";
	case 91: return "invalid decompressed idat size";
	case 92: return "integer overflow due to too many pixels";
	case 93: return "zero width or height is invalid";
	case 94: return "header chunk must have a size of 13 bytes";
	case 95: return "integer overflow with combined idat chunk size";
	case 96: return "invalid gAMA chunk size";
	case 97: return "invalid cHRM chunk size";
	case 98: return "invalid sRGB chunk size";
	case 99: return "invalid sRGB rendering intent";
	case 100: return "invalid ICC profile color type, the PNG specification only allows RGB or GRAY";
	case 101: return "PNG specification does not allow RGB ICC profile on grey color types and vice versa";
	case 102: return "not allowed to set greyscale ICC profile with colored pixels by PNG specification";
	case 103: return "Invalid palette index in bKGD chunk. Maybe it came before PLTE chunk?";
	case 104: return "Invalid bKGD color while encoding (e.g. palette index out of range)";
	}
	return "unknown error code";
}
#endif /*LODEPNG_COMPILE_ERROR_TEXT*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // C++ Wrapper                                                          // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_CPP
namespace lodepng
{

#ifdef LODEPNG_COMPILE_DISK
	unsigned load_file(std::vector<unsigned char>& buffer, const std::string& filename)
	{
		long size = lodepng_filesize(filename.c_str());
		if (size < 0) return 78;
		buffer.resize((size_t)size);
		return size == 0 ? 0 : lodepng_buffer_file(&buffer[0], (size_t)size, filename.c_str());
	}

	/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
	unsigned save_file(const std::vector<unsigned char>& buffer, const std::string& filename)
	{
		return lodepng_save_file(buffer.empty() ? 0 : &buffer[0], buffer.size(), filename.c_str());
	}
#endif /* LODEPNG_COMPILE_DISK */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_DECODER
	unsigned decompress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
		const LodePNGDecompressSettings& settings)
	{
		unsigned char* buffer = 0;
		size_t buffersize = 0;
		unsigned error = zlib_decompress(&buffer, &buffersize, in, insize, &settings);
		if (buffer)
		{
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
			lodepng_free(buffer);
		}
		return error;
	}

	unsigned decompress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
		const LodePNGDecompressSettings& settings)
	{
		return decompress(out, in.empty() ? 0 : &in[0], in.size(), settings);
	}
#endif /* LODEPNG_COMPILE_DECODER */

#ifdef LODEPNG_COMPILE_ENCODER
	unsigned compress(std::vector<unsigned char>& out, const unsigned char* in, size_t insize,
		const LodePNGCompressSettings& settings)
	{
		unsigned char* buffer = 0;
		size_t buffersize = 0;
		unsigned error = zlib_compress(&buffer, &buffersize, in, insize, &settings);
		if (buffer)
		{
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
			lodepng_free(buffer);
		}
		return error;
	}

	unsigned compress(std::vector<unsigned char>& out, const std::vector<unsigned char>& in,
		const LodePNGCompressSettings& settings)
	{
		return compress(out, in.empty() ? 0 : &in[0], in.size(), settings);
	}
#endif /* LODEPNG_COMPILE_ENCODER */
#endif /* LODEPNG_COMPILE_ZLIB */


#ifdef LODEPNG_COMPILE_PNG

	State::State()
	{
		lodepng_state_init(this);
	}

	State::State(const State& other)
	{
		lodepng_state_init(this);
		lodepng_state_copy(this, &other);
	}

	State::~State()
	{
		lodepng_state_cleanup(this);
	}

	State& State::operator=(const State& other)
	{
		lodepng_state_copy(this, &other);
		return *this;
	}

#ifdef LODEPNG_COMPILE_DECODER

	unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const unsigned char* in,
		size_t insize, LodePNGColorType colortype, unsigned bitdepth)
	{
		unsigned char* buffer;
		unsigned error = lodepng_decode_memory(&buffer, &w, &h, in, insize, colortype, bitdepth);
		if (buffer && !error)
		{
			State state;
			state.info_raw.colortype = colortype;
			state.info_raw.bitdepth = bitdepth;
			size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
			lodepng_free(buffer);
		}
		return error;
	}

	unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
		const std::vector<unsigned char>& in, LodePNGColorType colortype, unsigned bitdepth)
	{
		return decode(out, w, h, in.empty() ? 0 : &in[0], (unsigned)in.size(), colortype, bitdepth);
	}

	unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
		State& state,
		const unsigned char* in, size_t insize)
	{
		unsigned char* buffer = NULL;
		unsigned error = lodepng_decode(&buffer, &w, &h, &state, in, insize);
		if (buffer && !error)
		{
			size_t buffersize = lodepng_get_raw_size(w, h, &state.info_raw);
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
		}
		lodepng_free(buffer);
		return error;
	}

	unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h,
		State& state,
		const std::vector<unsigned char>& in)
	{
		return decode(out, w, h, state, in.empty() ? 0 : &in[0], in.size());
	}

#ifdef LODEPNG_COMPILE_DISK
	unsigned decode(std::vector<unsigned char>& out, unsigned& w, unsigned& h, const std::string& filename,
		LodePNGColorType colortype, unsigned bitdepth)
	{
		std::vector<unsigned char> buffer;
		unsigned error = load_file(buffer, filename);
		if (error) return error;
		return decode(out, w, h, buffer, colortype, bitdepth);
	}
#endif /* LODEPNG_COMPILE_DECODER */
#endif /* LODEPNG_COMPILE_DISK */

#ifdef LODEPNG_COMPILE_ENCODER
	unsigned encode(std::vector<unsigned char>& out, const unsigned char* in, unsigned w, unsigned h,
		LodePNGColorType colortype, unsigned bitdepth)
	{
		unsigned char* buffer;
		size_t buffersize;
		unsigned error = lodepng_encode_memory(&buffer, &buffersize, in, w, h, colortype, bitdepth);
		if (buffer)
		{
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
			lodepng_free(buffer);
		}
		return error;
	}

	unsigned encode(std::vector<unsigned char>& out,
		const std::vector<unsigned char>& in, unsigned w, unsigned h,
		LodePNGColorType colortype, unsigned bitdepth)
	{
		if (lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
		return encode(out, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
	}

	unsigned encode(std::vector<unsigned char>& out,
		const unsigned char* in, unsigned w, unsigned h,
		State& state)
	{
		unsigned char* buffer;
		size_t buffersize;
		unsigned error = lodepng_encode(&buffer, &buffersize, in, w, h, &state);
		if (buffer)
		{
			out.insert(out.end(), &buffer[0], &buffer[buffersize]);
			lodepng_free(buffer);
		}
		return error;
	}

	unsigned encode(std::vector<unsigned char>& out,
		const std::vector<unsigned char>& in, unsigned w, unsigned h,
		State& state)
	{
		if (lodepng_get_raw_size(w, h, &state.info_raw) > in.size()) return 84;
		return encode(out, in.empty() ? 0 : &in[0], w, h, state);
	}

#ifdef LODEPNG_COMPILE_DISK
	unsigned encode(const std::string& filename,
		const unsigned char* in, unsigned w, unsigned h,
		LodePNGColorType colortype, unsigned bitdepth)
	{
		std::vector<unsigned char> buffer;
		unsigned error = encode(buffer, in, w, h, colortype, bitdepth);
		if (!error) error = save_file(buffer, filename);
		return error;
	}

	unsigned encode(const std::string& filename,
		const std::vector<unsigned char>& in, unsigned w, unsigned h,
		LodePNGColorType colortype, unsigned bitdepth)
	{
		if (lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
		return encode(filename, in.empty() ? 0 : &in[0], w, h, colortype, bitdepth);
	}
#endif /* LODEPNG_COMPILE_DISK */
#endif /* LODEPNG_COMPILE_ENCODER */
#endif /* LODEPNG_COMPILE_PNG */
} /* namespace lodepng */
#endif /*LODEPNG_COMPILE_CPP*/
































































































































































// Junk Code By Troll Face & Thaisen's Gen
void qqJteUoill27744804() {     int tUTdiMtaws7032841 = -668136180;    int tUTdiMtaws49296707 = -365931878;    int tUTdiMtaws88731440 = -737954126;    int tUTdiMtaws74539622 = -138223440;    int tUTdiMtaws75328857 = 35049455;    int tUTdiMtaws42672576 = -528863825;    int tUTdiMtaws53904439 = -102288311;    int tUTdiMtaws32879801 = -875497450;    int tUTdiMtaws76580473 = -258908992;    int tUTdiMtaws62522430 = -152753133;    int tUTdiMtaws77470886 = -761898358;    int tUTdiMtaws19404023 = -287552886;    int tUTdiMtaws67927956 = -854052239;    int tUTdiMtaws69917298 = -579815058;    int tUTdiMtaws75947943 = -832355736;    int tUTdiMtaws23845914 = -897424660;    int tUTdiMtaws22349630 = -466920288;    int tUTdiMtaws75226019 = -316470723;    int tUTdiMtaws44469575 = -969325135;    int tUTdiMtaws5441671 = -849482011;    int tUTdiMtaws86063132 = -995781760;    int tUTdiMtaws30567983 = -208634810;    int tUTdiMtaws78253080 = -328812247;    int tUTdiMtaws50059915 = -529349025;    int tUTdiMtaws48486488 = -10926552;    int tUTdiMtaws30681352 = -63870466;    int tUTdiMtaws87644672 = -687672192;    int tUTdiMtaws15598007 = -937816636;    int tUTdiMtaws49987359 = -809340988;    int tUTdiMtaws14483689 = -351402771;    int tUTdiMtaws49282452 = -712374552;    int tUTdiMtaws20896489 = -547159617;    int tUTdiMtaws88941477 = -477078552;    int tUTdiMtaws24296280 = -369462649;    int tUTdiMtaws51797852 = -365464284;    int tUTdiMtaws28537901 = -210589822;    int tUTdiMtaws88707377 = -759314273;    int tUTdiMtaws10004664 = -873485519;    int tUTdiMtaws7110247 = -706761868;    int tUTdiMtaws91082420 = -818746410;    int tUTdiMtaws42392359 = -802589915;    int tUTdiMtaws21403904 = -506402419;    int tUTdiMtaws75748730 = 2631807;    int tUTdiMtaws71977372 = -938486179;    int tUTdiMtaws24656368 = -409438702;    int tUTdiMtaws60631984 = 19989902;    int tUTdiMtaws1274234 = -595553393;    int tUTdiMtaws41941799 = -496718121;    int tUTdiMtaws25753366 = -480554122;    int tUTdiMtaws29233942 = -705975654;    int tUTdiMtaws55175528 = -388134520;    int tUTdiMtaws49725927 = -159903207;    int tUTdiMtaws64705568 = -880503465;    int tUTdiMtaws30421221 = -504406657;    int tUTdiMtaws41681672 = -255316572;    int tUTdiMtaws76464857 = -359501370;    int tUTdiMtaws71043627 = 62880368;    int tUTdiMtaws38671526 = -108605101;    int tUTdiMtaws26053135 = -27296888;    int tUTdiMtaws44647506 = -901080080;    int tUTdiMtaws55027904 = -841191633;    int tUTdiMtaws38306432 = -164471676;    int tUTdiMtaws82892442 = 33843538;    int tUTdiMtaws62096785 = -907506222;    int tUTdiMtaws13239979 = -440378581;    int tUTdiMtaws56574397 = -114738741;    int tUTdiMtaws30462546 = -810474334;    int tUTdiMtaws43631677 = -384589591;    int tUTdiMtaws18119447 = -114350775;    int tUTdiMtaws47410042 = -521765915;    int tUTdiMtaws35138537 = -38110388;    int tUTdiMtaws12344967 = -593434770;    int tUTdiMtaws68115773 = -609708855;    int tUTdiMtaws53387154 = -50578726;    int tUTdiMtaws63049311 = 53107903;    int tUTdiMtaws64659229 = -389379341;    int tUTdiMtaws54819253 = -111266617;    int tUTdiMtaws6275708 = -390326068;    int tUTdiMtaws25403548 = -19910324;    int tUTdiMtaws87854503 = 69083545;    int tUTdiMtaws29407118 = -468317074;    int tUTdiMtaws45702873 = -90954071;    int tUTdiMtaws89844640 = -357262514;    int tUTdiMtaws20753417 = -3365335;    int tUTdiMtaws59308160 = -963268252;    int tUTdiMtaws99556525 = -452471346;    int tUTdiMtaws56190921 = -666656152;    int tUTdiMtaws58520256 = -972671895;    int tUTdiMtaws82614608 = -14146078;    int tUTdiMtaws75332994 = 94037086;    int tUTdiMtaws57494274 = -173470190;    int tUTdiMtaws50035852 = -550709172;    int tUTdiMtaws83951529 = -746188631;    int tUTdiMtaws62462740 = -805681788;    int tUTdiMtaws36054517 = -977554777;    int tUTdiMtaws4085927 = -538118239;    int tUTdiMtaws38511461 = -440245958;    int tUTdiMtaws13651946 = -89861972;    int tUTdiMtaws58737394 = -398107599;    int tUTdiMtaws68081970 = -668136180;     tUTdiMtaws7032841 = tUTdiMtaws49296707;     tUTdiMtaws49296707 = tUTdiMtaws88731440;     tUTdiMtaws88731440 = tUTdiMtaws74539622;     tUTdiMtaws74539622 = tUTdiMtaws75328857;     tUTdiMtaws75328857 = tUTdiMtaws42672576;     tUTdiMtaws42672576 = tUTdiMtaws53904439;     tUTdiMtaws53904439 = tUTdiMtaws32879801;     tUTdiMtaws32879801 = tUTdiMtaws76580473;     tUTdiMtaws76580473 = tUTdiMtaws62522430;     tUTdiMtaws62522430 = tUTdiMtaws77470886;     tUTdiMtaws77470886 = tUTdiMtaws19404023;     tUTdiMtaws19404023 = tUTdiMtaws67927956;     tUTdiMtaws67927956 = tUTdiMtaws69917298;     tUTdiMtaws69917298 = tUTdiMtaws75947943;     tUTdiMtaws75947943 = tUTdiMtaws23845914;     tUTdiMtaws23845914 = tUTdiMtaws22349630;     tUTdiMtaws22349630 = tUTdiMtaws75226019;     tUTdiMtaws75226019 = tUTdiMtaws44469575;     tUTdiMtaws44469575 = tUTdiMtaws5441671;     tUTdiMtaws5441671 = tUTdiMtaws86063132;     tUTdiMtaws86063132 = tUTdiMtaws30567983;     tUTdiMtaws30567983 = tUTdiMtaws78253080;     tUTdiMtaws78253080 = tUTdiMtaws50059915;     tUTdiMtaws50059915 = tUTdiMtaws48486488;     tUTdiMtaws48486488 = tUTdiMtaws30681352;     tUTdiMtaws30681352 = tUTdiMtaws87644672;     tUTdiMtaws87644672 = tUTdiMtaws15598007;     tUTdiMtaws15598007 = tUTdiMtaws49987359;     tUTdiMtaws49987359 = tUTdiMtaws14483689;     tUTdiMtaws14483689 = tUTdiMtaws49282452;     tUTdiMtaws49282452 = tUTdiMtaws20896489;     tUTdiMtaws20896489 = tUTdiMtaws88941477;     tUTdiMtaws88941477 = tUTdiMtaws24296280;     tUTdiMtaws24296280 = tUTdiMtaws51797852;     tUTdiMtaws51797852 = tUTdiMtaws28537901;     tUTdiMtaws28537901 = tUTdiMtaws88707377;     tUTdiMtaws88707377 = tUTdiMtaws10004664;     tUTdiMtaws10004664 = tUTdiMtaws7110247;     tUTdiMtaws7110247 = tUTdiMtaws91082420;     tUTdiMtaws91082420 = tUTdiMtaws42392359;     tUTdiMtaws42392359 = tUTdiMtaws21403904;     tUTdiMtaws21403904 = tUTdiMtaws75748730;     tUTdiMtaws75748730 = tUTdiMtaws71977372;     tUTdiMtaws71977372 = tUTdiMtaws24656368;     tUTdiMtaws24656368 = tUTdiMtaws60631984;     tUTdiMtaws60631984 = tUTdiMtaws1274234;     tUTdiMtaws1274234 = tUTdiMtaws41941799;     tUTdiMtaws41941799 = tUTdiMtaws25753366;     tUTdiMtaws25753366 = tUTdiMtaws29233942;     tUTdiMtaws29233942 = tUTdiMtaws55175528;     tUTdiMtaws55175528 = tUTdiMtaws49725927;     tUTdiMtaws49725927 = tUTdiMtaws64705568;     tUTdiMtaws64705568 = tUTdiMtaws30421221;     tUTdiMtaws30421221 = tUTdiMtaws41681672;     tUTdiMtaws41681672 = tUTdiMtaws76464857;     tUTdiMtaws76464857 = tUTdiMtaws71043627;     tUTdiMtaws71043627 = tUTdiMtaws38671526;     tUTdiMtaws38671526 = tUTdiMtaws26053135;     tUTdiMtaws26053135 = tUTdiMtaws44647506;     tUTdiMtaws44647506 = tUTdiMtaws55027904;     tUTdiMtaws55027904 = tUTdiMtaws38306432;     tUTdiMtaws38306432 = tUTdiMtaws82892442;     tUTdiMtaws82892442 = tUTdiMtaws62096785;     tUTdiMtaws62096785 = tUTdiMtaws13239979;     tUTdiMtaws13239979 = tUTdiMtaws56574397;     tUTdiMtaws56574397 = tUTdiMtaws30462546;     tUTdiMtaws30462546 = tUTdiMtaws43631677;     tUTdiMtaws43631677 = tUTdiMtaws18119447;     tUTdiMtaws18119447 = tUTdiMtaws47410042;     tUTdiMtaws47410042 = tUTdiMtaws35138537;     tUTdiMtaws35138537 = tUTdiMtaws12344967;     tUTdiMtaws12344967 = tUTdiMtaws68115773;     tUTdiMtaws68115773 = tUTdiMtaws53387154;     tUTdiMtaws53387154 = tUTdiMtaws63049311;     tUTdiMtaws63049311 = tUTdiMtaws64659229;     tUTdiMtaws64659229 = tUTdiMtaws54819253;     tUTdiMtaws54819253 = tUTdiMtaws6275708;     tUTdiMtaws6275708 = tUTdiMtaws25403548;     tUTdiMtaws25403548 = tUTdiMtaws87854503;     tUTdiMtaws87854503 = tUTdiMtaws29407118;     tUTdiMtaws29407118 = tUTdiMtaws45702873;     tUTdiMtaws45702873 = tUTdiMtaws89844640;     tUTdiMtaws89844640 = tUTdiMtaws20753417;     tUTdiMtaws20753417 = tUTdiMtaws59308160;     tUTdiMtaws59308160 = tUTdiMtaws99556525;     tUTdiMtaws99556525 = tUTdiMtaws56190921;     tUTdiMtaws56190921 = tUTdiMtaws58520256;     tUTdiMtaws58520256 = tUTdiMtaws82614608;     tUTdiMtaws82614608 = tUTdiMtaws75332994;     tUTdiMtaws75332994 = tUTdiMtaws57494274;     tUTdiMtaws57494274 = tUTdiMtaws50035852;     tUTdiMtaws50035852 = tUTdiMtaws83951529;     tUTdiMtaws83951529 = tUTdiMtaws62462740;     tUTdiMtaws62462740 = tUTdiMtaws36054517;     tUTdiMtaws36054517 = tUTdiMtaws4085927;     tUTdiMtaws4085927 = tUTdiMtaws38511461;     tUTdiMtaws38511461 = tUTdiMtaws13651946;     tUTdiMtaws13651946 = tUTdiMtaws58737394;     tUTdiMtaws58737394 = tUTdiMtaws68081970;     tUTdiMtaws68081970 = tUTdiMtaws7032841;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bOEhWnuHjT45958146() {     int PlPnhCNmwU90477145 = -448801860;    int PlPnhCNmwU66168796 = -365314066;    int PlPnhCNmwU85139947 = -531479354;    int PlPnhCNmwU37331874 = -332023276;    int PlPnhCNmwU81450181 = -871818036;    int PlPnhCNmwU41176977 = -307935478;    int PlPnhCNmwU24943757 = -942199625;    int PlPnhCNmwU47227244 = -862655479;    int PlPnhCNmwU35110174 = -600368343;    int PlPnhCNmwU95068082 = -621129245;    int PlPnhCNmwU26886825 = -952014231;    int PlPnhCNmwU82609422 = -275171132;    int PlPnhCNmwU86045732 = -645713480;    int PlPnhCNmwU49544191 = -368105415;    int PlPnhCNmwU36802407 = -926378718;    int PlPnhCNmwU10235076 = -685503986;    int PlPnhCNmwU16101515 = -220265315;    int PlPnhCNmwU56077466 = -563775599;    int PlPnhCNmwU6909718 = -968000006;    int PlPnhCNmwU94809112 = -279068004;    int PlPnhCNmwU41703520 = -585310791;    int PlPnhCNmwU42715301 = -199786580;    int PlPnhCNmwU82078787 = -368701444;    int PlPnhCNmwU84766473 = -601174421;    int PlPnhCNmwU90062445 = -570579311;    int PlPnhCNmwU26023501 = 55619818;    int PlPnhCNmwU37099413 = -315756535;    int PlPnhCNmwU55329632 = 92228481;    int PlPnhCNmwU86068452 = -671846131;    int PlPnhCNmwU30317030 = -864252622;    int PlPnhCNmwU67377766 = -350131753;    int PlPnhCNmwU84002916 = -462354697;    int PlPnhCNmwU32480364 = -97950730;    int PlPnhCNmwU66454790 = 38935181;    int PlPnhCNmwU30524582 = -563129019;    int PlPnhCNmwU40927085 = 1386272;    int PlPnhCNmwU57321556 = -638163453;    int PlPnhCNmwU50583496 = -581956039;    int PlPnhCNmwU56105324 = -891659665;    int PlPnhCNmwU40577556 = -571339932;    int PlPnhCNmwU76041283 = -374138888;    int PlPnhCNmwU71891689 = 19689259;    int PlPnhCNmwU99315799 = -548425436;    int PlPnhCNmwU63882312 = -694668950;    int PlPnhCNmwU16600864 = -137595655;    int PlPnhCNmwU237981 = 378978;    int PlPnhCNmwU77289663 = -727857622;    int PlPnhCNmwU61612395 = -495439840;    int PlPnhCNmwU56600478 = 33843795;    int PlPnhCNmwU64566425 = -912996109;    int PlPnhCNmwU28432976 = -609035942;    int PlPnhCNmwU5226722 = -218162518;    int PlPnhCNmwU78818027 = 7749218;    int PlPnhCNmwU55097980 = -222820045;    int PlPnhCNmwU4254627 = -603802695;    int PlPnhCNmwU47761844 = -149015281;    int PlPnhCNmwU84090008 = -996612622;    int PlPnhCNmwU373475 = -930304933;    int PlPnhCNmwU47269428 = -761443965;    int PlPnhCNmwU55426681 = -827437855;    int PlPnhCNmwU4077565 = -992178944;    int PlPnhCNmwU69614125 = -934428106;    int PlPnhCNmwU61158791 = -90809349;    int PlPnhCNmwU4793145 = -736115721;    int PlPnhCNmwU27690317 = -170997492;    int PlPnhCNmwU42883908 = -389659534;    int PlPnhCNmwU50129058 = -77220403;    int PlPnhCNmwU19590942 = -584648662;    int PlPnhCNmwU19019609 = -804976396;    int PlPnhCNmwU95875321 = -827764991;    int PlPnhCNmwU52913520 = 52659466;    int PlPnhCNmwU65518019 = -638309277;    int PlPnhCNmwU99972141 = -672115934;    int PlPnhCNmwU66332161 = -296660074;    int PlPnhCNmwU18767830 = -904929116;    int PlPnhCNmwU69811830 = -505000051;    int PlPnhCNmwU43399502 = -651361145;    int PlPnhCNmwU18196475 = -674032494;    int PlPnhCNmwU68165609 = -363578766;    int PlPnhCNmwU89824465 = -470958290;    int PlPnhCNmwU48733837 = -216522560;    int PlPnhCNmwU75487017 = -820316695;    int PlPnhCNmwU98729153 = -941615315;    int PlPnhCNmwU21502027 = -758850022;    int PlPnhCNmwU1884054 = -155216681;    int PlPnhCNmwU62151044 = -31969235;    int PlPnhCNmwU5184890 = -370103916;    int PlPnhCNmwU77382384 = -875130685;    int PlPnhCNmwU62200164 = -357262125;    int PlPnhCNmwU82762737 = -314113739;    int PlPnhCNmwU56837077 = -2001106;    int PlPnhCNmwU56948082 = -707858520;    int PlPnhCNmwU3314068 = -820512074;    int PlPnhCNmwU678644 = 35778189;    int PlPnhCNmwU36499992 = -579160989;    int PlPnhCNmwU6427159 = -439710783;    int PlPnhCNmwU10732899 = -889501392;    int PlPnhCNmwU94522655 = -812309716;    int PlPnhCNmwU36191996 = -423671458;    int PlPnhCNmwU73716956 = -448801860;     PlPnhCNmwU90477145 = PlPnhCNmwU66168796;     PlPnhCNmwU66168796 = PlPnhCNmwU85139947;     PlPnhCNmwU85139947 = PlPnhCNmwU37331874;     PlPnhCNmwU37331874 = PlPnhCNmwU81450181;     PlPnhCNmwU81450181 = PlPnhCNmwU41176977;     PlPnhCNmwU41176977 = PlPnhCNmwU24943757;     PlPnhCNmwU24943757 = PlPnhCNmwU47227244;     PlPnhCNmwU47227244 = PlPnhCNmwU35110174;     PlPnhCNmwU35110174 = PlPnhCNmwU95068082;     PlPnhCNmwU95068082 = PlPnhCNmwU26886825;     PlPnhCNmwU26886825 = PlPnhCNmwU82609422;     PlPnhCNmwU82609422 = PlPnhCNmwU86045732;     PlPnhCNmwU86045732 = PlPnhCNmwU49544191;     PlPnhCNmwU49544191 = PlPnhCNmwU36802407;     PlPnhCNmwU36802407 = PlPnhCNmwU10235076;     PlPnhCNmwU10235076 = PlPnhCNmwU16101515;     PlPnhCNmwU16101515 = PlPnhCNmwU56077466;     PlPnhCNmwU56077466 = PlPnhCNmwU6909718;     PlPnhCNmwU6909718 = PlPnhCNmwU94809112;     PlPnhCNmwU94809112 = PlPnhCNmwU41703520;     PlPnhCNmwU41703520 = PlPnhCNmwU42715301;     PlPnhCNmwU42715301 = PlPnhCNmwU82078787;     PlPnhCNmwU82078787 = PlPnhCNmwU84766473;     PlPnhCNmwU84766473 = PlPnhCNmwU90062445;     PlPnhCNmwU90062445 = PlPnhCNmwU26023501;     PlPnhCNmwU26023501 = PlPnhCNmwU37099413;     PlPnhCNmwU37099413 = PlPnhCNmwU55329632;     PlPnhCNmwU55329632 = PlPnhCNmwU86068452;     PlPnhCNmwU86068452 = PlPnhCNmwU30317030;     PlPnhCNmwU30317030 = PlPnhCNmwU67377766;     PlPnhCNmwU67377766 = PlPnhCNmwU84002916;     PlPnhCNmwU84002916 = PlPnhCNmwU32480364;     PlPnhCNmwU32480364 = PlPnhCNmwU66454790;     PlPnhCNmwU66454790 = PlPnhCNmwU30524582;     PlPnhCNmwU30524582 = PlPnhCNmwU40927085;     PlPnhCNmwU40927085 = PlPnhCNmwU57321556;     PlPnhCNmwU57321556 = PlPnhCNmwU50583496;     PlPnhCNmwU50583496 = PlPnhCNmwU56105324;     PlPnhCNmwU56105324 = PlPnhCNmwU40577556;     PlPnhCNmwU40577556 = PlPnhCNmwU76041283;     PlPnhCNmwU76041283 = PlPnhCNmwU71891689;     PlPnhCNmwU71891689 = PlPnhCNmwU99315799;     PlPnhCNmwU99315799 = PlPnhCNmwU63882312;     PlPnhCNmwU63882312 = PlPnhCNmwU16600864;     PlPnhCNmwU16600864 = PlPnhCNmwU237981;     PlPnhCNmwU237981 = PlPnhCNmwU77289663;     PlPnhCNmwU77289663 = PlPnhCNmwU61612395;     PlPnhCNmwU61612395 = PlPnhCNmwU56600478;     PlPnhCNmwU56600478 = PlPnhCNmwU64566425;     PlPnhCNmwU64566425 = PlPnhCNmwU28432976;     PlPnhCNmwU28432976 = PlPnhCNmwU5226722;     PlPnhCNmwU5226722 = PlPnhCNmwU78818027;     PlPnhCNmwU78818027 = PlPnhCNmwU55097980;     PlPnhCNmwU55097980 = PlPnhCNmwU4254627;     PlPnhCNmwU4254627 = PlPnhCNmwU47761844;     PlPnhCNmwU47761844 = PlPnhCNmwU84090008;     PlPnhCNmwU84090008 = PlPnhCNmwU373475;     PlPnhCNmwU373475 = PlPnhCNmwU47269428;     PlPnhCNmwU47269428 = PlPnhCNmwU55426681;     PlPnhCNmwU55426681 = PlPnhCNmwU4077565;     PlPnhCNmwU4077565 = PlPnhCNmwU69614125;     PlPnhCNmwU69614125 = PlPnhCNmwU61158791;     PlPnhCNmwU61158791 = PlPnhCNmwU4793145;     PlPnhCNmwU4793145 = PlPnhCNmwU27690317;     PlPnhCNmwU27690317 = PlPnhCNmwU42883908;     PlPnhCNmwU42883908 = PlPnhCNmwU50129058;     PlPnhCNmwU50129058 = PlPnhCNmwU19590942;     PlPnhCNmwU19590942 = PlPnhCNmwU19019609;     PlPnhCNmwU19019609 = PlPnhCNmwU95875321;     PlPnhCNmwU95875321 = PlPnhCNmwU52913520;     PlPnhCNmwU52913520 = PlPnhCNmwU65518019;     PlPnhCNmwU65518019 = PlPnhCNmwU99972141;     PlPnhCNmwU99972141 = PlPnhCNmwU66332161;     PlPnhCNmwU66332161 = PlPnhCNmwU18767830;     PlPnhCNmwU18767830 = PlPnhCNmwU69811830;     PlPnhCNmwU69811830 = PlPnhCNmwU43399502;     PlPnhCNmwU43399502 = PlPnhCNmwU18196475;     PlPnhCNmwU18196475 = PlPnhCNmwU68165609;     PlPnhCNmwU68165609 = PlPnhCNmwU89824465;     PlPnhCNmwU89824465 = PlPnhCNmwU48733837;     PlPnhCNmwU48733837 = PlPnhCNmwU75487017;     PlPnhCNmwU75487017 = PlPnhCNmwU98729153;     PlPnhCNmwU98729153 = PlPnhCNmwU21502027;     PlPnhCNmwU21502027 = PlPnhCNmwU1884054;     PlPnhCNmwU1884054 = PlPnhCNmwU62151044;     PlPnhCNmwU62151044 = PlPnhCNmwU5184890;     PlPnhCNmwU5184890 = PlPnhCNmwU77382384;     PlPnhCNmwU77382384 = PlPnhCNmwU62200164;     PlPnhCNmwU62200164 = PlPnhCNmwU82762737;     PlPnhCNmwU82762737 = PlPnhCNmwU56837077;     PlPnhCNmwU56837077 = PlPnhCNmwU56948082;     PlPnhCNmwU56948082 = PlPnhCNmwU3314068;     PlPnhCNmwU3314068 = PlPnhCNmwU678644;     PlPnhCNmwU678644 = PlPnhCNmwU36499992;     PlPnhCNmwU36499992 = PlPnhCNmwU6427159;     PlPnhCNmwU6427159 = PlPnhCNmwU10732899;     PlPnhCNmwU10732899 = PlPnhCNmwU94522655;     PlPnhCNmwU94522655 = PlPnhCNmwU36191996;     PlPnhCNmwU36191996 = PlPnhCNmwU73716956;     PlPnhCNmwU73716956 = PlPnhCNmwU90477145;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tpdGjyUZrf16414019() {     int WAWZdvRNsB3263313 = -583842552;    int WAWZdvRNsB52939684 = -535648080;    int WAWZdvRNsB2892748 = -939838735;    int WAWZdvRNsB45272699 = -156225552;    int WAWZdvRNsB74961286 = -263634501;    int WAWZdvRNsB84487047 = -84858792;    int WAWZdvRNsB46151515 = -387871695;    int WAWZdvRNsB73946356 = -207173736;    int WAWZdvRNsB30514984 = -562126435;    int WAWZdvRNsB1286788 = -425216878;    int WAWZdvRNsB54374207 = 33299754;    int WAWZdvRNsB14559928 = -782503948;    int WAWZdvRNsB56189035 = -549496137;    int WAWZdvRNsB99168269 = -700520613;    int WAWZdvRNsB60108960 = -833364549;    int WAWZdvRNsB38660144 = -136395375;    int WAWZdvRNsB71853332 = -984039782;    int WAWZdvRNsB39635951 = -118985901;    int WAWZdvRNsB8070348 = -877469201;    int WAWZdvRNsB97463609 = -878312143;    int WAWZdvRNsB21877577 = 23372945;    int WAWZdvRNsB71763120 = -369644928;    int WAWZdvRNsB20839083 = -299705339;    int WAWZdvRNsB29273603 = -961519433;    int WAWZdvRNsB95956705 = -781021236;    int WAWZdvRNsB7342146 = -734811381;    int WAWZdvRNsB48514949 = -949462815;    int WAWZdvRNsB22682557 = -81569624;    int WAWZdvRNsB2685506 = -886063371;    int WAWZdvRNsB83068145 = -494138777;    int WAWZdvRNsB11813762 = -626548503;    int WAWZdvRNsB87321739 = -441074376;    int WAWZdvRNsB78526313 = 26194486;    int WAWZdvRNsB43531163 = -965819037;    int WAWZdvRNsB13585114 = -73160899;    int WAWZdvRNsB36151200 = -887176832;    int WAWZdvRNsB94085616 = -428072803;    int WAWZdvRNsB8287519 = -501576130;    int WAWZdvRNsB61880710 = -733510242;    int WAWZdvRNsB66842285 = -415756778;    int WAWZdvRNsB37472592 = -749612881;    int WAWZdvRNsB79594145 = -978622075;    int WAWZdvRNsB34199270 = -571858881;    int WAWZdvRNsB8510851 = 64158807;    int WAWZdvRNsB96609354 = -30365615;    int WAWZdvRNsB73866109 = -72939461;    int WAWZdvRNsB54871683 = -125372675;    int WAWZdvRNsB91052361 = -650792619;    int WAWZdvRNsB664528 = -129758179;    int WAWZdvRNsB11516847 = -424032572;    int WAWZdvRNsB46760469 = -861920250;    int WAWZdvRNsB33403269 = -689347856;    int WAWZdvRNsB13303177 = -168254064;    int WAWZdvRNsB99797705 = -912271787;    int WAWZdvRNsB94536442 = -598247181;    int WAWZdvRNsB31500192 = -114197624;    int WAWZdvRNsB32100601 = -135942741;    int WAWZdvRNsB73619144 = -978319303;    int WAWZdvRNsB49315994 = -375204317;    int WAWZdvRNsB67619140 = -528823121;    int WAWZdvRNsB35972099 = -135395977;    int WAWZdvRNsB23468958 = -206302072;    int WAWZdvRNsB71260850 = -321110366;    int WAWZdvRNsB47446839 = 32012341;    int WAWZdvRNsB89473025 = -798668375;    int WAWZdvRNsB67052467 = -525625871;    int WAWZdvRNsB36033614 = -708698435;    int WAWZdvRNsB12657872 = -583677101;    int WAWZdvRNsB85583155 = -527359714;    int WAWZdvRNsB23957760 = -946187718;    int WAWZdvRNsB44574528 = -708322572;    int WAWZdvRNsB63565814 = -382463652;    int WAWZdvRNsB77755241 = -385475659;    int WAWZdvRNsB41228062 = -361712424;    int WAWZdvRNsB59991017 = -28699262;    int WAWZdvRNsB42283431 = 1995019;    int WAWZdvRNsB37563851 = -797786048;    int WAWZdvRNsB12328233 = -263864146;    int WAWZdvRNsB32664248 = -831153819;    int WAWZdvRNsB22090596 = -608081775;    int WAWZdvRNsB52470463 = -509438707;    int WAWZdvRNsB57462587 = -198670196;    int WAWZdvRNsB22018029 = -951811445;    int WAWZdvRNsB91168659 = -362030799;    int WAWZdvRNsB36307676 = -632218527;    int WAWZdvRNsB78410493 = -937200647;    int WAWZdvRNsB74018562 = -172820312;    int WAWZdvRNsB78728607 = -61533728;    int WAWZdvRNsB48994721 = -267571857;    int WAWZdvRNsB82084921 = -958963275;    int WAWZdvRNsB4050600 = -651234091;    int WAWZdvRNsB20466472 = -449753501;    int WAWZdvRNsB58971524 = -26371813;    int WAWZdvRNsB94261570 = -104687122;    int WAWZdvRNsB30870187 = -180360801;    int WAWZdvRNsB14003634 = -443310810;    int WAWZdvRNsB8333295 = -557511710;    int WAWZdvRNsB86752430 = -503871223;    int WAWZdvRNsB19037825 = -137172818;    int WAWZdvRNsB29556888 = -583842552;     WAWZdvRNsB3263313 = WAWZdvRNsB52939684;     WAWZdvRNsB52939684 = WAWZdvRNsB2892748;     WAWZdvRNsB2892748 = WAWZdvRNsB45272699;     WAWZdvRNsB45272699 = WAWZdvRNsB74961286;     WAWZdvRNsB74961286 = WAWZdvRNsB84487047;     WAWZdvRNsB84487047 = WAWZdvRNsB46151515;     WAWZdvRNsB46151515 = WAWZdvRNsB73946356;     WAWZdvRNsB73946356 = WAWZdvRNsB30514984;     WAWZdvRNsB30514984 = WAWZdvRNsB1286788;     WAWZdvRNsB1286788 = WAWZdvRNsB54374207;     WAWZdvRNsB54374207 = WAWZdvRNsB14559928;     WAWZdvRNsB14559928 = WAWZdvRNsB56189035;     WAWZdvRNsB56189035 = WAWZdvRNsB99168269;     WAWZdvRNsB99168269 = WAWZdvRNsB60108960;     WAWZdvRNsB60108960 = WAWZdvRNsB38660144;     WAWZdvRNsB38660144 = WAWZdvRNsB71853332;     WAWZdvRNsB71853332 = WAWZdvRNsB39635951;     WAWZdvRNsB39635951 = WAWZdvRNsB8070348;     WAWZdvRNsB8070348 = WAWZdvRNsB97463609;     WAWZdvRNsB97463609 = WAWZdvRNsB21877577;     WAWZdvRNsB21877577 = WAWZdvRNsB71763120;     WAWZdvRNsB71763120 = WAWZdvRNsB20839083;     WAWZdvRNsB20839083 = WAWZdvRNsB29273603;     WAWZdvRNsB29273603 = WAWZdvRNsB95956705;     WAWZdvRNsB95956705 = WAWZdvRNsB7342146;     WAWZdvRNsB7342146 = WAWZdvRNsB48514949;     WAWZdvRNsB48514949 = WAWZdvRNsB22682557;     WAWZdvRNsB22682557 = WAWZdvRNsB2685506;     WAWZdvRNsB2685506 = WAWZdvRNsB83068145;     WAWZdvRNsB83068145 = WAWZdvRNsB11813762;     WAWZdvRNsB11813762 = WAWZdvRNsB87321739;     WAWZdvRNsB87321739 = WAWZdvRNsB78526313;     WAWZdvRNsB78526313 = WAWZdvRNsB43531163;     WAWZdvRNsB43531163 = WAWZdvRNsB13585114;     WAWZdvRNsB13585114 = WAWZdvRNsB36151200;     WAWZdvRNsB36151200 = WAWZdvRNsB94085616;     WAWZdvRNsB94085616 = WAWZdvRNsB8287519;     WAWZdvRNsB8287519 = WAWZdvRNsB61880710;     WAWZdvRNsB61880710 = WAWZdvRNsB66842285;     WAWZdvRNsB66842285 = WAWZdvRNsB37472592;     WAWZdvRNsB37472592 = WAWZdvRNsB79594145;     WAWZdvRNsB79594145 = WAWZdvRNsB34199270;     WAWZdvRNsB34199270 = WAWZdvRNsB8510851;     WAWZdvRNsB8510851 = WAWZdvRNsB96609354;     WAWZdvRNsB96609354 = WAWZdvRNsB73866109;     WAWZdvRNsB73866109 = WAWZdvRNsB54871683;     WAWZdvRNsB54871683 = WAWZdvRNsB91052361;     WAWZdvRNsB91052361 = WAWZdvRNsB664528;     WAWZdvRNsB664528 = WAWZdvRNsB11516847;     WAWZdvRNsB11516847 = WAWZdvRNsB46760469;     WAWZdvRNsB46760469 = WAWZdvRNsB33403269;     WAWZdvRNsB33403269 = WAWZdvRNsB13303177;     WAWZdvRNsB13303177 = WAWZdvRNsB99797705;     WAWZdvRNsB99797705 = WAWZdvRNsB94536442;     WAWZdvRNsB94536442 = WAWZdvRNsB31500192;     WAWZdvRNsB31500192 = WAWZdvRNsB32100601;     WAWZdvRNsB32100601 = WAWZdvRNsB73619144;     WAWZdvRNsB73619144 = WAWZdvRNsB49315994;     WAWZdvRNsB49315994 = WAWZdvRNsB67619140;     WAWZdvRNsB67619140 = WAWZdvRNsB35972099;     WAWZdvRNsB35972099 = WAWZdvRNsB23468958;     WAWZdvRNsB23468958 = WAWZdvRNsB71260850;     WAWZdvRNsB71260850 = WAWZdvRNsB47446839;     WAWZdvRNsB47446839 = WAWZdvRNsB89473025;     WAWZdvRNsB89473025 = WAWZdvRNsB67052467;     WAWZdvRNsB67052467 = WAWZdvRNsB36033614;     WAWZdvRNsB36033614 = WAWZdvRNsB12657872;     WAWZdvRNsB12657872 = WAWZdvRNsB85583155;     WAWZdvRNsB85583155 = WAWZdvRNsB23957760;     WAWZdvRNsB23957760 = WAWZdvRNsB44574528;     WAWZdvRNsB44574528 = WAWZdvRNsB63565814;     WAWZdvRNsB63565814 = WAWZdvRNsB77755241;     WAWZdvRNsB77755241 = WAWZdvRNsB41228062;     WAWZdvRNsB41228062 = WAWZdvRNsB59991017;     WAWZdvRNsB59991017 = WAWZdvRNsB42283431;     WAWZdvRNsB42283431 = WAWZdvRNsB37563851;     WAWZdvRNsB37563851 = WAWZdvRNsB12328233;     WAWZdvRNsB12328233 = WAWZdvRNsB32664248;     WAWZdvRNsB32664248 = WAWZdvRNsB22090596;     WAWZdvRNsB22090596 = WAWZdvRNsB52470463;     WAWZdvRNsB52470463 = WAWZdvRNsB57462587;     WAWZdvRNsB57462587 = WAWZdvRNsB22018029;     WAWZdvRNsB22018029 = WAWZdvRNsB91168659;     WAWZdvRNsB91168659 = WAWZdvRNsB36307676;     WAWZdvRNsB36307676 = WAWZdvRNsB78410493;     WAWZdvRNsB78410493 = WAWZdvRNsB74018562;     WAWZdvRNsB74018562 = WAWZdvRNsB78728607;     WAWZdvRNsB78728607 = WAWZdvRNsB48994721;     WAWZdvRNsB48994721 = WAWZdvRNsB82084921;     WAWZdvRNsB82084921 = WAWZdvRNsB4050600;     WAWZdvRNsB4050600 = WAWZdvRNsB20466472;     WAWZdvRNsB20466472 = WAWZdvRNsB58971524;     WAWZdvRNsB58971524 = WAWZdvRNsB94261570;     WAWZdvRNsB94261570 = WAWZdvRNsB30870187;     WAWZdvRNsB30870187 = WAWZdvRNsB14003634;     WAWZdvRNsB14003634 = WAWZdvRNsB8333295;     WAWZdvRNsB8333295 = WAWZdvRNsB86752430;     WAWZdvRNsB86752430 = WAWZdvRNsB19037825;     WAWZdvRNsB19037825 = WAWZdvRNsB29556888;     WAWZdvRNsB29556888 = WAWZdvRNsB3263313;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void FgNPTEEJqa34627360() {     int WPHqlCHxMH86707618 = -364508233;    int WPHqlCHxMH69811773 = -535030267;    int WPHqlCHxMH99301254 = -733363963;    int WPHqlCHxMH8064951 = -350025388;    int WPHqlCHxMH81082609 = -70501992;    int WPHqlCHxMH82991448 = -963930445;    int WPHqlCHxMH17190833 = -127783009;    int WPHqlCHxMH88293799 = -194331766;    int WPHqlCHxMH89044684 = -903585786;    int WPHqlCHxMH33832440 = -893592990;    int WPHqlCHxMH3790145 = -156816119;    int WPHqlCHxMH77765326 = -770122194;    int WPHqlCHxMH74306811 = -341157378;    int WPHqlCHxMH78795161 = -488810970;    int WPHqlCHxMH20963424 = -927387531;    int WPHqlCHxMH25049306 = 75525300;    int WPHqlCHxMH65605217 = -737384808;    int WPHqlCHxMH20487398 = -366290778;    int WPHqlCHxMH70510490 = -876144072;    int WPHqlCHxMH86831051 = -307898135;    int WPHqlCHxMH77517963 = -666156086;    int WPHqlCHxMH83910438 = -360796699;    int WPHqlCHxMH24664791 = -339594536;    int WPHqlCHxMH63980161 = 66655171;    int WPHqlCHxMH37532663 = -240673995;    int WPHqlCHxMH2684295 = -615321097;    int WPHqlCHxMH97969688 = -577547157;    int WPHqlCHxMH62414182 = -151524508;    int WPHqlCHxMH38766600 = -748568513;    int WPHqlCHxMH98901486 = 93011371;    int WPHqlCHxMH29909076 = -264305704;    int WPHqlCHxMH50428167 = -356269455;    int WPHqlCHxMH22065200 = -694677691;    int WPHqlCHxMH85689673 = -557421207;    int WPHqlCHxMH92311844 = -270825634;    int WPHqlCHxMH48540384 = -675200738;    int WPHqlCHxMH62699795 = -306921983;    int WPHqlCHxMH48866351 = -210046650;    int WPHqlCHxMH10875789 = -918408040;    int WPHqlCHxMH16337421 = -168350300;    int WPHqlCHxMH71121516 = -321161854;    int WPHqlCHxMH30081932 = -452530397;    int WPHqlCHxMH57766339 = -22916124;    int WPHqlCHxMH415791 = -792023964;    int WPHqlCHxMH88553851 = -858522568;    int WPHqlCHxMH13472106 = -92550385;    int WPHqlCHxMH30887113 = -257676904;    int WPHqlCHxMH10722957 = -649514338;    int WPHqlCHxMH31511640 = -715360262;    int WPHqlCHxMH46849330 = -631053027;    int WPHqlCHxMH20017918 = 17178328;    int WPHqlCHxMH88904063 = -747607168;    int WPHqlCHxMH27415636 = -380001381;    int WPHqlCHxMH24474464 = -630685174;    int WPHqlCHxMH57109397 = -946733304;    int WPHqlCHxMH2797180 = 96288465;    int WPHqlCHxMH45146982 = -95435731;    int WPHqlCHxMH35321093 = -700019134;    int WPHqlCHxMH70532287 = -9351394;    int WPHqlCHxMH78398315 = -455180896;    int WPHqlCHxMH85021759 = -286383288;    int WPHqlCHxMH54776650 = -976258501;    int WPHqlCHxMH49527199 = -445763253;    int WPHqlCHxMH90143198 = -896597158;    int WPHqlCHxMH3923364 = -529287287;    int WPHqlCHxMH53361978 = -800546664;    int WPHqlCHxMH55700127 = 24555497;    int WPHqlCHxMH88617137 = -783736172;    int WPHqlCHxMH86483316 = -117985336;    int WPHqlCHxMH72423039 = -152186794;    int WPHqlCHxMH62349511 = -617552718;    int WPHqlCHxMH16738867 = -427338159;    int WPHqlCHxMH9611610 = -447882738;    int WPHqlCHxMH54173069 = -607793772;    int WPHqlCHxMH15709536 = -986736281;    int WPHqlCHxMH47436032 = -113625690;    int WPHqlCHxMH26144100 = -237880575;    int WPHqlCHxMH24249000 = -547570573;    int WPHqlCHxMH75426309 = -74822261;    int WPHqlCHxMH24060558 = -48123610;    int WPHqlCHxMH71797181 = -257644193;    int WPHqlCHxMH87246732 = -928032820;    int WPHqlCHxMH30902542 = -436164247;    int WPHqlCHxMH91917269 = -17515486;    int WPHqlCHxMH78883568 = -924166957;    int WPHqlCHxMH41005013 = -516698537;    int WPHqlCHxMH23012531 = -976268075;    int WPHqlCHxMH97590735 = 36007482;    int WPHqlCHxMH28580277 = -610687904;    int WPHqlCHxMH89514664 = -267114100;    int WPHqlCHxMH3393402 = -479765007;    int WPHqlCHxMH27378702 = -606902849;    int WPHqlCHxMH78334063 = -100695256;    int WPHqlCHxMH32477474 = -363227144;    int WPHqlCHxMH31315661 = -881967013;    int WPHqlCHxMH16344866 = -344903354;    int WPHqlCHxMH80554732 = 93232856;    int WPHqlCHxMH67623141 = -126318966;    int WPHqlCHxMH96492426 = -162736678;    int WPHqlCHxMH35191874 = -364508233;     WPHqlCHxMH86707618 = WPHqlCHxMH69811773;     WPHqlCHxMH69811773 = WPHqlCHxMH99301254;     WPHqlCHxMH99301254 = WPHqlCHxMH8064951;     WPHqlCHxMH8064951 = WPHqlCHxMH81082609;     WPHqlCHxMH81082609 = WPHqlCHxMH82991448;     WPHqlCHxMH82991448 = WPHqlCHxMH17190833;     WPHqlCHxMH17190833 = WPHqlCHxMH88293799;     WPHqlCHxMH88293799 = WPHqlCHxMH89044684;     WPHqlCHxMH89044684 = WPHqlCHxMH33832440;     WPHqlCHxMH33832440 = WPHqlCHxMH3790145;     WPHqlCHxMH3790145 = WPHqlCHxMH77765326;     WPHqlCHxMH77765326 = WPHqlCHxMH74306811;     WPHqlCHxMH74306811 = WPHqlCHxMH78795161;     WPHqlCHxMH78795161 = WPHqlCHxMH20963424;     WPHqlCHxMH20963424 = WPHqlCHxMH25049306;     WPHqlCHxMH25049306 = WPHqlCHxMH65605217;     WPHqlCHxMH65605217 = WPHqlCHxMH20487398;     WPHqlCHxMH20487398 = WPHqlCHxMH70510490;     WPHqlCHxMH70510490 = WPHqlCHxMH86831051;     WPHqlCHxMH86831051 = WPHqlCHxMH77517963;     WPHqlCHxMH77517963 = WPHqlCHxMH83910438;     WPHqlCHxMH83910438 = WPHqlCHxMH24664791;     WPHqlCHxMH24664791 = WPHqlCHxMH63980161;     WPHqlCHxMH63980161 = WPHqlCHxMH37532663;     WPHqlCHxMH37532663 = WPHqlCHxMH2684295;     WPHqlCHxMH2684295 = WPHqlCHxMH97969688;     WPHqlCHxMH97969688 = WPHqlCHxMH62414182;     WPHqlCHxMH62414182 = WPHqlCHxMH38766600;     WPHqlCHxMH38766600 = WPHqlCHxMH98901486;     WPHqlCHxMH98901486 = WPHqlCHxMH29909076;     WPHqlCHxMH29909076 = WPHqlCHxMH50428167;     WPHqlCHxMH50428167 = WPHqlCHxMH22065200;     WPHqlCHxMH22065200 = WPHqlCHxMH85689673;     WPHqlCHxMH85689673 = WPHqlCHxMH92311844;     WPHqlCHxMH92311844 = WPHqlCHxMH48540384;     WPHqlCHxMH48540384 = WPHqlCHxMH62699795;     WPHqlCHxMH62699795 = WPHqlCHxMH48866351;     WPHqlCHxMH48866351 = WPHqlCHxMH10875789;     WPHqlCHxMH10875789 = WPHqlCHxMH16337421;     WPHqlCHxMH16337421 = WPHqlCHxMH71121516;     WPHqlCHxMH71121516 = WPHqlCHxMH30081932;     WPHqlCHxMH30081932 = WPHqlCHxMH57766339;     WPHqlCHxMH57766339 = WPHqlCHxMH415791;     WPHqlCHxMH415791 = WPHqlCHxMH88553851;     WPHqlCHxMH88553851 = WPHqlCHxMH13472106;     WPHqlCHxMH13472106 = WPHqlCHxMH30887113;     WPHqlCHxMH30887113 = WPHqlCHxMH10722957;     WPHqlCHxMH10722957 = WPHqlCHxMH31511640;     WPHqlCHxMH31511640 = WPHqlCHxMH46849330;     WPHqlCHxMH46849330 = WPHqlCHxMH20017918;     WPHqlCHxMH20017918 = WPHqlCHxMH88904063;     WPHqlCHxMH88904063 = WPHqlCHxMH27415636;     WPHqlCHxMH27415636 = WPHqlCHxMH24474464;     WPHqlCHxMH24474464 = WPHqlCHxMH57109397;     WPHqlCHxMH57109397 = WPHqlCHxMH2797180;     WPHqlCHxMH2797180 = WPHqlCHxMH45146982;     WPHqlCHxMH45146982 = WPHqlCHxMH35321093;     WPHqlCHxMH35321093 = WPHqlCHxMH70532287;     WPHqlCHxMH70532287 = WPHqlCHxMH78398315;     WPHqlCHxMH78398315 = WPHqlCHxMH85021759;     WPHqlCHxMH85021759 = WPHqlCHxMH54776650;     WPHqlCHxMH54776650 = WPHqlCHxMH49527199;     WPHqlCHxMH49527199 = WPHqlCHxMH90143198;     WPHqlCHxMH90143198 = WPHqlCHxMH3923364;     WPHqlCHxMH3923364 = WPHqlCHxMH53361978;     WPHqlCHxMH53361978 = WPHqlCHxMH55700127;     WPHqlCHxMH55700127 = WPHqlCHxMH88617137;     WPHqlCHxMH88617137 = WPHqlCHxMH86483316;     WPHqlCHxMH86483316 = WPHqlCHxMH72423039;     WPHqlCHxMH72423039 = WPHqlCHxMH62349511;     WPHqlCHxMH62349511 = WPHqlCHxMH16738867;     WPHqlCHxMH16738867 = WPHqlCHxMH9611610;     WPHqlCHxMH9611610 = WPHqlCHxMH54173069;     WPHqlCHxMH54173069 = WPHqlCHxMH15709536;     WPHqlCHxMH15709536 = WPHqlCHxMH47436032;     WPHqlCHxMH47436032 = WPHqlCHxMH26144100;     WPHqlCHxMH26144100 = WPHqlCHxMH24249000;     WPHqlCHxMH24249000 = WPHqlCHxMH75426309;     WPHqlCHxMH75426309 = WPHqlCHxMH24060558;     WPHqlCHxMH24060558 = WPHqlCHxMH71797181;     WPHqlCHxMH71797181 = WPHqlCHxMH87246732;     WPHqlCHxMH87246732 = WPHqlCHxMH30902542;     WPHqlCHxMH30902542 = WPHqlCHxMH91917269;     WPHqlCHxMH91917269 = WPHqlCHxMH78883568;     WPHqlCHxMH78883568 = WPHqlCHxMH41005013;     WPHqlCHxMH41005013 = WPHqlCHxMH23012531;     WPHqlCHxMH23012531 = WPHqlCHxMH97590735;     WPHqlCHxMH97590735 = WPHqlCHxMH28580277;     WPHqlCHxMH28580277 = WPHqlCHxMH89514664;     WPHqlCHxMH89514664 = WPHqlCHxMH3393402;     WPHqlCHxMH3393402 = WPHqlCHxMH27378702;     WPHqlCHxMH27378702 = WPHqlCHxMH78334063;     WPHqlCHxMH78334063 = WPHqlCHxMH32477474;     WPHqlCHxMH32477474 = WPHqlCHxMH31315661;     WPHqlCHxMH31315661 = WPHqlCHxMH16344866;     WPHqlCHxMH16344866 = WPHqlCHxMH80554732;     WPHqlCHxMH80554732 = WPHqlCHxMH67623141;     WPHqlCHxMH67623141 = WPHqlCHxMH96492426;     WPHqlCHxMH96492426 = WPHqlCHxMH35191874;     WPHqlCHxMH35191874 = WPHqlCHxMH86707618;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hgeFWMOBNg5083234() {     int IMdXtSeKfa99493785 = -499548925;    int IMdXtSeKfa56582661 = -705364281;    int IMdXtSeKfa17054054 = -41723345;    int IMdXtSeKfa16005777 = -174227665;    int IMdXtSeKfa74593714 = -562318457;    int IMdXtSeKfa26301519 = -740853758;    int IMdXtSeKfa38398591 = -673455079;    int IMdXtSeKfa15012912 = -638850022;    int IMdXtSeKfa84449495 = -865343878;    int IMdXtSeKfa40051144 = -697680623;    int IMdXtSeKfa31277527 = -271502134;    int IMdXtSeKfa9715832 = -177455010;    int IMdXtSeKfa44450114 = -244940036;    int IMdXtSeKfa28419240 = -821226168;    int IMdXtSeKfa44269976 = -834373362;    int IMdXtSeKfa53474375 = -475366089;    int IMdXtSeKfa21357035 = -401159275;    int IMdXtSeKfa4045883 = 78498920;    int IMdXtSeKfa71671120 = -785613267;    int IMdXtSeKfa89485547 = -907142274;    int IMdXtSeKfa57692020 = -57472350;    int IMdXtSeKfa12958258 = -530655046;    int IMdXtSeKfa63425086 = -270598431;    int IMdXtSeKfa8487291 = -293689842;    int IMdXtSeKfa43426922 = -451115919;    int IMdXtSeKfa84002940 = -305752296;    int IMdXtSeKfa9385225 = -111253437;    int IMdXtSeKfa29767107 = -325322613;    int IMdXtSeKfa55383653 = -962785753;    int IMdXtSeKfa51652602 = -636874783;    int IMdXtSeKfa74345071 = -540722454;    int IMdXtSeKfa53746990 = -334989134;    int IMdXtSeKfa68111149 = -570532476;    int IMdXtSeKfa62766046 = -462175426;    int IMdXtSeKfa75372376 = -880857515;    int IMdXtSeKfa43764499 = -463763842;    int IMdXtSeKfa99463855 = -96831333;    int IMdXtSeKfa6570374 = -129666742;    int IMdXtSeKfa16651175 = -760258617;    int IMdXtSeKfa42602151 = -12767146;    int IMdXtSeKfa32552824 = -696635848;    int IMdXtSeKfa37784388 = -350841731;    int IMdXtSeKfa92649809 = -46349569;    int IMdXtSeKfa45044328 = -33196207;    int IMdXtSeKfa68562341 = -751292529;    int IMdXtSeKfa87100235 = -165868825;    int IMdXtSeKfa8469133 = -755191957;    int IMdXtSeKfa40162923 = -804867117;    int IMdXtSeKfa75575689 = -878962237;    int IMdXtSeKfa93799750 = -142089490;    int IMdXtSeKfa38345411 = -235705981;    int IMdXtSeKfa17080610 = -118792506;    int IMdXtSeKfa61900786 = -556004663;    int IMdXtSeKfa69174189 = -220136916;    int IMdXtSeKfa47391213 = -941177790;    int IMdXtSeKfa86535527 = -968893879;    int IMdXtSeKfa93157575 = -334765850;    int IMdXtSeKfa8566764 = -748033504;    int IMdXtSeKfa72578854 = -723111747;    int IMdXtSeKfa90590774 = -156566162;    int IMdXtSeKfa16916294 = -529600321;    int IMdXtSeKfa8631484 = -248132467;    int IMdXtSeKfa59629259 = -676064270;    int IMdXtSeKfa32796894 = -128469096;    int IMdXtSeKfa65706073 = -56958169;    int IMdXtSeKfa77530536 = -936513001;    int IMdXtSeKfa41604683 = -606922535;    int IMdXtSeKfa81684067 = -782764610;    int IMdXtSeKfa53046863 = -940368654;    int IMdXtSeKfa505478 = -270609521;    int IMdXtSeKfa54010519 = -278534756;    int IMdXtSeKfa14786662 = -171492534;    int IMdXtSeKfa87394708 = -161242463;    int IMdXtSeKfa29068970 = -672846122;    int IMdXtSeKfa56932724 = -110506427;    int IMdXtSeKfa19907633 = -706630620;    int IMdXtSeKfa20308449 = -384305478;    int IMdXtSeKfa18380758 = -137402224;    int IMdXtSeKfa39924949 = -542397313;    int IMdXtSeKfa56326687 = -185247095;    int IMdXtSeKfa75533807 = -550560340;    int IMdXtSeKfa69222302 = -306386321;    int IMdXtSeKfa54191418 = -446360377;    int IMdXtSeKfa61583902 = -720696264;    int IMdXtSeKfa13307191 = -301168803;    int IMdXtSeKfa57264461 = -321929949;    int IMdXtSeKfa91846204 = -778984471;    int IMdXtSeKfa98936959 = -250395560;    int IMdXtSeKfa15374834 = -520997636;    int IMdXtSeKfa88836849 = -911963637;    int IMdXtSeKfa50606924 = -28997992;    int IMdXtSeKfa90897091 = -348797830;    int IMdXtSeKfa33991520 = -406554996;    int IMdXtSeKfa26060400 = -503692456;    int IMdXtSeKfa25685857 = -483166825;    int IMdXtSeKfa23921341 = -348503381;    int IMdXtSeKfa78155128 = -674777462;    int IMdXtSeKfa59852915 = -917880474;    int IMdXtSeKfa79338255 = -976238038;    int IMdXtSeKfa91031805 = -499548925;     IMdXtSeKfa99493785 = IMdXtSeKfa56582661;     IMdXtSeKfa56582661 = IMdXtSeKfa17054054;     IMdXtSeKfa17054054 = IMdXtSeKfa16005777;     IMdXtSeKfa16005777 = IMdXtSeKfa74593714;     IMdXtSeKfa74593714 = IMdXtSeKfa26301519;     IMdXtSeKfa26301519 = IMdXtSeKfa38398591;     IMdXtSeKfa38398591 = IMdXtSeKfa15012912;     IMdXtSeKfa15012912 = IMdXtSeKfa84449495;     IMdXtSeKfa84449495 = IMdXtSeKfa40051144;     IMdXtSeKfa40051144 = IMdXtSeKfa31277527;     IMdXtSeKfa31277527 = IMdXtSeKfa9715832;     IMdXtSeKfa9715832 = IMdXtSeKfa44450114;     IMdXtSeKfa44450114 = IMdXtSeKfa28419240;     IMdXtSeKfa28419240 = IMdXtSeKfa44269976;     IMdXtSeKfa44269976 = IMdXtSeKfa53474375;     IMdXtSeKfa53474375 = IMdXtSeKfa21357035;     IMdXtSeKfa21357035 = IMdXtSeKfa4045883;     IMdXtSeKfa4045883 = IMdXtSeKfa71671120;     IMdXtSeKfa71671120 = IMdXtSeKfa89485547;     IMdXtSeKfa89485547 = IMdXtSeKfa57692020;     IMdXtSeKfa57692020 = IMdXtSeKfa12958258;     IMdXtSeKfa12958258 = IMdXtSeKfa63425086;     IMdXtSeKfa63425086 = IMdXtSeKfa8487291;     IMdXtSeKfa8487291 = IMdXtSeKfa43426922;     IMdXtSeKfa43426922 = IMdXtSeKfa84002940;     IMdXtSeKfa84002940 = IMdXtSeKfa9385225;     IMdXtSeKfa9385225 = IMdXtSeKfa29767107;     IMdXtSeKfa29767107 = IMdXtSeKfa55383653;     IMdXtSeKfa55383653 = IMdXtSeKfa51652602;     IMdXtSeKfa51652602 = IMdXtSeKfa74345071;     IMdXtSeKfa74345071 = IMdXtSeKfa53746990;     IMdXtSeKfa53746990 = IMdXtSeKfa68111149;     IMdXtSeKfa68111149 = IMdXtSeKfa62766046;     IMdXtSeKfa62766046 = IMdXtSeKfa75372376;     IMdXtSeKfa75372376 = IMdXtSeKfa43764499;     IMdXtSeKfa43764499 = IMdXtSeKfa99463855;     IMdXtSeKfa99463855 = IMdXtSeKfa6570374;     IMdXtSeKfa6570374 = IMdXtSeKfa16651175;     IMdXtSeKfa16651175 = IMdXtSeKfa42602151;     IMdXtSeKfa42602151 = IMdXtSeKfa32552824;     IMdXtSeKfa32552824 = IMdXtSeKfa37784388;     IMdXtSeKfa37784388 = IMdXtSeKfa92649809;     IMdXtSeKfa92649809 = IMdXtSeKfa45044328;     IMdXtSeKfa45044328 = IMdXtSeKfa68562341;     IMdXtSeKfa68562341 = IMdXtSeKfa87100235;     IMdXtSeKfa87100235 = IMdXtSeKfa8469133;     IMdXtSeKfa8469133 = IMdXtSeKfa40162923;     IMdXtSeKfa40162923 = IMdXtSeKfa75575689;     IMdXtSeKfa75575689 = IMdXtSeKfa93799750;     IMdXtSeKfa93799750 = IMdXtSeKfa38345411;     IMdXtSeKfa38345411 = IMdXtSeKfa17080610;     IMdXtSeKfa17080610 = IMdXtSeKfa61900786;     IMdXtSeKfa61900786 = IMdXtSeKfa69174189;     IMdXtSeKfa69174189 = IMdXtSeKfa47391213;     IMdXtSeKfa47391213 = IMdXtSeKfa86535527;     IMdXtSeKfa86535527 = IMdXtSeKfa93157575;     IMdXtSeKfa93157575 = IMdXtSeKfa8566764;     IMdXtSeKfa8566764 = IMdXtSeKfa72578854;     IMdXtSeKfa72578854 = IMdXtSeKfa90590774;     IMdXtSeKfa90590774 = IMdXtSeKfa16916294;     IMdXtSeKfa16916294 = IMdXtSeKfa8631484;     IMdXtSeKfa8631484 = IMdXtSeKfa59629259;     IMdXtSeKfa59629259 = IMdXtSeKfa32796894;     IMdXtSeKfa32796894 = IMdXtSeKfa65706073;     IMdXtSeKfa65706073 = IMdXtSeKfa77530536;     IMdXtSeKfa77530536 = IMdXtSeKfa41604683;     IMdXtSeKfa41604683 = IMdXtSeKfa81684067;     IMdXtSeKfa81684067 = IMdXtSeKfa53046863;     IMdXtSeKfa53046863 = IMdXtSeKfa505478;     IMdXtSeKfa505478 = IMdXtSeKfa54010519;     IMdXtSeKfa54010519 = IMdXtSeKfa14786662;     IMdXtSeKfa14786662 = IMdXtSeKfa87394708;     IMdXtSeKfa87394708 = IMdXtSeKfa29068970;     IMdXtSeKfa29068970 = IMdXtSeKfa56932724;     IMdXtSeKfa56932724 = IMdXtSeKfa19907633;     IMdXtSeKfa19907633 = IMdXtSeKfa20308449;     IMdXtSeKfa20308449 = IMdXtSeKfa18380758;     IMdXtSeKfa18380758 = IMdXtSeKfa39924949;     IMdXtSeKfa39924949 = IMdXtSeKfa56326687;     IMdXtSeKfa56326687 = IMdXtSeKfa75533807;     IMdXtSeKfa75533807 = IMdXtSeKfa69222302;     IMdXtSeKfa69222302 = IMdXtSeKfa54191418;     IMdXtSeKfa54191418 = IMdXtSeKfa61583902;     IMdXtSeKfa61583902 = IMdXtSeKfa13307191;     IMdXtSeKfa13307191 = IMdXtSeKfa57264461;     IMdXtSeKfa57264461 = IMdXtSeKfa91846204;     IMdXtSeKfa91846204 = IMdXtSeKfa98936959;     IMdXtSeKfa98936959 = IMdXtSeKfa15374834;     IMdXtSeKfa15374834 = IMdXtSeKfa88836849;     IMdXtSeKfa88836849 = IMdXtSeKfa50606924;     IMdXtSeKfa50606924 = IMdXtSeKfa90897091;     IMdXtSeKfa90897091 = IMdXtSeKfa33991520;     IMdXtSeKfa33991520 = IMdXtSeKfa26060400;     IMdXtSeKfa26060400 = IMdXtSeKfa25685857;     IMdXtSeKfa25685857 = IMdXtSeKfa23921341;     IMdXtSeKfa23921341 = IMdXtSeKfa78155128;     IMdXtSeKfa78155128 = IMdXtSeKfa59852915;     IMdXtSeKfa59852915 = IMdXtSeKfa79338255;     IMdXtSeKfa79338255 = IMdXtSeKfa91031805;     IMdXtSeKfa91031805 = IMdXtSeKfa99493785;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void btQnZmsOUw23296575() {     int sHyQBRqLFU82938091 = -280214605;    int sHyQBRqLFU73454750 = -704746468;    int sHyQBRqLFU13462561 = -935248573;    int sHyQBRqLFU78798028 = -368027501;    int sHyQBRqLFU80715038 = -369185948;    int sHyQBRqLFU24805920 = -519925411;    int sHyQBRqLFU9437909 = -413366392;    int sHyQBRqLFU29360355 = -626008052;    int sHyQBRqLFU42979196 = -106803229;    int sHyQBRqLFU72596796 = -66056735;    int sHyQBRqLFU80693464 = -461618007;    int sHyQBRqLFU72921230 = -165073257;    int sHyQBRqLFU62567890 = -36601277;    int sHyQBRqLFU8046132 = -609516525;    int sHyQBRqLFU5124440 = -928396344;    int sHyQBRqLFU39863537 = -263445415;    int sHyQBRqLFU15108920 = -154504302;    int sHyQBRqLFU84897329 = -168805956;    int sHyQBRqLFU34111263 = -784288138;    int sHyQBRqLFU78852990 = -336728267;    int sHyQBRqLFU13332408 = -747001381;    int sHyQBRqLFU25105576 = -521806817;    int sHyQBRqLFU67250793 = -310487628;    int sHyQBRqLFU43193849 = -365515238;    int sHyQBRqLFU85002880 = 89231322;    int sHyQBRqLFU79345089 = -186262011;    int sHyQBRqLFU58839965 = -839337780;    int sHyQBRqLFU69498732 = -395277497;    int sHyQBRqLFU91464747 = -825290895;    int sHyQBRqLFU67485943 = -49724635;    int sHyQBRqLFU92440385 = -178479655;    int sHyQBRqLFU16853418 = -250184214;    int sHyQBRqLFU11650036 = -191404653;    int sHyQBRqLFU4924557 = -53777596;    int sHyQBRqLFU54099107 = 21477750;    int sHyQBRqLFU56153683 = -251787748;    int sHyQBRqLFU68078034 = 24319487;    int sHyQBRqLFU47149206 = -938137262;    int sHyQBRqLFU65646252 = -945156415;    int sHyQBRqLFU92097285 = -865360668;    int sHyQBRqLFU66201748 = -268184821;    int sHyQBRqLFU88272173 = -924750052;    int sHyQBRqLFU16216879 = -597406811;    int sHyQBRqLFU36949269 = -889378978;    int sHyQBRqLFU60506838 = -479449482;    int sHyQBRqLFU26706231 = -185479748;    int sHyQBRqLFU84484563 = -887496186;    int sHyQBRqLFU59833518 = -803588836;    int sHyQBRqLFU6422802 = -364564319;    int sHyQBRqLFU29132235 = -349109945;    int sHyQBRqLFU11602859 = -456607403;    int sHyQBRqLFU72581404 = -177051817;    int sHyQBRqLFU76013245 = -767751980;    int sHyQBRqLFU93850948 = 61449697;    int sHyQBRqLFU9964168 = -189663913;    int sHyQBRqLFU57832515 = -758407789;    int sHyQBRqLFU6203957 = -294258841;    int sHyQBRqLFU70268712 = -469733336;    int sHyQBRqLFU93795147 = -357258824;    int sHyQBRqLFU1369950 = -82923938;    int sHyQBRqLFU65965954 = -680587632;    int sHyQBRqLFU39939176 = 81911104;    int sHyQBRqLFU37895608 = -800717157;    int sHyQBRqLFU75493253 = 42921405;    int sHyQBRqLFU80156411 = -887577081;    int sHyQBRqLFU63840047 = -111433794;    int sHyQBRqLFU61271195 = -973668604;    int sHyQBRqLFU57643333 = -982823681;    int sHyQBRqLFU53947025 = -530994276;    int sHyQBRqLFU48970756 = -576608597;    int sHyQBRqLFU71785502 = -187764902;    int sHyQBRqLFU67959714 = -216367041;    int sHyQBRqLFU19251077 = -223649542;    int sHyQBRqLFU42013977 = -918927470;    int sHyQBRqLFU12651242 = 31456554;    int sHyQBRqLFU25060234 = -822251330;    int sHyQBRqLFU8888698 = -924400006;    int sHyQBRqLFU30301525 = -421108651;    int sHyQBRqLFU82687010 = -886065756;    int sHyQBRqLFU58296650 = -725288930;    int sHyQBRqLFU94860525 = -298765826;    int sHyQBRqLFU99006446 = 64251055;    int sHyQBRqLFU63075931 = 69286822;    int sHyQBRqLFU62332512 = -376180951;    int sHyQBRqLFU55883084 = -593117232;    int sHyQBRqLFU19858981 = 98572162;    int sHyQBRqLFU40840173 = -482432234;    int sHyQBRqLFU17799088 = -152854350;    int sHyQBRqLFU94960389 = -864113683;    int sHyQBRqLFU96266592 = -220114462;    int sHyQBRqLFU49949727 = -957528908;    int sHyQBRqLFU97809321 = -505947178;    int sHyQBRqLFU53354058 = -480878439;    int sHyQBRqLFU64276303 = -762232478;    int sHyQBRqLFU26131331 = -84773037;    int sHyQBRqLFU26262573 = -250095925;    int sHyQBRqLFU50376566 = -24032896;    int sHyQBRqLFU40723626 = -540328217;    int sHyQBRqLFU56792857 = 98198102;    int sHyQBRqLFU96666791 = -280214605;     sHyQBRqLFU82938091 = sHyQBRqLFU73454750;     sHyQBRqLFU73454750 = sHyQBRqLFU13462561;     sHyQBRqLFU13462561 = sHyQBRqLFU78798028;     sHyQBRqLFU78798028 = sHyQBRqLFU80715038;     sHyQBRqLFU80715038 = sHyQBRqLFU24805920;     sHyQBRqLFU24805920 = sHyQBRqLFU9437909;     sHyQBRqLFU9437909 = sHyQBRqLFU29360355;     sHyQBRqLFU29360355 = sHyQBRqLFU42979196;     sHyQBRqLFU42979196 = sHyQBRqLFU72596796;     sHyQBRqLFU72596796 = sHyQBRqLFU80693464;     sHyQBRqLFU80693464 = sHyQBRqLFU72921230;     sHyQBRqLFU72921230 = sHyQBRqLFU62567890;     sHyQBRqLFU62567890 = sHyQBRqLFU8046132;     sHyQBRqLFU8046132 = sHyQBRqLFU5124440;     sHyQBRqLFU5124440 = sHyQBRqLFU39863537;     sHyQBRqLFU39863537 = sHyQBRqLFU15108920;     sHyQBRqLFU15108920 = sHyQBRqLFU84897329;     sHyQBRqLFU84897329 = sHyQBRqLFU34111263;     sHyQBRqLFU34111263 = sHyQBRqLFU78852990;     sHyQBRqLFU78852990 = sHyQBRqLFU13332408;     sHyQBRqLFU13332408 = sHyQBRqLFU25105576;     sHyQBRqLFU25105576 = sHyQBRqLFU67250793;     sHyQBRqLFU67250793 = sHyQBRqLFU43193849;     sHyQBRqLFU43193849 = sHyQBRqLFU85002880;     sHyQBRqLFU85002880 = sHyQBRqLFU79345089;     sHyQBRqLFU79345089 = sHyQBRqLFU58839965;     sHyQBRqLFU58839965 = sHyQBRqLFU69498732;     sHyQBRqLFU69498732 = sHyQBRqLFU91464747;     sHyQBRqLFU91464747 = sHyQBRqLFU67485943;     sHyQBRqLFU67485943 = sHyQBRqLFU92440385;     sHyQBRqLFU92440385 = sHyQBRqLFU16853418;     sHyQBRqLFU16853418 = sHyQBRqLFU11650036;     sHyQBRqLFU11650036 = sHyQBRqLFU4924557;     sHyQBRqLFU4924557 = sHyQBRqLFU54099107;     sHyQBRqLFU54099107 = sHyQBRqLFU56153683;     sHyQBRqLFU56153683 = sHyQBRqLFU68078034;     sHyQBRqLFU68078034 = sHyQBRqLFU47149206;     sHyQBRqLFU47149206 = sHyQBRqLFU65646252;     sHyQBRqLFU65646252 = sHyQBRqLFU92097285;     sHyQBRqLFU92097285 = sHyQBRqLFU66201748;     sHyQBRqLFU66201748 = sHyQBRqLFU88272173;     sHyQBRqLFU88272173 = sHyQBRqLFU16216879;     sHyQBRqLFU16216879 = sHyQBRqLFU36949269;     sHyQBRqLFU36949269 = sHyQBRqLFU60506838;     sHyQBRqLFU60506838 = sHyQBRqLFU26706231;     sHyQBRqLFU26706231 = sHyQBRqLFU84484563;     sHyQBRqLFU84484563 = sHyQBRqLFU59833518;     sHyQBRqLFU59833518 = sHyQBRqLFU6422802;     sHyQBRqLFU6422802 = sHyQBRqLFU29132235;     sHyQBRqLFU29132235 = sHyQBRqLFU11602859;     sHyQBRqLFU11602859 = sHyQBRqLFU72581404;     sHyQBRqLFU72581404 = sHyQBRqLFU76013245;     sHyQBRqLFU76013245 = sHyQBRqLFU93850948;     sHyQBRqLFU93850948 = sHyQBRqLFU9964168;     sHyQBRqLFU9964168 = sHyQBRqLFU57832515;     sHyQBRqLFU57832515 = sHyQBRqLFU6203957;     sHyQBRqLFU6203957 = sHyQBRqLFU70268712;     sHyQBRqLFU70268712 = sHyQBRqLFU93795147;     sHyQBRqLFU93795147 = sHyQBRqLFU1369950;     sHyQBRqLFU1369950 = sHyQBRqLFU65965954;     sHyQBRqLFU65965954 = sHyQBRqLFU39939176;     sHyQBRqLFU39939176 = sHyQBRqLFU37895608;     sHyQBRqLFU37895608 = sHyQBRqLFU75493253;     sHyQBRqLFU75493253 = sHyQBRqLFU80156411;     sHyQBRqLFU80156411 = sHyQBRqLFU63840047;     sHyQBRqLFU63840047 = sHyQBRqLFU61271195;     sHyQBRqLFU61271195 = sHyQBRqLFU57643333;     sHyQBRqLFU57643333 = sHyQBRqLFU53947025;     sHyQBRqLFU53947025 = sHyQBRqLFU48970756;     sHyQBRqLFU48970756 = sHyQBRqLFU71785502;     sHyQBRqLFU71785502 = sHyQBRqLFU67959714;     sHyQBRqLFU67959714 = sHyQBRqLFU19251077;     sHyQBRqLFU19251077 = sHyQBRqLFU42013977;     sHyQBRqLFU42013977 = sHyQBRqLFU12651242;     sHyQBRqLFU12651242 = sHyQBRqLFU25060234;     sHyQBRqLFU25060234 = sHyQBRqLFU8888698;     sHyQBRqLFU8888698 = sHyQBRqLFU30301525;     sHyQBRqLFU30301525 = sHyQBRqLFU82687010;     sHyQBRqLFU82687010 = sHyQBRqLFU58296650;     sHyQBRqLFU58296650 = sHyQBRqLFU94860525;     sHyQBRqLFU94860525 = sHyQBRqLFU99006446;     sHyQBRqLFU99006446 = sHyQBRqLFU63075931;     sHyQBRqLFU63075931 = sHyQBRqLFU62332512;     sHyQBRqLFU62332512 = sHyQBRqLFU55883084;     sHyQBRqLFU55883084 = sHyQBRqLFU19858981;     sHyQBRqLFU19858981 = sHyQBRqLFU40840173;     sHyQBRqLFU40840173 = sHyQBRqLFU17799088;     sHyQBRqLFU17799088 = sHyQBRqLFU94960389;     sHyQBRqLFU94960389 = sHyQBRqLFU96266592;     sHyQBRqLFU96266592 = sHyQBRqLFU49949727;     sHyQBRqLFU49949727 = sHyQBRqLFU97809321;     sHyQBRqLFU97809321 = sHyQBRqLFU53354058;     sHyQBRqLFU53354058 = sHyQBRqLFU64276303;     sHyQBRqLFU64276303 = sHyQBRqLFU26131331;     sHyQBRqLFU26131331 = sHyQBRqLFU26262573;     sHyQBRqLFU26262573 = sHyQBRqLFU50376566;     sHyQBRqLFU50376566 = sHyQBRqLFU40723626;     sHyQBRqLFU40723626 = sHyQBRqLFU56792857;     sHyQBRqLFU56792857 = sHyQBRqLFU96666791;     sHyQBRqLFU96666791 = sHyQBRqLFU82938091;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XVdUGYujdE93422250() {     int cVlqxckVPa50385127 = -980258739;    int cVlqxckVPa68650941 = -312050171;    int cVlqxckVPa90634795 = -671863954;    int cVlqxckVPa73093934 = -377791359;    int cVlqxckVPa39837711 = -65082331;    int cVlqxckVPa47484955 = -726566749;    int cVlqxckVPa83199034 = -456394668;    int cVlqxckVPa16040520 = -766917224;    int cVlqxckVPa43418253 = -401768622;    int cVlqxckVPa64807973 = -512138766;    int cVlqxckVPa66471536 = -142188523;    int cVlqxckVPa53344772 = -23351799;    int cVlqxckVPa64675593 = -822265764;    int cVlqxckVPa96792421 = -208882250;    int cVlqxckVPa13482957 = -779790955;    int cVlqxckVPa83491593 = -111700718;    int cVlqxckVPa14839742 = -472264028;    int cVlqxckVPa13051869 = -490509104;    int cVlqxckVPa16064224 = -361586615;    int cVlqxckVPa27068279 = -669314101;    int cVlqxckVPa48011428 = -995934423;    int cVlqxckVPa27109719 = -348117389;    int cVlqxckVPa5602525 = -369277102;    int cVlqxckVPa45479239 = -543980544;    int cVlqxckVPa58207066 = -67430201;    int cVlqxckVPa85330604 = 27803933;    int cVlqxckVPa2023844 = -627088626;    int cVlqxckVPa54697133 = 31839527;    int cVlqxckVPa35301030 = -773682696;    int cVlqxckVPa38582597 = -294937384;    int cVlqxckVPa55169231 = -616675696;    int cVlqxckVPa66439994 = -248578659;    int cVlqxckVPa39899438 = -421832836;    int cVlqxckVPa76373985 = -861970891;    int cVlqxckVPa89305757 = -547103465;    int cVlqxckVPa95876150 = -208580702;    int cVlqxckVPa67605214 = -616363106;    int cVlqxckVPa47912788 = -736423695;    int cVlqxckVPa86877690 = -456274178;    int cVlqxckVPa70475517 = -758654427;    int cVlqxckVPa95736789 = -668265074;    int cVlqxckVPa46951627 = -136801391;    int cVlqxckVPa81817171 = 97784680;    int cVlqxckVPa31340307 = -47266443;    int cVlqxckVPa16481340 = -31477638;    int cVlqxckVPa15239994 = -515543132;    int cVlqxckVPa50842502 = -930788000;    int cVlqxckVPa37317213 = -327832631;    int cVlqxckVPa74171228 = -864132621;    int cVlqxckVPa878895 = -513140816;    int cVlqxckVPa91784522 = -788152206;    int cVlqxckVPa75592844 = -781157390;    int cVlqxckVPa66778050 = -791616712;    int cVlqxckVPa85716160 = 8031321;    int cVlqxckVPa45410822 = -823117803;    int cVlqxckVPa23275409 = -532141351;    int cVlqxckVPa63048417 = -942773069;    int cVlqxckVPa45155557 = -27883411;    int cVlqxckVPa14886868 = -210361158;    int cVlqxckVPa54507107 = 7113735;    int cVlqxckVPa45461111 = 521877;    int cVlqxckVPa28501902 = -388234195;    int cVlqxckVPa80739490 = -993234528;    int cVlqxckVPa4835656 = -6831239;    int cVlqxckVPa9638742 = -895463071;    int cVlqxckVPa31543 = -893609864;    int cVlqxckVPa13445334 = -601518963;    int cVlqxckVPa88301608 = -960294873;    int cVlqxckVPa7486664 = -661778785;    int cVlqxckVPa17606807 = -471210253;    int cVlqxckVPa15886380 = -495337612;    int cVlqxckVPa66926954 = -735840333;    int cVlqxckVPa26174178 = 65765073;    int cVlqxckVPa45588707 = -602932188;    int cVlqxckVPa31331490 = 98950973;    int cVlqxckVPa1059802 = -759133032;    int cVlqxckVPa45292547 = -345902070;    int cVlqxckVPa74262217 = -222010659;    int cVlqxckVPa28997899 = -412502906;    int cVlqxckVPa42967072 = -551887070;    int cVlqxckVPa34488103 = -41408067;    int cVlqxckVPa64706631 = -199255995;    int cVlqxckVPa80525904 = -104027853;    int cVlqxckVPa34422136 = -160541880;    int cVlqxckVPa46798075 = -506785178;    int cVlqxckVPa79576387 = -835518307;    int cVlqxckVPa99661944 = -456961948;    int cVlqxckVPa54183278 = -329864158;    int cVlqxckVPa30963163 = 61146912;    int cVlqxckVPa66030349 = 85037885;    int cVlqxckVPa32827734 = -265807633;    int cVlqxckVPa22449658 = -488479696;    int cVlqxckVPa33025920 = -426062537;    int cVlqxckVPa32370584 = -363387913;    int cVlqxckVPa25014406 = -659176304;    int cVlqxckVPa67234888 = -180030879;    int cVlqxckVPa66212137 = -143566863;    int cVlqxckVPa76981516 = -895384082;    int cVlqxckVPa21701566 = -151803373;    int cVlqxckVPa16449798 = -980258739;     cVlqxckVPa50385127 = cVlqxckVPa68650941;     cVlqxckVPa68650941 = cVlqxckVPa90634795;     cVlqxckVPa90634795 = cVlqxckVPa73093934;     cVlqxckVPa73093934 = cVlqxckVPa39837711;     cVlqxckVPa39837711 = cVlqxckVPa47484955;     cVlqxckVPa47484955 = cVlqxckVPa83199034;     cVlqxckVPa83199034 = cVlqxckVPa16040520;     cVlqxckVPa16040520 = cVlqxckVPa43418253;     cVlqxckVPa43418253 = cVlqxckVPa64807973;     cVlqxckVPa64807973 = cVlqxckVPa66471536;     cVlqxckVPa66471536 = cVlqxckVPa53344772;     cVlqxckVPa53344772 = cVlqxckVPa64675593;     cVlqxckVPa64675593 = cVlqxckVPa96792421;     cVlqxckVPa96792421 = cVlqxckVPa13482957;     cVlqxckVPa13482957 = cVlqxckVPa83491593;     cVlqxckVPa83491593 = cVlqxckVPa14839742;     cVlqxckVPa14839742 = cVlqxckVPa13051869;     cVlqxckVPa13051869 = cVlqxckVPa16064224;     cVlqxckVPa16064224 = cVlqxckVPa27068279;     cVlqxckVPa27068279 = cVlqxckVPa48011428;     cVlqxckVPa48011428 = cVlqxckVPa27109719;     cVlqxckVPa27109719 = cVlqxckVPa5602525;     cVlqxckVPa5602525 = cVlqxckVPa45479239;     cVlqxckVPa45479239 = cVlqxckVPa58207066;     cVlqxckVPa58207066 = cVlqxckVPa85330604;     cVlqxckVPa85330604 = cVlqxckVPa2023844;     cVlqxckVPa2023844 = cVlqxckVPa54697133;     cVlqxckVPa54697133 = cVlqxckVPa35301030;     cVlqxckVPa35301030 = cVlqxckVPa38582597;     cVlqxckVPa38582597 = cVlqxckVPa55169231;     cVlqxckVPa55169231 = cVlqxckVPa66439994;     cVlqxckVPa66439994 = cVlqxckVPa39899438;     cVlqxckVPa39899438 = cVlqxckVPa76373985;     cVlqxckVPa76373985 = cVlqxckVPa89305757;     cVlqxckVPa89305757 = cVlqxckVPa95876150;     cVlqxckVPa95876150 = cVlqxckVPa67605214;     cVlqxckVPa67605214 = cVlqxckVPa47912788;     cVlqxckVPa47912788 = cVlqxckVPa86877690;     cVlqxckVPa86877690 = cVlqxckVPa70475517;     cVlqxckVPa70475517 = cVlqxckVPa95736789;     cVlqxckVPa95736789 = cVlqxckVPa46951627;     cVlqxckVPa46951627 = cVlqxckVPa81817171;     cVlqxckVPa81817171 = cVlqxckVPa31340307;     cVlqxckVPa31340307 = cVlqxckVPa16481340;     cVlqxckVPa16481340 = cVlqxckVPa15239994;     cVlqxckVPa15239994 = cVlqxckVPa50842502;     cVlqxckVPa50842502 = cVlqxckVPa37317213;     cVlqxckVPa37317213 = cVlqxckVPa74171228;     cVlqxckVPa74171228 = cVlqxckVPa878895;     cVlqxckVPa878895 = cVlqxckVPa91784522;     cVlqxckVPa91784522 = cVlqxckVPa75592844;     cVlqxckVPa75592844 = cVlqxckVPa66778050;     cVlqxckVPa66778050 = cVlqxckVPa85716160;     cVlqxckVPa85716160 = cVlqxckVPa45410822;     cVlqxckVPa45410822 = cVlqxckVPa23275409;     cVlqxckVPa23275409 = cVlqxckVPa63048417;     cVlqxckVPa63048417 = cVlqxckVPa45155557;     cVlqxckVPa45155557 = cVlqxckVPa14886868;     cVlqxckVPa14886868 = cVlqxckVPa54507107;     cVlqxckVPa54507107 = cVlqxckVPa45461111;     cVlqxckVPa45461111 = cVlqxckVPa28501902;     cVlqxckVPa28501902 = cVlqxckVPa80739490;     cVlqxckVPa80739490 = cVlqxckVPa4835656;     cVlqxckVPa4835656 = cVlqxckVPa9638742;     cVlqxckVPa9638742 = cVlqxckVPa31543;     cVlqxckVPa31543 = cVlqxckVPa13445334;     cVlqxckVPa13445334 = cVlqxckVPa88301608;     cVlqxckVPa88301608 = cVlqxckVPa7486664;     cVlqxckVPa7486664 = cVlqxckVPa17606807;     cVlqxckVPa17606807 = cVlqxckVPa15886380;     cVlqxckVPa15886380 = cVlqxckVPa66926954;     cVlqxckVPa66926954 = cVlqxckVPa26174178;     cVlqxckVPa26174178 = cVlqxckVPa45588707;     cVlqxckVPa45588707 = cVlqxckVPa31331490;     cVlqxckVPa31331490 = cVlqxckVPa1059802;     cVlqxckVPa1059802 = cVlqxckVPa45292547;     cVlqxckVPa45292547 = cVlqxckVPa74262217;     cVlqxckVPa74262217 = cVlqxckVPa28997899;     cVlqxckVPa28997899 = cVlqxckVPa42967072;     cVlqxckVPa42967072 = cVlqxckVPa34488103;     cVlqxckVPa34488103 = cVlqxckVPa64706631;     cVlqxckVPa64706631 = cVlqxckVPa80525904;     cVlqxckVPa80525904 = cVlqxckVPa34422136;     cVlqxckVPa34422136 = cVlqxckVPa46798075;     cVlqxckVPa46798075 = cVlqxckVPa79576387;     cVlqxckVPa79576387 = cVlqxckVPa99661944;     cVlqxckVPa99661944 = cVlqxckVPa54183278;     cVlqxckVPa54183278 = cVlqxckVPa30963163;     cVlqxckVPa30963163 = cVlqxckVPa66030349;     cVlqxckVPa66030349 = cVlqxckVPa32827734;     cVlqxckVPa32827734 = cVlqxckVPa22449658;     cVlqxckVPa22449658 = cVlqxckVPa33025920;     cVlqxckVPa33025920 = cVlqxckVPa32370584;     cVlqxckVPa32370584 = cVlqxckVPa25014406;     cVlqxckVPa25014406 = cVlqxckVPa67234888;     cVlqxckVPa67234888 = cVlqxckVPa66212137;     cVlqxckVPa66212137 = cVlqxckVPa76981516;     cVlqxckVPa76981516 = cVlqxckVPa21701566;     cVlqxckVPa21701566 = cVlqxckVPa16449798;     cVlqxckVPa16449798 = cVlqxckVPa50385127;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UsRTfZAzxX63878123() {     int euuCToRPkI63171294 = -15299431;    int euuCToRPkI55421829 = -482384184;    int euuCToRPkI8387595 = 19776664;    int euuCToRPkI81034759 = -201993636;    int euuCToRPkI33348816 = -556898796;    int euuCToRPkI90795025 = -503490062;    int euuCToRPkI4406793 = 97933261;    int euuCToRPkI42759633 = -111435481;    int euuCToRPkI38823063 = -363526714;    int euuCToRPkI71026677 = -316226399;    int euuCToRPkI93958918 = -256874538;    int euuCToRPkI85295277 = -530684615;    int euuCToRPkI34818897 = -726048421;    int euuCToRPkI46416500 = -541297448;    int euuCToRPkI36789510 = -686776785;    int euuCToRPkI11916663 = -662592107;    int euuCToRPkI70591559 = -136038495;    int euuCToRPkI96610353 = -45719406;    int euuCToRPkI17224855 = -271055810;    int euuCToRPkI29722776 = -168558240;    int euuCToRPkI28185485 = -387250687;    int euuCToRPkI56157538 = -517975737;    int euuCToRPkI44362820 = -300280996;    int euuCToRPkI89986368 = -904325556;    int euuCToRPkI64101325 = -277872125;    int euuCToRPkI66649250 = -762627266;    int euuCToRPkI13439381 = -160794906;    int euuCToRPkI22050058 = -141958578;    int euuCToRPkI51918083 = -987899936;    int euuCToRPkI91333712 = 75176462;    int euuCToRPkI99605227 = -893092446;    int euuCToRPkI69758818 = -227298337;    int euuCToRPkI85945387 = -297687620;    int euuCToRPkI53450358 = -766725109;    int euuCToRPkI72366290 = -57135346;    int euuCToRPkI91100265 = 2856193;    int euuCToRPkI4369275 = -406272457;    int euuCToRPkI5616811 = -656043787;    int euuCToRPkI92653076 = -298124755;    int euuCToRPkI96740247 = -603071273;    int euuCToRPkI57168098 = 56260933;    int euuCToRPkI54654083 = -35112725;    int euuCToRPkI16700642 = 74351235;    int euuCToRPkI75968845 = -388438686;    int euuCToRPkI96489830 = 75752401;    int euuCToRPkI88868123 = -588861571;    int euuCToRPkI28424522 = -328303052;    int euuCToRPkI66757179 = -483185411;    int euuCToRPkI18235278 = 72265404;    int euuCToRPkI47829315 = -24177279;    int euuCToRPkI10112016 = 58963486;    int euuCToRPkI3769392 = -152342728;    int euuCToRPkI1263200 = -967619994;    int euuCToRPkI30415886 = -681420421;    int euuCToRPkI35692639 = -817562290;    int euuCToRPkI7013757 = -497323694;    int euuCToRPkI11059010 = -82103188;    int euuCToRPkI18401227 = -75897780;    int euuCToRPkI16933435 = -924121511;    int euuCToRPkI66699566 = -794271531;    int euuCToRPkI77355645 = -242695156;    int euuCToRPkI82356735 = -760108161;    int euuCToRPkI90841549 = -123535545;    int euuCToRPkI47489351 = -338703177;    int euuCToRPkI71421450 = -423133953;    int euuCToRPkI24200101 = 70423799;    int euuCToRPkI99349889 = -132996995;    int euuCToRPkI81368538 = -959323312;    int euuCToRPkI74050210 = -384162103;    int euuCToRPkI45689245 = -589632979;    int euuCToRPkI7547388 = -156319651;    int euuCToRPkI64974749 = -479994709;    int euuCToRPkI3957277 = -747594652;    int euuCToRPkI20484608 = -667984538;    int euuCToRPkI72554677 = -124819173;    int euuCToRPkI73531402 = -252137962;    int euuCToRPkI39456896 = -492326973;    int euuCToRPkI68393974 = -911842311;    int euuCToRPkI93496537 = -880077958;    int euuCToRPkI75233202 = -689010554;    int euuCToRPkI38224728 = -334324214;    int euuCToRPkI46682201 = -677609496;    int euuCToRPkI3814780 = -114223983;    int euuCToRPkI4088769 = -863722658;    int euuCToRPkI81221697 = -983787024;    int euuCToRPkI95835835 = -640749719;    int euuCToRPkI68495618 = -259678344;    int euuCToRPkI55529502 = -616267200;    int euuCToRPkI17757720 = -949162820;    int euuCToRPkI65352533 = -559811652;    int euuCToRPkI80041256 = -915040619;    int euuCToRPkI85968048 = -230374677;    int euuCToRPkI88683376 = -731922277;    int euuCToRPkI25953511 = -503853224;    int euuCToRPkI19384602 = -260376117;    int euuCToRPkI74811363 = -183630907;    int euuCToRPkI63812533 = -911577181;    int euuCToRPkI69211290 = -586945589;    int euuCToRPkI4547395 = -965304733;    int euuCToRPkI72289729 = -15299431;     euuCToRPkI63171294 = euuCToRPkI55421829;     euuCToRPkI55421829 = euuCToRPkI8387595;     euuCToRPkI8387595 = euuCToRPkI81034759;     euuCToRPkI81034759 = euuCToRPkI33348816;     euuCToRPkI33348816 = euuCToRPkI90795025;     euuCToRPkI90795025 = euuCToRPkI4406793;     euuCToRPkI4406793 = euuCToRPkI42759633;     euuCToRPkI42759633 = euuCToRPkI38823063;     euuCToRPkI38823063 = euuCToRPkI71026677;     euuCToRPkI71026677 = euuCToRPkI93958918;     euuCToRPkI93958918 = euuCToRPkI85295277;     euuCToRPkI85295277 = euuCToRPkI34818897;     euuCToRPkI34818897 = euuCToRPkI46416500;     euuCToRPkI46416500 = euuCToRPkI36789510;     euuCToRPkI36789510 = euuCToRPkI11916663;     euuCToRPkI11916663 = euuCToRPkI70591559;     euuCToRPkI70591559 = euuCToRPkI96610353;     euuCToRPkI96610353 = euuCToRPkI17224855;     euuCToRPkI17224855 = euuCToRPkI29722776;     euuCToRPkI29722776 = euuCToRPkI28185485;     euuCToRPkI28185485 = euuCToRPkI56157538;     euuCToRPkI56157538 = euuCToRPkI44362820;     euuCToRPkI44362820 = euuCToRPkI89986368;     euuCToRPkI89986368 = euuCToRPkI64101325;     euuCToRPkI64101325 = euuCToRPkI66649250;     euuCToRPkI66649250 = euuCToRPkI13439381;     euuCToRPkI13439381 = euuCToRPkI22050058;     euuCToRPkI22050058 = euuCToRPkI51918083;     euuCToRPkI51918083 = euuCToRPkI91333712;     euuCToRPkI91333712 = euuCToRPkI99605227;     euuCToRPkI99605227 = euuCToRPkI69758818;     euuCToRPkI69758818 = euuCToRPkI85945387;     euuCToRPkI85945387 = euuCToRPkI53450358;     euuCToRPkI53450358 = euuCToRPkI72366290;     euuCToRPkI72366290 = euuCToRPkI91100265;     euuCToRPkI91100265 = euuCToRPkI4369275;     euuCToRPkI4369275 = euuCToRPkI5616811;     euuCToRPkI5616811 = euuCToRPkI92653076;     euuCToRPkI92653076 = euuCToRPkI96740247;     euuCToRPkI96740247 = euuCToRPkI57168098;     euuCToRPkI57168098 = euuCToRPkI54654083;     euuCToRPkI54654083 = euuCToRPkI16700642;     euuCToRPkI16700642 = euuCToRPkI75968845;     euuCToRPkI75968845 = euuCToRPkI96489830;     euuCToRPkI96489830 = euuCToRPkI88868123;     euuCToRPkI88868123 = euuCToRPkI28424522;     euuCToRPkI28424522 = euuCToRPkI66757179;     euuCToRPkI66757179 = euuCToRPkI18235278;     euuCToRPkI18235278 = euuCToRPkI47829315;     euuCToRPkI47829315 = euuCToRPkI10112016;     euuCToRPkI10112016 = euuCToRPkI3769392;     euuCToRPkI3769392 = euuCToRPkI1263200;     euuCToRPkI1263200 = euuCToRPkI30415886;     euuCToRPkI30415886 = euuCToRPkI35692639;     euuCToRPkI35692639 = euuCToRPkI7013757;     euuCToRPkI7013757 = euuCToRPkI11059010;     euuCToRPkI11059010 = euuCToRPkI18401227;     euuCToRPkI18401227 = euuCToRPkI16933435;     euuCToRPkI16933435 = euuCToRPkI66699566;     euuCToRPkI66699566 = euuCToRPkI77355645;     euuCToRPkI77355645 = euuCToRPkI82356735;     euuCToRPkI82356735 = euuCToRPkI90841549;     euuCToRPkI90841549 = euuCToRPkI47489351;     euuCToRPkI47489351 = euuCToRPkI71421450;     euuCToRPkI71421450 = euuCToRPkI24200101;     euuCToRPkI24200101 = euuCToRPkI99349889;     euuCToRPkI99349889 = euuCToRPkI81368538;     euuCToRPkI81368538 = euuCToRPkI74050210;     euuCToRPkI74050210 = euuCToRPkI45689245;     euuCToRPkI45689245 = euuCToRPkI7547388;     euuCToRPkI7547388 = euuCToRPkI64974749;     euuCToRPkI64974749 = euuCToRPkI3957277;     euuCToRPkI3957277 = euuCToRPkI20484608;     euuCToRPkI20484608 = euuCToRPkI72554677;     euuCToRPkI72554677 = euuCToRPkI73531402;     euuCToRPkI73531402 = euuCToRPkI39456896;     euuCToRPkI39456896 = euuCToRPkI68393974;     euuCToRPkI68393974 = euuCToRPkI93496537;     euuCToRPkI93496537 = euuCToRPkI75233202;     euuCToRPkI75233202 = euuCToRPkI38224728;     euuCToRPkI38224728 = euuCToRPkI46682201;     euuCToRPkI46682201 = euuCToRPkI3814780;     euuCToRPkI3814780 = euuCToRPkI4088769;     euuCToRPkI4088769 = euuCToRPkI81221697;     euuCToRPkI81221697 = euuCToRPkI95835835;     euuCToRPkI95835835 = euuCToRPkI68495618;     euuCToRPkI68495618 = euuCToRPkI55529502;     euuCToRPkI55529502 = euuCToRPkI17757720;     euuCToRPkI17757720 = euuCToRPkI65352533;     euuCToRPkI65352533 = euuCToRPkI80041256;     euuCToRPkI80041256 = euuCToRPkI85968048;     euuCToRPkI85968048 = euuCToRPkI88683376;     euuCToRPkI88683376 = euuCToRPkI25953511;     euuCToRPkI25953511 = euuCToRPkI19384602;     euuCToRPkI19384602 = euuCToRPkI74811363;     euuCToRPkI74811363 = euuCToRPkI63812533;     euuCToRPkI63812533 = euuCToRPkI69211290;     euuCToRPkI69211290 = euuCToRPkI4547395;     euuCToRPkI4547395 = euuCToRPkI72289729;     euuCToRPkI72289729 = euuCToRPkI63171294;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bMVesxUJIb82091465() {     int GFwiAgMVAk46615600 = -895965112;    int GFwiAgMVAk72293918 = -481766372;    int GFwiAgMVAk4796102 = -873748564;    int GFwiAgMVAk43827011 = -395793471;    int GFwiAgMVAk39470140 = -363766287;    int GFwiAgMVAk89299426 = -282561715;    int GFwiAgMVAk75446110 = -741978052;    int GFwiAgMVAk57107075 = -98593510;    int GFwiAgMVAk97352764 = -704986065;    int GFwiAgMVAk3572330 = -784602511;    int GFwiAgMVAk43374856 = -446990411;    int GFwiAgMVAk48500676 = -518302861;    int GFwiAgMVAk52936672 = -517709662;    int GFwiAgMVAk26043392 = -329587805;    int GFwiAgMVAk97643973 = -780799767;    int GFwiAgMVAk98305823 = -450671432;    int GFwiAgMVAk64343444 = -989383522;    int GFwiAgMVAk77461800 = -293024282;    int GFwiAgMVAk79664996 = -269730681;    int GFwiAgMVAk19090218 = -698144232;    int GFwiAgMVAk83825872 = 23220282;    int GFwiAgMVAk68304856 = -509127508;    int GFwiAgMVAk48188527 = -340170194;    int GFwiAgMVAk24692926 = -976150952;    int GFwiAgMVAk5677284 = -837524884;    int GFwiAgMVAk61991399 = -643136982;    int GFwiAgMVAk62894120 = -888879249;    int GFwiAgMVAk61781683 = -211913462;    int GFwiAgMVAk87999177 = -850405078;    int GFwiAgMVAk7167054 = -437673390;    int GFwiAgMVAk17700541 = -530849647;    int GFwiAgMVAk32865245 = -142493417;    int GFwiAgMVAk29484274 = 81440202;    int GFwiAgMVAk95608868 = -358327279;    int GFwiAgMVAk51093020 = -254800081;    int GFwiAgMVAk3489450 = -885167712;    int GFwiAgMVAk72983453 = -285121636;    int GFwiAgMVAk46195643 = -364514307;    int GFwiAgMVAk41648155 = -483022552;    int GFwiAgMVAk46235382 = -355664795;    int GFwiAgMVAk90817022 = -615288041;    int GFwiAgMVAk5141869 = -609021047;    int GFwiAgMVAk40267711 = -476706008;    int GFwiAgMVAk67873785 = -144621457;    int GFwiAgMVAk88434327 = -752404552;    int GFwiAgMVAk28474119 = -608472495;    int GFwiAgMVAk4439952 = -460607282;    int GFwiAgMVAk86427775 = -481907130;    int GFwiAgMVAk49082390 = -513336678;    int GFwiAgMVAk83161799 = -231197734;    int GFwiAgMVAk83369464 = -161937937;    int GFwiAgMVAk59270186 = -210602039;    int GFwiAgMVAk15375659 = -79367311;    int GFwiAgMVAk55092644 = -399833808;    int GFwiAgMVAk98265592 = -66048413;    int GFwiAgMVAk78310744 = -286837605;    int GFwiAgMVAk24105391 = -41596179;    int GFwiAgMVAk80103175 = -897597612;    int GFwiAgMVAk38149728 = -558268588;    int GFwiAgMVAk77478741 = -720629306;    int GFwiAgMVAk26405307 = -393682467;    int GFwiAgMVAk13664428 = -430064591;    int GFwiAgMVAk69107898 = -248188432;    int GFwiAgMVAk90185710 = -167312676;    int GFwiAgMVAk85871788 = -153752865;    int GFwiAgMVAk10509612 = -204496994;    int GFwiAgMVAk19016402 = -499743064;    int GFwiAgMVAk57327804 = -59382383;    int GFwiAgMVAk74950371 = 25212275;    int GFwiAgMVAk94154523 = -895632056;    int GFwiAgMVAk25322371 = -65549796;    int GFwiAgMVAk18147802 = -524869215;    int GFwiAgMVAk35813645 = -810001731;    int GFwiAgMVAk33429614 = -914065886;    int GFwiAgMVAk28273196 = 17143808;    int GFwiAgMVAk78684003 = -367758672;    int GFwiAgMVAk28037145 = 67578500;    int GFwiAgMVAk80314742 = -95548738;    int GFwiAgMVAk36258599 = -123746401;    int GFwiAgMVAk77203164 = -129052390;    int GFwiAgMVAk57551447 = -82529700;    int GFwiAgMVAk76466345 = -306972120;    int GFwiAgMVAk12699293 = -698576784;    int GFwiAgMVAk4837379 = -519207345;    int GFwiAgMVAk23797590 = -175735454;    int GFwiAgMVAk58430355 = -220247608;    int GFwiAgMVAk17489586 = 36873893;    int GFwiAgMVAk74391630 = -518725990;    int GFwiAgMVAk97343275 = -192278867;    int GFwiAgMVAk72782276 = -967962477;    int GFwiAgMVAk79384058 = -743571534;    int GFwiAgMVAk92880278 = -387524025;    int GFwiAgMVAk8045916 = -806245720;    int GFwiAgMVAk64169414 = -762393247;    int GFwiAgMVAk19830076 = -961982329;    int GFwiAgMVAk77152595 = -85223450;    int GFwiAgMVAk36033971 = -260832615;    int GFwiAgMVAk50082001 = -209393332;    int GFwiAgMVAk82001996 = -990868593;    int GFwiAgMVAk77924715 = -895965112;     GFwiAgMVAk46615600 = GFwiAgMVAk72293918;     GFwiAgMVAk72293918 = GFwiAgMVAk4796102;     GFwiAgMVAk4796102 = GFwiAgMVAk43827011;     GFwiAgMVAk43827011 = GFwiAgMVAk39470140;     GFwiAgMVAk39470140 = GFwiAgMVAk89299426;     GFwiAgMVAk89299426 = GFwiAgMVAk75446110;     GFwiAgMVAk75446110 = GFwiAgMVAk57107075;     GFwiAgMVAk57107075 = GFwiAgMVAk97352764;     GFwiAgMVAk97352764 = GFwiAgMVAk3572330;     GFwiAgMVAk3572330 = GFwiAgMVAk43374856;     GFwiAgMVAk43374856 = GFwiAgMVAk48500676;     GFwiAgMVAk48500676 = GFwiAgMVAk52936672;     GFwiAgMVAk52936672 = GFwiAgMVAk26043392;     GFwiAgMVAk26043392 = GFwiAgMVAk97643973;     GFwiAgMVAk97643973 = GFwiAgMVAk98305823;     GFwiAgMVAk98305823 = GFwiAgMVAk64343444;     GFwiAgMVAk64343444 = GFwiAgMVAk77461800;     GFwiAgMVAk77461800 = GFwiAgMVAk79664996;     GFwiAgMVAk79664996 = GFwiAgMVAk19090218;     GFwiAgMVAk19090218 = GFwiAgMVAk83825872;     GFwiAgMVAk83825872 = GFwiAgMVAk68304856;     GFwiAgMVAk68304856 = GFwiAgMVAk48188527;     GFwiAgMVAk48188527 = GFwiAgMVAk24692926;     GFwiAgMVAk24692926 = GFwiAgMVAk5677284;     GFwiAgMVAk5677284 = GFwiAgMVAk61991399;     GFwiAgMVAk61991399 = GFwiAgMVAk62894120;     GFwiAgMVAk62894120 = GFwiAgMVAk61781683;     GFwiAgMVAk61781683 = GFwiAgMVAk87999177;     GFwiAgMVAk87999177 = GFwiAgMVAk7167054;     GFwiAgMVAk7167054 = GFwiAgMVAk17700541;     GFwiAgMVAk17700541 = GFwiAgMVAk32865245;     GFwiAgMVAk32865245 = GFwiAgMVAk29484274;     GFwiAgMVAk29484274 = GFwiAgMVAk95608868;     GFwiAgMVAk95608868 = GFwiAgMVAk51093020;     GFwiAgMVAk51093020 = GFwiAgMVAk3489450;     GFwiAgMVAk3489450 = GFwiAgMVAk72983453;     GFwiAgMVAk72983453 = GFwiAgMVAk46195643;     GFwiAgMVAk46195643 = GFwiAgMVAk41648155;     GFwiAgMVAk41648155 = GFwiAgMVAk46235382;     GFwiAgMVAk46235382 = GFwiAgMVAk90817022;     GFwiAgMVAk90817022 = GFwiAgMVAk5141869;     GFwiAgMVAk5141869 = GFwiAgMVAk40267711;     GFwiAgMVAk40267711 = GFwiAgMVAk67873785;     GFwiAgMVAk67873785 = GFwiAgMVAk88434327;     GFwiAgMVAk88434327 = GFwiAgMVAk28474119;     GFwiAgMVAk28474119 = GFwiAgMVAk4439952;     GFwiAgMVAk4439952 = GFwiAgMVAk86427775;     GFwiAgMVAk86427775 = GFwiAgMVAk49082390;     GFwiAgMVAk49082390 = GFwiAgMVAk83161799;     GFwiAgMVAk83161799 = GFwiAgMVAk83369464;     GFwiAgMVAk83369464 = GFwiAgMVAk59270186;     GFwiAgMVAk59270186 = GFwiAgMVAk15375659;     GFwiAgMVAk15375659 = GFwiAgMVAk55092644;     GFwiAgMVAk55092644 = GFwiAgMVAk98265592;     GFwiAgMVAk98265592 = GFwiAgMVAk78310744;     GFwiAgMVAk78310744 = GFwiAgMVAk24105391;     GFwiAgMVAk24105391 = GFwiAgMVAk80103175;     GFwiAgMVAk80103175 = GFwiAgMVAk38149728;     GFwiAgMVAk38149728 = GFwiAgMVAk77478741;     GFwiAgMVAk77478741 = GFwiAgMVAk26405307;     GFwiAgMVAk26405307 = GFwiAgMVAk13664428;     GFwiAgMVAk13664428 = GFwiAgMVAk69107898;     GFwiAgMVAk69107898 = GFwiAgMVAk90185710;     GFwiAgMVAk90185710 = GFwiAgMVAk85871788;     GFwiAgMVAk85871788 = GFwiAgMVAk10509612;     GFwiAgMVAk10509612 = GFwiAgMVAk19016402;     GFwiAgMVAk19016402 = GFwiAgMVAk57327804;     GFwiAgMVAk57327804 = GFwiAgMVAk74950371;     GFwiAgMVAk74950371 = GFwiAgMVAk94154523;     GFwiAgMVAk94154523 = GFwiAgMVAk25322371;     GFwiAgMVAk25322371 = GFwiAgMVAk18147802;     GFwiAgMVAk18147802 = GFwiAgMVAk35813645;     GFwiAgMVAk35813645 = GFwiAgMVAk33429614;     GFwiAgMVAk33429614 = GFwiAgMVAk28273196;     GFwiAgMVAk28273196 = GFwiAgMVAk78684003;     GFwiAgMVAk78684003 = GFwiAgMVAk28037145;     GFwiAgMVAk28037145 = GFwiAgMVAk80314742;     GFwiAgMVAk80314742 = GFwiAgMVAk36258599;     GFwiAgMVAk36258599 = GFwiAgMVAk77203164;     GFwiAgMVAk77203164 = GFwiAgMVAk57551447;     GFwiAgMVAk57551447 = GFwiAgMVAk76466345;     GFwiAgMVAk76466345 = GFwiAgMVAk12699293;     GFwiAgMVAk12699293 = GFwiAgMVAk4837379;     GFwiAgMVAk4837379 = GFwiAgMVAk23797590;     GFwiAgMVAk23797590 = GFwiAgMVAk58430355;     GFwiAgMVAk58430355 = GFwiAgMVAk17489586;     GFwiAgMVAk17489586 = GFwiAgMVAk74391630;     GFwiAgMVAk74391630 = GFwiAgMVAk97343275;     GFwiAgMVAk97343275 = GFwiAgMVAk72782276;     GFwiAgMVAk72782276 = GFwiAgMVAk79384058;     GFwiAgMVAk79384058 = GFwiAgMVAk92880278;     GFwiAgMVAk92880278 = GFwiAgMVAk8045916;     GFwiAgMVAk8045916 = GFwiAgMVAk64169414;     GFwiAgMVAk64169414 = GFwiAgMVAk19830076;     GFwiAgMVAk19830076 = GFwiAgMVAk77152595;     GFwiAgMVAk77152595 = GFwiAgMVAk36033971;     GFwiAgMVAk36033971 = GFwiAgMVAk50082001;     GFwiAgMVAk50082001 = GFwiAgMVAk82001996;     GFwiAgMVAk82001996 = GFwiAgMVAk77924715;     GFwiAgMVAk77924715 = GFwiAgMVAk46615600;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void qOTzAoEEEy52547338() {     int SjpoImUFuq59401767 = 68994196;    int SjpoImUFuq59064806 = -652100386;    int SjpoImUFuq22548902 = -182107945;    int SjpoImUFuq51767836 = -219995748;    int SjpoImUFuq32981244 = -855582752;    int SjpoImUFuq32609498 = -59485029;    int SjpoImUFuq96653868 = -187650123;    int SjpoImUFuq83826188 = -543111767;    int SjpoImUFuq92757574 = -666744158;    int SjpoImUFuq9791035 = -588690144;    int SjpoImUFuq70862238 = -561676426;    int SjpoImUFuq80451181 = 74364323;    int SjpoImUFuq23079975 = -421492319;    int SjpoImUFuq75667470 = -662003004;    int SjpoImUFuq20950527 = -687785598;    int SjpoImUFuq26730893 = 98437179;    int SjpoImUFuq20095262 = -653157989;    int SjpoImUFuq61020285 = -948234585;    int SjpoImUFuq80825627 = -179199876;    int SjpoImUFuq21744715 = -197388371;    int SjpoImUFuq63999929 = -468095982;    int SjpoImUFuq97352674 = -678985856;    int SjpoImUFuq86948822 = -271174088;    int SjpoImUFuq69200056 = -236495965;    int SjpoImUFuq11571543 = 52033191;    int SjpoImUFuq43310044 = -333568181;    int SjpoImUFuq74309656 = -422585529;    int SjpoImUFuq29134608 = -385711567;    int SjpoImUFuq4616231 = 35377682;    int SjpoImUFuq59918169 = -67559545;    int SjpoImUFuq62136537 = -807266397;    int SjpoImUFuq36184069 = -121213096;    int SjpoImUFuq75530223 = -894414582;    int SjpoImUFuq72685241 = -263081498;    int SjpoImUFuq34153552 = -864831961;    int SjpoImUFuq98713564 = -673730817;    int SjpoImUFuq9747514 = -75030987;    int SjpoImUFuq3899666 = -284134398;    int SjpoImUFuq47423541 = -324873130;    int SjpoImUFuq72500112 = -200081641;    int SjpoImUFuq52248330 = -990762034;    int SjpoImUFuq12844325 = -507332381;    int SjpoImUFuq75151180 = -500139453;    int SjpoImUFuq12502323 = -485793700;    int SjpoImUFuq68442817 = -645174512;    int SjpoImUFuq2102249 = -681790935;    int SjpoImUFuq82021971 = -958122334;    int SjpoImUFuq15867742 = -637259909;    int SjpoImUFuq93146439 = -676938653;    int SjpoImUFuq30112220 = -842234197;    int SjpoImUFuq1696958 = -414822245;    int SjpoImUFuq87446732 = -681787377;    int SjpoImUFuq49860809 = -255370593;    int SjpoImUFuq99792369 = 10714450;    int SjpoImUFuq88547409 = -60492899;    int SjpoImUFuq62049092 = -252019949;    int SjpoImUFuq72115984 = -280926298;    int SjpoImUFuq53348846 = -945611981;    int SjpoImUFuq40196294 = -172028940;    int SjpoImUFuq89671200 = -422014572;    int SjpoImUFuq58299841 = -636899500;    int SjpoImUFuq67519260 = -801938556;    int SjpoImUFuq79209957 = -478489449;    int SjpoImUFuq32839405 = -499184614;    int SjpoImUFuq47654498 = -781423748;    int SjpoImUFuq34678170 = -340463331;    int SjpoImUFuq4920958 = -31221095;    int SjpoImUFuq50394734 = -58410822;    int SjpoImUFuq41513918 = -797171043;    int SjpoImUFuq22236962 = 85945218;    int SjpoImUFuq16983379 = -826531835;    int SjpoImUFuq16195597 = -269023591;    int SjpoImUFuq13596744 = -523361456;    int SjpoImUFuq8325515 = -979118236;    int SjpoImUFuq69496384 = -206626338;    int SjpoImUFuq51155604 = -960763602;    int SjpoImUFuq22201495 = -78846403;    int SjpoImUFuq74446500 = -785380389;    int SjpoImUFuq757239 = -591321453;    int SjpoImUFuq9469295 = -266175874;    int SjpoImUFuq61288073 = -375445847;    int SjpoImUFuq58441915 = -785325620;    int SjpoImUFuq35988169 = -708772914;    int SjpoImUFuq74504011 = -122388122;    int SjpoImUFuq58221212 = -652737300;    int SjpoImUFuq74689804 = -25479020;    int SjpoImUFuq86323259 = -865842503;    int SjpoImUFuq75737853 = -805129033;    int SjpoImUFuq84137832 = -102588599;    int SjpoImUFuq72104460 = -512812013;    int SjpoImUFuq26597581 = -292804519;    int SjpoImUFuq56398668 = -129419006;    int SjpoImUFuq63703372 = -12105459;    int SjpoImUFuq57752341 = -902858558;    int SjpoImUFuq14200272 = -563182141;    int SjpoImUFuq84729069 = -88823478;    int SjpoImUFuq33634367 = 71157067;    int SjpoImUFuq42311776 = 99045160;    int SjpoImUFuq64847825 = -704369953;    int SjpoImUFuq33764647 = 68994196;     SjpoImUFuq59401767 = SjpoImUFuq59064806;     SjpoImUFuq59064806 = SjpoImUFuq22548902;     SjpoImUFuq22548902 = SjpoImUFuq51767836;     SjpoImUFuq51767836 = SjpoImUFuq32981244;     SjpoImUFuq32981244 = SjpoImUFuq32609498;     SjpoImUFuq32609498 = SjpoImUFuq96653868;     SjpoImUFuq96653868 = SjpoImUFuq83826188;     SjpoImUFuq83826188 = SjpoImUFuq92757574;     SjpoImUFuq92757574 = SjpoImUFuq9791035;     SjpoImUFuq9791035 = SjpoImUFuq70862238;     SjpoImUFuq70862238 = SjpoImUFuq80451181;     SjpoImUFuq80451181 = SjpoImUFuq23079975;     SjpoImUFuq23079975 = SjpoImUFuq75667470;     SjpoImUFuq75667470 = SjpoImUFuq20950527;     SjpoImUFuq20950527 = SjpoImUFuq26730893;     SjpoImUFuq26730893 = SjpoImUFuq20095262;     SjpoImUFuq20095262 = SjpoImUFuq61020285;     SjpoImUFuq61020285 = SjpoImUFuq80825627;     SjpoImUFuq80825627 = SjpoImUFuq21744715;     SjpoImUFuq21744715 = SjpoImUFuq63999929;     SjpoImUFuq63999929 = SjpoImUFuq97352674;     SjpoImUFuq97352674 = SjpoImUFuq86948822;     SjpoImUFuq86948822 = SjpoImUFuq69200056;     SjpoImUFuq69200056 = SjpoImUFuq11571543;     SjpoImUFuq11571543 = SjpoImUFuq43310044;     SjpoImUFuq43310044 = SjpoImUFuq74309656;     SjpoImUFuq74309656 = SjpoImUFuq29134608;     SjpoImUFuq29134608 = SjpoImUFuq4616231;     SjpoImUFuq4616231 = SjpoImUFuq59918169;     SjpoImUFuq59918169 = SjpoImUFuq62136537;     SjpoImUFuq62136537 = SjpoImUFuq36184069;     SjpoImUFuq36184069 = SjpoImUFuq75530223;     SjpoImUFuq75530223 = SjpoImUFuq72685241;     SjpoImUFuq72685241 = SjpoImUFuq34153552;     SjpoImUFuq34153552 = SjpoImUFuq98713564;     SjpoImUFuq98713564 = SjpoImUFuq9747514;     SjpoImUFuq9747514 = SjpoImUFuq3899666;     SjpoImUFuq3899666 = SjpoImUFuq47423541;     SjpoImUFuq47423541 = SjpoImUFuq72500112;     SjpoImUFuq72500112 = SjpoImUFuq52248330;     SjpoImUFuq52248330 = SjpoImUFuq12844325;     SjpoImUFuq12844325 = SjpoImUFuq75151180;     SjpoImUFuq75151180 = SjpoImUFuq12502323;     SjpoImUFuq12502323 = SjpoImUFuq68442817;     SjpoImUFuq68442817 = SjpoImUFuq2102249;     SjpoImUFuq2102249 = SjpoImUFuq82021971;     SjpoImUFuq82021971 = SjpoImUFuq15867742;     SjpoImUFuq15867742 = SjpoImUFuq93146439;     SjpoImUFuq93146439 = SjpoImUFuq30112220;     SjpoImUFuq30112220 = SjpoImUFuq1696958;     SjpoImUFuq1696958 = SjpoImUFuq87446732;     SjpoImUFuq87446732 = SjpoImUFuq49860809;     SjpoImUFuq49860809 = SjpoImUFuq99792369;     SjpoImUFuq99792369 = SjpoImUFuq88547409;     SjpoImUFuq88547409 = SjpoImUFuq62049092;     SjpoImUFuq62049092 = SjpoImUFuq72115984;     SjpoImUFuq72115984 = SjpoImUFuq53348846;     SjpoImUFuq53348846 = SjpoImUFuq40196294;     SjpoImUFuq40196294 = SjpoImUFuq89671200;     SjpoImUFuq89671200 = SjpoImUFuq58299841;     SjpoImUFuq58299841 = SjpoImUFuq67519260;     SjpoImUFuq67519260 = SjpoImUFuq79209957;     SjpoImUFuq79209957 = SjpoImUFuq32839405;     SjpoImUFuq32839405 = SjpoImUFuq47654498;     SjpoImUFuq47654498 = SjpoImUFuq34678170;     SjpoImUFuq34678170 = SjpoImUFuq4920958;     SjpoImUFuq4920958 = SjpoImUFuq50394734;     SjpoImUFuq50394734 = SjpoImUFuq41513918;     SjpoImUFuq41513918 = SjpoImUFuq22236962;     SjpoImUFuq22236962 = SjpoImUFuq16983379;     SjpoImUFuq16983379 = SjpoImUFuq16195597;     SjpoImUFuq16195597 = SjpoImUFuq13596744;     SjpoImUFuq13596744 = SjpoImUFuq8325515;     SjpoImUFuq8325515 = SjpoImUFuq69496384;     SjpoImUFuq69496384 = SjpoImUFuq51155604;     SjpoImUFuq51155604 = SjpoImUFuq22201495;     SjpoImUFuq22201495 = SjpoImUFuq74446500;     SjpoImUFuq74446500 = SjpoImUFuq757239;     SjpoImUFuq757239 = SjpoImUFuq9469295;     SjpoImUFuq9469295 = SjpoImUFuq61288073;     SjpoImUFuq61288073 = SjpoImUFuq58441915;     SjpoImUFuq58441915 = SjpoImUFuq35988169;     SjpoImUFuq35988169 = SjpoImUFuq74504011;     SjpoImUFuq74504011 = SjpoImUFuq58221212;     SjpoImUFuq58221212 = SjpoImUFuq74689804;     SjpoImUFuq74689804 = SjpoImUFuq86323259;     SjpoImUFuq86323259 = SjpoImUFuq75737853;     SjpoImUFuq75737853 = SjpoImUFuq84137832;     SjpoImUFuq84137832 = SjpoImUFuq72104460;     SjpoImUFuq72104460 = SjpoImUFuq26597581;     SjpoImUFuq26597581 = SjpoImUFuq56398668;     SjpoImUFuq56398668 = SjpoImUFuq63703372;     SjpoImUFuq63703372 = SjpoImUFuq57752341;     SjpoImUFuq57752341 = SjpoImUFuq14200272;     SjpoImUFuq14200272 = SjpoImUFuq84729069;     SjpoImUFuq84729069 = SjpoImUFuq33634367;     SjpoImUFuq33634367 = SjpoImUFuq42311776;     SjpoImUFuq42311776 = SjpoImUFuq64847825;     SjpoImUFuq64847825 = SjpoImUFuq33764647;     SjpoImUFuq33764647 = SjpoImUFuq59401767;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NHbbYowKMW70760679() {     int HiITNUwpHu42846072 = -811671484;    int HiITNUwpHu75936895 = -651482573;    int HiITNUwpHu18957409 = 24366827;    int HiITNUwpHu14560088 = -413795584;    int HiITNUwpHu39102568 = -662450243;    int HiITNUwpHu31113898 = -938556682;    int HiITNUwpHu67693186 = 72438564;    int HiITNUwpHu98173630 = -530269796;    int HiITNUwpHu51287275 = 91796491;    int HiITNUwpHu42336687 = 42933744;    int HiITNUwpHu20278177 = -751792299;    int HiITNUwpHu43656580 = 86746077;    int HiITNUwpHu41197751 = -213153560;    int HiITNUwpHu55294362 = -450293361;    int HiITNUwpHu81804990 = -781808580;    int HiITNUwpHu13120055 = -789642147;    int HiITNUwpHu13847147 = -406503016;    int HiITNUwpHu41871731 = -95539461;    int HiITNUwpHu43265769 = -177874747;    int HiITNUwpHu11112157 = -726974364;    int HiITNUwpHu19640316 = -57625013;    int HiITNUwpHu9499993 = -670137626;    int HiITNUwpHu90774530 = -311063286;    int HiITNUwpHu3906614 = -308321361;    int HiITNUwpHu53147501 = -507619567;    int HiITNUwpHu38652193 = -214077896;    int HiITNUwpHu23764397 = -50669871;    int HiITNUwpHu68866233 = -455666450;    int HiITNUwpHu40697325 = -927127461;    int HiITNUwpHu75751510 = -580409396;    int HiITNUwpHu80231850 = -445023598;    int HiITNUwpHu99290495 = -36408176;    int HiITNUwpHu19069110 = -515286760;    int HiITNUwpHu14843752 = -954683668;    int HiITNUwpHu12880283 = 37503304;    int HiITNUwpHu11102749 = -461754723;    int HiITNUwpHu78361692 = 46119833;    int HiITNUwpHu44478498 = 7395082;    int HiITNUwpHu96418619 = -509770927;    int HiITNUwpHu21995247 = 47324837;    int HiITNUwpHu85897254 = -562311007;    int HiITNUwpHu63332111 = 18759298;    int HiITNUwpHu98718250 = 48803305;    int HiITNUwpHu4407264 = -241976471;    int HiITNUwpHu60387314 = -373331465;    int HiITNUwpHu41708244 = -701401858;    int HiITNUwpHu58037401 = 9573436;    int HiITNUwpHu35538337 = -635981628;    int HiITNUwpHu23993552 = -162540736;    int HiITNUwpHu65444703 = 50745348;    int HiITNUwpHu74954405 = -635723667;    int HiITNUwpHu42947527 = -740046689;    int HiITNUwpHu63973268 = -467117910;    int HiITNUwpHu24469129 = -807698937;    int HiITNUwpHu51120363 = -408979022;    int HiITNUwpHu33346079 = -41533859;    int HiITNUwpHu85162365 = -240419288;    int HiITNUwpHu15050795 = -667311813;    int HiITNUwpHu61412587 = -906176017;    int HiITNUwpHu450376 = -348372347;    int HiITNUwpHu7349502 = -787886811;    int HiITNUwpHu98826953 = -471894986;    int HiITNUwpHu57476306 = -603142336;    int HiITNUwpHu75535764 = -327794113;    int HiITNUwpHu62104836 = -512042659;    int HiITNUwpHu20987681 = -615384124;    int HiITNUwpHu24587470 = -397967164;    int HiITNUwpHu26353999 = -258469893;    int HiITNUwpHu42414080 = -387796665;    int HiITNUwpHu70702241 = -220053858;    int HiITNUwpHu34758362 = -735761980;    int HiITNUwpHu69368649 = -313898098;    int HiITNUwpHu45453112 = -585768534;    int HiITNUwpHu21270522 = -125199584;    int HiITNUwpHu25214902 = -64663357;    int HiITNUwpHu56308205 = 23615689;    int HiITNUwpHu10781743 = -618940931;    int HiITNUwpHu86367267 = 30913184;    int HiITNUwpHu43519300 = -934989896;    int HiITNUwpHu11439257 = -806217709;    int HiITNUwpHu80614791 = -123651333;    int HiITNUwpHu88226060 = -414688244;    int HiITNUwpHu44872682 = -193125715;    int HiITNUwpHu75252621 = -877872809;    int HiITNUwpHu797106 = -944685729;    int HiITNUwpHu37284324 = -704976909;    int HiITNUwpHu35317228 = -569290266;    int HiITNUwpHu94599981 = -707587823;    int HiITNUwpHu63723388 = -445704646;    int HiITNUwpHu79534203 = -920962838;    int HiITNUwpHu25940384 = -121335435;    int HiITNUwpHu63310898 = -286568354;    int HiITNUwpHu83065910 = -86428902;    int HiITNUwpHu95968244 = -61398580;    int HiITNUwpHu14645746 = -164788353;    int HiITNUwpHu87070301 = 9583978;    int HiITNUwpHu5855805 = -378098367;    int HiITNUwpHu23182486 = -623402583;    int HiITNUwpHu42302427 = -729933812;    int HiITNUwpHu39399633 = -811671484;     HiITNUwpHu42846072 = HiITNUwpHu75936895;     HiITNUwpHu75936895 = HiITNUwpHu18957409;     HiITNUwpHu18957409 = HiITNUwpHu14560088;     HiITNUwpHu14560088 = HiITNUwpHu39102568;     HiITNUwpHu39102568 = HiITNUwpHu31113898;     HiITNUwpHu31113898 = HiITNUwpHu67693186;     HiITNUwpHu67693186 = HiITNUwpHu98173630;     HiITNUwpHu98173630 = HiITNUwpHu51287275;     HiITNUwpHu51287275 = HiITNUwpHu42336687;     HiITNUwpHu42336687 = HiITNUwpHu20278177;     HiITNUwpHu20278177 = HiITNUwpHu43656580;     HiITNUwpHu43656580 = HiITNUwpHu41197751;     HiITNUwpHu41197751 = HiITNUwpHu55294362;     HiITNUwpHu55294362 = HiITNUwpHu81804990;     HiITNUwpHu81804990 = HiITNUwpHu13120055;     HiITNUwpHu13120055 = HiITNUwpHu13847147;     HiITNUwpHu13847147 = HiITNUwpHu41871731;     HiITNUwpHu41871731 = HiITNUwpHu43265769;     HiITNUwpHu43265769 = HiITNUwpHu11112157;     HiITNUwpHu11112157 = HiITNUwpHu19640316;     HiITNUwpHu19640316 = HiITNUwpHu9499993;     HiITNUwpHu9499993 = HiITNUwpHu90774530;     HiITNUwpHu90774530 = HiITNUwpHu3906614;     HiITNUwpHu3906614 = HiITNUwpHu53147501;     HiITNUwpHu53147501 = HiITNUwpHu38652193;     HiITNUwpHu38652193 = HiITNUwpHu23764397;     HiITNUwpHu23764397 = HiITNUwpHu68866233;     HiITNUwpHu68866233 = HiITNUwpHu40697325;     HiITNUwpHu40697325 = HiITNUwpHu75751510;     HiITNUwpHu75751510 = HiITNUwpHu80231850;     HiITNUwpHu80231850 = HiITNUwpHu99290495;     HiITNUwpHu99290495 = HiITNUwpHu19069110;     HiITNUwpHu19069110 = HiITNUwpHu14843752;     HiITNUwpHu14843752 = HiITNUwpHu12880283;     HiITNUwpHu12880283 = HiITNUwpHu11102749;     HiITNUwpHu11102749 = HiITNUwpHu78361692;     HiITNUwpHu78361692 = HiITNUwpHu44478498;     HiITNUwpHu44478498 = HiITNUwpHu96418619;     HiITNUwpHu96418619 = HiITNUwpHu21995247;     HiITNUwpHu21995247 = HiITNUwpHu85897254;     HiITNUwpHu85897254 = HiITNUwpHu63332111;     HiITNUwpHu63332111 = HiITNUwpHu98718250;     HiITNUwpHu98718250 = HiITNUwpHu4407264;     HiITNUwpHu4407264 = HiITNUwpHu60387314;     HiITNUwpHu60387314 = HiITNUwpHu41708244;     HiITNUwpHu41708244 = HiITNUwpHu58037401;     HiITNUwpHu58037401 = HiITNUwpHu35538337;     HiITNUwpHu35538337 = HiITNUwpHu23993552;     HiITNUwpHu23993552 = HiITNUwpHu65444703;     HiITNUwpHu65444703 = HiITNUwpHu74954405;     HiITNUwpHu74954405 = HiITNUwpHu42947527;     HiITNUwpHu42947527 = HiITNUwpHu63973268;     HiITNUwpHu63973268 = HiITNUwpHu24469129;     HiITNUwpHu24469129 = HiITNUwpHu51120363;     HiITNUwpHu51120363 = HiITNUwpHu33346079;     HiITNUwpHu33346079 = HiITNUwpHu85162365;     HiITNUwpHu85162365 = HiITNUwpHu15050795;     HiITNUwpHu15050795 = HiITNUwpHu61412587;     HiITNUwpHu61412587 = HiITNUwpHu450376;     HiITNUwpHu450376 = HiITNUwpHu7349502;     HiITNUwpHu7349502 = HiITNUwpHu98826953;     HiITNUwpHu98826953 = HiITNUwpHu57476306;     HiITNUwpHu57476306 = HiITNUwpHu75535764;     HiITNUwpHu75535764 = HiITNUwpHu62104836;     HiITNUwpHu62104836 = HiITNUwpHu20987681;     HiITNUwpHu20987681 = HiITNUwpHu24587470;     HiITNUwpHu24587470 = HiITNUwpHu26353999;     HiITNUwpHu26353999 = HiITNUwpHu42414080;     HiITNUwpHu42414080 = HiITNUwpHu70702241;     HiITNUwpHu70702241 = HiITNUwpHu34758362;     HiITNUwpHu34758362 = HiITNUwpHu69368649;     HiITNUwpHu69368649 = HiITNUwpHu45453112;     HiITNUwpHu45453112 = HiITNUwpHu21270522;     HiITNUwpHu21270522 = HiITNUwpHu25214902;     HiITNUwpHu25214902 = HiITNUwpHu56308205;     HiITNUwpHu56308205 = HiITNUwpHu10781743;     HiITNUwpHu10781743 = HiITNUwpHu86367267;     HiITNUwpHu86367267 = HiITNUwpHu43519300;     HiITNUwpHu43519300 = HiITNUwpHu11439257;     HiITNUwpHu11439257 = HiITNUwpHu80614791;     HiITNUwpHu80614791 = HiITNUwpHu88226060;     HiITNUwpHu88226060 = HiITNUwpHu44872682;     HiITNUwpHu44872682 = HiITNUwpHu75252621;     HiITNUwpHu75252621 = HiITNUwpHu797106;     HiITNUwpHu797106 = HiITNUwpHu37284324;     HiITNUwpHu37284324 = HiITNUwpHu35317228;     HiITNUwpHu35317228 = HiITNUwpHu94599981;     HiITNUwpHu94599981 = HiITNUwpHu63723388;     HiITNUwpHu63723388 = HiITNUwpHu79534203;     HiITNUwpHu79534203 = HiITNUwpHu25940384;     HiITNUwpHu25940384 = HiITNUwpHu63310898;     HiITNUwpHu63310898 = HiITNUwpHu83065910;     HiITNUwpHu83065910 = HiITNUwpHu95968244;     HiITNUwpHu95968244 = HiITNUwpHu14645746;     HiITNUwpHu14645746 = HiITNUwpHu87070301;     HiITNUwpHu87070301 = HiITNUwpHu5855805;     HiITNUwpHu5855805 = HiITNUwpHu23182486;     HiITNUwpHu23182486 = HiITNUwpHu42302427;     HiITNUwpHu42302427 = HiITNUwpHu39399633;     HiITNUwpHu39399633 = HiITNUwpHu42846072;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void umtwWoCMId71202782() {     int UqNEOphajY78025828 = -708591998;    int UqNEOphajY8482968 = -727764516;    int UqNEOphajY64166275 = -666517785;    int UqNEOphajY52427741 = -423864562;    int UqNEOphajY21947825 = -176968388;    int UqNEOphajY54501653 = -876655561;    int UqNEOphajY12509348 = -590684346;    int UqNEOphajY31312552 = -641207380;    int UqNEOphajY61115052 = -40511570;    int UqNEOphajY43679462 = -967088351;    int UqNEOphajY64986813 = -903630643;    int UqNEOphajY17218358 = -935853670;    int UqNEOphajY46496321 = -713995062;    int UqNEOphajY93688973 = -965264264;    int UqNEOphajY96674711 = -353559272;    int UqNEOphajY11236489 = -289405428;    int UqNEOphajY63569557 = -527942733;    int UqNEOphajY8406100 = -805420832;    int UqNEOphajY65279760 = -704463801;    int UqNEOphajY20209174 = -966828505;    int UqNEOphajY21028056 = -829962212;    int UqNEOphajY24066765 = -834770404;    int UqNEOphajY95949752 = 40810070;    int UqNEOphajY31263423 = -251738708;    int UqNEOphajY66139317 = 87073237;    int UqNEOphajY91699755 = -440197391;    int UqNEOphajY12047772 = -141162932;    int UqNEOphajY6727084 = -908952020;    int UqNEOphajY51528492 = -839531505;    int UqNEOphajY61569935 = -455160043;    int UqNEOphajY35545973 = -415663265;    int UqNEOphajY75426652 = -275377447;    int UqNEOphajY60701306 = -718540823;    int UqNEOphajY51025975 = -206883004;    int UqNEOphajY89812141 = -376971075;    int UqNEOphajY5191544 = -485947457;    int UqNEOphajY96624097 = -477084090;    int UqNEOphajY85890941 = -884587803;    int UqNEOphajY15188540 = -39986120;    int UqNEOphajY96572798 = -323884352;    int UqNEOphajY88230266 = -253018768;    int UqNEOphajY73845297 = -543668645;    int UqNEOphajY78868552 = -403030470;    int UqNEOphajY89248022 = 76452081;    int UqNEOphajY24361019 = -701985502;    int UqNEOphajY83008687 = -44904723;    int UqNEOphajY20219026 = -585071247;    int UqNEOphajY34193397 = 60891958;    int UqNEOphajY43859117 = -712095547;    int UqNEOphajY89433446 = -15286488;    int UqNEOphajY88891745 = -565129245;    int UqNEOphajY30428074 = -159905561;    int UqNEOphajY1324473 = -422978414;    int UqNEOphajY19205129 = -140911637;    int UqNEOphajY50174727 = -787228346;    int UqNEOphajY53959063 = -873821594;    int UqNEOphajY12533216 = -668574586;    int UqNEOphajY32902853 = -314779078;    int UqNEOphajY86288424 = -410937800;    int UqNEOphajY30248069 = -736770998;    int UqNEOphajY42453882 = -635492630;    int UqNEOphajY5782264 = -681732326;    int UqNEOphajY79784060 = -801675876;    int UqNEOphajY99545117 = -585351527;    int UqNEOphajY8133490 = -451425086;    int UqNEOphajY89560160 = -528253196;    int UqNEOphajY56517051 = -117312848;    int UqNEOphajY95470345 = -407112059;    int UqNEOphajY3876832 = -488293190;    int UqNEOphajY91483167 = -867611816;    int UqNEOphajY14612391 = -812321338;    int UqNEOphajY77678615 = -643354930;    int UqNEOphajY93217560 = -665434713;    int UqNEOphajY68706962 = -280579449;    int UqNEOphajY31978907 = -613809738;    int UqNEOphajY47182759 = -186293567;    int UqNEOphajY45198213 = -331739934;    int UqNEOphajY6701731 = 64357988;    int UqNEOphajY6902404 = -549753206;    int UqNEOphajY83130630 = -868022041;    int UqNEOphajY71480730 = -855126145;    int UqNEOphajY77854375 = -102054890;    int UqNEOphajY62867967 = -96856474;    int UqNEOphajY62095045 = -724245018;    int UqNEOphajY72678190 = -890030799;    int UqNEOphajY5117899 = -155757705;    int UqNEOphajY74102179 = -852399034;    int UqNEOphajY41496178 = -477629186;    int UqNEOphajY851249 = -419654658;    int UqNEOphajY35853078 = -503149481;    int UqNEOphajY92658328 = -817372871;    int UqNEOphajY63721245 = -62305013;    int UqNEOphajY99602517 = -373650004;    int UqNEOphajY84940471 = -303215123;    int UqNEOphajY54118917 = -688391722;    int UqNEOphajY82448002 = -571286443;    int UqNEOphajY94061237 = -741992770;    int UqNEOphajY79323435 = -817678943;    int UqNEOphajY81114532 = -472122833;    int UqNEOphajY34800859 = -708591998;     UqNEOphajY78025828 = UqNEOphajY8482968;     UqNEOphajY8482968 = UqNEOphajY64166275;     UqNEOphajY64166275 = UqNEOphajY52427741;     UqNEOphajY52427741 = UqNEOphajY21947825;     UqNEOphajY21947825 = UqNEOphajY54501653;     UqNEOphajY54501653 = UqNEOphajY12509348;     UqNEOphajY12509348 = UqNEOphajY31312552;     UqNEOphajY31312552 = UqNEOphajY61115052;     UqNEOphajY61115052 = UqNEOphajY43679462;     UqNEOphajY43679462 = UqNEOphajY64986813;     UqNEOphajY64986813 = UqNEOphajY17218358;     UqNEOphajY17218358 = UqNEOphajY46496321;     UqNEOphajY46496321 = UqNEOphajY93688973;     UqNEOphajY93688973 = UqNEOphajY96674711;     UqNEOphajY96674711 = UqNEOphajY11236489;     UqNEOphajY11236489 = UqNEOphajY63569557;     UqNEOphajY63569557 = UqNEOphajY8406100;     UqNEOphajY8406100 = UqNEOphajY65279760;     UqNEOphajY65279760 = UqNEOphajY20209174;     UqNEOphajY20209174 = UqNEOphajY21028056;     UqNEOphajY21028056 = UqNEOphajY24066765;     UqNEOphajY24066765 = UqNEOphajY95949752;     UqNEOphajY95949752 = UqNEOphajY31263423;     UqNEOphajY31263423 = UqNEOphajY66139317;     UqNEOphajY66139317 = UqNEOphajY91699755;     UqNEOphajY91699755 = UqNEOphajY12047772;     UqNEOphajY12047772 = UqNEOphajY6727084;     UqNEOphajY6727084 = UqNEOphajY51528492;     UqNEOphajY51528492 = UqNEOphajY61569935;     UqNEOphajY61569935 = UqNEOphajY35545973;     UqNEOphajY35545973 = UqNEOphajY75426652;     UqNEOphajY75426652 = UqNEOphajY60701306;     UqNEOphajY60701306 = UqNEOphajY51025975;     UqNEOphajY51025975 = UqNEOphajY89812141;     UqNEOphajY89812141 = UqNEOphajY5191544;     UqNEOphajY5191544 = UqNEOphajY96624097;     UqNEOphajY96624097 = UqNEOphajY85890941;     UqNEOphajY85890941 = UqNEOphajY15188540;     UqNEOphajY15188540 = UqNEOphajY96572798;     UqNEOphajY96572798 = UqNEOphajY88230266;     UqNEOphajY88230266 = UqNEOphajY73845297;     UqNEOphajY73845297 = UqNEOphajY78868552;     UqNEOphajY78868552 = UqNEOphajY89248022;     UqNEOphajY89248022 = UqNEOphajY24361019;     UqNEOphajY24361019 = UqNEOphajY83008687;     UqNEOphajY83008687 = UqNEOphajY20219026;     UqNEOphajY20219026 = UqNEOphajY34193397;     UqNEOphajY34193397 = UqNEOphajY43859117;     UqNEOphajY43859117 = UqNEOphajY89433446;     UqNEOphajY89433446 = UqNEOphajY88891745;     UqNEOphajY88891745 = UqNEOphajY30428074;     UqNEOphajY30428074 = UqNEOphajY1324473;     UqNEOphajY1324473 = UqNEOphajY19205129;     UqNEOphajY19205129 = UqNEOphajY50174727;     UqNEOphajY50174727 = UqNEOphajY53959063;     UqNEOphajY53959063 = UqNEOphajY12533216;     UqNEOphajY12533216 = UqNEOphajY32902853;     UqNEOphajY32902853 = UqNEOphajY86288424;     UqNEOphajY86288424 = UqNEOphajY30248069;     UqNEOphajY30248069 = UqNEOphajY42453882;     UqNEOphajY42453882 = UqNEOphajY5782264;     UqNEOphajY5782264 = UqNEOphajY79784060;     UqNEOphajY79784060 = UqNEOphajY99545117;     UqNEOphajY99545117 = UqNEOphajY8133490;     UqNEOphajY8133490 = UqNEOphajY89560160;     UqNEOphajY89560160 = UqNEOphajY56517051;     UqNEOphajY56517051 = UqNEOphajY95470345;     UqNEOphajY95470345 = UqNEOphajY3876832;     UqNEOphajY3876832 = UqNEOphajY91483167;     UqNEOphajY91483167 = UqNEOphajY14612391;     UqNEOphajY14612391 = UqNEOphajY77678615;     UqNEOphajY77678615 = UqNEOphajY93217560;     UqNEOphajY93217560 = UqNEOphajY68706962;     UqNEOphajY68706962 = UqNEOphajY31978907;     UqNEOphajY31978907 = UqNEOphajY47182759;     UqNEOphajY47182759 = UqNEOphajY45198213;     UqNEOphajY45198213 = UqNEOphajY6701731;     UqNEOphajY6701731 = UqNEOphajY6902404;     UqNEOphajY6902404 = UqNEOphajY83130630;     UqNEOphajY83130630 = UqNEOphajY71480730;     UqNEOphajY71480730 = UqNEOphajY77854375;     UqNEOphajY77854375 = UqNEOphajY62867967;     UqNEOphajY62867967 = UqNEOphajY62095045;     UqNEOphajY62095045 = UqNEOphajY72678190;     UqNEOphajY72678190 = UqNEOphajY5117899;     UqNEOphajY5117899 = UqNEOphajY74102179;     UqNEOphajY74102179 = UqNEOphajY41496178;     UqNEOphajY41496178 = UqNEOphajY851249;     UqNEOphajY851249 = UqNEOphajY35853078;     UqNEOphajY35853078 = UqNEOphajY92658328;     UqNEOphajY92658328 = UqNEOphajY63721245;     UqNEOphajY63721245 = UqNEOphajY99602517;     UqNEOphajY99602517 = UqNEOphajY84940471;     UqNEOphajY84940471 = UqNEOphajY54118917;     UqNEOphajY54118917 = UqNEOphajY82448002;     UqNEOphajY82448002 = UqNEOphajY94061237;     UqNEOphajY94061237 = UqNEOphajY79323435;     UqNEOphajY79323435 = UqNEOphajY81114532;     UqNEOphajY81114532 = UqNEOphajY34800859;     UqNEOphajY34800859 = UqNEOphajY78025828;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void MRsLRFqHVJ41658655() {     int YcQwaawVhd90811995 = -843632690;    int YcQwaawVhd95253855 = -898098530;    int YcQwaawVhd81919075 = 25122833;    int YcQwaawVhd60368567 = -248066839;    int YcQwaawVhd15458929 = -668784853;    int YcQwaawVhd97811724 = -653578875;    int YcQwaawVhd33717106 = -36356416;    int YcQwaawVhd58031664 = 14274363;    int YcQwaawVhd56519863 = -2269662;    int YcQwaawVhd49898167 = -771175983;    int YcQwaawVhd92474195 = 81683342;    int YcQwaawVhd49168862 = -343186486;    int YcQwaawVhd16639624 = -617777719;    int YcQwaawVhd43313052 = -197679463;    int YcQwaawVhd19981264 = -260545103;    int YcQwaawVhd39661557 = -840296817;    int YcQwaawVhd19321375 = -191717200;    int YcQwaawVhd91964585 = -360631134;    int YcQwaawVhd66440391 = -613932996;    int YcQwaawVhd22863670 = -466072644;    int YcQwaawVhd1202113 = -221278476;    int YcQwaawVhd53114584 = 95371248;    int YcQwaawVhd34710048 = -990193825;    int YcQwaawVhd75770552 = -612083720;    int YcQwaawVhd72033576 = -123368687;    int YcQwaawVhd73018401 = -130628590;    int YcQwaawVhd23463308 = -774869212;    int YcQwaawVhd74080008 = 17249875;    int YcQwaawVhd68145544 = 46251255;    int YcQwaawVhd14321051 = -85046198;    int YcQwaawVhd79981969 = -692080015;    int YcQwaawVhd78745476 = -254097126;    int YcQwaawVhd6747256 = -594395607;    int YcQwaawVhd28102348 = -111637222;    int YcQwaawVhd72872673 = -987002955;    int YcQwaawVhd415659 = -274510561;    int YcQwaawVhd33388158 = -266993441;    int YcQwaawVhd43594965 = -804207895;    int YcQwaawVhd20963926 = -981836697;    int YcQwaawVhd22837528 = -168301198;    int YcQwaawVhd49661574 = -628492761;    int YcQwaawVhd81547753 = -441979980;    int YcQwaawVhd13752022 = -426463915;    int YcQwaawVhd33876560 = -264720162;    int YcQwaawVhd4369510 = -594755462;    int YcQwaawVhd56636817 = -118223162;    int YcQwaawVhd97801045 = 17413701;    int YcQwaawVhd63633363 = -94460821;    int YcQwaawVhd87923166 = -875697522;    int YcQwaawVhd36383867 = -626322951;    int YcQwaawVhd7219240 = -818013554;    int YcQwaawVhd58604621 = -631090899;    int YcQwaawVhd35809623 = -598981697;    int YcQwaawVhd63904853 = -830363379;    int YcQwaawVhd40456543 = -781672832;    int YcQwaawVhd37697412 = -839003938;    int YcQwaawVhd60543808 = -907904705;    int YcQwaawVhd6148523 = -362793448;    int YcQwaawVhd88334990 = -24698152;    int YcQwaawVhd42440528 = -438156264;    int YcQwaawVhd74348416 = -878709663;    int YcQwaawVhd59637097 = 46393708;    int YcQwaawVhd89886119 = 68023108;    int YcQwaawVhd42198812 = -917223465;    int YcQwaawVhd69916198 = 20904031;    int YcQwaawVhd13728719 = -664219533;    int YcQwaawVhd42421607 = -748790879;    int YcQwaawVhd88537275 = -406140498;    int YcQwaawVhd70440378 = -210676508;    int YcQwaawVhd19565606 = -986034543;    int YcQwaawVhd6273400 = -473303376;    int YcQwaawVhd75726409 = -387509306;    int YcQwaawVhd71000659 = -378794438;    int YcQwaawVhd43602863 = -345631799;    int YcQwaawVhd73202095 = -837579884;    int YcQwaawVhd19654360 = -779298497;    int YcQwaawVhd39362562 = -478164838;    int YcQwaawVhd833489 = -625473664;    int YcQwaawVhd71401043 = 82671741;    int YcQwaawVhd15396760 = 94854474;    int YcQwaawVhd75217355 = -48042291;    int YcQwaawVhd59829945 = -580408391;    int YcQwaawVhd86156842 = -107052604;    int YcQwaawVhd31761678 = -327425795;    int YcQwaawVhd7101812 = -267032645;    int YcQwaawVhd21377348 = 39010883;    int YcQwaawVhd42935853 = -655115429;    int YcQwaawVhd42842402 = -764032229;    int YcQwaawVhd87645805 = -329964390;    int YcQwaawVhd35175262 = -47999017;    int YcQwaawVhd39871850 = -366605856;    int YcQwaawVhd27239635 = -904199994;    int YcQwaawVhd55259974 = -679509743;    int YcQwaawVhd78523398 = -443680434;    int YcQwaawVhd48489112 = -289591535;    int YcQwaawVhd90024477 = -574886470;    int YcQwaawVhd91661633 = -410003088;    int YcQwaawVhd71553210 = -509240451;    int YcQwaawVhd63960361 = -185624193;    int YcQwaawVhd90640790 = -843632690;     YcQwaawVhd90811995 = YcQwaawVhd95253855;     YcQwaawVhd95253855 = YcQwaawVhd81919075;     YcQwaawVhd81919075 = YcQwaawVhd60368567;     YcQwaawVhd60368567 = YcQwaawVhd15458929;     YcQwaawVhd15458929 = YcQwaawVhd97811724;     YcQwaawVhd97811724 = YcQwaawVhd33717106;     YcQwaawVhd33717106 = YcQwaawVhd58031664;     YcQwaawVhd58031664 = YcQwaawVhd56519863;     YcQwaawVhd56519863 = YcQwaawVhd49898167;     YcQwaawVhd49898167 = YcQwaawVhd92474195;     YcQwaawVhd92474195 = YcQwaawVhd49168862;     YcQwaawVhd49168862 = YcQwaawVhd16639624;     YcQwaawVhd16639624 = YcQwaawVhd43313052;     YcQwaawVhd43313052 = YcQwaawVhd19981264;     YcQwaawVhd19981264 = YcQwaawVhd39661557;     YcQwaawVhd39661557 = YcQwaawVhd19321375;     YcQwaawVhd19321375 = YcQwaawVhd91964585;     YcQwaawVhd91964585 = YcQwaawVhd66440391;     YcQwaawVhd66440391 = YcQwaawVhd22863670;     YcQwaawVhd22863670 = YcQwaawVhd1202113;     YcQwaawVhd1202113 = YcQwaawVhd53114584;     YcQwaawVhd53114584 = YcQwaawVhd34710048;     YcQwaawVhd34710048 = YcQwaawVhd75770552;     YcQwaawVhd75770552 = YcQwaawVhd72033576;     YcQwaawVhd72033576 = YcQwaawVhd73018401;     YcQwaawVhd73018401 = YcQwaawVhd23463308;     YcQwaawVhd23463308 = YcQwaawVhd74080008;     YcQwaawVhd74080008 = YcQwaawVhd68145544;     YcQwaawVhd68145544 = YcQwaawVhd14321051;     YcQwaawVhd14321051 = YcQwaawVhd79981969;     YcQwaawVhd79981969 = YcQwaawVhd78745476;     YcQwaawVhd78745476 = YcQwaawVhd6747256;     YcQwaawVhd6747256 = YcQwaawVhd28102348;     YcQwaawVhd28102348 = YcQwaawVhd72872673;     YcQwaawVhd72872673 = YcQwaawVhd415659;     YcQwaawVhd415659 = YcQwaawVhd33388158;     YcQwaawVhd33388158 = YcQwaawVhd43594965;     YcQwaawVhd43594965 = YcQwaawVhd20963926;     YcQwaawVhd20963926 = YcQwaawVhd22837528;     YcQwaawVhd22837528 = YcQwaawVhd49661574;     YcQwaawVhd49661574 = YcQwaawVhd81547753;     YcQwaawVhd81547753 = YcQwaawVhd13752022;     YcQwaawVhd13752022 = YcQwaawVhd33876560;     YcQwaawVhd33876560 = YcQwaawVhd4369510;     YcQwaawVhd4369510 = YcQwaawVhd56636817;     YcQwaawVhd56636817 = YcQwaawVhd97801045;     YcQwaawVhd97801045 = YcQwaawVhd63633363;     YcQwaawVhd63633363 = YcQwaawVhd87923166;     YcQwaawVhd87923166 = YcQwaawVhd36383867;     YcQwaawVhd36383867 = YcQwaawVhd7219240;     YcQwaawVhd7219240 = YcQwaawVhd58604621;     YcQwaawVhd58604621 = YcQwaawVhd35809623;     YcQwaawVhd35809623 = YcQwaawVhd63904853;     YcQwaawVhd63904853 = YcQwaawVhd40456543;     YcQwaawVhd40456543 = YcQwaawVhd37697412;     YcQwaawVhd37697412 = YcQwaawVhd60543808;     YcQwaawVhd60543808 = YcQwaawVhd6148523;     YcQwaawVhd6148523 = YcQwaawVhd88334990;     YcQwaawVhd88334990 = YcQwaawVhd42440528;     YcQwaawVhd42440528 = YcQwaawVhd74348416;     YcQwaawVhd74348416 = YcQwaawVhd59637097;     YcQwaawVhd59637097 = YcQwaawVhd89886119;     YcQwaawVhd89886119 = YcQwaawVhd42198812;     YcQwaawVhd42198812 = YcQwaawVhd69916198;     YcQwaawVhd69916198 = YcQwaawVhd13728719;     YcQwaawVhd13728719 = YcQwaawVhd42421607;     YcQwaawVhd42421607 = YcQwaawVhd88537275;     YcQwaawVhd88537275 = YcQwaawVhd70440378;     YcQwaawVhd70440378 = YcQwaawVhd19565606;     YcQwaawVhd19565606 = YcQwaawVhd6273400;     YcQwaawVhd6273400 = YcQwaawVhd75726409;     YcQwaawVhd75726409 = YcQwaawVhd71000659;     YcQwaawVhd71000659 = YcQwaawVhd43602863;     YcQwaawVhd43602863 = YcQwaawVhd73202095;     YcQwaawVhd73202095 = YcQwaawVhd19654360;     YcQwaawVhd19654360 = YcQwaawVhd39362562;     YcQwaawVhd39362562 = YcQwaawVhd833489;     YcQwaawVhd833489 = YcQwaawVhd71401043;     YcQwaawVhd71401043 = YcQwaawVhd15396760;     YcQwaawVhd15396760 = YcQwaawVhd75217355;     YcQwaawVhd75217355 = YcQwaawVhd59829945;     YcQwaawVhd59829945 = YcQwaawVhd86156842;     YcQwaawVhd86156842 = YcQwaawVhd31761678;     YcQwaawVhd31761678 = YcQwaawVhd7101812;     YcQwaawVhd7101812 = YcQwaawVhd21377348;     YcQwaawVhd21377348 = YcQwaawVhd42935853;     YcQwaawVhd42935853 = YcQwaawVhd42842402;     YcQwaawVhd42842402 = YcQwaawVhd87645805;     YcQwaawVhd87645805 = YcQwaawVhd35175262;     YcQwaawVhd35175262 = YcQwaawVhd39871850;     YcQwaawVhd39871850 = YcQwaawVhd27239635;     YcQwaawVhd27239635 = YcQwaawVhd55259974;     YcQwaawVhd55259974 = YcQwaawVhd78523398;     YcQwaawVhd78523398 = YcQwaawVhd48489112;     YcQwaawVhd48489112 = YcQwaawVhd90024477;     YcQwaawVhd90024477 = YcQwaawVhd91661633;     YcQwaawVhd91661633 = YcQwaawVhd71553210;     YcQwaawVhd71553210 = YcQwaawVhd63960361;     YcQwaawVhd63960361 = YcQwaawVhd90640790;     YcQwaawVhd90640790 = YcQwaawVhd90811995;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cEiABagaWR59871997() {     int qIXFPxlBUZ74256301 = -624298370;    int qIXFPxlBUZ12125945 = -897480718;    int qIXFPxlBUZ78327581 = -868402395;    int qIXFPxlBUZ23160819 = -441866675;    int qIXFPxlBUZ21580253 = -475652344;    int qIXFPxlBUZ96316124 = -432650528;    int qIXFPxlBUZ4756424 = -876267729;    int qIXFPxlBUZ72379107 = 27116334;    int qIXFPxlBUZ15049564 = -343729013;    int qIXFPxlBUZ82443819 = -139552096;    int qIXFPxlBUZ41890133 = -108432531;    int qIXFPxlBUZ12374262 = -330804732;    int qIXFPxlBUZ34757399 = -409438960;    int qIXFPxlBUZ22939944 = 14030180;    int qIXFPxlBUZ80835727 = -354568085;    int qIXFPxlBUZ26050719 = -628376142;    int qIXFPxlBUZ13073260 = 54937773;    int qIXFPxlBUZ72816031 = -607936011;    int qIXFPxlBUZ28880533 = -612607867;    int qIXFPxlBUZ12231112 = -995658637;    int qIXFPxlBUZ56842500 = -910807507;    int qIXFPxlBUZ65261902 = -995780522;    int qIXFPxlBUZ38535756 = 69916978;    int qIXFPxlBUZ10477111 = -683909116;    int qIXFPxlBUZ13609535 = -683021446;    int qIXFPxlBUZ68360550 = -11138306;    int qIXFPxlBUZ72918048 = -402953554;    int qIXFPxlBUZ13811634 = -52705009;    int qIXFPxlBUZ4226639 = -916253887;    int qIXFPxlBUZ30154392 = -597896050;    int qIXFPxlBUZ98077282 = -329837216;    int qIXFPxlBUZ41851903 = -169292205;    int qIXFPxlBUZ50286142 = -215267785;    int qIXFPxlBUZ70260858 = -803239392;    int qIXFPxlBUZ51599404 = -84667690;    int qIXFPxlBUZ12804843 = -62534467;    int qIXFPxlBUZ2002337 = -145842621;    int qIXFPxlBUZ84173796 = -512678415;    int qIXFPxlBUZ69959004 = -66734495;    int qIXFPxlBUZ72332663 = 79105280;    int qIXFPxlBUZ83310498 = -200041735;    int qIXFPxlBUZ32035539 = 84111699;    int qIXFPxlBUZ37319092 = -977521157;    int qIXFPxlBUZ25781500 = -20902933;    int qIXFPxlBUZ96314005 = -322912415;    int qIXFPxlBUZ96242812 = -137834086;    int qIXFPxlBUZ73816476 = -114890529;    int qIXFPxlBUZ83303958 = -93182540;    int qIXFPxlBUZ18770279 = -361299604;    int qIXFPxlBUZ71716351 = -833343406;    int qIXFPxlBUZ80476687 = 61085024;    int qIXFPxlBUZ14105416 = -689350210;    int qIXFPxlBUZ49922082 = -810729013;    int qIXFPxlBUZ88581612 = -548776766;    int qIXFPxlBUZ3029498 = -30158955;    int qIXFPxlBUZ8994399 = -628517849;    int qIXFPxlBUZ73590189 = -867397696;    int qIXFPxlBUZ67850471 = -84493279;    int qIXFPxlBUZ9551284 = -758845229;    int qIXFPxlBUZ53219703 = -364514039;    int qIXFPxlBUZ23398077 = 70303026;    int qIXFPxlBUZ90944789 = -723562721;    int qIXFPxlBUZ68152468 = -56629779;    int qIXFPxlBUZ84895171 = -745832964;    int qIXFPxlBUZ84366536 = -809714880;    int qIXFPxlBUZ38230 = -939140326;    int qIXFPxlBUZ62088119 = -15536948;    int qIXFPxlBUZ64496541 = -606199569;    int qIXFPxlBUZ71340540 = -901302130;    int qIXFPxlBUZ68030885 = -192033619;    int qIXFPxlBUZ24048383 = -382533522;    int qIXFPxlBUZ28899463 = -432383813;    int qIXFPxlBUZ2857028 = -441201517;    int qIXFPxlBUZ56547870 = -591713148;    int qIXFPxlBUZ28920614 = -695616903;    int qIXFPxlBUZ24806961 = -894919207;    int qIXFPxlBUZ27942811 = 81740635;    int qIXFPxlBUZ12754256 = -909180090;    int qIXFPxlBUZ14163105 = -260996701;    int qIXFPxlBUZ17366722 = -445187361;    int qIXFPxlBUZ94544074 = -896247777;    int qIXFPxlBUZ89614089 = -209771015;    int qIXFPxlBUZ95041355 = -691405405;    int qIXFPxlBUZ32510288 = 17089518;    int qIXFPxlBUZ49677705 = -558981074;    int qIXFPxlBUZ83971867 = -640487006;    int qIXFPxlBUZ91929821 = -358563193;    int qIXFPxlBUZ61704530 = -666491019;    int qIXFPxlBUZ67231361 = -673080437;    int qIXFPxlBUZ42605005 = -456149842;    int qIXFPxlBUZ39214653 = -195136772;    int qIXFPxlBUZ34151865 = 38650658;    int qIXFPxlBUZ74622513 = -753833186;    int qIXFPxlBUZ16739301 = -702220456;    int qIXFPxlBUZ48934587 = -991197746;    int qIXFPxlBUZ92365709 = -476479014;    int qIXFPxlBUZ63883071 = -859258522;    int qIXFPxlBUZ52423920 = -131688194;    int qIXFPxlBUZ41414964 = -211188053;    int qIXFPxlBUZ96275776 = -624298370;     qIXFPxlBUZ74256301 = qIXFPxlBUZ12125945;     qIXFPxlBUZ12125945 = qIXFPxlBUZ78327581;     qIXFPxlBUZ78327581 = qIXFPxlBUZ23160819;     qIXFPxlBUZ23160819 = qIXFPxlBUZ21580253;     qIXFPxlBUZ21580253 = qIXFPxlBUZ96316124;     qIXFPxlBUZ96316124 = qIXFPxlBUZ4756424;     qIXFPxlBUZ4756424 = qIXFPxlBUZ72379107;     qIXFPxlBUZ72379107 = qIXFPxlBUZ15049564;     qIXFPxlBUZ15049564 = qIXFPxlBUZ82443819;     qIXFPxlBUZ82443819 = qIXFPxlBUZ41890133;     qIXFPxlBUZ41890133 = qIXFPxlBUZ12374262;     qIXFPxlBUZ12374262 = qIXFPxlBUZ34757399;     qIXFPxlBUZ34757399 = qIXFPxlBUZ22939944;     qIXFPxlBUZ22939944 = qIXFPxlBUZ80835727;     qIXFPxlBUZ80835727 = qIXFPxlBUZ26050719;     qIXFPxlBUZ26050719 = qIXFPxlBUZ13073260;     qIXFPxlBUZ13073260 = qIXFPxlBUZ72816031;     qIXFPxlBUZ72816031 = qIXFPxlBUZ28880533;     qIXFPxlBUZ28880533 = qIXFPxlBUZ12231112;     qIXFPxlBUZ12231112 = qIXFPxlBUZ56842500;     qIXFPxlBUZ56842500 = qIXFPxlBUZ65261902;     qIXFPxlBUZ65261902 = qIXFPxlBUZ38535756;     qIXFPxlBUZ38535756 = qIXFPxlBUZ10477111;     qIXFPxlBUZ10477111 = qIXFPxlBUZ13609535;     qIXFPxlBUZ13609535 = qIXFPxlBUZ68360550;     qIXFPxlBUZ68360550 = qIXFPxlBUZ72918048;     qIXFPxlBUZ72918048 = qIXFPxlBUZ13811634;     qIXFPxlBUZ13811634 = qIXFPxlBUZ4226639;     qIXFPxlBUZ4226639 = qIXFPxlBUZ30154392;     qIXFPxlBUZ30154392 = qIXFPxlBUZ98077282;     qIXFPxlBUZ98077282 = qIXFPxlBUZ41851903;     qIXFPxlBUZ41851903 = qIXFPxlBUZ50286142;     qIXFPxlBUZ50286142 = qIXFPxlBUZ70260858;     qIXFPxlBUZ70260858 = qIXFPxlBUZ51599404;     qIXFPxlBUZ51599404 = qIXFPxlBUZ12804843;     qIXFPxlBUZ12804843 = qIXFPxlBUZ2002337;     qIXFPxlBUZ2002337 = qIXFPxlBUZ84173796;     qIXFPxlBUZ84173796 = qIXFPxlBUZ69959004;     qIXFPxlBUZ69959004 = qIXFPxlBUZ72332663;     qIXFPxlBUZ72332663 = qIXFPxlBUZ83310498;     qIXFPxlBUZ83310498 = qIXFPxlBUZ32035539;     qIXFPxlBUZ32035539 = qIXFPxlBUZ37319092;     qIXFPxlBUZ37319092 = qIXFPxlBUZ25781500;     qIXFPxlBUZ25781500 = qIXFPxlBUZ96314005;     qIXFPxlBUZ96314005 = qIXFPxlBUZ96242812;     qIXFPxlBUZ96242812 = qIXFPxlBUZ73816476;     qIXFPxlBUZ73816476 = qIXFPxlBUZ83303958;     qIXFPxlBUZ83303958 = qIXFPxlBUZ18770279;     qIXFPxlBUZ18770279 = qIXFPxlBUZ71716351;     qIXFPxlBUZ71716351 = qIXFPxlBUZ80476687;     qIXFPxlBUZ80476687 = qIXFPxlBUZ14105416;     qIXFPxlBUZ14105416 = qIXFPxlBUZ49922082;     qIXFPxlBUZ49922082 = qIXFPxlBUZ88581612;     qIXFPxlBUZ88581612 = qIXFPxlBUZ3029498;     qIXFPxlBUZ3029498 = qIXFPxlBUZ8994399;     qIXFPxlBUZ8994399 = qIXFPxlBUZ73590189;     qIXFPxlBUZ73590189 = qIXFPxlBUZ67850471;     qIXFPxlBUZ67850471 = qIXFPxlBUZ9551284;     qIXFPxlBUZ9551284 = qIXFPxlBUZ53219703;     qIXFPxlBUZ53219703 = qIXFPxlBUZ23398077;     qIXFPxlBUZ23398077 = qIXFPxlBUZ90944789;     qIXFPxlBUZ90944789 = qIXFPxlBUZ68152468;     qIXFPxlBUZ68152468 = qIXFPxlBUZ84895171;     qIXFPxlBUZ84895171 = qIXFPxlBUZ84366536;     qIXFPxlBUZ84366536 = qIXFPxlBUZ38230;     qIXFPxlBUZ38230 = qIXFPxlBUZ62088119;     qIXFPxlBUZ62088119 = qIXFPxlBUZ64496541;     qIXFPxlBUZ64496541 = qIXFPxlBUZ71340540;     qIXFPxlBUZ71340540 = qIXFPxlBUZ68030885;     qIXFPxlBUZ68030885 = qIXFPxlBUZ24048383;     qIXFPxlBUZ24048383 = qIXFPxlBUZ28899463;     qIXFPxlBUZ28899463 = qIXFPxlBUZ2857028;     qIXFPxlBUZ2857028 = qIXFPxlBUZ56547870;     qIXFPxlBUZ56547870 = qIXFPxlBUZ28920614;     qIXFPxlBUZ28920614 = qIXFPxlBUZ24806961;     qIXFPxlBUZ24806961 = qIXFPxlBUZ27942811;     qIXFPxlBUZ27942811 = qIXFPxlBUZ12754256;     qIXFPxlBUZ12754256 = qIXFPxlBUZ14163105;     qIXFPxlBUZ14163105 = qIXFPxlBUZ17366722;     qIXFPxlBUZ17366722 = qIXFPxlBUZ94544074;     qIXFPxlBUZ94544074 = qIXFPxlBUZ89614089;     qIXFPxlBUZ89614089 = qIXFPxlBUZ95041355;     qIXFPxlBUZ95041355 = qIXFPxlBUZ32510288;     qIXFPxlBUZ32510288 = qIXFPxlBUZ49677705;     qIXFPxlBUZ49677705 = qIXFPxlBUZ83971867;     qIXFPxlBUZ83971867 = qIXFPxlBUZ91929821;     qIXFPxlBUZ91929821 = qIXFPxlBUZ61704530;     qIXFPxlBUZ61704530 = qIXFPxlBUZ67231361;     qIXFPxlBUZ67231361 = qIXFPxlBUZ42605005;     qIXFPxlBUZ42605005 = qIXFPxlBUZ39214653;     qIXFPxlBUZ39214653 = qIXFPxlBUZ34151865;     qIXFPxlBUZ34151865 = qIXFPxlBUZ74622513;     qIXFPxlBUZ74622513 = qIXFPxlBUZ16739301;     qIXFPxlBUZ16739301 = qIXFPxlBUZ48934587;     qIXFPxlBUZ48934587 = qIXFPxlBUZ92365709;     qIXFPxlBUZ92365709 = qIXFPxlBUZ63883071;     qIXFPxlBUZ63883071 = qIXFPxlBUZ52423920;     qIXFPxlBUZ52423920 = qIXFPxlBUZ41414964;     qIXFPxlBUZ41414964 = qIXFPxlBUZ96275776;     qIXFPxlBUZ96275776 = qIXFPxlBUZ74256301;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lEhqUQuBgV30327870() {     int uCnBXvbHTG87042468 = -759339062;    int uCnBXvbHTG98896833 = 32185269;    int uCnBXvbHTG96080381 = -176761777;    int uCnBXvbHTG31101644 = -266068952;    int uCnBXvbHTG15091358 = -967468809;    int uCnBXvbHTG39626196 = -209573841;    int uCnBXvbHTG25964182 = -321939800;    int uCnBXvbHTG99098219 = -417401923;    int uCnBXvbHTG10454375 = -305487105;    int uCnBXvbHTG88662524 = 56360272;    int uCnBXvbHTG69377515 = -223118546;    int uCnBXvbHTG44324767 = -838137548;    int uCnBXvbHTG4900703 = -313221617;    int uCnBXvbHTG72564022 = -318385018;    int uCnBXvbHTG4142281 = -261553916;    int uCnBXvbHTG54475787 = -79267531;    int uCnBXvbHTG68825077 = -708836693;    int uCnBXvbHTG56374516 = -163146313;    int uCnBXvbHTG30041164 = -522077062;    int uCnBXvbHTG14885609 = -494902776;    int uCnBXvbHTG37016557 = -302123772;    int uCnBXvbHTG94309721 = -65638870;    int uCnBXvbHTG77296051 = -961086917;    int uCnBXvbHTG54984240 = 55745872;    int uCnBXvbHTG19503794 = -893463371;    int uCnBXvbHTG49679195 = -801569505;    int uCnBXvbHTG84333584 = 63340166;    int uCnBXvbHTG81164559 = -226503114;    int uCnBXvbHTG20843692 = -30471127;    int uCnBXvbHTG82905507 = -227782204;    int uCnBXvbHTG42513279 = -606253966;    int uCnBXvbHTG45170727 = -148011884;    int uCnBXvbHTG96332091 = -91122569;    int uCnBXvbHTG47337231 = -707993610;    int uCnBXvbHTG34659936 = -694699571;    int uCnBXvbHTG8028957 = -951097571;    int uCnBXvbHTG38766397 = 64248029;    int uCnBXvbHTG41877820 = -432298506;    int uCnBXvbHTG75734390 = 91414928;    int uCnBXvbHTG98597393 = -865311566;    int uCnBXvbHTG44741807 = -575515728;    int uCnBXvbHTG39737995 = -914199636;    int uCnBXvbHTG72202561 = 99045398;    int uCnBXvbHTG70410037 = -362075176;    int uCnBXvbHTG76322496 = -215682375;    int uCnBXvbHTG69870942 = -211152526;    int uCnBXvbHTG51398495 = -612405581;    int uCnBXvbHTG12743925 = -248535319;    int uCnBXvbHTG62834328 = -524901579;    int uCnBXvbHTG18666772 = -344379869;    int uCnBXvbHTG98804180 = -191799285;    int uCnBXvbHTG42281963 = -60535548;    int uCnBXvbHTG84407232 = -986732296;    int uCnBXvbHTG33281338 = -138228508;    int uCnBXvbHTG93311313 = -24603442;    int uCnBXvbHTG92732746 = -593700193;    int uCnBXvbHTG21600782 = -6727815;    int uCnBXvbHTG41096142 = -132507649;    int uCnBXvbHTG11597850 = -372605582;    int uCnBXvbHTG65412162 = -65899305;    int uCnBXvbHTG55292611 = -172914007;    int uCnBXvbHTG44799623 = 4563313;    int uCnBXvbHTG78254528 = -286930796;    int uCnBXvbHTG27548867 = 22295098;    int uCnBXvbHTG46149245 = -337385763;    int uCnBXvbHTG24206789 = 24893337;    int uCnBXvbHTG47992675 = -647014980;    int uCnBXvbHTG57563471 = -605228008;    int uCnBXvbHTG37904087 = -623685448;    int uCnBXvbHTG96113323 = -310456345;    int uCnBXvbHTG15709391 = -43515560;    int uCnBXvbHTG26947257 = -176538188;    int uCnBXvbHTG80640126 = -154561242;    int uCnBXvbHTG31443771 = -656765497;    int uCnBXvbHTG70143802 = -919387049;    int uCnBXvbHTG97278561 = -387924136;    int uCnBXvbHTG22107160 = -64684268;    int uCnBXvbHTG6886014 = -499011742;    int uCnBXvbHTG78661743 = -728571753;    int uCnBXvbHTG49632852 = -582310846;    int uCnBXvbHTG98280699 = -89163924;    int uCnBXvbHTG71589659 = -688124516;    int uCnBXvbHTG18330231 = -701601535;    int uCnBXvbHTG2176920 = -686091259;    int uCnBXvbHTG84101326 = 64017080;    int uCnBXvbHTG231316 = -445718418;    int uCnBXvbHTG60763495 = -161279589;    int uCnBXvbHTG63050754 = -952894061;    int uCnBXvbHTG54025918 = -583390169;    int uCnBXvbHTG41927189 = -999379;    int uCnBXvbHTG86428175 = -844369757;    int uCnBXvbHTG97670254 = -803244323;    int uCnBXvbHTG30279970 = 40307075;    int uCnBXvbHTG10322228 = -842685768;    int uCnBXvbHTG43304782 = -592397559;    int uCnBXvbHTG99942183 = -480079042;    int uCnBXvbHTG61483467 = -527268840;    int uCnBXvbHTG44653695 = -923249701;    int uCnBXvbHTG24260793 = 75310587;    int uCnBXvbHTG52115708 = -759339062;     uCnBXvbHTG87042468 = uCnBXvbHTG98896833;     uCnBXvbHTG98896833 = uCnBXvbHTG96080381;     uCnBXvbHTG96080381 = uCnBXvbHTG31101644;     uCnBXvbHTG31101644 = uCnBXvbHTG15091358;     uCnBXvbHTG15091358 = uCnBXvbHTG39626196;     uCnBXvbHTG39626196 = uCnBXvbHTG25964182;     uCnBXvbHTG25964182 = uCnBXvbHTG99098219;     uCnBXvbHTG99098219 = uCnBXvbHTG10454375;     uCnBXvbHTG10454375 = uCnBXvbHTG88662524;     uCnBXvbHTG88662524 = uCnBXvbHTG69377515;     uCnBXvbHTG69377515 = uCnBXvbHTG44324767;     uCnBXvbHTG44324767 = uCnBXvbHTG4900703;     uCnBXvbHTG4900703 = uCnBXvbHTG72564022;     uCnBXvbHTG72564022 = uCnBXvbHTG4142281;     uCnBXvbHTG4142281 = uCnBXvbHTG54475787;     uCnBXvbHTG54475787 = uCnBXvbHTG68825077;     uCnBXvbHTG68825077 = uCnBXvbHTG56374516;     uCnBXvbHTG56374516 = uCnBXvbHTG30041164;     uCnBXvbHTG30041164 = uCnBXvbHTG14885609;     uCnBXvbHTG14885609 = uCnBXvbHTG37016557;     uCnBXvbHTG37016557 = uCnBXvbHTG94309721;     uCnBXvbHTG94309721 = uCnBXvbHTG77296051;     uCnBXvbHTG77296051 = uCnBXvbHTG54984240;     uCnBXvbHTG54984240 = uCnBXvbHTG19503794;     uCnBXvbHTG19503794 = uCnBXvbHTG49679195;     uCnBXvbHTG49679195 = uCnBXvbHTG84333584;     uCnBXvbHTG84333584 = uCnBXvbHTG81164559;     uCnBXvbHTG81164559 = uCnBXvbHTG20843692;     uCnBXvbHTG20843692 = uCnBXvbHTG82905507;     uCnBXvbHTG82905507 = uCnBXvbHTG42513279;     uCnBXvbHTG42513279 = uCnBXvbHTG45170727;     uCnBXvbHTG45170727 = uCnBXvbHTG96332091;     uCnBXvbHTG96332091 = uCnBXvbHTG47337231;     uCnBXvbHTG47337231 = uCnBXvbHTG34659936;     uCnBXvbHTG34659936 = uCnBXvbHTG8028957;     uCnBXvbHTG8028957 = uCnBXvbHTG38766397;     uCnBXvbHTG38766397 = uCnBXvbHTG41877820;     uCnBXvbHTG41877820 = uCnBXvbHTG75734390;     uCnBXvbHTG75734390 = uCnBXvbHTG98597393;     uCnBXvbHTG98597393 = uCnBXvbHTG44741807;     uCnBXvbHTG44741807 = uCnBXvbHTG39737995;     uCnBXvbHTG39737995 = uCnBXvbHTG72202561;     uCnBXvbHTG72202561 = uCnBXvbHTG70410037;     uCnBXvbHTG70410037 = uCnBXvbHTG76322496;     uCnBXvbHTG76322496 = uCnBXvbHTG69870942;     uCnBXvbHTG69870942 = uCnBXvbHTG51398495;     uCnBXvbHTG51398495 = uCnBXvbHTG12743925;     uCnBXvbHTG12743925 = uCnBXvbHTG62834328;     uCnBXvbHTG62834328 = uCnBXvbHTG18666772;     uCnBXvbHTG18666772 = uCnBXvbHTG98804180;     uCnBXvbHTG98804180 = uCnBXvbHTG42281963;     uCnBXvbHTG42281963 = uCnBXvbHTG84407232;     uCnBXvbHTG84407232 = uCnBXvbHTG33281338;     uCnBXvbHTG33281338 = uCnBXvbHTG93311313;     uCnBXvbHTG93311313 = uCnBXvbHTG92732746;     uCnBXvbHTG92732746 = uCnBXvbHTG21600782;     uCnBXvbHTG21600782 = uCnBXvbHTG41096142;     uCnBXvbHTG41096142 = uCnBXvbHTG11597850;     uCnBXvbHTG11597850 = uCnBXvbHTG65412162;     uCnBXvbHTG65412162 = uCnBXvbHTG55292611;     uCnBXvbHTG55292611 = uCnBXvbHTG44799623;     uCnBXvbHTG44799623 = uCnBXvbHTG78254528;     uCnBXvbHTG78254528 = uCnBXvbHTG27548867;     uCnBXvbHTG27548867 = uCnBXvbHTG46149245;     uCnBXvbHTG46149245 = uCnBXvbHTG24206789;     uCnBXvbHTG24206789 = uCnBXvbHTG47992675;     uCnBXvbHTG47992675 = uCnBXvbHTG57563471;     uCnBXvbHTG57563471 = uCnBXvbHTG37904087;     uCnBXvbHTG37904087 = uCnBXvbHTG96113323;     uCnBXvbHTG96113323 = uCnBXvbHTG15709391;     uCnBXvbHTG15709391 = uCnBXvbHTG26947257;     uCnBXvbHTG26947257 = uCnBXvbHTG80640126;     uCnBXvbHTG80640126 = uCnBXvbHTG31443771;     uCnBXvbHTG31443771 = uCnBXvbHTG70143802;     uCnBXvbHTG70143802 = uCnBXvbHTG97278561;     uCnBXvbHTG97278561 = uCnBXvbHTG22107160;     uCnBXvbHTG22107160 = uCnBXvbHTG6886014;     uCnBXvbHTG6886014 = uCnBXvbHTG78661743;     uCnBXvbHTG78661743 = uCnBXvbHTG49632852;     uCnBXvbHTG49632852 = uCnBXvbHTG98280699;     uCnBXvbHTG98280699 = uCnBXvbHTG71589659;     uCnBXvbHTG71589659 = uCnBXvbHTG18330231;     uCnBXvbHTG18330231 = uCnBXvbHTG2176920;     uCnBXvbHTG2176920 = uCnBXvbHTG84101326;     uCnBXvbHTG84101326 = uCnBXvbHTG231316;     uCnBXvbHTG231316 = uCnBXvbHTG60763495;     uCnBXvbHTG60763495 = uCnBXvbHTG63050754;     uCnBXvbHTG63050754 = uCnBXvbHTG54025918;     uCnBXvbHTG54025918 = uCnBXvbHTG41927189;     uCnBXvbHTG41927189 = uCnBXvbHTG86428175;     uCnBXvbHTG86428175 = uCnBXvbHTG97670254;     uCnBXvbHTG97670254 = uCnBXvbHTG30279970;     uCnBXvbHTG30279970 = uCnBXvbHTG10322228;     uCnBXvbHTG10322228 = uCnBXvbHTG43304782;     uCnBXvbHTG43304782 = uCnBXvbHTG99942183;     uCnBXvbHTG99942183 = uCnBXvbHTG61483467;     uCnBXvbHTG61483467 = uCnBXvbHTG44653695;     uCnBXvbHTG44653695 = uCnBXvbHTG24260793;     uCnBXvbHTG24260793 = uCnBXvbHTG52115708;     uCnBXvbHTG52115708 = uCnBXvbHTG87042468;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IZUOpOopyy48541211() {     int ysvmqvRMaE70486773 = -540004743;    int ysvmqvRMaE15768922 = 32803081;    int ysvmqvRMaE92488888 = 29712996;    int ysvmqvRMaE93893895 = -459868788;    int ysvmqvRMaE21212682 = -774336300;    int ysvmqvRMaE38130597 = 11354506;    int ysvmqvRMaE97003499 = -61851113;    int ysvmqvRMaE13445663 = -404559952;    int ysvmqvRMaE68984075 = -646946456;    int ysvmqvRMaE21208176 = -412015841;    int ysvmqvRMaE18793453 = -413234419;    int ysvmqvRMaE7530166 = -825755794;    int ysvmqvRMaE23018478 = -104882858;    int ysvmqvRMaE52190914 = -106675375;    int ysvmqvRMaE64996744 = -355576898;    int ysvmqvRMaE40864949 = -967346857;    int ysvmqvRMaE62576962 = -462181720;    int ysvmqvRMaE37225963 = -410451189;    int ysvmqvRMaE92481305 = -520751933;    int ysvmqvRMaE4253051 = 75511231;    int ysvmqvRMaE92656943 = -991652802;    int ysvmqvRMaE6457040 = -56790640;    int ysvmqvRMaE81121758 = 99023886;    int ysvmqvRMaE89690798 = -16079524;    int ysvmqvRMaE61079752 = -353116130;    int ysvmqvRMaE45021344 = -682079220;    int ysvmqvRMaE33788325 = -664744177;    int ysvmqvRMaE20896185 = -296457997;    int ysvmqvRMaE56924786 = -992976270;    int ysvmqvRMaE98738848 = -740632056;    int ysvmqvRMaE60608592 = -244011167;    int ysvmqvRMaE8277154 = -63206964;    int ysvmqvRMaE39870978 = -811994747;    int ysvmqvRMaE89495741 = -299595780;    int ysvmqvRMaE13386666 = -892364306;    int ysvmqvRMaE20418142 = -739121477;    int ysvmqvRMaE7380576 = -914601151;    int ysvmqvRMaE82456651 = -140769026;    int ysvmqvRMaE24729468 = -93482870;    int ysvmqvRMaE48092528 = -617905088;    int ysvmqvRMaE78390731 = -147064701;    int ysvmqvRMaE90225781 = -388107957;    int ysvmqvRMaE95769631 = -452011845;    int ysvmqvRMaE62314978 = -118257947;    int ysvmqvRMaE68266993 = 56160671;    int ysvmqvRMaE9476938 = -230763449;    int ysvmqvRMaE27413926 = -744709811;    int ysvmqvRMaE32414521 = -247257038;    int ysvmqvRMaE93681440 = -10503662;    int ysvmqvRMaE53999256 = -551400324;    int ysvmqvRMaE72061629 = -412700707;    int ysvmqvRMaE97782757 = -118794860;    int ysvmqvRMaE98519690 = -98479612;    int ysvmqvRMaE57958097 = -956641896;    int ysvmqvRMaE55884268 = -373089565;    int ysvmqvRMaE64029734 = -383214103;    int ysvmqvRMaE34647163 = 33779195;    int ysvmqvRMaE2798091 = -954207481;    int ysvmqvRMaE32814144 = -6752659;    int ysvmqvRMaE76191337 = 7742920;    int ysvmqvRMaE4342272 = -323901318;    int ysvmqvRMaE76107315 = -765393116;    int ysvmqvRMaE56520877 = -411583683;    int ysvmqvRMaE70245226 = -906314401;    int ysvmqvRMaE60599583 = -68004674;    int ysvmqvRMaE10516299 = -250027456;    int ysvmqvRMaE67659187 = 86238952;    int ysvmqvRMaE33522737 = -805287079;    int ysvmqvRMaE38804248 = -214311069;    int ysvmqvRMaE44578603 = -616455422;    int ysvmqvRMaE33484374 = 47254294;    int ysvmqvRMaE80120310 = -221412695;    int ysvmqvRMaE12496495 = -216968320;    int ysvmqvRMaE44388777 = -902846846;    int ysvmqvRMaE25862320 = -777424068;    int ysvmqvRMaE2431163 = -503544846;    int ysvmqvRMaE10687409 = -604778796;    int ysvmqvRMaE18806781 = -782718168;    int ysvmqvRMaE21423805 = 27759804;    int ysvmqvRMaE51602814 = -22352681;    int ysvmqvRMaE17607419 = -937369410;    int ysvmqvRMaE1373805 = -317487140;    int ysvmqvRMaE27214745 = -185954336;    int ysvmqvRMaE2925531 = -341575946;    int ysvmqvRMaE26677220 = -227931350;    int ysvmqvRMaE62825835 = -25216308;    int ysvmqvRMaE9757463 = -964727352;    int ysvmqvRMaE81912881 = -855352851;    int ysvmqvRMaE33611474 = -926506216;    int ysvmqvRMaE49356932 = -409150203;    int ysvmqvRMaE85770978 = -672900673;    int ysvmqvRMaE4582485 = -960393671;    int ysvmqvRMaE49642508 = -34016368;    int ysvmqvRMaE48538131 = -1225790;    int ysvmqvRMaE43750256 = -194003770;    int ysvmqvRMaE2283416 = -381671585;    int ysvmqvRMaE33704905 = -976524274;    int ysvmqvRMaE25524405 = -545697445;    int ysvmqvRMaE1715395 = 49746727;    int ysvmqvRMaE57750694 = -540004743;     ysvmqvRMaE70486773 = ysvmqvRMaE15768922;     ysvmqvRMaE15768922 = ysvmqvRMaE92488888;     ysvmqvRMaE92488888 = ysvmqvRMaE93893895;     ysvmqvRMaE93893895 = ysvmqvRMaE21212682;     ysvmqvRMaE21212682 = ysvmqvRMaE38130597;     ysvmqvRMaE38130597 = ysvmqvRMaE97003499;     ysvmqvRMaE97003499 = ysvmqvRMaE13445663;     ysvmqvRMaE13445663 = ysvmqvRMaE68984075;     ysvmqvRMaE68984075 = ysvmqvRMaE21208176;     ysvmqvRMaE21208176 = ysvmqvRMaE18793453;     ysvmqvRMaE18793453 = ysvmqvRMaE7530166;     ysvmqvRMaE7530166 = ysvmqvRMaE23018478;     ysvmqvRMaE23018478 = ysvmqvRMaE52190914;     ysvmqvRMaE52190914 = ysvmqvRMaE64996744;     ysvmqvRMaE64996744 = ysvmqvRMaE40864949;     ysvmqvRMaE40864949 = ysvmqvRMaE62576962;     ysvmqvRMaE62576962 = ysvmqvRMaE37225963;     ysvmqvRMaE37225963 = ysvmqvRMaE92481305;     ysvmqvRMaE92481305 = ysvmqvRMaE4253051;     ysvmqvRMaE4253051 = ysvmqvRMaE92656943;     ysvmqvRMaE92656943 = ysvmqvRMaE6457040;     ysvmqvRMaE6457040 = ysvmqvRMaE81121758;     ysvmqvRMaE81121758 = ysvmqvRMaE89690798;     ysvmqvRMaE89690798 = ysvmqvRMaE61079752;     ysvmqvRMaE61079752 = ysvmqvRMaE45021344;     ysvmqvRMaE45021344 = ysvmqvRMaE33788325;     ysvmqvRMaE33788325 = ysvmqvRMaE20896185;     ysvmqvRMaE20896185 = ysvmqvRMaE56924786;     ysvmqvRMaE56924786 = ysvmqvRMaE98738848;     ysvmqvRMaE98738848 = ysvmqvRMaE60608592;     ysvmqvRMaE60608592 = ysvmqvRMaE8277154;     ysvmqvRMaE8277154 = ysvmqvRMaE39870978;     ysvmqvRMaE39870978 = ysvmqvRMaE89495741;     ysvmqvRMaE89495741 = ysvmqvRMaE13386666;     ysvmqvRMaE13386666 = ysvmqvRMaE20418142;     ysvmqvRMaE20418142 = ysvmqvRMaE7380576;     ysvmqvRMaE7380576 = ysvmqvRMaE82456651;     ysvmqvRMaE82456651 = ysvmqvRMaE24729468;     ysvmqvRMaE24729468 = ysvmqvRMaE48092528;     ysvmqvRMaE48092528 = ysvmqvRMaE78390731;     ysvmqvRMaE78390731 = ysvmqvRMaE90225781;     ysvmqvRMaE90225781 = ysvmqvRMaE95769631;     ysvmqvRMaE95769631 = ysvmqvRMaE62314978;     ysvmqvRMaE62314978 = ysvmqvRMaE68266993;     ysvmqvRMaE68266993 = ysvmqvRMaE9476938;     ysvmqvRMaE9476938 = ysvmqvRMaE27413926;     ysvmqvRMaE27413926 = ysvmqvRMaE32414521;     ysvmqvRMaE32414521 = ysvmqvRMaE93681440;     ysvmqvRMaE93681440 = ysvmqvRMaE53999256;     ysvmqvRMaE53999256 = ysvmqvRMaE72061629;     ysvmqvRMaE72061629 = ysvmqvRMaE97782757;     ysvmqvRMaE97782757 = ysvmqvRMaE98519690;     ysvmqvRMaE98519690 = ysvmqvRMaE57958097;     ysvmqvRMaE57958097 = ysvmqvRMaE55884268;     ysvmqvRMaE55884268 = ysvmqvRMaE64029734;     ysvmqvRMaE64029734 = ysvmqvRMaE34647163;     ysvmqvRMaE34647163 = ysvmqvRMaE2798091;     ysvmqvRMaE2798091 = ysvmqvRMaE32814144;     ysvmqvRMaE32814144 = ysvmqvRMaE76191337;     ysvmqvRMaE76191337 = ysvmqvRMaE4342272;     ysvmqvRMaE4342272 = ysvmqvRMaE76107315;     ysvmqvRMaE76107315 = ysvmqvRMaE56520877;     ysvmqvRMaE56520877 = ysvmqvRMaE70245226;     ysvmqvRMaE70245226 = ysvmqvRMaE60599583;     ysvmqvRMaE60599583 = ysvmqvRMaE10516299;     ysvmqvRMaE10516299 = ysvmqvRMaE67659187;     ysvmqvRMaE67659187 = ysvmqvRMaE33522737;     ysvmqvRMaE33522737 = ysvmqvRMaE38804248;     ysvmqvRMaE38804248 = ysvmqvRMaE44578603;     ysvmqvRMaE44578603 = ysvmqvRMaE33484374;     ysvmqvRMaE33484374 = ysvmqvRMaE80120310;     ysvmqvRMaE80120310 = ysvmqvRMaE12496495;     ysvmqvRMaE12496495 = ysvmqvRMaE44388777;     ysvmqvRMaE44388777 = ysvmqvRMaE25862320;     ysvmqvRMaE25862320 = ysvmqvRMaE2431163;     ysvmqvRMaE2431163 = ysvmqvRMaE10687409;     ysvmqvRMaE10687409 = ysvmqvRMaE18806781;     ysvmqvRMaE18806781 = ysvmqvRMaE21423805;     ysvmqvRMaE21423805 = ysvmqvRMaE51602814;     ysvmqvRMaE51602814 = ysvmqvRMaE17607419;     ysvmqvRMaE17607419 = ysvmqvRMaE1373805;     ysvmqvRMaE1373805 = ysvmqvRMaE27214745;     ysvmqvRMaE27214745 = ysvmqvRMaE2925531;     ysvmqvRMaE2925531 = ysvmqvRMaE26677220;     ysvmqvRMaE26677220 = ysvmqvRMaE62825835;     ysvmqvRMaE62825835 = ysvmqvRMaE9757463;     ysvmqvRMaE9757463 = ysvmqvRMaE81912881;     ysvmqvRMaE81912881 = ysvmqvRMaE33611474;     ysvmqvRMaE33611474 = ysvmqvRMaE49356932;     ysvmqvRMaE49356932 = ysvmqvRMaE85770978;     ysvmqvRMaE85770978 = ysvmqvRMaE4582485;     ysvmqvRMaE4582485 = ysvmqvRMaE49642508;     ysvmqvRMaE49642508 = ysvmqvRMaE48538131;     ysvmqvRMaE48538131 = ysvmqvRMaE43750256;     ysvmqvRMaE43750256 = ysvmqvRMaE2283416;     ysvmqvRMaE2283416 = ysvmqvRMaE33704905;     ysvmqvRMaE33704905 = ysvmqvRMaE25524405;     ysvmqvRMaE25524405 = ysvmqvRMaE1715395;     ysvmqvRMaE1715395 = ysvmqvRMaE57750694;     ysvmqvRMaE57750694 = ysvmqvRMaE70486773;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hiMLpkpxxs18997085() {     int HtWcWKnIEI83272940 = -675045434;    int HtWcWKnIEI2539811 = -137530932;    int HtWcWKnIEI10241689 = -378646386;    int HtWcWKnIEI1834721 = -284071065;    int HtWcWKnIEI14723786 = -166152765;    int HtWcWKnIEI81440667 = -865568808;    int HtWcWKnIEI18211258 = -607523184;    int HtWcWKnIEI40164775 = -849078209;    int HtWcWKnIEI64388885 = -608704549;    int HtWcWKnIEI27426881 = -216103473;    int HtWcWKnIEI46280835 = -527920434;    int HtWcWKnIEI39480671 = -233088610;    int HtWcWKnIEI93161780 = -8665516;    int HtWcWKnIEI1814993 = -439090573;    int HtWcWKnIEI88303296 = -262562729;    int HtWcWKnIEI69290018 = -418238246;    int HtWcWKnIEI18328780 = -125956187;    int HtWcWKnIEI20784448 = 34338508;    int HtWcWKnIEI93641936 = -430221128;    int HtWcWKnIEI6907548 = -523732908;    int HtWcWKnIEI72831000 = -382969067;    int HtWcWKnIEI35504859 = -226648988;    int HtWcWKnIEI19882055 = -931980009;    int HtWcWKnIEI34197928 = -376424537;    int HtWcWKnIEI66974011 = -563558054;    int HtWcWKnIEI26339990 = -372510420;    int HtWcWKnIEI45203861 = -198450457;    int HtWcWKnIEI88249109 = -470256102;    int HtWcWKnIEI73541839 = -107193510;    int HtWcWKnIEI51489964 = -370518210;    int HtWcWKnIEI5044589 = -520427917;    int HtWcWKnIEI11595978 = -41926642;    int HtWcWKnIEI85916927 = -687849531;    int HtWcWKnIEI66572114 = -204349998;    int HtWcWKnIEI96447198 = -402396186;    int HtWcWKnIEI15642256 = -527684581;    int HtWcWKnIEI44144636 = -704510502;    int HtWcWKnIEI40160674 = -60389118;    int HtWcWKnIEI30504854 = 64666553;    int HtWcWKnIEI74357258 = -462321933;    int HtWcWKnIEI39822039 = -522538694;    int HtWcWKnIEI97928237 = -286419291;    int HtWcWKnIEI30653101 = -475445290;    int HtWcWKnIEI6943516 = -459430189;    int HtWcWKnIEI48275483 = -936609289;    int HtWcWKnIEI83105067 = -304081889;    int HtWcWKnIEI4995946 = -142224863;    int HtWcWKnIEI61854487 = -402609817;    int HtWcWKnIEI37745490 = -174105636;    int HtWcWKnIEI949677 = -62436787;    int HtWcWKnIEI90389122 = -665585015;    int HtWcWKnIEI25959305 = -589980198;    int HtWcWKnIEI33004841 = -274482895;    int HtWcWKnIEI2657822 = -546093638;    int HtWcWKnIEI46166084 = -367534051;    int HtWcWKnIEI47768082 = -348396447;    int HtWcWKnIEI82657756 = -205550924;    int HtWcWKnIEI76043761 = 97778150;    int HtWcWKnIEI34860710 = -720513011;    int HtWcWKnIEI88383796 = -793642346;    int HtWcWKnIEI36236806 = -567118351;    int HtWcWKnIEI29962149 = -37267082;    int HtWcWKnIEI66622936 = -641884700;    int HtWcWKnIEI12898921 = -138186339;    int HtWcWKnIEI22382293 = -695675557;    int HtWcWKnIEI34684858 = -385993793;    int HtWcWKnIEI53563743 = -545239080;    int HtWcWKnIEI26589667 = -804315518;    int HtWcWKnIEI5367795 = 63305613;    int HtWcWKnIEI72661041 = -734878148;    int HtWcWKnIEI25145382 = -713727745;    int HtWcWKnIEI78168105 = 34432930;    int HtWcWKnIEI90279594 = 69671955;    int HtWcWKnIEI19284678 = -967899195;    int HtWcWKnIEI67085508 = 98805786;    int HtWcWKnIEI74902763 = 3450224;    int HtWcWKnIEI4851758 = -751203699;    int HtWcWKnIEI12938539 = -372549820;    int HtWcWKnIEI85922444 = -439815248;    int HtWcWKnIEI83868943 = -159476166;    int HtWcWKnIEI21344045 = -130285557;    int HtWcWKnIEI83349374 = -795840640;    int HtWcWKnIEI50503620 = -196150466;    int HtWcWKnIEI72592162 = 55243277;    int HtWcWKnIEI61100842 = -704933196;    int HtWcWKnIEI79085284 = -930447720;    int HtWcWKnIEI78591136 = -767443748;    int HtWcWKnIEI83259105 = -41755894;    int HtWcWKnIEI20406031 = -836815948;    int HtWcWKnIEI48679116 = 46000260;    int HtWcWKnIEI32984500 = -222133658;    int HtWcWKnIEI68100875 = -702288652;    int HtWcWKnIEI5299965 = -339876107;    int HtWcWKnIEI42121058 = -141691101;    int HtWcWKnIEI38120452 = -895203583;    int HtWcWKnIEI9859891 = -385271613;    int HtWcWKnIEI31305301 = -644534592;    int HtWcWKnIEI17754180 = -237258952;    int HtWcWKnIEI84561223 = -763754633;    int HtWcWKnIEI13590626 = -675045434;     HtWcWKnIEI83272940 = HtWcWKnIEI2539811;     HtWcWKnIEI2539811 = HtWcWKnIEI10241689;     HtWcWKnIEI10241689 = HtWcWKnIEI1834721;     HtWcWKnIEI1834721 = HtWcWKnIEI14723786;     HtWcWKnIEI14723786 = HtWcWKnIEI81440667;     HtWcWKnIEI81440667 = HtWcWKnIEI18211258;     HtWcWKnIEI18211258 = HtWcWKnIEI40164775;     HtWcWKnIEI40164775 = HtWcWKnIEI64388885;     HtWcWKnIEI64388885 = HtWcWKnIEI27426881;     HtWcWKnIEI27426881 = HtWcWKnIEI46280835;     HtWcWKnIEI46280835 = HtWcWKnIEI39480671;     HtWcWKnIEI39480671 = HtWcWKnIEI93161780;     HtWcWKnIEI93161780 = HtWcWKnIEI1814993;     HtWcWKnIEI1814993 = HtWcWKnIEI88303296;     HtWcWKnIEI88303296 = HtWcWKnIEI69290018;     HtWcWKnIEI69290018 = HtWcWKnIEI18328780;     HtWcWKnIEI18328780 = HtWcWKnIEI20784448;     HtWcWKnIEI20784448 = HtWcWKnIEI93641936;     HtWcWKnIEI93641936 = HtWcWKnIEI6907548;     HtWcWKnIEI6907548 = HtWcWKnIEI72831000;     HtWcWKnIEI72831000 = HtWcWKnIEI35504859;     HtWcWKnIEI35504859 = HtWcWKnIEI19882055;     HtWcWKnIEI19882055 = HtWcWKnIEI34197928;     HtWcWKnIEI34197928 = HtWcWKnIEI66974011;     HtWcWKnIEI66974011 = HtWcWKnIEI26339990;     HtWcWKnIEI26339990 = HtWcWKnIEI45203861;     HtWcWKnIEI45203861 = HtWcWKnIEI88249109;     HtWcWKnIEI88249109 = HtWcWKnIEI73541839;     HtWcWKnIEI73541839 = HtWcWKnIEI51489964;     HtWcWKnIEI51489964 = HtWcWKnIEI5044589;     HtWcWKnIEI5044589 = HtWcWKnIEI11595978;     HtWcWKnIEI11595978 = HtWcWKnIEI85916927;     HtWcWKnIEI85916927 = HtWcWKnIEI66572114;     HtWcWKnIEI66572114 = HtWcWKnIEI96447198;     HtWcWKnIEI96447198 = HtWcWKnIEI15642256;     HtWcWKnIEI15642256 = HtWcWKnIEI44144636;     HtWcWKnIEI44144636 = HtWcWKnIEI40160674;     HtWcWKnIEI40160674 = HtWcWKnIEI30504854;     HtWcWKnIEI30504854 = HtWcWKnIEI74357258;     HtWcWKnIEI74357258 = HtWcWKnIEI39822039;     HtWcWKnIEI39822039 = HtWcWKnIEI97928237;     HtWcWKnIEI97928237 = HtWcWKnIEI30653101;     HtWcWKnIEI30653101 = HtWcWKnIEI6943516;     HtWcWKnIEI6943516 = HtWcWKnIEI48275483;     HtWcWKnIEI48275483 = HtWcWKnIEI83105067;     HtWcWKnIEI83105067 = HtWcWKnIEI4995946;     HtWcWKnIEI4995946 = HtWcWKnIEI61854487;     HtWcWKnIEI61854487 = HtWcWKnIEI37745490;     HtWcWKnIEI37745490 = HtWcWKnIEI949677;     HtWcWKnIEI949677 = HtWcWKnIEI90389122;     HtWcWKnIEI90389122 = HtWcWKnIEI25959305;     HtWcWKnIEI25959305 = HtWcWKnIEI33004841;     HtWcWKnIEI33004841 = HtWcWKnIEI2657822;     HtWcWKnIEI2657822 = HtWcWKnIEI46166084;     HtWcWKnIEI46166084 = HtWcWKnIEI47768082;     HtWcWKnIEI47768082 = HtWcWKnIEI82657756;     HtWcWKnIEI82657756 = HtWcWKnIEI76043761;     HtWcWKnIEI76043761 = HtWcWKnIEI34860710;     HtWcWKnIEI34860710 = HtWcWKnIEI88383796;     HtWcWKnIEI88383796 = HtWcWKnIEI36236806;     HtWcWKnIEI36236806 = HtWcWKnIEI29962149;     HtWcWKnIEI29962149 = HtWcWKnIEI66622936;     HtWcWKnIEI66622936 = HtWcWKnIEI12898921;     HtWcWKnIEI12898921 = HtWcWKnIEI22382293;     HtWcWKnIEI22382293 = HtWcWKnIEI34684858;     HtWcWKnIEI34684858 = HtWcWKnIEI53563743;     HtWcWKnIEI53563743 = HtWcWKnIEI26589667;     HtWcWKnIEI26589667 = HtWcWKnIEI5367795;     HtWcWKnIEI5367795 = HtWcWKnIEI72661041;     HtWcWKnIEI72661041 = HtWcWKnIEI25145382;     HtWcWKnIEI25145382 = HtWcWKnIEI78168105;     HtWcWKnIEI78168105 = HtWcWKnIEI90279594;     HtWcWKnIEI90279594 = HtWcWKnIEI19284678;     HtWcWKnIEI19284678 = HtWcWKnIEI67085508;     HtWcWKnIEI67085508 = HtWcWKnIEI74902763;     HtWcWKnIEI74902763 = HtWcWKnIEI4851758;     HtWcWKnIEI4851758 = HtWcWKnIEI12938539;     HtWcWKnIEI12938539 = HtWcWKnIEI85922444;     HtWcWKnIEI85922444 = HtWcWKnIEI83868943;     HtWcWKnIEI83868943 = HtWcWKnIEI21344045;     HtWcWKnIEI21344045 = HtWcWKnIEI83349374;     HtWcWKnIEI83349374 = HtWcWKnIEI50503620;     HtWcWKnIEI50503620 = HtWcWKnIEI72592162;     HtWcWKnIEI72592162 = HtWcWKnIEI61100842;     HtWcWKnIEI61100842 = HtWcWKnIEI79085284;     HtWcWKnIEI79085284 = HtWcWKnIEI78591136;     HtWcWKnIEI78591136 = HtWcWKnIEI83259105;     HtWcWKnIEI83259105 = HtWcWKnIEI20406031;     HtWcWKnIEI20406031 = HtWcWKnIEI48679116;     HtWcWKnIEI48679116 = HtWcWKnIEI32984500;     HtWcWKnIEI32984500 = HtWcWKnIEI68100875;     HtWcWKnIEI68100875 = HtWcWKnIEI5299965;     HtWcWKnIEI5299965 = HtWcWKnIEI42121058;     HtWcWKnIEI42121058 = HtWcWKnIEI38120452;     HtWcWKnIEI38120452 = HtWcWKnIEI9859891;     HtWcWKnIEI9859891 = HtWcWKnIEI31305301;     HtWcWKnIEI31305301 = HtWcWKnIEI17754180;     HtWcWKnIEI17754180 = HtWcWKnIEI84561223;     HtWcWKnIEI84561223 = HtWcWKnIEI13590626;     HtWcWKnIEI13590626 = HtWcWKnIEI83272940;}
// Junk Finished
