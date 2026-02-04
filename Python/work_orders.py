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
        9. Removes surrounding quotes (single or double) from values
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
