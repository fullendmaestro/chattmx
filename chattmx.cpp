#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function for curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
        return newLength;
    }
    catch (std::bad_alloc& e) {
        return 0;
    }
}

// Function to make API request
std::string makeApiRequest(const std::string& prompt, const std::string& apiKey) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=" + apiKey;

        json requestBody;
        requestBody["contents"][0]["parts"][0]["text"] = prompt;
        std::string jsonBody = requestBody.dump();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }

    return readBuffer;
}

// Function to extract response text from API response
std::string extractResponseText(const std::string& jsonResponse) {
    try {
        json response = json::parse(jsonResponse);
        return response["candidates"][0]["content"]["parts"][0]["text"];
    }
    catch (json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return "Error parsing response";
    }
    catch (json::exception& e) {
        std::cerr << "JSON exception: " << e.what() << std::endl;
        return "Error processing response";
    }
}

int main() {
    std::string apiKey;
    std::cout << "Enter your Gemini API key: ";
    std::cin >> apiKey;
    std::cin.ignore(); // Clear the newline from the buffer

    std::cout << "Chatbot: Hello! How can I assist you today?" << std::endl;

    std::string userInput;
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, userInput);

        if (userInput == "exit" || userInput == "quit") {
            break;
        }

        std::string apiResponse = makeApiRequest(userInput, apiKey);
        std::string botResponse = extractResponseText(apiResponse);

        std::cout << "Chatbot: " << botResponse << std::endl;
    }

    std::cout << "Chatbot: Goodbye! Have a great day!" << std::endl;

    return 0;
}