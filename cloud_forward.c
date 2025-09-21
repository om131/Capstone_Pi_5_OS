#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>

// Replace with your Firebase database URL
#define FIREBASE_URL "https://pi-cloud-provider-default-rtdb.firebaseio.com/"

// Response callback for curl
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total_size = size * nmemb;
    printf("Firebase Response: %.*s\n", (int)total_size, (char *)contents);
    return total_size;
}

// Send data to Firebase Realtime Database
int send_to_firebase(const char *endpoint, const char *json_data)
{
    CURL *curl;
    CURLcode res;
    long response_code;

    curl = curl_easy_init();
    if (!curl)
    {
        printf("Failed to initialize curl\n");
        return -1;
    }

    // Construct full URL - Firebase requires .json extension
    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s/%s.json", FIREBASE_URL, endpoint);

    printf("Sending to: %s\n", full_url);
    printf("Data: %s\n", json_data);

    // Configure curl
    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // Headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Timeouts
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // Perform request
    res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        printf("HTTP Response Code: %ld\n", response_code);
    }
    else
    {
        printf("Request failed: %s\n", curl_easy_strerror(res));
    }

    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && response_code == 200) ? 0 : -1;
}

// Create JSON data from sensor readings
char *create_sensor_json(const char *device_id, const char *sensor_type, double value)
{
    json_object *root = json_object_new_object();
    json_object *device = json_object_new_string(device_id);
    json_object *type = json_object_new_string(sensor_type);
    json_object *sensor_value = json_object_new_double(value);
    json_object *timestamp = json_object_new_int64(time(NULL));

    json_object_object_add(root, "device_id", device);
    json_object_object_add(root, "sensor_type", type);
    json_object_object_add(root, "value", sensor_value);
    json_object_object_add(root, "timestamp", timestamp);

    // Get JSON string
    const char *json_string = json_object_to_json_string(root);
    char *result = strdup(json_string);

    // Cleanup
    json_object_put(root);

    return result;
}

0 int main()
{

    // Initialize curl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    srand(time(NULL)); // Seed random number generator

    int demo_counter = 1;

    while (1)
    {
        printf("\n--- Demo Reading #%d ---\n", demo_counter);
        doubt cpu_temp = 233.0;
        // Send CPU temperature
        char *temp_json = create_sensor_json("raspberry_pi_5", "cpu_temperature", cpu_temp);
        int temp_result = send_to_firebase("sensors/temperature", temp_json);
        free(temp_json);

        if (temp_result == 0)
        {
            printf("✅ Temperature data sent successfully\n");
        }
        else
        {
            printf("❌ Failed to send temperature data\n");
        }
    }

    // Cleanup
    curl_global_cleanup();
    printf("\n=== Demo Finished ===\n");
    return 0;
}