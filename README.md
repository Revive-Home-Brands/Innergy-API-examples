# Innergy API Examples

I made some code examples for people who want to learn about APIs and using Innergy's APIs.
After running the scripts, here are the results of the speed:

```
Go: 6187.55ms
PHP: 6430.12ms
C++: 5996.39ms
Python: 6472.56ms
```

Adding concurrency is where youll see the speed differences. On one of my web pages, I have 5 APIs fetching concurrently in 
Go and the page takes about 7 seconds total to load.

## Overview

These examples demonstrate how to authenticate and fetch work orders from `https://app.innergy.com/api/projectWorkOrders`. Each implementation follows the same pattern: load an API key from a `.env` file, make an authenticated GET request and output the results as JSON.

**Note:** The Work Orders API returns project/job data, but many real-world integrations require combining data from multiple Innergy API endpoints concurrently. Choose your language based on how you plan to use these calls.

## Setup

1. Copy the environment template:
   ```
   cp .env-template .env
   ```

2. Add your Innergy API key to `.env`:
   ```
   API_KEY=your_api_key_here
   ```

## Language Examples

### PHP
**Best for:** Laravel applications, traditional web backends, Wordpress and quick scripts

```
php PHP/work_orders.php
```

PHP integrates naturally with Laravel's HTTP client and job queues. If you're already running a Laravel app, use this pattern and swap cURL for `Http::get()`.

### Go
**Best for:** High concurrency, parallel API calls and microservices

```
cd GoLang && go build -o work_orders && ./work_orders
```

Go's goroutines make it super easy when you need to fetch from multiple API endpoints simultaneously.

### C++
**Best for:** Real-time systems and low-latency requirements

```
cd C++ && g++ -std=c++17 -o work_orders work_orders.cpp -lcurl && ./work_orders
```

Use C++ when you need minimal overhead and predictable performance, such as real-time dashboards or systems with strict latency requirements.

### Python
**Best for:** Jupyter notebooks, data analysis, prototyping and scripting

```
pip install requests
python Python/work_orders.py
```

Python is ideal for exploratory data analysis in Jupyter. Fetch work orders, transform with pandas and visualize—all in one notebook.

## Working with Multiple APIs

The Work Orders endpoint is often just the starting point. Real integrations typically need data from several endpoints:

- Work orders + Shipment Items
- Work orders + Projects
- Work orders + Companies
- Work orders + Impediments
- Projects + Work orders + Shipment Items + Impediments

**Go** handles this well with goroutines for parallel fetches. **PHP** can use Laravel's `Http::pool()` or queue jobs. **Python** works with `asyncio` or `concurrent.futures`. **C++** can use threading or async I/O libraries.

Choose based on your existing stack and concurrency needs.

## Response Format

All examples output JSON in this structure:

```json
{
  "success": true,
  "workOrders": [...],
  "count": 42
}
```

On error:

```json
{
  "success": false,
  "message": "Error description"
}
```