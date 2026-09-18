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

    union {
        uint8_t u8[8];
        uint16_t u16[4];
        uint32_t u32[2];
        uint64_t u64;
    } uni64;
    uni64.u64 = *(uint64_t*)(uint8_t[]){0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
    for(int i=0;i<8;i++)
    printf(" u8[%d]=0x%02X\n",i,uni64.u8[i]);
    for(int i=0;i<4;i++)
    printf("u16[%d]=0x%02X\n",i,uni64.u16[i]);
    for(int i=0;i<2;i++)
    printf("u32[%d]=0x%02X\n",i,uni64.u32[i]);
    for(int i=0;i<1;i++)
    printf("u64[%d]=0x%02lX\n",i,uni64.u64   );

    return 0;
}
