#include "impl/impl.h"
#include "struct-jit.h"

void put(FILE *f, impl i, uint32_t extra = (uint32_t) - 1, bool jump = false) {
    size_t offset = impl_offset[(uint16_t) i],
           size   = impl_offset[(uint16_t) i + 1] - offset;

    const uint8_t *data = impl_data + offset;

    if (extra == (uint32_t) - 1) {
        if (fwrite(data, size, 1, f) != 1) {
            fprintf(stderr, "I/O error (1)!\n");
            exit(-1);
        }
    } else {
        const uint32_t dummy = 0x12345678;
        for (ssize_t i = 0; i <= (ssize_t) size - 4; ++i) {
            if (dummy == *((uint32_t *) (data + i))) {
                bool err = fwrite(data, i, 1, f) != 1;
                if (jump)
                    extra = extra - ((uint32_t) ftell(f) + 4);
                err |= fwrite(&extra, 4, 1, f) != 1;
                if (size - i != 4)
                    err |= fwrite(data + i + 4, size - i - 4, 1, f) != 1;
                if (err) {
                    fprintf(stderr, "I/O error (2)!\n");
                    exit(-1);
                }
                return;
            }
        }
        fprintf(stderr, "Marker not found!\n");
        exit(-1);
    }
}

int main(int args, char **argv) {
    FILE * f = fopen("out.bin", "wb");

    put(f, impl::main_prefix);
    // put(f, impl::main_init_idx);

    size_t loop_start = ftell(f);
    put(f, impl::main_start);

    size_t loop_body = ftell(f);
    put(f, impl::loadd_f32, 0);
    put(f, impl::stored_f32, 0);

    put(f, impl::main_inc_src, 4);
    put(f, impl::main_inc_dst, 4);
    // put(f, impl::main_inc_idx);

    put(f, impl::main_suffix_inner, loop_body, true);
    put(f, impl::main_suffix_outer, loop_start, true);
    put(f, impl::main_postfix);

    fclose(f);
}
