#ifndef WEBAUDIT_INCLUDE_KEYWORD_H_
#define WEBAUDIT_INCLUDE_KEYWORD_H_

#include <string>
#include <iostream>

using namespace std;

class Keyword {

    public:
        Keyword( )
        {
            risk_weight_value_     = 0;
            received_date_value_   = 0;
            updated_date_value_    = 0;
            name_string_           = "";
        }
        
        Keyword(int risk_weight_value,
                unsigned long int received_date_value,
                unsigned long int updated_date_value,
                string name_string)
        {
            risk_weight_value_     = risk_weight_value;
            received_date_value_   = received_date_value;
            updated_date_value_    = updated_date_value;
            name_string_           = name_string;
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
    
        /** Member variable name_string_
         */
        void set_name_string(string name_string)
        {
            name_string_ = name_string;
        }
    
        string get_name_string(void)
        {
            return name_string_;
        }
    
    private:
        int                risk_weight_value_;
        unsigned long int  received_date_value_;
        unsigned long int  updated_date_value_;
        string             name_string_;

};

#endif  // WEBAUDIT_INCLUDE_KEYWORD_H_

