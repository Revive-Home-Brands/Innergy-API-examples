# PHP API Fetch Example

Here is my PHP example to read the Work Orders API. We have a laravel intranet application that uses these for single api calls. 

## Why Use PHP for API Calls?

### Benefits

1. **Ubiquity** - PHP is everywhere. Most web hosting environments have PHP pre-installed. If you have a web server, you probably have PHP.

2. **Quick to Write** - PHP's syntax is forgiving and flexible. You can write a working API call in minutes without worrying about types or compilation.

3. **Built-in JSON** - `json_encode()` and `json_decode()` are native functions. No libraries needed for parsing API responses.

4. **cURL Integration** - PHP's cURL extension is mature and well-documented. It handles all HTTP methods, headers, authentication and SSL.

5. **Web-Ready** - PHP scripts can run as CLI tools or serve as web endpoints. The same code works in both contexts.

6. **Large Ecosystem** - Composer packages for virtually everything. Need OAuth? Rate limiting? Retry logic? There's a package for that.

### When to Choose PHP

- Building web applications that need to call APIs
- Quick scripts for data fetching or processing
- When your hosting environment is already PHP-based
- Prototyping before building in another language
- WordPress/Laravel/Symfony integrations

### Trade-offs

- **Performance** - PHP is interpreted, not compiled. Slower than Go or C++ for CPU-intensive tasks
- **Memory usage** - Higher memory overhead per request compared to compiled languages
- **Type safety** - Dynamic typing can lead to runtime errors that compiled languages catch earlier
- **Concurrency** - Traditional PHP isn't designed for concurrent operations (though workarounds exist)

PHP is great for quick scripts and web integrations. For high-performance concurrent API calls, I prefer Go.

---

## Code Walkthrough

### 1. Memory Configuration

```php
ini_set('memory_limit', '512M');
```

**What this does:**
- Sets the maximum memory PHP can use to 512 megabytes (we have tons of work orders, so this number may change. 512 is kinda the sweetspot for all the APIs I use)
- API responses can be large, especially with many work orders
- Without this, PHP might hit the default limit (usually 128M) and crash

---

### 2. Loading the .env File

```php
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
```

- Opens a `.env` file (which contains `API_KEY=your_key_here`)
- Uses `file()` to read all lines into an array
- Skips empty lines and comments
- Splits each line at the `=` sign
- Returns an associative array of key-value pairs

**PHP-specific notes:**
- `file()` is a convenient PHP function that reads an entire file into an array
- `FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES` - flags that clean up the input
- `str_starts_with()` - PHP 8+ function (use `substr($line, 0, 1) === '#'` for older PHP)

---

### 3. Making the API Request

```php
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
```

1. Initializes a cURL session with `curl_init()`
2. Sets options using `curl_setopt_array()` for cleaner code
3. `CURLOPT_RETURNTRANSFER` - returns response as string instead of outputting it
4. `CURLOPT_TIMEOUT` - 2-minute timeout for large responses
5. Sets Accept and Api-Key headers
6. Executes and captures HTTP status code
7. Decodes JSON response into a PHP array

**Error handling:**
- Checks for cURL errors (network issues, SSL problems)
- Checks HTTP status code (API errors)
- Checks JSON decode errors (malformed response)

---

### 4. Output Helper

```php
function outputJson(array $data, bool $prettyPrint = false): void {
    header('Content-Type: application/json');
    $flags = $prettyPrint ? JSON_PRETTY_PRINT : 0;
    echo json_encode($data, $flags);
    echo "\n";
}
```

**What this does:**
- Sets the Content-Type header (useful when running as a web script)
- Encodes PHP array as JSON
- `JSON_PRETTY_PRINT` adds indentation for readability
- Outputs to stdout

---

### 5. The Main Function

```php
function main(): void {
    $envPath = '../.env';

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
```

1. Sets the path to the .env file
2. Loads environment variables
3. Checks that API_KEY exists
4. Fetches work orders from the API
5. Extracts the Items array using null coalescing (`??`)
6. Outputs success JSON with count
7. Catches exceptions and outputs error JSON

**PHP patterns:**
- `$apiResponse['Items'] ?? []` - returns empty array if 'Items' key doesn't exist
- `try/catch` - PHP's exception handling
- Wrapping in a `main()` function keeps the global scope clean

---

## Running

### Prerequisites

**Check PHP is installed:**
```bash
php -v
```

**Check cURL extension is enabled:**
```bash
php -m | grep curl
```

If cURL is missing:

macOS (with Homebrew):
```bash
brew install php
```

Ubuntu/Debian:
```bash
sudo apt-get install php-curl
```

### Run from Command Line

```bash
php work_orders.php
```

Or with a custom .env path:
```bash
php work_orders.php --env-path=/path/to/.env
```

---

## Understanding the Output

```json
{
    "success": true,
    "workOrders": [...],
    "count": 150
}
```

- `success` - Whether the API call succeeded
- `workOrders` - Array of work order objects
- `count` - Number of work orders returned

On error:
```json
{
    "success": false,
    "message": "API_KEY not found in .env file"
}
```

---

## Concurrency in PHP

PHP wasn't designed for concurrency like Go, but there are ways to make parallel API calls.

### Why Concurrency Matters for APIs

Without concurrency (sequential):
```
API Call 1: 7 seconds
API Call 2: 7 seconds
API Call 3: 7 seconds
Total: 21 seconds
```

With concurrency (parallel):
```
API Call 1: ─────────► 7 seconds
API Call 2: ─────────► 7 seconds  (running at same time)
API Call 3: ─────────► 7 seconds  (running at same time)
Total: ~7 seconds
```

### Option 1: curl_multi - Built-in Parallel Requests

PHP's cURL extension includes `curl_multi_*` functions for parallel HTTP requests.

```php
function fetchMultipleEndpoints(string $apiKey, array $endpoints): array {
    $multiHandle = curl_multi_init();
    $handles = [];

    // Create a cURL handle for each endpoint
    foreach ($endpoints as $endpoint) {
        $ch = curl_init();
        curl_setopt_array($ch, [
            CURLOPT_URL => "https://app.innergy.com/api/$endpoint",
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_TIMEOUT => 120,
            CURLOPT_HTTPHEADER => [
                'Accept: application/json',
                "Api-Key: $apiKey"
            ]
        ]);

        curl_multi_add_handle($multiHandle, $ch);
        $handles[$endpoint] = $ch;
    }

    // Execute all requests in parallel
    $running = null;
    do {
        curl_multi_exec($multiHandle, $running);
        curl_multi_select($multiHandle);
    } while ($running > 0);

    // Collect results
    $results = [];
    foreach ($handles as $endpoint => $ch) {
        $response = curl_multi_getcontent($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

        if ($httpCode >= 200 && $httpCode < 300) {
            $results[$endpoint] = json_decode($response, true);
        } else {
            $results[$endpoint] = ['error' => "HTTP $httpCode"];
        }

        curl_multi_remove_handle($multiHandle, $ch);
        curl_close($ch);
    }

    curl_multi_close($multiHandle);

    return $results;
}

// Usage
$endpoints = ['projectWorkOrders', 'projects', 'customers'];
$results = fetchMultipleEndpoints($apiKey, $endpoints);

// All 3 endpoints fetched in parallel!
foreach ($results as $endpoint => $data) {
    echo "$endpoint: " . count($data['Items'] ?? []) . " items\n";
}
```

**How it works:**
1. Create multiple cURL handles, one per endpoint
2. Add them all to a "multi handle"
3. `curl_multi_exec()` runs them all simultaneously
4. `curl_multi_select()` waits for activity efficiently
5. Collect results from each handle

**Pros:**
- Built into PHP, no extensions needed
- True parallel network I/O

**Cons:**
- Verbose API
- Only works for HTTP requests, not general concurrency

---

### Option 2: Guzzle with Promises

[Guzzle](https://docs.guzzlephp.org/) is a popular HTTP client that makes async requests easier.

```bash
composer require guzzlehttp/guzzle
```

```php
use GuzzleHttp\Client;
use GuzzleHttp\Promise;

$client = new Client([
    'base_uri' => 'https://app.innergy.com/api/',
    'timeout' => 120,
    'headers' => [
        'Accept' => 'application/json',
        'Api-Key' => $apiKey
    ]
]);

// Start all requests (they run in parallel)
$promises = [
    'workOrders' => $client->getAsync('projectWorkOrders'),
    'projects' => $client->getAsync('projects'),
    'customers' => $client->getAsync('customers'),
];

// Wait for all to complete
$results = Promise\Utils::unwrap($promises);

// Process results
foreach ($results as $name => $response) {
    $data = json_decode($response->getBody(), true);
    echo "$name: " . count($data['Items'] ?? []) . " items\n";
}
```

**Pros:**
- Clean, readable API
- Built-in retry, middleware, error handling
- Promises are easier than curl_multi

**Cons:**
- Requires Composer dependency
- Adds overhead for simple scripts

---

### Option 3: Parallel Extension (PHP 8+)

The [parallel](https://www.php.net/manual/en/book.parallel.php) extension adds true multi-threading to PHP.

```bash
pecl install parallel
```

```php
use parallel\Runtime;
use parallel\Future;

function fetchEndpoint(string $endpoint, string $apiKey): array {
    // Each thread has its own cURL instance
    $ch = curl_init();
    curl_setopt_array($ch, [
        CURLOPT_URL => "https://app.innergy.com/api/$endpoint",
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_TIMEOUT => 120,
        CURLOPT_HTTPHEADER => [
            'Accept: application/json',
            "Api-Key: $apiKey"
        ]
    ]);

    $response = curl_exec($ch);
    curl_close($ch);

    return json_decode($response, true);
}

// Create runtimes (threads)
$runtime1 = new Runtime();
$runtime2 = new Runtime();
$runtime3 = new Runtime();

// Start parallel execution
$future1 = $runtime1->run('fetchEndpoint', ['projectWorkOrders', $apiKey]);
$future2 = $runtime2->run('fetchEndpoint', ['projects', $apiKey]);
$future3 = $runtime3->run('fetchEndpoint', ['customers', $apiKey]);

// Get results (blocks until each is ready)
$workOrders = $future1->value();
$projects = $future2->value();
$customers = $future3->value();
```

**Pros:**
- True multi-threading
- Works for any PHP code, not just HTTP

**Cons:**
- Requires PECL extension (not always available)
- PHP must be compiled with ZTS (thread safety)
- More complex setup

---

### Performance Comparison

| Approach | 4 API Calls (6s each) | Complexity | Dependencies |
|----------|----------------------|------------|--------------|
| Sequential | 24 seconds | Simple | None |
| curl_multi | ~6 seconds | Medium | None |
| Guzzle Promises | ~6 seconds | Clean | Composer |
| parallel ext | ~6 seconds | Complex | PECL |

For most PHP projects, `curl_multi` provides the best balance - it's built-in and gets the job done. For larger projects already using Composer, Guzzle is cleaner.

---

## PHP Version Notes

This script uses PHP 8+ features:
- `str_starts_with()` - PHP 8.0+
- Named arguments in arrays
- Type declarations with `: array`, `: void`

For PHP 7.x compatibility, replace:
```php
// PHP 8+
if (str_starts_with($line, '#')) {

// PHP 7.x
if (substr($line, 0, 1) === '#') {
```
