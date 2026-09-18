/*
 * uint64_endian_demo.c
 *
 * システムのエンディアン（リトル/ビッグ）を判定するデモ。
 * uint64_t 値のバイト配置から LSB が低位アドレスに来るかを確認する。
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void)
{
    uint64_t test64 = 0x0123456789ABCDEFULL;
    uint8_t *p = (uint8_t *)&test64;
    int is_little_endian = (p[0] == 0xEF);

    printf("========================================================\n");
    printf(" uint64_t Endian Detection Demo\n");
    printf("========================================================\n\n");

    printf("Test value: 0x%016" PRIx64 "\n", (uint64_t)test64);
    printf("Byte at lowest address: 0x%02X\n\n", (unsigned)p[0]);

    if (is_little_endian) {
        printf(">>> This system is LITTLE-ENDIAN <<<\n");
        printf("    Least-significant byte stored at lowest address.\n");
    } else {
        printf(">>> This system is BIG-ENDIAN <<<\n");
        printf("    Most-significant byte stored at lowest address.\n");
    }

    return 0;
}
