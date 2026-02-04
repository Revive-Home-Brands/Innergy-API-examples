# Python API Fetch Example

Here is my Python example to read the Work Orders API. This script is designed as a standalone CLI tool for fetching work order data.

## Why Use Python for API Calls?

### Benefits

1. **Simplicity** - Python's clean syntax makes it easy to read and write. You can focus on logic rather than boilerplate code.

2. **Rich Standard Library** - Built-in modules like `json`, `pathlib` and `argparse` mean less dependency management for basic tasks.

3. **Requests Library** - The `requests` library is the gold standard for HTTP in Python.

4. **Type Hints** - Modern Python supports type annotations, making code more maintainable and catching errors earlier with tools like mypy.

5. **Cross-Platform** - Runs on Windows, macOS and Linux without modification. Python is often pre-installed on Unix systems.

6. **Data Science Ready** - If you need to analyze the API data, Python integrates seamlessly with pandas, numpy, Jupyter and other data tools.

### When to Choose Python

- Building CLI tools for data fetching
- Prototyping and exploratory work
- When you need to analyze or transform API data
- Data pipelines and ETL processes
- Cross-platform automation scripts
- Integration with data science workflows

### Trade-offs

- **Performance** - Slower than compiled languages like Go or Rust for CPU-intensive tasks
- **Dependencies** - The `requests` library isn't in the standard library
- **Deployment** - Requires Python runtime on target systems
- **GIL** - Global Interpreter Lock limits true parallelism for CPU-bound tasks

Python excels at readable, maintainable code for I/O-bound tasks.

---

## Code Walkthrough

### 1. Script Header and Dependencies

```python
#!/usr/bin/env python3
"""
Work Orders API Fetch Example

Dependencies:
    pip install requests

Run:
    python Python/work_orders.py
    python Python/work_orders.py --env-path=/path/to/.env
"""

import argparse
import json
import sys
from pathlib import Path

try:
    import requests
except ImportError:
    print(json.dumps({
        "success": False,
        "message": "requests library not installed. Run: pip install requests"
    }, indent=2))
    sys.exit(1)
```

**What this does:**
- `#!/usr/bin/env python3` - Shebang for Unix systems to run as executable
- Docstring provides quick reference for usage
- Imports from standard library
- Try/except for `requests` import with helpful error message
- Exits gracefully if dependency is missing

**Python-specific notes:**
- `pathlib.Path` is the modern way to handle file paths (better than `os.path`)
- Early dependency check prevents confusing errors later
- Using `sys.exit(1)` signals failure to the shell

---

### 2. Loading the .env File

```python
def load_env_file(filepath: str) -> dict:
    """
    load_env_file - Reads a .env file and returns a dictionary.

    How it works:
        1. Creates a Path object from the filepath string
        2. Checks if the file exists, raises FileNotFoundError if not
        3. Opens the file and reads it line by line
        4. Strips whitespace from each line
        5. Skips empty lines and lines starting with #
        6. Skips lines that don't contain an = sign
        7. Splits each line at the first = into key and value
        8. Strips whitespace from both key and value
        9. Removes surrounding quotes from values
        10. Stores the pair in a dictionary and returns it

    Args:
        filepath: Path to the .env file

    Returns:
        dict: Key-value pairs from the file

    Raises:
        FileNotFoundError: If the file does not exist
    """
    env = {}
    path = Path(filepath)

    if not path.exists():
        raise FileNotFoundError(f"Environment file not found: {filepath}")

    with open(path, 'r') as f:
        for line in f:
            line = line.strip()

            if not line or line.startswith('#'):
                continue

            if '=' not in line:
                continue

            key, value = line.split('=', 1)
            key = key.strip()
            value = value.strip()

            if (value.startswith('"') and value.endswith('"')) or \
               (value.startswith("'") and value.endswith("'")):
                value = value[1:-1]

            env[key] = value

    return env
```

**Step-by-step breakdown:**

1. **Path validation** - Uses `pathlib.Path` for cross-platform compatibility
2. **File reading** - Context manager (`with`) ensures file is closed properly
3. **Line processing** - Strips whitespace, skips comments and empty lines
4. **Parsing** - Splits on first `=` only (allows `=` in values)
5. **Quote removal** - Handles both single and double quotes

**Python patterns:**
- `line.strip()` - removes leading/trailing whitespace
- `line.startswith('#')` - cleaner than string slicing
- `split('=', 1)` - splits only on first occurrence
- `value[1:-1]` - string slicing to remove first and last characters
- Type hints (`filepath: str -> dict`) for better IDE support

---

### 3. Making the API Request

```python
def fetch_work_orders(api_key: str) -> dict:
    """
    fetch_work_orders - Makes an HTTP GET request to the Innergy API.

    How it works:
        1. Sets the URL to the projectWorkOrders endpoint
        2. Creates a headers dictionary with:
           - Accept: application/json
           - Api-Key: the authentication key
        3. Makes a GET request using the requests library
        4. Sets a 120 second timeout for large responses
        5. Calls raise_for_status() to throw on non-2xx responses
        6. Parses the JSON response and returns it as a dictionary

    Args:
        api_key: The API key for authentication

    Returns:
        dict: Parsed JSON response from the API

    Raises:
        requests.exceptions.HTTPError: On non-2xx HTTP status
        requests.exceptions.RequestException: On network errors
    """
    url = "https://app.innergy.com/api/projectWorkOrders"

    headers = {
        "Accept": "application/json",
        "Api-Key": api_key
    }

    response = requests.get(url, headers=headers, timeout=120)
    response.raise_for_status()

    return response.json()
```

**What this does:**

1. **URL setup** - Hardcoded API endpoint
2. **Headers** - Dictionary with Accept and Api-Key headers
3. **GET request** - `requests.get()` with headers and timeout
4. **Error handling** - `raise_for_status()` throws exception on 4xx/5xx
5. **JSON parsing** - `.json()` method automatically parses response

**Requests library advantages:**
- Automatically handles connection pooling
- Supports sessions for multiple requests
- Built-in JSON encoding/decoding
- Cleaner than urllib from standard library

---

### 4. The Main Function

```python
def main():
    """
    main - Entry point of the program.

    How it works:
        1. Creates an argument parser with --env-path option
        2. Sets default path to the .env file location
        3. Parses command line arguments
        4. Loads environment variables from the .env file
        5. Gets the API_KEY from the environment dictionary
        6. Raises ValueError if API_KEY is missing
        7. Calls fetch_work_orders to get data from the API
        8. Extracts the Items array from the response
        9. Creates a result dictionary with success, workOrders, and count
        10. Outputs the result as pretty-printed JSON
        11. Catches specific request exceptions for better error messages
        12. Catches all other exceptions and outputs error JSON
    """
    parser = argparse.ArgumentParser(description="Fetch work orders from Innergy API")
    parser.add_argument("--env-path", default="../.env", help="Path to .env file")
    args = parser.parse_args()

    try:
        env = load_env_file(args.env_path)

        api_key = env.get("API_KEY")
        if not api_key:
            raise ValueError("API_KEY not found in .env file")

        api_response = fetch_work_orders(api_key)

        work_orders = api_response.get("Items", [])

        result = {
            "success": True,
            "workOrders": work_orders,
            "count": len(work_orders)
        }
        print(json.dumps(result, indent=2))

    except requests.exceptions.HTTPError as e:
        result = {
            "success": False,
            "message": f"API error: {e.response.status_code}"
        }
        print(json.dumps(result, indent=2))

    except requests.exceptions.RequestException as e:
        result = {
            "success": False,
            "message": f"Request error: {str(e)}"
        }
        print(json.dumps(result, indent=2))

    except Exception as e:
        result = {
            "success": False,
            "message": str(e)
        }
        print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
```

**Component breakdown:**

**Argument parsing:**
- `argparse.ArgumentParser` - built-in CLI argument handler
- `--env-path` - optional flag with default value
- `args.env_path` - access parsed arguments as attributes

**Error handling hierarchy:**
1. `requests.exceptions.HTTPError` - API returned error status
2. `requests.exceptions.RequestException` - Network/connection errors
3. `Exception` - Catch-all for unexpected errors

**Dictionary methods:**
- `env.get("API_KEY")` - returns None if key doesn't exist (safe)
- `api_response.get("Items", [])` - returns empty list as default

**Python idiom:**
```python
if __name__ == "__main__":
    main()
```
This allows the file to be imported without running main(), useful for testing.

---

## Running

### Prerequisites

**Check Python is installed:**
```bash
python3 --version
```

Should show Python 3.6 or higher (3.8+ recommended for best type hint support).

**Install dependencies:**
```bash
pip install requests
```

Or with a virtual environment:
```bash
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate
pip install requests
```

### Run from Command Line

**Basic usage:**
```bash
python3 work_orders.py
```

**With custom .env path:**
```bash
python3 work_orders.py --env-path=/path/to/.env
```

**Make executable (Unix):**
```bash
chmod +x work_orders.py
./work_orders.py
```

---

## Understanding the Output

**Success response:**
```json
{
  "success": true,
  "workOrders": [
    {
      "id": "WO-001",
      "projectName": "Solar Installation",
      "status": "In Progress"
    }
  ],
  "count": 150
}
```

- `success` - Boolean indicating if the API call succeeded
- `workOrders` - Array of work order objects from the API
- `count` - Total number of work orders returned

**Error responses:**

Missing API key:
```json
{
  "success": false,
  "message": "API_KEY not found in .env file"
}
```

API error:
```json
{
  "success": false,
  "message": "API error: 401"
}
```

Network error:
```json
{
  "success": false,
  "message": "Request error: Connection refused"
}
```

---

## Concurrency in Python

Python has several approaches to parallel API calls, each with different trade-offs.

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

---

### Option 1: Threading - For I/O-Bound Tasks

Python's `threading` module works well for I/O-bound operations like API calls

```python
import threading
from typing import Dict, List

def fetch_endpoint(endpoint: str, api_key: str, results: Dict[str, dict]):
    """Fetch a single endpoint and store result in shared dictionary."""
    url = f"https://app.innergy.com/api/{endpoint}"
    headers = {
        "Accept": "application/json",
        "Api-Key": api_key
    }
    
    try:
        response = requests.get(url, headers=headers, timeout=120)
        response.raise_for_status()
        results[endpoint] = response.json()
    except Exception as e:
        results[endpoint] = {"error": str(e)}


def fetch_multiple_endpoints(api_key: str, endpoints: List[str]) -> Dict[str, dict]:
    """Fetch multiple endpoints in parallel using threads."""
    results = {}
    threads = []
    
    # Create and start a thread for each endpoint
    for endpoint in endpoints:
        thread = threading.Thread(
            target=fetch_endpoint,
            args=(endpoint, api_key, results)
        )
        thread.start()
        threads.append(thread)
    
    # Wait for all threads to complete
    for thread in threads:
        thread.join()
    
    return results


# Usage
endpoints = ['projectWorkOrders', 'projects', 'customers']
results = fetch_multiple_endpoints(api_key, endpoints)

for endpoint, data in results.items():
    item_count = len(data.get('Items', []))
    print(f"{endpoint}: {item_count} items")
```

**How it works:**
1. Create a thread for each endpoint
2. Each thread makes its API call independently
3. Store results in a shared dictionary
4. `join()` waits for all threads to finish

**Pros:**
- Built into Python, no extra dependencies
- Good for I/O-bound tasks
- Simple to understand and implement

**Cons:**
- GIL limits CPU-bound parallelism
- Manual thread management
- Race conditions possible with shared state

---

### Option 2: ThreadPoolExecutor - Cleaner Threading

The `concurrent.futures` module provides a higher-level threading interface.

```python
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import Dict, List

def fetch_endpoint_simple(endpoint: str, api_key: str) -> tuple:
    """Fetch endpoint and return (endpoint_name, data) tuple."""
    url = f"https://app.innergy.com/api/{endpoint}"
    headers = {
        "Accept": "application/json",
        "Api-Key": api_key
    }
    
    try:
        response = requests.get(url, headers=headers, timeout=120)
        response.raise_for_status()
        return (endpoint, response.json())
    except Exception as e:
        return (endpoint, {"error": str(e)})


def fetch_multiple_with_pool(api_key: str, endpoints: List[str]) -> Dict[str, dict]:
    """Fetch multiple endpoints using a thread pool."""
    results = {}
    
    # Create a thread pool with 5 workers
    with ThreadPoolExecutor(max_workers=5) as executor:
        # Submit all tasks
        future_to_endpoint = {
            executor.submit(fetch_endpoint_simple, endpoint, api_key): endpoint
            for endpoint in endpoints
        }
        
        # Collect results as they complete
        for future in as_completed(future_to_endpoint):
            endpoint_name, data = future.result()
            results[endpoint_name] = data
    
    return results


# Usage
endpoints = ['projectWorkOrders', 'projects', 'customers']
results = fetch_multiple_with_pool(api_key, endpoints)
```

**How it works:**
1. Creates a pool of worker threads (max_workers=5)
2. Submits tasks to the pool
3. Pool manages thread creation and recycling
4. `as_completed()` yields results as they finish

**Pros:**
- Cleaner API than raw threading
- Automatic thread pool management
- Easy to limit concurrent requests
- Built into Python 3.2+

**Cons:**
- Still subject to GIL for CPU-bound work
- Slightly more complex than basic threading

---

### Option 3: asyncio - Native Async/Await

Modern Python supports async/await with `asyncio` and `aiohttp`.

```bash
pip install aiohttp
```

```python
import asyncio
import aiohttp
from typing import Dict, List

async def fetch_endpoint_async(
    session: aiohttp.ClientSession,
    endpoint: str,
    api_key: str
) -> tuple:
    """Async fetch of a single endpoint."""
    url = f"https://app.innergy.com/api/{endpoint}"
    headers = {
        "Accept": "application/json",
        "Api-Key": api_key
    }
    
    try:
        async with session.get(url, headers=headers, timeout=120) as response:
            response.raise_for_status()
            data = await response.json()
            return (endpoint, data)
    except Exception as e:
        return (endpoint, {"error": str(e)})


async def fetch_multiple_async(api_key: str, endpoints: List[str]) -> Dict[str, dict]:
    """Fetch multiple endpoints asynchronously."""
    async with aiohttp.ClientSession() as session:
        # Create tasks for all endpoints
        tasks = [
            fetch_endpoint_async(session, endpoint, api_key)
            for endpoint in endpoints
        ]
        
        # Wait for all tasks to complete
        results_list = await asyncio.gather(*tasks)
        
        # Convert list of tuples to dictionary
        return dict(results_list)


# Usage
endpoints = ['projectWorkOrders', 'projects', 'customers']
results = asyncio.run(fetch_multiple_async(api_key, endpoints))

for endpoint, data in results.items():
    item_count = len(data.get('Items', []))
    print(f"{endpoint}: {item_count} items")
```

**How it works:**
1. `async def` creates coroutines
2. `await` pauses execution until result is ready
3. `asyncio.gather()` runs multiple coroutines concurrently
4. Single-threaded cooperative multitasking

**Pros:**
- More efficient than threading for many concurrent I/O operations
- Single-threaded
- Modern Python pattern
- Scales better

**Cons:**
- Requires async-compatible libraries
- Steeper learning curve
- Can't mix sync and async code easily
- Requires Python 3.7+ for `asyncio.run()`

---

### Performance Comparison

| Approach | 4 API Calls (6s each) | Complexity | Dependencies |
|----------|----------------------|------------|--------------|
| Sequential | 24 seconds | Simple | requests only |
| threading | ~6 seconds | Medium | stdlib |
| ThreadPoolExecutor | ~6 seconds | Medium | stdlib |
| asyncio + aiohttp | ~6 seconds | High | aiohttp |

**Recommendations:**
- **Simple scripts**: Use ThreadPoolExecutor (stdlib, clean API)
- **High concurrency** (100+ requests): Use asyncio + aiohttp
- **Mixed sync/async code**: Stick with ThreadPoolExecutor
- **Learning async**: Start with ThreadPoolExecutor, graduate to asyncio

---

## Python Version Notes

This script is compatible with Python 3.6+, but some features work better in newer versions:

**Python 3.6:**
- Type hints work (`: str`, `-> dict`)
- f-strings work (`f"API error: {code}"`)

**Python 3.7+:**
- `asyncio.run()` available
- Guaranteed dictionary ordering

**Python 3.8+:**
- Walrus operator (`:=`) available
- Better type hints (`typing.Literal`, `typing.TypedDict`)

**Python 3.9+:**
- Built-in generic types (`list[str]` instead of `List[str]`)
- Dictionary union operator (`|`)

**Python 3.10+:**
- Match/case statements
- Better error messages
- Union type hints (`str | None` instead of `Optional[str]`)

For maximum compatibility, stick with Python 3.7+. For modern features, use Python 3.10+.

---

## Best Practices

### Virtual Environments

Always use virtual environments to isolate dependencies:

```bash
# Create virtual environment
python3 -m venv venv

# Activate (Unix)
source venv/bin/activate

# Activate (Windows)
venv\Scripts\activate

# Install dependencies
pip install requests

# Save dependencies
pip freeze > requirements.txt

# Deactivate when done
deactivate
```

### Type Checking

Use `mypy` to catch type errors:

```bash
pip install mypy
mypy work_orders.py
```

### Logging

For production scripts, use `logging` instead of `print()`:

```python
import logging

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

logger = logging.getLogger(__name__)

# Instead of print()
logger.info(f"Fetched {count} work orders")
logger.error(f"API error: {status_code}")
```

### Configuration

For multiple environments, use a config file:

```python
import configparser

config = configparser.ConfigParser()
config.read('config.ini')

api_key = config['DEFAULT']['ApiKey']
timeout = config.getint('DEFAULT', 'Timeout')
```