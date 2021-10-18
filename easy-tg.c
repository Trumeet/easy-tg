/* 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at

 *   http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */

#include "easy-tg.h"

#include <stdio.h>
#include <string.h>

#include <td/telegram/td_json_client.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

void *td = NULL;
const char *tg_update_type = NULL;
json_object *tg_update = NULL;
int tg_errno = 0;
const void *tg_reg1 = NULL;
const void *tg_reg2 = NULL;
const void *tg_reg3 = NULL;
const void *tg_reg4 = NULL;
const void *tg_reg5 = NULL;
const void *tg_reg6 = NULL;

json_object *tg_mkreq(const char *extra, const char *type, const char *args_key, json_object *args)
{
	json_object *root = json_object_new_object();
	json_object_object_add(root, "@extra", json_object_new_string(extra));
        json_object_object_add(root, "@type", json_object_new_string(type));
        if(args_key != NULL) json_object_object_add(root, args_key, args);
        return root;
}

int tg_send_raw(bool execute, const char *req)
{
#ifdef TG_DEBUG
	printf("> %s\n", req);
#endif
	if(execute)
		td_json_client_execute(td, req);
	else
	        td_json_client_send(td, req);
	return 0;
}

int tg_send(bool execute, json_object *root, const bool dispose)
{
	int r = 0;
        const char *req = json_object_to_json_string(root);
	r = tg_send_raw(execute, req);
	if(dispose)
	{
        	json_object_put(root);
	}
	return r;
}

int tg_login_phone(
		const char *phone_number
		)
{
	int r = 0;
        json_object *raw_req = json_object_new_string(phone_number);
        tg_send(false, tg_mkreq(TG_REQ_LOGIN_PHONE, "setAuthenticationPhoneNumber", "phone_number", raw_req), true);
	goto cleanup;
cleanup:
        return r;
}

int tg_login_code(
		const char *code
		)
{
	int r = 0;
        json_object *raw_req = json_object_new_string(code);
        tg_send(false, tg_mkreq(TG_REQ_LOGIN_CODE, "checkAuthenticationCode", "code", raw_req), true);
	goto cleanup;
cleanup:
        return r;
}

int tg_login_pass(
		const char *pass
		)
{
	int r = 0;
        json_object *raw_req = json_object_new_string(pass);
        tg_send(false, tg_mkreq(TG_REQ_LOGIN_PASS, "checkAuthenticationPassword", "password", raw_req), true);
	goto cleanup;
cleanup:
        return r;
}

int tg_login_enc_key(
		const char *encryption_key
		)
{
	int r = 0;
        json_object *raw_req = json_object_new_string(encryption_key);
        tg_send(false, tg_mkreq("-1", "checkDatabaseEncryptionKey", "encryption_key", raw_req), true);
	goto cleanup;
cleanup:
        return r;
}

int tg_set_params(
		const bool use_test_dc,
		const char *database_directory,
		const char *files_directory,
		const bool use_file_database,
		const bool use_chat_info_database,
		const bool use_message_database,
		const bool use_secret_chats,
		const int32_t api_id,
		const char *api_hash,
		const char *system_language_code,
		const char *device_model,
		const char *system_version,
		const char *application_version,
		const bool enable_storage_optimizer,
		const bool ignore_file_names
		)
{
	int r = 0;
        json_object *raw_req = json_object_new_object();
        json_object_object_add(raw_req, "use_test_dc", json_object_new_boolean(use_test_dc));
        json_object_object_add(raw_req, "database_directory", json_object_new_string(database_directory));
	if(files_directory != NULL)
	        json_object_object_add(raw_req, "files_directory", json_object_new_string(files_directory));
        json_object_object_add(raw_req, "use_file_database", json_object_new_boolean(use_file_database));
        json_object_object_add(raw_req, "use_message_database", json_object_new_boolean(use_message_database));
        json_object_object_add(raw_req, "use_secret_chats", json_object_new_boolean(use_secret_chats));
        json_object_object_add(raw_req, "api_id", json_object_new_int(api_id));
        json_object_object_add(raw_req, "api_hash", json_object_new_string(api_hash));
        json_object_object_add(raw_req, "system_language_code", json_object_new_string(system_language_code));
        json_object_object_add(raw_req, "device_model", json_object_new_string(device_model));
        json_object_object_add(raw_req, "system_version", json_object_new_string(system_version));
        json_object_object_add(raw_req, "application_version", json_object_new_string(application_version));
        json_object_object_add(raw_req, "use_secret_chats", json_object_new_boolean(use_secret_chats));
        json_object_object_add(raw_req, "enable_storage_optimizer", json_object_new_boolean(enable_storage_optimizer));
        json_object_object_add(raw_req, "ignore_file_names", json_object_new_boolean(ignore_file_names));
        tg_send(false, tg_mkreq(TG_REQ_SET_PARAMS, "setTdlibParameters", "parameters", raw_req), true);
	goto cleanup;
cleanup:
        return r;
}

static int handle_update_error()
{
	int r = TG_IGNORE;
	int code = -1;
	const char *msg = NULL;
	const char *extra = NULL;
	json_object *obj = NULL;
	if(!json_object_object_get_ex(tg_update, "code", &obj))
		goto cleanup;
	code = json_object_get_int(obj);
	if(!json_object_object_get_ex(tg_update, "message", &obj))
		goto cleanup;
	msg = json_object_get_string(obj);
	if(json_object_object_get_ex(tg_update, "@extra", &obj))
		extra = json_object_get_string(obj);
	tg_errno = code;
	tg_reg1 = extra;
	tg_reg2 = msg;
	r = TG_ERROR;
	goto cleanup;
cleanup:
	return r;
}

static int handle_update_authorization_state()
{
        int r = 0;
        json_object *obj = NULL;
        if(!json_object_object_get_ex(tg_update, "authorization_state", &obj))
                goto cleanup;
	json_object *type_obj = NULL;
        if(!json_object_object_get_ex(obj, "@type", &type_obj))
                goto cleanup;
        const char *type = json_object_get_string(type_obj);
#ifdef TG_DEBUG
        printf("AUTH: %s\n", type);
#endif
        if(!strcmp(type, "authorizationStateClosed"))
		r = TG_CLOSED;
	else if(!strcmp(type, "authorizationStateClosing"))
		r = TG_CLOSING;
	else if(!strcmp(type, "authorizationStateLoggingOut"))
		r = TG_LOGGING_OUT;
	else if(!strcmp(type, "authorizationStateWaitTdlibParameters"))
                r = TG_PARAMS;
        else if(!strcmp(type, "authorizationStateWaitEncryptionKey"))
                r = TG_LOGIN_ENC;
        else if(!strcmp(type, "authorizationStateWaitPhoneNumber"))
                r = TG_LOGIN_PHONE;
        else if(!strcmp(type, "authorizationStateWaitCode"))
                r = TG_LOGIN_CODE;
        else if(!strcmp(type, "authorizationStateWaitPassword"))
                r = TG_LOGIN_PW;
	else if(!strcmp(type, "authorizationStateWaitRegistration"))
		r = TG_LOGIN_REG;
	else if(!strcmp(type, "authorizationStateWaitOtherDeviceConfirmation"))
	{
		r = TG_LOGIN_QR;
		if(json_object_object_get_ex(obj, "link", &obj))
			tg_reg1 = json_object_get_string(obj);
	}
	else if(!strcmp(type, "authorizationStateReady"))
		r = TG_LOGGED_IN;
	else
		r = TG_RUN;
        goto cleanup;
cleanup:
        return r;
}

int tg_init()
{
	int r = 0;
	td = td_json_client_create();
	if(td == NULL)
	{
		r = TG_SYSERR;
		goto cleanup;
	}
	tg_send_raw(true, "{ \"@extra\": \"log\", \"@type\": \"setLogVerbosityLevel\", \"@args\": { \"new_verbosity_level\": 0 } }");
	goto cleanup;
cleanup:
	return r;
}

int tg_destroy()
{
	if(td == NULL)
	{
		return 0;
	}
	td_json_client_destroy(td);
	td = NULL;
	return 0;
}

static int handle_update_connection_state()
{
	int r = TG_IGNORE;
	json_object *obj = NULL;
	if(!json_object_object_get_ex(tg_update, "state", &obj) ||
			!json_object_object_get_ex(obj, "@type", &obj))
		goto cleanup;
	r = TG_CONNECTION;
	tg_reg1 = json_object_get_string(obj);
	if(!strcmp(tg_reg1, "connectionStateConnecting"))
		tg_errno = TG_CONNECTION_CONNECTING;
	else if(!strcmp(tg_reg1, "connectionStateConnectingToProxy"))
		tg_errno = TG_CONNECTION_CONNECTING_PROXY;
	else if(!strcmp(tg_reg1, "connectionStateReady"))
		tg_errno = TG_CONNECTION_READY;
	else if(!strcmp(tg_reg1, "connectionStateUpdating"))
		tg_errno = TG_CONNECTION_UPDATING;
	else if(!strcmp(tg_reg1, "connectionStateWaitingForNetwork"))
		tg_errno = TG_CONNECTION_WAITING_NETWORK;
	else
		r = TG_RUN;

	goto cleanup;
cleanup:
	return r;
}

int tg_loop()
{
	if(tg_update != NULL)
	{
        	json_object_put(tg_update);
		tg_update = NULL;
		tg_update_type = NULL;
		tg_errno = 0;
		tg_reg1 = NULL;
		tg_reg2 = NULL;
		tg_reg3 = NULL;
		tg_reg4 = NULL;
		tg_reg5 = NULL;
		tg_reg6 = NULL;
	}
	const char *event = td_json_client_receive(td, 3);
	int r = TG_IGNORE;
	if(event == NULL) goto cleanup;
#ifdef TG_DEBUG
        printf("< %s\n", event);
#endif
        tg_update = json_tokener_parse(event);
        json_object *obj = NULL;
        if(!json_object_object_get_ex(tg_update, "@type", &obj))
		goto cleanup;
        tg_update_type = json_object_get_string(obj);
        if(!strcmp(tg_update_type, "updateAuthorizationState"))
                r = handle_update_authorization_state();
	else if(!strcmp(tg_update_type, "error"))
		r = handle_update_error();
	else if(!strcmp(tg_update_type, "updateConnectionState"))
		r = handle_update_connection_state();
	else
		r = TG_RUN;
        goto cleanup;
cleanup:
        return r;
}

int tg_close()
{
	if(td == NULL) return 0;
	/* Using json object seems to be leaking */
	tg_send_raw(false, "{ \"@type\": \"close\" }");
	return 0;
}
