// External Headers
#include <pthread.h>
#include <fstream>

// Internal Headers
#include "website_blacklist.h"
#include "keyword_audit.h"
#include "curl/curl.h"


#define  CHINESE_EXPRESSION_MAX_LENGTH    5


enum WebPageEncoding {
    kUnknownEncoding = 0,     // Unknown Encoding Style
    kUTF8,                    // UTF-8
    kGB2312                   // GB2312
};

typedef struct {
    const char * file_name_str;                  // Input argument
    WebsiteBlacklist * website_blacklist_ptr;    // Input argument
    int  output_website_risk_weight;             // Output argument
} ArgumentsForWebsiteAudit;

typedef struct {
    const char * file_name_str;                  // Input argument
    KeywordAudit * keyword_audit_ptr;            // Input argument
    int  output_keyword_risk_weight;             // Output argument
} ArgumentsForKeywordAudit;

typedef  void* (*ThreadFuncType)(void*);


//---------------------------------------------------------------------------------------
// Function Name :    size_t WriteDataToFile(void *buffer, size_t size, size_t nmemb, void *stream)
// Function Usage :    write received data to local file
// Arguments :
//     In :    void *buffer -- The received data buffer
//              size_t size -- The length of data
//              size_t nmemb -- The number of data fragment
//              void *stream -- The user defined pointer
//     Out :  The length of received data
//---------------------------------------------------------------------------------------
size_t WriteDataToFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
    FILE *fp = (FILE *)stream;
    size_t return_size = fwrite(buffer, size, nmemb, fp);

    return return_size;
}


//---------------------------------------------------------------------------------------
// Function Name :    int GetAndSaveWebpage(const char* url_str, const char* file_name_str)
// Function Usage :    Get raw datas of webpage and then write received data to local file
// Arguments :
//     In :    const char* url_str -- The URL string to be browsed
//              const char* file_name_str -- The name of file to be saved
//     Out :  The exit status -- 0 means success, otherwise not
//---------------------------------------------------------------------------------------
int GetAndSaveWebpageToFile(const char* url_str, const char* file_name_str)
{
    // Initialize libcurl
    CURLcode return_code;
    return_code = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != return_code)
    {
        cerr << "init libcurl failed." << endl;
        return -1;
    }

    // Get easy handle
    CURL *easy_handle = curl_easy_init();
    if (NULL == easy_handle)
    {
        cerr << "get a easy handle failed." << endl;
        curl_global_cleanup();

        return -1;
    }

    FILE *fp = NULL;
    if (NULL == (fp = fopen(file_name_str, "w")))
    {
        cerr << "can not create the file \"" << file_name_str << "\"" << endl;
        curl_easy_cleanup(easy_handle);

        return -1;
    }

    curl_easy_setopt(easy_handle, CURLOPT_URL, url_str);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, &WriteDataToFile);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);

    curl_easy_perform(easy_handle);

    // Release resource
    fclose(fp);
    curl_easy_cleanup(easy_handle);
    curl_global_cleanup();

    return 0;
}


//---------------------------------------------------------------------------------------
// Function Name :    int AuditContentRiskByWebsiteBlacklist(ArgumentsForWebsiteAudit *)
// Function Usage :    Extract all the href links from the local file,
//                                 then audit the total risk weights of the links based on the website blacklist.
// Arguments :
//     In :    const char * file_name_str -- The name of webpage file to be extracted
//     In :    WebsiteBlacklist * website_blacklist_ptr -- The pointer to WebsiteBlacklist instance
//     Out :   The total risk weights be correspondent with the webpage file
//---------------------------------------------------------------------------------------
void *  AuditContentRiskByWebsiteBlacklist(ArgumentsForWebsiteAudit * arg_for_website_audit_ptr)
{
    int  risk_weight_value = 0;
    string  line = "";
    string  html_content = "";

    map<string, bool>  IsWebsiteUrlAuditedMap;        // This map is used to save the audit status of websites URL.
                                                      // ( true means this URL was already audited; otherwise false )

    // Get input arguments
    const char * file_name_str = arg_for_website_audit_ptr->file_name_str;
    WebsiteBlacklist * website_blacklist_ptr = arg_for_website_audit_ptr->website_blacklist_ptr;

    ifstream  read_stream(file_name_str);

    if (!read_stream)
    {
        cerr << "failed to open the file \"" << file_name_str << "\"" << endl;
        return NULL;
    }

    cout << "    Auditing risk weight in webpage file \"" << file_name_str << "\" (by website blacklist)  ... ..." << endl;

    // Get the content of the webpage file
    while ( (!read_stream.eof()) && (getline(read_stream, line)) )
    {
        html_content += line + "\n";
    }
    read_stream.close();

    string::size_type  str_begin, str_end;
    string link_string;
    string anchor_string;

    str_begin = 0;
    while (true)  // Extract href links
    {
        anchor_string = "href=\"";
        str_begin = html_content.find(anchor_string, str_begin);

        if (str_begin != string::npos)    // if the "href=\"" exists
        {
            // Get the full string of href
            str_end = html_content.find("\"", str_begin + anchor_string.length());
            link_string = html_content.substr(str_begin + anchor_string.length(), str_end - (str_begin + anchor_string.length()));

            // Remove the prefix "http://" or "https://" from link_string
            anchor_string = "http://";
            if (link_string.find(anchor_string) != string::npos)
            {
                link_string.erase(link_string.find(anchor_string), anchor_string.length());
            }
            else    // Can not find "http://" in link_string
            {
                anchor_string = "https://";
                if (link_string.find(anchor_string) != string::npos)
                {
                    link_string.erase(link_string.find(anchor_string), anchor_string.length());
                }
            }

            // Query and accumulate the risk weight
            int tmp_risk_weight_value = website_blacklist_ptr->QueryWebsiteRiskWeight(link_string);
            if ( (0 != tmp_risk_weight_value) && (false == IsWebsiteUrlAuditedMap[link_string]) )
            {
                IsWebsiteUrlAuditedMap[link_string] = true;    // To avoid repeatedly audit
                risk_weight_value += tmp_risk_weight_value;
                
                cout << "        found href URL \"" << link_string << "\" with risk weight " << tmp_risk_weight_value << " percent(s)." << endl;
            }
            else    // link_string is not contained in the website blacklist
            {
                // Split link_string into smaller parts
                anchor_string = "/";
                if (link_string.find(anchor_string) != string::npos)
                {
                    // Remove the substring with suffix "/"
                    link_string.erase(link_string.find(anchor_string));

                    tmp_risk_weight_value = website_blacklist_ptr->QueryWebsiteRiskWeight(link_string);
                    if ( (0 != tmp_risk_weight_value) && (false == IsWebsiteUrlAuditedMap[link_string]) )
                    {
                        IsWebsiteUrlAuditedMap[link_string] = true;    // To avoid repeatedly audit
                        risk_weight_value += tmp_risk_weight_value;
                
                        cout << "        found href URL \"" << link_string << "\" with risk weight " << tmp_risk_weight_value << " percent(s)." << endl;
                    }
                }
            }

            // Remove all the characters from link_string
            link_string.erase();
            str_begin = str_end + 1;
        }
        else    // At the end of html_content
        {
            break;
        }
    }

    cout << "    Total risk weight in webpage file \"" << file_name_str << "\" (audited by website blacklist) is "       \
         << risk_weight_value << " percent(s)." << endl << endl;

    // Save the risk weight for thread function
    arg_for_website_audit_ptr->output_website_risk_weight = risk_weight_value;
}


//---------------------------------------------------------------------------------------
// Function Name :    int AuditContentRiskByKeyword(ArgumentsForKeywordAudit *)
// Function Usage :    Extract all the chinese characters from the local file,
//                                 then audit the total risk weights of the chinese characters based on the keyword config.
// Arguments :
//     In :    const char * file_name_str -- The name of webpage file to be audited
//     In :    KeywordAudit * keyword_audit_ptr -- The pointer to KeywordAudit instance
//     Out :   The total risk weights be correspondent with the webpage file
//---------------------------------------------------------------------------------------
void *  AuditContentRiskByKeyword(ArgumentsForKeywordAudit * arg_for_keyword_audit_ptr)
{
    int  risk_weight_value = 0, tmp_risk_weight_value = 0;
    int  i, start_position, number_of_chinese_characters;
    string  line = "";
    string  html_content = "";
    string  chinese_characters_content = "";
    string  query_characters = "";
    string  anchor_string = "";
    WebPageEncoding  webpage_encoding;
    
    map<string, bool>  IsKeywordAuditedMap;        // This map is used to save the audit status of keywords.
                                                   // ( true means this keyword was already audited; otherwise false )

    // Get input arguments
    const char * file_name_str = arg_for_keyword_audit_ptr->file_name_str;
    KeywordAudit * keyword_audit_ptr = arg_for_keyword_audit_ptr->keyword_audit_ptr;

    ifstream  read_stream(file_name_str);

    if (!read_stream)
    {
        cerr << "failed to open the file \"" << file_name_str << "\"" << endl;
        return NULL;
    }

    cout << "    Auditing risk weight in webpage file \"" << file_name_str << "\" (by keyword)  ... ..." << endl;

    // Get the content of the webpage file
    while ( (!read_stream.eof()) && (getline(read_stream, line)) )
    {
        html_content += line + "\n";
    }
    read_stream.close();

    anchor_string = "charset=";
    if(html_content.find(anchor_string) != string::npos)
    {
        query_characters = html_content.substr(html_content.find(anchor_string) + strlen("charset="), 6);
        
        if ( (query_characters == "gb2312") || (query_characters == "GB2312") )
        {
            webpage_encoding  = kGB2312;
        }
        else
        {
            webpage_encoding  = kUTF8;
        }
    }

    //
    // Extract all the characters in the html_content
    // And then save them into string variable "chinese_characters_content"
    //
    chinese_characters_content.erase();
    if (kGB2312 == webpage_encoding)
    {
        for (i = 0; i < html_content.length() - 1; i++)
        {
            if ( (((unsigned char)(html_content[i])) >= (unsigned char)0xB0) &&        \
                 (((unsigned char)(html_content[i])) <= (unsigned char)0xF7) &&        \
        	       (((unsigned char)(html_content[i+1])) >= (unsigned char)0xA0) )
            {
                // GB2312 encoding style
                chinese_characters_content.append(html_content.substr(i, 2));
                i += 1;
            }
        }
    }
    else    // kUTF8 == webpage_encoding
    {
        for (i = 0; i < html_content.length() - 2; i++)
        {
            if ( (html_content[i] & 0xE0) &&             \
                 (html_content[i+1] & 0x80) && (html_content[i+2] & 0x80) )
            {
                // UTF-8 encoding style
                chinese_characters_content.append(html_content.substr(i, 3));
                i += 2;
            }
        }
    }

    #ifdef DEBUG
        cout << "        The total chinese_characters_content is:" << endl;
        cout << "            " << chinese_characters_content << endl << endl;
    #endif
    
    if (kUTF8 == webpage_encoding)    // UTF-8
    {
        for (i = 0; i < chinese_characters_content.length() - 2; i += 3)
        {
            query_characters.erase();    // reset query_characters

            for ( number_of_chinese_characters = 1, start_position = i;                                                                                \
                 ((number_of_chinese_characters <= CHINESE_EXPRESSION_MAX_LENGTH) && (start_position < (chinese_characters_content.length() - 2)));    \
                  number_of_chinese_characters++, start_position += 3)
            {
                // append one chinese character every time
                query_characters.append(chinese_characters_content.substr(start_position, 3));

                tmp_risk_weight_value = keyword_audit_ptr->QueryKeywordRiskWeight(query_characters);
                if ( (0 != tmp_risk_weight_value) && (false == IsKeywordAuditedMap[query_characters]) )
                {
                    IsKeywordAuditedMap[query_characters] = true;    // To avoid repeatedly audit
                    risk_weight_value += tmp_risk_weight_value;
                    
                    cout << "        found keyword \"" << query_characters << "\" with risk weight " << tmp_risk_weight_value << " percent(s)." << endl;
                }
            }
        }
    }
    else if (kGB2312 == webpage_encoding)    // GB2312
    {
        for (i = 0; i < chinese_characters_content.length() - 1; i += 2)
        {
            query_characters.erase();    // reset query_characters

            for ( number_of_chinese_characters = 1, start_position = i;                                                                                \
                 ((number_of_chinese_characters <= CHINESE_EXPRESSION_MAX_LENGTH) && (start_position < (chinese_characters_content.length() - 1)));    \
                  number_of_chinese_characters++, start_position += 2)
            {
                // append one chinese character every time
                query_characters.append(chinese_characters_content.substr(start_position, 2));

                tmp_risk_weight_value = keyword_audit_ptr->QueryKeywordRiskWeight(query_characters);
                if ( (0 != tmp_risk_weight_value) && (false == IsKeywordAuditedMap[query_characters]) )
                {
                    IsKeywordAuditedMap[query_characters] = true;    // To avoid repeatedly audit
                    risk_weight_value += tmp_risk_weight_value;
                    
                    cout << "        found keyword \"" << query_characters << "\" with risk weight " << tmp_risk_weight_value << " percent(s)." << endl;
                }
            }
        }
    }
    else    // Unknown Encoding Style
    {
        //
        // TODO
        //
    }

    cout << "    Total risk weight in webpage file \"" << file_name_str << "\" (audited by keyword) is "       \
         << risk_weight_value << " percent(s)." << endl << endl;

    // Save the risk weight for thread function
    arg_for_keyword_audit_ptr->output_keyword_risk_weight = risk_weight_value;
}


//------------------------------------------------------------------------------------------------------------------
// Function Name :    int AuditUrlRisk(const char * , WebsiteBlacklist * , KeywordAudit * )
// Function Usage :    Audit the input string and its related infor to check whether there is risk on top of it
// Arguments :
//     In :    const char * url_str -- The URL string to be audited
//     In :    WebsiteBlacklist * website_blacklist_ptr -- The pointer to WebsiteBlacklist instance
//     In :    KeywordAudit * keyword_audit_ptr -- The pointer to KeywordAudit instance
//     Out :  The risk weight be correspondent with the input
//------------------------------------------------------------------------------------------------------------------
int AuditUrlRisk(const char * url_str, WebsiteBlacklist * website_blacklist_ptr, KeywordAudit * keyword_audit_ptr)
{
    int  risk_weight_value = 0;
    pthread_t  website_audit_thread_id;
    pthread_t  keyword_audit_thread_id;

    // safety check for the input arguments
    if ( (url_str == NULL) || (strcmp(url_str, "") == 0)         \
          || (website_blacklist_ptr == NULL) || (keyword_audit_ptr == NULL) )
    {
        cerr << "illegal input arguments of func AuditUrlRisk" << endl;
        return 0;
    }

    // concatenate url_str with "_webpage_data.html" to generate the related file_name_str
    string file_valid_name_str = url_str;
    if (file_valid_name_str.find("/") != string::npos)
    {
        file_valid_name_str.erase(file_valid_name_str.find("/"));
    }
    file_valid_name_str += "_webpage_data.html";

    // 0 means successfully returned
    if (0 != GetAndSaveWebpageToFile(url_str, file_valid_name_str.c_str()))
    {
        cerr << "failed to GetAndSaveWebpageToFile for \"" << file_valid_name_str.c_str() << "\"" << endl;
        return 0;
    }

    // Initialize the input arguments for thread functions
    ArgumentsForWebsiteAudit arg_for_website_audit = {file_valid_name_str.c_str(), website_blacklist_ptr, 0};
    ArgumentsForKeywordAudit arg_for_keyword_audit = {file_valid_name_str.c_str(), keyword_audit_ptr, 0};

    // Create two threads
    pthread_create(&website_audit_thread_id, NULL, 
                   (ThreadFuncType)AuditContentRiskByWebsiteBlacklist, (void *)&arg_for_website_audit);

    pthread_create(&keyword_audit_thread_id, NULL, 
                   (ThreadFuncType)AuditContentRiskByKeyword, (void *)&arg_for_keyword_audit);

    // Wait for the threads returned
    pthread_join(website_audit_thread_id, NULL);
    pthread_join(keyword_audit_thread_id, NULL);


    //risk_weight_value = risk_weight_value_by_website_blacklist + risk_weight_value_by_keyword;
    risk_weight_value = arg_for_website_audit.output_website_risk_weight + arg_for_keyword_audit.output_keyword_risk_weight;


    return risk_weight_value;
}


//---------------------------------------------------------------------------------------
// Function Name :    int main(int argc, const char * argv[])
// Function Usage :    The main function
// Arguments :
//     In :    int argc -- The counts of input arguments
//              const char * argv[] -- The vectors of input arguments
//     Out :  The exit status -- 0 means normal exit, otherwise abnormal exit
//---------------------------------------------------------------------------------------
int main(int argc, const char * argv[])
{
    int  website_risk_weight;
    WebsiteBlacklist  website_blacklist_obj("./config/website_blacklist_config.xml");
    KeywordAudit      keyword_audit_obj("./config/keyword_audit_config.xml");


    //
    // For DEBUG use -- Dump the website blacklist and keyword audit config
    //                  So as to check the config file were successfully loaded
    //
    #ifdef DEBUG
        website_blacklist_obj.DumpWebsiteBlacklist();
        keyword_audit_obj.DumpKeywordAuditConfig();
    #endif

    // Audit instance "www.yinghuahuiguan.net"
    website_risk_weight = AuditUrlRisk("www.yinghuahuiguan.net", &website_blacklist_obj, &keyword_audit_obj);
    cout << "Total risk weight of \"www.yinghuahuiguan.net\" is " << website_risk_weight << " percent(s)." << endl << endl;

    // Audit instance "www.suzhoumzx.com"
    website_risk_weight = AuditUrlRisk("www.suzhoumzx.com", &website_blacklist_obj, &keyword_audit_obj);
    cout << "Total risk weight of \"www.suzhoumzx.com\" is " << website_risk_weight << " percent(s)." << endl << endl;

    // Audit instance "www.bjzsyfam.com"
    website_risk_weight = AuditUrlRisk("www.bjzsyfam.com", &website_blacklist_obj, &keyword_audit_obj);
    cout << "Total risk weight of \"www.bjzsyfam.com\" is " << website_risk_weight << " percent(s)." << endl << endl;

    return 0;
}
