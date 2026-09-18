# man ページ → 各アーキ実行 デモ — 作業引継ぎ

**作成日**: 2026-09-18
**目的**: Linux man ページの EXAMPLES セクションにあるコードを、
エンディアンの異なる 4 アーキテクチャでビルド・実行し、
`htole32` / `htobe32` の変換結果が期待通りに反転することを確認する。

---

## 1. ファイル構成

```
proj040/
├── uint64_endian_demo.c             # デモ本体
├── endian_demo_handoff.md           # このファイル
├── run_demo.sh                      # 自動化スクリプト（任意）
└── .github/workflows/endian-demo.yml # GH Actions ワークフロー
```

---

## 2. 自動化スクリプト `run_demo.sh`

```bash
#!/bin/bash
#
# uint64_endian_demo のコード修正 → GH Actions 実行 → ログ確認 を自動化
#
set -euo pipefail

REPO_DIR="$(dirname "$0")"
WORKFLOW="endian-demo.yml"

# 1. Pull latest
cd "$REPO_DIR"
git pull --rebase origin main

# 2. コード修正（このスクリプトに埋め込んだバージョンに上書き）
# ※ 手動でコードを編集した場合はこのステップをコメントアウト
cat > uint64_endian_demo.c << 'EOF'
#define _DEFAULT_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    union {
        uint32_t u32;
        uint8_t arr[4];
     } x;

    x.arr[0] = 0x11;       /* Lowest-address byte */
    x.arr[1] = 0x22;
    x.arr[2] = 0x33;
    x.arr[3] = 0x44;       /* Highest-address byte */

    printf("x.u32 = %#x\n", x.u32);
    printf("htole32(x.u32) = %#x\n", htole32(x.u32));
    printf("htobe32(x.u32) = %#x\n", htobe32(x.u32));

    exit(EXIT_SUCCESS);
}
EOF

# 3. ローカルビルド＆実行
gcc -std=c11 -pedantic -Wall -Wextra -O2 uint64_endian_demo.c -o uint64_endian_demo
./uint64_endian_demo

# 4. git commit & push
git add uint64_endian_demo.c
if ! git diff --cached --quiet; then
    git commit -m "Update endian demo source"
fi
git push

# 5. GH Actions ワークフローをトリガー
gh workflow run "$WORKFLOW" --ref main
sleep 5
RUN_ID=$(gh run list --limit 1 --json databaseId --jq '.[0].databaseId')
echo "Run ID: $RUN_ID"

# 6. 完了待ち（最大5分）
MAX_WAIT=300
ELAPSED=0
while [ $ELAPSED -lt $MAX_WAIT ]; do
    STATUS=$(gh run view "$RUN_ID" --json status --jq '.status' | tr -d '"')
    if [ "$STATUS" = "completed" ]; then
        break
    fi
    sleep 15
    ELAPSED=$((ELAPSED + 15))
    CONCLUSION=$(gh run view "$RUN_ID" --json conclusion --jq '.conclusion' | tr -d '"')
    echo "   ... (${ELAPSED}s) status=$STATUS conclusion=${CONCLUSION:-in_progress}"
done

if [ $ELAPSED -ge $MAX_WAIT ]; then
    echo "ERROR: Workflow did not complete within ${MAX_WAIT}s"
    exit 1
fi

echo "Done! conclusion=$(gh run view "$RUN_ID" --json conclusion --jq '.conclusion' | tr -d '"')"

# 7. 各ジョブのログを取得＆表示
JOBS_RAW=$(gh run view "$RUN_ID" --json jobs --jq '.jobs')
echo "$JOBS_RAW" | jq -c '.[]' | while read -r JOB; do
    JOB_ID=$(echo "$JOB" | jq -r '.databaseId')
    JOB_NAME=$(echo "$JOB" | jq -r '.name')

    echo "--- $JOB_NAME ---"
    gh api repos/kanedayo/uint64_endian_demo/actions/jobs/"$JOB_ID"/logs 2>&1 \
        | sed 's/^[0-9T:.Z-]*Z //' \
        | grep -E "^(x\.u32|htole32|htobe32)" || echo "(no output found)"
done
```

---

## 3. 作業フロー

### 手順

```
1. man ページからコードを拾ってくる
   → man <コマンド> | grep -A100 "EXAMPLES"
   → コード部分を uint64_endian_demo.c にコピー

2. 必要に応じて修正
   → _DEFAULT_SOURCE 等の feature test macro 追加
   → #include の調整

3. git commit & push
   → git add uint64_endian_demo.c && git commit -m "..." && git push

4. GH Actions ワークフローをトリガー
   → gh workflow run endian-demo.yml --ref main

5. 完了待ち + ログ確認
   → gh run view <RUN_ID> --json jobs
   → gh api repos/<user>/<repo>/actions/jobs/<JOB_ID>/logs
```

### run_demo.sh を使わない手動手順

```bash
# 1. デモコードを uint64_endian_demo.c に書く
#    （man ページの EXAMPLES セクションをコピー）

# 2. commit & push
git add uint64_endian_demo.c
git commit -m "Update endian demo source"
git push

# 3. GH Actions トリガー
gh workflow run endian-demo.yml --ref main

# 4. 完了待ち
RUN_ID=$(gh run list --limit 1 --json databaseId --jq '.[0].databaseId')
sleep 60   # 手動で待つ

# 5. ログ確認（全アーキ）
JOBS_RAW=$(gh run view "$RUN_ID" --json jobs --jq '.jobs')
echo "$JOBS_RAW" | jq -c '.[]' | while read -r JOB; do
    JOB_ID=$(echo "$JOB" | jq -r '.databaseId')
    JOB_NAME=$(echo "$JOB" | jq -r '.name')
    echo "--- $JOB_NAME ---"
    gh api repos/kanedayo/uint64_endian_demo/actions/jobs/"$JOB_ID"/logs 2>&1 \
        | sed 's/^[0-9T:.Z-]*Z //' \
        | grep -E "^(x\.u32|htole32|htobe32)"
done
```

---

## 4. 期待される実行結果（man endian(3) の場合）

```
x.u32 = 0x44332211
htole32(x.u32) = 0x44332211
htobe32(x.u32) = 0x11223344
```

| アーキ | x.u32 | htole32 | htobe32 |
|--------|-------|---------|---------|
| **x86_64** (LE) | `0x44332211` | `0x44332211` | `0x11223344` |
| **AArch64** (LE) | `0x44332211` | `0x44332211` | `0x11223344` |
| **PowerPC64** (BE) | `0x11223344` | `0x44332211` | `0x11223344` |
| **s390x** (BE) | `0x11223344` | `0x44332211` | `0x11223344` |

**確認ポイント**:
- リトルエンディアン（x86_64 / AArch64）: `htole32` はそのまま、`htobe32` で逆順
- ビッグエンディアン（PowerPC64 / s390x）: `htobe32` はそのまま、`htole32` で逆順
- 各アーキで期待通りの変換が行われていることを確認

---

## 5. GitHub Actions ワークフロー `.github/workflows/endian-demo.yml`

```yaml
name: uint64_t Endian Layout Demo

on:
  push:
    paths:
       - 'uint64_endian_demo.c'
       - '.github/workflows/endian-demo.yml'
  workflow_dispatch:

jobs:
  endian-test:
    runs-on: ubuntu-24.04
    strategy:
      fail-fast: false
      matrix:
        target:
           - name: "x86_64 native"
            arch: x86_64
            cc: gcc
            qemu: ""
            endian: "Little-Endian"
            triplet: x86_64-linux-gnu
            apt_pkg: ""

           - name: "AArch64 (native)"
            arch: aarch64
            cc: aarch64-linux-gnu-gcc
            qemu: qemu-aarch64-static
            endian: "Little-Endian"
            triplet: aarch64-linux-gnu
            apt_pkg: gcc-aarch64-linux-gnu

           - name: "PowerPC64 (BE)"
            arch: ppc64
            cc: powerpc64-linux-gnu-gcc
            qemu: qemu-ppc64-static
            endian: "Big-Endian"
            triplet: powerpc64-linux-gnu
            apt_pkg: gcc-powerpc64-linux-gnu

           - name: "IBM Z (s390x)"
            arch: s390x
            cc: s390x-linux-gnu-gcc
            qemu: qemu-s390x-static
            endian: "Big-Endian"
            triplet: s390x-linux-gnu
            apt_pkg: gcc-s390x-linux-gnu

    steps:
       - name: Checkout code
        uses: actions/checkout@v4

       - name: Install toolchain
        run: |
          sudo apt-get update
          sudo apt-get install -y qemu-user-static binutils
          if [ -n "${{ matrix.target.apt_pkg }}" ]; then
            sudo apt-get install -y "${{ matrix.target.apt_pkg }}"
          fi

       - name: Show compiler info
        run: |
          echo "=== Target: ${{ matrix.target.name }} ==="
           ${{ matrix.target.cc }} --version | head -n 1
           ${{ matrix.target.cc }} -dumpmachine

       - name: Compile
        run: |
           ${{ matrix.target.cc }} \
             -std=c11 -pedantic -Wall -Wextra -O2 \
            uint64_endian_demo.c \
             -o demo_${{ matrix.target.arch }}

          echo "Binary:"
          file demo_${{ matrix.target.arch }}

       - name: Extract raw bytes
        run: |
          echo "--- Hex dump (first 64 bytes of binary) ---"
          objdump -s -j .rodata demo_${{ matrix.target.arch }} 2>/dev/null \
             | head -n 40

       - name: Run (native)
        if: matrix.target.qemu == ''
        run: |
          echo "======================================================"
          echo "   ${{ matrix.target.name }} — ${{ matrix.target.endian }}"
          echo "======================================================"
           ./demo_${{ matrix.target.arch }}

       - name: Run (QEMU user-mode)
        if: matrix.target.qemu != ''
        run: |
          echo "======================================================"
          echo "   ${{ matrix.target.name }} — ${{ matrix.target.endian }}"
          echo "======================================================"
           ${{ matrix.target.qemu }} \
             -L /usr/${{ matrix.target.triplet }} \
             ./demo_${{ matrix.target.arch }}
```

---

## 6. 関連知識メモ

- x86_64 / ARM64 / RISC-V = Little-Endian
- PowerPC (ppc64) / IBM Z (s390x) / SPARC = Big-Endian
  - 注: `ppc64le` は Little-Endian 版。ワークフローでは **`ppc64`（BE）** を使用
- ネットワークバイトオーダー = Big-Endian（`htobe64` / `be64toh` 等を使うこと）
- `man endian(3)` の関数を使うには `#define _DEFAULT_SOURCE` が必要（glibc）
