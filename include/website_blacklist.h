#ifndef WEBAUDIT_INCLUDE_WEBSITEBLACKLIST_H_
#define WEBAUDIT_INCLUDE_WEBSITEBLACKLIST_H_

#include <string>
#include <iostream>
#include <cstdlib>
#include <map>

#include "tinyxml.h"
#include "website.h"

using namespace std;

class WebsiteBlacklist {

    public:
        WebsiteBlacklist( )
        {
            config_file_name_ = "";
        }
        
        WebsiteBlacklist(string config_file_name)
        {
            config_file_name_ = config_file_name;
            LoadConfigFile(config_file_name_);
        }
        
        bool LoadConfigFile(string config_file_name);
        
        int QueryWebsiteRiskWeight(string url_string);

		void DumpWebsiteBlacklist(void);
        
        ~WebsiteBlacklist( )
        {
            config_file_name_ = "";
            
            map<string, Website*>::iterator iter;
            for( iter = website_map.begin(); iter != website_map.end(); iter ++ )
                delete iter->second;
            
            website_map.clear();
        }

    private:
        string  config_file_name_;
        map<string, Website*>  website_map;

};


bool WebsiteBlacklist::LoadConfigFile(string config_file_name)
{
    TiXmlDocument doc(config_file_name.c_str());
    if (doc.LoadFile())
    {
        TiXmlElement* root_element    = doc.RootElement();  // WebsiteBlacklistConfig
        
        for(TiXmlNode* website = root_element->FirstChild("Website");
                website != NULL;
                website = website->NextSibling("Website"))
        {
            // read Url
            TiXmlNode* child = website->FirstChild();
            string url_string = child->ToElement()->GetText();
            
            // read Subject
            child = website->IterateChildren(child);
            string subject_string = child->ToElement()->GetText();

            // read Keyword
            child = website->IterateChildren(child);
            string keyword_string = child->ToElement()->GetText();

            // read Owner
            child = website->IterateChildren(child);
            string owner_string = child->ToElement()->GetText();

            // read Description
            child = website->IterateChildren(child);
            string description_string = child->ToElement()->GetText();

            // read RiskWeight
            child = website->IterateChildren(child);
            const char* risk_weight_string = child->ToElement()->GetText();
            int risk_weight_value = atoi(risk_weight_string);

            // read RiskReason
            child = website->IterateChildren(child);
            string risk_reason_string = child->ToElement()->GetText();

            // read ReceivedDate
            child = website->IterateChildren(child);
            const char*  received_date_string = child->ToElement()->GetText();
            unsigned long int  received_date_value = atol(received_date_string);

            // read UpdatedDate
            child = website->IterateChildren(child);
            const char* updated_date_string = child->ToElement()->GetText();
            unsigned long int  updated_date_value = atol(updated_date_string);
            
            // Create pointer to Website object
            Website* website_ptr = new Website(risk_weight_value, 
                                               received_date_value, 
                                               updated_date_value, 
                                               url_string, 
                                               subject_string, 
                                               keyword_string, 
                                               owner_string, 
                                               description_string, 
                                               risk_reason_string);
        
            // Insert into map
            website_map[url_string] = website_ptr;
        }
        
        return true;
    }
    else
    {
        cout << "Can not load config file \'" << config_file_name.c_str()    \
             << "\'; Error is \'" << doc.ErrorDesc() << "\'" << endl;
        cout << "Exiting" << endl;
        
        return false;
    }
}


int WebsiteBlacklist::QueryWebsiteRiskWeight(string url_string)
{
    map<string, Website*>::iterator iter;
    
    iter = website_map.find(url_string);
    
    if(iter != website_map.end())
        return iter->second->get_risk_weight_value();
    else
        return 0;
}

void WebsiteBlacklist::DumpWebsiteBlacklist(void)
{
    map<string, Website*>::iterator iter;

    cout << "Website Blacklist Config:" << endl;

    for( iter = website_map.begin(); iter != website_map.end(); iter ++ )
    {		
        cout << "    Url: "          << iter->second->get_url_string()          << endl;
        cout << "    Subject: "      << iter->second->get_subject_string()      << endl;
        cout << "    Keyword: "      << iter->second->get_keyword_string()      << endl;
        cout << "    Owner: "        << iter->second->get_owner_string()        << endl;
        cout << "    Description: "  << iter->second->get_description_string()  << endl;
        cout << "    RiskWeight: "   << iter->second->get_risk_weight_value()   << endl;
        cout << "    RiskReason: "   << iter->second->get_risk_reason_string()  << endl;
        cout << "    ReceivedDate: " << iter->second->get_received_date_value() << endl;
        cout << "    UpdatedDate: "  << iter->second->get_updated_date_value()  << endl << endl;;
    }

    cout << endl;
}


#endif  // WEBAUDIT_INCLUDE_WEBSITEBLACKLIST_H_
