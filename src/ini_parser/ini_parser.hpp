// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * From https://github.com/jhpy1024/ini_parser
 * 
 * Copyright (c) 2014 Jake Horsfield
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef INI_PARSER_H
#define INI_PARSER_H

#include <map>
#include <list>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <fstream>
#include <stdexcept>


#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>


class confVar{ 
    /**
     * @todo simpleTask оптимизировать функции get_* и set_* так чтоб не преобразовывать каждый раз из строки в нужный тип 
     */ 
    std::string value;
    std::list<std::string> list;
public:
    
    int get_int() const
    {
        try{
            return std::stoi(value);
        }catch(...)
        {
            printf("\x1b[1;31mget_int exeption data=%s\x1b[0m\n", value.data());
            return 0;
        }
    }

    /*
     * The legal values for bools are BOOL_TRUE and BOOL_FALSE.
     * Anything other than these values are illegal.
     */
    bool get_bool() const
    {
        if(value.empty())
        {
            return false;
        }

        //printf("get_bool [%s] %s=%s\n", section.data(), name.data(), value.data());
        if (value.compare("True") == 0)
        {
            return true;
        }

        if (value.compare("true") == 0)
        {
            return true;
        }

        if (value.compare("TRUE") == 0)
        {
            return true;
        }

        return false;
    }

    long get_long() const
    {
        try{
            //printf("get_long [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
            return std::stol(value);
        }catch(...)
        {
            printf("\x1b[1;31mget_long exeption data=%s\x1b[0m\n", value.data()); 
            return 0;
        } 
    }

    float get_float() const
    {
        try{
            //printf("get_float [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
            return std::stof(value);
        }catch(...)
        {
            printf("\x1b[1;31mget_float exeption data=%s\x1b[0m\n", value.data());  
            return 0;
        }  
    }

    double get_double() const
    { 
        try{ 
            return std::stod(value);
        }catch(...)
        {
            printf("\x1b[1;31mget_double exeption data=%s\x1b[0m\n", value.data());   
            return 0;
        }   
    }

    std::string get_string() const
    { 
        //printf("get_string [%s] %s=%s\n", section.data(), name.data(), sections.at(section).at(name).data());
        return value;
    }

    const char* get_chars() const
    {  
        return value.data();
    } 
     
    void set_value(const std::string& val)
    {
        value = val; 
        
        if(val.empty() && !list.empty())
        {
            list.erase(list.begin(), list.end());
        }
        else if(val.compare(0, 2, "[]"))
        {
            list.push_back(val.erase(0, 2));
        }
    } 
    
    void add_value(const std::string& val)
    {
        list.push_back(val);
    } 
    
    const std::string& operator=(const std::string& val){ 
        value = val;
        return value;
    }
     
};


// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}
class ini_parser
{
    public:
        const std::string BOOL_TRUE;
        const std::string BOOL_FALSE;

        ini_parser(const std::string& filename)
            : BOOL_TRUE("BOOL_TRUE")
            , BOOL_FALSE("BOOL_FALSE")
            , filename(filename)
            , current_section("")
        {
            parse(filename);
        }

        ini_parser()
            : BOOL_TRUE("BOOL_TRUE")
            , BOOL_FALSE("BOOL_FALSE") 
            , current_section("")
        {
        }
         
        int get_int(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return 0;
            }
             
            return sections.at(section).at(name).get_int(); 
        }

        /*
         * The legal values for bools are BOOL_TRUE and BOOL_FALSE.
         * Anything other than these values are illegal.
         */
        bool get_bool(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return false;
            }
             
            return sections.at(section).at(name).get_bool(); 
        }

        long get_long(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return 0;
            }
            
            return sections.at(section).at(name).get_long();  
        }

        float get_float(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return 0;
            }
            
            return sections.at(section).at(name).get_float();   
        }

        double get_double(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return 0;
            }
            
            return sections.at(section).at(name).get_double();     
        }

        std::string get_string(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return std::string("");
            }
            
            return sections.at(section).at(name).get_string(); 
        }
        
        const char* get_chars(const std::string& section, const std::string& name= "") const
        {
            if(!is_property_exists(section, name))
            {
                return NULL;
            }
            
            return sections.at(section).at(name).get_chars();  
        }

        void set_value(const std::string& section, const std::string& name, int value)
        {
            set_value(section, name, std::to_string(value));
        }

        void set_value(const std::string& section, const std::string& name, bool value)
        {
            set_value(section, name, (value ? std::string(BOOL_TRUE) : std::string(BOOL_FALSE)));
        }

        void set_value(const std::string& section, const std::string& name, long value)
        {
            set_value(section, name, std::to_string(value));
        }

        void set_value(const std::string& section, const std::string& name, float value)
        {
            set_value(section, name, std::to_string(value));
        }

        void set_value(const std::string& section, const std::string& name, double value)
        {
            set_value(section, name, std::to_string(value));
        }

        void set_value(const std::string& section, const std::string& name, const std::string& value)
        {  
            sections[section][name].set_value(value);
        }

        bool is_property_exists(const std::string& section, const std::string& name) const
        {
            if(section.empty())
            {
                //printf("false section.empty [%s] %s\n", section.data(), name.data());
                return false;
            }
            
            if (sections.find(section) == sections.end())
            {
                //printf("false sections.end [%s] %s\n", section.data(), name.data());
                return false;
            }

            if (sections.at(section).find(name) == sections.at(section).end())
            {
                //printf("false sections.at(section).end [%s] %s\n", section.data(), name.data());
                return false;
            }
            
            //printf("true is_property_exists [%s] %s\n", section.data(), name.data());
            return true;
        }
        
        bool is_section_exists(const std::string& section) const
        {
            if (section != "" && sections.find(section) == sections.end())
            {
                return false;
            }

            return true;
        }
         
        bool parse(const std::string& FileName)
        {
            filename = FileName;
            
            std::fstream file;
            file.open(filename);
            if (!file.is_open())
            {
                std::printf("error: could not open \"%s\". terminated parsing.\n", filename.c_str());
                file.close();
                return false;
            }

            std::string line;
            while (std::getline(file, line))
            { 
                input.push_back(line);

                if (is_comment_line(line))
                {
                    //printf("comment_line:%s\n", line.data());
                    continue;
                }
                else if (is_section_start_line(line))
                {
                    //printf("section_start_line:%s\n", line.data());
                    start_section(line);
                }
                else if (is_assignment_line(line))
                {
                    //printf("assignment_line:%s\n", line.data());
                    handle_assignment(line);
                }
            }
            
            return true;
        }

    protected:
         
        void start_section(const std::string& line)
        {
            current_section = extract_section_name(line);
        }

        std::string extract_section_name(const std::string& line) const
        {
            std::string name;

            for (int i = 1; line[i] != ']'; ++i)
            {
                name += line[i];
            }

            return name;
        }

        void handle_assignment(const std::string& line)
        {
            std::string key = extract_key(line);
            std::string value = extract_value(line);

            //printf("assignment:%s[%s]=%s\n", current_section.data(), key.data(), value.data());
            set_value(current_section, key, value); 
        }

        std::string extract_key(const std::string& line) const
        {
            std::string key;

            for (int i = 0; line[i] != '='; ++i)
            {
                key += line[i];
            }

            return trim(key);
        }

        std::string extract_value(const std::string& line) const
        {
            std::string value;

            int equals_pos;
            for (equals_pos = 0; line[equals_pos] != '='; ++equals_pos)
            {
                /* Skip to equals sign. */
            }

            /* Get everything from the character following the equals sign to the end of the line. */
            for (unsigned i = equals_pos + 1; i < line.length(); ++i)
            {
                if(line[i] == '#' || line[i] == ';')
                {
                    break;
                }
                
                value += line[i];
            }
            
            return trim(value);
        }

        /*
         * A line is a comment if the first character is a semi-colon.
         */
        bool is_comment_line(const std::string& line) const
        {
            return (line.length() > 0) && (line[0] == ';' || line[0] == '#');
        }

        /*
         * A line is the start of a section if the first character is an open
         * bracket and the last character is a closing bracket.
         */
        bool is_section_start_line(const std::string& line) const
        {
            return (line.length() > 0) && (line[0] == '[') && (line[line.length() - 1] == ']');
        }

        /*
         * A line contains an assignment if it contains an equals sign and
         * there is text before and after this equals sign.
         */
        bool is_assignment_line(const std::string& line) const
        {
            std::size_t equals_pos = line.find("=");
            return (equals_pos != std::string::npos) && (equals_pos != 0) && (equals_pos != line.length() - 1);
        }
  
        std::string filename;
        std::vector<std::string> input;

        typedef std::map<std::string, confVar> properties;
        std::map<std::string, properties> sections;

        std::string current_section;
};

#endif