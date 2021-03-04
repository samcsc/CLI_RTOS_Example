#include <stdint.h>
#include <string.h>

#include "main.h"
#include "../../embeddedCLI/embeddedCLI.h"

// Callback function prototypes
static CLI_Status_t echo_command_cb(const char *cli_buffer, char *output_buffer,
		uint8_t output_buffer_lenght);
static CLI_Status_t ledonoff_command_cb(const char *cli_buffer,
		char *output_buffer, uint8_t output_buffer_lenght);

// Command struct definitions
const cli_command_t echo_command = { "echo",
		"Simply sends its arguments back to the user.\n", 0, echo_command_cb };
const cli_command_t ledonoff_command = { "led", "Turn the led on or off\n", 1,
		ledonoff_command_cb };

// Command registration function
void register_commands(void) {
	cli_register_command(&echo_command);
	cli_register_command(&ledonoff_command);
}

// Callback implementation
static CLI_Status_t echo_command_cb(const char *cli_buffer, char *output_buffer,
		uint8_t output_buffer_lenght) {

	uint8_t parameter_length = 0;
	const char *string = cli_get_parameter(cli_buffer, &parameter_length);
	strncpy(output_buffer, string, output_buffer_lenght);

	return CLI_OK;
}

static CLI_Status_t ledonoff_command_cb(const char *cli_buffer,
		char *output_buffer, uint8_t output_buffer_lenght) {

	uint8_t arglen = 0;
	const char *arg = cli_get_parameter(cli_buffer, &arglen);

	if (arglen == 2 && strcmp("on", arg) == 0) {
		strncpy(output_buffer, "LED -> ON", output_buffer_lenght);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

	} else if (arglen == 3 && strcmp("off", arg) == 0) {
		strncpy(output_buffer, "LED -> OFF", output_buffer_lenght);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	} else {
		strncpy(output_buffer, "Argument can be either on or off", output_buffer_lenght);
	}

	return CLI_OK;
}

//static CLI_Status_t echo_command_cb(const char *cli_buffer, char *output_buffer,
//		uint8_t output_buffer_lenght) {
//
//	CLI_Status_t status = CLI_RUNNING;
//
//	static const char *parameter = NULL;
//	uint8_t parameter_length = 0;
//
//	if (parameter == NULL)
//		parameter = cli_get_parameter(cli_buffer, &parameter_length);
//	else
//		parameter = cli_get_parameter(parameter, &parameter_length);
//
//	if (parameter != NULL) {
//		strncpy(output_buffer, parameter, parameter_length);
//	} else {
//		parameter = NULL;
//		status = CLI_OK;
//	}
//
//	return status;
//}
