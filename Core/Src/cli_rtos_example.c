#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "main.h"
#include "cmsis_os.h"
#include "../../embeddedCLI/embeddedCLI.h"
// #define
#define RX_BUFFER_SIZE 60
#define TX_BUFFER_SIZE 120
#define CLI_BUFFER_SIZE 60

// External variables
extern UART_HandleTypeDef hlpuart1;
extern DMA_HandleTypeDef hdma_lpuart1_rx;
extern DMA_HandleTypeDef hdma_lpuart1_tx;

extern void register_commands(void);

// CubeMX has to generate this task... So Let's just delete it.
void StartDefaultTask(void *argument) {
	osThreadExit();
}

// Startup code.
void StartupTask(void *argument) {
	const char *tx_buffer =
			"-------------------- Program Start --------------------\n";
	HAL_UART_Transmit_DMA(&hlpuart1, (uint8_t*) tx_buffer, strlen(tx_buffer));
	register_commands();
	// Delete this task after startup code complete.
	osThreadExit();
}

// This task is in charge of getting data from dma and putting them in the CLI buffer
// for processing. It will send out data if there is any.
void CommandInterpreterTask(void *argument) {

	CLI_Status_t status = CLI_ERROR;
	char rx_buffer[RX_BUFFER_SIZE] = { 0 };			// Buffer for RX DMA
	char tx_buffer[TX_BUFFER_SIZE] = { 0 };			// Buffer for TX DMA
	char cli_buffer[CLI_BUFFER_SIZE + 1] = { 0 }; // Account for the string terminating character.
	uint8_t rx_buffer_head = RX_BUFFER_SIZE;
	uint8_t rx_buffer_tail = RX_BUFFER_SIZE;
	char rx_char;
	uint8_t count = 0;
	uint8_t new_command_received = 0;

	// Start the UART RX DMA and it SHOULD be set to ring buffer mode.
	HAL_UART_Receive_DMA(&hlpuart1, (uint8_t*) rx_buffer, RX_BUFFER_SIZE);

	while (1) {

		//
		rx_buffer_head = __HAL_DMA_GET_COUNTER(&hdma_lpuart1_rx);
		while (rx_buffer_head != rx_buffer_tail && !new_command_received) {
			HAL_GPIO_TogglePin(PA8_GPIO_Port, PA8_Pin);	// TODO: Delete it later

			rx_char = rx_buffer[RX_BUFFER_SIZE - rx_buffer_tail];

			// Ring buffer wrapping
			rx_buffer_tail =
					(--rx_buffer_tail == 0) ? RX_BUFFER_SIZE : rx_buffer_tail;

			// Handle different cases if rx_char
			if (isprint(rx_char)) {
				if (count < CLI_BUFFER_SIZE) {
					cli_buffer[count++] = rx_char;
				}
			} else if (rx_char == '\b') {
				if (count > 0) {
					cli_buffer[--count] = '\0';
				}
			} else if (rx_char == '\r') {
				// Do nothing.
			} else if (rx_char == '\n') {
				cli_buffer[count] = '\0';
				new_command_received = 1;
			}
			HAL_GPIO_TogglePin(PA8_GPIO_Port, PA8_Pin); // TODO: Delete it later
		}

		//
		while (new_command_received) {

			HAL_GPIO_TogglePin(PA9_GPIO_Port, PA9_Pin); // TODO: Delete it later

			char *ch;
			do {
				status = cli_process(cli_buffer, tx_buffer, TX_BUFFER_SIZE);
				ch = tx_buffer;
				while (*ch != '\0') {
					HAL_UART_Transmit(&hlpuart1, (uint8_t*) ch, 1, 100);
					ch++;
				}
				memset(tx_buffer, '\0', TX_BUFFER_SIZE);
			} while (status == CLI_RUNNING);

			HAL_UART_Transmit(&hlpuart1, (uint8_t*)"\n", 1, 100);

			new_command_received = 0;
			count = 0;

			HAL_GPIO_TogglePin(PA9_GPIO_Port, PA9_Pin); // TODO: Delete it later
		}

		osDelay(500);
	}
}

