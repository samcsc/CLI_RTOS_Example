/*
 * embeddedCLI.h
 *
 *  Created on: 1 Feb 2021
 *      Author: Sam
 */

#ifndef EMBEDDEDCLI_H_
#define EMBEDDEDCLI_H_

typedef enum {
	CLI_OK = 0x00U, CLI_RUNNING, CLI_ERROR
} CLI_Status_t;

// Callback function prototype
typedef CLI_Status_t (*command_callback)(const char *cli_buffer,
		char *output_buffer, uint8_t output_buffer_lenght);

typedef struct cli_command_t {
	const char *const keyword;
	const char *const help_message;
	const uint8_t number_of_param;
	const command_callback callback;
} cli_command_t;

CLI_Status_t cli_register_command(const cli_command_t *const command);
CLI_Status_t cli_process(const char *cli_buffer, char *output_buffer,
		uint8_t output_buffer_lenght);
const char* cli_get_parameter(const char *cli_buffer, uint8_t *param_len);

#endif /* EMBEDDEDCLI_H_ */
