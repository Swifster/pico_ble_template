#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ble/gatt-service/nordic_spp_service_server.h"
#include "btstack.h"
#include "hardware/adc.h"
#include "pico/btstack_cyw43.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "bluefruit_uart.h"
#include "pico_ble_stack.h"

#define HEARTBEAT_PERIOD_MS 1000
#define ADC_CHANNEL_TEMPSENSOR 4
#define APP_AD_FLAGS 0x06
#define DEVICE_NAME_MAX_LEN 29

static uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_128_BIT_SERVICE_CLASS_UUIDS,
    0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0,
    0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e,
};
static const uint8_t adv_data_len = sizeof(adv_data);

static char device_name[DEVICE_NAME_MAX_LEN + 1] = "UART-ND";
static uint8_t scan_response_data[2 + DEVICE_NAME_MAX_LEN];
static uint8_t scan_response_data_len;

static hci_con_handle_t spp_con_handle = HCI_CON_HANDLE_INVALID;
static float current_temp_c;
static char spp_line[64];
static uint16_t spp_line_len;
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_context_callback_registration_t spp_send_request;
static pico_ble_stack_handlers_t handlers;

extern uint8_t const profile_data[];

static void nordic_can_send(void *context);

static void update_scan_response_data(void)
{
    size_t name_len = strnlen(device_name, DEVICE_NAME_MAX_LEN);

    scan_response_data[0] = (uint8_t)(name_len + 1);
    scan_response_data[1] = BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
    memcpy(&scan_response_data[2], device_name, name_len);
    scan_response_data_len = (uint8_t)(name_len + 2);
}

static uint16_t att_read_callback(hci_con_handle_t connection_handle,
                                  uint16_t att_handle,
                                  uint16_t offset,
                                  uint8_t *buffer,
                                  uint16_t buffer_size)
{
    UNUSED(connection_handle);

    if (att_handle == ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE) {
        return att_read_callback_handle_blob((const uint8_t *)device_name,
                                             (uint16_t)strnlen(device_name, DEVICE_NAME_MAX_LEN),
                                             offset,
                                             buffer,
                                             buffer_size);
    }

    return 0;
}

static void poll_temp(void)
{
    adc_select_input(ADC_CHANNEL_TEMPSENSOR);
    uint32_t raw32 = adc_read();
    const uint32_t bits = 12;
    uint16_t raw16 = raw32 << (16 - bits) | raw32 >> (2 * bits - 16);

    const float conversion_factor = 3.3f / 65535.0f;
    float reading = raw16 * conversion_factor;
    current_temp_c = 27.0f - (reading - 0.706f) / 0.001721f;

    printf("Write temp %.2f degc\n", current_temp_c);
}

void pico_ble_stack_set_handlers(const pico_ble_stack_handlers_t *new_handlers)
{
    if (new_handlers == NULL) {
        memset(&handlers, 0, sizeof(handlers));
    } else {
        handlers = *new_handlers;
    }
}

void pico_ble_stack_set_device_name(const char *name)
{
    if (name == NULL || name[0] == '\0') {
        return;
    }

    snprintf(device_name, sizeof(device_name), "%s", name);
}

const char *pico_ble_stack_get_device_name(void)
{
    return device_name;
}

bool pico_ble_stack_uart_is_connected(void)
{
    return spp_con_handle != HCI_CON_HANDLE_INVALID;
}

float pico_ble_stack_get_temperature_c(void)
{
    return current_temp_c;
}

void pico_ble_stack_uart_send(const char *message)
{
    if (message == NULL || spp_con_handle == HCI_CON_HANDLE_INVALID) {
        return;
    }

    spp_line_len = (uint16_t)snprintf(spp_line, sizeof(spp_line), "%s", message);

    spp_send_request.callback = &nordic_can_send;
    nordic_spp_service_server_request_can_send_now(&spp_send_request, spp_con_handle);
}

void pico_ble_stack_uart_sendf(const char *format, ...)
{
    if (format == NULL || spp_con_handle == HCI_CON_HANDLE_INVALID) {
        return;
    }

    va_list args;
    va_start(args, format);
    spp_line_len = (uint16_t)vsnprintf(spp_line, sizeof(spp_line), format, args);
    va_end(args);

    spp_send_request.callback = &nordic_can_send;
    nordic_spp_service_server_request_can_send_now(&spp_send_request, spp_con_handle);
}

static void nordic_can_send(void *context)
{
    UNUSED(context);

    if (spp_con_handle == HCI_CON_HANDLE_INVALID || spp_line_len == 0) {
        return;
    }

    nordic_spp_service_server_send(spp_con_handle, (const uint8_t *)spp_line, spp_line_len);
}

static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(size);
    UNUSED(channel);

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    switch (hci_event_packet_get_type(packet)) {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
            return;
        }

        bd_addr_t local_addr;
        gap_local_bd_addr(local_addr);
        printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));

        bd_addr_t null_addr;
        memset(null_addr, 0, sizeof(null_addr));
        gap_advertisements_set_params(800, 800, 0, 0, null_addr, 0x07, 0x00);
        assert(adv_data_len <= 31);
        gap_advertisements_set_data(adv_data_len, adv_data);
        update_scan_response_data();
        gap_scan_response_set_data(scan_response_data_len, (uint8_t *)scan_response_data);
        gap_advertisements_enable(1);

        poll_temp();
        break;

    default:
        break;
    }
}

static void nordic_spp_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(channel);

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        if (hci_event_packet_get_type(packet) != HCI_EVENT_GATTSERVICE_META) {
            break;
        }

        switch (hci_event_gattservice_meta_get_subevent_code(packet)) {
        case GATTSERVICE_SUBEVENT_SPP_SERVICE_CONNECTED:
            spp_con_handle = gattservice_subevent_spp_service_connected_get_con_handle(packet);
            printf("Bluefruit UART connected: 0x%04x\n", spp_con_handle);
            if (handlers.on_connection_change != NULL) {
                handlers.on_connection_change(true);
            }
            break;

        case GATTSERVICE_SUBEVENT_SPP_SERVICE_DISCONNECTED:
            printf("Bluefruit UART disconnected: 0x%04x\n", spp_con_handle);
            spp_con_handle = HCI_CON_HANDLE_INVALID;
            if (handlers.on_connection_change != NULL) {
                handlers.on_connection_change(false);
            }
            break;

        default:
            break;
        }
        break;

    case RFCOMM_DATA_PACKET:
        printf("Bluefruit RX: ");
        printf_hexdump(packet, size);
        if (handlers.on_uart_receive != NULL) {
            handlers.on_uart_receive(packet, size);
        }
        break;

    default:
        break;
    }
}

static void heartbeat_handler(void)
{
    poll_temp();

    if (handlers.on_tick != NULL) {
        handlers.on_tick();
    }
}

int pico_ble_stack_run(void)
{
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }

    adc_init();
    adc_select_input(ADC_CHANNEL_TEMPSENSOR);
    adc_set_temp_sensor_enabled(true);

    l2cap_init();
    sm_init();

    att_server_init(profile_data, att_read_callback, NULL);
    nordic_spp_service_server_init(&nordic_spp_packet_handler);

    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    hci_power_control(HCI_POWER_ON);

    absolute_time_t next_heartbeat = make_timeout_time_ms(HEARTBEAT_PERIOD_MS);
    while (true) {
        async_context_poll(cyw43_arch_async_context());

        if (absolute_time_diff_us(get_absolute_time(), next_heartbeat) <= 0) {
            heartbeat_handler();
            next_heartbeat = make_timeout_time_ms(HEARTBEAT_PERIOD_MS);
        }

        async_context_wait_for_work_until(cyw43_arch_async_context(), next_heartbeat);
    }
}
