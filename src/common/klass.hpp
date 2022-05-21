#pragma once

#define LN_KLZ_DELETE_COPY_MOVE(i_klz_name)                                    \
    LN_KLZ_DELETE_COPY(i_klz_name);                                            \
    LN_KLZ_DELETE_MOVE(i_klz_name)

#define LN_KLZ_DELETE_COPY(i_klz_name)                                         \
    i_klz_name(const i_klz_name &) = delete;                                   \
    i_klz_name &operator=(const i_klz_name &) = delete

#define LN_KLZ_DELETE_MOVE(i_klz_name)                                         \
    i_klz_name(i_klz_name &&) = delete;                                        \
    i_klz_name &operator=(i_klz_name &&) = delete

#define LN_KLZ_DEFAULT_COPY_MOVE(i_klz_name)                                   \
    LN_KLZ_DEFAULT_COPY(i_klz_name);                                           \
    LN_KLZ_DEFAULT_MOVE(i_klz_name)

#define LN_KLZ_DEFAULT_COPY(i_klz_name)                                        \
    i_klz_name(const i_klz_name &) = default;                                  \
    i_klz_name &operator=(const i_klz_name &) = default

#define LN_KLZ_DEFAULT_MOVE(i_klz_name)                                        \
    i_klz_name(i_klz_name &&) = default;                                       \
    i_klz_name &operator=(i_klz_name &&) = default
