# Web Scraper Documentation

This is a web scraping program written in C++, designed to extract the main content of webpages and save them to a file. The program utilizes various libraries and techniques to handle HTTP requests, parse HTML, calculate MD5 hashes, and more. In this documentation, we will provide a detailed description of the program, including its features, dependencies, usage instructions, and the overall design.

## Table of Contents
- Introduction
- Features
- Dependencies
- Installation
- Usage
- Overview of the Program
- Design and Implementation
- Conclusion
- Acknowledgments
- References

## Introduction
Web scraping is a technique used to extract data from websites, usually for the purpose of analysis or data processing. This program focuses on scraping the main content of webpages, which is typically the primary text and relevant information. The program fetches the HTML content of a given URL, processes it using various techniques, and then saves the extracted content to a file.

## Features
- Fetches HTML content of webpages using libcurl
- Extracts the main content from HTML using a Python script and the readability and Beautiful Soup libraries
- Filters out short or irrelevant content based on the character count
- Calculates and checks MD5 hashes of the extracted content to avoid duplication
- Keeps track of statistics such as the number of analyzed articles and skipped articles
- Supports multi-threading for efficient scraping
- Provides printing of real-time statistics during scraping

## Dependencies
The program relies on the following dependencies:
- GNU Compiler Collection (GCC)
- libcurl
- Python
- Boost System

## Installation
To compile the program, run the following command:
```
g++ -std=c++17 main.cpp -pthread -I/usr/include/x86_64-linux-gnu/curl/ -lcurl -I/usr/include/python3.11 -lpython3.11 -lboost_system
```
Make sure you have the necessary dependencies installed before compiling.

## Usage
After compiling, you can execute the program using the generated binary file. The program requires a starting URL to begin scraping. You can modify the `startedUrl` variable in the code to specify the URL of your choice. 

Once the program starts, it will print real-time statistics in the terminal, including the number of saved articles, disk space used, total analyzed articles, and skipped articles. Press `Ctrl+C` to stop the program gracefully.

Please note that the program saves the extracted content to three different files: `Texts.txt` (containing the actual content), `MD5.txt` (storing the MD5 hashes of saved articles), and `cookie.txt` (used for storing cookies during the scraping process). Make sure the program has write permissions in the directory where these files are located.

## Overview of the Program
The program consists of several components and functionalities working together to accomplish the web scraping task. Here is an overview of the main components:

1. `Fetch`: This function utilizes libcurl library to fetch the HTML content of a given URL. It also sets various options for the HTTP request, including the user agent and timeout duration.

2. `extractMainContent`: This function takes an HTML code as input and uses a Python script to extract the main content from the HTML. It makes use of the readability and BeautifulSoup libraries to remove unnecessary tags and clean the text.

3. `Stats`: This struct keeps track of various statistics related to the scraping process, including the number of analyzed articles, saved articles, skipped articles, empty MD5s, and articles that were too short.

4. `SaveToFile`: This function saves the extracted content to the `Texts.txt` file. It calculates the MD5 hash of the content and checks if it has already been saved to avoid duplication.

5. `ExtractLinks`: This function searches for links within the HTML content. It uses regular expressions to extract absolute URLs and relative URLs. The extracted links are stored in the `links` vector.

6. `Scrape`: This recursive function performs the actual scraping process. It fetches the HTML content of a given URL, extracts the main content, saves it to a file, and then recursively continues the process for the extracted links. The recursion continues until the maximum depth is reached or a termination signal is received.

7. `UpdateAndPrintStats`: This function updates and prints the real-time statistics during the scraping process. It uses escape sequences to overwrite the previous statistics in the terminal, creating a visual representation of the scraping progress.

## Design and Implementation
The program follows a modular design, with each function responsible for a specific task. It utilizes libraries such as libcurl, Boost, Python, and the `filesystem` library to handle various tasks efficiently. The use of multithreading enables concurrent scraping of multiple URLs, improving the overall speed of the program.

The program makes use of regular expressions to extract links from the HTML content. It also utilizes the Python scripting language to leverage additional libraries for improved HTML processing. By integrating Python with C++, the program benefits from enhanced HTML content extraction capabilities through the readability and Beautiful Soup libraries.

Throughout the program, appropriate error checking and exception handling are implemented to handle potential errors gracefully and avoid crashes. The program also incorporates signal handling to gracefully terminate the scraping process when a termination signal is received.

## Conclusion
The web scraper program described in this documentation provides a powerful and efficient tool for extracting the main content of webpages. By utilizing various techniques and libraries, it is able to handle HTTP requests, parse HTML, calculate MD5 hashes, and more. The program offers several features, including real-time statistics, duplicate content checking based on MD5 hashes, and multi-threading support for optimal performance.

The program's modular design, error handling, and signal handling contribute to the overall reliability and robustness of the scraping process. With the provided usage instructions and detailed documentation, users should be able to set up and utilize the web scraper effectively for their own scraping tasks.

## Acknowledgments
We would like to acknowledge the creators of the libraries used in this program, including libcurl, Boost, readability, and Beautiful Soup. Their contributions have greatly enhanced the functionality and capabilities of this web scraper.

## References
- libcurl: [https://curl.se/libcurl/](https://curl.se/libcurl/)
- Boost C++ Libraries: [https://www.boost.org/](https://www.boost.org/)
- Python: [https://www.python.org/](https://www.python.org/)
- readability library: [https://github.com/buriy/python-readability](https://github.com/buriy/python-readability)
- Beautiful Soup: [https://www.crummy.com/software/BeautifulSoup/](https://www.crummy.com/software/BeautifulSoup/)
