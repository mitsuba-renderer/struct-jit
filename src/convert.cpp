#include <struct-jit/struct-jit.h>
#include <stdexcept>
#include <cstring>

NAMESPACE_BEGIN(struct_jit)


/// Convenience wrapper to raise an exception
static void raise(const std::string &msg) {
    throw std::runtime_error(msg);
}

/// Indicates a missing source/destination field
constexpr size_t None = size_t(-1);

using Plan = std::vector<std::pair<size_t, size_t>>;

/**
 * Analyze the input data and output data structure and prepare a list of
 * fields that should be visited.
 */
static Plan create_plan(const Struct &in_struct, const Struct &out_struct) {
    Plan plan;
    size_t weight_in = None, weight_out = None;

    // Check whether the input data structure has a (single) weight field.
    for (size_t i = 0; i < in_struct.fields(); ++i) {
        const Field &f = in_struct[i];

        if (has_flag(f.flags, Flag::Weight)) {
            if (weight_in != None)
                raise("struct_jit::create_plan(): the input data structure "
                      "contains multiple weight fields!");
            weight_in = i;
        }
    }

    for (size_t i = 0; i < out_struct.fields(); ++i) {
        const Field &f = out_struct[i];
        bool is_weight = has_flag(f.flags, Flag::Weight);

        // Check whether the output data structure has a (single) weight field.
        if (is_weight) {
            if (weight_out != None)
                raise("struct_jit::create_plan(): the output data structure "
                      "contains multiple weight fields!");
            weight_out = i;
        }

        Struct::ConstFieldIterator it = in_struct.find(f.name);

        if (it != in_struct.end()) {
            if (!is_weight)
                plan.emplace_back(it - in_struct.begin(), i);
        } else if (has_flag(f.flags, Flag::Default)) {
            if (!is_weight)
                plan.emplace_back(None, i);
        } else {
            raise("struct_jit::create_plan(): the output data structure "
                  "contains a field with name \"" + f.name +
                  "\" that could not be found in the input ,and which lacks a "
                  "default initialization.");
        }
    }

    if (weight_in != None || weight_out != None) {
        if (weight_in != None && weight_out != None) {
            const Field &fi = in_struct[weight_in],
                        &fo = out_struct[weight_out];

            if (fi.name != fo.name)
                raise("struct_jit::convert(): the weight fields of the input (\"" +
                      fi.name + "\") and output (\"" + fo.name +
                      "\") data structure have mismatched names!");
        }

        plan.insert(plan.begin(), std::make_pair(weight_in, weight_out));
    }

    return plan;
}

struct Temp {
    Type type;
    uint8_t value[8];
};

bool convert_1(const Plan &plan, const uint8_t *in, const Struct &in_struct,
               uint8_t *out, const Struct &out_struct, size_t x, size_t y) {
    (void) x; (void) y;

    Temp temp;
    for (auto [i_in, i_out] : plan) {
        if (i_in != None) {
            const Field &f = in_struct[i_in];
            memcpy(&temp.value, in + f.offset, size(f.type));
            temp.type = f.type;
        } else {
            const Field &f = out_struct[i_out];
            memcpy(&temp.value, &f.value, size(f.type));
            temp.type = f.type;
        }

        if (i_out != None) {
            const Field &f = out_struct[i_out];
            memcpy(out + f.offset, &temp.value, size(f.type));
        }
    }
    return true;
}

bool convert(const void *in, const Struct &in_struct, void *out,
             const Struct &out_struct, size_t width, size_t height) {

    Plan plan = create_plan(in_struct, out_struct);

    const uint8_t *in_ptr = (const uint8_t *) in;
    uint8_t *out_ptr = (uint8_t *) out;
    size_t in_size = in_struct.size(),
           out_size = out_struct.size();

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (!convert_1(plan, in_ptr, in_struct, out_ptr, out_struct, x, y))
                return false;

            in_ptr += in_size;
            out_ptr += out_size;
        }
    }

    return true;
}

NAMESPACE_END(struct_jit)
