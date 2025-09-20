#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <stdbool.h>

typedef struct
{
    DBusConnection *connection;
    char *adapter_path;
} BluetoothManager;

static void check_dbus_error(DBusError *error, const char *operation)
{
    if (dbus_error_is_set(error))
    {
        fprintf(stderr, "D-Bus error in %s: %s\n", operation, error->message);
        dbus_error_free(error);
        exit(1);
    }
}

BluetoothManager *bluez_init()
{
    BluetoothManager *mang = malloc(sizeof(BluetoothManager));

    DBusError error;

    dbus_error_init(&error);

    mang->connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

    check_dbus_error(&error, "Connecting to system Bus");

    if (!mang->connection)
    {
        fprintf(stderr, "D-Bus bus connection failed");
    }

    return mang;
}

int set_property(BluetoothManager *Manager, char *property, dbus_bool_t *value)
{
    DBusMessage *msg, *reply;
    DBusMessageIter iter, variant_iter;
    DBusError error;
    char *interface = "org.bluez.Adapter1";

    /*Init the Error*/
    dbus_error_init(&error);

    msg = dbus_message_new_method_call(
        "org.bluez",                       // destination
        "/org/bluez/hci0",                 // object path
        "org.freedesktop.DBus.Properties", // interface
        "Set"                              // method
    );

    if (dbus_error_is_set(&error))
    {
        check_dbus_error(&error, "message created for setting prop");
    }

    // Initialize the argument iterator
    dbus_message_iter_init_append(msg, &iter);

    // Argument 1: Interface name ("org.bluez.Adapter1")
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);

    // Argument 2: Property name ("Powered")
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

    // Argument 3: The new value (wrapped in a variant)
    dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, "b", &variant_iter);
    dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, value);
    dbus_message_iter_close_container(&iter, &variant_iter);

    reply = dbus_connection_send_with_reply_and_block(Manager->connection, msg, -1, &error);
    check_dbus_error(&error, "Setting property...");

    if (dbus_error_is_set(&error))
    {
        dbus_error_free(&error);
        dbus_message_unref(msg);
        return -1;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);
    return 1;
}

bool bluez_adapter_powered(BluetoothManager *Manager)
{
    DBusMessage *msg, *reply;
    DBusError error;
    dbus_bool_t powered = TRUE;
    char *property = "Powered";
    char *interface = "org.bluez.Adapter1";
    dbus_error_init(&error);

    msg = dbus_message_new_method_call(
        "org.bluez",                       // destination
        "/org/bluez/hci0",                 // object path
        "org.freedesktop.DBus.Properties", // interface
        "Get"                              // method
    );
    if (!msg)
    {
        fprintf(stderr, "Failed to create message\n");
        return 0;
    }
    // Add parameters: interface name and property name
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

    reply = dbus_connection_send_with_reply_and_block(Manager->connection, msg, -1, &error);
    dbus_message_unref(msg);

    if (!reply)
    {
        return 0;
    }
    else
    {
        DBusMessageIter iter, variant_iter;

        if (dbus_message_iter_init(reply, &iter))
        {
            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT)
            {
                dbus_message_iter_recurse(&iter, &variant_iter);
                if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN)
                {
                    dbus_message_iter_get_basic(&variant_iter, &powered);
                }
            }
        }

        dbus_message_unref(reply);
        printf("Powered Vard-> %d/n", powered);

        if (powered == false)
        {
            set_property(Manager, "Powered", &powered);
        }
        return powered;
    }
}

DBusHandlerResult device_handle_dbus(DBusConnection *connection,
                                     DBusMessage *message,
                                     void *user_data)
{
    if (dbus_message_is_signal(message, "org.bluez.Adapter1", "DeviceFound"))
    {
        DBusMessageIter iter;
        const char *device_path;

        if (dbus_message_iter_init(message, &iter))
        {
            // First parameter: device object path
            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH)
            {
                dbus_message_iter_get_basic(&iter, &device_path);
                printf("Found device: %s\n", device_path);

                // Second parameter: properties dictionary
                // (You can parse this to get device name, address, etc.)
            }
        }

        return DBUS_HANDLER_RESULT_HANDLED;
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void blwuz_signal_init(BluetoothManager *Manager)
{
    DBusError error;
    dbus_error_init(&error);

    dbus_bus_add_match(Manager->connection,
                       "type='signal',interface='org.bluez.Adapter1',member='DeviceFound'",
                       &error);

    check_dbus_error(&error, "adding signal match");

    if (!dbus_connection_add_filter(Manager->connection,
                                    device_handle_dbus,
                                    Manager, NULL))
    {
        fprintf(stderr, "Failed to add signal handler\n");
        exit(1);
    }
}

bool bluez_enable_discoverable(BluetoothManager *Manager)
{
    DBusMessage *msg, *reply;
    DBusError error;

    dbus_error_init(&error); // Init the Error system of Dbus

    // Discoverey enable
    msg = dbus_message_new_method_call(
        "org.bluez",          // destination
        "/org/bluez/hci0",    // object path
        "org.bluez.Adapter1", // interface
        "StartDiscovery"      // method
    );

    reply = dbus_connection_send_with_reply_and_block(Manager->connection, msg, -1, &error);
    check_dbus_error(&error, "starting discovery");

    if (reply)
        dbus_message_unref(reply);
    dbus_message_unref(msg);
}

void bluetooth_run_event_loop(BluetoothManager *manager)
{
    printf("Starting event loop (press Ctrl+C to exit)...\n");

    while (dbus_connection_read_write_dispatch(manager->connection, -1))
    {
        // Event loop - handles incoming signals and method replies
        // This will call our registered signal handlers
    }
}

int main(void)
{
    BluetoothManager *Manager;
    int powere_is_on_off = false;
    dbus_bool_t powered = TRUE;

    Manager = bluez_init();

    Manager->adapter_path = strdup("/org/bluez/hci0");

    printf("Adapter path: %s\n", Manager->adapter_path);
    printf("Connection valid: %s\n", Manager->connection ? "YES" : "NO");

    powere_is_on_off = set_property(Manager, "Powered", &powered);

    if (powere_is_on_off == 1)
    {
        // bluez_adapter_powered(Manager);
        printf("Powered ON----->>>\n");
    }
    else
    {
        printf("Fialed powertred ON\n");
    }
    // blwuz_signal_init(Manager);
    // // Init the Bluetooth lib for scanning
    // bluez_enable_discoverable(Manager);

    // bluetooth_run_event_loop(Manager);
    // dbus_connection_unref(conn);

    return 0;
}