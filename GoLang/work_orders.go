/*
Work Orders API Fetch Example

Build:
  go build -o bin/work_orders GoLang/work_orders.go

Run:
  ./bin/work_orders -env-path=.env
*/

package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"time"
)

// Person represents a user reference in the API response
type Person struct {
	Id       string `json:"Id"`
	FullName string `json:"FullName"`
}

// MoneyValue represents a monetary amount
type MoneyValue struct {
	Value         float64 `json:"Value"`
	OriginalValue float64 `json:"OriginalValue"`
	CurrencyCode  string  `json:"CurrencyCode"`
}

// Margin represents margin data with cash and percentage
type Margin struct {
	Cash       MoneyValue `json:"Cash"`
	Percentage float64    `json:"Percentage"`
}

// CustomField represents a custom field entry
type CustomField struct {
	Name  string `json:"Name"`
	Type  int    `json:"Type"`
	Value string `json:"Value"`
}

// Finish represents a finish option
type Finish struct {
	Id     string `json:"Id"`
	Name   string `json:"Name"`
	Code   string `json:"Code"`
	Number string `json:"Number"`
}

// WorkOrder represents a work order from the API
type WorkOrder struct {
	Id                    string        `json:"Id"`
	Number                string        `json:"Number"`
	Name                  string        `json:"Name"`
	Type                  string        `json:"Type"`
	CreatedBy             Person        `json:"CreatedBy"`
	CreatedOn             string        `json:"CreatedOn"`
	Facility              string        `json:"Facility"`
	Outsourced            bool          `json:"Outsourced"`
	Tags                  []string      `json:"Tags"`
	Status                string        `json:"Status"`
	MaterialOnHandDays    int           `json:"MaterialOnHandDays"`
	Step                  string        `json:"Step"`
	StepIndex             int           `json:"StepIndex"`
	StepType              string        `json:"StepType"`
	InvoiceStatus         string        `json:"InvoiceStatus"`
	Owner                 Person        `json:"Owner"`
	Assignees             []Person      `json:"Assignees"`
	Drafters              []Person      `json:"Drafters"`
	Engineers             []Person      `json:"Engineers"`
	Estimators            []Person      `json:"Estimators"`
	SalesPersons          []Person      `json:"SalesPersons"`
	Coordinators          []Person      `json:"Coordinators"`
	Installers            []Person      `json:"Installers"`
	ProjectManager        Person        `json:"ProjectManager"`
	PlannedStartDate      string        `json:"PlannedStartDate"`
	ActualStartDate       string        `json:"ActualStartDate"`
	PlannedCriticalDate   string        `json:"PlannedCriticalDate"`
	MaterialNeededDate    string        `json:"MaterialNeededDate"`
	PlannedEndMonth       string        `json:"PlannedEndMonth"`
	ActualEndDate         string        `json:"ActualEndDate"`
	ActualEndMonth        string        `json:"ActualEndMonth"`
	Instructions          string        `json:"Instructions"`
	EstimatedLaborCost    MoneyValue    `json:"EstimatedLaborCost"`
	EstimatedMaterialCost MoneyValue    `json:"EstimatedMaterialCost"`
	EstimatedCost         MoneyValue    `json:"EstimatedCost"`
	EstimatedHours        string        `json:"EstimatedHours"`
	EstimatedMargin       Margin        `json:"EstimatedMargin"`
	RemainingHours        string        `json:"RemainingHours"`
	PlannedHours          string        `json:"PlannedHours"`
	PlannedLaborCost      MoneyValue    `json:"PlannedLaborCost"`
	LaborGrandTotalPrice  MoneyValue    `json:"LaborGrandTotalPrice"`
	ActualLaborHours      string        `json:"ActualLaborHours"`
	ActualCost            MoneyValue    `json:"ActualCost"`
	ActualMaterialCost    MoneyValue    `json:"ActualMaterialCost"`
	ActualLaborCost       MoneyValue    `json:"ActualLaborCost"`
	ActualExpensesCost    MoneyValue    `json:"ActualExpensesCost"`
	ActualMargin          Margin        `json:"ActualMargin"`
	MarginVariance        MoneyValue    `json:"MarginVariance"`
	GrandTotalPrice       MoneyValue    `json:"GrandTotalPrice"`
	PreSalesTaxPrice      MoneyValue    `json:"PreSalesTaxPrice"`
	SalesTax              MoneyValue    `json:"SalesTax"`
	ExternalIdentifier    string        `json:"ExternalIdentifier"`
	WorkflowName          string        `json:"WorkflowName"`
	ProjectNumber         string        `json:"ProjectNumber"`
	ProjectName           string        `json:"ProjectName"`
	CustomFields          []CustomField `json:"CustomFields"`
	Finishes              []Finish      `json:"Finishes"`
}

// APIResponse represents the API response structure
type APIResponse struct {
	Items []WorkOrder `json:"Items"`
}

// Response represents the output structure
type Response struct {
	Success    bool        `json:"success"`
	WorkOrders []WorkOrder `json:"workOrders"`
	Count      int         `json:"count"`
	Message    string      `json:"message,omitempty"`
}

// loadEnvFile reads a .env file and returns a map of key-value pairs.
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

/*
fetchWorkOrders makes an HTTP GET request to the Innergy API.

 1. Creates an HTTP client with a 120 second timeout
 2. Builds a GET request to the projectWorkOrders endpoint
 3. Sets the Accept header to application/json
 4. Sets the Api-Key header for authentication
 5. Executes the request and checks the response status
 6. Returns the raw response body as bytes
*/
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

/*
main is the entry point of the program.

How it works:
 1. Parses command line flags to get the .env file path
 2. Loads environment variables from the .env file
 3. Retrieves the API_KEY from the environment
 4. Calls fetchWorkOrders to get data from the API
 5. Unmarshals the JSON response into Go structs
 6. Outputs the result as formatted JSON to stdout
 7. Handles errors at each step and outputs error JSON if needed
*/
func main() {
	envPath := flag.String("env-path", "../.env", "Path to .env file")
	flag.Parse()

	normalizedEnvPath := filepath.Clean(*envPath)

	env, err := loadEnvFile(normalizedEnvPath)
	if err != nil {
		response := Response{
			Success: false,
			Message: fmt.Sprintf("Failed to load .env file: %v", err),
		}
		output, _ := json.MarshalIndent(response, "", "  ")
		fmt.Println(string(output))
		return
	}

	apiKey := env["API_KEY"]
	if apiKey == "" {
		response := Response{
			Success: false,
			Message: "API_KEY not found in .env file",
		}
		output, _ := json.MarshalIndent(response, "", "  ")
		fmt.Println(string(output))
		return
	}

	data, err := fetchWorkOrders(apiKey)
	if err != nil {
		response := Response{
			Success: false,
			Message: fmt.Sprintf("Failed to fetch work orders: %v", err),
		}
		output, _ := json.MarshalIndent(response, "", "  ")
		fmt.Println(string(output))
		return
	}

	var apiResponse APIResponse
	if err := json.Unmarshal(data, &apiResponse); err != nil {
		response := Response{
			Success: false,
			Message: fmt.Sprintf("Failed to parse response: %v", err),
		}
		output, _ := json.MarshalIndent(response, "", "  ")
		fmt.Println(string(output))
		return
	}

	response := Response{
		Success:    true,
		WorkOrders: apiResponse.Items,
		Count:      len(apiResponse.Items),
	}

	output, _ := json.MarshalIndent(response, "", "  ")
	fmt.Println(string(output))
}
