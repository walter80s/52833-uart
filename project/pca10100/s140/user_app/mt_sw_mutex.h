/* Copyright (C) Shenzhen Minew Technologies Co., Ltd 
   All rights reserved. */

#ifndef MT_SW_MUTEX_H
#define MT_SW_MUTEX_H

/* Mutex handler with return with specific value. */
#define MT_SW_MUTEX_T_R(m, ret) \
    do {                        \
        if (*m == 1) {          \
            *m = 0;             \
        } else {                \
            return ret;         \
        }                       \
    } while (0)

#define MT_SW_MUTEX_G_R(m, ret) \
    do {                        \
        if (*m == 0) {          \
            *m = 1;             \
        } else {                \
            return ret;         \
        }                       \
    } while (0)

/* Mutex handler without specific return value. */
#define MT_SW_MUTEX_T(m) \
    do {                 \
        if (*m == 1) {   \
            *m = 0;      \
        } else {         \
            return;      \
        }                \
    } while (0)

#define MT_SW_MUTEX_G(m) \
    do {                 \
        if (*m == 0) {   \
            *m = 1;      \
        } else {         \
            return;      \
        }                \
    } while (0)

#endif
