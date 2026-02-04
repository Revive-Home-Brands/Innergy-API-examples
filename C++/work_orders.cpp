/**
 * Work Orders API Fetch Example
 *
 * Dependencies: libcurl
 *
 * Install on Mac:
 *   xcode-select --install
 *
 * Install on Ubuntu/Debian:
 *   sudo apt-get install g++ libcurl4-openssl-dev
 *
 * Build:
 *   g++ -std=c++17 -o work_orders work_orders.cpp -lcurl
 *
 * Run:
 *   ./work_orders
 *   ./work_orders --env-path=/path/to/.env
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <curl/curl.h>

/**
 * JsonWriter - Helper class for JSON string operations.
 *
 * This class provides static methods for working with JSON strings.
 * Since we're not using an external JSON library, we need to handle
 * escaping special characters and formatting manually.
 */
class JsonWriter {
public:
    /**
     * escape - Escapes special characters in a string for JSON output.
     * really meant for readability.
     */
    static std::string escape(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }

    /**
     * prettyPrint - Formats a JSON string with indentation and newlines.
     *
     *   1. Iterates through each character in the JSON string
     *   2. Tracks whether we're inside a quoted string to avoid formatting string contents
     *   3. When encountering { or [: adds newline and increases indent
     *   4. When encountering } or ]: decreases indent and adds newline before
     *   5. When encountering comma: adds newline and current indentation
     *   6. When encountering colon: adds a space after for readability
     *   7. Skips whitespace outside of strings, we add our own formatting
     *   8. Returns the formatted JSON string
     */
    static std::string prettyPrint(const std::string& json) {
        std::string result;
        int indent = 0;
        bool inString = false;
        char prevChar = 0;

        for (size_t i = 0; i < json.length(); i++) {
            char c = json[i];

            if (c == '"' && prevChar != '\\') {
                inString = !inString;
            }

            if (!inString) {
                if (c == '{' || c == '[') {
                    result += c;
                    result += '\n';
                    indent++;
                    result += std::string(indent * 2, ' ');
                } else if (c == '}' || c == ']') {
                    result += '\n';
                    indent--;
                    result += std::string(indent * 2, ' ');
                    result += c;
                } else if (c == ',') {
                    result += c;
                    result += '\n';
                    result += std::string(indent * 2, ' ');
                } else if (c == ':') {
                    result += c;
                    result += ' ';
                } else if (c != ' ' && c != '\n' && c != '\r' && c != '\t') {
                    result += c;
                }
            } else {
                result += c;
            }

            prevChar = c;
        }

        return result;
    }
};

/**
 * loadEnvFile - Reads a .env file and returns a map of key-value pairs.
 *
 *   1. Opens the file at the given filepath using ifstream
 *   2. Throws an exception if the file cannot be opened
 *   3. Reads the file line by line using std::getline
 *   4. Trims leading and trailing whitespace from each line
 *   5. Skips empty lines and lines starting with # (comments)
 *   6. Finds the first = sign and splits into key and value
 *   7. Trims whitespace from both key and value
 *   8. Removes surrounding quotes (single or double) from values
 *   9. Stores the pair in a std::map and returns it
 */
std::map<std::string, std::string> loadEnvFile(const std::string& filepath) {
    std::map<std::string, std::string> env;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open .env file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }

        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if ((value.front() == '"' && value.back() == '"') ||
            (value.front() == '\'' && value.back() == '\'')) {
            value = value.substr(1, value.length() - 2);
        }

        env[key] = value;
    }

    return env;
}

/**
 * writeCallback - Callback function for cURL to handle response data.
 *
 *   1. cURL calls this function each time it receives a chunk of data
 *   2. The data comes as a void pointer with size information
 *   3. We calculate the total size * nmemb
 *   4. Cast the void pointer to char* and append to our response string
 *   5. Return the number of bytes processed. cURL expects this
 *
 * Data arrives in chunks, not all at once, so we accumulate it.
 */
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

/**
 * fetchWorkOrders - Makes an HTTP GET request to the Innergy API.
 *
 *   1. Initializes a cURL easy handle the connection object
 *   2. Creates an empty string to store the response
 *   3. Sets up HTTP headers: Accept for JSON, Api-Key for auth
 *   4. Configures cURL options:
 *      - URL: the API endpoint
 *      - Headers: our custom headers list
 *      - Write function: our callback to handle response
 *      - Write data: pointer to our response string
 *      - Timeout: 120 seconds for large responses
 *   5. Executes the request with curl_easy_perform
 *   6. Gets the HTTP response code
 *   7. Cleans up the headers list and cURL handle
 *   8. Checks for cURL errors and throws if any
 *   9. Checks HTTP status code and throws if not 2xx
 *   10. Returns the response string
 */
std::string fetchWorkOrders(const std::string& apiKey) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize cURL");
    }

    std::string response;
    std::string url = "https://app.innergy.com/api/projectWorkOrders";

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    std::string apiKeyHeader = "Api-Key: " + apiKey;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("cURL error: ") + curl_easy_strerror(res));
    }

    if (httpCode < 200 || httpCode >= 300) {
        throw std::runtime_error("API returned status " + std::to_string(httpCode));
    }

    return response;
}

/**
 * outputSuccess - Outputs a success JSON response to stdout.
 *
 *   1. Counts the number of work orders by finding "Id": patterns
 *      Simple parsing without a JSON library
 *   2. Pretty prints the API response using JsonWriter::prettyPrint
 *   3. Outputs a JSON object with:
 *      - success: true
 *      - count: number of items found
 *      - data: the formatted API response
 */
void outputSuccess(const std::string& apiResponse) {
    int count = 0;
    size_t pos = 0;
    while ((pos = apiResponse.find("\"Id\":", pos)) != std::string::npos) {
        count++;
        pos++;
    }

    std::string formattedData = JsonWriter::prettyPrint(apiResponse);

    std::cout << "{\n";
    std::cout << "  \"success\": true,\n";
    std::cout << "  \"count\": " << count << ",\n";
    std::cout << "  \"data\": " << formattedData << "\n";
    std::cout << "}" << std::endl;
}

/**
 * outputError - Outputs an error JSON response to stdout.
 *
 * How it works:
 *   1. Escapes any special characters in the error message
 *   2. Outputs a JSON object with:
 *      - success: false
 *      - message: the escaped error message
 */
void outputError(const std::string& message) {
    std::cout << "{\n";
    std::cout << "  \"success\": false,\n";
    std::cout << "  \"message\": \"" << JsonWriter::escape(message) << "\"\n";
    std::cout << "}" << std::endl;
}

/**
 * parseEnvPath - Parses command line arguments for the --env-path option.
 *
 * How it works:
 *   1. Sets a default path for the .env file
 *   2. Loops through all command line arguments
 *   3. Looks for arguments starting with "--env-path="
 *   4. Extracts the path after the = sign
 *   5. Returns the path
 */
std::string parseEnvPath(int argc, char* argv[]) {
    std::string envPath = "../.env";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.find("--env-path=") == 0) {
            envPath = arg.substr(11);
        }
    }

    return envPath;
}

/**
 * main - Entry point of the program.
 *
 *   1. Initializes cURL globally (required once before any cURL calls)
 *   2. Parses command line arguments to get .env file path
 *   3. Loads environment variables from the .env file
 *   4. Checks that API_KEY exists and is not empty
 *   5. Calls fetchWorkOrders to get data from the API
 *   6. Outputs the successful response as formatted JSON
 *   7. Catches any exceptions and outputs error JSON instead
 *   8. Cleans up cURL globally before exiting
 *   9. Returns 0 for success
 */
int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    try {
        std::string envPath = parseEnvPath(argc, argv);
        auto env = loadEnvFile(envPath);

        if (env.find("API_KEY") == env.end() || env["API_KEY"].empty()) {
            throw std::runtime_error("API_KEY not found in .env file");
        }

        std::string response = fetchWorkOrders(env["API_KEY"]);
        outputSuccess(response);

    } catch (const std::exception& e) {
        outputError(e.what());
    }

    curl_global_cleanup();

    return 0;
}
