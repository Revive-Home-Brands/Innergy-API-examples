# C++ API Fetch Example

Here is my C++ example to read the Work Orders API.

## Why Use C++ for API Calls?

### Benefits

1. **Speed** - C++ is one of the fastest programming languages. Once compiled, the code runs directly as machine code with no interpreter overhead. Works on any machine, no dependencies since it's compiled meaning you don't have to download any programming languages.

2. **Memory Efficiency** - C++ gives you direct control over memory allocation. For large API responses, this means lower memory usage compared to languages like Python or PHP.

3. **No Runtime Required** - The compiled binary runs without needing Python, PHP or Node.js installed. Just the binary file works on any compatible system.

4. **Predictable Performance** - No garbage collection pauses or just-in-time compilation. The code runs at consistent speeds.

5. **System Integration** - C++ can easily integrate with operating system features, other C/C++ libraries, and embedded systems.

### When to Choose C++

- Building high-performance backend services
- Embedded systems or IoT devices with limited resources
- Applications where startup time matters (C++ starts instantly)
- When you need to integrate with existing C/C++ codebases

### Trade-offs

- **Longer development time** - More code is needed compared to Python or PHP
- **Manual memory management** - You must be careful with memory allocation
- **Compilation required** - Changes require recompiling before testing

---

## Code Walkthrough

### 1. Include Headers

```cpp
#include <iostream>      // For console output (std::cout)
#include <fstream>       // For reading files (.env file)
#include <sstream>       // For string manipulation
#include <string>        // For std::string type
#include <map>           // For key-value storage (like a dictionary)
#include <chrono>        // For timing/measuring performance
#include <iomanip>       // For formatting output (decimal places)
#include <curl/curl.h>   // For making HTTP requests
```

---

### 2. The JsonWriter Class

```cpp
class JsonWriter {
public:
    static std::string escape(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                default: result += c;
            }
        }
        return result;
    }
};
```

**What this does:**
- JSON strings need special characters to be escaped
- A quote `"` becomes `\"` so it doesn't break the JSON format
- A newline becomes `\n` so it stays on one line in the JSON
---

### 3. Loading the .env File

```cpp
std::map<std::string, std::string> loadEnvFile(const std::string& filepath) {
    std::map<std::string, std::string> env;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open .env file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Find the = sign
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        // Split into key and value
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        env[key] = value;
    }

    return env;
}
```

**What this does:**
- Opens a `.env` file (which contains `API_KEY=your_key_here`)
- Reads it line by line
- Splits each line at the `=` sign
- Stores key-value pairs in a map
---

### 4. The cURL Write Callback

```cpp
size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}
```

**What this does:**
- cURL calls this function every time it receives data from the server
- The data arrives in chunks, not all at once
- Each chunk gets appended to our `response` string

**Why is this needed?**
cURL is a C library, not C++. It uses callback functions to let you decide how to handle incoming data. This is a common pattern when working with C libraries.
---

### 5. Making the API Request

```cpp
std::string fetchWorkOrders(const std::string& apiKey) {
    // Initialize cURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize cURL");
    }

    std::string response;
    std::string url = "https://app.innergy.com/api/projectWorkOrders";

    // Set up HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    std::string apiKeyHeader = "Api-Key: " + apiKey;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());

    // Configure the request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    // Execute the request
    CURLcode res = curl_easy_perform(curl);

    // Check HTTP status code
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    // Clean up
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Handle errors
    if (res != CURLE_OK) {
        throw std::runtime_error(std::string("cURL error: ") + curl_easy_strerror(res));
    }

    if (httpCode < 200 || httpCode >= 300) {
        throw std::runtime_error("API returned status " + std::to_string(httpCode));
    }

    return response;
}
```

**What this does:**
1. Creates a cURL handle
2. Sets the URL to fetch
3. Adds headers
4. Tells cURL to use our callback function for response data
5. Executes the request
6. Cleans up memory
7. Returns the response or throws an error
---

### 6. The Main Function

```cpp
int main(int argc, char* argv[]) {
    // Initialize cURL globally
    curl_global_init(CURL_GLOBAL_DEFAULT);

    try {
        std::string envPath = parseEnvPath(argc, argv);

        // Load environment variables
        auto env = loadEnvFile(envPath);

        if (env.find("API_KEY") == env.end() || env["API_KEY"].empty()) {
            throw std::runtime_error("API_KEY not found in .env file");
        }

        // Fetch work orders
        std::string response = fetchWorkOrders(env["API_KEY"]);

        // Output success response
        outputSuccess(response);

    } catch (const std::exception& e) {
        outputError(e.what());
    }

    // Cleanup cURL globally
    curl_global_cleanup();

    return 0;
}
```

**What this does:**
1. Initializes the cURL library
3. Loads the API key from the .env file
4. Makes the API request
5. Outputs the result as JSON
6. Handles any errors gracefully
7. Cleans up resources
---

## Building and Running

### Prerequisites

**macOS:**
```bash
xcode-select --install
```

**Ubuntu/Debian:**
```bash
sudo apt-get install g++ libcurl4-openssl-dev
```

### Compile

```bash
g++ -std=c++17 -o work_orders work_orders.cpp -lcurl
```

**What the flags mean:**
- `g++` - The C++ compiler
- `-std=c++17` - Use C++17 standard
- `-o work_orders` - Output filename
- `work_orders.cpp` - Input source file
- `-lcurl` - Link with the cURL library

### Run

```bash
./work_orders
```

---

## Understanding the Output

```json
{
  "success": true,
  "count": 150,
  "data": {
    "Items": [...]
  },
}
```

- `success` - Whether the API call succeeded
- `count` - Number of work orders returned
- `data` - The actual API response
---

## Memory Management Notes

C++ requires manual memory management. In this code:

1. **cURL handles** - `curl_easy_cleanup()` frees the handle
2. **Header list** - `curl_slist_free_all()` frees the headers
3. **Strings** - Automatically managed (RAII pattern)

The code uses RAII (Resource Acquisition Is Initialization) where possible - local variables like `std::string` automatically clean up when they go out of scope.

I hate dealing with this, my main choice is Go for API calls. I dont really care about trash collectors and willing to take the small performance hit instead of worrying about memory leaks.
---

## Concurrency in C++

Concurrency means doing multiple things at the same time. For API calls, this is powerful because you can fetch data from multiple endpoints simultaneously instead of waiting for each one to finish. On some of the functions we need to tie in multiple APIs, like shipment items, work orders and projects and doing it concurrently cute the duration to a fraction than looping it. 

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

### C++ Concurrency Options

#### 1. std::thread - Basic Threads

A thread is like a separate worker that can run code independently.

```cpp
#include <thread>
#include <vector>

void fetchData(int id) {
    // This runs in its own thread
    std::cout << "Thread " << id << " starting\n";
    // ... do work ...
    std::cout << "Thread " << id << " done\n";
}

int main() {
    std::vector<std::thread> threads;

    // Create 3 threads
    for (int i = 0; i < 3; i++) {
        threads.push_back(std::thread(fetchData, i));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();  // "join" means wait for this thread
    }

    return 0;
}
```
---

#### 2. std::async - Easier Async Operations

`std::async` is simpler than raw threads and automatically returns results.

```cpp
#include <future>
#include <string>

std::string fetchWorkOrders(const std::string& apiKey) {
    // ... fetch from API ...
    return response;
}

std::string fetchProjects(const std::string& apiKey) {
    // ... fetch from API ...
    return response;
}

int main() {
    std::string apiKey = "your_key";

    // Start both requests at the same time
    auto future1 = std::async(std::launch::async, fetchWorkOrders, apiKey);
    auto future2 = std::async(std::launch::async, fetchProjects, apiKey);

    // Do other work while waiting...

    // Get results (this blocks until each is ready)
    std::string workOrders = future1.get();
    std::string projects = future2.get();

    // Both are now complete!
    return 0;
}
```

**Key concepts:**
- `std::async` - Runs a function asynchronously (in the background)
- `std::future` - A handle to get the result later
- `std::launch::async` - Forces it to run in a new thread
- `.get()` - Waits for and retrieves the result

---

#### 3. Thread Safety with std::mutex

When multiple threads access the same data, you need to protect it.

**The Problem:**
```cpp
int counter = 0;

void increment() {
    for (int i = 0; i < 1000; i++) {
        counter++;  // DANGER: Multiple threads writing at once!
    }
}
```

If two threads run `increment()` simultaneously, they might both read `counter = 5`, both add 1, and both write `6`. You lost an increment!

**The Solution - Mutex (Mutual Exclusion):**
```cpp
#include <mutex>

int counter = 0;
std::mutex counterMutex;  // A lock

void increment() {
    for (int i = 0; i < 1000; i++) {
        std::lock_guard<std::mutex> lock(counterMutex);  // Lock
        counter++;  // Safe: only one thread can be here at a time
        // Automatically unlocks when lock_guard goes out of scope
    }
}
```
---

#### 4. Practical Example: Parallel API Calls

Here's how you might fetch multiple API endpoints at once:

```cpp
#include <future>
#include <vector>
#include <string>
#include <iostream>

// Simulated API call function
std::string fetchEndpoint(const std::string& endpoint, const std::string& apiKey) {
    // In real code, this would use cURL
    std::cout << "Fetching: " << endpoint << "\n";
    // ... make HTTP request ...
    return "response from " + endpoint;
}

int main() {
    std::string apiKey = "your_key";

    // List of endpoints to fetch
    std::vector<std::string> endpoints = {
        "projectWorkOrders",
        "projects",
        "customers",
        "inventory"
    };

    // Start all requests in parallel
    std::vector<std::future<std::string>> futures;
    for (const auto& endpoint : endpoints) {
        futures.push_back(
            std::async(std::launch::async, fetchEndpoint, endpoint, apiKey)
        );
    }

    // Collect all results
    std::vector<std::string> results;
    for (auto& future : futures) {
        results.push_back(future.get());  // Wait for each result
    }

    // All 4 endpoints fetched in parallel!
    std::cout << "Fetched " << results.size() << " endpoints\n";

    return 0;
}
```

**What happens:**
1. Four async tasks are created immediately
2. All four HTTP requests start at roughly the same time
3. We wait for each result in order
4. Total time ≈ slowest single request

---

### Thread Safety with cURL

**Important:** cURL requires special handling for multi-threaded use.

```cpp
curl_global_init(CURL_GLOBAL_DEFAULT);

// Each thread needs its own cURL handle
void threadFunction() {
    CURL* curl = curl_easy_init();  // Each thread gets its own
    // ... use curl ...
    curl_easy_cleanup(curl);
}

// At program end (call once, after all threads done)
curl_global_cleanup();
```

**Rules:**
1. `curl_global_init()` - Call once before creating threads
2. `curl_global_cleanup()` - Call once after all threads finish
3. Never share a `CURL*` handle between threads
4. Each thread creates and destroys its own handle

---

### When to Use Concurrency

**Good uses:**
- Fetching multiple independent API endpoints
- Processing large datasets in chunks
- Handling multiple client requests (servers)

**Avoid when:**
- Single API call (no benefit, adds complexity)
- Operations that depend on each other's results
- Simple scripts where speed doesn't matter
---

### Compiling with Threads

On Linux, you need to link the pthread library:

```bash
g++ -std=c++17 -o program program.cpp -lcurl -pthread
```

On macOS, threads are built-in so no extra flag needed.
---

### Common Concurrency Pitfalls

1. **Race conditions** - Two threads modifying the same data
   - Fix: Use `std::mutex`

2. **Deadlocks** - Two threads waiting for each other forever
   - Fix: Always lock mutexes in the same order

3. **Forgetting to join** - Program exits before threads finish
   - Fix: Always call `.join()` or `.detach()`

4. **Sharing cURL handles** - Causes crashes
   - Fix: One handle per thread

---

### Performance Comparison

| Approach | 4 API Calls (6s each) | Complexity |
|----------|----------------------|------------|
| Sequential | 24 seconds | Simple |
| std::async | ~6 seconds | Medium |
| Thread pool | ~6 seconds | Complex |

For most API work, `std::async` provides the best balance of simplicity and performance