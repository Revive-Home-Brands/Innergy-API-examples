# Go API Fetch Example

Here is my Go example to read the Work Orders API. I prefer Go, most of the api functions we use is in Go, especially when it comes to concurrency. Go has the easiest concurrency ability imo. 

## Why Use Go for API Calls?

### Benefits

1. **Simplicity** - Go has a clean, minimal syntax. The code is readable and easy to maintain. It compiles to a single binary with no dependencies.

2. **Built-in HTTP** - Go's standard library includes a full HTTP client. No external libraries needed for API calls - `net/http` handles everything.

3. **Fast Compilation** - Go compiles incredibly fast. Large projects build in seconds, making the edit-compile-test cycle quick.

4. **Garbage Collection** - Memory is automatically managed. No need to manually free memory or worry about leaks. The GC is highly optimized and has minimal pause times.

5. **Concurrency Built-in** - Goroutines and channels are part of the language. Making concurrent API calls is trivial compared to other languages.

6. **Cross-Compilation** - Build for any OS from any OS. One command to build for Linux, Windows, or macOS.

### When to Choose Go

- Building microservices or backend APIs
- CLI tools that need to be distributed as single binaries
- Applications that need to make many concurrent HTTP requests
- Projects where development speed matters as much as runtime speed

### Trade-offs

- **Less control than C++** - You can't fine-tune memory allocation
- **Larger binaries** - Go binaries include the runtime
- **Garbage collection** - While optimized, GC can cause brief pauses in latency-sensitive apps

Like I said, this is my preferred language for API work. The simplicity of the HTTP client combined with goroutines for concurrency makes it ideal.

---

## Code Walkthrough

### 1. Imports

```go
import (
    "bufio"        // For reading files line by line
    "encoding/json" // For JSON encoding/decoding
    "flag"         // For command line argument parsing
    "fmt"          // For formatted I/O (like printf)
    "io"           // For I/O utilities
    "net/http"     // For making HTTP requests
    "os"           // For file operations
    "path/filepath" // For path manipulation
    "strings"      // For string operations
    "time"         // For timeouts and durations
)
```

**Note:** All imports are from Go's standard library. No external dependencies needed.

---

### 2. Struct Definitions

```go
type Person struct {
    Id       string `json:"Id"`
    FullName string `json:"FullName"`
}

type WorkOrder struct {
    Id     string `json:"Id"`
    Number string `json:"Number"`
    Name   string `json:"Name"`
    Status string `json:"Status"`
}

type APIResponse struct {
    Items []WorkOrder `json:"Items"`
}
```

**What this does:**
- Defines Go structs that mirror the JSON structure from the API
- The backtick tags (`` `json:"Id"` ``) tell Go how to map JSON fields to struct fields
- Go's `encoding/json` package automatically handles the conversion

**Why structs matter:**
- Type safety - the compiler catches typos and type mismatches
- IDE autocomplete works with struct fields
- Documentation is built into the code
- Pay attention to the structs in the API documentation, what sounds like ints are sometimes strings, date formats aren't always consistance. Structs are very important in Go.

---

### 3. Loading the .env File

```go
func loadEnvFile(filepath string) (map[string]string, error) {
    env := make(map[string]string)
    file, err := os.Open(filepath)
    if err != nil {
        return nil, err
    }
    defer file.Close()

    scanner := bufio.NewScanner(file)
    for scanner.Scan() {
        line := strings.TrimSpace(scanner.Text())
        if line == "" || strings.HasPrefix(line, "#") {
            continue
        }
        parts := strings.SplitN(line, "=", 2)
        if len(parts) != 2 {
            continue
        }
        key := strings.TrimSpace(parts[0])
        value := strings.TrimSpace(parts[1])
        value = strings.Trim(value, `"'`)
        env[key] = value
    }
    return env, scanner.Err()
}
```
- Opens a `.env` file (which contains `API_KEY=your_key_here`)
- Reads it line by line using a scanner
- Skips empty lines and comments (lines starting with `#`)
- Splits each line at the `=` sign
- Stores key-value pairs in a map

---

### 4. Making the API Request

```go
func fetchWorkOrders(apiKey string) ([]byte, error) {
    client := &http.Client{Timeout: 120 * time.Second}

    req, err := http.NewRequest("GET", "https://app.innergy.com/api/projectWorkOrders", nil)
    if err != nil {
        return nil, err
    }

    req.Header.Set("Accept", "application/json")
    req.Header.Set("Api-Key", apiKey)

    resp, err := client.Do(req)
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()

    if resp.StatusCode < 200 || resp.StatusCode >= 300 {
        return nil, fmt.Errorf("API returned status %d", resp.StatusCode)
    }

    return io.ReadAll(resp.Body)
}
```

1. Creates an HTTP client with a 2-minute timeout
2. Builds a GET request to the API endpoint
3. Sets the required headers
4. Executes the request
5. Checks for HTTP errors
6. Returns the response body as bytes

**Compare to C++:** This is ~15 lines vs ~50 lines with cURL. No callbacks, no manual memory management, no cleanup code needed.

---

### 5. The Main Function

```go
func main() {
    envPath := flag.String("env-path", "../.env", "Path to .env file")
    flag.Parse()

    normalizedEnvPath := filepath.Clean(*envPath)

    env, err := loadEnvFile(normalizedEnvPath)
    if err != nil {
        // Output error as JSON and return
    }

    apiKey := env["API_KEY"]
    if apiKey == "" {
        // Output error as JSON and return
    }

    data, err := fetchWorkOrders(apiKey)
    if err != nil {
        // Output error as JSON and return
    }

    var apiResponse APIResponse
    if err := json.Unmarshal(data, &apiResponse); err != nil {
        // Output error as JSON and return
    }

    response := Response{
        Success:    true,
        WorkOrders: apiResponse.Items,
        Count:      len(apiResponse.Items),
    }

    output, _ := json.MarshalIndent(response, "", "  ")
    fmt.Println(string(output))
}
```

1. Parses command line flags for the .env path
2. Loads environment variables from the .env file
3. Retrieves the API_KEY
4. Fetches work orders from the API
5. Unmarshals the JSON into Go structs
6. Outputs the result as formatted JSON

**Error handling pattern:** Each step checks for errors and outputs a JSON error response if something fails. This makes the program's output consistent and machine-readable.

---

## Building and Running

### Prerequisites

**Install Go:**

macOS (with Homebrew):
```bash
brew install go
```

Ubuntu/Debian:
```bash
sudo apt-get install golang
```

Or download from https://go.dev/dl/

### Build

```bash
go build -o work_orders work_orders.go
```

**What the command does:**
- `go build` - Compiles the program
- `-o work_orders` - Output filename
- `work_orders.go` - Input source file

### Run

```bash
./work_orders -env-path=../.env
```

### Cross-Compile for Other Platforms

Build for Linux from macOS: (I develop on a mac)
```bash
GOOS=linux GOARCH=amd64 go build -o work_orders_linux work_orders.go
```

Build for Windows from macOS:
```bash
GOOS=windows GOARCH=amd64 go build -o work_orders.exe work_orders.go
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
  "message": "Failed to load .env file: open .env: no such file or directory"
}
```

---

## Concurrency in Go

Concurrency in Go is built into the language through goroutines and channels. This makes parallel API calls trivial.

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

### Goroutines - Lightweight Threads

A goroutine is a lightweight thread managed by Go. Starting one is as simple as adding `go` before a function call.

```go
func fetchData(id int) {
    fmt.Printf("Fetching %d\n", id)
    // ... do work ...
}

func main() {
    // Run 3 fetches concurrently
    go fetchData(1)
    go fetchData(2)
    go fetchData(3)

    // Wait a bit (we'll see better ways below)
    time.Sleep(time.Second)
}
```

**Key differences from C++ threads:**
- Goroutines are extremely cheap
- You can easily run thousands of goroutines
- The Go scheduler handles mapping goroutines to OS threads

---

### WaitGroups - Waiting for Goroutines

```go
import "sync"

func main() {
    var wg sync.WaitGroup

    for i := 0; i < 3; i++ {
        wg.Add(1)  // Increment counter
        go func(id int) {
            defer wg.Done()  // Decrement counter when done
            fetchData(id)
        }(i)
    }

    wg.Wait()  // Block until counter is 0
    fmt.Println("All done!")
}
```

**What happens:**
1. `wg.Add(1)` - Tell WaitGroup we're starting a goroutine
2. `defer wg.Done()` - Mark as complete when function exits
3. `wg.Wait()` - Block until all goroutines finish

---

### Channels - Communication Between Goroutines

Channels are Go's way of safely passing data between goroutines.

```go
func fetchEndpoint(endpoint string, results chan<- string) {
    // Simulate API call
    response := "data from " + endpoint
    results <- response  // Send result to channel
}

func main() {
    results := make(chan string, 3)  // Buffered channel

    endpoints := []string{"workOrders", "projects", "customers"}

    for _, endpoint := range endpoints {
        go fetchEndpoint(endpoint, results)
    }

    // Collect results
    for i := 0; i < len(endpoints); i++ {
        result := <-results  // Receive from channel
        fmt.Println(result)
    }
}
```

**Key concepts:**
- `make(chan string, 3)` - Create a channel that holds strings, buffer size 3
- `results <- response` - Send a value into the channel
- `result := <-results` - Receive a value from the channel

---

### Practical Example: Parallel API Calls

Here's how you'd fetch multiple API endpoints concurrently:

```go
package main

import (
    "fmt"
    "io"
    "net/http"
    "sync"
    "time"
)

type Result struct {
    Endpoint string
    Data     []byte
    Error    error
}

func fetchEndpoint(endpoint, apiKey string, results chan<- Result, wg *sync.WaitGroup) {
    defer wg.Done()

    client := &http.Client{Timeout: 120 * time.Second}
    url := "https://app.innergy.com/api/" + endpoint

    req, err := http.NewRequest("GET", url, nil)
    if err != nil {
        results <- Result{Endpoint: endpoint, Error: err}
        return
    }

    req.Header.Set("Accept", "application/json")
    req.Header.Set("Api-Key", apiKey)

    resp, err := client.Do(req)
    if err != nil {
        results <- Result{Endpoint: endpoint, Error: err}
        return
    }
    defer resp.Body.Close()

    data, err := io.ReadAll(resp.Body)
    results <- Result{Endpoint: endpoint, Data: data, Error: err}
}

func main() {
    apiKey := "your_key"
    endpoints := []string{"projectWorkOrders", "projects", "customers"}

    results := make(chan Result, len(endpoints))
    var wg sync.WaitGroup

    // Start all requests concurrently
    for _, endpoint := range endpoints {
        wg.Add(1)
        go fetchEndpoint(endpoint, apiKey, results, &wg)
    }

    // Close channel when all goroutines complete
    go func() {
        wg.Wait()
        close(results)
    }()

    // Collect results as they arrive
    for result := range results {
        if result.Error != nil {
            fmt.Printf("%s: error - %v\n", result.Endpoint, result.Error)
        } else {
            fmt.Printf("%s: received %d bytes\n", result.Endpoint, len(result.Data))
        }
    }
}
```

**What happens:**
1. Three goroutines start immediately, one for each endpoint
2. All HTTP requests happen in parallel
3. Results arrive on the channel as each completes
4. Total time ≈ slowest single request

---

### Error Handling with errgroup

For more sophisticated error handling, use `golang.org/x/sync/errgroup`:

```go
import "golang.org/x/sync/errgroup"

func main() {
    var g errgroup.Group

    endpoints := []string{"workOrders", "projects", "customers"}
    results := make([][]byte, len(endpoints))

    for i, endpoint := range endpoints {
        i, endpoint := i, endpoint  // Capture variables
        g.Go(func() error {
            data, err := fetchEndpoint(endpoint)
            if err != nil {
                return err
            }
            results[i] = data
            return nil
        })
    }

    if err := g.Wait(); err != nil {
        fmt.Printf("Error: %v\n", err)
        return
    }

    // All results are now in the results slice
}
```

**Benefits:**
- Automatically waits for all goroutines
- Returns the first error encountered
- Cleaner code than manual WaitGroup + error handling

---

### Concurrency Safety

Go makes concurrency safer than C++, but you still need to be careful.

**Safe - Using Channels:**
```go
results := make(chan int, 10)
for i := 0; i < 10; i++ {
    go func(n int) {
        results <- n * 2
    }(i)
}
```

**Unsafe - Shared Variable:**
```go
counter := 0
for i := 0; i < 10; i++ {
    go func() {
        counter++
    }()
}
```

**Safe - Using Mutex:**
```go
var mu sync.Mutex
counter := 0
for i := 0; i < 10; i++ {
    go func() {
        mu.Lock()
        counter++
        mu.Unlock()
    }()
}
```

**Go's race detector:**
```bash
go run -race work_orders.go
```
This will detect race conditions at runtime - use it during development!

---

### Performance Comparison

| Approach | 4 API Calls (6s each) | Code Complexity |
|----------|----------------------|-----------------|
| Sequential | 24 seconds | Trivial |
| Goroutines | ~6 seconds | Simple |
| Goroutines + errgroup | ~6 seconds | Clean |

Go's concurrency is simpler than C++ while achieving similar performance. The goroutine model maps well to I/O-bound tasks like API calls.
