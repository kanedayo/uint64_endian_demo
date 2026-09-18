/*
 * uint64_endian_demo.c
 *
 * uint64_t のメモリレイアウトがエンディアンによってどのように異なるか
 * を可視化するデモプログラム。
 *
 * 値 0x0123456789ABCDEF を各バイト単位で表示し、
 * リトルエンディアンとビッグエンディアンの違いを明確にする。
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>

/* 値の設定 — ULL サフィックス必須 */
#define TEST_VALUE 0x0123456789ABCDEFULL

/* バイト列を16進数で出力 */
static void print_bytes(const char *label, const uint8_t *bytes, size_t len)
{
    printf("%-16s", label);
    for (size_t i = 0; i < len; i++) {
        printf(" 0x%02X", bytes[i]);
    }
    printf("\n");
}

/* 値のバイト単位での順序（アドレス低位→高位）を表示 */
static void print_memory_layout(const char *label, const void *ptr, size_t len)
{
    const uint8_t *bytes = (const uint8_t *)ptr;

    printf("%-16s", label);
    for (size_t i = 0; i < len; i++) {
        printf(" [%zu]=0x%02X", i, bytes[i]);
    }
    printf("\n");
}

/* ビッグエンディアン風に変換した値のレイアウトも表示 */
static void to_big_endian_bytes(uint64_t val, uint8_t *out)
{
    for (int i = 0; i < 8; i++) {
        out[i] = (uint8_t)((val >> (56 - i * 8)) & 0xFFULL);
    }
}

/* エンディアン文字列を取得 */
static const char *endian_str(int val)
{
    switch (val) {
#ifdef __ORDER_LITTLE_ENDIAN__
        case __ORDER_LITTLE_ENDIAN__: return "LITTLE";
#endif
#ifdef __ORDER_BIG_ENDIAN__
        case __ORDER_BIG_ENDIAN__:    return "BIG";
#endif
#ifdef __ORDER_PDP_ENDIAN__
        case __ORDER_PDP_ENDIAN__:    return "PDP";
#endif
        default: return "UNKNOWN";
    }
}

int main(void)
{
    uint64_t value = TEST_VALUE;
    uint8_t bytes[8];
    uint8_t swapped_bytes[8];

    printf("========================================================\n");
    printf(" uint64_t Endian Layout Demo\n");
    printf("========================================================\n\n");

    /* 基本情報 */
    printf("[Value]   0x%016" PRIx64 "\n\n", (uint64_t)TEST_VALUE);

    /* uint64_t のサイズ確認 */
    printf("[Size]    sizeof(uint64_t) = %zu bytes\n\n", sizeof(uint64_t));

    /* メモリ上のバイト配置（アドレス低位→高位） */
    memcpy(bytes, &value, sizeof(value));
    print_memory_layout("Memory Layout:", bytes, sizeof(bytes));

    /* バイト列の表示 */
    print_bytes("Raw Bytes:   ", bytes, sizeof(bytes));

    /* エンディアン判定 */
    uint32_t test32 = 0x01020304;
    uint8_t *p = (uint8_t *)&test32;
    int is_little_endian = (p[0] == 0x04);

    printf("\n");
    if (is_little_endian) {
        printf(">>> This system is LITTLE-ENDIAN <<<\n");
        printf("    Least-significant byte stored at lowest address.\n");
    } else {
        printf(">>> This system is BIG-ENDIAN <<<\n");
        printf("    Most-significant byte stored at lowest address.\n");
    }

    /* ビッグエンディアンでのレイアウト（シミュレーション） */
    printf("\n");
    printf("[Simulated BIG-ENDIAN Layout]\n");
    to_big_endian_bytes(TEST_VALUE, swapped_bytes);
    print_memory_layout("Memory Layout:", swapped_bytes, sizeof(swapped_bytes));
    print_bytes("Raw Bytes:    ", swapped_bytes, sizeof(swapped_bytes));

    /* 各バイトの解説 */
    printf("\n");
    printf("========================================================\n");
    printf(" Byte-by-Byte Explanation\n");
    printf("========================================================\n");
    printf(" Value: 0x%016" PRIx64 "\n", (uint64_t)TEST_VALUE);
    printf("         +--+--+--+--+--+--+--+--+\n");
    printf("         |%02llX|%02llX|%02llX|%02llX|%02llX|%02llX|%02llX|%02llX|\n",
            ((uint64_t)TEST_VALUE >> 56) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 48) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 40) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 32) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 24) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 16) & 0xFFULL,
            ((uint64_t)TEST_VALUE >> 8)  & 0xFFULL,
           (uint64_t)TEST_VALUE          & 0xFFULL);
    printf("         +--+--+--+--+--+--+--+--+\n");
    printf("         B7  B6  B5  B4  B3  B2  B1  B0\n");
    printf("        (MSB)                       (LSB)\n");

    printf("\n");
    printf("========================================================\n");
    printf(" Memory Layout Comparison\n");
    printf("========================================================\n");
    printf(" Address   ^\n");
    printf("           |\n");

    if (is_little_endian) {
        printf("  LE:      | 0x45 0x67 0x89 0xAB 0xCD 0xEF 0x01 0x23\n");
        printf("           | (LSB)                      (MSB)\n");
        printf("           |\n");
        printf("  BE:      | 0x01 0x23 0x45 0x67 0x89 0xAB 0xCD 0xEF\n");
        printf("           | (MSB)                      (LSB)\n");
    } else {
        printf("  LE:      | 0xEF 0xCD 0xAB 0x89 0x67 0x45 0x23 0x01\n");
        printf("           | (LSB)                      (MSB)\n");
        printf("           |\n");
        printf("  BE:      | 0x01 0x23 0x45 0x67 0x89 0xAB 0xCD 0xEF\n");
        printf("           | (MSB)                      (LSB)\n");
    }
    printf("           v\n");

    /* コンパイラ/プラットフォーム情報 */
    printf("\n");
    printf("========================================================\n");
    printf(" Compiler / Platform Info\n");
    printf("========================================================\n");
#ifdef __BYTE_ORDER__
    printf("__BYTE_ORDER__      = %s\n", endian_str(__BYTE_ORDER__));
#endif
    printf("sizeof(void*)       = %zu\n", sizeof(void *));
    printf("Compiler: ");
#ifdef __clang__
    printf("Clang\n");
#elif defined(__GNUC__)
    printf("GCC\n");
#elif defined(_MSC_VER)
    printf("MSVC\n");
#else
    printf("Unknown\n");
#endif

    printf("\n");
    printf("========================================================\n");
    printf(" Practical Implications\n");
    printf("========================================================\n");
    printf(" 1. Network protocol: Use big-endian (network byte order)\n");
    printf("    - Always use htobe64()/be64toh() or ntoh64()/hton64()\n");
    printf(" 2. File formats: Check spec for endianness\n");
    printf("    - PNG, JPEG: Big-endian\n");
    printf("    - x86 binary: Little-endian\n");
    printf(" 3. Cross-platform serialization:\n");
    printf("    - Never memcpy(uint64_t) across different endian systems\n");
    printf("    - Use explicit conversion or a serialization library\n");
    printf(" 4. Memory-mapped I/O:\n");
    printf("    - Hardware registers may expect specific byte order\n");
    printf("    - Always use explicit byte extraction for register access\n");
    printf("\n");

    return 0;
}
