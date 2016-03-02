/*
 * storage-plugin-sample
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

// System header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <storage-adaptor.h>
#include "storage-plugin-sample-log.h"

// 3rd
#include <glib.h>

#define SAMPLE_PACKAGE_NAME		"org.tizen.service-plugin-sample"
#define VIRTUAL_ROOT_PATH_BASE	"/tmp/service-plugin-sample/"
#define VIRTUAL_ROOT_PATH(context)	((char *)context->plugin_data)

typedef struct async_job
{
	int fd;
	storage_adaptor_transfer_state_e state;
	char *local_path;
	char *cloud_path;
} async_job_t;

static async_job_t *create_job_s(int fd)
{
	async_job_t *job = (async_job_t *) calloc(1, sizeof(async_job_t));
	if (job) {
		job->local_path = NULL;
		job->cloud_path = NULL;
		job->fd = fd;
	}

	return job;
}

static void destroy_job_s(async_job_t *job)
{
	if (job) {
		free(job->local_path);
		free(job->cloud_path);
		free(job);
	}
}

static GList *g_job_queue = NULL;

static void start_job(int fd)
{
	g_job_queue = g_list_append(g_job_queue, (void *)(intptr_t)fd);
}

static void cancel_job(int fd)
{
	g_job_queue = g_list_remove(g_job_queue, (void *)(intptr_t)fd);
}

static bool is_job_progress(int fd)
{
	GList *target = NULL;
	target = g_list_find(g_job_queue, (void *)(intptr_t)fd);

	if (NULL == target)
		return false;
	return true;
}

static storage_adaptor_file_info_h create_chunk_file_info(storage_adaptor_plugin_context_h context, int content_type, char *path)
{
	storage_adaptor_file_info_h _file_info = storage_adaptor_create_file_info();

	if (_file_info) {
		_file_info->media_meta->mime_type          = strdup("Application/octet-stream");
		_file_info->media_meta->title              = strdup(path);
		_file_info->media_meta->album              = strdup("chunk");
		_file_info->media_meta->artist             = strdup("chunk");
		_file_info->media_meta->genere             = strdup("chunk");
		_file_info->media_meta->recorded_date      = strdup("chunk");
		_file_info->media_meta->width              = 1;
		_file_info->media_meta->height             = 177;
		_file_info->media_meta->duration           = 28;
		_file_info->media_meta->copyright          = strdup("chunk");
		_file_info->media_meta->track_num          = strdup("chunk");
		_file_info->media_meta->description        = strdup("chunk");
		_file_info->media_meta->composer           = strdup("chunk");
		_file_info->media_meta->year               = strdup("2016");
		_file_info->media_meta->bitrate            = 19200;
		_file_info->media_meta->samplerate         = 8000;
		_file_info->media_meta->channel            = 2;
		_file_info->media_meta->extra_media_meta   = strdup("chunk");
		_file_info->cloud_meta->service_name       = strdup("chunk");
		_file_info->cloud_meta->usage_byte         = 4096ULL;
		_file_info->cloud_meta->quota_byte         = 1048576ULL;
		_file_info->cloud_meta->extra_cloud_meta   = strdup("chunk");

		_file_info->plugin_uri          = strdup(SAMPLE_PACKAGE_NAME);
		_file_info->object_id           = strdup(path + strlen(VIRTUAL_ROOT_PATH(context)) - 1);
		_file_info->storage_path        = strdup(path + strlen(VIRTUAL_ROOT_PATH(context)) - 1);

		_file_info->file_size           = 4096ULL;
		_file_info->created_time        = 628185600ULL;
		_file_info->modified_time       = 1391731200ULL;
		_file_info->file_info_index     = 0;
		_file_info->content_type        = content_type;
		_file_info->extra_file_info     = strdup("chunk");
	}

	return _file_info;
}

static char *create_virtual_area(char *app_id)
{
	mkdir(VIRTUAL_ROOT_PATH_BASE, S_IRWXU|S_IRWXG|S_IRWXO);
	storage_plugin_sample_debug("Make root base path");

	char *subdir = "";
	if (NULL == app_id)
		subdir = "public";
	else if ('\0' == app_id[0])
		subdir = "public";
	else
		subdir = app_id;

	char *private_path = g_strconcat(VIRTUAL_ROOT_PATH_BASE, subdir, "/", NULL);
	mkdir(private_path, S_IRWXU|S_IRWXG|S_IRWXO);
	storage_plugin_sample_debug("Make private root path : %s", private_path);

	return private_path;
}

/**
* @brief Create Storage Plugin context
*
* @param[out]	context		specifies Storage Plugin context
* @param[in]	app_id		specifies app id
* @param[in]	app_secret	specifies app secret
* @param[in]	access_token	specifies access token
* @param[in]	cid		specifies client id issued by sample
* @param[in]	uid		specifies user id issued by Easy Signup(duid)
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_create_context(storage_adaptor_plugin_context_h *context,
						const char *app_id,
						const char *app_secret,
						const char *access_token,
						const char *cid,
						const char *uid)
{
	storage_plugin_sample_debug("Create sample storage plugin context");

	storage_adaptor_plugin_context_h _context =
			(storage_adaptor_plugin_context_h) calloc(1, sizeof(storage_adaptor_plugin_context_t));

	if (NULL == _context) {
		storage_plugin_sample_error("Memory allocation failed");
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	_context->app_id		= strdup(app_id ? app_id : "");
	_context->app_secret		= strdup(app_secret ? app_secret : "");
	_context->access_token		= strdup(access_token ? access_token : "");
	_context->cid			= strdup(cid ? cid : "");
	_context->uid			= strdup(uid ? uid : "");

	_context->plugin_data	= (void *)create_virtual_area(app_id ? app_id : "public");

	*context = _context;

	return STORAGE_ADAPTOR_ERROR_NONE;
}

/**
* @brief Destroy Storage Plugin context
*
* @param[in]	context		specifies Storage Plugin context
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_destroy_context(storage_adaptor_plugin_context_h context)
{
	storage_plugin_sample_debug("Destroy sample storage plugin context");

	if (NULL == context)
	{
		storage_plugin_sample_error("Invalid argument");
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	// Free context members
	free(context->app_id);
	free(context->app_secret);
	free(context->access_token);
	free(context->cid);
	free(context->uid);

	// Free context
	free(context);

	return STORAGE_ADAPTOR_ERROR_NONE;
}

/**
* @brief Destroy Storage Plugin handle
*
* @param[in]	handle		specifies Storage Plugin handle
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_destroy_plugin_handle(storage_adaptor_plugin_handle_h handle)
{
	storage_plugin_sample_debug("Destroy sample storage plugin handle");

	if (NULL == handle)
	{
		storage_plugin_sample_error("Invalid argument");
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(handle->plugin_uri);
	free(handle);

	return STORAGE_ADAPTOR_ERROR_NONE;
}

storage_adaptor_plugin_task_progress_reply_cb sample_progress_reply;

void sample_progress_cb(int fd, unsigned long long progress, unsigned long long total, storage_adaptor_error_code_h error, void *user_data)
{
	sample_progress_reply(fd, progress, total, error, user_data);
}

storage_adaptor_plugin_upload_state_changed_reply_cb sample_upload_reply;

void sample_upload_cb(int fd,
		storage_adaptor_transfer_state_e state,
		storage_adaptor_file_info_h file_info,
		storage_adaptor_error_code_h error,
		void *user_data)
{
	sample_upload_reply(fd, state, file_info, error, user_data);
}

storage_adaptor_plugin_download_state_changed_reply_cb sample_download_reply;

void sample_download_cb(int fd,
		storage_adaptor_transfer_state_e state,
		storage_adaptor_error_code_h error,
		void *user_data)
{
	sample_download_reply(fd, state, error, user_data);
}


/**
* @brief Register Storage Plugin listener
*
* @param[in]	listener	specifies Storage Plugin listener
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_register_listener(storage_adaptor_plugin_listener_h listener)
{
	storage_plugin_sample_debug("Set sample storage plugin listener");

	if (NULL != listener)
	{
		sample_progress_reply = listener->storage_adaptor_task_progress_reply;
		sample_upload_reply = listener->storage_adaptor_upload_state_changed_reply;
		sample_download_reply = listener->storage_adaptor_download_state_changed_reply;
	}

	return 0;
}

/**
* @brief Unregister Storage Plugin listener
*
* @param[in]	listener	specifies Storage Plugin listener
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_unregister_listener(void)
{
	storage_plugin_sample_debug("Unset sample storage plugin listener");

	return 0;
}

/**
* @brief Set server information for Storage Plugin
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	server_info			specifies server information for Storage Plugin
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_set_server_info(storage_adaptor_plugin_context_h context,
						GHashTable *server_info,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Set server info");

	return STORAGE_ADAPTOR_ERROR_NONE;
}

/**
* @brief Makes a directory at cloud
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies path to locate the folder you want to create
* @param[in]	folder_name			specifies folder name to be created at cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_make_directory(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Make directory");

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), folder_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", folder_name, NULL);
	storage_plugin_sample_debug("Make path (%s)", path);
	int ret = mkdir(path, 0777);
	storage_plugin_sample_debug("Make path (%d)", ret);

	if (0 == ret)
	{

		storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, path);

		*file_info = _file_info;
	}
	else
	{
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(path);

	return ret;
}

/**
* @brief Removes a directory at cloud
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to delete
* @param[in]	folder_name			specifies folder name to be deleted from cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_remove_directory(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Remove directory");

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), folder_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", folder_name, NULL);
	storage_plugin_sample_debug("Remove path (%s)", path);
	int ret = remove(path);
	storage_plugin_sample_debug("Remove ret (%d)", ret);

	if (0 == ret)
	{
		storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, path);

		*file_info = _file_info;
	}
	else
	{
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(path);

	return ret;
}

char *replace_path_str(char *original)
{
	char *base = strdup(original);
	char *slash = "//";
	char *pt = NULL;

	char *temp1 = NULL, *temp2 = NULL;

	while(NULL != (pt = strstr(base, slash))) {
		temp1 = strndup(base, strlen(base) - strlen(pt));
		temp2 = strndup(pt + 1, strlen(pt) - 1);
		free(base);
		base = g_strconcat(temp1, temp2, NULL);
		free(temp1);
		free(temp2);
		temp1 = temp2 = NULL;
	}

	return base;
}

/**
* @brief Requests folder and file list in a folder
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to get list
* @param[in]	folder_name			specifies folder name you want to get list
* @param[in]	request				specifies optional parameter
* @param[out]	file_info_list			specifies Storage Adaptor File Info handle
* @param[out]	file_info_list_len		specifies length of the file_info_list
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_get_list(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *folder_name,
						void *request,
						storage_adaptor_file_info_h **file_list,
						int *file_list_len,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Get list information");
	if ((NULL == file_list) || (NULL == file_list_len)) {
		return STORAGE_ADAPTOR_ERROR_INVALID_ARGUMENT;
	}

	storage_plugin_sample_debug("parent_path (%s) folder_name(%s)", parent_path, folder_name);

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), folder_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", folder_name, NULL);

	storage_plugin_sample_debug("gets path (%s)", path);

	DIR *dirp = NULL;
	struct dirent dent, *result = NULL;

	dirp = opendir(path);
	if (NULL == dirp) {
		storage_plugin_sample_error("dir open error");
		free(path);
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	GList *_file_list = NULL;
	char *entry_path = NULL;
	while (0 == readdir_r(dirp, &dent, &result)) {
		if (NULL == result) {
			break;
		}
		storage_plugin_sample_debug("===== entry name [%s]", dent.d_name);
		storage_plugin_sample_debug("entry type [%d]", (int)((unsigned char)dent.d_type));

		if ((0 == strcmp(".", dent.d_name)) || (0 == strcmp("..", dent.d_name))) {
			continue;
		}

		storage_adaptor_file_info_h file = storage_adaptor_create_file_info();
		if (4 == (int)((unsigned char)dent.d_type)) {
			file->content_type	= STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER;
		} else {
			file->content_type	= STORAGE_ADAPTOR_CONTENT_TYPE_OTHER;
		}

		entry_path = g_strconcat(path, "/", dent.d_name, NULL);
		char *temp = replace_path_str(entry_path);
		free(entry_path);
		entry_path = temp;
		storage_plugin_sample_info("entry path (%s) open", entry_path);
		file->object_id           = strdup(entry_path + strlen(VIRTUAL_ROOT_PATH(context)) - 1);
		file->storage_path        = strdup(entry_path + strlen(VIRTUAL_ROOT_PATH(context)) - 1);
		file->extra_file_info     = strdup(dent.d_name);
		storage_plugin_sample_info("file->storage_path (%s)", file->storage_path);

		struct stat sbuf;
		int ret = stat(entry_path, &sbuf);
		storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
		file->file_size = (unsigned long long) sbuf.st_size;
		storage_plugin_sample_debug("entry size (%llu)", (unsigned long long) sbuf.st_size);

		/*
		DIR *ent_dirp = NULL;
		ent_dirp = opendir(entry_path);
		if (NULL == ent_dirp) {
			file->content_type	= STORAGE_ADAPTOR_CONTENT_TYPE_OTHER;
			storage_plugin_sample_debug("entry type (%s)", "file");
			closedir(ent_dirp);
		} else {
			file->content_type	= STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER;
			storage_plugin_sample_debug("entry type (%s)", "folder");
			closedir(ent_dirp);
		}
		*/

		g_free(entry_path);
		entry_path = NULL;

		_file_list = g_list_append(_file_list, (void *)file);
	}

	g_free(path);
	closedir(dirp);

	storage_adaptor_file_info_h *list = NULL;
	int len = g_list_length(_file_list);
	if (0 < len) {
		list = (storage_adaptor_file_info_h *) calloc(len, sizeof(storage_adaptor_file_info_h));
		for (int i = 0; i < len; i++) {
			list[i] = (storage_adaptor_file_info_h) g_list_nth_data(_file_list, i);
		}
	}

	*file_list = list;
	*file_list_len = len;
	g_list_free(_file_list);

	return 0;
}

/**
* @brief Uploads a file to cloud (Sync)
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to upload
* @param[in]	file_name			specifies file name to be uploaded to cloud
* @param[in]	upload_file_local_path		specifies local path of the file to be uploaded
* @param[in]	publish				specifies Allow to share file with no authentication
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_upload_file_sync(storage_adaptor_plugin_context_h context,
						const char *dir_path,
						const char *file_name,
						const char *upload_file_local_path,
						const int publish,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Upload file synchronously");

	int src = open(upload_file_local_path, O_RDONLY);
	if (0 > src)
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), dir_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), file_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), dir_path?dir_path:"", "/", file_name, NULL);
	storage_plugin_sample_info("dir path: (%s)", dir_path);
	storage_plugin_sample_info("file name: (%s)", file_name);

	struct stat sbuf;
	storage_plugin_sample_debug("cloud_path (%s)", path);
	int ret = fstat(src, &sbuf);
	storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
	unsigned long long total = 0ULL;
	total = (unsigned long long) sbuf.st_size;
	storage_plugin_sample_debug("total size (%llu)", total);

	remove(path);
	int dst = open(path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	storage_plugin_sample_debug("open result (%d)", dst);

	char buf[1025] = {0, };
	int len = 0, wlen = 0;

	while (0 < (len = read(src, buf, 1024))){
		storage_plugin_sample_debug("len(%d) str(%s)", len, buf);

		wlen = write(dst, buf, len);
		storage_plugin_sample_debug("written len(%d)", wlen);

		memset(buf, 0, 1025);
	}

	close(dst);
	close(src);

	storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, path);

	*file_info = _file_info;

	free(path);

	return 0;
}


/**
* @brief Downloads a file to local (Sync)
*
* @param[in]	context				specifies Storage Adaptor context
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to download
* @param[in]	file_name			specifies file name to be downloaded to local
* @param[in]	download_file_local_path	specifies local path to download
* @param[in]	request				specifies optional parameter
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_download_file_sync(storage_adaptor_plugin_context_h context,
						const char *dir_path,
						const char *file_name,
						const char *download_file_local_path,
						void *request,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Download file synchronously");

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), dir_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), file_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), dir_path?dir_path:"", "/", file_name, NULL);

	int src = open(path, O_RDONLY);
	if (0 > src)
	{
		free(path);
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	struct stat sbuf;
	storage_plugin_sample_debug("cloud_path (%s)", path);
	int ret = fstat(src, &sbuf);
	storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
	unsigned long long total = 0ULL;
	total = (unsigned long long) sbuf.st_size;
	storage_plugin_sample_debug("total size (%llu)", total);

	remove(download_file_local_path);
	int dst = open(download_file_local_path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	char buf[1025] = {0, };
	int len = 0, wlen = 0;

	while (0 < (len = read(src, buf, 1024))){
		storage_plugin_sample_debug("len(%d) str(%s)", len, buf);

		wlen = write(dst, buf, len);
		storage_plugin_sample_debug("written len(%d)", wlen);

		memset(buf, 0, 1025);
	}

	close(dst);
	close(src);
	free(path);

	return 0;
}
/**
* @brief Removes a file at cloud
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to delete
* @param[in]	file_name			specifies file name to be deleted from cloud
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_delete_file(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Remove file");

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), file_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", file_name, NULL);
	storage_plugin_sample_debug("Remove path (%s)", path);
	int ret = remove(path);
	storage_plugin_sample_debug("Remove ret (%d)", ret);

	if (0 == ret)
	{
		storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, path);

		*file_info = _file_info;
	}
	else
	{
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(path);
	return ret;
}
/**
* @brief Gets metatdata of file at cloud
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies folder path of file you want to get meta data
* @param[in]	file_name			specifies file name
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	meta_data			specifies meta data (A pair of Key, Value)
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_get_meta(storage_adaptor_plugin_context_h context,
						const char *parent_folder_storage_path,
						const char *file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						void **meta_data,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Get meta data");



	return 0;
}

/**
* @brief Move a file into destination file
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies parent folder path of file you want to move
* @param[in]	file_name			specifies file name to be moved
* @param[in]	dest_parent_folder_storage_path	specifies new folder path
* @param[in]	new_file_name			specifies new file name (Not supported)
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_move_file(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *file_name,
						const char *dest_parent_path,
						const char *new_file_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Move file");

	char *old_path = NULL, *new_path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		old_path = g_strconcat(VIRTUAL_ROOT_PATH(context), file_name, NULL);
	else
		old_path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", file_name, NULL);
	storage_plugin_sample_debug("Old path (%s)", old_path);

	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), dest_parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		new_path = g_strconcat(VIRTUAL_ROOT_PATH(context), new_file_name, NULL);
	else
		new_path = g_strconcat(VIRTUAL_ROOT_PATH(context), dest_parent_path?dest_parent_path:"", "/", new_file_name, NULL);
	storage_plugin_sample_debug("New path (%s)", new_path);

	int ret = rename(old_path, new_path);

	if (0 == ret)
	{
		storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, new_path);

		*file_info = _file_info;
	}
	else
	{
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(old_path);
	free(new_path);

	return ret;
}

/**
* @brief Move a folder into destination folder
*
* @param[in]	context				specifies Storage Plugin context
* @param[in]	parent_folder_storage_path	specifies parent folder path of folder you want to move
* @param[in]	folder_name			specifies folder name to be moved
* @param[in]	dest_parent_folder_storage_path	specifies new parent folder path
* @param[in]	new_folder_name			specifies new folder name (Not supported)
* @param[in]	request				specifies optional parameter
* @param[out]	file_info			specifies Storage Adaptor File Info handle
* @param[out]	error				specifies error code
* @param[out]	response			specifies optional parameter
* @return	0 on success, otherwise a positive error value
* @retval	error code defined in storage_error_code_t - STORAGE_ADAPTOR_ERROR_NONE if Successful
*/
storage_error_code_t sample_move_directory(storage_adaptor_plugin_context_h context,
						const char *parent_path,
						const char *folder_name,
						const char *dest_parent_path,
						const char *new_folder_name,
						void *request,
						storage_adaptor_file_info_h *file_info,
						storage_adaptor_error_code_h *error,
						void *response)
{
	storage_plugin_sample_debug("Move directory");

	char *old_path = NULL, *new_path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		old_path = g_strconcat(VIRTUAL_ROOT_PATH(context), folder_name, NULL);
	else
		old_path = g_strconcat(VIRTUAL_ROOT_PATH(context), parent_path?parent_path:"", "/", folder_name, NULL);
	storage_plugin_sample_debug("Old path (%s)", old_path);

	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), dest_parent_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		new_path = g_strconcat(VIRTUAL_ROOT_PATH(context), new_folder_name, NULL);
	else
		new_path = g_strconcat(VIRTUAL_ROOT_PATH(context), dest_parent_path?dest_parent_path:"", "/", new_folder_name, NULL);
	storage_plugin_sample_debug("New path (%s)", new_path);

	int ret = rename(old_path, new_path);

	if (0 == ret)
	{
		storage_adaptor_file_info_h _file_info = create_chunk_file_info(context, STORAGE_ADAPTOR_CONTENT_TYPE_FOLDER, new_path);

		*file_info = _file_info;
	}
	else
	{
		ret = STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	free(old_path);
	free(new_path);

	return ret;
}

storage_error_code_t sample_get_root(storage_adaptor_plugin_context_h context,
                                                        void *request,
                                                        char **root_folder_path,
                                                        storage_adaptor_error_code_h *error,
                                                        void *response)
{
	//*root_folder_path = strdup(VIRTUAL_ROOT_PATH(context));
	*root_folder_path = strdup("/");

	return 0;
}

void *_async_uploader(void *data)
{
	async_job_t *plugin_task = (async_job_t *) data;
	int fd = plugin_task->fd;

	struct stat sbuf;
	storage_plugin_sample_debug("cloud_path (%s)", plugin_task->cloud_path);
	int ret = fstat(fd, &sbuf);
	storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
	unsigned long long total, progress;
	total = (unsigned long long) sbuf.st_size;
	storage_plugin_sample_debug("total size (%llu)", total);
	progress = 0ULL;

	remove(plugin_task->cloud_path);
	int dst = open(plugin_task->cloud_path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	char buf[1025] = {0, };
	int len = 0, wlen = 0;

	/* The 'sleep' is just helper code.
		it delay to download/upload speed, so it can simulate delay of real network environment. */
	int sleep_sec = 0;
	int usleep_sec = 0;
	if (total < 5000ULL) {
		sleep_sec = 1;
	} else if (total < 10240ULL) {
		usleep_sec = 500000;
	} else if (total < 100000ULL){
		usleep_sec = 100000;
	} else {
		usleep_sec = 1000;
	}

	sample_upload_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_IN_PROGRESS, NULL, NULL, NULL);
	while (0 < (len = read(fd, buf, 1024))){
		storage_plugin_sample_debug("len(%d) str(%s)", len, buf);

		wlen = write(dst, buf, len);
		storage_plugin_sample_debug("written len(%d)", len);

		progress += (unsigned long long) wlen;
		storage_plugin_sample_debug("progress (%llu / %llu)", progress, total);
		sample_progress_cb(fd, progress, total, NULL, NULL);

		memset(buf, 0, 1025);

		if (false == is_job_progress(fd)) {
			storage_plugin_sample_debug("job canceled");
			sample_upload_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED, NULL, NULL, NULL);
			destroy_job_s(plugin_task);
			return NULL;
		}
		if (sleep_sec)
			sleep(sleep_sec);
		if (usleep_sec)
			usleep(usleep_sec);
	}

	sample_upload_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED, NULL, NULL, NULL);
	cancel_job(fd);

	destroy_job_s(plugin_task);
	close(dst);

	return data;
}

storage_error_code_t sample_upload_async(storage_adaptor_plugin_context_h context,
						int src_file_descriptor,
						const char *dir_path,
						const char *file_name,
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{

	start_job(src_file_descriptor);
	pthread_t th;
	async_job_t *plugin_task = create_job_s(src_file_descriptor);

	char *path = NULL;
	path = g_strconcat(VIRTUAL_ROOT_PATH(context), dir_path?dir_path:"", "/", file_name, NULL);
	plugin_task->cloud_path = path;
	storage_plugin_sample_debug("cloud_path (%s)", path);

	pthread_create(&th, NULL, _async_uploader, (void *)plugin_task);

	return 0;
}


void *_async_downloader(void *data)
{
	async_job_t *plugin_task = (async_job_t *) data;
	int fd = plugin_task->fd;

	struct stat sbuf;
	storage_plugin_sample_debug("cloud_path (%s)", plugin_task->cloud_path);
	int ret = stat(plugin_task->cloud_path, &sbuf);
	storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
	unsigned long long total, progress;
	total = (unsigned long long) sbuf.st_size;
	storage_plugin_sample_debug("total size (%llu)", total);
	progress = 0ULL;

	int src = open(plugin_task->cloud_path, O_RDONLY);

	char buf[1025] = {0, };
	int len = 0, wlen = 0;

	/* The 'sleep' is just helper code.
		it delay to download/upload speed, so it can simulate delay of real network environment. */
	int sleep_sec = 0;
	int usleep_sec = 0;
	if (total < 5000ULL) {
		sleep_sec = 1;
	} else if (total < 10240ULL) {
		usleep_sec = 500000;
	} else if (total < 100000ULL){
		usleep_sec = 100000;
	} else {
		usleep_sec = 1000;
	}

	sample_download_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_IN_PROGRESS, NULL, NULL);
	while (0 < (len = read(src, buf, 1024))){
		storage_plugin_sample_debug("len(%d) str(%s)", len, buf);

		wlen = write(fd, buf, len);
		storage_plugin_sample_debug("written len(%d)", wlen);

		progress += (unsigned long long) wlen;
		storage_plugin_sample_debug("progress (%llu / %llu)", progress, total);
		sample_progress_cb(fd, progress, total, NULL, NULL);

		memset(buf, 0, 1025);

		if (false == is_job_progress(fd)) {
			storage_plugin_sample_debug("job canceled");
			sample_download_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_CANCELED, NULL, NULL);
			destroy_job_s(plugin_task);
			return NULL;
		}
		if (sleep_sec)
			sleep(sleep_sec);
		if (usleep_sec)
			usleep(usleep_sec);
	}

	sample_download_cb(fd, STORAGE_ADAPTOR_TRANSFER_STATE_FINISHED, NULL, NULL);
	cancel_job(fd);

	destroy_job_s(plugin_task);
	close(src);

	return data;
}

storage_error_code_t sample_download_async(storage_adaptor_plugin_context_h context,
						const char *dir_path,
						const char *file_name,
						int dst_file_descriptor,
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{
	start_job(dst_file_descriptor);
	pthread_t th;
	async_job_t *plugin_task = create_job_s(dst_file_descriptor);

	char *path = NULL;
	path = g_strconcat(VIRTUAL_ROOT_PATH(context), dir_path?dir_path:"", "/", file_name, NULL);
	plugin_task->cloud_path = path;

	pthread_create(&th, NULL, _async_downloader, (void *)plugin_task);

	return 0;
}

storage_error_code_t sample_download_thumbnail_async(storage_adaptor_plugin_context_h context,
						const char *dir_path,
						const char *file_name,
						int dst_file_descriptor,
						int thumbnail_size,
						bool need_progress,
						storage_adaptor_error_code_h *error,
						void *user_data)
{
	start_job(dst_file_descriptor);
	pthread_t th;
	async_job_t *plugin_task = create_job_s(dst_file_descriptor);

	char *path = NULL;
	path = g_strconcat(VIRTUAL_ROOT_PATH(context), dir_path?dir_path:"", "/", file_name, NULL);
	plugin_task->cloud_path = path;

	pthread_create(&th, NULL, _async_downloader, (void *)plugin_task);

	return 0;
}

storage_error_code_t sample_cancel_upload_file(storage_adaptor_plugin_context_h context,
						int file_descriptor,
						storage_adaptor_error_code_h *error)
{
	printf("<plugin> Cancel upload called\n");
	cancel_job(file_descriptor);
	sleep(1);
	return 0;
}

storage_error_code_t sample_cancel_download_file(storage_adaptor_plugin_context_h context,
						int file_descriptor,
						storage_adaptor_error_code_h *error)
{
	printf("<plugin> Cancel download called\n");
	cancel_job(file_descriptor);
	sleep(1);
	return 0;
}

storage_error_code_t sample_cancel_download_thumbnail(storage_adaptor_plugin_context_h context,
						int file_descriptor,
						storage_adaptor_error_code_h *error)
{
	printf("<plugin> Cancel download thumbnail called\n");
	cancel_job(file_descriptor);
	sleep(1);
	return 0;
}

storage_error_code_t sample_download_thumbnail(storage_adaptor_plugin_context_h context,
						const char *folder_path,
						const char *file_name,
						const char *local_path,
						int thumbnail_size,
						void *req,
						storage_adaptor_error_code_h *error,
						void *res)
{
	printf("folder path : %s\n", folder_path);
	printf("file name : %s\n", file_name);
	printf("local path : %s\n", local_path);
	printf("thumbnail size : %d\n", thumbnail_size);

	char *path = NULL;
	if (0 == strncmp(VIRTUAL_ROOT_PATH(context), folder_path, strlen(VIRTUAL_ROOT_PATH(context)) - 1))
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), file_name, NULL);
	else
		path = g_strconcat(VIRTUAL_ROOT_PATH(context), folder_path?folder_path:"", "/", file_name, NULL);

	int src = open(path, O_RDONLY);
	if (0 > src)
	{
		free(path);
		return STORAGE_ADAPTOR_ERROR_PLUGIN_INTERNAL;
	}

	struct stat sbuf;
	storage_plugin_sample_debug("cloud_path (%s)", path);
	int ret = fstat(src, &sbuf);
	storage_plugin_sample_debug("stat ret (%d) (%d)", ret, errno);
	unsigned long long total = 0ULL;
	total = (unsigned long long) sbuf.st_size;
	storage_plugin_sample_debug("total size (%llu)", total);

	remove(local_path);
	int dst = open(local_path, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

	char buf[1025] = {0, };
	int len = 0, wlen = 0;

	while (0 < (len = read(src, buf, 1024))){
		storage_plugin_sample_debug("len(%d) str(%s)", len, buf);

		wlen = write(dst, buf, len);
		storage_plugin_sample_debug("written len(%d)", wlen);

		memset(buf, 0, 1025);
	}

	close(dst);
	close(src);
	free(path);

	return 0;
}

/**
* @brief Create Storage Plugin handle
*
* @return Storage Plugin handle on success, otherwise NULL value
*/
EXPORT_API storage_adaptor_plugin_handle_h create_plugin_handle(void)
{
	storage_plugin_sample_debug("Create sample plugin handle");

	storage_adaptor_plugin_handle_h handle =
			(storage_adaptor_plugin_handle_h) calloc(1, sizeof(storage_adaptor_plugin_handle_t));

	if (NULL == handle)
	{
		storage_plugin_sample_error("Memory allocation failed");
		return NULL;
	}

	// Mandatory functions
	handle->create_context				= sample_create_context;
	handle->destroy_context				= sample_destroy_context;
	handle->destroy_handle				= sample_destroy_plugin_handle;
	handle->set_listener				= sample_register_listener;
	handle->unset_listener				= sample_unregister_listener;

	// Optional functions
	// Uified Storage System internal
	handle->get_root_folder_path		= sample_get_root;
	handle->set_server_info				= sample_set_server_info;
	handle->make_directory				= sample_make_directory;
	handle->remove_directory			= sample_remove_directory;
	handle->upload_file_sync			= sample_upload_file_sync;
	handle->download_file_sync			= sample_download_file_sync;
	handle->delete_file					= sample_delete_file;
	handle->move_file					= sample_move_file;
	handle->move_directory				= sample_move_directory;

	// Public
	handle->list						= sample_get_list;
	handle->start_upload_task			= sample_upload_async;
	handle->start_download_task			= sample_download_async;
	handle->start_download_thumb_task	= sample_download_thumbnail_async;
	handle->cancel_upload_task			= sample_cancel_upload_file;
	handle->cancel_download_task		= sample_cancel_download_file;
	handle->cancel_download_thumb_task	= sample_cancel_download_thumbnail;

	handle->download_thumbnail			= sample_download_thumbnail;

	// Plugin name
	handle->plugin_uri = strdup(SAMPLE_PACKAGE_NAME);

	return handle;
}
