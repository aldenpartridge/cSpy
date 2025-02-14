#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <getopt.h>
#include <sstream>
#include <fstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <filesystem>

using namespace std;
using namespace std::filesystem;
using json = nlohmann::json;

// Terminal Colors
const string Black        = "\033[30m";
const string Red          = "\033[31m";
const string Green        = "\033[32m";
const string Yellow       = "\033[33m";
const string Blue         = "\033[34m";
const string Magenta      = "\033[35m";
const string Cyan         = "\033[36m";
const string LightGray    = "\033[37m";
const string DarkGray     = "\033[90m";
const string LightRed     = "\033[91m";
const string LightGreen   = "\033[92m";
const string LightYellow  = "\033[93m";
const string LightBlue    = "\033[94m";
const string LightMagenta = "\033[95m";
const string LightCyan    = "\033[96m";
const string White        = "\033[97m";
const string Default      = "\033[0m";

// Banner string
const string banner =
    "\n" + LightGreen + "          _____ __                     ____\n"
    "         / ____/ /_  ____ _____  _____/ __ \\__  __\n"
    "        / /   / __ \\/ __ `/ __ \\/ ___/ /_/ / / / /\n"
    "       / /___/ / / / /_/ / /_/ (__  ) ____/ /_/ /\n"
    "       \\____/_/ /_/\\__,_/\\____/____/_/    \\__, /\n"
    "                                          /____/\n"
    + Yellow + "  Small tool inspired by chaos from projectdiscovery.io\n"
    + DarkGray + "          https://chaos.projectdiscovery.io/\n"
    + DarkGray + "    *Author -> Moaaz (https://x.com/zor0x0x)*\n"
    + DarkGray + "    *Ported by aldenpartridge (https://x.com/0xkmac)*\n\n" +
    White;

// Write callback for CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Perform HTTP GET and return result as string
string httpGet(const string &url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // For HTTPS
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Fetch JSON data from URL
json getdata() {
    string data = httpGet("https://chaos-data.projectdiscovery.io/index.json");
    return json::parse(data);
}

void info(const json &jdata) {
    int newCount = 0, hackerone = 0, bugcrowd = 0, intigriti = 0, external = 0, changed = 0, subdomains = 0;
    int rewards = 0, norewards = 0, swags = 0;
    for (const auto &item : jdata) {
        if (item.value("is_new", false))
            newCount++;
        if (item.value("platform", "") == "hackerone")
            hackerone++;
        if (item.value("platform", "") == "bugcrowd")
            bugcrowd++;
        if (item["program_url"].get<string>().find("intigriti") != string::npos)
            intigriti++;
        if (item.value("platform", "") == "")
            external++;
        if (item.value("change", 0) != 0)
            changed++;
        subdomains += item.value("count", 0);
        if (item.value("bounty", false))
            rewards++;
        else
            norewards++;
        if (item.find("swag") != item.end())
            swags++;
    }
    // Using last item's last_updated field for last update date.
    string lastUpdated = jdata.empty() ? "N/A" : jdata.back().value("last_updated", "N/A");
    cout << White << "[!] Programs Last Updated: " << lastUpdated.substr(0,10) << "\n";
    cout << LightGreen << "[!] Subdomains: " << subdomains << "\n";
    cout << Green << "[!] Total programs: " << jdata.size() << Default << "\n";
    cout << LightCyan << "[!] Updated programs: " << changed << Default << "\n";
    cout << Blue << "[!] New programs: " << newCount << Default << "\n";
    cout << LightGray << "[!] HackerOne programs: " << hackerone << Default << "\n";
    cout << Magenta << "[!] Intigriti programs: " << intigriti << Default << "\n";
    cout << Yellow << "[!] BugCrowd programs: " << bugcrowd << Default << "\n";
    cout << LightGreen << "[!] Self-hosted programs: " << external << Default << "\n";
    cout << Cyan << "[!] Programs with rewards: " << rewards << Default << "\n";
    cout << Yellow << "[!] Programs offering swag: " << swags << Default << "\n";
    cout << LightRed << "[!] Programs with no rewards: " << norewards << Default << "\n";
}

void downloadFile(const string &url, const string &filepath) {
    string data = httpGet(url);
    ofstream outfile(filepath, ios::binary);
    outfile.write(data.c_str(), data.size());
    outfile.close();
    cout << LightGreen << "[!] File " << filepath << " downloaded successfully." << Default << "\n";
}

// Helper: Determine the category folder for a program.
// If the program offers swag, return "swag"; otherwise, return its platform value (or "external" if none).
string getCategory(const json &item) {
    if (item.find("swag") != item.end())
        return "swag";
    string plat = item.value("platform", "");
    return (!plat.empty() ? plat : "external");
}

// Helper: Create the proper directory structure and download the program ZIP.
// For swag items, the file is stored in: 
//     $HOME/subdomains/swag/<programName>/<programName>.zip
// For non-swag items, the file is stored directly in: 
//     $HOME/subdomains/<platform>/<programName>.zip
void downloadProgramItem(const json &item) {
    string name = item.value("name", "");
    string url = item.value("URL", "");
    string category = getCategory(item);

    // Determine the base download directory from the HOME environment variable.
    const char* home = getenv("HOME");
    string baseDir = home ? string(home) + "/subdomains" : "subdomains";

    // Create the base directory.
    create_directories(baseDir);

    // For non-swag items, use the platform (or "external") folder.
    path categoryDir = path(baseDir) / category;
    create_directories(categoryDir);

    path filePath;
    if (category == "swag") {
        // For swag, create a subdirectory with the program name.
        path programDir = categoryDir / name;
        create_directories(programDir);
        filePath = programDir / (name + ".zip");
    } else {
        // For non-swag, place the file directly into the category folder.
        filePath = categoryDir / (name + ".zip");
    }
    downloadFile(url, filePath.string());
}

void down(const json &jdata, const string &downloadName) {
    cout << "--------------------\n";
    for (const auto &item : jdata) {
        if(item.value("name", "") == downloadName) {
            cout << LightGreen << "[!] Program found." << Default << "\n";
            cout << Cyan << "[!] Downloading " << downloadName << " ..." << Default << "\n";
            downloadProgramItem(item);
            return;
        }
    }
}

void list_all(const json &jdata) {
    cout << "--------------------\n";
    cout << White << "[!] Listing all programs:" << Default << "\n";
    for (const auto &item : jdata)
        cout << Blue << item.value("name", "") << Default << "\n";
}

void bugcrowd(const json &jdata) {
    cout << "--------------------\n";
    cout << Yellow << "[!] Listing BugCrowd programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "bugcrowd")
            cout << Yellow << item.value("name", "") << Default << "\n";
}

void hackerone(const json &jdata) {
    cout << "--------------------\n";
    cout << White << "[!] Listing HackerOne programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "hackerone")
            cout << White << item.value("name", "") << Default << "\n";
}

void intigriti(const json &jdata) {
    cout << "--------------------\n";
    cout << Magenta << "[!] Listing Intigriti programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item["program_url"].get<string>().find("intigriti") != string::npos)
            cout << Magenta << item.value("name", "") << Default << "\n";
}

void external(const json &jdata) {
    cout << "--------------------\n";
    cout << Cyan << "[!] Listing self-hosted programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "")
            cout << Cyan << item.value("name", "") << Default << "\n";
}

void swags(const json &jdata) {
    cout << "--------------------\n";
    cout << LightGreen << "[!] Listing swag programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.find("swag") != item.end())
            cout << LightGreen << item.value("name", "") << Default << "\n";
}

void rewards(const json &jdata) {
    cout << "--------------------\n";
    cout << Cyan << "[!] Listing programs with rewards:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("bounty", false))
            cout << Cyan << item.value("name", "") << Default << "\n";
}

void norewards(const json &jdata) {
    cout << "--------------------\n";
    cout << Red << "[!] Listing programs with no rewards:" << Default << "\n";
    for (const auto &item : jdata)
        if(!item.value("bounty", false))
            cout << Red << item.value("name", "") << Default << "\n";
}

void list_new(const json &jdata) {
    cout << "--------------------\n";
    cout << LightGreen << "[!] Listing new programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("is_new", false))
            cout << LightGreen << item.value("name", "") << Default << "\n";
}

void changed(const json &jdata) {
    cout << "--------------------\n";
    cout << Cyan << "[!] Listing updated programs:" << Default << "\n";
    for (const auto &item : jdata)
        if(item.value("change", 0) != 0)
            cout << Cyan << item.value("name", "") << Default << "\n";
}

void download_all(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata) {
        cout << Blue << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
        downloadProgramItem(item);
    }
    cout << LightGreen << "[!] All programs downloaded successfully." << Default << "\n";
}

void bc_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "bugcrowd") {
            cout << Yellow << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All BugCrowd programs downloaded successfully." << Default << "\n";
}

void h1_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "hackerone") {
            cout << White << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All HackerOne programs downloaded successfully." << Default << "\n";
}

void intigriti_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item["program_url"].get<string>().find("intigriti") != string::npos) {
            cout << Magenta << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All Intigriti programs downloaded successfully." << Default << "\n";
}

void external_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("platform", "") == "") {
            cout << White << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All self-hosted programs downloaded successfully." << Default << "\n";
}

void new_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("is_new", false)) {
            cout << Cyan << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All new programs downloaded successfully." << Default << "\n";
}

void updated_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("change", 0) != 0) {
            cout << Blue << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All updated programs downloaded successfully." << Default << "\n";
}

void swags_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.find("swag") != item.end()) {
            cout << LightYellow << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All swag programs downloaded successfully." << Default << "\n";
}

void rewards_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(item.value("bounty", false)) {
            cout << Cyan << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All programs with rewards downloaded successfully." << Default << "\n";
}

void norewards_down(const json &jdata) {
    cout << "--------------------\n";
    for (const auto &item : jdata)
        if(!item.value("bounty", false)) {
            cout << LightRed << "[!] Downloading " << item.value("name", "") << "                   \r" << Default;
            downloadProgramItem(item);
        }
    cout << LightGreen << "[!] All programs with no rewards downloaded successfully." << Default << "\n";
}

struct Options {
    bool list = false;
    bool list_bugcrowd = false;
    bool list_hackerone = false;
    bool list_intigriti = false;
    bool list_external = false;
    bool list_swags = false;
    bool list_rewards = false;
    bool list_norewards = false;
    bool list_new = false;
    bool list_updated = false;

    bool download_all = false;
    bool download_bugcrowd = false;
    bool download_hackerone = false;
    bool download_intigriti = false;
    bool download_external = false;
    bool download_swags = false;
    bool download_rewards = false;
    bool download_norewards = false;
    bool download_new = false;
    bool download_updated = false;
    string download;
};

void printUsage(const char* progName) {
    cout << "Usage: " << progName << " [options]\n"
         << "Options:\n"
         << "  -list                        List all programs\n"
         << "  --list-bugcrowd              List BugCrowd programs\n"
         << "  --list-hackerone             List HackerOne programs\n"
         << "  --list-intigriti             List Intigriti programs\n"
         << "  --list-external              List Self Hosted programs\n"
         << "  --list-swags                 List Swags Offers programs\n"
         << "  --list-rewards               List programs with rewards\n"
         << "  --list-norewards             List programs with no rewards\n"
         << "  --list-new                   List new programs\n"
         << "  --list-updated               List updated programs\n"
         << "  -download <program>          Download specific program\n"
         << "  --download-all               Download all programs\n"
         << "  --download-bugcrowd          Download BugCrowd programs\n"
         << "  --download-hackerone         Download HackerOne programs\n"
         << "  --download-intigriti         Download Intigriti programs\n"
         << "  --download-external          Download External programs\n"
         << "  --download-swags             Download Swags programs\n"
         << "  --download-rewards           Download programs with rewards\n"
         << "  --download-norewards         Download programs with no rewards\n"
         << "  --download-new               Download new programs\n"
         << "  --download-updated           Download updated programs\n";
}

int main(int argc, char* argv[]) {
    // Clear the screen
    system("clear");
    cout << banner << "\n";

    Options opts;
    static struct option long_options[] = {
        {"list-bugcrowd",    no_argument, 0, 0},
        {"list-hackerone",   no_argument, 0, 0},
        {"list-intigriti",   no_argument, 0, 0},
        {"list-external",    no_argument, 0, 0},
        {"list-swags",       no_argument, 0, 0},
        {"list-rewards",     no_argument, 0, 0},
        {"list-norewards",   no_argument, 0, 0},
        {"list-new",         no_argument, 0, 0},
        {"list-updated",     no_argument, 0, 0},
        {"download-all",     no_argument, 0, 0},
        {"download-bugcrowd",no_argument, 0, 0},
        {"download-hackerone",no_argument,0, 0},
        {"download-intigriti",no_argument,0, 0},
        {"download-external",no_argument, 0, 0},
        {"download-swags",   no_argument, 0, 0},
        {"download-rewards", no_argument, 0, 0},
        {"download-norewards", no_argument,0, 0},
        {"download-new",      no_argument,0, 0},
        {"download-updated",  no_argument,0, 0},
        {0,0,0,0}
    };

    int opt;
    int long_index = 0;
    while ((opt = getopt_long(argc, argv, "l d:", long_options, &long_index)) != -1) {
        if (opt == 'l')
            opts.list = true;
        else if(opt == 'd')
            opts.download = optarg;
        else if(opt == 0) {
            string optName = long_options[long_index].name;
            if(optName == "list-bugcrowd") opts.list_bugcrowd = true;
            else if(optName == "list-hackerone") opts.list_hackerone = true;
            else if(optName == "list-intigriti") opts.list_intigriti = true;
            else if(optName == "list-external") opts.list_external = true;
            else if(optName == "list-swags") opts.list_swags = true;
            else if(optName == "list-rewards") opts.list_rewards = true;
            else if(optName == "list-norewards") opts.list_norewards = true;
            else if(optName == "list-new") opts.list_new = true;
            else if(optName == "list-updated") opts.list_updated = true;
            else if(optName == "download-all") opts.download_all = true;
            else if(optName == "download-bugcrowd") opts.download_bugcrowd = true;
            else if(optName == "download-hackerone") opts.download_hackerone = true;
            else if(optName == "download-intigriti") opts.download_intigriti = true;
            else if(optName == "download-external") opts.download_external = true;
            else if(optName == "download-swags") opts.download_swags = true;
            else if(optName == "download-rewards") opts.download_rewards = true;
            else if(optName == "download-norewards") opts.download_norewards = true;
            else if(optName == "download-new") opts.download_new = true;
            else if(optName == "download-updated") opts.download_updated = true;
        }
        else {
            printUsage(argv[0]);
            return 1;
        }
    }

    json jdata = getdata();
    // Print summary info always
    info(jdata);

    if (!opts.download.empty())
        down(jdata, opts.download);
    if (opts.download_all)
        download_all(jdata);
    if (opts.list)
        list_all(jdata);
    if (opts.list_bugcrowd)
        bugcrowd(jdata);
    if (opts.list_hackerone)
        hackerone(jdata);
    if (opts.list_intigriti)
        intigriti(jdata);
    if (opts.list_external)
        external(jdata);
    if (opts.list_swags)
        swags(jdata);
    if (opts.list_rewards)
        rewards(jdata);
    if (opts.list_norewards)
        norewards(jdata);
    if (opts.list_new)
        list_new(jdata);
    if (opts.list_updated)
        changed(jdata);
    if (opts.download_bugcrowd)
        bc_down(jdata);
    if (opts.download_hackerone)
        h1_down(jdata);
    if (opts.download_intigriti)
        intigriti_down(jdata);
    if (opts.download_external)
        external_down(jdata);
    if (opts.download_swags)
        swags_down(jdata);
    if (opts.download_rewards)
        rewards_down(jdata);
    if (opts.download_norewards)
        norewards_down(jdata);
    if (opts.download_new)
        new_down(jdata);
    if (opts.download_updated)
        updated_down(jdata);

    return 0;
}
