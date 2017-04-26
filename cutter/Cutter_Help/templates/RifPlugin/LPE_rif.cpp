/*
LPE_rif.cpp
Useage:
    Save a text file (trials.lpe) in the Maya project directory containing
    one or more lines each with a lpe. For example,
    lpe:CS<L.>
    lpe:CD<L.>
    In Render Settings->Advanced->Ri Filters,
        - "Rif Plugin" load LPE_Rif.so (or .dll)
        - "Rif Args" enter trials.lpe
    Make sure RenderMan/RIS is active.
    Render a frame.
    Inspect the AOV's in the "it" catalog. 
    Edit the lpe's in trials.lpe and hit "r" to render again.
Malcolm Kesson
May 29 2015
  
May 30 2015: In the display() method if the primary display is not using "it" the
lpes stings are concatenated and the openexr driver puts all the aovs into a single
.exr file. However, if "it" is being used the lpes are sequently numbered and they
too use "it". If trials.lpe has been edited on Windows but is then used on Linux 
each line will have \r. The strip() method now removes them.

Jan 28 2016: Refer to,
	www.fundza.com/devkit/rifplugins/lpe_rif/index.html
for more details about using this rif.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
  
#include <RifPlugin.h>
#include <RifFilter.h>
#include <RixInterfaces.h>
#include <rx.h>
#include <ri.h>
  
#define LINUX 0
#define WINDOWS 1
#if defined(unix) || defined(__unix__) || defined(__unix)  || (defined(__APPLE__) && defined(__MACH__))
    #define OS_HOST LINUX
#else
    #define OS_HOST WINDOWS
#endif
  
class LPE_Rif : public RifPlugin {
    public:
             LPE_Rif(std::vector <std::string> lpes);
    virtual ~LPE_Rif() { }    
    virtual RifFilter     &GetFilter() { return m_filter; }
  
    virtual std::vector <std::string> getLPEs(std::vector <std::string> inputs);
    virtual std::vector <std::string> tokenize(char *input, RtToken sep);
    virtual std::string     getMayaProjPath();
    virtual std::string     join(std::string head, std::string tail);
    virtual std::string        strip(std::string str);
    virtual bool              pathIsGood(std::string path);
    
    private:
        RifFilter        m_filter;
        RixMessages     *m_msg;
        RixRenderState  *m_rstate;
        RixTokenStorage *m_tok_store;
        std::vector <std::string> m_lpes;
        static RtVoid    displayV(char *name, RtToken type, RtToken mode,
                                RtInt num, RtToken tokens[], RtPointer params[]);
    };
    
//-----------------------------------------------------
// RifPluginManufacture
//-----------------------------------------------------
RifPlugin *RifPluginManufacture(int argc, char **argv) {
    std::vector <std::string> inputs;
    if(argc == 1) {
        // Tokenize the input into separate strings and add them
        // to a list (vector) of inputs.
        char *ptr = strtok(*argv, " ");
        while(ptr) {
            inputs.push_back(std::string(ptr));
            ptr = strtok(NULL, " ");
            }
        }
    else // multiple inputs
        for(int n = 0; n < argc; n++) 
            inputs.push_back(std::string(argv[n]));
    return new LPE_Rif(inputs);
    }
    
//-----------------------------------------------------
// LPE_Rif
//-----------------------------------------------------
LPE_Rif::LPE_Rif(std::vector <std::string> inputs) {
    m_filter.DisplayV = displayV;  // install our custom callback
    m_lpes = getLPEs(inputs);
    m_filter.Filtering = RifFilter::k_Continue;
  
    // Connect to a couple of helpful rix contexts
    RixContext    *rixContext = RxGetRixContext();
    m_msg = (RixMessages*) rixContext->GetRixInterface(k_RixMessages);
    m_rstate = (RixRenderState*) rixContext->GetRixInterface(k_RixRenderState);
    m_tok_store = (RixTokenStorage*) rixContext->GetRixInterface(k_RixGlobalTokenData);
    }
    
//-----------------------------------------------------
// displayV
//-----------------------------------------------------
// Our custom callback version of "RiDisplayV". 
// path           "renderman/linux_0/images/linux.0002.tif",
// primary_driver "it", 
// mode           "rgba"
RtVoid LPE_Rif::displayV(char *primary_path, RtToken primary_driver, RtToken mode,
                                RtInt num, RtToken tokens[], RtPointer params[]) {
    LPE_Rif *self = static_cast<LPE_Rif*> (RifGetCurrentPlugin());
    std::string channel;
    
    // Output the primary display
    RiDisplayV(primary_path, primary_driver, mode, num, tokens, params);
  
    // Ignore secondary outputs that were assigned via the RenderMan
    // Controls window...
    if(strncmp(primary_path, "+", 1) == 0)
        return;
        
    // Assume we're using the "it" driver for the lpes
    bool useIT = true;
    std::string  parent_dir("");
    std::string  full_name("");
    std::string  frame_num("");
    std::string  lpe_base_name("+lpe_");       // keep it short
    RtToken      lpe_driver = primary_driver;  // "it"
    
    // If the primary display is not using the "it" driver we force 
    // our lpes to use "openexr" - otherwise we use "it".
    if(strncmp(primary_driver, "it", 2) != 0) {
        useIT = false;
        size_t  first, last;
        
        // Break "renderman/linux_0/images/linux.0002.tif" into,
        // parent_dir -> "renderman/linux_0/images" and,
        // full_name  -> "linux.0002.tif" then grab
        // frame_num  -> "0002"
        std::string lpe_path(primary_path);
        last = lpe_path.find_last_of('/');
        if(last != std::string::npos) {
            parent_dir = lpe_path.substr(0, last);
            full_name = lpe_path.substr(last + 1, lpe_path.length());
            
            first = full_name.find_first_of('.');
            last =  full_name.find_last_of('.');
            if(first != std::string::npos && last != std::string::npos)
                frame_num = full_name.substr(first + 1, last - first - 1);
        //    std::cout << "full_name: " << full_name << "\n";
        //    std::cout << "frame_num: " << frame_num << "\n";
        //    std::cout << "last: " << last << "\n";
            }
        lpe_base_name = "+";
        lpe_base_name += parent_dir;
        lpe_base_name += "/lpe_";
        lpe_driver = (char*)self->m_tok_store->GetToken("openexr");
        //std::cout << "primary: " << lpe_base_name << "\n";
        }
        
    // When the lpes are written to an openexr file they are specified
    // as a concatenated string separated by commas.
    std::string concat_lprs("");
    
    // Output the DisplayChannel and secondary Display for each lpe
    std::string typed_lpe;  // ex. color lpe:CD<L.>
    std::string lpe_str;    // ex. lpe:CD<L.>
    std::string lpe_type;    // ex. color
    for(unsigned int n = 0; n < self->m_lpes.size(); n++) {    
        typed_lpe = self->m_lpes[n];
        std::vector <std::string> tokens = self->tokenize((char*)typed_lpe.c_str(), RtToken(" "));
        if(tokens.size() != 2)
            continue;
        lpe_type = tokens[0];
        lpe_str = tokens[1];
        channel = lpe_type + " " + lpe_str;
        RiDisplayChannelV(RtToken(channel.c_str()), 0, NULL, NULL);
        
        if(useIT) {
            std::string    lpe_display_name = lpe_base_name;
            std::ostringstream ss;
             ss << n;
            lpe_display_name += ss.str();
            
            RtToken  lpe_name = (char*)self->m_tok_store->GetToken(lpe_display_name.c_str());
            RtToken  lpe_text = (char*)self->m_tok_store->GetToken(lpe_str.c_str());
            RiDisplayV(lpe_name, lpe_driver, lpe_text, 0, NULL, NULL);
            }
        else
            {
            concat_lprs += lpe_str;
            if(n < self->m_lpes.size() - 1)
                concat_lprs += ",";
            }
        }
    if(concat_lprs.length() > 0) {
        std::string    lpe_display_name = lpe_base_name;
        
        lpe_display_name += "aovs.";
        if(frame_num.length() > 0) 
            lpe_display_name += frame_num + ".";
            
        lpe_display_name += "exr";
        
        //std::cout << lpe_display_name << "\n";
        RtToken  lpe_name = (char*)self->m_tok_store->GetToken(lpe_display_name.c_str());
           RtToken  lpe_text = (char*)self->m_tok_store->GetToken(concat_lprs.c_str());
           RiDisplayV(lpe_name, lpe_driver, lpe_text, 0, NULL, NULL);
        }
        
    }
  
//-----------------------------------------------------
// getLPEs
//-----------------------------------------------------
// The user may have specified one or more lpe's OR the name of
// a text file located either in the current Maya proj directory
// or by a full path. Either way this method returns a list of
// lpe strings. Called by the constructor.
std::vector <std::string> LPE_Rif::getLPEs(std::vector <std::string> inputs) {
    std::vector <std::string> lpes;
    std::string path;
    std::string lpe;
    std::string copy; 
    std::string datatype_prefix;
    for(unsigned int n = 0; n < inputs.size(); n++)  {
        if(strncmp(inputs[n].c_str(), "lpe:", 4) == 0) {
            lpe = strip(inputs[n]);
            lpes.push_back(lpe);
            }
        else    // user specified a text file that lists their lpe's
            {
            path = inputs[n];
            if(pathIsGood(path) == false) {
                // Check if the lpe file is located in the Maya project dir.
                path = join(getMayaProjPath(), path);
                if(pathIsGood(path) == false)
                    continue;  // Not there so we ignore it.
                }
            std::string   line;
            std::ifstream infile(path.c_str());
            if(infile.is_open()) {
                while(std::getline(infile, line)) {
                    if(line.find_first_not_of(" \t") == line.npos || line.empty())
                        continue;
                    line = strip(line);
                    // If the line does begin with color or float we add color
                    // to the string that is assigned to the lpes vector. 
                    datatype_prefix = "color "; 
                    
                    if(line.length() > 5) {
                        // If color or float is not specified ie. lpe:CS<L.> it is considered to be "color".
                        if(strncmp(line.c_str(), "lpe:", 4) == 0) {
                            line = strip(line);
                            lpes.push_back(datatype_prefix + line);
                            }
                            
                        // Line might be "float lpe:CS<L.>" OR "color lpe:CS<L.>"
                        // Don't assume a single space between the datatype and "lpe:".
                        else if((strncmp(line.c_str(), "float", 5) == 0 ||
                                 strncmp(line.c_str(), "color", 5) == 0)) {
                            std::vector <std::string> tokens = tokenize((char*)line.c_str(), RtToken(" \t"));
                            if(tokens.size() < 2)
                                continue;
                            std::string lpe(tokens[0]); // the datatype
                            lpe += " ";
                            lpe += tokens[1];            // the lpe text
                            lpes.push_back(lpe);
                            }
                        }
                    }
                }
            infile.close();
            }
        }
    return lpes;
    }
    
//-----------------------------------------------------
// getMayaProjPath
//-----------------------------------------------------
// Can handle both Linux/OSX and Windows paths. Called by
// getLPEs().
std::string LPE_Rif::getMayaProjPath() {
    char    *resultStr = 0;
    RtInt     resultCount, resultError, resultLen = sizeof(char*);
    RixRenderState::Type resultType;
    resultError = m_rstate->GetOption("user:RMSPROJ", &resultStr, resultLen, 
                                   &resultType, &resultCount);
    if(resultStr == NULL)
        return std::string("");
    std::string str(resultStr);
    return str;
    }    
  
//-----------------------------------------------------
// pathIsGood
//-----------------------------------------------------
// This works on Windows and Linux/OSX. Called by getLPEs().
bool LPE_Rif::pathIsGood(std::string path) {
    struct stat info;
    return !stat(path.c_str(),&info );
    }
    
//-----------------------------------------------------
// join file paths
//-----------------------------------------------------
// Called by getLPEs().
std::string LPE_Rif::join(std::string head, std::string tail) {
    if(OS_HOST == LINUX) {
        char lastChar = *head.rbegin();
        return (lastChar != '/') ? head + '/' + tail : head + tail;
        }
    // Swap 
    std::replace(head.begin(), head.end(), '/', '\\');
    std::replace(tail.begin(), tail.end(), '/', '\\');
    char lastChar = *head.rbegin();
    return (lastChar != '\\') ? head + '\\' + tail : head + tail;
    }
  
//-----------------------------------------------------
// strip leading and trailing spaces
//-----------------------------------------------------
// Called by getLPEs().
std::string LPE_Rif::strip(std::string str) {
    for(unsigned int i = 0; i < str.length(); ++i) 
        if (str[i] == '\t' || str[i] == '\r')
              str[i] = ' ';
    size_t first = str.find_first_not_of(' ');
    if(first == std::string::npos)
        return str;
    size_t last = str.find_last_not_of(' ');
    if(last == std::string::npos)
        return str;
    return str.substr(first, (last - first + 1));
    }
  
//-----------------------------------------------------
// tokenize
//-----------------------------------------------------
// Called by getLPEs().
std::vector <std::string> LPE_Rif::tokenize(char *input, RtToken sep) {
    std::vector <std::string> out;
    char *ptr = strtok(input, sep);
    while(ptr) {
        out.push_back(std::string(ptr));
        ptr = strtok(NULL, sep);
        }
    return out;
    }
