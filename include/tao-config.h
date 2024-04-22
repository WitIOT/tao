// tao-config.c -
//
// Definitions for handling global configuration in TAO library.
//
//-----------------------------------------------------------------------------
//
// This file if part of TAO real-time software licensed under the MIT license
// (https://git-cral.univ-lyon1.fr/tao/tao-rt).
//
// Copyright (C) 2021-2022, Éric Thiébaut.

#ifndef TAO_CONFIG_H_
#define TAO_CONFIG_H_ 1

#include <tao-basics.h>
#include <tao-shared-memory.h>

#include <stdio.h>

TAO_BEGIN_DECLS

/**
 * @defgroup Configuration Configuration
 *
 * @ingroup Utilities
 *
 * @brief Management of global configuration parameters.
 *
 * TAO implement a simple mechanism to globally store configuration parameters.
 * Each parameter is written (in a human readable form) in a file.  The file
 * name has the form `"/tmp/tao/$name"` where `$name` is the name of the
 * parameter, possibly with `"/"` to separated parameters in groups.
 *
 * @{
 */

/**
 * @def TAO_CONFIG_DIR
 *
 * This macro exppands to the name of the directory where global TAO
 * configuration is saved.
 */
#define TAO_CONFIG_DIR "/tmp/tao"

/**
 * Open a file.
 *
 * This function is like `fopen` except that it is able to create intermediate
 * directories if the file is open in write mode and that, in case of failure,
 * it updates the last error information for of the calling thread.  The caller
 * is responsible of calling `fclose` or tao_file_close() to eventually close
 * the file.
 *
 * @param path   Path to the file.
 *
 * @param mode   Access mode for the file.  One of: `"r"`, `"r+"`, `"w"`,
 *               `"w+"`, `"a"`, or `"a+"`.
 *
 * @return A file pointer or `NULL` in case of error.
 */
extern FILE* tao_file_open(
    const char* path,
    const char* mode);

/**
 * Close a file.
 *
 * This function is like `fclose` except that it ignores a `NULL` argument
 * and,in case of failure, it updates the last error information for of the
 * calling thread.
 *
 * @param file   Address of open file (may be `NULL`).
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_file_close(
    FILE* file);

/**
 * Open a configuration file.
 *
 * This function opens the file storing a configuration parameter.  The caller
 * is responsible of calling `fclose` or tao_file_close() to eventually close
 * the file.
 *
 * @param name   Name of configuration parameter.
 *
 * @param mode   Access mode for the file.  One of: `"r"`, `"r+"`, `"w"`, `"w+"`,
 *               `"a"`, or `"a+"`.
 *
 * @return A file pointer or `NULL` in case of error.
 */
extern FILE* tao_config_open(
    const char* name,
    const char* mode);

/**
 * Concatenate configuration path.
 *
 * This function builds the path of the file storing a configuration parameter.
 *
 * @param path   Destination buffer.
 *
 * @param size   Size (in bytes) of destination buffer.
 *
 * @param name   Name of configuration parameter.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_config_path(
    char*       path,
    long        size,
    const char* name);

/**
 * Read integer configuration parameter.
 *
 * This function reads the value of an integer configuration parameter.
 *
 * @param name   Name of configuration parameter.
 *
 * @param ptr    Address to store parameter value.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_config_read_long(
    const char* name,
    long*       ptr);

/**
 * Write integer configuration parameter.
 *
 * This function writes the value of an integer configuration parameter.
 *
 * @param name   Name of configuration parameter.
 *
 * @param value  Parameter value.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_config_write_long(
    const char* name,
    long        value);

/**
 * Read shared memory identifier configuration parameter.
 *
 * This function reads the shared memory identifier saved as a configuration
 * parameter.  This function provides quick means to retrieve shared memory
 * identifier form some running server.  In case of failure, any errors are
 * discarded and @ref TAO_BAD_SHMID is returned.
 *
 * @param param  Name of configuration parameter.
 *
 * @return A shared memory identifier of @ref TAO_BAD_SHMID in case of failure.
 */
extern tao_shmid tao_config_read_shmid(
    const char* name);

/**
 * Read configuration parameter.
 *
 * This function reads the value of a configuration parameter.
 *
 * @param name   Name of configuration parameter.
 *
 * @param format Format string as in `scanf` function.
 *
 * @param ...    Address(es) to store parameter value(s).
 *
 * @return The number of scanned values, -1 in case of error.
 */
extern int tao_config_read(
    const char* name,
    const char* format,
    ...) TAO_FORMAT_SCANF(2, 3);

/**
 * Write configuration parameter.
 *
 * This function writes the value of a configuration parameter.  A newline
 * `'\n'` is automatically appended if the last character of the format string
 * is not a newline.
 *
 * @param name   Name of configuration parameter.
 *
 * @param format Format string as in `printf` function.
 *
 * @param ...    Parameter value.
 *
 * @return @ref TAO_OK on success; @ref TAO_ERROR in case of failure.
 */
extern tao_status tao_config_write(
    const char* name,
    const char* format,
    ...) TAO_FORMAT_PRINTF(2, 3);

/**
 * @}
 */

TAO_END_DECLS

#endif // TAO_CONFIG_H_
