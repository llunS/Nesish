#pragma once

#define NB_KLZ_DELETE_COPY_MOVE(klass)                                         \
    NB_KLZ_DELETE_COPY(klass);                                                 \
    NB_KLZ_DELETE_MOVE(klass)

#define NB_KLZ_DELETE_COPY(klass)                                              \
    klass(const klass &) = delete;                                             \
    klass &operator=(const klass &) = delete

#define NB_KLZ_DELETE_MOVE(klass)                                              \
    klass(klass &&) = delete;                                                  \
    klass &operator=(klass &&) = delete

#define NB_KLZ_DEFAULT_COPY_MOVE(klass)                                        \
    NB_KLZ_DEFAULT_COPY(klass);                                                \
    NB_KLZ_DEFAULT_MOVE(klass)

#define NB_KLZ_DEFAULT_COPY(klass)                                             \
    klass(const klass &) = default;                                            \
    klass &operator=(const klass &) = default

#define NB_KLZ_DEFAULT_MOVE(klass)                                             \
    klass(klass &&) = default;                                                 \
    klass &operator=(klass &&) = default
