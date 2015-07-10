/*
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <service_app.h>
#include <dlog.h>

#include "service_plugin_sample.h"
#include "service_provider.h"

service_provider_h service_provider = NULL;

service_adaptor_error_e _connect()
{
	SLOGI("connect");

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e _disconnect()
{
	SLOGI("disconnect");

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e _oauth1_get_access_token(char **access_token)
{
	SLOGI("oauth1_get_access_token");

	*access_token = strdup("test");

	return SERVICE_ADAPTOR_ERROR_NONE;
}

service_adaptor_error_e _cloud_remove_file(const char *cloud_path)
{
	SLOGI("cloud_remove_file: %s", cloud_path);

	return SERVICE_ADAPTOR_ERROR_NONE;
}

static bool service_app_create(void *data)
{
	return true;
}

static void service_app_control(app_control_h app_control, void *data)
{
	SLOGI("app_control");

	/* Handle the launch request. */
	service_provider_message(service_provider, app_control, data);
}

static void service_app_terminate(void *data)
{
	/* Release all resources. */
}

static void service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	return;
}

static void service_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char *argv[])
{
	char ad[50] = {0,};
	int ret = 0;

	/*Service Plugin Client*/
	service_provider_create(&service_provider);
	service_provider->connect = _connect;
	service_provider->disconnect = _disconnect;

	auth_provider_h auth_provider = NULL;
	auth_provider_create(&auth_provider);
	auth_provider->oauth1_get_access_token = _oauth1_get_access_token;

	service_provider_set_auth_provider(service_provider, auth_provider);

	storage_provider_h storage_provider = NULL;
	storage_provider_create(&storage_provider);
	storage_provider->cloud_remove_file = _cloud_remove_file;

	service_provider_set_storage_provider(service_provider, storage_provider);
	/*Service Plugin Client*/

	service_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, service_app_orient_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	ret = service_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE)
	{
		SLOGE(LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	service_provider_unset_auth_provider(service_provider);
	service_provider_unset_storage_provider(service_provider);
	service_provider_destroy(service_provider);

	return ret;
}
