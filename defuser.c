#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t first = (low - high) * 5u + 0x3eu;
    uint32_t second = high * 4u + (middle ^ 0x1e6cu);
    values[0] = first ^ ((second + low * 4u) ^ 0xa2f6u);
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = 4u + 2u * (uint32_t)((challenge >> 25) & 15u);
    for (unsigned i = 1; i < count; ++i) {
        uint32_t part = (uint32_t)((challenge >> (i + 2)) & 63u);
        values[i] = 3u * (values[i - 1] + i) - part;
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = (uint32_t)(challenge & 7u);
    switch (values[0]) {
        case 0: values[1] = 5u * x + 0x6cbu; break;
        case 1: values[1] = 5u * x + 0xd730u; break;
        case 2: values[1] = 5u * x + 0x36984u; break;
        case 3: values[1] = (x ^ 0x68c9u) + (x >> 1); break;
        case 4: values[1] = (2u * x) ^ 0x4d2bu; break;
        case 5: values[1] = 5u * x + 0x18b5fu; break;
        case 6: values[1] = (x ^ 0x9f94u) + (x >> 2); break;
        case 7: values[1] = (2u * x) ^ 0x61f2u; break;
    }
    return write_answer(out, cap, values, count);
}

static uint32_t walk(uint32_t target, uint32_t low, uint32_t high, uint32_t bias) {
    uint32_t middle = low + ((high - low) >> 1);
    if (target == middle) return (middle + 0x6eu) ^ bias;
    if (target < middle)
        return (3u * walk(target, low, middle - 1u, bias)) ^ (middle + bias) ^ 5u;
    return (3u * walk(target, middle + 1u, high, bias)) ^ (middle + bias) ^ 0x10u;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t target = (uint32_t)(challenge & 127u);
    uint32_t bias = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = walk(target, 0, 127, bias);
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    /* 原节点的 next 指针用表内下标表示，数值和 tag 保持不变。 */
    static const struct {
        unsigned next;
        uint32_t value;
        uint32_t tag;
    } nodes[] = {
        {4,  0x64a, 0xe939}, {7,  0x966, 0x861d},
        {11, 0x242, 0x1401}, {2,  0x83b, 0x3efe},
        {6,  0x134, 0x48e0}, {10, 0xa4d, 0x86f5},
        {5,  0xb78, 0xe54b}, {3,  0x42a, 0x912a},
        {1,  0x32d, 0x2a7d}, {8,  0x064, 0x399f},
        {12, 0x506, 0x9d61}, {0,  0x773, 0x4e32}
    };
    static const unsigned heads[] = {9, 8, 1, 7, 3, 2, 11, 0};
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    unsigned node = heads[challenge & 7u];
    unsigned visits = (unsigned)((challenge >> 3) & 3u) + 1u;
    uint32_t sum = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0xf72fu;
    for (unsigned i = 0; i < visits; ++i) {
        sum = (sum ^ nodes[node].tag) + nodes[node].value;
        values[1] = nodes[node].value;
        node = nodes[node].next;
    }
    values[0] = sum;
    return write_answer(out, cap, values, count);
}

static uint32_t transform(unsigned slot, uint32_t x) {
    switch (slot) {
        case 0: return (x + 0xe1d5u) ^ (x >> 3);
        case 1: return (x ^ 0x581au) + 4u * x;
        case 2:
        case 3: return ((x << 5) + 0x9a0du) ^ (3u * x);
        case 4:
        case 7: return ((x << 3) | (x >> 29)) ^ 0xe606u;
        case 5: return (x ^ (x >> 3)) + 0x4504u;
        case 6: return (x >> 2) | (x << 30);
        default: return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    unsigned first = (unsigned)(challenge & 7u);
    unsigned second = (unsigned)((challenge >> 3) & 7u);
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    x = transform(first, x) ^ 0xef9fu;
    values[0] = transform(second, x) + 0xd49au;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t kind = (uint32_t)(context & 3u);
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t x = a ^ b;
    uint32_t middle = ((x >> 3) | (x << 29)) ^ 0xed8fu;
    middle ^= kind * 0x5d59u ^ (uint32_t)(context >> 16);
    uint32_t last = middle ^ (b + kind + 0xb0d6u + (a >> 3));
    /* scratch 占 32 字节，后面依次是 OPEN 标记和三个整数。 */
    int size = snprintf(out, cap, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32
                        " %" PRIu32 " %" PRIu32, kind, middle, last);
    return size < 0 || (size_t)size >= cap;
}
