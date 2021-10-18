#include <struct-jit/struct-jit.h>

#if defined(_WIN32)
#  include <windows.h>
#  include "../asm/asm_x64_win.h"
#else
#  include <dlfcn.h>
#  include <sys/mman.h>
#  include <errno.h>
#  include "../asm/asm_x64_sysv.h"
#endif

/// Indicates a missing source/destination field
constexpr size_t None = size_t(-1);

NAMESPACE_BEGIN(struct_jit)

void Converter::create_kernel() {
    std::vector<uint8_t> kernel;

    auto put = [&kernel](impl i, uint32_t payload = 0xffffffffu,
                         bool relative = false) {
        size_t offset0 = impl_offset[size_t(i)],
               offset1 = impl_offset[size_t(i)+1],
               kernel_offset = kernel.size();

        kernel.insert(kernel.end(), impl_data + offset0, impl_data + offset1);

        if (payload != 0xffffffffu) {
            uint8_t *ptr = kernel.data() + kernel_offset;
            uint32_t dummy = 0x12345678;
            bool found = false;

            if (relative)
                payload -= kernel.size();

            for (size_t i = offset0; i + 3 < offset1; ++i) {
                if (memcmp(ptr, &dummy, 4) == 0) {
                    memcpy(ptr, &payload, 4);
                    found = true;
                    break;
                }
                ptr++;
            }

            if (!found) {
                fprintf(stderr, "struct-jit: payload of impl %zu not found!", size_t(i));
                abort();
            }
        }
    };

    kernel.reserve(200);

    // Clear registers
    put(impl::main_prefix);

    size_t main_start = kernel.size();
    put(impl::main_start);

    bool bswap_in  = m_in.byte_order()  != native_byte_order(),
         bswap_out = m_out.byte_order() != native_byte_order();

    size_t loop_body = kernel.size();

    for (auto [i_in, i_out] : m_plan) {
        Type type;
        bool gamma;

        if (i_in != None) {
            const Field &fi = m_in[i_in];

            type = fi.type;
            gamma = fi.flags & Flag::Gamma;
            bool normalized = fi.flags & Flag::Normalized;

            switch (type) {
                case Type::Int8:
                case Type::UInt8:
                    put(impl::load_u1, fi.offset);
                    break;

                case Type::Int16:
                case Type::UInt16:
                    put(impl::load_u2, fi.offset);
                    if (bswap_in)
                        put(impl::bswap_u2);
                    break;

                case Type::Int32:
                case Type::UInt32:
                    put(impl::load_u4, fi.offset);
                    if (bswap_in)
                        put(impl::bswap_u4);
                    break;

                case Type::Int64:
                case Type::UInt64:
                    put(impl::load_u8, fi.offset);
                    if (bswap_in)
                        put(impl::bswap_u8);
                    break;

                case Type::Float16:
                    if (!bswap_in) {
                        put(impl::load_f2, fi.offset);
                    } else {
                        put(impl::load_u2, fi.offset);
                        put(impl::bswap_u2);
                        put(impl::mov_u2_f2);
                    }
                    type = Type::Float32;
                    break;

                case Type::Float32:
                    if (!bswap_in) {
                        put(impl::load_f4, fi.offset);
                    } else {
                        put(impl::load_u4, fi.offset);
                        put(impl::bswap_u4);
                        put(impl::mov_u4_f4);
                    }
                    break;

                case Type::Float64:
                    if (!bswap_in) {
                        put(impl::load_f8, fi.offset);
                    } else {
                        put(impl::load_u8, fi.offset);
                        put(impl::bswap_u8);
                        put(impl::mov_u8_f8);
                    }
                    break;

                default:
                    fprintf(stderr, "struct-jit: unhandled case (1)");
                    abort();
            }

            bool requires_conversion = false;

            if (i_out != None) {
                const Field &fo = m_out[i_out];
                requires_conversion =
                    requires_conversion ||
                    fo.type != type ||
                    (fo.flags & Flag::Gamma) != gamma ||
                    (fo.flags & Flag::Normalized) != normalized;
            }

            if (requires_conversion) {
                switch (type) {
                    case Type::Int8:   put(impl::cvt_i1_f4); type = Type::Float32; break;
                    case Type::UInt8:  put(impl::cvt_u1_f4); type = Type::Float32; break;
                    case Type::Int16:  put(impl::cvt_i2_f4); type = Type::Float32; break;
                    case Type::UInt16: put(impl::cvt_u2_f4); type = Type::Float32; break;
                    case Type::Int32:  put(impl::cvt_i4_f4); type = Type::Float32; break;
                    case Type::UInt32: put(impl::cvt_u4_f4); type = Type::Float32; break;
                    case Type::Int64:  put(impl::cvt_i8_f8); type = Type::Float64; break;
                    case Type::UInt64: put(impl::cvt_u8_f8); type = Type::Float64; break;
                }
            }
        } else {
            // Input field is missing, substitute a default
            const Field &fo = m_out[i_out];
            type = fo.type;
            gamma = fo.flags & Flag::Gamma;
            // TODO
        }

        //     convert_scalar(
        //         temp, size(temp.type) >= 4 ? Type::Float64 : Type::Float32,
        //         normalize);

        if (i_out == None)
            continue;

        const Field &fo = m_out[i_out];
        switch (fo.type) {
            case Type::Int8:
            case Type::UInt8:
                put(impl::store_u1, fo.offset);
                break;

            case Type::Int16:
            case Type::UInt16:
                if (bswap_out)
                    put(impl::bswap_u2);
                put(impl::store_u2, fo.offset);
                break;

            case Type::Int32:
            case Type::UInt32:
                if (bswap_out)
                    put(impl::bswap_u4);
                put(impl::store_u4, fo.offset);
                break;

            case Type::Int64:
            case Type::UInt64:
                if (bswap_out)
                    put(impl::bswap_u8);
                put(impl::store_u8, fo.offset);
                break;

            case Type::Float16:
                if (!bswap_out) {
                    put(impl::store_f2, fo.offset);
                } else {
                    put(impl::mov_f2_u2);
                    put(impl::bswap_u2);
                    put(impl::store_u2, fo.offset);
                }
                break;

            case Type::Float32:
                if (!bswap_out) {
                    put(impl::store_f4, fo.offset);
                } else {
                    put(impl::mov_f4_u4);
                    put(impl::bswap_u4);
                    put(impl::store_u4, fo.offset);
                }
                break;

            case Type::Float64:
                if (!bswap_out) {
                    put(impl::store_f8, fo.offset);
                } else {
                    put(impl::mov_f8_u8);
                    put(impl::bswap_u8);
                    put(impl::store_u8, fo.offset);
                }
                break;

            default:
                fprintf(stderr, "struct-jit: unhandled case (2)");
                abort();
        }
    }

    // Loop body goes here
    put(impl::main_inc_in, m_in.size());
    put(impl::main_inc_out, m_out.size());
    put(impl::main_suffix_inner, loop_body, true);
    put(impl::main_suffix_outer, main_start, true);
    put(impl::main_postfix);

    FILE *f = fopen("out.bin", "wb");
    fwrite(kernel.data(), kernel.size(), 1, f);
    fclose(f);

    m_kernel_size = kernel.size();

#if !defined(_WIN32)
    void *ptr = mmap(nullptr, m_kernel_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "struct-jit: could not mmap() memory: %s",
                strerror(errno));
        abort();
    }

#else
    void *ptr = VirtualAlloc(nullptr, m_kernel_size, MEM_RESERVE | MEM_COMMIT,
                             PAGE_READWRITE);
    if (!ptr) {
        fprintf(stderr, "struct-jit: could not VirtualAlloc() memory: %u",
                GetLastError());
        abort();
    }
#endif
    memcpy(ptr, kernel.data(), m_kernel_size);

#if !defined(_WIN32)
    if (mprotect(ptr, m_kernel_size, PROT_READ | PROT_EXEC) == -1) {
        fprintf(stderr, "struct-jit: mprotect() failed: %s", strerror(errno));
        abort();
    }
#else
    DWORD unused;
    if (VirtualProtect(ptr, m_kernel_size, PAGE_EXECUTE_READ, &unused) == 0) {
        fprintf(stderr, "struct-jit: VirtualProtect() failed: %u",
                GetLastError());
        abort();
    }
#endif

    m_kernel = (Kernel *) ptr;
}

void Converter::release_kernel() {
#if !defined(_WIN32)
    if (munmap((void *) m_kernel, m_kernel_size) == -1) {
        fprintf(stderr, "struct-jit: munmap() failed!");
        abort();
    }
#else
    if (VirtualFree((void*) m_kernel, 0, MEM_RELEASE) == 0) {
        fprintf(stderr, "struct-jit: VirtualFree() failed!");
        abort();
    }
#endif
}

NAMESPACE_END(struct_jit)
