// tao-generic.h -
//
// Definitions of generic functions for TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_GENERIC_H_

#define TAO_GENERIC_H_ 1

#include <math.h>
#include <stdbool.h>

#include <tao-threads.h>
#include <tao-remote-cameras.h>
#include <tao-remote-mirrors.h>
#include <tao-remote-objects.h>
#include <tao-remote-sensors.h>
#include <tao-rwlocked-objects.h>
#include <tao-shared-objects.h>

/**
 * @defgroup GenericMacros Generic macros
 *
 * @ingroup Utilities
 *
 * @brief C11 `_Generic` macros.
 *
 * Generic macros are provided to make the code more readable and more agnostic
 * about the type of arguments.
 *
 * @{
 */

#define TAO_ENCODE_(T, sfx)                                             \
    static inline void tao_forced_store_##sfx(T const* arg1_, T arg2_)  \
    {                                                                   \
        *(T*)arg1_ = arg2_;                                             \
    }

TAO_ENCODE_(bool,               b);
TAO_ENCODE_(char,               c);
TAO_ENCODE_(signed char,        sc);
TAO_ENCODE_(unsigned char,      uc);
TAO_ENCODE_(short,              s);
TAO_ENCODE_(unsigned short,     us);
TAO_ENCODE_(int,                i);
TAO_ENCODE_(unsigned int,       ui);
TAO_ENCODE_(long,               l);
TAO_ENCODE_(unsigned long,      ul);
TAO_ENCODE_(long long,          ll);
TAO_ENCODE_(unsigned long long, ull);
TAO_ENCODE_(float,              f);
TAO_ENCODE_(double,             d);
TAO_ENCODE_(long double,        ld);
TAO_ENCODE_(void*,              ptr);
TAO_ENCODE_(const char*,        str);

#undef TAO_ENCODE_

/**
 * @def tao_forced_store(ptr, val)
 *
 * Force storing value `val` at constant pointer address `ptr`.
 */
#ifdef TAO_DOXYGEN_
#define tao_forced_store(ptr, val) ...
#else
#define tao_forced_store(ptr, val)                                      \
    _Generic(                                                           \
        ptr,                                                            \
        bool               const*: tao_forced_store_b,                  \
        char               const*: tao_forced_store_c,                  \
        signed char        const*: tao_forced_store_sc,                 \
        unsigned char      const*: tao_forced_store_uc,                 \
        short              const*: tao_forced_store_s,                  \
        unsigned short     const*: tao_forced_store_us,                 \
        int                const*: tao_forced_store_i,                  \
        unsigned int       const*: tao_forced_store_ui,                 \
        long               const*: tao_forced_store_l,                  \
        unsigned long      const*: tao_forced_store_ul,                 \
        long long          const*: tao_forced_store_ll,                 \
        unsigned long long const*: tao_forced_store_ull,                \
        float              const*: tao_forced_store_f,                  \
        double             const*: tao_forced_store_d,                  \
        long double        const*: tao_forced_store_ld,                 \
        void*              const*: tao_forced_store_ptr,                \
        const char*        const*: tao_forced_store_str)(ptr, val)
#endif /* TAO_DOXYGEN_ */

// tao_min(x,y), tao_max(x,y), tao_clamp(x,lo,hi)  yields their first argument
// if any of their arguments is a NaN.
//
// tao_clamp(x,lo,hi) Clamp x in the range [lo,hi].
#define TAO_ENCODE_(T, sfx)                                             \
    static inline T tao_min_##sfx(T arg1_, T arg2_)                     \
    {                                                                   \
        return arg1_ > arg2_ ? arg2_ : arg1_;                           \
    }                                                                   \
    static inline T tao_max_##sfx(T arg1_, T arg2_)                     \
    {                                                                   \
        return arg1_ < arg2_ ? arg2_ : arg1_;                           \
    }                                                                   \
    static inline T tao_clamp_##sfx(T arg1_, T arg2_, T arg3_)          \
    {                                                                   \
        return tao_min_##sfx(tao_max_##sfx(arg1_, arg2_), arg3_);       \
    }

TAO_ENCODE_(char,               c);
TAO_ENCODE_(signed char,        sc);
TAO_ENCODE_(unsigned char,      uc);
TAO_ENCODE_(short,              s);
TAO_ENCODE_(unsigned short,     us);
TAO_ENCODE_(int,                i);
TAO_ENCODE_(unsigned int,       ui);
TAO_ENCODE_(long,               l);
TAO_ENCODE_(unsigned long,      ul);
TAO_ENCODE_(long long,          ll);
TAO_ENCODE_(unsigned long long, ull);
TAO_ENCODE_(float,              f);
TAO_ENCODE_(double,             d);
TAO_ENCODE_(long double,        ld);

#undef TAO_ENCODE_

static inline bool tao_min_b(bool arg1_, bool arg2_)
{
    return arg1_ & arg2_;
}

static inline bool tao_max_b(bool arg1_, bool arg2_)
{
    return arg1_ | arg2_;
}

static inline bool tao_clamp_b(bool arg1_, bool arg2_, bool arg3_)
{
    return tao_min_b(tao_max_b(arg1_, arg2_), arg3_);
}

// Choice based on numeric type of expression
#define TAO_NUMERIC_CHOICE(pfx, expr)           \
    _Generic(                                   \
        (expr),                                 \
        bool:               pfx##_b,            \
        char:               pfx##_c,            \
        signed char:        pfx##_sc,           \
        unsigned char:      pfx##_uc,           \
        short:              pfx##_s,            \
        unsigned short:     pfx##_us,           \
        int:                pfx##_i,            \
        unsigned int:       pfx##_ui,           \
        long:               pfx##_l,            \
        unsigned long:      pfx##_ul,           \
        long long:          pfx##_ll,           \
        unsigned long long: pfx##_ull,          \
        float:              pfx##_f,            \
        double:             pfx##_d,            \
        long double:        pfx##_ld)

/**
 * @def tao_min(x, y)
 *
 * Generic macro to yield the minimum of `x` and `y`.  The resulting code is
 * inlined, each argument is evaluated once.
 */
#define tao_min(x, y) \
    TAO_NUMERIC_CHOICE(tao_min, (x) + (y))(x, y)

/**
 * @def tao_max(x, y)
 *
 * Generic macro to yield the maximum of `x` and `y`.  The resulting code is
 * inlined, each argument is evaluated once.
 */
#define tao_max(x, y) \
    TAO_NUMERIC_CHOICE(tao_max, (x) + (y))(x, y)

/**
 * @def tao_clamp(x, lo, hi)
 *
 * Generic macro to clamp the value of `x` in the bounds `lo` and `hi`.  The
 * resulting code is inlined, each argument is evaluated once.
 */
#define tao_clamp(x, lo, hi) \
    TAO_NUMERIC_CHOICE(tao_clamp, (x) + (lo) + (hi))(x, lo, hi)

// Choice based on numeric floating-point type of expression
#define TAO_FLOATING_POINT_CHOICE(pfx, expr)    \
    _Generic(                                   \
        (expr),                                 \
        bool:               pfx##_d,            \
        char:               pfx##_d,            \
        signed char:        pfx##_d,            \
        unsigned char:      pfx##_d,            \
        short:              pfx##_d,            \
        unsigned short:     pfx##_d,            \
        int:                pfx##_d,            \
        unsigned int:       pfx##_d,            \
        long:               pfx##_d,            \
        unsigned long:      pfx##_d,            \
        long long:          pfx##_d,            \
        unsigned long long: pfx##_d,            \
        float:              pfx##_f,            \
        double:             pfx##_d,            \
        long double:        pfx##_ld)

// Clamp x in the range [a,b] returning c if the result of clamping is a NaN.
// Neither a, nor b should be NaN, a ≤ b must hold.
//
// The following expressions assume that a comparison with a NaN always yields
// false and that only argument x can be a NaN.
//
// FIXME: These assumptions are broken by compilation option `-ffast-math`.
#define TAO_ENCODE_(T, sfx)                                     \
    static inline T tao_safe_clamp_##sfx(                       \
        double arg_x_,                                          \
        double arg_a_,                                          \
        double arg_b_,                                          \
        double arg_c_)                                          \
    {                                                           \
        /* y is the greatest of x and a; NaN, if x is a NaN */  \
        double arg_y_ = (arg_x_ < arg_a_ ? arg_a_ : arg_x_);    \
        /* z is the least of y and b; NaN, if y is a NaN */     \
        double arg_z_ = (arg_y_ > arg_b_ ? arg_b_ : arg_y_);    \
        return (isnan(arg_z_) ? arg_c_ : arg_z_);               \
    }
TAO_ENCODE_(float,       f);
TAO_ENCODE_(double,      d);
TAO_ENCODE_(long double, ld);
#undef TAO_ENCODE_

/**
 * @def tao_safe_clamp(x, a, b, c)
 *
 * Generic macro to clamp `x` in the range `[a,b]` returning `c` if the result
 * of clamping is a NaN.  The resulting code is inlined, each argument is
 * evaluated once.  Argument `x` may be a NaN but neither `a` nor `b` should be
 * a NaN.  In addition, `a ≤ b` is assumed.  Argument `c` may be any value, not
 * necessarily `c ∈ [a,b]`.
 *
 * @warning These assumptions are broken by compilation option `-ffast-math`.
 */
#define tao_safe_clamp(x, a, b, c) TAO_FLOATING_POINT_CHOICE(   \
        tao_safe_clamp, (x) + (a) + (b) + (c))(x, a, b, c)

#define TAO_ENCODE_(T, sfx)                             \
    static inline T tao_ifelse_##sfx(                   \
        bool arg_test_,                                 \
        T arg_true_,                                    \
        T arg_false_)                                   \
    {                                                   \
        return arg_test_ ?  arg_true_ : arg_false_;     \
    }

TAO_ENCODE_(char,               c);
TAO_ENCODE_(signed char,        sc);
TAO_ENCODE_(unsigned char,      uc);
TAO_ENCODE_(short,              s);
TAO_ENCODE_(unsigned short,     us);
TAO_ENCODE_(int,                i);
TAO_ENCODE_(unsigned int,       ui);
TAO_ENCODE_(long,               l);
TAO_ENCODE_(unsigned long,      ul);
TAO_ENCODE_(long long,          ll);
TAO_ENCODE_(unsigned long long, ull);
TAO_ENCODE_(float,              f);
TAO_ENCODE_(double,             d);
TAO_ENCODE_(long double,        ld);

#undef TAO_ENCODE_

/**
 * @def tao_ifelse(cond, x, y)
 *
 * Generic macro which yields `x` if condition `cond` is true and `y`
 * otherwise.  The resulting code is inlined, each argument is evaluated once.
 * This macro differs from the ternary operator `cond ? x : y` or `if ... else
 * ...` statement in that it is a function, so all the arguments are evaluated
 * first. In some cases, using `tao_ifelse` instead of an if statement can
 * eliminate the branch in generated code and provide higher performance in
 * tight loops.
 */
#define tao_ifelse(cond, x, y) \
    TAO_NUMERIC_CHOICE(tao_ifelse, (x) + (y))(cond, x, y)

//-----------------------------------------------------------------------------
// Casting of objects.

/**
 * @def tao_shared_object_cast(obj)
 *
 * This macro casts a pointer to an object instance into a @ref
 * tao_shared_object pointer.  Only objects of valid type are accepted.
 */
#ifdef TAO_DOXYGEN_
#define tao_shared_object_cast(obj) ...
#else
#define tao_shared_object_cast(obj)                                     \
    _Generic(                                                           \
        (obj),                                                          \
        tao_shared_object        *: (obj),                              \
        tao_remote_object        *: (tao_shared_object*)(obj),          \
        tao_remote_camera        *: (tao_shared_object*)(obj),          \
        tao_remote_mirror        *: (tao_shared_object*)(obj),          \
        tao_remote_sensor        *: (tao_shared_object*)(obj),          \
        tao_rwlocked_object      *: (tao_shared_object*)(obj),          \
        tao_shared_array         *: (tao_shared_object*)(obj),          \
        tao_shared_object   const*: (obj),                              \
        tao_remote_object   const*: (tao_shared_object const*)(obj),    \
        tao_remote_camera   const*: (tao_shared_object const*)(obj),    \
        tao_remote_mirror   const*: (tao_shared_object const*)(obj),    \
        tao_remote_sensor   const*: (tao_shared_object const*)(obj),    \
        tao_rwlocked_object const*: (tao_shared_object const*)(obj),    \
        tao_shared_array    const*: (tao_shared_object const*)(obj))
#endif /* TAO_DOXYGEN_ */

/**
 * @def tao_rwlocked_object_cast(obj)
 *
 * This macro casts a pointer to an object instance into a @ref
 * tao_rwlocked_object pointer.  Only objects of valid type are accepted.
 */
#ifdef TAO_DOXYGEN_
#define tao_rwlocked_object_cast(obj) ...
#else
#define tao_rwlocked_object_cast(obj)                                   \
    _Generic(                                                           \
        (obj),                                                          \
        tao_rwlocked_object      *: (obj),                              \
        tao_shared_array         *: (tao_rwlocked_object*)(obj),        \
        tao_rwlocked_object const*: (obj),                              \
        tao_shared_array    const*: (tao_rwlocked_object const*)(obj))
#endif /* TAO_DOXYGEN_ */

/**
 * @def tao_remote_object_cast(obj)
 *
 * This macro casts a pointer to an object instance into a @ref
 * tao_remote_object pointer.  Only objects of valid type are accepted.
 */
#ifdef TAO_DOXYGEN_
#define tao_remote_object_cast(obj) ...
#else
#define tao_remote_object_cast(obj)                                     \
    _Generic(                                                           \
        (obj),                                                          \
        tao_remote_object      *: (obj),                                \
        tao_remote_camera      *: (tao_remote_object*)(obj),            \
        tao_remote_mirror      *: (tao_remote_object*)(obj),            \
        tao_remote_sensor      *: (tao_remote_object*)(obj),            \
        tao_remote_object const*: (obj),                                \
        tao_remote_camera const*: (tao_remote_object const*)(obj),      \
        tao_remote_mirror const*: (tao_remote_object const*)(obj),      \
        tao_remote_sensor const*: (tao_remote_object const*)(obj))
#endif /* TAO_DOXYGEN_ */

//-----------------------------------------------------------------------------
// Generic methods for shared objects and descendants.

#define TAO_ANY_OBJECT_METHOD(verb, obj)                        \
    _Generic(                                                   \
        (obj),                                                  \
        tao_remote_camera        *: tao_remote_camera_##verb,   \
        tao_remote_camera   const*: tao_remote_camera_##verb,   \
        tao_remote_mirror        *: tao_remote_mirror_##verb,   \
        tao_remote_mirror   const*: tao_remote_mirror_##verb,   \
        tao_remote_object        *: tao_remote_object_##verb,   \
        tao_remote_object   const*: tao_remote_object_##verb,   \
        tao_remote_sensor        *: tao_remote_sensor_##verb,   \
        tao_remote_sensor   const*: tao_remote_sensor_##verb,   \
        tao_rwlocked_object      *: tao_rwlocked_object_##verb, \
        tao_rwlocked_object const*: tao_rwlocked_object_##verb, \
        tao_shared_array         *: tao_remote_object_##verb,   \
        tao_shared_array    const*: tao_remote_object_##verb,   \
        tao_shared_object        *: tao_shared_object_##verb,   \
        tao_shared_object   const*: tao_shared_object_##verb)

/**
 * @def tao_attach(T,shmid,...)
 *
 * Attach shared data/object instance to caller's address space.
 *
 * @param T         Type of shared data/object to attach.
 *
 * @param shmid     Shared memory identifier.
 *
 * @return A valid address on success; `NULL` in case of failure.
 */
#define tao_attach(T, ...) \
    TAO_ANY_OBJECT_METHOD(attach, (T*)0)(__VA_ARGS__)

/**
 * @def tao_detach(obj)
 *
 * Detach shared data/object instance from caller's address space.
 *
 * @param obj       Pointer to a shared data/object instance.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
#define tao_detach(obj) \
     TAO_ANY_OBJECT_METHOD(detach, obj)(obj)

/**
 * @def tao_get_shmid(obj)
 *
 * Query the shared memory identifier of a shared data/object instance.
 *
 * @param obj      Pointer to a shared data/object instance.
 */
#define tao_get_shmid(obj) \
    TAO_ANY_OBJECT_METHOD(get_shmid, obj)(obj)

/*
#define tao_get_type(obj) \
    TAO_ANY_OBJECT_METHOD(get_type, obj)(obj)

#define tao_get_size(obj) \
    TAO_ANY_OBJECT_METHOD(get_size, obj)(obj)
*/

//-----------------------------------------------------------------------------
// Generic methods for mutexes, shared objects and descendants excluding r/w
// locked objects.

#define TAO_MUTEX_OR_SHARED_OBJECT_METHOD(verb, obj)    \
    _Generic(                                           \
        (obj),                                          \
        tao_mutex        *: tao_mutex_##verb,           \
        tao_shared_object*: tao_shared_object_##verb,   \
        tao_remote_object*: tao_remote_object_##verb,   \
        tao_remote_camera*: tao_remote_camera_##verb,   \
        tao_remote_mirror*: tao_remote_mirror_##verb,   \
        tao_remote_sensor*: tao_remote_sensor_##verb)

/**
 * @def tao_lock(obj)
 *
 * Generic macro to lock the object pointed by `obj`.  The returned
 * value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_try_lock, tao_timed_lock, tao_abstimed_lock,
 *      tao_rdlock, tao_rwlock.
 */
#define tao_lock(obj) \
    TAO_MUTEX_OR_SHARED_OBJECT_METHOD(lock, obj)(obj)

/**
 * @def tao_try_lock(obj)
 *
 * Generic macro to try to immediately lock the object pointed by `obj`.  The
 * returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_lock, tao_timed_lock, tao_abstimed_lock,
 *      tao_try_rdlock, tao_try_wrlock.
 */
#define tao_try_lock(obj) \
    TAO_MUTEX_OR_SHARED_OBJECT_METHOD(try_lock, obj)(obj)

/**
 * @def tao_timed_lock(obj, secs)
 *
 * Generic macro to lock the object pointed by `obj` not waiting more than
 * `secs` seconds.  The returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_lock, tao_try_lock, tao_abstimed_lock,
 *      tao_timed_rdlock, tao_timed_wrlock.
 */
#define tao_timed_lock(obj, secs) \
    TAO_MUTEX_OR_SHARED_OBJECT_METHOD(timed_lock, obj)(obj, secs)

/**
 * @def tao_abstimed_lock(obj, secs)
 *
 * Generic macro to lock the object pointed by `obj` not exceeding the absolute
 * time limit `abstime`.  The returned value is a standard @ref tao_status
 * result.
 *
 * @see tao_unlock, tao_lock, tao_try_lock, tao_timed_lock,
 *      tao_abstimed_rdlock, tao_abstimed_wrlock.
 */
#define tao_abstimed_lock(obj, abstime) \
    TAO_MUTEX_OR_SHARED_OBJECT_METHOD(abstimed_lock, obj)(obj, abstime)

//-----------------------------------------------------------------------------
// Generic methods of r/w locks and r/w locked objects and descendants.

#define TAO_RWLOCK_METHOD(verb, obj)                            \
    _Generic(                                                   \
        (obj),                                                  \
        tao_rwlock         *: tao_rwlock_##verb,                \
        tao_rwlocked_object*: tao_rwlocked_object_##verb,       \
        tao_shared_array   *: tao_remote_object_##verb)

/**
 * @def tao_wrlock(obj)
 *
 * Generic macro to lock the object pointed by `obj` for read/write access.
 * The returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_try_wrlock, tao_timed_wrlock, tao_abstimed_wrlock,
 *      tao_rdlock.
 */
#define tao_wrlock(obj) \
    TAO_RWLOCK_METHOD(wrlock, obj)(obj)

/**
 * @def tao_try_wrlock(obj)
 *
 * Generic macro to try to immediately lock the object pointed by `obj` for
 * read/write access.  The returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_wrlock, tao_timed_wrlock, tao_abstimed_wrlock,
 *      tao_try_rdlock.
 */
#define tao_try_wrlock(obj) \
    TAO_RWLOCK_METHOD(try_wrlock, obj)(obj)

/**
 * @def tao_timed_wrlock(obj, secs)
 *
 * Generic macro to lock the object pointed by `obj` for read/write access not
 * waiting more than `secs` seconds.  The returned value is a standard @ref
 * tao_status result.
 *
 * @see tao_unlock, tao_wrlock, tao_try_wrlock, tao_abstimed_wrlock,
 *      tao_timed_rdlock.
 */
#define tao_timed_wrlock(obj, secs) \
    TAO_RWLOCK_METHOD(timed_wrlock, obj)(obj, secs)

/**
 * @def tao_abstimed_wrlock(obj, abstime)
 *
 * Generic macro to lock the object pointed by `obj` for read/write access not
 * exceeding the absolute time limit `abstime`.  The returned value is a
 * standard @ref tao_status result.
 *
 * @see tao_unlock, tao_wrlock, tao_try_wrlock, tao_timed_wrlock,
 *      tao_abstimed_rdlock.
 */
#define tao_abstimed_wrlock(obj, abstime) \
    TAO_RWLOCK_METHOD(abstimed_wrlock, obj)(obj, abstime)

/**
 * @def tao_rdlock(obj)
 *
 * Generic macro to lock the object pointed by `obj` for read only access.
 * The returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_try_rdlock, tao_timed_rdlock, tao_abstimed_rdlock,
 *      tao_wrlock.
 */
#define tao_rdlock(obj) \
    TAO_RWLOCK_METHOD(rdlock, obj)(obj)

/**
 * @def tao_try_rdlock(obj)
 *
 * Generic macro to try to immediately lock the object pointed by `obj` for
 * read only access.  The returned value is a standard @ref tao_status result.
 *
 * @see tao_unlock, tao_rdlock, tao_timed_rdlock, tao_abstimed_rdlock,
 *      tao_try_wrlock.
 */
#define tao_try_rdlock(obj) \
    TAO_RWLOCK_METHOD(try_rdlock, obj)(obj)

/**
 * @def tao_timed_rdlock(obj, secs)
 *
 * Generic macro to lock the object pointed by `obj` for read only access not
 * waiting more than `secs` seconds.  The returned value is a standard @ref
 * tao_status result.
 *
 * @see tao_unlock, tao_rdlock, tao_try_rdlock, tao_abstimed_rdlock,
 *      tao_timed_wrlock.
 */
#define tao_timed_rdlock(obj, secs) \
    TAO_RWLOCK_METHOD(timed_rdlock, obj)(obj, secs)

/**
 * @def tao_abstimed_rdlock(obj, abstime)
 *
 * Generic macro to lock the object pointed by `obj` for read only access not
 * exceeding the absolute time limit `abstime`.  The returned value is a
 * standard @ref tao_status result.
 *
 * @see tao_unlock, tao_rdlock, tao_try_rdlock, tao_timed_rdlock,
 *      tao_abstimed_wrlock.
 */
#define tao_abstimed_rdlock(obj, abstime) \
    TAO_RWLOCK_METHOD(abstimed_rdlock, obj)(obj, abstime)

//-----------------------------------------------------------------------------
// Generic methods for mutexes, shared objects and any descendants.

#define TAO_MUTEX_OR_OBJECT_METHOD(verb, obj)                   \
    _Generic(                                                   \
        (obj),                                                  \
        tao_mutex          *: tao_mutex_##verb,                 \
        tao_rwlock         *: tao_rwlock_##verb,                \
        tao_shared_object  *: tao_shared_object_##verb,         \
        tao_remote_object  *: tao_remote_object_##verb,         \
        tao_remote_camera  *: tao_remote_camera_##verb,         \
        tao_remote_mirror  *: tao_remote_mirror_##verb,         \
        tao_remote_snesor  *: tao_remote_sensor_##verb,         \
        tao_rwlocked_object*: tao_rwlocked_object_##verb,       \
        tao_shared_array   *: tao_remote_object_##verb)

/**
 * @def tao_unlock(obj)
 *
 * Generic macro to unlock the object pointed by `obj`.  The returned value is
 * a standard @ref tao_status result.
 *
 * @see tao_lock, tao_try_lock, tao_timed_lock, tao_abstimed_lock,
 *      tao_rdlock, tao_try_rdlock, tao_timed_rdlock, tao_abstimed_rdlock,
 *      tao_wrlock, tao_try_wrlock, tao_timed_wrlock, tao_abstimed_wrlock.
 */
#define tao_unlock(obj) \
    TAO_MUTEX_OR_OBJECT_METHOD(unlock, obj)(obj)

//-----------------------------------------------------------------------------

#define tao_get_owner(obj) \
    tao_remote_object_get_owner(tao_remote_object_cast(obj))

/**
 * @}
 */

#endif // TAO_GENERIC_H_
