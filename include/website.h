#ifndef WEBAUDIT_INCLUDE_WEBSITE_H_
#define WEBAUDIT_INCLUDE_WEBSITE_H_

#include <string>
#include <iostream>

using namespace std;

class Website {

    public:
        Website( )
        {
            risk_weight_value_     = 0;
            received_date_value_   = 0;
            updated_date_value_    = 0;
            url_string_            = "";
            subject_string_        = "";
            keyword_string_        = "";
            owner_string_          = "";
            description_string_    = "";
            risk_reason_string_    = "";
        }
        
        Website(int risk_weight_value,
                unsigned long int received_date_value,
                unsigned long int updated_date_value,
                string url_string,
                string subject_string,
                string keyword_string,
                string owner_string,
                string description_string,
                string risk_reason_string)
        {
            risk_weight_value_     = risk_weight_value;
            received_date_value_   = received_date_value;
            updated_date_value_    = updated_date_value;
            url_string_            = url_string;
            subject_string_        = subject_string;
            keyword_string_        = keyword_string;
            owner_string_          = owner_string;
            description_string_    = description_string;
            risk_reason_string_    = risk_reason_string;
        }
        
        /** Member variable risk_weight_value_
         */
        void set_risk_weight_value(int risk_weight_value)
        {
            risk_weight_value_ = risk_weight_value;
        }
    
        int get_risk_weight_value(void)
        {
            return risk_weight_value_;
        }
    
        /** Member variable received_date_value_
         */
        void set_received_date_value(unsigned long int received_date_value)
        {
            received_date_value_ = received_date_value;
        }
    
        unsigned long int get_received_date_value(void)
        {
            return received_date_value_;
        }
    
        /** Member variable updated_date_value_
         */
        void set_updated_date_value(unsigned long int updated_date_value)
        {
            updated_date_value_ = updated_date_value;
        }
    
        unsigned long int get_updated_date_value(void)
        {
            return updated_date_value_;
        }
    
        /** Member variable url_string_
         */
        void set_url_string(string url_string)
        {
            url_string_ = url_string;
        }
    
        string get_url_string(void)
        {
            return url_string_;
        }
    
        /** Member variable subject_string_
         */
        void set_subject_string(string subject_string)
        {
            subject_string_ = subject_string;
        }
    
        string get_subject_string(void)
        {
            return subject_string_;
        }
    
        /** Member variable keyword_string_
         */
        void set_keyword_string(string keyword_string)
        {
            keyword_string_ = keyword_string;
        }
    
        string get_keyword_string(void)
        {
            return keyword_string_;
        }
    
        /** Member variable owner_string_
         */
        void set_owner_string(string owner_string)
        {
            owner_string_ = owner_string;
        }
    
        string get_owner_string(void)
        {
            return owner_string_;
        }
    
        /** Member variable description_string_
         */
        void set_description_string(string description_string)
        {
            description_string_ = description_string;
        }
    
        string get_description_string(void)
        {
            return description_string_;
        }
    
        /** Member variable risk_reason_string_
         */
        void set_risk_reason_string(string risk_reason_string)
        {
            risk_reason_string_ = risk_reason_string;
        }
    
        string get_risk_reason_string(void)
        {
            return risk_reason_string_;
        }
    
    private:
        int                risk_weight_value_;
        unsigned long int  received_date_value_;
        unsigned long int  updated_date_value_;
        string  url_string_;
        string  subject_string_;
        string  keyword_string_;
        string  owner_string_;
        string  description_string_;
        string  risk_reason_string_;

};

#endif  // WEBAUDIT_INCLUDE_WEBSITE_H_

