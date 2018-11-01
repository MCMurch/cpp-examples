/* 
 * File:   main.cpp
 * Author: Matthew
 * Copyright [2018] <mruchmc>
 * Created on October 29, 2018, 2:59 PM
 */

/**
 * A program to use multiple threads to count words from data obtained
 * via a given URL.
 *
 */

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iterator>
#include <cctype>
#include <algorithm>
#include <thread>

// Using namespace to streamline working with Boost socket.
using namespace boost::asio;
using namespace boost::system;

// Shortcut to a vector of strings.
using StrVec = std::vector<std::string>;
using ThreadList = std::vector<std::thread>;

// Forward declaration for method.
StrVec loadDictionary(const std::string& filePath);

// The global dictionary of valid words
const StrVec dictionary = loadDictionary("english.txt");

// The host used for every url
const std::string ServerName = "ceclnx01.cec.miamioh.edu";

/** Return a sorted list of words from a file to use as an dictionary.
 *
 * \param[in] filePath Path to the dictionary file to be used.
 *
 * \return A vector containing a sorted list of words loaded from the
 * given file.
 */
StrVec loadDictionary(const std::string& filePath = "english.txt") {
    std::ifstream englishWords(filePath);
    if (!englishWords.good()) {
        std::cerr << "Error opening dictionary " << filePath << std::endl;
        return {};
    }
    std::istream_iterator<std::string> in(englishWords), eof;
    StrVec dictionary(in, eof);
    std::sort(dictionary.begin(), dictionary.end());
    return dictionary;
}

/**
 * Check if a given word is a valid English word.
 *
 * \param[in] dictionary A sorted dictionary of words to be used for
 * checking.  NOTE: This list *must* be sorted in order to use it with
 * binary_search.
 *
 * \param[in] word The word to be checked.
 *
 * \return This method returns true if the word was found in the
 * dictionary.  Otherwise it returns false.
 */
bool isValidWord(std::string word) {
    // Convert the word to lower case to check against the dictionary.
    std::transform(word.begin(), word.end(), word.begin(), tolower);
    // Use binary search to find word in the dictionary.
    return std::binary_search(dictionary.begin(), dictionary.end(), word);
}

/**
 * Helper method to change punctuation in a given line to blank
 * spaces.
 *
 * This method replaces puctuations and special characters in a line
 * with blank spaces. For example, "<div class='line'>and,</div>" is
 * transformed to " div class line and div".
 *
 * \param[in] line The line of data in which punctuation are to be
 * removed.
 *
 * \return A string in which punctuation and special characters have
 * been replaced with blank spaces to ease extracting words.
 */
std::string changePunct(std::string line) {
    std::replace_if(line.begin(), line.end(), ispunct, ' ');
    return line;
}

/**
 * Prints the output line with the word count and valid word count
 * @param words - word count
 * @param validWords - valid word count
 * @param path - the second half of the url (after the host)
 */
std::string printOutput(int words, int validWords, std::string path) {
    std::string url = "URL: http://ceclnx01.cec.miamioh.edu" + path; 
    std::string wCount =  ", words: " + std::to_string(words); 
    std::string valid = ", English words: " + std::to_string(validWords);
    return url + wCount + valid;
}

/**
 * Method from Exercise 8 which processes a GET request
 * @param getPath - the url
 */
std::string processRequest(const std::string& getPath) {
    std::string path = "/~raodm/SlowGet.cgi?file=" + getPath;
    const std::string host = "ceclnx01.cec.miamioh.edu";
    int words = 0;
    int validWords = 0;
    
    ip::tcp::iostream stream(host, "80");
    if (!stream) {
        // Can't connect to chirp server.
        std::cout << "Error";
    }
    // Set HTTP request to the server.
    stream << "GET " << path << " HTTP/1.1\r\n";
    stream << "Host: " << host << "\r\n";
    stream << "Connection: Close\r\n\r\n";
    // Process response from the server.  Skip header lines first.
    std::string line;
    while (std::getline(stream, line), line != "\r") {}
    std::string word;
    while (std::getline(stream, line)) {
        std::istringstream edited(changePunct(line));
        while (edited >> word) {
            words++;
            if (isValidWord(word)) 
                validWords++;
        }
    }
    return printOutput(words, validWords, path);
}

void thrMain(const StrVec& list, StrVec& results, const int startIdx, 
        const int count) {
    int end = (startIdx + count);
    for (int i = startIdx; (i < end); i++) {
        results[i] = processRequest(list[i]);
    }
}     

int main(int argc, char** argv) {
    StrVec list, results;
    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            list.push_back(argv[i]);
        }
    }

    int NumThreads = std::stoi(argv[1]);
    results.resize(list.size());
    const int count = (list.size() / NumThreads);
    
    ThreadList thrList;
    for (int start = 0, thr = 0; (thr < NumThreads); thr++, start += count) {
        thrList.push_back(std::thread(thrMain, std::ref(list),
                std::ref(results), start, count));
    }
    // If the number of threads is 3, the count will be 1 when dividing int 4 
    // by 3, and so this is to add the last file so it does not skip over it
    if (NumThreads == 3)
        thrList.push_back(std::thread(thrMain, std::ref(list),
                std::ref(results), 3, 1));
        
    for (auto& t : thrList)
        t.join();
    for (std::string line : results) {
        std::cout << line << "\n";
    }
    return 0;
}


