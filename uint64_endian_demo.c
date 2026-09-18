/*
 * uint64_endian_demo.c
 *
 * システムのエンディアン（リトル/ビッグ）を判定するデモ。
 * 32bit 値のバイト配置から LSB が低位アドレスに来るかを確認する。
 */

#include <stdio.h>
#include <stdint.h>

int main(void)
{
    uint32_t test32 = 0x01020304;
    uint8_t *p = (uint8_t *)&test32;
    int is_little_endian = (p[0] == 0x04);

    printf("========================================================\n");
    printf(" Endian Detection Demo\n");
    printf("========================================================\n\n");

    printf("Test value: 0x%08X\n", (unsigned)test32);
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
