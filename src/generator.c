#define NOB_STRIP_PREFIX
#include "nob.h"
#include "serdec.h"
#include "serdec_jim.h"
#include "serdec_debug.h"

void usage(const char *program_name)
{
    fprintf(stderr, "Usage: %s <input> <output>\n", program_name);
}

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);

    if (argc <= 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }
    const char *input_path = shift(argv, argc);

    if (argc <= 0) {
        usage(program_name);
        fprintf(stderr, "ERROR: no output is provided\n");
        return 1;
    }
    const char *output_path = shift(argv, argc);

    String_Builder sb = {0};

    if (!read_entire_file(input_path, &sb)) return 1;

    static char string_store[1024];

    stb_lexer l = {0};
    stb_c_lexer_init(&l, sb.items, sb.items + sb.count, string_store, sizeof(string_store));

    Structs structs = {0};
    if (!parse_structs(&l, input_path, &structs)) return 1;

    String_Builder out = {0};

    da_foreach(Struct, strukt, &structs) {
        generate_struct_jim_serializer(*strukt, &out);
        generate_struct_jimp_deserializer(*strukt, &out);
        generate_struct_c_printer(*strukt, &out);
    }
    if (!write_entire_file(output_path, out.items, out.count)) return 1;
    nob_log(INFO, "generated %s", output_path);
    return 0;
}

#define NOB_IMPLEMENTATION
#include "nob.h"
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"
#define SERDEC_IMPLEMENTATION
#include "serdec.h"
#define SERDEC_JIM_IMPLEMENTATION
#include "serdec_jim.h"
#define SERDEC_DEBUG_IMPLEMENTATION
#include "serdec_debug.h"
