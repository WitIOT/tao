// tao-options.h -
//
// Definitions for parsing of command line options in TAO.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2018-2022, Éric Thiébaut.

#ifndef TAO_OPTIONS_H_
#define TAO_OPTIONS_H_ 1

#include <tao-basics.h>
#include <tao-buffers.h>

#include <stdbool.h>
#include <stdio.h>

TAO_BEGIN_DECLS

/**
 * @addtogroup Commands
 *
 * @{
 *
 *     @addtogroup CommandLineOptions
 *
 *     @{
 */

/**
 * Structure describing a command line option.
 */
typedef struct tao_option  tao_option;
struct tao_option {
    int          pass; /**< When to process this option */
    const char*  name; /**< Name of option as it appears in the command line */
    int         nargs; /**< Number of expected arguments */
    const char*  args; /**< Description of argument(s) or NULL */
    const char* descr; /**< Description of option */
    void*         ptr; /**< Address to store argument value */
    void (*show)(FILE*, const tao_option*);
    /**<                    Callback to print argument(s) value */
    bool (*parse)(const tao_option*, char**);
    /**<                    Callback to parse (or apply) argument(s) */
};

/**
 * Structure expected by the tao_print_help() callback.
 */
typedef struct tao_help_info tao_help_info;
struct tao_help_info {
    const char*       program; /**< Program name */
    const char*          args; /**< Positional arguments (or `NULL`) */
    const char*       purpose; /**< Description of program */
    FILE*              output; /**< Output stream */
    const tao_option* options; /**< Table of options */
};

/**
 * Parse command line options.
 *
 * The tao_parse_options() function parses standard command line arguments
 * and process options listed in the option table.  On return, parsed
 * options are removed from the argument list.
 *
 * The tao_parse_options() function may be called several times with different
 * increasing values of argument @b pass.  Each call only consider options in
 * the list whose @b pass members equal the value of argument @b pass.  In the
 * last pass, if the first remaining argument is `"--"`, it is removed from the
 * list so that the count returned by the last pass is the number of positional
 * arguments plus one and @b argv only contains the name of the program (in
 * `argv[0]`) and the positional arguments (in `argv[1]`, `argv[2]`, ...).
 *
 * Each entry of the option table given by @b options gives:
 *
 * - The pass number when the option shall be taken into account.
 * - The name of the option as it appears in the command line but without
 *   leading dash(es).
 * - The number of arguments taken by the option.
 * - A textual description of the arguments.
 * - A textual description of the option.
 * - A textual description of the positional arguments (to be printed in the
 *   @i usage line) or `NULL` if none.
 * - A pointer to related contextual data.  In general, the address of a
 *   variable to store the option value.
 * - A callback to print the option value to a given stream (used by the
 *   tao_print_help() callback).  This callback can be `NULL` if there is
 *   nothing to print (e.g. the help option).
 * - A callback to parse or apply the option; this callback is called with a
 *   pointer to the `tao_option` structure describing the option and an
 *   argument list starting at the first argument after the option in the
 *   command line (it is guaranteed by the caller that the correct number of
 *   arguments is available).  This callback shall return a boolean value
 *   indicating whether the option argument(s) were correct.
 *
 * A number of callbacks are provided by TAO library.  These callbacks are
 * named `tao_show_..._option`, `tao_parse_..._option` and tao_print_help().
 *
 * @param msg      Address of dynamic buffer to store error messages.  If
 *                 `NULL`, any error message is printed to the standard
 *                 ouput error.
 * @param argc     Number of arguments.
 * @param argv     List of arguments.
 * @param pass     Pass number.
 * @param options  Table of possible options.  Last entry is indicated
 *                 by a option with a `NULL` name.
 *
 * @return The number of remaining arguments, `-1` on error.
 */

extern int tao_parse_options(
    tao_buffer* msg,
    int argc,
    char* argv[],
    int pass,
    const tao_option options[]);

/**
 * Callback to print the help.
 *
 * @param opt    Address of option in table of options.  The pointer
 *               `opt->ptr` shall give the address of a `tao_help_info`
 *               structure.
 * @param args   Arguments after the option.
 *
 * @return `true`.
 */
extern bool tao_print_help(
    const tao_option* opt,
    char* args[]);

/**
 * Callback to print the help.
 *
 * This callback is identical to tao_print_help() but calls `exit(0)` and
 * thus never returns.
 *
 * @return Never.
 */
extern bool tao_print_help_and_exit0(
    const tao_option* opt,
    char* args[]) TAO_NORETURN;

/**
 * Callback to print the help.
 *
 * This callback is identical to tao_print_help() but calls `exit(1)` and
 * thus never returns.
 *
 * @return Never.
 */
extern bool tao_print_help_and_exit1(
    const tao_option* opt,
    char* args[]) TAO_NORETURN;

/**
 * Show the state of a switch option.
 */
extern void tao_show_switch_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse a switch option.
 *
 * A switch option takes no argument.  It is associated to a boolean value.  If
 * the option is present in the command line, the associated value is set true.
 */
extern bool tao_parse_switch_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the state of a toggle option.
 */
extern void tao_show_toggle_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse a toggle option.
 *
 * A toggle option takes no argument.  It is associated to a boolean value.  If
 * the option is present in the command line, the associated value is toggled.
 */
extern bool tao_parse_toggle_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the help.
 *
 * @param file   Output stream.
 * @param opt    Address of option in table of options.  The pointer
 *               `opt->ptr` shall gives the address of the option table.
 */
extern void tao_show_help(
    FILE* output,
    const tao_option* opt);

/**
 * Show the value of an option taking a single string argument.
 */
extern void tao_show_string_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single string argument.
 */
extern bool tao_parse_string_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the value of an option taking a single yes/no argument.
 */
extern void tao_show_yesno_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single yes/no argument.
 */
extern bool tao_parse_yesno_option(
    const tao_option* opt,
    char* args[]);
/**
 * Show the value of an option taking a single integer argument.
 */
extern void tao_show_int_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single integer argument.
 */
extern bool tao_parse_int_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single nonnegative integer argument.
 */
extern bool tao_parse_nonnegative_int_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single positive integer argument.
 */
extern bool tao_parse_positive_int_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the value of an option taking a single long integer argument.
 */
extern void tao_show_long_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single long integer argument.
 */
extern bool tao_parse_long_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single nonnegative long integer
 * argument.
 */
extern bool tao_parse_nonnegative_long_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single positive long integer
 * argument.
 */
extern bool tao_parse_positive_long_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the value of an option taking a single double precision floating-point
 * argument.
 */
extern void tao_show_double_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single double precision floating-point
 * argument.
 */
extern bool tao_parse_double_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single nonnegative double precision
 * floating-point argument.
 */
extern bool tao_parse_nonnegative_double_option(
    const tao_option* opt,
    char* args[]);

/**
 * Parse the value of an option taking a single positive double precision
 * floating-point argument.
 */
extern bool tao_parse_positive_double_option(
    const tao_option* opt,
    char* args[]);

/**
 * Show the value of an option taking a single region of interest argument.
 */
extern void tao_show_roi_option(
    FILE* file,
    const tao_option* opt);

/**
 * Parse the value of an option taking a single region of interest argument.
 */
extern bool tao_parse_roi_option(
    const tao_option* opt,
    char* args[]);

//-----------------------------------------------------------------------------
// OPTION HELPERS

// Last entry in option table.
#define TAO_OPTION_LAST_ENTRY {0, NULL, 0, NULL, NULL, NULL, NULL, NULL}

// Helper macro for help option.
#define TAO_OPTION_HELP_AND_EXIT(pass, n) \
    {pass, "help", 0, NULL, "Print this help", \
     NULL, NULL, tao_print_help_and_exit##n}

// Helper macro for switch options.
#define TAO_OPTION_SWITCH(pass, name, descr, addr) \
    {pass, name, 0, NULL, descr, addr, \
     tao_show_switch_option, tao_parse_switch_option}

// Helper macro for toggle options.
#define TAO_OPTION_TOGGLE(pass, name, descr, addr) \
    {pass, name, 0, NULL, descr, addr, \
     tao_show_toggle_option, tao_parse_toggle_option}

// Helper macro for options taking a single yes/no argument.
#define TAO_OPTION_YESNO(pass, name, descr, addr) \
    {pass, name, 1, "yes|no", descr, addr, \
     tao_show_yesno_option, tao_parse_yesno_option}

// Helper macro for options taking a single string argument.
#define TAO_OPTION_STRING(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_string_option, tao_parse_string_option}

// Helpers for options taking a single integer argument.
#define TAO_OPTION_INT(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_int_option, tao_parse_int_option}
#define TAO_OPTION_NONNEGATIVE_INT(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_int_option, tao_parse_nonnegative_int_option}
#define TAO_OPTION_POSITIVE_INT(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_int_option, tao_parse_positive_int_option}

// Helpers for options taking a single long integer argument.
#define TAO_OPTION_LONG(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_long_option, tao_parse_long_option}
#define TAO_OPTION_NONNEGATIVE_LONG(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_long_option, tao_parse_nonnegative_long_option}
#define TAO_OPTION_POSITIVE_LONG(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_long_option, tao_parse_positive_long_option}

// Helpers for options taking a single double precision argument.
#define TAO_OPTION_DOUBLE(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_double_option, tao_parse_double_option}
#define TAO_OPTION_NONNEGATIVE_DOUBLE(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_double_option, tao_parse_nonnegative_double_option}
#define TAO_OPTION_POSITIVE_DOUBLE(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_show_double_option, tao_parse_positive_double_option}

// Helpers for options taking a single camera ROI argument.
#define TAO_OPTION_CAMERA_ROI(pass, name, args, descr, addr) \
    {pass, name, 1, args, descr, addr, \
     tao_camera_roi_option_show, tao_camera_roi_option_parse}

/**
 *     @}
 * @}
 */

TAO_END_DECLS

#endif // TAO_OPTIONS_H_
