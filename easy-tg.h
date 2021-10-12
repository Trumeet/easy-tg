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

#ifndef _EASY_TG_H
#define _EASY_TG_H

#define TG_IGNORE	-1	/* Invalid update received. Just ignore it. */
#define TG_RUN		0	/* Normal update received. */
#define TG_CLOSED	1	/* TDLib closed. */
#define TG_CLOSING	2	/* TDLib is closing. */
#define TG_LOGGING_OUT	3	/* Logging out. */
#define	TG_LOGGED_IN	4	/* Login ready. */
#define	TG_LOGIN_PHONE	5	/* Need to input phone number. */
#define TG_LOGIN_CODE	6	/* Need to input verification code. */
#define TG_LOGIN_ENC	7	/* Need to provide encryption key. */
#define TG_LOGIN_QR	8	/* Need to scan the QR code. r1 = link. */
#define TG_LOGIN_PW	9	/* Need to input the password. */
#define TG_LOGIN_REG	10	/* Need to register. */
#define TG_PARAMS	11	/* Need to provide TDLib parameters. */
#define TG_ERROR	12	/* TDLib returns an error. tg_errno = code, r1 = extra, r2 = msg. */
#define TG_SYSERR	13	/* EasyTD encounters an error. */
#define TG_CONNECTION	14	/* Telegram connection state update. tg_errno = code, r1 = raw type */

#define TG_REQ_SET_PARAMS	"set_params"
#define TG_REQ_LOGIN_PHONE	"login_phone"
#define TG_REQ_LOGIN_CODE	"login_code"
#define TG_REQ_LOGIN_PASS	"login_pass"

#define TG_CONNECTION_CONNECTING	1
#define TG_CONNECTION_CONNECTING_PROXY	2
#define TG_CONNECTION_READY		3
#define TG_CONNECTION_UPDATING		4
#define TG_CONNECTION_WAITING_NETWORK	5

#include <json-c/json_object.h>
#include <stdint.h>
#include <stdbool.h>

extern void *td;
extern const char *tg_update_type;
extern json_object *tg_update;
extern int tg_errno;
extern const void *tg_reg1;
extern const void *tg_reg2;
extern const void *tg_reg3;
extern const void *tg_reg4;
extern const void *tg_reg5;
extern const void *tg_reg6;

/* MT-Unsafe */
int tg_init();
/* MT-Unsafe */
int tg_destroy();
/* MT-Unsafe */
int tg_loop();
/* MT-Safe */
int tg_close();
/* MT-Safe */
int tg_logout();
/* MT-Safe */
int tg_set_params(
		const bool use_test_dc,
		const char *database_directory,
		const char *files_directory,
		const bool use_file_database,
		const bool use_message_database,
		const bool use_chat_info_database,
		const bool use_secret_chats,
		const int32_t api_id,
		const char *api_hash,
		const char *system_language_code,
		const char *device_model,
		const char *system_version,
		const char *application_version,
		const bool enable_storage_optimizer,
		const bool ignore_file_names
		);
/* MT-Safe */
int tg_login_phone(
		const char *phone_number
		);
/* MT-Safe */
int tg_login_code(
		const char *code
		);
/* MT-Safe */
int tg_login_pass(
		const char *password
		);
/* MT-Safe */
int tg_login_enc_key(
		const char *encryption_key
	      );
/* MT-Safe */
json_object *tg_mkreq(
		const char *extra,
		const char *type,
		const char *args_key,
		json_object *args
		);
/* MT-Safe */
int tg_send(
		bool execute,
		json_object *req,
		const bool dispose
	   );
/* MT-Safe */
int tg_send_raw(
		bool execute,
		const char *req
	       );

#endif /* _EASY_TG_H */
