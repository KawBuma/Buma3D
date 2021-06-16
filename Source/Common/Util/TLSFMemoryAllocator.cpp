#include "./TLSFMemoryAllocator.h"

#include <cassert>
#include <string>


/*
** Architecture-specific bit manipulation routines.
**
** TLSF achieves O(1) cost for malloc and free operations by limiting
** the search for a free block to a free list of guaranteed size
** adequate to fulfill the request, combined with efficient free list
** queries using bitmasks and architecture-specific bit-manipulation
** routines.
**
** Most modern processors provide instructions to count leading zeroes
** in a word, find the lowest and highest set bit, etc. These
** specific implementations will be used when available, falling back
** to a reasonably efficient generic implementation.
**
** NOTE: TLSF spec relies on ffs/fls returning value 0..31.
** ffs/fls return 1-32 by default, returning 0 for error.
*/
/*
アーキテクチャ固有のビット操作ルーチン。

TLSFは、
フリーブロックの検索を、要求を満たすのに十分な保証サイズのフリーリストに制限し、
ビットマスクとアーキテクチャ固有のビット操作ルーチンを使用した効率的なフリーリストクエリと組み合わせることにより、
mallocおよびfree操作のO（1）コストを実現します。

最近のほとんどのプロセッサは、ワード内の先行ゼロをカウントする、最下位ビットと最上位セットビットを見つけるなどの命令を提供しています
これらの特定の実装は、利用可能な場合に使用され、かなり効率的な汎用実装にフォールバックします。

注意:
TLSF仕様は、値0..31を返すffs/flsに依存しています。
ffs/flsはデフォルトで1~32を返し、エラーの場合は0を返します。
*/

/*
** Detect whether or not we are building for a 32- or 64-bit (LP/LLP)
** architecture. There is no reliable portable method at compile-time.
*/
/*
/* 32ビットまたは64ビット（LP / LLP）アーキテクチャ用に構築しているかどうかを検出します。
コンパイル時に信頼できる移植可能な方法はありません。*/
#if defined (__alpha__) || defined (__ia64__) || defined (__x86_64__) \
    || defined (_WIN64) || defined (__LP64__) || defined (__LLP64__)
#define TLSF_64BIT
#endif

#define tlsf_decl inline

/*
** gcc 3.4 and above have builtin support, specialized for architecture.
** Some compilers masquerade as gcc; patchlevel test filters them out.
*/
/*gcc 3.4以降には、アーキテクチャに特化したサポートが組み込まれています。
一部のコンパイラはgccを偽装しています。 パッチレベルテストはそれらを除外します。*/
#if defined (__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)) \
    && defined (__GNUC_PATCHLEVEL__)

#if defined (__SNC__)
/* SNC for Playstation 3. */

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __builtin_clz(reverse);
    return bit - 1;
}

#else

tlsf_decl int tlsf_ffs(unsigned int word)
{
    return __builtin_ffs(word) - 1;
}

#endif

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __builtin_clz(word) : 0;
    return bit - 1;
}

#elif defined (_MSC_VER) && (_MSC_VER >= 1400) && (defined (_M_IX86) || defined (_M_X64))
/* Microsoft Visual C++ support on x86/X64 architectures. */
/* Microsoft Visual C++ x86/X64アーキテクチャのサポート。 */
#include <intrin.h>

#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

tlsf_decl int tlsf_fls(unsigned int word)
{
    unsigned long index;
    return _BitScanReverse(&index, word) ? index : -1;
}

tlsf_decl int tlsf_ffs(unsigned int word)
{
    unsigned long index;
    return _BitScanForward(&index, word) ? index : -1;
}

#elif defined (_MSC_VER) && defined (_M_PPC)
/* Microsoft Visual C++ support on PowerPC architectures. */

#include <ppcintrinsics.h>

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = 32 - _CountLeadingZeros(word);
    return bit - 1;
}

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - _CountLeadingZeros(reverse);
    return bit - 1;
}

#elif defined (__ARMCC_VERSION)
/* RealView Compilation Tools for ARM */

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __clz(reverse);
    return bit - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __clz(word) : 0;
    return bit - 1;
}

#elif defined (__ghs__)
/* Green Hills support for PowerPC */

#include <ppc_ghs.h>

tlsf_decl int tlsf_ffs(unsigned int word)
{
    const unsigned int reverse = word & (~word + 1);
    const int bit = 32 - __CLZ32(reverse);
    return bit - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    const int bit = word ? 32 - __CLZ32(word) : 0;
    return bit - 1;
}

#else
/* Fall back to generic implementation. */

tlsf_decl int tlsf_fls_generic(unsigned int word)
{
    int bit = 32;

    if (!word) bit -= 1;
    if (!(word & 0xffff0000)) { word <<= 16; bit -= 16; }
    if (!(word & 0xff000000)) { word <<= 8; bit -= 8; }
    if (!(word & 0xf0000000)) { word <<= 4; bit -= 4; }
    if (!(word & 0xc0000000)) { word <<= 2; bit -= 2; }
    if (!(word & 0x80000000)) { word <<= 1; bit -= 1; }

    return bit;
}

/* Implement ffs in terms of fls. */
tlsf_decl int tlsf_ffs(unsigned int word)
{
    return tlsf_fls_generic(word & (~word + 1)) - 1;
}

tlsf_decl int tlsf_fls(unsigned int word)
{
    return tlsf_fls_generic(word) - 1;
}

#endif

/* Possibly 64-bit version of tlsf_fls. */
#if defined (TLSF_64BIT)
tlsf_decl int tlsf_fls_sizet(size_t size)
{
    int high = (int)(size >> 32);
    int bits = 0;
    if (high)
    {
        bits = 32 + tlsf_fls(high);
    }
    else
    {
        bits = tlsf_fls((int)size & 0xffffffff);
    }
    return bits;
}
#else
#define tlsf_fls_sizet tlsf_fls
#endif

#undef tlsf_decl

namespace tlsf
{

/*
** Constants.
*/

/* Public constants: may be modified. */
enum tlsf_public
{
    /* log2 of number of linear subdivisions of block sizes. Larger
    ** values require more memory in the control structure. Values of
    ** 4 or 5 are typical.
    */
    /*  ブロックサイズの線形サブディビジョンの数のlog2。
    値が大きいほど、制御構造でより多くのメモリが必要になります。
    4または5の値が一般的です。*/
    SL_INDEX_COUNT_LOG2 = 5,
};

/* Private constants: do not modify. */
enum tlsf_private
{
#if defined (TLSF_64BIT)
    /* All allocation sizes and addresses are aligned to 8 bytes. */
    ALIGN_SIZE_LOG2 = 3,
#else
    /* All allocation sizes and addresses are aligned to 4 bytes. */
    ALIGN_SIZE_LOG2 = 2,
#endif
    ALIGN_SIZE = (1 << ALIGN_SIZE_LOG2),

    /*
    ** We support allocations of sizes up to (1 << FL_INDEX_MAX) bits.
    ** However, because we linearly subdivide the second-level lists, and
    ** our minimum size granularity is 4 bytes, it doesn't make sense to
    ** create first-level lists for sizes smaller than SL_INDEX_COUNT * 4,
    ** or (1 << (SL_INDEX_COUNT_LOG2 + 2)) bytes, as there we will be
    ** trying to split size ranges into more slots than we have available.
    ** Instead, we calculate the minimum threshold size, and place all
    ** blocks below that size into the 0th first-level list.
    */
    /*
    （1 << FL_INDEX_MAX）ビットまでのサイズの割り当てをサポートしています。
    
    ただし、第2レベルのリストを線形分割し、最小サイズの粒度が4バイトであるため、
    SL_INDEX_COUNT * 4または1 << (SL_INDEX_COUNT_LOG2 + 2)バイトより小さいサイズの第1レベルのリストを作成しても意味がありません。
    サイズの範囲を、使用可能な数よりも多くのスロットに分割しようとしているためです。
    
    代わりに、最小しきい値サイズを計算し、そのサイズ以下のすべてのブロックを0番目の第1レベルリストに配置します。
    */

#if defined (TLSF_64BIT)
    /*
    ** TODO: We can increase this to support larger sizes, at the expense
    ** of more overhead in the TLSF structure.
    */
    /*TODO：TLSF構造のオーバーヘッドが増える代わりに、これを増やしてより大きなサイズをサポートできます。*/
    FL_INDEX_MAX = 32,
#else
    FL_INDEX_MAX = 30,
#endif
    SL_INDEX_COUNT = (1 << SL_INDEX_COUNT_LOG2),
    FL_INDEX_SHIFT = (SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2),
    FL_INDEX_COUNT = (FL_INDEX_MAX - FL_INDEX_SHIFT + 1),

    SMALL_BLOCK_SIZE = (1 << FL_INDEX_SHIFT),
};

/*
** Cast and min/max macros.
*/

#define tlsf_cast(t, exp)   ((t) (exp))
#define tlsf_min(a, b)      ((a) < (b) ? (a) : (b))
#define tlsf_max(a, b)      ((a) > (b) ? (a) : (b))

/*
** Set assert macro, if it has not been provided by the user.
*/
#if !defined (tlsf_assert)
#define tlsf_assert assert
#endif

/*
** Static assertion mechanism.
*/

#define _tlsf_glue2(x, y) x ## y
#define _tlsf_glue(x, y) _tlsf_glue2(x, y)
#define tlsf_static_assert(exp) \
    typedef char _tlsf_glue(static_assert, __LINE__) [(exp) ? 1 : -1]

/* This code has been tested on 32- and 64-bit (LP/LLP) architectures. */
tlsf_static_assert(sizeof(int)* CHAR_BIT == 32);
tlsf_static_assert(sizeof(size_t)* CHAR_BIT >= 32);
tlsf_static_assert(sizeof(size_t)* CHAR_BIT <= 64);

/* SL_INDEX_COUNT must be <= number of bits in sl_bitmap's storage type. */
tlsf_static_assert(sizeof(unsigned int)* CHAR_BIT >= SL_INDEX_COUNT);

/* Ensure we've properly tuned our sizes. */
tlsf_static_assert(ALIGN_SIZE == SMALL_BLOCK_SIZE / SL_INDEX_COUNT);

/*
** Data structures and associated constants.
*/

/*
** Block header structure.
**
** There are several implementation subtleties involved:
** - The prev_phys_block field is only valid if the previous block is free.
** - The prev_phys_block field is actually stored at the end of the
**   previous block. It appears at the beginning of this structure only to
**   simplify the implementation.
** - The next_free / prev_free fields are only valid if the block is free.
*/
/*
ブロックヘッダー構造。

実装にはいくつかの微妙な点があります:
    - prev_phys_blockフィールドは、前のブロックがfreeの場合にのみ有効です。
    
    - prev_phys_blockフィールドは、実際には前のブロックの終わりの位置に格納されます。
      この構造の最初にprev_phys_blockを置くのは、実装を単純化するためだけです。
      ただし、上述は名目上そう定義しているだけで、
      メモリ割り当てが行われブロック分割をする際、その実装のために結局分割する空きブロックにはprev_phys_blockとsize変数のサイズを予め割り当てておく。
    
    - next_free / prev_freeフィールドは、ブロックが空いている場合にのみ有効です。
      ブロックを使用している際はnext_freeからのアドレスが実メモリ領域として扱われ提供される。

*/
typedef struct block_header_t
{
    /* Points to the previous physical block. */
    /* 前の物理ブロックを指します。 前のブロックがfreeの場合にのみ有効です。*/
    struct block_header_t* prev_phys_block;

    /* The size of this block, excluding the block header. */
    /*このブロックのサイズ（ブロックヘッダーが除かれたサイズ。）。*/
    size_t size;

    /* Next and previous free blocks. */
    /* 次と前の空きブロック。*/
    struct block_header_t* next_free;
    struct block_header_t* prev_free;
} block_header_t;

/*
** Since block sizes are always at least a multiple of 4, the two least
** significant bits of the size field are used to store the block status:
** - bit 0: whether block is busy or free
** - bit 1: whether previous block is busy or free
*/
/*
ブロックサイズは常に少なくとも4の倍数であるため、
block_header_t::sizeフィールドの最下位2ビットをブロックステータスの格納に使用されます。
ビット0：ブロックがビジーかフリーか
ビット1：前のブロックがビジーかフリーか
*/
static constexpr size_t BLOCK_HEADER_FREE_BIT = 1 << 0;
static constexpr size_t BLOCK_HEADER_PREV_FREE_BIT = 1 << 1;

/*
** The size of the block header exposed to used blocks is the size field.
** The prev_phys_block field is stored *inside* the previous free block.
*/
/*
使用済みブロックに公開されるブロックヘッダーのサイズは、sizeフィールドです。
prev_phys_blockフィールドは、前の空きブロックの「内部」に格納されます。
...prev   next
          ↓↓ここblock_header(size)
...*****oohh-----...
        ↑↑ここBLOCK_HEADER_OVERHEAD(prev_phys_block)
*/
static constexpr size_t BLOCK_HEADER_OVERHEAD = sizeof(size_t);

/* User data starts directly after the size field in a used block. */
/*ユーザーデータは、使用済みブロックのsizeフィールドの直後から始まります。*/
static constexpr size_t BLOCK_START_OFFSET = offsetof(block_header_t, size) + sizeof(size_t);

/*
** A free block must be large enough to store its header minus the size of
** the prev_phys_block field, and no larger than the number of addressable
** bits for FL_INDEX.
*/
/*
フリーブロックは、
    ヘッダーからprev_phys_blockフィールドのサイズを引いたもの(x64:24バイト)を格納するのに十分な大きさで、
    FL_INDEXのアドレス可能なビット数以下でなければなりません。
*/
/* block_header_t::sizeの最小、最大サイズ */
static constexpr size_t BLOCK_SIZE_MIN = sizeof(block_header_t) - sizeof(block_header_t*);
static constexpr size_t BLOCK_SIZE_MAX = tlsf_cast(size_t, 1) << FL_INDEX_MAX;// 現状1度に4Gib以上の割当が行えない...


/* The TLSF control structure. */
typedef struct control_t
{
    /* Empty lists point at this block to indicate they are free. */
    /*空のリストは、このブロックをポイントして、それらが空であることを示します。*/
    block_header_t block_null;

    /* Bitmaps for free lists. */
    /* フリーリストのビットマップ。*/
    unsigned int fl_bitmap;
    unsigned int sl_bitmap[FL_INDEX_COUNT];

    /* フリーリストのヘッド。*/
    block_header_t* blocks[FL_INDEX_COUNT][SL_INDEX_COUNT];
} control_t;

/* A type used for casting when doing pointer arithmetic. */
/*ポインター演算を行うときにキャストに使用されるタイプ。*/
typedef ptrdiff_t tlsfptr_t;

/*
** block_header_tメンバー関数。
*/
/*ブロックサイズは常に少なくとも4の倍数であるため、
block_header_t::sizeフィールドの最下位2ビットをブロックステータスの格納に使用されます。*/
static size_t block_size(const block_header_t* block)
{
    return block->size & ~(BLOCK_HEADER_FREE_BIT | BLOCK_HEADER_PREV_FREE_BIT);
}

static void block_set_size(block_header_t* block, size_t size)
{
    const size_t oldsize = block->size;
    block->size = size | (oldsize & (BLOCK_HEADER_FREE_BIT | BLOCK_HEADER_PREV_FREE_BIT));
}

static int block_is_last(const block_header_t* block)
{
    return block_size(block) == 0;
}

static int block_is_free(const block_header_t* block)
{
    return tlsf_cast(int, block->size & BLOCK_HEADER_FREE_BIT);
}

static void block_set_free(block_header_t* block)
{
    block->size |= BLOCK_HEADER_FREE_BIT;
}

static void block_set_used(block_header_t* block)
{
    block->size &= ~BLOCK_HEADER_FREE_BIT;
}

static int block_is_prev_free(const block_header_t* block)
{
    return tlsf_cast(int, block->size & BLOCK_HEADER_PREV_FREE_BIT);
}

static void block_set_prev_free(block_header_t* block)
{
    block->size |= BLOCK_HEADER_PREV_FREE_BIT;
}

static void block_set_prev_used(block_header_t* block)
{
    block->size &= ~BLOCK_HEADER_PREV_FREE_BIT;
}

static block_header_t* block_from_ptr(const void* ptr)
{
    return tlsf_cast(block_header_t*,
                     tlsf_cast(unsigned char*, ptr) - BLOCK_START_OFFSET);
}

static void* block_to_ptr(const block_header_t* block)
{
    return tlsf_cast(void*,
                     tlsf_cast(unsigned char*, block) + BLOCK_START_OFFSET);
}

/* Return location of next block after block of given size. */
/* 指定されたサイズのブロックの次のブロックの位置を返します。 */
static block_header_t* offset_to_block(const void* ptr, size_t size)
{
    return tlsf_cast(block_header_t*, tlsf_cast(tlsfptr_t, ptr) + size);
}

/* Return location of previous block. */
static block_header_t* block_prev(const block_header_t* block)
{
    tlsf_assert(block_is_prev_free(block) && "previous block must be free");
    return block->prev_phys_block;
}

/* Return location of next existing block. */
/* 次の既存のブロックの位置を返します。*/
/* 次の既存のブロックが既に存在する場合assert失敗*/
static block_header_t* block_next(const block_header_t* block)
{
    block_header_t* next = offset_to_block(block_to_ptr(block),
                                           block_size(block) - BLOCK_HEADER_OVERHEAD);
    tlsf_assert(!block_is_last(block));
    return next;
}

/* Link a new block with its physical neighbor, return the neighbor. */
/* 新しいブロックを物理的なネイバーにリンクし、ネイバーを返します。*/
static block_header_t* block_link_next(block_header_t* block)
{
    block_header_t* next = block_next(block);
    next->prev_phys_block = block;
    return next;
}

static void block_mark_as_free(block_header_t* block)
{
    /* block引数がblock_split()からやって来たremainingブロックなどの場合、blockの以前のブロックが使用中で、remainingがフリー。
    このような場合は、remainingの将来のnextブロックに、
    block_link_next()からremainingの将来のnextブロックにblock_set_prev_free()関数でremaining自身がフリーであることをセットする。*/

    /* Link the block to the next block, first. */
    /* まず、block(自身)を次のブロックにリンクします。 */
    block_header_t* next = block_link_next(block);// nextはblockがprevと知っている。
    block_set_prev_free(next);// nextはblockがフリーと知る。
    block_set_free(block);// blockはフリーである。
}

static void block_mark_as_used(block_header_t* block)
{
    block_header_t* next = block_next(block);
    block_set_prev_used(next);// nextのprevブロックが使用中と知る。
    block_set_used(block);// blockは使用中である。
}

static size_t align_up(size_t x, size_t align)
{
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return (x + (align - 1)) & ~(align - 1);
}

static size_t align_down(size_t x, size_t align)
{
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return x - (x & (align - 1));
}

static void* align_ptr(const void* ptr, size_t align)
{
    const tlsfptr_t aligned =
        (tlsf_cast(tlsfptr_t, ptr) + (align - 1)) & ~(align - 1);
    tlsf_assert(0 == (align & (align - 1)) && "must align to a power of two");
    return tlsf_cast(void*, aligned);
}

/*
** Adjust an allocation size to be aligned to word size, and no smaller
** than internal minimum.
*/
/*
割り当てサイズを調整して、WORD サイズに合わせ、内部最小値以上にします。
*/
static size_t adjust_request_size(size_t size, size_t align)
{
    size_t adjust = 0;
    if (size)
    {
        const size_t aligned = align_up(size, align);

        /* aligned sized must not exceed BLOCK_SIZE_MAX or we'll go out of bounds on sl_bitmap */
        /* アラインされたサイズは、BLOCK_SIZE_MAXを超えてはなりません。そうしないと、sl_bitmapの範囲外になります。 */
        if (aligned < BLOCK_SIZE_MAX)
        {
            adjust = tlsf_max(aligned, BLOCK_SIZE_MIN);
        }
    }
    return adjust;
}

/*
** TLSF utility functions. In most cases, these are direct translations of
** the documentation found in the white paper.
*/
/*TLSFユーティリティ関数。 ほとんどの場合、これらはホワイトペーパーにあるドキュメントを直接翻訳したものです。*/
static void mapping_insert(size_t size, int* fli, int* sli)
{
    int fl, sl;
    if (size < SMALL_BLOCK_SIZE)
    {
        /* Store small blocks in first list. */
        /* 最初のリストに小さなブロックを格納します。 */
        fl = 0;
        sl = tlsf_cast(int, size) / (SMALL_BLOCK_SIZE / SL_INDEX_COUNT);
    }
    else
    {
        fl = tlsf_fls_sizet(size);
        /*シフト後のsizeの有効な最上位ビット(第1レベルインデックスを示していた)を排他論理和で排除した値が第2レベルインデックス。*/
        constexpr int CORRECT_INDEX = (1 << SL_INDEX_COUNT_LOG2);
        sl = tlsf_cast(int, size >> (fl - SL_INDEX_COUNT_LOG2)) ^ CORRECT_INDEX;// (CORRECT_INDEX-1)にして論理積をしても同じ結果を求められます。
        fl -= (FL_INDEX_SHIFT - 1);
    }
    *fli = fl;
    *sli = sl;
}

/* This version rounds up to the next block size (for allocations) */
/* このバージョンは、次のブロックサイズに切り上げます（割り当て用） */
static void mapping_search(size_t size, int* fli, int* sli)
{
    if (size >= SMALL_BLOCK_SIZE)
    {
        /* 第1レベルの区分サイズを第2レベルのインデックス総数で分割する際に割り切れるように値を切り上げする。
        -1によって割り切れない値の桁のビットが全て有効になり、これを加算することで切り上げられる。
        ただ、このまま(size+=round)だと依然として割り切れない桁が存在しているかもしれないが、実際に必要なのは第2レベルのインデックス(切り上げ桁以上の桁からスタートする)で、
        これはsize >> (tlsf_fls_sizet(size) - SL_INDEX_COUNT_LOG2)のようにmapping_insert関数内でシフトされて取得するので、実質切り上げ桁未満の桁の値は破棄される*/
        const size_t round = (1ull << (tlsf_fls_sizet(size) - tlsf_cast(size_t, SL_INDEX_COUNT_LOG2))) - 1ull;
        size += round;
    }
    mapping_insert(size, fli, sli);
}

static block_header_t* search_suitable_block(control_t* control, int* fli, int* sli)
{
    int fl = *fli;
    int sl = *sli;

    /*
    ** First, search for a block in the list associated with the given
    ** fl/sl index.
    */
    /*最初に、指定された fl/sl インデックスに関連付けられたリストでブロックを検索します。*/
    // sl以下のサイズ用ビットを排除
    unsigned int sl_map = control->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map)
    {
        /* No block exists. Search in the next largest first-level list. */
        /* ブロックは存在しません。 次に大きい第1レベルのリストを検索します。 */
        const unsigned int fl_map = control->fl_bitmap & (~0U << (fl + 1/*現在の第1レベルインデックスの1つ上*/));
        if (!fl_map)
        {
            /* No free blocks available, memory has been exhausted. */
            /*利用可能な空きブロックがありません。メモリを使い果たしました。*/
            // add_pool等でメモリを追加する必要がある。
            return 0;
        }

        fl = tlsf_ffs(fl_map);
        *fli = fl;
        /* 既に第1レベルの1つ大きいインデックスから検索しており、
        すなわち第2レベルはそのfl中の最下位に確実に属することが可能なのでビットの排除は行わない。*/
        sl_map = control->sl_bitmap[fl];// & (~0U << sl) slは確実にゼロなので必要ない。
    }
    tlsf_assert(sl_map && "internal error - second level bitmap is null");
    sl = tlsf_ffs(sl_map);
    *sli = sl;

    /* Return the first block in the free list. */
    /* フリーリストの最初のブロックを返します。 */
    return control->blocks[fl][sl];
}

/* Remove a free block from the free list.*/
/* フリーリストからフリーブロックを削除します。*/
static void remove_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    // prevとnextの間のblockを抜き取る。
    block_header_t* prev = block->prev_free;
    block_header_t* next = block->next_free;
    tlsf_assert(prev && "prev_free field can not be null");
    tlsf_assert(next && "next_free field can not be null");
    next->prev_free = prev;
    prev->next_free = next;

    /* If this block is the head of the free list, set new head. */
    /* このブロックがフリーリストの先頭である場合は、新しい先頭を設定します。 */
    // search_suitable_blockからの引数だったら必ずこのifに入る
    if (control->blocks[fl][sl] == block)
    {
        control->blocks[fl][sl] = next;

        /* If the new head is null, clear the bitmap. */
        /* 新しいヘッドがnullの場合、ビットマップをクリアします。 */
        if (next == &control->block_null)
        {
            control->sl_bitmap[fl] &= ~(1U << sl);// slに次のブロックが無い。

            /* If the second bitmap is now empty, clear the fl bitmap. */
            /* 2番目のビットマップが空になった場合は、flビットマップをクリアします。 */
            if (!control->sl_bitmap[fl])
            {
                control->fl_bitmap &= ~(1U << fl);// flの指すslに次のブロックが無い。
            }
        }
    }
}

/* Insert a free block into the free block list. */
/*フリーブロックリストにフリーブロックを挿入します。 */
/*挙動はpush_front */
static void insert_free_block(control_t* control, block_header_t* block, int fl, int sl)
{
    block_header_t* current = control->blocks[fl][sl];
    tlsf_assert(current && "free list cannot have a null entry");
    tlsf_assert(block && "cannot insert a null entry into the free list");
    block->next_free = current;
    block->prev_free = &control->block_null;
    current->prev_free = block;

    tlsf_assert(block_to_ptr(block) == align_ptr(block_to_ptr(block), ALIGN_SIZE)
                && "block not aligned properly");
    /*
    ** Insert the new block at the head of the list, and mark the first-
    ** and second-level bitmaps appropriately.
    */
    /*リストの先頭に新しいブロックを挿入し、第1レベルと第2レベルのビットマップを適切にマークします。*/

    control->blocks[fl][sl] = block;
    control->fl_bitmap |= (1U << fl);
    control->sl_bitmap[fl] |= (1U << sl);
}

/* Remove a given block from the free list. */
/* フリーリストから特定のブロックを削除します。 */
static void block_remove(control_t* control, block_header_t* block)
{
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

/* Insert a given block into the free list. */
/* 特定のブロックをフリーリストに挿入します。 */
static void block_insert(control_t* control, block_header_t* block)
{
    int fl, sl;
    mapping_insert(block_size(block), &fl, &sl);
    insert_free_block(control, block, fl, sl);
}

static int block_can_split(block_header_t* block, size_t size)
{
    /* 1つのブロックを分割することは、ヘッダーを2つに増やさなければならず、
    少なくとも分割されるブロックが 
        block_header_t のサイズ(block->sizeが返すのは その構造が持つメモリの実際のサイズ-block_header_t のサイズ)
        と要求されたメモリのサイズ
    の合計以上の空きが存在する必要がある。*/
    /*実際のサイズは block->size + sizeof(block_header_t)なので、size変数は自身のサイズを隠蔽している。*/
    return block_size(block) >= sizeof(block_header_t) + size;
}

/* Split a block into two, the second of which is free. */
/* ブロックを2つに分割し、2つ目をフリーブロックとします。 */
static block_header_t* block_split(block_header_t* block, size_t size)
{
    /* Calculate the amount of space left in the remaining block. */
    /* 残りのブロックに残っているスペースの量を計算します。*/
    // block->next_freeからのアドレス + ( 調節されたサイズ(最小24バイト) - prev_phys_blockのサイズ(8バイト) )
    // ブロックを使用している際はblock->next_freeからのアドレスを実メモリ領域として扱う。
    /* ここでBLOCK_HEADER_OVERHEADを引いているのは実際のblock_header_tの先頭アドレス(&prev_phys_block)は、以前のブロックの最後の位置に設置されるため。
       oohh---size---...
           ↓block_to_ptr(block)
       oohh---size-θθ...
                   ↑remainingはここから*/
    /* sizeはALIGN_SIZEでアライメントされている(この関数の前に要求された実際のサイズではない)ため、
    size - BLOCK_HEADER_OVERHEADとしてしまってもそれは管理領域なので正しく動作する。
    この関数だけで考えてしまうと混乱する。*/
    block_header_t* remaining =
        offset_to_block(block_to_ptr(block), size - BLOCK_HEADER_OVERHEAD);

    // prev_phys_block変数のサイズを除いたメモリのサイズ - ( 調節されたサイズ(最小24バイト) + prev_phys_blockのサイズ(8バイト) )
    // ここでprev_phys_blockのサイズを加算しているのはremainingの最後の位置にprev_phys_blockを配置するため。
    const size_t remain_size = block_size(block) - (size + BLOCK_HEADER_OVERHEAD);
    
    tlsf_assert(block_to_ptr(remaining) == align_ptr(block_to_ptr(remaining), ALIGN_SIZE)
                && "remaining block not aligned properly");

    tlsf_assert(block_size(block) == remain_size + size + BLOCK_HEADER_OVERHEAD);
    block_set_size(remaining, remain_size);
    tlsf_assert(block_size(remaining) >= BLOCK_SIZE_MIN && "block split with invalid size");

    // blockはsize引数サイズの使用中ブロックにされて、残りの所有していたサイズ(remainingヘッダ用のサイズは含まず)をremainingに譲渡し、それをフリーブロックとしてマークする。
    block_set_size(block, size);
    block_mark_as_free(remaining);

    return remaining;
}

/* Absorb a free block's storage into an adjacent previous free block. */
/* フリーブロックのストレージを隣接する前のフリーブロックに吸収します。 */
static block_header_t* block_absorb(block_header_t* prev, block_header_t* block)
{
    tlsf_assert(!block_is_last(prev) && "previous block can't be last");
    /* Note: Leaves flags untouched. */
    /* Note: フラグはそのままにしておきます。 */
    prev->size += block_size(block) + BLOCK_HEADER_OVERHEAD;
    block_link_next(prev);
    return prev;
}

/* Merge a just-freed block with an adjacent previous free block. */
static block_header_t* block_merge_prev(control_t* control, block_header_t* block)
{
    if (block_is_prev_free(block))
    {
        block_header_t* prev = block_prev(block);
        tlsf_assert(prev && "prev physical block can't be null");
        tlsf_assert(block_is_free(prev) && "prev block is not free though marked as such");
        block_remove(control, prev);
        block = block_absorb(prev, block);
    }

    return block;
}

/* Merge a just-freed block with an adjacent free block. */
/* 解放されたばかりのブロックを隣接する空きブロックとマージします。 */
static block_header_t* block_merge_next(control_t* control, block_header_t* block)
{
    block_header_t* next = block_next(block);
    tlsf_assert(next && "next physical block can't be null");

    if (block_is_free(next))
    {
        tlsf_assert(!block_is_last(block) && "previous block can't be last");
        block_remove(control, next);
        block = block_absorb(block, next);
    }

    return block;
}

/* Trim any trailing block space off the end of a block, return to pool. */
/* 末尾のブロックスペースをブロックの終わりから切り取り、プールに返却します。*/
static void block_trim_free(control_t* control, block_header_t* block, size_t size)
{
    tlsf_assert(block_is_free(block) && "block must be free");
    // 現在のブロックが残りのメモリサイズで自身を分割可能か確認。
    if (block_can_split(block, size))
    {
        block_header_t* remaining_block = block_split(block, size);
        block_link_next(block);                 // 使用中と化したblockのnextブロックに、remaining_block を登録する。
        block_set_prev_free(remaining_block);   // 次の関数でprevブロックにnullブロックが挿入される事に注意してください。
        block_insert(control, remaining_block); // 空きブロックリストにremaining_blockをpush_front
    }
}

/* Trim any trailing block space off the end of a used block, return to pool. */
/* 使用済みブロックの末尾から末尾のブロックスペースをすべて切り取り、プールに返却します。 */
static void block_trim_used(control_t* control, block_header_t* block, size_t size)
{
    tlsf_assert(!block_is_free(block) && "block must be used");
    if (block_can_split(block, size))
    {
        /* If the next block is free, we must coalesce. */
        /* 次のブロックが空いている場合は、合体する必要があります。 */
        block_header_t* remaining_block = block_split(block, size);
        block_set_prev_used(remaining_block);

        remaining_block = block_merge_next(control, remaining_block);
        block_insert(control, remaining_block);
    }
}

static block_header_t* block_trim_free_leading(control_t* control, block_header_t* block, size_t size)
{
    block_header_t* remaining_block = block;
    if (block_can_split(block, size))
    {
        /* We want the 2nd block. */
        /* 2番目のブロックが必要です。*/
        /*アライメントの都合で、前のブロックと分割後のアライメントが必要な次のブロックとの間に橋渡しのような存在のブロックが必要。
        そのブロックは、block_splitによって分割された｢1番目｣ブロックで、後に再利用可能です。
        従って2番目ブロックがアライメントが必要なブロックで、使用中のブロックになります。
        (block->prev <-> block(to bridge) <-> block->next)*/
        remaining_block = block_split(block, size - BLOCK_HEADER_OVERHEAD);
        block_set_prev_free(remaining_block);

        block_link_next(block);
        block_insert(control, block);
    }

    return remaining_block;
}

static block_header_t* block_locate_free(control_t* control, size_t size)
{
    int fl = 0, sl = 0;
    block_header_t* block = 0;

    if (size)
    {
        mapping_search(size, &fl, &sl);

        /*
        ** mapping_search can futz with the size, so for excessively large sizes it can sometimes wind up
        ** with indices that are off the end of the block array.
        ** So, we protect against that here, since this is the only callsite of mapping_search.
        ** Note that we don't need to check sl, since it comes from a modulo operation that guarantees it's always in range.
        */
        /*mapping_searchはサイズを混乱させる可能性があるため、サイズが過度に大きい場合、ブロック配列の末尾から外れるインデックスが発生することがあります。 
        これは、ここがmapping_searchの唯一の呼び出し場所であるため、ここでは保護します。
        slをチェックする必要がないことに注意してください。これは、常に範囲内にあることを保証するモジュロ演算からのものであるためです。*/
        if (fl < FL_INDEX_COUNT)
        {
            block = search_suitable_block(control, &fl, &sl);
        }
    }

    if (block)
    {
        tlsf_assert(block_size(block) >= size);
        remove_free_block(control, block, fl, sl);
    }

    return block;
}

static void* block_prepare_used(control_t* control, block_header_t* block, size_t size)
{
    void* p = 0;
    if (block)
    {
        tlsf_assert(size && "size must be non-zero");
        block_trim_free(control, block, size);
        block_mark_as_used(block);
        p = block_to_ptr(block);
    }
    return p;
}

/* Clear structure and point all empty lists at the null block. */
/* 構造をクリアし、空のリストをすべてnullブロックに向けます。 */
static void control_construct(control_t* control)
{
    *control = {};

    int i, j;

    control->block_null.next_free = &control->block_null;
    control->block_null.prev_free = &control->block_null;

    control->fl_bitmap = 0;
    for (i = 0; i < FL_INDEX_COUNT; ++i)
    {
        control->sl_bitmap[i] = 0;
        for (j = 0; j < SL_INDEX_COUNT; ++j)
        {
            control->blocks[i][j] = &control->block_null;
        }
    }
}

/*
** Debugging utilities.
*/

typedef struct integrity_t
{
    int prev_status;
    int status;
} integrity_t;

#define tlsf_insist(x) { tlsf_assert(x); if (!(x)) { status--; } }

static void integrity_walker(void* ptr, size_t size, int used, void* user)
{
    block_header_t* block = block_from_ptr(ptr);
    integrity_t* integ = tlsf_cast(integrity_t*, user);
    const int this_prev_status = block_is_prev_free(block) ? 1 : 0;
    const int this_status = block_is_free(block) ? 1 : 0;
    const size_t this_block_size = block_size(block);

    int status = 0;
    (void)used;
    tlsf_insist(integ->prev_status == this_prev_status && "prev status incorrect");
    tlsf_insist(size == this_block_size && "block size incorrect");

    integ->prev_status = this_status;
    integ->status += status;
}

int tlsf_check(tlsf_t tlsf)
{
    int i, j;

    control_t* control = tlsf_cast(control_t*, tlsf);
    int status = 0;

    /* Check that the free lists and bitmaps are accurate. */
    for (i = 0; i < FL_INDEX_COUNT; ++i)
    {
        for (j = 0; j < SL_INDEX_COUNT; ++j)
        {
            const int fl_map = control->fl_bitmap & (1U << i);
            const int sl_list = control->sl_bitmap[i];
            const int sl_map = sl_list & (1U << j);
            const block_header_t* block = control->blocks[i][j];

            /* Check that first- and second-level lists agree. */
            if (!fl_map)
            {
                tlsf_insist(!sl_map && "second-level map must be null");
            }

            if (!sl_map)
            {
                tlsf_insist(block == &control->block_null && "block list must be null");
                continue;
            }

            /* Check that there is at least one free block. */
            tlsf_insist(sl_list && "no free blocks in second-level map");
            tlsf_insist(block != &control->block_null && "block should not be null");

            while (block != &control->block_null)
            {
                int fli, sli;
                tlsf_insist(block_is_free(block) && "block should be free");
                tlsf_insist(!block_is_prev_free(block) && "blocks should have coalesced");
                tlsf_insist(!block_is_free(block_next(block)) && "blocks should have coalesced");
                tlsf_insist(block_is_prev_free(block_next(block)) && "block should be free");
                tlsf_insist(block_size(block) >= BLOCK_SIZE_MIN && "block not minimum size");

                mapping_insert(block_size(block), &fli, &sli);
                tlsf_insist(fli == i && sli == j && "block size indexed in wrong list");
                block = block->next_free;
            }
        }
    }

    return status;
}

#undef tlsf_insist

static void default_walker(void* ptr, size_t size, int used, void* user)
{
    (void)user;
    printf("\t%p %s size: %x (%p)\n", ptr, used ? "used" : "free", (unsigned int)size, block_from_ptr(ptr));
}

void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void* user)
{
    tlsf_walker pool_walker = walker ? walker : default_walker;
    block_header_t* block =
        offset_to_block(pool, size_t(-(int)BLOCK_HEADER_OVERHEAD));

    while (block && !block_is_last(block))
    {
        pool_walker(
            block_to_ptr(block),
            block_size(block),
            !block_is_free(block),
            user);
        block = block_next(block);
    }
}

size_t tlsf_block_size(void* ptr)
{
    size_t size = 0;
    if (ptr)
    {
        const block_header_t* block = block_from_ptr(ptr);
        size = block_size(block);
    }
    return size;
}

int tlsf_check_pool(pool_t pool)
{
    /* Check that the blocks are physically correct. */
    /* ブロックが物理的に正しいことを確認します。 */
    integrity_t integ = { 0, 0 };
    tlsf_walk_pool(pool, integrity_walker, &integ);

    return integ.status;
}

/*
** Size of the TLSF structures in a given memory block passed to
** tlsf_create, equal to the size of a control_t
*/
size_t tlsf_size(void)
{
    return sizeof(control_t);
}

size_t tlsf_align_size(void)
{
    return ALIGN_SIZE;
}

size_t tlsf_block_size_min(void)
{
    return BLOCK_SIZE_MIN;
}

size_t tlsf_block_size_max(void)
{
    return BLOCK_SIZE_MAX;
}

/*
** Overhead of the TLSF structures in a given memory block passed to
** tlsf_add_pool, equal to the overhead of a free block and the
** sentinel block.
*/
/*
tlsf_add_poolに渡された特定のメモリブロック内のTLSF構造のオーバーヘッド。
これは、空きブロックおよび監視ブロックのオーバーヘッドと同じです。
*/
size_t tlsf_pool_overhead(void)
{
    return 2 * BLOCK_HEADER_OVERHEAD;
}

size_t tlsf_alloc_overhead(void)
{
    return BLOCK_HEADER_OVERHEAD;
}

size_t tlsf_block_header_overhead(void)
{
    return BLOCK_HEADER_OVERHEAD;
}

pool_t tlsf_add_pool(tlsf_t tlsf, void* mem, size_t bytes)
{
    block_header_t* block;
    block_header_t* next;

    /*prev_phys_blockとsize変数のサイズを合計した値。
    初めてプールにブロックを割り当てる際は、以前のブロックは当然存在しない。
    prev_phys_blockは実際は以前のブロックの最後の位置に割り当てる必要があるが、それが出来ない。
    なので初めてプールにブロックを割り当てる際はprev_phys_blockのみを割り当てる領域も一緒に確保する。
    (実際tlsf_pool_overhead()関数ここでしか使われていないので恐らくは前述の通り)*/
    const size_t pool_overhead = tlsf_pool_overhead();
    const size_t pool_bytes = align_down(bytes - pool_overhead, ALIGN_SIZE);

    if (((ptrdiff_t)mem % ALIGN_SIZE) != 0)
    {
        printf("tlsf_add_pool: Memory must be aligned by %u bytes.\n",
               (unsigned int)ALIGN_SIZE);
        return 0;
    }

    if (pool_bytes < BLOCK_SIZE_MIN || pool_bytes > BLOCK_SIZE_MAX)
    {
    #if defined (TLSF_64BIT)
        printf("tlsf_add_pool: Memory size must be between 0x%x and 0x%x00 bytes.\n",
               (unsigned int)(pool_overhead + BLOCK_SIZE_MIN),
               (unsigned int)((pool_overhead + BLOCK_SIZE_MAX) / 256));
    #else
        printf("tlsf_add_pool: Memory size must be between %u and %u bytes.\n",
               (unsigned int)(pool_overhead + BLOCK_SIZE_MIN),
               (unsigned int)(pool_overhead + BLOCK_SIZE_MAX));
    #endif
        return 0;
    }

    /*
    ** Create the main free block. Offset the start of the block slightly
    ** so that the prev_phys_block field falls outside of the pool -
    ** it will never be used.
    */
    /*メインのフリーブロックを作成します。 
    prev_phys_blockフィールドがプールの外側になるように、ブロックの開始を少しオフセットします
    -これは決して使用されません。
    */
    // WARNING: memはここで -BLOCK_HEADER_OVERHEAD 分オフセットされるので、引数memを予め +BLOCK_HEADER_OVERHEAD 分オフセットして渡す必要がある。
    block = offset_to_block(mem, size_t(-(tlsfptr_t)BLOCK_HEADER_OVERHEAD));
    block_set_size(block, pool_bytes);
    block_set_free(block);
    block_set_prev_used(block);
    block_insert(tlsf_cast(control_t*, tlsf), block);

    /* Split the block to create a zero-size sentinel block. */
    /*ブロックを分割して、サイズがゼロのセンチネルブロックを作成します。*/
    next = block_link_next(block);
    block_set_size(next, 0);
    block_set_used(next);
    block_set_prev_free(next);

    // WARNING: tlsf_remove_pool側で size_t(-(int)BLOCK_HEADER_OVERHEAD) オフセットしているので、返ったポインタを更にオフセットしてはならない。
    return mem;
}

void tlsf_remove_pool(tlsf_t tlsf, pool_t pool)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    block_header_t* block = offset_to_block(pool, size_t(-(int)BLOCK_HEADER_OVERHEAD));

    int fl = 0, sl = 0;

    tlsf_assert(block_is_free(block) && "block should be free");
    tlsf_assert(!block_is_free(block_next(block)) && "next block should not be free");
    tlsf_assert(block_size(block_next(block)) == 0 && "next block size should be zero");

    mapping_insert(block_size(block), &fl, &sl);
    remove_free_block(control, block, fl, sl);
}

bool tlsf_can_remove_pool(pool_t pool)
{
    block_header_t* block = offset_to_block(pool, size_t(-(int)BLOCK_HEADER_OVERHEAD));
    return	block_is_free(block)                &&
            !block_is_free(block_next(block))   &&
            block_size(block_next(block)) == 0;
}

/*
** TLSF main interface.
*/

#if _DEBUG
int test_ffs_fls()
{
    /* Verify ffs/fls work properly. */
    int rv = 0;
    rv += (tlsf_ffs(0) == -1) ? 0 : 0x1;
    rv += (tlsf_fls(0) == -1) ? 0 : 0x2;
    rv += (tlsf_ffs(1) == 0) ? 0 : 0x4;
    rv += (tlsf_fls(1) == 0) ? 0 : 0x8;
    rv += (tlsf_ffs(0x80000000) == 31) ? 0 : 0x10;
    rv += (tlsf_ffs(0x80008000) == 15) ? 0 : 0x20;
    rv += (tlsf_fls(0x80000008) == 31) ? 0 : 0x40;
    rv += (tlsf_fls(0x7FFFFFFF) == 30) ? 0 : 0x80;

#if defined (TLSF_64BIT)
    rv += (tlsf_fls_sizet(0x80000000) == 31) ? 0 : 0x100;
    rv += (tlsf_fls_sizet(0x100000000) == 32) ? 0 : 0x200;
    rv += (tlsf_fls_sizet(0xffffffffffffffff) == 63) ? 0 : 0x400;
#endif

    if (rv)
    {
        printf("test_ffs_fls: %x ffs/fls tests failed.\n", rv);
        tlsf_assert(false);
    }
    return rv;
}
#endif

tlsf_t tlsf_create(void* mem)
{
#if _DEBUG
    if (test_ffs_fls())
    {
        return 0;
    }
#endif

    if (((tlsfptr_t)mem % ALIGN_SIZE) != 0)
    {
        printf("tlsf_create: Memory must be aligned to %u bytes.\n",
               (unsigned int)ALIGN_SIZE);
        return 0;
    }

    control_construct(tlsf_cast(control_t*, mem));

    return tlsf_cast(tlsf_t, mem);
}

tlsf_t tlsf_create_with_pool(void* mem, size_t bytes)
{
    tlsf_t tlsf = tlsf_create(mem);
    tlsf_add_pool(tlsf, (char*)mem + tlsf_size(), bytes - tlsf_size());
    return tlsf;
}

void tlsf_destroy(tlsf_t tlsf)
{
    /* Nothing to do. */
    (void)tlsf;
}

pool_t tlsf_get_pool(tlsf_t tlsf)
{
    return tlsf_cast(pool_t, (char*)tlsf + tlsf_size());
}

void* tlsf_malloc(tlsf_t tlsf, size_t size)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);
    block_header_t* block = block_locate_free(control, adjust);
    return block_prepare_used(control, block, adjust);
}

void* tlsf_aligned_malloc(tlsf_t tlsf, size_t size, size_t align)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    const size_t adjust = adjust_request_size(size, ALIGN_SIZE);// ブロック管理用サイズのために、まずTLSFアライメントに

    /*
    ** We must allocate an additional minimum block size bytes so that if
    ** our free block will leave an alignment gap which is smaller, we can
    ** trim a leading free block and release it back to the pool. We must
    ** do this because the previous physical block is in use, therefore
    ** the prev_phys_block field is not valid, and we can't simply adjust
    ** the size of that block.
    */
    /*追加のミニマムブロックサイズバイトを割り当てる必要があります。
    これにより、空きブロックが小さいアライメントギャップを残す場合、
    先頭の空きブロックをトリミングして解放してプールに戻すことができます。
    これは、前の物理ブロックが使用中であり、したがってprev_phys_blockフィールドが無効であり、
    そのブロックのサイズを単純に調整できないためです。*/
    // アライメントによって生じた隙間にフリーブロックを構築出来ないか試行する。
    // 構築可能な場合のために、例のごとくblock_header_tの領域分のサイズを要求サイズに加算している。
    // block_header_tのリスト構造の実装のためでもあり、アライメントされたブロックとその以前のブロックの橋渡しの役割も担う。
    constexpr size_t GAP_MINIMUM = sizeof(block_header_t);
    // + alignしているのは橋渡しブロック自体のサイズ確保のため
    const size_t size_with_gap = adjust_request_size(adjust + align + GAP_MINIMUM, align);

    /*32バイト要求、16バイトアライメント、取得したブロックポインタアドレスが8アラインの場合、
    最初ｷﾞｬｯﾌﾟが発生した時点で架け橋フリーブロックの作成が確定する。
    16|             16|      8|     16|      8|     16|             16|      8|     16|             16|             16|             16|
      ------------------------|---------------|---------------------------------------|------------------------------------------------
                              |               |↓最初ｷﾞｬｯﾌﾟ,ｱﾗｲﾒﾝﾄ             ↓ｱﾗｲﾒﾝﾄ↓|                                                
      ************************hhhhhhhhssssssssplllllllggggggggggggggggggggggggllllllllaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.................
              block->prev_phys↑               ↑blockのポインタ                        ↑ここからアライメントされたブロックのポインタ    
                           block->size↑       |       ↑-最小ギャップのremain-↑       ↑ここまでがgapの値                                
    
    架け橋ブロック作成後
                              |               |                                       ↓ここからアライメントされたブロックのポインタ    
      ************************hhhhhhhhsssssssspbbbbbbbbbbbbbbbbbbbbbbbhhhhhhhhsssssssspaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.................
                              ↑-----ここまで架け橋フリーブロック-----↑↑ここからアライメントされたブロックヘッダ(prev_phys_block)       
    */

    /*
    ** If alignment is less than or equals base alignment, we're done.
    ** If we requested 0 bytes, return null, as tlsf_malloc(0) does.
    */
    /*アライメントがベースアライメント以下の場合は、完了です。
    0バイトを要求した場合は、tlsf_malloc（0）と同様にnullを返します。*/
    const size_t aligned_size = (adjust && align > ALIGN_SIZE) ? size_with_gap : adjust;

    block_header_t* block = block_locate_free(control, aligned_size);

    /* This can't be a static assert. */
    //tlsf_assert(sizeof(block_header_t) == BLOCK_SIZE_MIN + BLOCK_HEADER_OVERHEAD);
    tlsf_static_assert(sizeof(block_header_t) == BLOCK_SIZE_MIN + BLOCK_HEADER_OVERHEAD);

    if (block)
    {
        void* ptr = block_to_ptr(block);
        void* aligned = align_ptr(ptr, align);
        size_t gap = tlsf_cast(size_t,
                               tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

        // ギャップが存在した時点で架け橋フリーブロックの作成が確定する。
        if (gap)
        {
            /* If gap size is too small, offset to next aligned boundary. */
            /* ギャップサイズが小さすぎる場合は、次の位置合わせされた境界にオフセットします。 */
            if (gap < GAP_MINIMUM)
            {
                const size_t gap_remain = GAP_MINIMUM - gap;
                const size_t offset = tlsf_max(gap_remain, align);
                const void* next_aligned = tlsf_cast(void*,
                                                     tlsf_cast(tlsfptr_t, aligned) + offset);

                aligned = align_ptr(next_aligned, align);
                gap = tlsf_cast(size_t,
                                tlsf_cast(tlsfptr_t, aligned) - tlsf_cast(tlsfptr_t, ptr));

            }
            // 架け橋となるフリーブロックを作成してprevブロックとnextであるアライメントされたブロックをそれでつなぐ。
            tlsf_assert(gap >= GAP_MINIMUM && "gap size too small");
            block = block_trim_free_leading(control, block, gap);
        }
    }

    // ギャップが存在したなら、blockは架け橋フリーブロックからとなる。
    return block_prepare_used(control, block, adjust);
}

void tlsf_free(tlsf_t tlsf, void* ptr)
{
    /* Don't attempt to free a NULL pointer. */
    /* NULLポインターを解放しようとしないでください。 */
    if (ptr)
    {
        control_t* control = tlsf_cast(control_t*, tlsf);
        block_header_t* block = block_from_ptr(ptr);
        tlsf_assert(!block_is_free(block) && "block already marked as free");
        block_mark_as_free(block);
        block = block_merge_prev(control, block);
        block = block_merge_next(control, block);
        block_insert(control, block);
    }
}

/*
** The TLSF block information provides us with enough information to
** provide a reasonably intelligent implementation of realloc, growing or
** shrinking the currently allocated block as required.
**
** This routine handles the somewhat esoteric edge cases of realloc:
** - a non-zero size with a null pointer will behave like malloc
** - a zero size with a non-null pointer will behave like free
** - a request that cannot be satisfied will leave the original buffer
**   untouched
** - an extended buffer size will leave the newly-allocated area with
**   contents undefined
*/
/*
TLSFブロック情報は、必要に応じて現在割り当てられているブロックを拡大または縮小する、
reallocの合理的にインテリジェントな実装を提供するのに十分な情報を提供します。

このルーチンは、reallocのやや難解なエッジケースを処理します:
    - nullポインタを持つゼロ以外のサイズは、mallocのように動作します。
    - null以外のポインタを持つサイズがゼロの場合、freeのように動作します。
    - 満たすことができないリクエストは、元のバッファをそのまま残します。
    - 拡張バッファサイズを指定すると、新しく割り当てられた領域は内容が未定義のままになります。

*/
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    void* p = 0;

    /* Zero-size requests are treated as free. */
    /* サイズがゼロのリクエストは無料として扱われます。*/
    if (ptr && size == 0)
    {
        tlsf_free(tlsf, ptr);
    }
    /* Requests with NULL pointers are treated as malloc. */
    /* NULLポインターを持つ要求は、mallocとして扱われます。 */
    else if (!ptr)
    {
        p = tlsf_malloc(tlsf, size);
    }
    else
    {
        block_header_t* block = block_from_ptr(ptr);
        block_header_t* next = block_next(block);

        const size_t cursize = block_size(block);
        const size_t combined = cursize + block_size(next) + BLOCK_HEADER_OVERHEAD;
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

        tlsf_assert(!block_is_free(block) && "block already marked as free");

        /*
        ** If the next block is used, or when combined with the current
        ** block, does not offer enough space, we must reallocate and copy.
        */
        /* 次のブロックが使用されている場合、または現在のブロックと組み合わせた場合に十分なスペースがない場合は、
        再割り当てしてコピーする必要があります。*/
        if (adjust > cursize && (!block_is_free(next) || adjust > combined))
        {
            p = tlsf_malloc(tlsf, size);
            if (p)
            {
                const size_t minsize = tlsf_min(cursize, size);
                memcpy(p, ptr, minsize);
                tlsf_free(tlsf, ptr);
            }
        }
        else
        {
            /* Do we need to expand to the next block? */
            if (adjust > cursize)
            {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }

            /* Trim the resulting block and return the original pointer. */
            block_trim_used(control, block, adjust);
            p = ptr;
        }
    }

    return p;

}

void* tlsf_aligned_realloc(tlsf_t tlsf, void* ptr, size_t size, size_t align)
{
    control_t* control = tlsf_cast(control_t*, tlsf);
    void* p = 0;

    /* Zero-size requests are treated as free. */
    /* サイズがゼロのリクエストはfreeとして扱われます。*/
    if (ptr && size == 0)
    {
        tlsf_free(tlsf, ptr);
    }
    /* Requests with NULL pointers are treated as malloc. */
    /* NULLポインターを持つ要求は、mallocとして扱われます。 */
    else if (!ptr)
    {
        p = tlsf_aligned_malloc(tlsf, size, align);
    }
    else
    {
        // NOTE: msdn、_aligned_realloc関数より、メモリを再割り当てしてブロックのアライメントを変更すると、エラーになります。
        // blockのポインタアドレスはアライメントされている前提なので、考慮する必要はない。
        // WARNING: アライメント指定が以前の割当の際と異なる場合のエラー処理が行われない。

        block_header_t* block = block_from_ptr(ptr);
        block_header_t* next = block_next(block);

        const size_t cursize = block_size(block);
        const size_t combined = cursize + block_size(next) + BLOCK_HEADER_OVERHEAD;
        const size_t adjust = adjust_request_size(size, ALIGN_SIZE);

        const size_t aligned_size = (adjust && align > ALIGN_SIZE) ? adjust_request_size(adjust, align) : adjust;

        tlsf_assert(!block_is_free(block) && "block already marked as free");

        /*
        ** If the next block is used, or when combined with the current
        ** block, does not offer enough space, we must reallocate and copy.
        */
        /* 次のブロックが使用されている場合、または現在のブロックと組み合わせた場合に十分なスペースがない場合は、
        再割り当てしてコピーする必要があります。*/
        if (aligned_size > cursize && (!block_is_free(next) || aligned_size > combined))
        {
            p = tlsf_aligned_malloc(tlsf, size, align);
            if (p)
            {
                const size_t minsize = tlsf_min(cursize, size);
                memcpy(p, ptr, minsize);
                tlsf_free(tlsf, ptr);
            }
        }
        else
        {
            /* Do we need to expand to the next block? */
            /* 次のブロックに拡張する必要がありますか？*/
            if (aligned_size > cursize)
            {
                block_merge_next(control, block);
                block_mark_as_used(block);
            }
    
            /* Trim the resulting block and return the original pointer. */
            /* 結果のブロックをトリムし、元のポインターを返します。 */
            block_trim_used(control, block, aligned_size);
            p = ptr;
        }
    }

    return p;
}


}// namespace tlsf
