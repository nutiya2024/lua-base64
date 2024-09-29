#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Base64字符表
static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// 用于计算填充长度的表
static const int mod_table[] = {0, 2, 1};

// Base64编码函数
static char *base64_encode(const unsigned char *input, int length) {
    int output_length = 4 * ((length + 2) / 3);
    char *encoded = malloc(output_length + 1);
    if (encoded == NULL) return NULL;

    for (int i = 0, j = 0; i < length;) {
        uint32_t octet_a = i < length ? input[i++] : 0;
        uint32_t octet_b = i < length ? input[i++] : 0;
        uint32_t octet_c = i < length ? input[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded[j++] = b64chars[(triple >> 18) & 0x3F];
        encoded[j++] = b64chars[(triple >> 12) & 0x3F];
        encoded[j++] = b64chars[(triple >> 6) & 0x3F];
        encoded[j++] = b64chars[triple & 0x3F];
    }

    for (int i = 0; i < mod_table[length % 3]; i++)
        encoded[output_length - 1 - i] = '=';

    encoded[output_length] = '\0';
    return encoded;
}

// Base64解码函数
static unsigned char *base64_decode(const char *input, int length, int *output_length) {
    if (length % 4 != 0) return NULL;

    *output_length = length / 4 * 3;
    if (input[length - 1] == '=') (*output_length)--;
    if (input[length - 2] == '=') (*output_length)--;

    unsigned char *decoded = malloc(*output_length);
    if (decoded == NULL) return NULL;

    for (int i = 0, j = 0; i < length;) {
        uint32_t sextet_a = input[i] == '=' ? 0 & i++ : strchr(b64chars, input[i++]) - b64chars;
        uint32_t sextet_b = input[i] == '=' ? 0 & i++ : strchr(b64chars, input[i++]) - b64chars;
        uint32_t sextet_c = input[i] == '=' ? 0 & i++ : strchr(b64chars, input[i++]) - b64chars;
        uint32_t sextet_d = input[i] == '=' ? 0 & i++ : strchr(b64chars, input[i++]) - b64chars;

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded[j++] = triple & 0xFF;
    }

    return decoded;
}

// Lua绑定的Base64编码函数
static int lua_base64_encode(lua_State *L) {
    size_t input_length;
    const unsigned char *input = (const unsigned char *)luaL_checklstring(L, 1, &input_length);
    char *output = base64_encode(input, input_length);
    if (output) {
        lua_pushstring(L, output);
        free(output);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// Lua绑定的Base64解码函数
static int lua_base64_decode(lua_State *L) {
    size_t input_length;
    int output_length;
    const char *input = luaL_checklstring(L, 1, &input_length);
    unsigned char *output = base64_decode(input, input_length, &output_length);
    if (output) {
        lua_pushlstring(L, (const char *)output, output_length);
        free(output);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

// 模块注册函数
static const struct luaL_Reg base64_lib[] = {
    {"encode", lua_base64_encode},
    {"decode", lua_base64_decode},
    {NULL, NULL}
};

int luaopen_base64(lua_State *L) {
    luaL_newlib(L, base64_lib);
    return 1;
}
