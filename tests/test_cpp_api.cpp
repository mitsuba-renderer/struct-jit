#include <struct-jit/struct-jit.h>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
using namespace struct_jit;
namespace sjit = struct_jit;

int main() {
    static_assert(!std::is_copy_constructible_v<Converter>);
    static_assert(!std::is_copy_assignable_v<Converter>);
    static_assert(std::is_move_constructible_v<Converter>);
    static_assert(std::is_move_assignable_v<Converter>);

    Struct automatic;
    automatic.append("x", Type::UInt8);
    if (automatic.nbytes() != 1)
        return 1;

    Field late;
    late.name = "b";
    late.type = Type::UInt8;
    late.offset = 8;

    Field early;
    early.name = "a";
    early.type = Type::UInt8;
    early.offset = 0;

    Struct manual;
    manual.append(late);
    manual.append(early);
    if (manual.size() != 2)
        return 1;
    if (manual[0].name != "a" || manual[1].name != "b")
        return 1;
    if (manual.nbytes() != 9)
        return 1;

    auto range32 = type_range(Type::Float32);
    auto range64 = type_range(Type::Float64);
    if (!(range32.first < 0.0 && range32.second > 0.0))
        return 1;
    if (!(range64.first < 0.0 && range64.second > 0.0))
        return 1;

    Field check;
    check.name = "v";
    check.type = Type::UInt8;
    check.flags = +Flag::Check;
    check.value = 0x010000000000000aull;

    Struct checked_in;
    checked_in.append(check);
    Struct checked_out;
    checked_out.append("v", Type::UInt8);

    uint8_t input = 10, output_jit = 0, output_fallback = 0;
    Converter check_jit(checked_in, checked_out, true);
    Converter check_fallback(checked_in, checked_out, false);
    if (!check_jit.convert(&input, &output_jit, 1, 1))
        return 1;
    if (!check_fallback.convert(&input, &output_fallback, 1, 1))
        return 1;
    if (output_jit != 10 || output_fallback != 10)
        return 1;

    Struct cache_in;
    cache_in.append("v", Type::UInt8, +Flag::Normalized);
    Struct cache_out;
    cache_out.append("v", Type::Float32);

    sjit::clear_cache();
    const Converter &cached_1 = sjit::make_converter(cache_in, cache_out);
    const Converter &cached_2 = sjit::make_converter(cache_in, cache_out);
    if (&cached_1 != &cached_2)
        return 1;

    const Converter &cached_fallback =
        sjit::make_converter(cache_in, cache_out, false);
    if (&cached_fallback == &cached_1)
        return 1;

    uint8_t cache_input = 255;
    float cache_output_1 = 0.f, cache_output_2 = 0.f;
    if (!cached_1.convert(&cache_input, &cache_output_1, 1, 1))
        return 1;
    if (!cached_fallback.convert(&cache_input, &cache_output_2, 1, 1))
        return 1;
    if (cache_output_1 != cache_output_2 || cache_output_1 != 1.f)
        return 1;

    sjit::clear_cache();

    Field unknown_flags;
    unknown_flags.name = "bad";
    unknown_flags.type = Type::UInt8;
    unknown_flags.flags = 1u << 31;
    try {
        Struct().append(unknown_flags);
        return 1;
    } catch (const std::runtime_error &) { }

    Struct duplicate_names;
    duplicate_names.append("a", Type::UInt8);
    duplicate_names.append("b", Type::UInt8);
    duplicate_names["b"].name = "a";
    try {
        duplicate_names.validate();
        return 1;
    } catch (const std::runtime_error &) { }

    Field large;
    large.name = "large";
    large.type = Type::UInt64;
    large.offset = 4088;
    Struct too_large;
    too_large.append(large);
    try {
        Converter rejected(too_large, too_large, false);
        return 1;
    } catch (const std::runtime_error &) { }

    return 0;
}
