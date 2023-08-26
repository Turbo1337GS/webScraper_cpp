
#include <iostream>
#include <string>
#include <curl.h>
#include <vector>
#include <regex>
#include <fstream>
#include <Python.h>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <unistd.h>
#include <csignal>
#include <stdexcept>
#include <thread>
#include <codecvt>
#include <locale>
#include <unordered_set>
#include <queue>
#include <filesystem>
#include <mutex>
#include <future>
#include <limits>


const int MAX_DEPTH = 50;
const size_t Minimum_Characters_To_save = 50;
const size_t TIME_OUT = 5L;
const long Max_Links = 10000; //std::numeric_limits<long>::max(); 
std::string UserAgent = "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0";
std::string startedUrl = "https://www.wikimedia.org/";


volatile std::sig_atomic_t signal_received = 0;
std::vector<std::string> links;
std::unordered_set<std::string> MD5Set;
std::mutex mtx;

struct Stats {
    size_t AnalyzedAt = 0;
    size_t SavedArt = 0;
    size_t Tooshort = 0;
    size_t Alreadydone = 0;
    size_t EmptyMD_5 = 0;
};


std::string computeMD5(const std::string &input)
{
    static boost::uuids::detail::md5 hash;
    boost::uuids::detail::md5::digest_type digest;

    hash.process_bytes(input.data(), input.length());
    hash.get_digest(digest);

    static std::ostringstream oss; 
    oss.str(""); 
    oss.clear(); 

    oss << std::hex;
    for (auto &byte : digest)
    {
        oss << std::setw(2) << std::setfill('0') << (int)byte;
    }
    return oss.str();
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string Fetch(const std::string &url)
{
    if (url.empty())
        return "";


    //printf("\rFetching %s\n",url.c_str());
    static CURL *curl = nullptr;
    CURLcode res;
    std::string readBuffer;

    if (!curl)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if (curl)
        {
            std::string UserAgentCurl = "User-Agent: " + UserAgent;
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, UserAgentCurl.c_str());
            headers = curl_slist_append(headers, "Accept: text/html");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIME_OUT);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        }
    }

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookie.txt");

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            return "";
        }
    }

    return readBuffer;
}

void handleSignal(int signal)
{
    signal_received = 1;
}

void loadMD5FromFile(const std::string &filename)
{
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line))
        MD5Set.insert(line);
}

bool isMD5Present(const std::string &MD5)
{
    return MD5Set.find(MD5) != MD5Set.end();
}

void addMD5(const std::string &MD5, const std::string &filename)
{
    std::ofstream file(filename, std::ios::app);
    if (file.is_open())
    {
        file << MD5 << std::endl;
        MD5Set.insert(MD5);
    }
}

std::string getFileSizeInMB()
{
    const std::string filename = "Texts.txt";
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        printf("Couldn't open file %s\n", filename.c_str());
        return "";
    }
    std::streamsize fileSize = file.tellg();
    file.close();

    double fileSizeInMB = static_cast<double>(fileSize) / (1024 * 1024);
    std::stringstream ss;
    ss << fileSizeInMB;

    return ss.str() + " MB";
}

Stats stats;

void UpdateAndPrintStats() {
    printf("\033[5A\r");
    for (int i = 0; i < 5; ++i) {
        printf("\r\033[K");
        switch (i) {
            case 0:
                printf("Articles saved: %ld / %ld", stats.SavedArt, links.size());
                break;
            case 1:
                printf("Disk space used: %s", getFileSizeInMB().c_str());
                break;
            case 2:
                printf("Total analyzed: %ld", stats.AnalyzedAt);
                break;
            case 3:
                printf("Skipped due to short length: %ld", stats.Tooshort);
                break;
            case 4:
                printf("Skipped as already processed: %ld", stats.Alreadydone - stats.EmptyMD_5);
                break;
        }
        printf("\n");
    }
    std::fflush(stdout);
}

bool SaveToFile(const std::string &CleanHtml) {
    stats.AnalyzedAt++;
    std::string MD5 = computeMD5(CleanHtml);
    std::ofstream MD5F("MD5.txt", std::ios::app);

    loadMD5FromFile("MD5.txt");
    if (MD5.empty()) {
        stats.EmptyMD_5++;
        return false;
    }

    if (isMD5Present(MD5)) {
        stats.Alreadydone++;
        return false;
    }

    if (CleanHtml.size() < Minimum_Characters_To_save) {
        stats.Tooshort++;
        return false;
    }

    addMD5(MD5.c_str(), "MD5.txt");
    stats.SavedArt++;

    UpdateAndPrintStats();

    std::ofstream file("Texts.txt", std::ios::app);
    if (!file) {
        perror("\nError opening Texts.txt\n");
        return false;
    }
    file << CleanHtml << "\n\n";
    return true;
}

bool MakeFile(const char *fileName)
{
    if (std::string(fileName).empty())
    {
        printf("Error: Please enter name file in fuction MakeFile()\n");
        return false;
    }

    std::ofstream file(fileName, std::ios::app);

    if (!file.is_open())
    {
        return false;
    }

    file.close();
    return true;
}
bool isLinkPresent(const std::vector<std::string> &links, const std::string &link)
{
    return std::find(links.begin(), links.end(), link) != links.end();
}
std::string extractMainContent(const std::string &html_code)
{
    if (html_code.empty())
        return html_code;

    static const char *PythonCode = R"(
import re
from readability import Document
from bs4 import BeautifulSoup

def get_main_content_enhanced(html_content):
    try:
        decoded_html = html_content.decode('utf-8', 'ignore')
        doc = Document(decoded_html)
        
        # Get summary HTML based on readability algorithm
        readable_html = doc.summary()
        
        # Create a BeautifulSoup object and specify the parser
        soup = BeautifulSoup(readable_html, 'html.parser')
        
        # Remove unnecessary tags like script, style, etc.
        for script_or_style in soup(['script', 'style']):
            script_or_style.extract()
        
        # Get text and replace multiple spaces with single space
        text = soup.get_text()
        clean_text = re.sub(r'\s+', ' ', text).strip()
        
        # Return the cleaned main content
        return clean_text
    except Exception as e:
        return ''
    )";

    if (PyRun_SimpleString(PythonCode) != 0)
    {
        return "";
    }

    PyObject *pModule = PyImport_AddModule("__main__");
    if (pModule == nullptr)
    {
        return "";
    }

    PyObject *pFunc = PyObject_GetAttrString(pModule, "get_main_content_enhanced");
    if (pFunc == nullptr || !PyCallable_Check(pFunc))
    {
        return "";
    }

    PyObject *pValue = PyBytes_FromString(html_code.c_str());
    if (pValue == nullptr)
    {
        return "";
    }

    PyObject *pArgs = PyTuple_Pack(1, pValue);
    if (pArgs == nullptr)
    {
        return "";
    }

    PyObject *pResult = PyObject_CallObject(pFunc, pArgs);
    std::string result = "";
    if (pResult != nullptr)
    {
        result = PyUnicode_AsUTF8(pResult);
        Py_DECREF(pResult);
    }

    Py_DECREF(pArgs);
    Py_DECREF(pValue);
    Py_DECREF(pFunc);

    return result;
}

std::vector<std::string> ExtractLinks(const std::string &html)
{
    if (html.empty())
        return links;

    std::unordered_set<std::string> uniqueLinks;

    std::regex baseRegex(R"(<base\s+href=\"(https?:\/\/[^\s\"]+)\")");
    std::smatch baseMatch;
    std::string baseUrl;

    if (std::regex_search(html, baseMatch, baseRegex))
        baseUrl = baseMatch[1].str();

    std::regex linkRegex(R"(href=\"(https?:\/\/[^\s\"]+|\/[^\s\"]+)\")");
    std::smatch linkMatch;

    std::string::const_iterator searchStart(html.cbegin());
    while (std::regex_search(searchStart, html.cend(), linkMatch, linkRegex))
    {
        std::string link = linkMatch[1].str();

        if (baseUrl.empty() && link.find("http") == 0)
        {
            baseUrl = link.substr(0, link.find("/", 8));
        }

        if (!link.empty() && link[0] == '/')
        {
            link = baseUrl + link;
        }

        if (link.find("http") == 0 && uniqueLinks.find(link) == uniqueLinks.end() && !link.empty() && !isLinkPresent(links, link))
        {
            if (links.size() >= Max_Links)
            {
                break;
            }
            links.push_back(link);
            uniqueLinks.insert(link);
        }

        searchStart = linkMatch.suffix().first;
    }

    return links;
}

void Scrape(const std::string& url, int depth = 0) {
    if(url.empty()) 
        return;

    if (depth > MAX_DEPTH )
        return;

    if (signal_received)
        return;


    std::string html = Fetch(url);
    std::string Main = extractMainContent(html.c_str());

    SaveToFile(Main.c_str());

    std::vector<std::string> newLinks = ExtractLinks(html); 
    for (const auto& link : newLinks) {
        Scrape(link, depth + 1);
    }
}

int main()
{

    system("clear");
    for (size_t i = 0; i < 5; i++)
    {
        printf("\n");
    }
    
    std::signal(SIGINT, handleSignal);
    MakeFile("cookie.txt");
    MakeFile("MD5.txt");
    MakeFile("Texts.txt");

    Py_Initialize();

    if (startedUrl.empty())
    {
        printf("Please enter started url\n");
        return -1;
    }

    std::string FirstHTML = Fetch(startedUrl);


    ExtractLinks(FirstHTML);
    Scrape(startedUrl);
    Py_Finalize();

    return 0;
}


