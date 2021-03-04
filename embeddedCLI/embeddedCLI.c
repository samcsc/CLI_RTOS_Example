/*
 * embeddedCLI.c
 *
 *  Created on: 1 Feb 2021
 *      Author: Sam
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>

#include "embeddedCLI.h"

#define TRUE 1
#define FALSE 0

// Linked list for the registered commands
typedef struct command_node_t {
	const cli_command_t *command;
	struct command_node_t *next;
} command_node_t;

// Create the command for help and this will be our head for the
// command linked list.
static CLI_Status_t help_command_cb(const char *cli_buffer, char *output_buffer,
		uint8_t output_buffer_lenght);
char help_message[] =
		"\n     List of all registered commands\n-----------------------------------------\n";
static const cli_command_t help_command = { "help", help_message, 0,
		help_command_cb };
static command_node_t command_list = { .command = &help_command, .next = NULL };

static uint8_t get_number_of_parameters(const char *cli_buffer);
static const cli_command_t* find_matching_command(const char *cli_buffer);

// Functions
CLI_Status_t cli_register_command(const cli_command_t *const command) {

	CLI_Status_t status = CLI_ERROR;
	static command_node_t *command_list_tail = &command_list;
	command_node_t *new_node;
	new_node = (command_node_t*) pvPortMalloc(sizeof(command_node_t));

	if (new_node != NULL) {
		new_node->command = command;
		new_node->next = NULL;
		command_list_tail->next = new_node;
		command_list_tail = new_node;
		status = CLI_OK;
	}

	return status;
}

static uint8_t get_number_of_parameters(const char *cli_buffer) {
	uint8_t parameter_count = 0;
	uint8_t space_found = FALSE;

	while (*cli_buffer != '\0') {
		if (*cli_buffer == ' ') {
			space_found = TRUE;
		} else {
			if (space_found == TRUE) {
				space_found = FALSE;
				parameter_count++;
			}
		}

		cli_buffer++;
	}

	return parameter_count;
}

static const cli_command_t* find_matching_command(const char *cli_buffer) {

	command_node_t *p = &command_list;
	uint8_t command_length = 0;

	while (p != NULL) {
		command_length = strlen(p->command->keyword);
		if (strncmp(p->command->keyword, cli_buffer, command_length) == 0) {
			if ((cli_buffer[command_length] == ' '
					|| cli_buffer[command_length] == '\0') == 1) {
				break;
			}
		}
		p = p->next;
	}

	if (p != NULL)
		return p->command;
	else
		return NULL;
}

const char* cli_get_parameter(const char *input_string, uint8_t *param_len) {
	const char *parameter = NULL;
	uint8_t space_found = FALSE;

	while (*input_string != '\0') {
		if (*input_string == ' ') {
			space_found = TRUE;
		} else if (space_found == TRUE && *input_string != ' ') {
			parameter = input_string;
			while (*input_string != ' ' && *input_string != '\0') {
				input_string++;
				(*param_len)++;
			}
			break;
		}
		input_string++;
	}

	return parameter;
}

CLI_Status_t cli_process(const char *cli_buffer, char *output_buffer,
		const uint8_t output_buffer_lenght) {

	CLI_Status_t status = CLI_OK;

	static const cli_command_t *p = NULL;
	static uint8_t iscommand = 0;

	// Look for a matching command

	if (p == NULL) {
		p = find_matching_command(cli_buffer);
		iscommand = 0;
	}

	if (!iscommand) {
		if (p == NULL) {
			strncpy(output_buffer, "Command is not recognized. Try 'help' "
					"for the list of registered commands",
					output_buffer_lenght);
		} else {
			if (p->number_of_param == 0) {
				iscommand = 1;
			} else if (get_number_of_parameters(cli_buffer)
					== p->number_of_param) {
				iscommand = 1;
			} else {
				strncpy(output_buffer,
						"Number of parameters does not match the expected "
								"number of parameters", output_buffer_lenght);
			}
		}
	}
	if (iscommand) {
		status = p->callback(cli_buffer, output_buffer, output_buffer_lenght);
	}

	if (status == CLI_OK)
		p = NULL;

	return status;
}

static CLI_Status_t help_command_cb(const char *cli_buffer, char *output_buffer,
		uint8_t output_buffer_lenght) {

	CLI_Status_t status = CLI_RUNNING;

	static command_node_t *p = NULL;

	if (p == NULL) {
		p = &command_list;
		strncpy(output_buffer, p->command->help_message, output_buffer_lenght);
	} else {
		strncpy(output_buffer, p->command->keyword, output_buffer_lenght);
		strncat(output_buffer, ":\t", output_buffer_lenght);
		strncat(output_buffer, p->command->help_message, output_buffer_lenght);
	}

	p = p->next;
	if (p == NULL) {
		status = CLI_OK;
	} else {
		status = CLI_RUNNING;
	}

	return status;
}

