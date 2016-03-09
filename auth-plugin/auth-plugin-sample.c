/*
 * auth-plugin-sample
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved.
 *
 * PROPRIETARY/CONFIDENTIAL
 *
 * This software is the confidential and proprietary information of SAMSUNG ELECTRONICS
 * ("Confidential Information"). You agree and acknowledge that this software is owned by
 * Samsung and you shall not disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into with SAMSUNG
 * ELECTRONICS.SAMSUNG make no representations or warranties about the suitability of
 * the software, either express or implied, including but not limited to the implied warranties
 * of merchantability, fitness for a particular purpose, or non-infringement.SAMSUNG shall not
 * be liable for any damages suffered by licensee arising out of or related to this software.
 *
 */

/* System */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 3rd Party */
#include <glib.h>
#include <bundle.h>

/* Auth Plugin */
#include <auth-adaptor.h>
#include "auth-plugin-sample-log.h"

#define SAMPLE_PACKAGE_NAME "org.tizen.service-plugin-sample"

/**
* @brief Create Auth Plugin context
*
* @param[out]	context		specifies Auth Plugin context
* @param[in]	app_id		specifies app id
* @param[in]	app_secret	specifies app secret
* @param[in]	imsi		specifies International mobile subscriber identity
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_create_context(auth_adaptor_plugin_context_h *context,
						const char *user_id,
						const char *user_password,
						const char *app_id,
						const char *app_secret,
						const char *imsi)
{
	auth_plugin_sample_debug("Create sample auth plugin context");

	auth_adaptor_plugin_context_h _context = (auth_adaptor_plugin_context_h) calloc(1, sizeof(auth_adaptor_plugin_context_t));

	_context->user_id       = strdup(user_id ? user_id : "");
	_context->user_password = strdup(user_password ? user_password : "");
	_context->app_id        = strdup(app_id ? app_id : "");
	_context->app_secret    = strdup(app_secret ? app_secret : "");
	_context->imsi		= strdup(imsi ? imsi : "");

	_context->plugin_data = NULL;

	_context->access_token = NULL;
	_context->refresh_token = NULL;
	_context->uid = NULL;

	*context = _context;

	return AUTH_ADAPTOR_ERROR_NONE;
}


/**
* @brief Destroy Auth Plugin context
*
* @param[in]	context		specifies Auth Plugin context
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_destroy_context(auth_adaptor_plugin_context_h context)
{
	auth_plugin_sample_debug("Destroy sample auth plugin context");

	if (NULL == context) {
		auth_plugin_sample_error("Invalid argument");
		return AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	/* Free context members */
	free(context->app_id);
	free(context->app_secret);
	free(context->user_id);
	free(context->user_password);
	free(context->imsi);

	free(context->plugin_uri);

	free(context->access_token);
	free(context->refresh_token);
	free(context->uid);

	/* Free context */
	free(context);

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Destroy Auth Plugin handle
*
* @param[in]	handle		specifies Auth Plugin handle
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_destroy_plugin_handle(auth_adaptor_plugin_handle_h handle)
{
	auth_plugin_sample_debug("Destory sample auth plugin handle");

	if (NULL == handle) {
		auth_plugin_sample_error("Invalid argument");
		return AUTH_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	/* Free */
	free(handle->plugin_uri);
	free(handle);

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Register Auth Plugin listener
*
* @param[in]	listener	specifies Auth Plugin listener
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_register_listener(auth_adaptor_plugin_listener_h listener)
{
	auth_plugin_sample_debug("Set sample auth plugin listener");

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Unregister Auth Plugin listener
*
* @param[in]	listener	specifies Auth Plugin listener
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_unregister_listener(void)
{
	auth_plugin_sample_debug("Unset sample auth plugin listener");

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Check whether authenticated or not
*
* @param[in]	context		specifies Auth Plugin context
* @param[in]	request		specifies optional request parameter
* @param[out]	is_auth		specifies whether authenticated or not
* @param[out]	error		specifies error code & message
* @param[out]	response	specifies optional response parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_is_auth(auth_adaptor_plugin_context_h context,
						void *request,
						int *is_auth,
						auth_adaptor_error_code_h *error,
						void *response)
{
	auth_plugin_sample_debug("Is auth to sample auth plugin");

	if (context) {
		auth_plugin_sample_debug_func("============= param debug =============");
		auth_plugin_sample_debug_func("user_id (%s)", context->user_id);
	/*	auth_plugin_sample_debug_func("user_pw (%s)", context->user_password);*/
		auth_plugin_sample_debug_func("app_id  (%s)", context->app_id);
	/*	auth_plugin_sample_debug_func("app_sec (%s)", context->app_secret);*/
	/*	auth_plugin_sample_debug_func("imsi    (%s)", context->imsi);*/
		auth_plugin_sample_debug_func("svc_nm  (%s)", context->service_name);
		auth_plugin_sample_debug_func("=======================================");
	}

	*is_auth = 1;

	return AUTH_ADAPTOR_ERROR_NONE;
}


/**
* @brief Login to Easy Signup (Get access token)
*
* @param[in]	context		specifies Auth Plugin context
* @param[in]	request		specifies optional request parameter
* @param[out]	error		specifies error code & message
* @param[out]	response	specifies optional response parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_join(auth_adaptor_plugin_context_h context,
						const char *did,
						void *request,
						auth_adaptor_error_code_h *error,
						void *response)
{
	auth_plugin_sample_debug("Join to sample auth plugin");

	if (context) {
		auth_plugin_sample_debug_func("============= param debug =============");
		auth_plugin_sample_debug_func("dev_id  (%s)", did);
		auth_plugin_sample_debug_func("user_id (%s)", context->user_id);
	/*	auth_plugin_sample_debug_func("user_pw (%s)", context->user_password);*/
		auth_plugin_sample_debug_func("app_id  (%s)", context->app_id);
	/*	auth_plugin_sample_debug_func("app_sec (%s)", context->app_secret);*/
	/*	auth_plugin_sample_debug_func("imsi    (%s)", context->imsi);*/
		auth_plugin_sample_debug_func("svc_nm  (%s)", context->service_name);
		auth_plugin_sample_debug_func("=======================================");
	}

	auth_plugin_sample_debug_func("Join success");
	return AUTH_ADAPTOR_ERROR_NONE;
}

auth_error_code_t sample_login(auth_adaptor_plugin_context_h context,
						void *request,
						auth_adaptor_error_code_h *error,
						void *response)
{
	auth_plugin_sample_debug("Login to sample auth plugin");

	auth_plugin_sample_info("request : %p", request);
	if (request) {
		bundle *bd = (bundle *) request;
		char *str = NULL;

		bundle_get_str(bd, "http://tizen.org/service-adaptor/plugin/property/app_id", &str);
		auth_plugin_sample_info("property [app_id][%s]", str);
	}

	context->access_token	= strdup("sample_access");
	context->refresh_token	= strdup("sample_refresh");
	context->uid		= strdup("sample_uniqueID");

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
	* @brief Login to Easy Signup if access token is expired(Get new access token)
	*
	* @param[in]	context		specifies Auth Plugin context
	* @param[in]	request		specifies optional request parameter
	* @param[out]	error		specifies error code & message
	* @param[out]	response	specifies optional response parameter
	* @return	0 on success, otherwise a positive error value
	* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
	*/
auth_error_code_t sample_refresh_access_token(auth_adaptor_plugin_context_h context,
						void *request,
						auth_adaptor_error_code_h *error,
						void *response)
{
	auth_plugin_sample_debug("Refresh Login to sample auth plugin");

	free(context->access_token);
	context->access_token = strdup("sample_access2");

	return AUTH_ADAPTOR_ERROR_NONE;
}

/**
* @brief Get server information
*
* @param[in]	context		specifies Auth Plugin context
* @param[in]	request		specifies optional request parameter
* @param[out]	server_info	specifies server info table
* @param[out]	error		specifies error code & message
* @param[out]	response	specifies optional response parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in auth_error_code_t - AUTH_ADAPTOR_ERROR_NONE if Successful
*/
auth_error_code_t sample_get_server_info(auth_adaptor_plugin_context_h context,
						void *request,
						GHashTable **server_info,
						auth_adaptor_error_code_h *error,
						void *response)
{
	auth_plugin_sample_debug("Get server info from sample auth plugin");
	return 0;
}

/**
* @brief Create Auth Plugin handle
*
* @return Auth Plugin handle on success, otherwise NULL value
*/
EXPORT_API auth_adaptor_plugin_handle_h create_plugin_handle(void)
{
	auth_plugin_sample_debug("Create sample plugin handle");

	auth_adaptor_plugin_handle_h handle =
			(auth_adaptor_plugin_handle_h) calloc(1, sizeof(auth_adaptor_plugin_handle_t));

	if (NULL == handle) {
		auth_plugin_sample_error("Memory allocation failed");
		return NULL;
	}

	/* Mandatory functions */
	handle->create_context		= sample_create_context;
	handle->destroy_context		= sample_destroy_context;
	handle->destroy_handle		= sample_destroy_plugin_handle;
	handle->set_listener		= sample_register_listener;
	handle->unset_listener		= sample_unregister_listener;

	/* Optional functions */
	handle->is_auth			= sample_is_auth;
	handle->login			= sample_login;
	handle->join			= sample_join;
	handle->refresh_access_token	= sample_refresh_access_token;
	handle->get_server_info		= sample_get_server_info;

	/* Plugin name */
	handle->plugin_uri = strdup(SAMPLE_PACKAGE_NAME);

	return handle;
}
