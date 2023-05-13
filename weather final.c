#include <stdio.h>
#include <stdlib.h>
#include <string.h>						
#define API_ENDPOINT "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s"
#define API_KEY "55b3949b4c7ade2ced744b65036b34ad"

int get_weather_data(json_t *json_data, char *data_field, char *result)
{
    json_t *field_value = json_object_get(json_data, data_field);
    if (!json_is_number(field_value))
        return -1;
    snprintf(result, 32, "%.1f", json_real_value(field_value));
    return 0;
}

int main()
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return EXIT_FAILURE;
    }

    char location[128];
    printf("Enter location: ");
    fgets(location, 128, stdin);
    location[strcspn(location, "\n")] = '\0';

    char api_url[1024];
    snprintf(api_url, 1024, API_ENDPOINT, location, API_KEY);
    curl_easy_setopt(curl, CURLOPT_URL, api_url);

    char response_buffer[4096];
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Failed to fetch weather data: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    json_t *json_data = json_loads(response_buffer, 0, NULL);
    if (!json_data)
    {
        fprintf(stderr, "Failed to parse JSON data\n");
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    char temp[32], humidity[32], wind_speed[32];
    if (get_weather_data(json_data, "main.temp", temp) != 0 ||
        get_weather_data(json_data, "main.humidity", humidity) != 0 ||
        get_weather_data(json_data, "wind.speed", wind_speed) != 0)
    {
        fprintf(stderr, "Failed to extract weather data from JSON\n");
        json_decref(json_data);
        curl_easy_cleanup(curl);
        return EXIT_FAILURE;
    }

    printf("Temperature: %s K\n", temp);
    printf("Humidity: %s %%\n", humidity);
    printf("Wind speed: %s m/s\n", wind_speed);

    json_decref(json_data);
    curl_easy_cleanup(curl);

    return EXIT_SUCCESS;
}
