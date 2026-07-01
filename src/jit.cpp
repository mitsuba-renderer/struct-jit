#include <struct-jit/struct-jit.h>
#include "transfer.h"
#include "dither.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>
#if defined(_MSC_VER)
// C4324: robin_map pads buckets to honor an alignment specifier (benign).
#  pragma warning(push)
#  pragma warning(disable: 4324)
#endif
#include <tsl/robin_map.h>
#if defined(_MSC_VER)
#  pragma warning(pop)
#endif
#include "gamma_coeffs.h"

#if defined(__x86_64__) || defined(_M_X64)
#  define SJIT_ARCH_X64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#  define SJIT_ARCH_AARCH64 1
#endif

#if defined(SJIT_ARCH_X64) || defined(SJIT_ARCH_AARCH64)
#  define SJIT_HAS_JIT 1
#else
#  define SJIT_HAS_JIT 0
#endif

#if SJIT_HAS_JIT
#  if defined(_WIN32)
#    include <windows.h>
#  else
#    include <sys/mman.h>
#    include <errno.h>
#  endif
#  if defined(SJIT_ARCH_X64)
#    include "asm_x64.h"
#  elif defined(SJIT_ARCH_AARCH64)
#    include "asm_aarch64.h"
#    if defined(__APPLE__)
#      include <pthread.h>
#      include <libkern/OSCacheControl.h>
#    endif
#  endif
#endif

NAMESPACE_BEGIN(struct_jit)

#if !SJIT_HAS_JIT

void Converter::create_kernel() {
    // No JIT backend on this architecture; convert() falls back to the
    // software interpreter when m_kernel is null.
    m_kernel = nullptr;
    m_kernel_size = 0;
}

void Converter::release_kernel() { }

#else

// Snippets emitted by asm/asm_x64.S encode patch sites with this 32-bit
// placeholder so the kernel builder can locate and overwrite them.
#if defined(SJIT_ARCH_X64)
static constexpr uint32_t kSentinelX64 = 0x12345678u;
#endif

// AArch64 patch sentinels stay in sync with asm/asm_aarch64.S: imm12 sites
// use a 0xFFF placeholder, branches use a full b.ne encoding, and scale_fX
// records literal-load fixups that are resolved after the tail pool is placed.
#if defined(SJIT_ARCH_AARCH64)
static constexpr uint32_t kSentinelImm12 = 0xFFFu;
static constexpr uint32_t kSentinelImm9  = 0xFFu;
static constexpr uint32_t kSentinelBNe   = 0x54FFFFE1u;

/// Splice a 19-bit signed PC-relative offset into the imm19 field (bits [23:5])
/// of a b.cond or LDR-literal instruction, leaving the rest of the encoding
/// intact. Shared by every imm19 fixup (branches and literal-pool loads).
static inline uint32_t set_imm19(uint32_t insn, int64_t imm19) {
    return (insn & ~(uint32_t(0x7FFFFu) << 5)) | ((uint32_t(imm19) & 0x7FFFFu) << 5);
}

/// Splice a 21-bit signed PC-relative byte offset into an ADR instruction's
/// split immediate: immlo in bits [30:29], immhi in bits [23:5]. Points a gamma
/// snippet's table-base register (x7) at the shared constant pool.
static inline uint32_t set_adr_imm(uint32_t insn, int64_t delta) {
    uint32_t immlo = (uint32_t) (delta & 0x3),
             immhi = (uint32_t) ((delta >> 2) & 0x7FFFFu);
    insn &= ~((uint32_t(0x3u) << 29) | (uint32_t(0x7FFFFu) << 5));
    return insn | (immlo << 29) | (immhi << 5);
}
#endif

inline bool is_branch_impl(impl i) {
    return i == impl::main_suffix_inner || i == impl::main_suffix_outer;
}

#if defined(SJIT_ARCH_X64)
// These snippets treat the payload as a kernel-buffer target offset; the
// patcher converts it into the final RIP-relative displacement.
inline bool is_pc_relative_impl(impl i) {
    return is_branch_impl(i) ||
           i == impl::scale_f4 || i == impl::scale_f8 ||
           i == impl::min_f4   || i == impl::min_f8   ||
           i == impl::max_f4   || i == impl::max_f8   ||
           i == impl::load_imm || i == impl::load_imm_chk ||
           i == impl::chk_ne   ||
           i == impl::gamma_decode_f4 || i == impl::gamma_encode_f4 ||
           i == impl::gamma_decode_f8 || i == impl::gamma_encode_f8;
}
#endif

// Pick the integer-to-float snippet that matches both the source integer type
// and the shared working precision selected by the Transfer recipe. These
// snippets share the same `impl` names on both backends, so this mapping is
// architecture-independent.
static impl int_to_float_cvt(Type src, Type working) {
    if (working == Type::Float32) {
        switch (src) {
            case Type::Int8:   return impl::cvt_i1_f4;
            case Type::UInt8:  return impl::cvt_u1_f4;
            case Type::Int16:  return impl::cvt_i2_f4;
            case Type::UInt16: return impl::cvt_u2_f4;
            case Type::Int32:  return impl::cvt_i4_f4;
            case Type::UInt32: return impl::cvt_u4_f4;
            case Type::Int64:  return impl::cvt_i8_f4;
            case Type::UInt64: return impl::cvt_u8_f4;
            default: break;
        }
    } else {
        switch (src) {
            case Type::Int8:   return impl::cvt_i1_f8;
            case Type::UInt8:  return impl::cvt_u1_f8;
            case Type::Int16:  return impl::cvt_i2_f8;
            case Type::UInt16: return impl::cvt_u2_f8;
            case Type::Int32:  return impl::cvt_i4_f8;
            case Type::UInt32: return impl::cvt_u4_f8;
            case Type::Int64:  return impl::cvt_i8_f8;
            case Type::UInt64: return impl::cvt_u8_f8;
            default: break;
        }
    }
    fprintf(stderr, "struct-jit: int_to_float_cvt: unsupported types\n");
    abort();
}

static impl float_to_int_cvt(Type working, Type dst) {
#if defined(SJIT_ARCH_X64)
    if (working == Type::Float32) {
        switch (dst) {
            case Type::UInt8:  return impl::cvt_f4_u1;
            case Type::Int8:   return impl::cvt_f4_i1;
            case Type::UInt16: return impl::cvt_f4_u2;
            case Type::Int16:  return impl::cvt_f4_i2;
            case Type::Int32:  return impl::cvt_f4_i4;
            case Type::UInt32: return impl::cvt_f4_u4;
            case Type::Int64:  return impl::cvt_f4_i8;
            case Type::UInt64: return impl::cvt_f4_u8;
            default: break;
        }
    } else {
        switch (dst) {
            case Type::UInt8:  return impl::cvt_f8_u1;
            case Type::Int8:   return impl::cvt_f8_i1;
            case Type::UInt16: return impl::cvt_f8_u2;
            case Type::Int16:  return impl::cvt_f8_i2;
            case Type::Int32:  return impl::cvt_f8_i4;
            case Type::UInt32: return impl::cvt_f8_u4;
            case Type::Int64:  return impl::cvt_f8_i8;
            case Type::UInt64: return impl::cvt_f8_u8;
            default: break;
        }
    }
#elif defined(SJIT_ARCH_AARCH64)
    bool large_gpr = type_size(dst) > 4;
    bool sign = type_is_signed_int(dst);
    if (working == Type::Float32) {
        if (large_gpr)
            return sign ? impl::cvt_f4_ix : impl::cvt_f4_ux;
        return sign ? impl::cvt_f4_iw : impl::cvt_f4_uw;
    } else {
        if (large_gpr)
            return sign ? impl::cvt_f8_ix : impl::cvt_f8_ux;
        return sign ? impl::cvt_f8_iw : impl::cvt_f8_uw;
    }
#endif
    fprintf(stderr, "struct-jit: float_to_int_cvt: unsupported types\n");
    abort();
}

static impl float_to_float_cvt(Type src, Type dst) {
    if (src == Type::Float32 && dst == Type::Float64)
        return impl::cvt_f4_f8;
    if (src == Type::Float64 && dst == Type::Float32)
        return impl::cvt_f8_f4;
    fprintf(stderr, "struct-jit: float_to_float_cvt: unsupported types\n");
    abort();
}

static impl round_cvt(Type working) {
    return working == Type::Float32 ? impl::round_f4 : impl::round_f8;
}

// Zero-extending raw integer load (by byte size) used to fetch a Check-flagged
// field for comparison. AArch64's ordinary loads already zero-extend; x64 needs
// movzx for the 1- and 2-byte cases (chk_zload_*), while its 4/8-byte loads
// zero-extend on their own.
#if defined(SJIT_ARCH_X64)
static impl chk_load_impl(size_t size) {
    switch (size) {
        case 1: return impl::chk_zload_u1;
        case 2: return impl::chk_zload_u2;
        case 4: return impl::load_u4;
        case 8: return impl::load_u8;
    }
    fprintf(stderr, "struct-jit: chk_load_impl: unsupported size\n");
    abort();
}
#endif

/// Snippet that converts the working value between sRGB and linear in place.
static impl gamma_cvt(Type working, bool to_srgb) {
    if (working == Type::Float32)
        return to_srgb ? impl::gamma_encode_f4 : impl::gamma_decode_f4;
    return to_srgb ? impl::gamma_encode_f8 : impl::gamma_decode_f8;
}

void Converter::create_kernel() {
    std::vector<uint8_t> kernel;
    std::vector<uint8_t> literal_pool;

#if defined(SJIT_ARCH_X64)
    struct LiteralFixup {
        impl which;
        size_t kernel_offset;
        size_t pool_offset;
    };
    std::vector<LiteralFixup> literal_fixups;
#elif defined(SJIT_ARCH_AARCH64)
    struct LdrFixup {
        size_t kernel_offset;
        size_t pool_offset;
    };
    std::vector<LdrFixup> ldr_fixups;
    // ADR fixups (gamma table base): same payload as LdrFixup, but the patched
    // instruction is an ADR (21-bit split immediate) rather than an LDR-literal.
    std::vector<LdrFixup> adr_fixups;
#endif

    // Kernel offsets of chk_ne snippets whose forward branch to the failure
    // epilogue is resolved once that epilogue's position is known.
    std::vector<size_t> fail_fixups;

    // Every constant the kernel reads at runtime -- normalization factors, clamp
    // bounds, default values, and each individual sRGB polynomial coefficient --
    // is a 4- or 8-byte scalar in a single tail pool, deduplicated by (width, raw
    // bits) so identical values are stored once. Multiple fixups may resolve to
    // the same offset, which is fine.
    struct PoolKeyHash {
        size_t operator()(const std::pair<size_t, uint64_t> &k) const {
            return (size_t) (k.second * 0x9E3779B97F4A7C15ull + k.first);
        }
    };
    tsl::robin_map<std::pair<size_t, uint64_t>, size_t, PoolKeyHash> pool_dedup;

    auto append_bits = [&](size_t size, uint64_t bits) -> size_t {
        auto key = std::make_pair(size, bits);
        auto it = pool_dedup.find(key);
        if (it != pool_dedup.end())
            return it->second;

        while (literal_pool.size() % size != 0)
            literal_pool.push_back(0);
        size_t pool_offset = literal_pool.size();
        const uint8_t *p = (const uint8_t *) &bits; // low `size` bytes (LE)
        literal_pool.insert(literal_pool.end(), p, p + size);
        pool_dedup.emplace(key, pool_offset);
        return pool_offset;
    };

    // Append a single floating point constant in the working precision.
    auto append_literal = [&](Type working, double value) -> size_t {
        if (working == Type::Float32) {
            float v = (float) value;
            uint32_t bits;
            memcpy(&bits, &v, 4);
            return append_bits(4, bits);
        } else {
            uint64_t bits;
            memcpy(&bits, &value, 8);
            return append_bits(8, bits);
        }
    };

#if defined(SJIT_ARCH_X64)
    auto patch_x64 = [&](impl which, size_t kernel_offset, uint32_t payload) {
        SnippetSpan snippet = get_snippet(which);
        uint8_t *base = kernel.data() + kernel_offset;
        int matches = 0;
        for (size_t off = 0; off + 4 <= snippet.size; ++off) {
            if (memcmp(base + off, &kSentinelX64, 4) != 0)
                continue;

            // The code generator speaks in kernel-buffer offsets; x64 branch
            // and RIP-relative snippets need those translated at patch time. A
            // rel32 displacement is relative to the end of *its own* instruction
            // -- the four sentinel bytes at `off` -- so the reference point is
            // `off + 4`. That equals snippet.size for every single-instruction
            // pc-relative snippet, and is earlier for gamma's leading `lea`.
            uint32_t patched = payload;
            if (is_pc_relative_impl(which))
                patched -= (uint32_t) (kernel_offset + off + 4);
            memcpy(base + off, &patched, 4);
            matches++;
        }

        if (matches != 1) {
            fprintf(stderr, "struct-jit: expected exactly one sentinel in impl "
                            "%u, found %d\n", (unsigned) which, matches);
            abort();
        }
    };
#endif

#if defined(SJIT_ARCH_AARCH64)
    // Splice the single b.cond sentinel within [base, base + size) so it branches
    // to the kernel-buffer byte offset \c target, returning the number of
    // sentinels patched (expected to be exactly one). Shared by the generic
    // patcher and the deferred check-failure fixups.
    auto patch_bcond = [&](size_t base, size_t size, size_t target) -> int {
        int matches = 0;
        for (size_t off = 0; off + 4 <= size; off += 4) {
            uint32_t insn;
            memcpy(&insn, kernel.data() + base + off, 4);
            if (insn != kSentinelBNe)
                continue;

            size_t bcc_pc = base + off;
            int64_t delta = (int64_t) target - (int64_t) bcc_pc;
            int64_t imm19 = delta / 4;
            if ((delta & 3) != 0 ||
                imm19 < -(1LL << 18) || imm19 >= (1LL << 18))
                raise("Converter::create_kernel(): AArch64 conditional branch "
                      "target is out of range!");

            uint32_t new_insn = set_imm19(insn, imm19);
            memcpy(kernel.data() + base + off, &new_insn, 4);
            matches++;
        }
        return matches;
    };

    auto patch_aarch64 = [&](impl which, size_t kernel_offset, uint32_t payload) {
        SnippetSpan snippet = get_snippet(which);
        PatchKind kind = patch_kind(which);
        if (kind == PatchKind::None)
            raise("Converter::create_kernel(): internal error: snippet has no "
                  "AArch64 patch kind!");
        if (is_branch_impl(which) != (kind == PatchKind::Imm19_BCond))
            raise("Converter::create_kernel(): internal error: inconsistent "
                  "AArch64 branch patch kind!");

        int matches = 0;
        if (kind == PatchKind::Imm19_BCond) {
            matches = patch_bcond(kernel_offset, snippet.size, payload);
        } else if (kind == PatchKind::Imm9_Unscaled) {
            if (payload > 255u)
                raise("Converter::create_kernel(): AArch64 unscaled memory "
                      "offset is out of range!");

            uint32_t imm9_mask = uint32_t(0x1FFu) << 12,
                     sentinel = kSentinelImm9 << 12,
                     imm9_value = payload;
            for (size_t off = 0; off + 4 <= snippet.size; off += 4) {
                uint32_t insn;
                memcpy(&insn, kernel.data() + kernel_offset + off, 4);
                if ((insn & imm9_mask) != sentinel)
                    continue;

                uint32_t new_insn = (insn & ~imm9_mask) | (imm9_value << 12);
                memcpy(kernel.data() + kernel_offset + off, &new_insn, 4);
                matches++;
            }
        } else {
            // For load/store/add snippets the payload is a byte offset; the
            // patch kind defines how that value is scaled into imm12 bits.
            uint32_t imm12_value = 0;
            switch (kind) {
                case PatchKind::Imm12_S1:
                    if (payload >= 4096u)
                        raise("Converter::create_kernel(): AArch64 memory "
                              "offset is out of range!");
                    imm12_value = payload;
                    break;
                case PatchKind::Imm12_S2:
                    if ((payload & 1u) != 0 || (payload >> 1) >= 4096u)
                        raise("Converter::create_kernel(): AArch64 scaled "
                              "memory offset is not encodable!");
                    imm12_value = payload >> 1;
                    break;
                case PatchKind::Imm12_S4:
                    if ((payload & 3u) != 0 || (payload >> 2) >= 4096u)
                        raise("Converter::create_kernel(): AArch64 scaled "
                              "memory offset is not encodable!");
                    imm12_value = payload >> 2;
                    break;
                case PatchKind::Imm12_S8:
                    if ((payload & 7u) != 0 || (payload >> 3) >= 4096u)
                        raise("Converter::create_kernel(): AArch64 scaled "
                              "memory offset is not encodable!");
                    imm12_value = payload >> 3;
                    break;
                case PatchKind::Imm12_Add:
                    if (payload >= 4096u)
                        raise("Converter::create_kernel(): AArch64 record "
                              "stride is out of range!");
                    imm12_value = payload;
                    break;
                default:
                    abort();
            }

            uint32_t slot_mask = kSentinelImm12 << 10;
            for (size_t off = 0; off + 4 <= snippet.size; off += 4) {
                uint32_t insn;
                memcpy(&insn, kernel.data() + kernel_offset + off, 4);
                if ((insn & slot_mask) != slot_mask)
                    continue;

                uint32_t new_insn = (insn & ~slot_mask) | (imm12_value << 10);
                memcpy(kernel.data() + kernel_offset + off, &new_insn, 4);
                matches++;
            }
        }

        if (matches != 1) {
            fprintf(stderr, "struct-jit: expected exactly one sentinel in impl "
                            "%u, found %d\n", (unsigned) which, matches);
            abort();
        }
    };
#endif

    auto put = [&](impl which, uint32_t payload = 0xffffffffu) {
        SnippetSpan snippet = get_snippet(which);
        size_t kernel_offset = kernel.size();
        kernel.insert(kernel.end(), snippet.data, snippet.data + snippet.size);

        if (payload == 0xffffffffu)
            return;

#if defined(SJIT_ARCH_X64)
        patch_x64(which, kernel_offset, payload);
#elif defined(SJIT_ARCH_AARCH64)
        patch_aarch64(which, kernel_offset, payload);
#endif
    };

    // Emit the f4 or f8 variant of a snippet according to the working precision.
    auto put_fp = [&](impl f4, impl f8) {
        put(m_working == Type::Float32 ? f4 : f8);
    };

    // Append a raw 8-byte (8-aligned) value to the tail pool and return its byte
    // offset (deduplicated). Used for non-float pool entries (default bits, the
    // dither table pointer).
    auto append_pool_u64 = [&](uint64_t bits) -> size_t {
        return append_bits(8, bits);
    };

#if defined(SJIT_ARCH_AARCH64)
    // Queue a fixup for the lone LDR-literal placeholder (imm19 == 0x7FFFF) in
    // the snippet just emitted at [kernel_offset, +size); its imm19 is patched
    // to reach pool_offset once the literal pool is placed. The opcode mask
    // matches every LDR-literal form (ldr w/x/s/d) and excludes register-offset
    // loads (e.g. the dither table read) that may also carry the imm19 bits.
    auto record_ldr_fixup = [&](size_t kernel_offset, size_t size, size_t pool_offset) {
        for (size_t off = 0; off + 4 <= size; off += 4) {
            uint32_t insn;
            memcpy(&insn, kernel.data() + kernel_offset + off, 4);
            if (((insn >> 24) & 0x3Bu) == 0x18u && ((insn >> 5) & 0x7FFFFu) == 0x7FFFFu) {
                ldr_fixups.push_back({ kernel_offset + off, pool_offset });
                return;
            }
        }
        fprintf(stderr, "struct-jit: ldr-literal sentinel not found\n");
        abort();
    };

    // Queue a fixup for the lone ADR placeholder (`adr x7, .`) in the snippet just
    // emitted; its 21-bit immediate is patched to reach pool_offset once the pool
    // is placed. Matches the ADR opcode with Rd == x7 (the gamma table base).
    auto record_adr_fixup = [&](size_t kernel_offset, size_t size, size_t pool_offset) {
        for (size_t off = 0; off + 4 <= size; off += 4) {
            uint32_t insn;
            memcpy(&insn, kernel.data() + kernel_offset + off, 4);
            if ((insn & 0x9F00001Fu) == 0x10000007u) {
                adr_fixups.push_back({ kernel_offset + off, pool_offset });
                return;
            }
        }
        fprintf(stderr, "struct-jit: adr sentinel not found\n");
        abort();
    };
#endif

    // Emit a snippet that reads a constant from the tail literal pool (scale_f*,
    // min_f*, max_f*, load_imm*) and queue the fixup that points its embedded
    // load at pool_offset: a RIP-relative displacement on x64, an LDR imm19 on
    // aarch64.
    auto emit_pool_ref = [&](impl which, size_t pool_offset) {
        SnippetSpan snippet = get_snippet(which);
        size_t kernel_offset = kernel.size();
        kernel.insert(kernel.end(), snippet.data, snippet.data + snippet.size);
#if defined(SJIT_ARCH_X64)
        literal_fixups.push_back({ which, kernel_offset, pool_offset });
#elif defined(SJIT_ARCH_AARCH64)
        record_ldr_fixup(kernel_offset, snippet.size, pool_offset);
#endif
    };

    // sRGB coefficient tables are appended contiguously (the gamma snippet reads
    // them as base + fixed offset), unlike the per-scalar pool entries. Cache the
    // offset per variant so a conversion's RGB channels share one copy.
    size_t gamma_table_off[4];
    for (size_t &o : gamma_table_off)
        o = SIZE_MAX;

    auto append_gamma_table = [&](Type working, bool to_srgb) -> size_t {
        size_t idx = (working == Type::Float64 ? 2u : 0u) + (to_srgb ? 1u : 0u);
        if (gamma_table_off[idx] != SIZE_MAX)
            return gamma_table_off[idx];

        const void *data;
        size_t bytes, align;
        if (working == Type::Float32) {
            align = 4;
            if (to_srgb) { data = gamma_encode_f4_coeffs; bytes = sizeof(gamma_encode_f4_coeffs); }
            else         { data = gamma_decode_f4_coeffs; bytes = sizeof(gamma_decode_f4_coeffs); }
        } else {
            align = 8;
            if (to_srgb) { data = gamma_encode_f8_coeffs; bytes = sizeof(gamma_encode_f8_coeffs); }
            else         { data = gamma_decode_f8_coeffs; bytes = sizeof(gamma_decode_f8_coeffs); }
        }

        while (literal_pool.size() % align != 0)
            literal_pool.push_back(0);
        size_t off = literal_pool.size();
        const uint8_t *p = (const uint8_t *) data;
        literal_pool.insert(literal_pool.end(), p, p + bytes);
        gamma_table_off[idx] = off;
        return off;
    };

    // Emit an sRGB <-> linear conversion. The monolithic snippet reads its
    // coefficients from the shared pool through a table-base register (adr x7 on
    // aarch64, lea tmp on x64); its lone base reference is fixed up to the table.
    auto emit_gamma = [&](Type working, bool to_srgb) {
        impl which = gamma_cvt(working, to_srgb);
        size_t pool_offset = append_gamma_table(working, to_srgb);
        SnippetSpan snippet = get_snippet(which);
        size_t kernel_offset = kernel.size();
        kernel.insert(kernel.end(), snippet.data, snippet.data + snippet.size);
#if defined(SJIT_ARCH_X64)
        literal_fixups.push_back({ which, kernel_offset, pool_offset });
#elif defined(SJIT_ARCH_AARCH64)
        record_adr_fixup(kernel_offset, snippet.size, pool_offset);
#endif
    };

    // Multiply/clamp the working value by a floating point constant (scale_f*,
    // min_f*, max_f*); the constant is appended in the working precision.
    auto emit_pool_op = [&](impl which, Type working, double value) {
        emit_pool_ref(which, append_literal(working, value));
    };

    auto emit_scale = [&](Type working, double value) {
        emit_pool_op(working == Type::Float32 ? impl::scale_f4 : impl::scale_f8,
                     working, value);
    };

    // Saturate the working value to the destination integer range, mirroring
    // the software fallback (round -> clamp-low -> clamp-high -> convert).
    auto emit_clamp = [&](Type working, Type dst) {
        std::pair<double, double> b = int_clamp_bounds(dst, working);
        emit_pool_op(working == Type::Float32 ? impl::max_f4 : impl::max_f8,
                     working, b.first);
        emit_pool_op(working == Type::Float32 ? impl::min_f4 : impl::min_f8,
                     working, b.second);
    };

    // Per-record dither value into the reserved register (v18 / xmm8); it depends
    // only on the pixel, so all of a record's float -> int stores share it.
    auto emit_dither_load = [&](Type working) {
        impl which = working == Type::Float32 ? impl::dither_load_f4
                                              : impl::dither_load_f8;
        SnippetSpan snippet = get_snippet(which);
        size_t kernel_offset = kernel.size();
        kernel.insert(kernel.end(), snippet.data, snippet.data + snippet.size);

        uint64_t base = (uint64_t) (uintptr_t) dither_matrix256;
#if defined(SJIT_ARCH_X64)
        // x64 bakes the table base into a movabs immediate (8-byte sentinel).
        uint64_t sentinel = 0x0123456789ABCDEFull;
        for (size_t off = 0; off + 8 <= snippet.size; ++off) {
            if (memcmp(kernel.data() + kernel_offset + off, &sentinel, 8) == 0) {
                memcpy(kernel.data() + kernel_offset + off, &base, 8);
                return;
            }
        }
        fprintf(stderr, "struct-jit: dither sentinel not found\n");
        abort();
#elif defined(SJIT_ARCH_AARCH64)
        // aarch64 loads the table base from the literal pool.
        record_ldr_fixup(kernel_offset, snippet.size, append_pool_u64(base));
#endif
    };

#if defined(SJIT_ARCH_AARCH64)
    auto aarch64_select_memory_impl = [](impl aligned, impl unaligned,
                                         size_t offset, size_t scale) {
        if (scale == 1 || offset % scale == 0)
            return aligned;
        return unaligned;
    };
#endif

#if defined(SJIT_ARCH_AARCH64)
#  define SJIT_PUT_MEM(aligned, unaligned, offset, scale) \
    put(aarch64_select_memory_impl(impl::aligned, impl::unaligned, (offset), (scale)), (uint32_t) (offset))
#else
#  define SJIT_PUT_MEM(aligned, unaligned, offset, scale) \
    put(impl::aligned, (uint32_t) (offset))
#endif

    bool bswap_in = m_source.byte_order() != native_byte_order();
    bool bswap_out = m_target.byte_order() != native_byte_order();

    auto emit_load = [&](const Field &field) {
        switch (field.type) {
            case Type::Int8:
            case Type::UInt8:
                put(impl::load_u1, (uint32_t) field.offset);
                return Type(field.type);

            case Type::Int16:
            case Type::UInt16:
                SJIT_PUT_MEM(load_u2, load_u2_unaligned, field.offset, 2);
                if (bswap_in)
                    put(impl::bswap_u2);
                return Type(field.type);

            case Type::Int32:
            case Type::UInt32:
                SJIT_PUT_MEM(load_u4, load_u4_unaligned, field.offset, 4);
                if (bswap_in)
                    put(impl::bswap_u4);
                return Type(field.type);

            case Type::Int64:
            case Type::UInt64:
                SJIT_PUT_MEM(load_u8, load_u8_unaligned, field.offset, 8);
                if (bswap_in)
                    put(impl::bswap_u8);
                return Type(field.type);

            case Type::Float16:
                if (!bswap_in) {
                    SJIT_PUT_MEM(load_f2, load_f2_unaligned, field.offset, 2);
                } else {
                    SJIT_PUT_MEM(load_u2, load_u2_unaligned, field.offset, 2);
                    put(impl::bswap_u2);
                    put(impl::mov_u2_f2);
                }
                return Type::Float32;

            case Type::Float32:
                if (!bswap_in) {
                    SJIT_PUT_MEM(load_f4, load_f4_unaligned, field.offset, 4);
                } else {
                    SJIT_PUT_MEM(load_u4, load_u4_unaligned, field.offset, 4);
                    put(impl::bswap_u4);
                    put(impl::mov_u4_f4);
                }
                return Type::Float32;

            case Type::Float64:
                if (!bswap_in) {
                    SJIT_PUT_MEM(load_f8, load_f8_unaligned, field.offset, 8);
                } else {
                    SJIT_PUT_MEM(load_u8, load_u8_unaligned, field.offset, 8);
                    put(impl::bswap_u8);
                    put(impl::mov_u8_f8);
                }
                return Type::Float64;

            default:
                fprintf(stderr, "struct-jit: unsupported input load type\n");
                abort();
        }
    };

    // Load/bitcast the value into the working float precision and apply input
    // normalization (the "linearize" step shared with the weight source).
    auto emit_to_working_float = [&](const Transfer &t, Type &type) {
        if (type_is_signed_int(type) || type_is_unsigned_int(type))
            put(int_to_float_cvt(type, t.working_type));
        else if (type != t.working_type)
            put(float_to_float_cvt(type, t.working_type));
        type = t.working_type;

        if (t.input_normalized) {
            double src_max = type_range(t.input->type).second;
            emit_scale(t.working_type, 1.0 / src_max);
        }
    };

    // Load a source field and fully linearize it to the working precision
    // (int->float with normalization, precision adjust, sRGB decode). Used for
    // the weight/alpha preamble and for each blend term; leaves the value in the
    // working register (v0 / xmm0).
    auto emit_linearize_source = [&](const Field &f) {
        Transfer t;
        t.input = &f;
        t.input_normalized = has_flag(f.flags, Flag::Normalized);
        t.working_type = m_working;
        Type type = emit_load(f);
        emit_to_working_float(t, type);
        if (has_flag(f.flags, Flag::Gamma))
            emit_gamma(m_working, /* to_srgb = */ false);
    };

    // Transform a loaded source value into the linear working value the output
    // store expects: load/bitcast to the working float, sRGB decode, weight
    // division, alpha (un)premultiply. The destination lowering (gamma encode,
    // rescale/dither/round/clamp/convert, store) is left to emit_store_working,
    // which the blend path shares.
    auto emit_value_transform = [&](const Transfer &t, Type &type) {
        emit_to_working_float(t, type);

        if (t.gamma_decode)
            emit_gamma(t.working_type, /* to_srgb = */ false);

        if (t.weight_apply)
            put_fp(impl::weight_scale_f4, impl::weight_scale_f8);

        if (t.alpha_premul)
            put_fp(impl::alpha_premul_f4, impl::alpha_premul_f8);
        if (t.alpha_unpremul)
            put_fp(impl::alpha_unpremul_f4, impl::alpha_unpremul_f8);
    };

    auto emit_store = [&](const Field &field) {
        switch (field.type) {
            case Type::Int8:
            case Type::UInt8:
                put(impl::store_u1, (uint32_t) field.offset);
                break;

            case Type::Int16:
            case Type::UInt16:
                if (bswap_out)
                    put(impl::bswap_u2);
                SJIT_PUT_MEM(store_u2, store_u2_unaligned, field.offset, 2);
                break;

            case Type::Int32:
            case Type::UInt32:
                if (bswap_out)
                    put(impl::bswap_u4);
                SJIT_PUT_MEM(store_u4, store_u4_unaligned, field.offset, 4);
                break;

            case Type::Int64:
            case Type::UInt64:
                if (bswap_out)
                    put(impl::bswap_u8);
                SJIT_PUT_MEM(store_u8, store_u8_unaligned, field.offset, 8);
                break;

            case Type::Float16:
                if (!bswap_out) {
                    SJIT_PUT_MEM(store_f2, store_f2_unaligned, field.offset, 2);
                } else {
                    put(impl::mov_f2_u2);
                    put(impl::bswap_u2);
                    SJIT_PUT_MEM(store_u2, store_u2_unaligned, field.offset, 2);
                }
                break;

            case Type::Float32:
                if (!bswap_out) {
                    SJIT_PUT_MEM(store_f4, store_f4_unaligned, field.offset, 4);
                } else {
                    put(impl::mov_f4_u4);
                    put(impl::bswap_u4);
                    SJIT_PUT_MEM(store_u4, store_u4_unaligned, field.offset, 4);
                }
                break;

            case Type::Float64:
                if (!bswap_out) {
                    SJIT_PUT_MEM(store_f8, store_f8_unaligned, field.offset, 8);
                } else {
                    put(impl::mov_f8_u8);
                    put(impl::bswap_u8);
                    SJIT_PUT_MEM(store_u8, store_u8_unaligned, field.offset, 8);
                }
                break;

            default:
                fprintf(stderr, "struct-jit: unsupported output store type\n");
                abort();
        }
    };

    // First dithered store materializes the per-record value; later ones are
    // straight-line after it, so they just add the register.
    bool dither_loaded = false;

    // Lower a working-precision value (already in v0 / xmm0) into output field
    // \c fo: optional sRGB encode, then either the float -> int path (rescale,
    // dither, round, clamp, convert) or a float precision adjust, then store.
    // This is the destination half of the value path (emit_value_transform
    // produces the linear working value); shared by the mapped-field and blend
    // stores.
    auto emit_store_working = [&](const Field &fo, bool gamma_encode) {
        if (gamma_encode)
            emit_gamma(m_working, /* to_srgb = */ true);

        if (type_is_signed_int(fo.type) || type_is_unsigned_int(fo.type)) {
            if (has_flag(fo.flags, Flag::Normalized))
                emit_scale(m_working, type_range(fo.type).second);
            if (m_dither) {
                if (!dither_loaded) {
                    emit_dither_load(m_working);
                    dither_loaded = true;
                }
                put(m_working == Type::Float32 ? impl::dither_add_f4
                                               : impl::dither_add_f8);
            }
            put(round_cvt(m_working));
            emit_clamp(m_working, fo.type);
            put(float_to_int_cvt(m_working, fo.type));
        } else {
            Type ort = register_type(fo.type);
            if (ort != m_working)
                put(float_to_float_cvt(m_working, ort));
        }
        emit_store(fo);
    };

    // Load a raw 8-byte constant (zero-extended) from the tail literal pool into
    // the snippet's target register: the value GPR for impl::load_imm (defaults)
    // or the check scratch register for impl::load_imm_chk.
    auto emit_pool_load = [&](impl which, uint64_t bits) {
        emit_pool_ref(which, append_pool_u64(bits));
    };

    // Substitute a missing input field with the output field's default value:
    // load the raw default bits into the value register and write them verbatim
    // (no numeric conversion), exactly as the software fallback does. Endianness
    // is applied to the raw value if the output's byte order differs from native.
    auto emit_default = [&](const Field &field) {
        emit_pool_load(impl::load_imm, encode_value(field));

        // The raw value lives in the integer register, so store it through the
        // integer path regardless of whether the field is float- or int-typed.
        switch (type_size(field.type)) {
            case 1:
                put(impl::store_u1, (uint32_t) field.offset);
                break;
            case 2:
                if (bswap_out) put(impl::bswap_u2);
                SJIT_PUT_MEM(store_u2, store_u2_unaligned, field.offset, 2);
                break;
            case 4:
                if (bswap_out) put(impl::bswap_u4);
                SJIT_PUT_MEM(store_u4, store_u4_unaligned, field.offset, 4);
                break;
            case 8:
                if (bswap_out) put(impl::bswap_u8);
                SJIT_PUT_MEM(store_u8, store_u8_unaligned, field.offset, 8);
                break;
            default:
                fprintf(stderr, "struct-jit: unsupported default field size\n");
                abort();
        }
    };

    // Verify a Check-flagged input field against its expected value. The raw
    // field bytes are loaded zero-extended, byte-swapped if the input is
    // big-endian, and compared against the expected bits; on mismatch the kernel
    // branches forward to the failure epilogue (resolved after it is placed).
    auto emit_check = [&](const Field &field) {
        size_t sz = type_size(field.type);
#if defined(SJIT_ARCH_AARCH64)
        switch (sz) {
            case 1: put(impl::load_u1, (uint32_t) field.offset); break;
            case 2: SJIT_PUT_MEM(load_u2, load_u2_unaligned, field.offset, 2); break;
            case 4: SJIT_PUT_MEM(load_u4, load_u4_unaligned, field.offset, 4); break;
            case 8: SJIT_PUT_MEM(load_u8, load_u8_unaligned, field.offset, 8); break;
            default:
                fprintf(stderr, "struct-jit: unsupported check field size\n");
                abort();
        }
#else
        put(chk_load_impl(sz), (uint32_t) field.offset);
#endif
        if (bswap_in && sz >= 2)
            put(sz == 2 ? impl::bswap_u2 : sz == 4 ? impl::bswap_u4
                                                   : impl::bswap_u8);

        emit_pool_load(impl::load_imm_chk, encode_value(field));

        SnippetSpan snippet = get_snippet(impl::chk_ne);
        size_t kernel_offset = kernel.size();
        kernel.insert(kernel.end(), snippet.data, snippet.data + snippet.size);
        fail_fixups.push_back(kernel_offset);
    };

    kernel.reserve(200);

    put(impl::main_prefix);
    size_t main_start_offset = kernel.size();
    put(impl::main_start);
    size_t loop_body = kernel.size();

    // Up front (per record): verify every Check-flagged input field before any
    // value is processed, independent of the conversion plan, so input-only
    // check fields are covered too.
    for (size_t i = 0; i < m_source.size(); ++i) {
        if (has_flag(m_source[i].flags, Flag::Check))
            emit_check(m_source[i]);
    }

    // Preamble (per record): linearize the source weight and stash its
    // reciprocal in the reserved register (xmm2 / v2), then likewise compute the
    // alpha and inverse alpha (xmm6/xmm7 / v16/v17). Both survive the loop body.
    if (m_weight_divide) {
        emit_linearize_source(m_source[m_weight_in]);
        put_fp(impl::weight_recip_f4, impl::weight_recip_f8);
    }
    if (m_alpha_apply) {
        emit_linearize_source(m_source[m_alpha_in]);
        put_fp(impl::alpha_recip_f4, impl::alpha_recip_f8);
    }

    for (const auto &entry : m_plan) {
        // The planner strips the conversion problem down to one transfer at a
        // time; the backend only has to lower that scalar recipe into snippets.
        Transfer t = make_transfer(m_source, m_target, entry, m_working,
                                   m_weight_divide, m_alpha_apply);

        if (!t.input) {
            // Missing input: write the output field's default value verbatim.
            emit_default(*t.output);
            continue;
        }

        Type type = emit_load(*t.input);
        if (t.needs_conversion) {
            emit_value_transform(t, type);
            emit_store_working(*t.output, t.gamma_encode);
        } else {
            // No numeric change: store the loaded value as-is.
            emit_store(*t.output);
        }
    }

    // Blended outputs: accumulate sum(weight_i * linearize(src_i)) in the blend
    // register, divide by the weight if requested, then store. Blend fields skip
    // alpha (un)premultiplication (matching the fallback).
    for (const BlendEntry &be : m_blend) {
        const Field &fo = m_target[be.output];
        for (size_t i = 0; i < be.terms.size(); ++i) {
            emit_linearize_source(m_source[be.terms[i].first]);
            emit_scale(m_working, be.terms[i].second);
            if (i == 0)
                put_fp(impl::blend_init_f4, impl::blend_init_f8);
            else
                put_fp(impl::blend_add_f4, impl::blend_add_f8);
        }
        put_fp(impl::blend_fini_f4, impl::blend_fini_f8);
        if (m_weight_divide)
            put_fp(impl::weight_scale_f4, impl::weight_scale_f8);
        emit_store_working(fo, has_flag(fo.flags, Flag::Gamma));
    }

    put(impl::main_inc_in, (uint32_t) m_source.nbytes());
    put(impl::main_inc_out, (uint32_t) m_target.nbytes());
    put(impl::main_suffix_inner, (uint32_t) loop_body);
    put(impl::main_suffix_outer, (uint32_t) main_start_offset);
    put(impl::main_postfix);

    // Failure epilogue (only when checks exist). The success path returns above
    // and never falls into it; chk_ne's forward branches are resolved here.
    if (!fail_fixups.empty()) {
        size_t fail_offset = kernel.size();
        put(impl::fail_postfix);

        for (size_t base : fail_fixups) {
#if defined(SJIT_ARCH_X64)
            // chk_ne is RIP-relative, so patch_x64 turns the kernel-buffer
            // offset into the final rel32 displacement.
            patch_x64(impl::chk_ne, base, (uint32_t) fail_offset);
#elif defined(SJIT_ARCH_AARCH64)
            // chk_ne carries a single forward b.cond to the failure epilogue.
            patch_bcond(base, get_snippet(impl::chk_ne).size, fail_offset);
#endif
        }
    }

    if (!literal_pool.empty()) {
        while (kernel.size() % 8 != 0)
            kernel.push_back(0);

        size_t pool_base = kernel.size();
        kernel.insert(kernel.end(), literal_pool.begin(), literal_pool.end());

#if defined(SJIT_ARCH_X64)
        for (const LiteralFixup &fix : literal_fixups)
            patch_x64(fix.which, fix.kernel_offset,
                      (uint32_t) (pool_base + fix.pool_offset));
#elif defined(SJIT_ARCH_AARCH64)
        for (const LdrFixup &fix : ldr_fixups) {
            int64_t target = (int64_t) (pool_base + fix.pool_offset);
            int64_t delta = target - (int64_t) fix.kernel_offset;
            int64_t imm19 = delta / 4;
            if ((delta & 3) != 0 || delta < 0 || imm19 >= (1LL << 18))
                raise("Converter::create_kernel(): AArch64 literal load target "
                      "is out of range!");

            uint32_t insn;
            memcpy(&insn, kernel.data() + fix.kernel_offset, 4);
            uint32_t new_insn = set_imm19(insn, imm19);
            memcpy(kernel.data() + fix.kernel_offset, &new_insn, 4);
        }
        // Gamma table base: an ADR computes a PC-relative byte offset (no /4
        // scaling, range +-1 MiB), spliced into the split immediate.
        for (const LdrFixup &fix : adr_fixups) {
            int64_t delta = (int64_t) (pool_base + fix.pool_offset) -
                            (int64_t) fix.kernel_offset;
            if (delta < -(1LL << 20) || delta >= (1LL << 20))
                raise("Converter::create_kernel(): AArch64 ADR target is out "
                      "of range!");
            uint32_t insn;
            memcpy(&insn, kernel.data() + fix.kernel_offset, 4);
            uint32_t new_insn = set_adr_imm(insn, delta);
            memcpy(kernel.data() + fix.kernel_offset, &new_insn, 4);
        }
#endif
    }

    m_kernel_size = kernel.size();

#if defined(_WIN32)
    void *ptr = VirtualAlloc(nullptr, m_kernel_size, MEM_RESERVE | MEM_COMMIT,
                             PAGE_READWRITE);
    if (!ptr) {
        fprintf(stderr, "struct-jit: could not VirtualAlloc() memory: %lu",
                GetLastError());
        abort();
    }
    memcpy(ptr, kernel.data(), m_kernel_size);
    DWORD unused;
    if (VirtualProtect(ptr, m_kernel_size, PAGE_EXECUTE_READ, &unused) == 0) {
        fprintf(stderr, "struct-jit: VirtualProtect() failed: %lu",
                GetLastError());
        abort();
    }
#elif defined(SJIT_ARCH_AARCH64) && defined(__APPLE__)
    void *ptr = mmap(nullptr, m_kernel_size,
                     PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_JIT, -1, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "struct-jit: could not mmap(MAP_JIT) memory: %s",
                strerror(errno));
        abort();
    }
    pthread_jit_write_protect_np(false);
    memcpy(ptr, kernel.data(), m_kernel_size);
    pthread_jit_write_protect_np(true);
    sys_icache_invalidate(ptr, m_kernel_size);
#else
    void *ptr = mmap(nullptr, m_kernel_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "struct-jit: could not mmap() memory: %s",
                strerror(errno));
        abort();
    }
    memcpy(ptr, kernel.data(), m_kernel_size);
    if (mprotect(ptr, m_kernel_size, PROT_READ | PROT_EXEC) == -1) {
        fprintf(stderr, "struct-jit: mprotect() failed: %s", strerror(errno));
        abort();
    }
#  if defined(SJIT_ARCH_AARCH64)
    __builtin___clear_cache((char *) ptr, (char *) ptr + m_kernel_size);
#  endif
#endif

    m_kernel = (Kernel *) ptr;

#undef SJIT_PUT_MEM
}

void Converter::release_kernel() {
#if !defined(_WIN32)
    if (munmap((void *) m_kernel, m_kernel_size) == -1) {
        fprintf(stderr, "struct-jit: munmap() failed!");
        abort();
    }
#else
    if (VirtualFree((void *) m_kernel, 0, MEM_RELEASE) == 0) {
        fprintf(stderr, "struct-jit: VirtualFree() failed!");
        abort();
    }
#endif
}

#endif

NAMESPACE_END(struct_jit)
