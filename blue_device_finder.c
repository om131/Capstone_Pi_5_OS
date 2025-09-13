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

bool bluez_adapter_powered(BluetoothManager *Manager)
{
    DBusMessage *msg, *reply;
    DBusError error;
    dbus_bool_t powered = TRUE;
    char *property = "Powered";
    char *interface = "org.freedesktop.DBus.Properties";

    msg = dbus_message_new_method_call(
        "org.bluez",                       // destination
        Manager->adapter_path,             // object path
        "org.freedesktop.DBus.Properties", // interface
        "Get"                              // method
    );
    // Add parameters: interface name and property name
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

    reply = dbus_connection_send_with_reply_and_block(Manager->connection, msg, -1, &error);
    dbus_message_unref(msg);

    if (!reply)
    { // if Not powered ON then manually power ON.
        printf("Powering on Bluetooth adapter...\n");

        msg = dbus_message_new_method_call(
            "org.bluez",                       // destination
            Manager->adapter_path,             // object path
            "org.freedesktop.DBus.Properties", // interface
            "Set"                              // method
        );
        if (!msg)
        {
            fprintf(stderr, "Failed to create message\n");
            return 0;
        }

        DBusMessageIter iter, variant_iter;
        dbus_message_iter_init_append(msg, &iter);
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
        dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);

        char type_str[2] = {DBUS_TYPE_BOOLEAN, '\0'};
        dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, type_str, &variant_iter);
        dbus_message_iter_append_basic(&variant_iter, DBUS_TYPE_BOOLEAN, &powered);
        dbus_message_iter_close_container(&iter, &variant_iter);
        reply = dbus_connection_send_with_reply_and_block(Manager->connection, msg, -1, &error);
        check_dbus_error(&error, "setting property");
    }
    else
    {
        return 1;
    }
}

// bool bluez_enable_discoverable()
// {
//     DBusConnection connection_dbus;
//     DBusError error;

//     dbus_error_init(&error); // Init the Error system of Dbus

//     // Discoverey enable
//     DBusMessage *method_call = dbus_message_new_method_call(const char *destination,
//                                                             const char *path,
//                                                             const char *interface,
//                                                             StartDiscovery);
// }

void main(void)
{
    BluetoothManager *Manager;

    Manager->adapter_path = strdup("/org/bluez/hci0");

    Manager = bluez_init();

    if (bluez_adapter_powered(Manager))
    {
        printf("Powered ON----->>>");
    }
    else
    {
        printf("Fialed powertred ON");
    }

    // Init the Bluetooth lib for scanning
    // bluez_enable_discoverable();
}