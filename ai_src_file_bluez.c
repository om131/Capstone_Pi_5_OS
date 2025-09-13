#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>

typedef struct {
    DBusConnection *connection;
    char *adapter_path;
} BluetoothManager;

// Error handling helper
static void check_dbus_error(DBusError *error, const char *operation) {
    if (dbus_error_is_set(error)) {
        fprintf(stderr, "D-Bus error in %s: %s\n", operation, error->message);
        dbus_error_free(error);
        exit(1);
    }
}

// Initialize D-Bus connection
BluetoothManager* bluetooth_manager_init() {
    BluetoothManager *manager = malloc(sizeof(BluetoothManager));
    DBusError error;
    
    dbus_error_init(&error);
    
    // Connect to system bus
    manager->connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    check_dbus_error(&error, "connecting to system bus");
    
    if (!manager->connection) {
        fprintf(stderr, "Failed to connect to D-Bus\n");
        exit(1);
    }
    
    manager->adapter_path = strdup("/org/bluez/hci0");
    return manager;
}

// Generic property getter
DBusMessage* get_property(DBusConnection *connection, 
                         const char *path,
                         const char *interface,
                         const char *property) {
    DBusMessage *msg, *reply;
    DBusError error;
    
    dbus_error_init(&error);
    
    // Create method call message
    msg = dbus_message_new_method_call(
        "org.bluez",                           // destination
        path,                                  // object path
        "org.freedesktop.DBus.Properties",     // interface
        "Get"                                  // method
    );
    
    if (!msg) {
        fprintf(stderr, "Failed to create message\n");
        return NULL;
    }
    
    // Add parameters: interface name and property name
    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);
    
    // Send message and get reply
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);
    check_dbus_error(&error, "getting property");
    
    dbus_message_unref(msg);
    return reply;
}

// Generic property setter
void set_property(DBusConnection *connection,
                  const char *path,
                  const char *interface,
                  const char *property,
                  int type,
                  void *value) {
    DBusMessage *msg, *reply;
    DBusError error;
    
    dbus_error_init(&error);
    
    msg = dbus_message_new_method_call(
        "org.bluez",
        path,
        "org.freedesktop.DBus.Properties",
        "Set"
    );
    
    if (!msg) {
        fprintf(stderr, "Failed to create message\n");
        return;
    }
    
    // Build parameters: interface, property, variant value
    DBusMessageIter iter, variant_iter;
    dbus_message_iter_init_append(msg, &iter);
    
    // Interface name
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface);
    
    // Property name  
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);
    
    // Variant containing the actual value
    char type_str[2] = {type, '\0'};
    dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT, type_str, &variant_iter);
    dbus_message_iter_append_basic(&variant_iter, type, value);
    dbus_message_iter_close_container(&iter, &variant_iter);
    
    // Send message
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &error);
    check_dbus_error(&error, "setting property");
    
    if (reply) dbus_message_unref(reply);
    dbus_message_unref(msg);
}

// Power on Bluetooth adapter
void bluetooth_power_on(BluetoothManager *manager) {
    dbus_bool_t powered = TRUE;
    printf("Powering on Bluetooth adapter...\n");
    
    set_property(manager->connection, 
                manager->adapter_path,
                "org.bluez.Adapter1",
                "Powered",
                DBUS_TYPE_BOOLEAN,
                &powered);
    
    printf("Bluetooth adapter powered on\n");
}

// Check if adapter is powered
dbus_bool_t bluetooth_is_powered(BluetoothManager *manager) {
    DBusMessage *reply;
    DBusMessageIter iter, variant_iter;
    dbus_bool_t powered = FALSE;
    
    reply = get_property(manager->connection,
                        manager->adapter_path,
                        "org.bluez.Adapter1", 
                        "Powered");
    
    if (!reply) return FALSE;
    
    // Extract boolean value from variant
    if (dbus_message_iter_init(reply, &iter)) {
        if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_VARIANT) {
            dbus_message_iter_recurse(&iter, &variant_iter);
            if (dbus_message_iter_get_arg_type(&variant_iter) == DBUS_TYPE_BOOLEAN) {
                dbus_message_iter_get_basic(&variant_iter, &powered);
            }
        }
    }
    
    dbus_message_unref(reply);
    return powered;
}

// Start device discovery
void bluetooth_start_discovery(BluetoothManager *manager) {
    DBusMessage *msg, *reply;
    DBusError error;
    
    dbus_error_init(&error);
    
    printf("Starting device discovery...\n");
    
    msg = dbus_message_new_method_call(
        "org.bluez",
        manager->adapter_path,
        "org.bluez.Adapter1",
        "StartDiscovery"
    );
    
    reply = dbus_connection_send_with_reply_and_block(manager->connection, msg, -1, &error);
    check_dbus_error(&error, "starting discovery");
    
    printf("Device discovery started\n");
    
    if (reply) dbus_message_unref(reply);
    dbus_message_unref(msg);
}

// Signal handler for DeviceFound
DBusHandlerResult device_found_handler(DBusConnection *connection,
                                      DBusMessage *message,
                                      void *user_data) {
    if (dbus_message_is_signal(message, "org.bluez.Adapter1", "DeviceFound")) {
        DBusMessageIter iter;
        const char *device_path;
        
        if (dbus_message_iter_init(message, &iter)) {
            // First parameter: device object path
            if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_OBJECT_PATH) {
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

// Subscribe to DeviceFound signals
void bluetooth_monitor_devices(BluetoothManager *manager) {
    DBusError error;
    dbus_error_init(&error);
    
    // Add signal match rule
    dbus_bus_add_match(manager->connection,
        "type='signal',"
        "interface='org.bluez.Adapter1',"
        "member='DeviceFound'", &error);
    check_dbus_error(&error, "adding signal match");
    
    // Register signal handler
    if (!dbus_connection_add_filter(manager->connection, 
                                   device_found_handler, 
                                   manager, NULL)) {
        fprintf(stderr, "Failed to add signal handler\n");
        exit(1);
    }
    
    printf("Monitoring for device discoveries...\n");
}

// Main event loop
void bluetooth_run_event_loop(BluetoothManager *manager) {
    printf("Starting event loop (press Ctrl+C to exit)...\n");
    
    while (dbus_connection_read_write_dispatch(manager->connection, -1)) {
        // Event loop - handles incoming signals and method replies
        // This will call our registered signal handlers
    }
}

// Cleanup
void bluetooth_manager_cleanup(BluetoothManager *manager) {
    if (manager) {
        if (manager->connection) {
            dbus_connection_unref(manager->connection);
        }
        free(manager->adapter_path);
        free(manager);
    }
}

// Example usage
int main() {
    BluetoothManager *manager = bluetooth_manager_init();
    
    // Check if adapter is already powered
    if (!bluetooth_is_powered(manager)) {
        bluetooth_power_on(manager);
        
        // Wait a moment for adapter to power up
        sleep(1);
    }
    
    // Set up device monitoring
    bluetooth_monitor_devices(manager);
    
    // Start discovery
    bluetooth_start_discovery(manager);
    
    // Run event loop to receive signals
    bluetooth_run_event_loop(manager);
    
    // Cleanup (won't reach here due to infinite loop)
    bluetooth_manager_cleanup(manager);
    
    return 0;
}