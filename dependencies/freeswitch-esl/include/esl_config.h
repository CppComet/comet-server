/*
 * Copyright (c) 2007-2014, Anthony Minessale II
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @defgroup config Config File Parser
 * @ingroup config
 * This module implements a basic interface and file format parser
 * 
 * <pre>
 *
 * EXAMPLE 
 * 
 * [category1]
 * var1 => val1
 * var2 => val2
 * \# lines that begin with \# are comments
 * \#var3 => val3
 * </pre>
 * @{
 */

#ifndef ESL_CONFIG_H
#define ESL_CONFIG_H

#include "esl.h"

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */


#define ESL_URL_SEPARATOR "://"


#ifdef WIN32
#define ESL_PATH_SEPARATOR "\\"
#ifndef ESL_CONFIG_DIR
#define ESL_CONFIG_DIR "c:\\openesl"
#endif
#define esl_is_file_path(file) (*(file +1) == ':' || *file == '/' || strstr(file, SWITCH_URL_SEPARATOR))
#else
#define ESL_PATH_SEPARATOR "/"
#ifndef ESL_CONFIG_DIR
#define ESL_CONFIG_DIR "/etc/openesl"
#endif
#define esl_is_file_path(file) ((*file == '/') || strstr(file, SWITCH_URL_SEPARATOR))
#endif

/*!
  \brief Evaluate the truthfullness of a string expression
  \param expr a string expression
  \return true or false 
*/
static __inline__ int esl_true(const char *expr) {
	return (expr && (!strcasecmp(expr, "yes")
					 || !strcasecmp(expr, "on")
					 || !strcasecmp(expr, "true")
					 || !strcasecmp(expr, "enabled")
					 || !strcasecmp(expr, "active")
					 || !strcasecmp(expr, "allow")
					 || atoi(expr)));
}

/*!
  \brief Evaluate the falsefullness of a string expression
  \param expr a string expression
  \return true or false 
*/
static __inline__ int esl_false(const char *expr) {
	return (expr && (!strcasecmp(expr, "no")
					 || !strcasecmp(expr, "off")
					 || !strcasecmp(expr, "false")
					 || !strcasecmp(expr, "disabled")
					 || !strcasecmp(expr, "inactive")
					 || !strcasecmp(expr, "disallow")
					 || !atoi(expr)));
}

typedef struct esl_config esl_config_t;

/*! \brief A simple file handle representing an open configuration file **/
struct esl_config {
	/*! FILE stream buffer to the opened file */
	FILE *file;
	/*! path to the file */
	char path[512];
	/*! current category */
	char category[256];
	/*! current section */
	char section[256];
	/*! buffer of current line being read */
	char buf[1024];
	/*! current line number in file */
	int lineno;
	/*! current category number in file */
	int catno;
	/*! current section number in file */
	int sectno;

	int lockto;
};

/*!
  \brief Open a configuration file
  \param cfg (esl_config_t *) config handle to use
  \param file_path path to the file
  \return 1 (true) on success 0 (false) on failure
*/
ESL_DECLARE(int) esl_config_open_file(esl_config_t * cfg, const char *file_path);

/*!
  \brief Close a previously opened configuration file
  \param cfg (esl_config_t *) config handle to use
*/
ESL_DECLARE(void) esl_config_close_file(esl_config_t * cfg);

/*!
  \brief Retrieve next name/value pair from configuration file
  \param cfg (esl_config_t *) config handle to use
  \param var pointer to aim at the new variable name
  \param val pointer to aim at the new value
*/
ESL_DECLARE(int) esl_config_next_pair(esl_config_t * cfg, char **var, char **val);

/*!
  \brief Retrieve the CAS bits from a configuration string value
  \param strvalue pointer to the configuration string value (expected to be in format whatever:xxxx)
  \param outbits pointer to aim at the CAS bits
*/
ESL_DECLARE(int) esl_config_get_cas_bits(char *strvalue, unsigned char *outbits);


/** @} */

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* defined(ESL_CONFIG_H) */

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet:
 */
