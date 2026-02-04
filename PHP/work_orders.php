<?php
/**
 * Work Orders API Fetch Example
 *
 * Run:
 *   php PHP/work_orders.php
 *   php PHP/work_orders.php --env-path=/path/to/.env
 */

ini_set('memory_limit', '512M');

/**
 * loadEnvFile - Reads a .env file and returns an associative array.
 *
 *   1. Checks if the file exists, throws exception if not
 *   2. Reads all lines from the file, ignoring empty lines
 *   3. Skips lines that start with # comments
 *   4. Splits each line at the first = sign
 *   5. Trims whitespace and removes surrounding quotes from values
 *   6. Returns an associative array with all key-value pairs
 */
function loadEnvFile(string $filepath): array {
    $env = [];

    if (!file_exists($filepath)) {
        throw new Exception("Environment file not found: $filepath");
    }

    $lines = file($filepath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);

    foreach ($lines as $line) {
        $line = trim($line);

        if (str_starts_with($line, '#')) {
            continue;
        }

        $parts = explode('=', $line, 2);
        if (count($parts) !== 2) {
            continue;
        }

        $key = trim($parts[0]);
        $value = trim($parts[1]);
        $value = trim($value, '"\'');

        $env[$key] = $value;
    }

    return $env;
}

/**
 * fetchWorkOrders - Makes an HTTP GET request to the Innergy API.
 *
 *   1. Initializes a cURL session
 *   2. Sets the URL to the projectWorkOrders endpoint
 *   3. Configures cURL to return the response as a string
 *   4. Sets a 120 second timeout for large responses
 *   5. Adds Accept and Api-Key headers for authentication
 *   6. Executes the request and captures the response
 *   7. Checks for cURL errors and HTTP status codes
 *   8. Decodes the JSON response and returns it as an array
 */
function fetchWorkOrders(string $apiKey): array {
    $url = 'https://app.innergy.com/api/projectWorkOrders';

    $ch = curl_init();

    curl_setopt_array($ch, [
        CURLOPT_URL => $url,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_TIMEOUT => 120,
        CURLOPT_HTTPHEADER => [
            'Accept: application/json',
            "Api-Key: $apiKey"
        ]
    ]);

    $response = curl_exec($ch);
    $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $error = curl_error($ch);

    curl_close($ch);

    if (!empty($error)) {
        throw new Exception("cURL error: $error");
    }

    if ($httpCode < 200 || $httpCode >= 300) {
        throw new Exception("API returned status $httpCode");
    }

    $data = json_decode($response, true);

    if (json_last_error() !== JSON_ERROR_NONE) {
        throw new Exception("JSON decode error: " . json_last_error_msg());
    }

    return $data;
}

/**
 * outputJson - Outputs data as a JSON string to stdout.
 *
 *   1. Sets the Content-Type header for JSON, useful if run as web script
 *   2. Encodes the data array as JSON
 *   3. Optionally pretty-prints with indentation if $prettyPrint is true
 *   4. Outputs the JSON string followed by a newline
 */
function outputJson(array $data, bool $prettyPrint = false): void {
    header('Content-Type: application/json');
    $flags = $prettyPrint ? JSON_PRETTY_PRINT : 0;
    echo json_encode($data, $flags);
    echo "\n";
}

/**
 * main - Entry point of the program.
 *
 *   1. Parses command line arguments for --env-path option
 *   2. Loads environment variables from the .env file
 *   3. Checks that API_KEY exists in the environment
 *   4. Calls fetchWorkOrders to get data from the API
 *   5. Extracts the Items array from the response
 *   6. Outputs a success JSON with workOrders and count
 *   7. Catches any exceptions and outputs error JSON instead
 */
function main(): void {
    $envPath = '../.env' ?? '';

    try {
        $env = loadEnvFile($envPath);

        if (empty($env['API_KEY'])) {
            throw new Exception("API_KEY not found in .env file");
        }

        $apiResponse = fetchWorkOrders($env['API_KEY']);

        $workOrders = $apiResponse['Items'] ?? [];

        outputJson([
            'success' => true,
            'workOrders' => $workOrders,
            'count' => count($workOrders)
        ], true);

    } catch (Exception $e) {
        outputJson([
            'success' => false,
            'message' => $e->getMessage()
        ], true);
    }
}

main();
