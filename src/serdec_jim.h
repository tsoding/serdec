// This is a Serdec Generator for jim.h/jimp.h immediate serialization/deserialization libraries.
#ifndef SERDEC_JIM_H_
#define SERDEC_JIM_H_

#include "serdec.h"
#define NOB_STRIP_PREFIX
#include "nob.h"

void generate_struct_jim_serializer(Struct strukt, String_Builder *out);
void generate_struct_jimp_deserializer(Struct strukt, String_Builder *out);

#endif // SERDEC_JIM_H_

#ifdef SERDEC_JIM_IMPLEMENTATION

void generate_struct_jim_serializer(Struct strukt, String_Builder *out)
{
    sb_appendf(out, "void jim_%s(Jim *jim, %s s)\n", strukt.name, strukt.name);
    sb_appendf(out, "{\n");
    sb_appendf(out, "    jim_object_begin(jim);\n");
    da_foreach(Field, field, &strukt) {
        sb_appendf(out, "        jim_member_key(jim, \"%s\");\n", field->tag);
        switch (field->p_type) {
        case TYPE_STRING:
            sb_appendf(out, "        jim_string(jim, s.%s);\n", field->name);
            break;
        case TYPE_DOUBLE:
            sb_appendf(out, "        jim_float(jim, s.%s, 4);\n", field->name);
            break;
        case TYPE_INT:
            sb_appendf(out, "        jim_integer(jim, s.%s);\n", field->name);
            break;
        case TYPE_BOOL:
            sb_appendf(out, "        jim_bool(jim, s.%s);\n", field->name);
            break;
        case TYPE_AUTO:
            sb_appendf(out, "        jim_%s(jim, s.%s);\n", field->type, field->name);
            break;
        case COUNT_TYPES: default: UNREACHABLE("field->type");
        }
    }
    sb_appendf(out, "    jim_object_end(jim);\n");
    sb_appendf(out, "}\n");
}

void generate_struct_jimp_deserializer(Struct strukt, String_Builder *out)
{
    sb_appendf(out, "bool jimp_%s(Jimp *jimp, %s *s)\n", strukt.name, strukt.name);
    sb_appendf(out, "{\n");
    sb_appendf(out, "    if (!jimp_object_begin(jimp)) return false;\n");
    sb_appendf(out, "    while (jimp_object_member(jimp)) {\n");
    da_foreach(Field, field, &strukt) {
        sb_appendf(out, "        if (strcmp(jimp->string, \"%s\") == 0) {\n", field->tag); // TODO: tag should be properly escaped
        switch (field->p_type) {
        case TYPE_STRING:
            sb_appendf(out, "            if (!jimp_string(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = strdup(jimp->string); // memory leak\n", field->name);
            break;
        case TYPE_DOUBLE:
            sb_appendf(out, "            if (!jimp_number(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->number;\n", field->name);
            break;
        case TYPE_INT:
            sb_appendf(out, "            if (!jimp_number(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->number;\n", field->name);
            break;
        case TYPE_BOOL:
            sb_appendf(out, "            if (!jimp_bool(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->boolean;\n", field->name);
            break;
        case TYPE_AUTO:
            sb_appendf(out, "            if (!jimp_%s(jimp, &s->%s)) return false;\n", field->type, field->name);
            break;
        case COUNT_TYPES: default: UNREACHABLE("field->type");
        }
        sb_appendf(out, "            continue;\n");
        sb_appendf(out, "        }\n");
    }
    sb_appendf(out, "    }\n");
    sb_appendf(out, "    if (!jimp_object_end(jimp)) return false;\n");
    sb_appendf(out, "    return true;\n");
    sb_appendf(out, "}\n");
}

#endif // SERDEC_JIM_IMPLEMENTATION
