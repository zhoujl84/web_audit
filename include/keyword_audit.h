#ifndef WEBAUDIT_INCLUDE_KEYWORDAUDIT_H_
#define WEBAUDIT_INCLUDE_KEYWORDAUDIT_H_

#include <string>
#include <iostream>
#include <cstdlib>
#include <map>

#include "tinyxml.h"
#include "keyword.h"

#define KEYWORD_MAX_GROUP      8

using namespace std;

typedef map<string, Keyword*> KeywordInfoMap;

enum KeywordGroups {
    kUnknownKeywordGroup = 0,     /*  Unknown Keyword Group  */
    kTrademarkCounterfeit,        /*  "商标仿冒"  */
    kProductPiracy,               /*  "产品仿冒"  */
    kFalsePropaganda,             /*  "虚假宣传"  */
    kViolenceSuspect,             /*  "涉嫌暴力"  */
    kIllegalGamble,               /*  "聚众赌博"  */
    kOnlineDeception,             /*  "网上诱骗"  */
    kAdultPornography             /*  "成人色情"  */
};


class KeywordAudit {

    public:
        KeywordAudit( )
        {
            config_file_name_ = "";
        }
        
        KeywordAudit(string config_file_name)
        {
            config_file_name_ = config_file_name;
            LoadConfigFile(config_file_name_);
        }
        
        bool LoadConfigFile(string config_file_name);
        
        int QueryKeywordRiskWeight(string name_string);
        
        KeywordInfoMap* GetKeywordInfoMapByIndex(int keyword_group_index);

		void DumpKeywordAuditConfig(void);
        
        ~KeywordAudit( )
        {
            config_file_name_ = "";
            
            KeywordInfoMap::iterator iter;
            
            for( int i = 0; i < KEYWORD_MAX_GROUP; i ++ )
            {
                for( iter = keyword_map_array[i].begin(); iter != keyword_map_array[i].end(); iter ++ )
                    delete iter->second;
                
                keyword_map_array[i].clear();
            }
        }

    private:
        string  config_file_name_;
        KeywordInfoMap  keyword_map_array[KEYWORD_MAX_GROUP];

};


bool KeywordAudit::LoadConfigFile(string config_file_name)
{
    TiXmlDocument doc(config_file_name.c_str());
    if (doc.LoadFile())
    {
        TiXmlElement* root_element    = doc.RootElement();  // KeywordAuditConfig
        
        // Outer Loop for "KeywordGroup"
        for(TiXmlNode* keyword_group = root_element->FirstChild("KeywordGroup");
                keyword_group != NULL;
                keyword_group = keyword_group->NextSibling("KeywordGroup"))
        {
            string  keyword_group_name        = "";      /* "name" Attribute value of "KeywordGroup" node */
            string  name_string               = "";      /* Text string of "Name" node */
            const char*  risk_weight_string   = NULL;    /* Text string of "RiskWeight" node */
            const char*  received_date_string = NULL;    /* Text string of "ReceivedDate" node */
            const char*  updated_date_string  = NULL;    /* Text string of "UpdatedDate" node */
            int  risk_weight_value;
            unsigned long int  received_date_value;
            unsigned long int  updated_date_value;
            enum KeywordGroups   keyword_group_index;

            // Get the "name" Attribute value of "KeywordGroup" node
            TiXmlAttribute* keyword_group_attribute = keyword_group->ToElement()->FirstAttribute();
            if ( keyword_group_attribute != NULL )
                keyword_group_name = keyword_group_attribute->Value();

            // Get the Keyword Group
            if ( keyword_group_name == "商标仿冒" )
            {
                keyword_group_index = kTrademarkCounterfeit;
            }
            else if ( keyword_group_name == "产品仿冒" )
            {
                keyword_group_index = kProductPiracy;
            }
            else if ( keyword_group_name == "虚假宣传" )
            {
                keyword_group_index = kFalsePropaganda;
            }
            else if ( keyword_group_name == "涉嫌暴力" )
            {
                keyword_group_index = kViolenceSuspect;
            }
            else if ( keyword_group_name == "聚众赌博" )
            {
                keyword_group_index = kIllegalGamble;
            }
            else if ( keyword_group_name == "网上诱骗" )
            {
                keyword_group_index = kOnlineDeception;
            }
            else if ( keyword_group_name == "成人色情" )
            {
                keyword_group_index = kAdultPornography;
            }
            else
            {
                keyword_group_index = kUnknownKeywordGroup;
            }

            // Inner Loop for "Keyword"
            for(TiXmlNode* keyword = keyword_group->ToElement()->FirstChild("Keyword");
                    keyword != NULL;
                    keyword = keyword->NextSibling("Keyword"))
            {
                // read Name
                TiXmlNode* child = keyword->FirstChild();
                name_string = child->ToElement()->GetText();
    
                // read RiskWeight
                child = keyword->IterateChildren(child);
                risk_weight_string = child->ToElement()->GetText();
                risk_weight_value = atoi(risk_weight_string);
    
                // read ReceivedDate.  
                child = keyword->IterateChildren(child);
                received_date_string = child->ToElement()->GetText();
                received_date_value = atol(received_date_string);
    
                // read UpdatedDate.  
                child = keyword->IterateChildren(child);
                updated_date_string = child->ToElement()->GetText();
                updated_date_value = atol(updated_date_string);
    
                // Create pointer to Keyword object
                Keyword* keyword_ptr = new Keyword(risk_weight_value, 
                                                   received_date_value, 
                                                   updated_date_value, 
                                                   name_string);
            
                // Insert into map
                keyword_map_array[keyword_group_index][name_string] = keyword_ptr;
            }
        }
        
        return true;
    }
    else
    {
        cout << "Can not load config file \'" << config_file_name.c_str() 
             << "\'; Error is \'" << doc.ErrorDesc() << "\'" << endl;
        cout << "Exiting" << endl;
        
        return false;
    }
}


int KeywordAudit::QueryKeywordRiskWeight(string name_string)
{
    KeywordInfoMap::iterator iter;
    
    for( int i = 0; i < KEYWORD_MAX_GROUP; i ++ )
    {
        iter = keyword_map_array[i].find(name_string);
        
        if(iter != keyword_map_array[i].end())
        {
            return iter->second->get_risk_weight_value();
        }
    }
    
    return 0;
}


KeywordInfoMap* KeywordAudit::GetKeywordInfoMapByIndex(int keyword_group_index)
{
    if ( (0 > keyword_group_index) || (KEYWORD_MAX_GROUP <= keyword_group_index) )
        return NULL;
    else
        return &(keyword_map_array[keyword_group_index]);
}


void KeywordAudit::DumpKeywordAuditConfig(void)
{
    KeywordInfoMap::iterator iter;

    cout << "Keyword Audit Config:" << endl;
    
    for( int i = 0; i < KEYWORD_MAX_GROUP; i ++ )
    {
        cout << "    KeywordGroup " << i << ":" << endl;
    
        for( iter = keyword_map_array[i].begin(); iter != keyword_map_array[i].end(); iter ++ )
    	  {
            cout << "        Name: "          << iter->second->get_name_string()         << endl;
            cout << "        RiskWeight: "    << iter->second->get_risk_weight_value()   << endl;
            cout << "        ReceivedDate: "  << iter->second->get_received_date_value() << endl;
            cout << "        UpdatedDate: "   << iter->second->get_updated_date_value()  << endl << endl;
        }

        cout << endl;
    }
    
    cout << endl;

}


#endif  // WEBAUDIT_INCLUDE_KEYWORDAUDIT_H_
