#define _POSIX_C_SOURCE 200809L
#include "easy-tg.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

static void read_input(const char *prompt, char *buf, unsigned int len)
{
	printf("%s", prompt);
	fflush(stdout);
	ssize_t size = read(0, buf, len + 1);
	if(size >= 1) buf[size - 1] = '\0';
	printf("Read: %s\n", buf);
}

static void run()
{
	/* In each loop, the action our application needs to take is returned via return code.
	 * Other data, such as current update type and the whole JSON response, are stored in registers.
	 * There are nine registers:
	 * int tg_errno: In case of error, it will store the error code.
	 * const char *tg_update_type: Always stores the parsed type of update.
	 * json_object *tg_update: Always stores the whole tokenized JSON update.
	 * void *tg_reg1: Optional data field.
	 * void *tg_reg2: Optional data field.
	 * void *tg_reg3: Optional data field.
	 * void *tg_reg4: Optional data field.
	 * void *tg_reg5: Optional data field.
	 * void *tg_reg6: Optional data field.
	 * Each register has the lifetime from the end of tg_loop() to the next call of tg_loop().
	 * After calling tg_loop() for the next time, all memory will be released and the registers will be reset to default values, which are 0 for int and NULL for pointers.
	 */
	printf("%s\n", tg_update_type);
}

static void *main_sighandler(void *arg)
{
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        int r = 0;
        sigset_t *set = arg;
        int sig;
        while(true)
        {
                r = sigwait(set, &sig);
                if(r)
                {
                        fprintf(stderr, "Cannot call sigwait(): %d.\n", r);
                        goto cleanup;
                }
                switch(sig)
                {
                        case SIGINT:
                        case SIGTERM:
                                printf("Received SIGINT or SIGTERM, asking TDLib to close.\n");
                                tg_close();
                                goto cleanup;
                }
        }
        goto cleanup;
cleanup:
        pthread_exit(NULL);
        return NULL;
}

int main(int argc, char **argv)
{
	bool sighandler_setup = false;
	pthread_t thread_sighandler;
	int r = 0;
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGTERM);
        sigaddset(&set, SIGINT);
        r = pthread_sigmask(SIG_BLOCK, &set, NULL);
        if(r)
        {
                fprintf(stderr, "Cannot call pthread_sigmask(): %d\n", r);
                goto cleanup;
        }
        r = pthread_create(&thread_sighandler, NULL, &main_sighandler, &set);
        if(r)
        {
                fprintf(stderr, "Cannot call pthread_create(): %d\n", r);
                goto cleanup;
        }
        sighandler_setup = true;
	r = tg_init();
	if(r) goto cleanup; /* Fetal errors encountered. */
	while(1)
	{
		switch(tg_loop())
		{
			case TG_IGNORE: /* Invalid response from TDLib. Just ignore it. */
				break;
			case TG_RUN: /* Non-login update received. Run our custom function. */
				run();
				break;
			case TG_CLOSED: /* TDLib is shutting down. We have nothing to do except releasing resources. */
				goto destroy;
			case TG_CLOSING: /* TDLib is closing. Currently we don't need to do nothing. */
			case TG_LOGGING_OUT: /* The account is logging out. No need for now. */
				break;
			case TG_LOGGED_IN: /* Login ready. */
				printf("You are logged in!\n");
				break;
			case TG_LOGIN_PHONE: /* We need to provide the phone number to login. */
				char phone[20];
				read_input("Phone number: ", phone, 19);
				tg_login_phone(phone);
				break;
			case TG_LOGIN_CODE: /* We need to provide the verification code. */
				char code[7];
				read_input("Verification code: ", code, 6);
				tg_login_code(code);
				break;
			case TG_LOGIN_ENC: /* During initialization, TDLib is asking us for database encryption key. */
				tg_login_enc_key(""); /* Just leave it empty for now. */
				break;
			case TG_LOGIN_QR: /* QR login. Currently not supported in the demo. */
				/* The QR link is stored in register 1. */
				/* generate_qr((char*)tg_reg1); */
				printf("QR login not supported.\n");
				break;
			case TG_LOGIN_PW: /* We need to type the password. */
				char pass[257];
				read_input("Password: ", pass, 256);
				tg_login_pass(pass);
				break;
			case TG_LOGIN_REG: /* The phone number is verified but unknown. We need fill in the registration form. In this case, we will ask the user to register in the official client instead. */
				printf("Your phone number is not known.\n");
				break;
			case TG_PARAMS: /* TDLib is not been setup and is asking you for parameters. */
				tg_set_params(
					true, /* Use test datacenter */
					"/tmp/td", /* Database directory */
					NULL, /* Files directory */
					false, /* Use files database */
					false, /* Use message database */
					false, /* Use chat info database */
					false, /* Support secret chats */
					API_ID, /* API ID */
					API_HASH, /* API Hash */
					"en", /* System language code */
					"Desktop", /* Device model */
					"Windoge 114.514", /* System version */
					"Moe Bot", /* Application version */
					false, /* Enable storage optimizer */
					true /* Ignore file names */
				);
				break;
			case TG_ERROR: /* TDLib returned an error. */
				/* Error code is stored in tg_errno;
				 * Request extra (ID) is stored in register 1;
				 * Error message is stored in register 2.
				 */
				/* Built-in request extras */
				if(!strcmp(TG_REQ_SET_PARAMS, tg_reg1))
				{
					printf("Invalid TDLib parameters! %d (%s)\n", tg_errno, tg_reg2);
				} else if(!strcmp(TG_REQ_LOGIN_PHONE, tg_reg1))
				{
					printf("Invalid phone number! %d (%s)\n", tg_errno, tg_reg2);
				} else if(!strcmp(TG_REQ_LOGIN_PASS, tg_reg1))
				{
					printf("Invalid password! %d (%s)\n", tg_errno, tg_reg2);
				} else
				{
					printf("Invalid request: %s! %d (%s)\n", tg_reg1, tg_errno, tg_reg2);
				}
				break;
			case TG_SYSERR: /* Easy-TG encountered an error. */
				/* Error code is stored in tg_errno. */
				printf("Fetal: %d\n", tg_errno);
				break;
			case TG_CONNECTION: /* Telegram connection status is changed. */
				printf("[CONNECTION]: %s (%d)\n", tg_reg1, tg_errno);
				break;
			default:
				printf("???\n");
				break;
		}
	}
	goto destroy;
destroy:
	tg_destroy();
	goto cleanup;
cleanup:
	if(sighandler_setup)
        {
                pthread_cancel(thread_sighandler);
                pthread_join(thread_sighandler, NULL);
        }
	return r;
}
