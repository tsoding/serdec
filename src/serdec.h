// This is the "base" of Serdec which only parses a small subset of C which then
// can be used by specific generators like serdec_jim.h and serdec_debug.h.
#ifndef SERDEC_H_
#define SERDEC_H_

#include "stb_c_lexer.h"
#define NOB_STRIP_PREFIX
#include "nob.h"

typedef enum {
    TYPE_STRING,
    TYPE_DOUBLE,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_AUTO,
    COUNT_TYPES,
} Primitive_Type;

typedef struct {
    const char *type;
    const char *name;
    const char *tag;
    Primitive_Type p_type;
} Field;

typedef struct {
    const char *name;

    Field *items;
    size_t count;
    size_t capacity;
} Struct;

typedef struct {
    Struct *items;
    size_t count;
    size_t capacity;
} Structs;

void dump_struct(Struct strukt);
bool expect_token(stb_lexer *l, const char *file_path, long token);
bool get_and_expect_token(stb_lexer *l, const char *file_path, long token);
bool get_and_expect_id(stb_lexer *l, const char *file_path, const char *name);
bool parse_type(stb_lexer *l, const char *file_path, Field *field);
bool parse_field(stb_lexer *l, const char *file_path, Field *field);
bool parse_struct(stb_lexer *l, const char *file_path, Struct *strukt);
bool parse_structs(stb_lexer *l, const char *file_path, Structs *structs);

#endif // SERDEC_H_

#ifdef SERDEC_IMPLEMENTATION

void dump_struct(Struct strukt)
{
    printf("struct %s:\n", strukt.name);
    da_foreach(Field, field, &strukt) {
        printf("    %s: ", field->name);
        switch (field->p_type) {
        case TYPE_STRING: printf("string"); break;
        case TYPE_DOUBLE: printf("double"); break;
        case TYPE_INT:    printf("int"); break;
        case TYPE_BOOL:   printf("bool"); break;
        case COUNT_TYPES: default: UNREACHABLE("field->type");
        }
        printf("\n");
    }
}

bool expect_token(stb_lexer *l, const char *file_path, long token)
{
    if (l->token != token) {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(l, l->where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: expected %ld, but got %ld\n", file_path, loc.line_number, loc.line_offset, token, l->token);
        return false;
    }
    return true;
}

bool get_and_expect_token(stb_lexer *l, const char *file_path, long token)
{
    stb_c_lexer_get_token(l);
    return expect_token(l, file_path, token);
}

bool get_and_expect_id(stb_lexer *l, const char *file_path, const char *name)
{
    if (!get_and_expect_token(l, file_path, CLEX_id)) return false;
    if (strcmp(l->string, name) != 0) {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(l, l->where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: expected `%s`, but got `%s`\n", file_path, loc.line_number, loc.line_offset, name, l->string);
        return false;
    }
    return true;
}

bool parse_type(stb_lexer *l, const char *file_path, Field *field)
{
    if (!get_and_expect_token(l, file_path, CLEX_id)) return false;

    if (strcmp(l->string, "const") == 0) {
        if (!get_and_expect_id(l, file_path, "char")) return false;
        if (!get_and_expect_token(l, file_path, '*')) return false;
        field->p_type = TYPE_STRING;
        field->type = "const char *";
        return true;
    }

    if (strcmp(l->string, "double") == 0) {
        field->p_type = TYPE_DOUBLE;
        field->type = strdup(l->string);
        return true;
    }

    if (strcmp(l->string, "int") == 0) {
        field->p_type = TYPE_INT;
        field->type = strdup(l->string);
        return true;
    }

    if (strcmp(l->string, "bool") == 0) {
        field->p_type = TYPE_BOOL;
        field->type = strdup(l->string);
        return true;
    }
    
    static_assert(COUNT_TYPES == 5, "Amount of types have changed");

    field->p_type = TYPE_AUTO;
    field->type = strdup(l->string);
    return true;

    // stb_lex_location loc = {0};
    // stb_c_lexer_get_location(l, l->where_firstchar, &loc);
    // fprintf(stderr, "%s:%d:%d: ERROR: unsupported type starting with `%s`\n", file_path, loc.line_number, loc.line_offset, l->string);

    // return false;
}

bool parse_struct(stb_lexer *l, const char *file_path, Struct *strukt)
{
    if (!get_and_expect_id(l, file_path, "typedef")) return false;
    if (!get_and_expect_id(l, file_path, "struct"))  return false;
    if (!get_and_expect_token(l, file_path, '{'))    return false;

    for (;;) {
        char *saved_point = l->parse_point;
        stb_c_lexer_get_token(l);
        if (l->token == '}') break;
        l->parse_point = saved_point;

        Field field = {0};
        if (!parse_field(l, file_path, &field)) return false;
        da_append(strukt, field);
    }

    if (!get_and_expect_token(l, file_path, CLEX_id)) return false;
    strukt->name = strdup(l->string); // memory leak
    if (!get_and_expect_token(l, file_path, ';'))    return false;
    return true;
}

bool parse_field(stb_lexer *l, const char *file_path, Field *field)
{
    if (!parse_type(l, file_path, field))      return false;
    if (!get_and_expect_token(l, file_path, CLEX_id)) return false;
    field->name = strdup(l->string); // memory leak

    char *saved_point = l->parse_point;
    stb_c_lexer_get_token(l);
    if (l->token == CLEX_id && strcmp(l->string, "tag") == 0) {
        if (!get_and_expect_token(l, file_path, '(')) return false;
        if (!get_and_expect_token(l, file_path, CLEX_dqstring)) return false;
        field->tag = strdup(l->string); // memory leak
        if (!get_and_expect_token(l, file_path, ')')) return false;
    } else {
        field->tag = field->name;
        l->parse_point = saved_point;
    }

    if (!get_and_expect_token(l, file_path, ';'))     return false;
    return true;
}

bool parse_structs(stb_lexer *l, const char *file_path, Structs *structs)
{
    for (;;) {
        char *saved_point = l->parse_point;
        stb_c_lexer_get_token(l);
        if (l->token == CLEX_eof) return true;
        l->parse_point = saved_point;

        Struct strukt = {0};
        if (!parse_struct(l, file_path, &strukt)) return false;
        da_append(structs, strukt);
    }

    UNREACHABLE("parse_structs");
}

#endif // SERDEC_IMPLEMENTATION
