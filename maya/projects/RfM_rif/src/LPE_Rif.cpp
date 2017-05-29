/*
LPE_Rif.cpp
Useage:
For details about using this rif refer to,
	www.fundza.com/devkit/rifplugins/lpe_rif/index.html
Jan 20 2017
Author: Malcolm Kesson
*/
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <vector>  
#include <RifPlugin.h>
#include <RifFilter.h>
#include <RixInterfaces.h>
#include <rx.h>

#define LINUX 0
#define WINDOWS 1
#if defined(unix) || defined(__unix__) || defined(__unix)  || (defined(__APPLE__) && defined(__MACH__))
	#define OS_HOST LINUX
#else
	#define OS_HOST WINDOWS
#endif

typedef struct {
	std::vector <std::string> lpe_type;
	std::vector <std::string> lpe_text;
	std::vector <std::string> lpe_name;
	} LpeDB;
		
class LPE_Rif : public RifPlugin {
	public:
			 LPE_Rif(std::vector <std::string> lpes);
	virtual ~LPE_Rif() { }	
	virtual RifFilter	 &GetFilter() { return m_filter; }
  
	virtual LpeDB 		 readLPEs(std::vector <std::string> inputs);
	virtual std::vector <std::string> tokenize(char *input, RtToken sep);
	virtual std::string	 getMayaProjPath();
	virtual std::string	 join(std::string head, std::string tail);
	virtual std::string	 strip(std::string str);
	virtual bool 	     pathIsGood(std::string path);
	
	private:
		RifFilter		m_filter;
		RixMessages	 	*m_msg;
		RixRenderState  *m_rstate;
		RixTokenStorage *m_tok_store;
		std::vector <std::string> m_lpe_type;
		std::vector <std::string> m_lpe_text;
		std::vector <std::string> m_lpe_name;
		static RtVoid	displayV(char *name, RtToken type, RtToken mode,
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
	
	// Read the input text file that lists the lpes.
	LpeDB db = readLPEs(inputs);
	m_lpe_type = db.lpe_type;
	m_lpe_text = db.lpe_text;
	m_lpe_name = db.lpe_name;
		
	m_filter.Filtering = RifFilter::k_Continue;
  
	// Connect to a couple of helpful rix contexts
	RixContext	*rixContext = RxGetRixContext();
	m_msg = (RixMessages*) rixContext->GetRixInterface(k_RixMessages);
	m_rstate = (RixRenderState*) rixContext->GetRixInterface(k_RixRenderState);
	m_tok_store = (RixTokenStorage*) rixContext->GetRixInterface(k_RixGlobalTokenData);
	
	}
	
//-----------------------------------------------------
// displayV
//-----------------------------------------------------
// Our custom callback version of "RiDisplayV". 
// primary_img_path  ie.  "renderman/lpe_test/images/lpe_test.0002.exr",
// primary_img_path  ie.  "+renderman/lpe_test/images/lpe_test.0002.exr",
// primary_driver    ie.  "it" or "openexr", 
// mode 		     ie.  "rgba"
// This is where we write our own DisplayChannel and extra Display statements.
//
RtVoid LPE_Rif::displayV(char *primary_img_path, RtToken primary_driver, RtToken mode,
								RtInt num, RtToken tokens[], RtPointer params[]) {	
	// Output the primary Display without any alterations.
	RiDisplayV(primary_img_path, primary_driver, mode, num, tokens, params);
  
	// Allow AOVs that were assigned via the RenderMan Controls window or the
	// Render Settings "Passes" tab to be written without alteration.
	if(strncmp(primary_img_path, "+", 1) == 0) {
		return;
		}
	LPE_Rif *self = static_cast<LPE_Rif*> (RifGetCurrentPlugin());
		
	// Assume the driver is "it".
	bool 	driverIsIT = true;
	RtToken	lpe_driver = primary_driver; 
	if(strncmp(primary_driver, "it", 2) != 0) {
		driverIsIT = false;
		// Force the driver to be "openexr".
		lpe_driver = (char*)self->m_tok_store->GetToken("openexr");
		}

	// Extract the following,		
	std::string  img_parent_dir("");	// "renderman/lpe_test/images"
	std::string  img_name("");			// "lpe_test.0002.exr" or "lpe_test.0002"
	std::string  img_frame_num("0001");
	
	size_t  firstIndex, lastIndex;
	// Make a copy of the primary image path so some string methods can be used.
	std::string lpe_path(primary_img_path);
	lastIndex = lpe_path.find_last_of('/');
	if(lastIndex != std::string::npos) {
		img_parent_dir = lpe_path.substr(0, lastIndex);
		img_name = lpe_path.substr(lastIndex + 1, lpe_path.length());
		
		firstIndex = img_name.find_first_of('.');
		lastIndex =  img_name.find_last_of('.');
		if(firstIndex != std::string::npos && lastIndex != std::string::npos)
			img_frame_num = img_name.substr(firstIndex + 1, lastIndex - firstIndex - 1);
		}
	// When the lpes are written to an openexr file they are specified
	// as a concatenated string - "untitled_0,untitled_1,untitled_2".
	std::string concat_alias_names("");
	std::string	lpe_it_basepath("+" + img_parent_dir + "/");
	std::string comment(" Begin LPE_Rif output______________________");
	RiArchiveRecord(RI_COMMENT, (char*)comment.c_str());

	for(unsigned int n = 0; n < self->m_lpe_type.size(); n++) {
		std::string lpe_type = self->m_lpe_type[n];
		std::string lpe_text = self->m_lpe_text[n];
		std::string lpe_name = self->m_lpe_name[n] + std::to_string(static_cast<long long>(n));
		std::string lpe_typed_name(self->m_lpe_type[n] + " " + lpe_name);	// "color untitled_0"
		std::string lpe_typed_text(self->m_lpe_type[n] + " " + lpe_text);	// "color lpe:CD<L.'foo'>"
		
		// Ex. "color untitled_1"
		RtToken   name_token = (char*)self->m_tok_store->GetToken(lpe_typed_name.c_str());
		// Ex. "string source"
		RtToken   src_token  = (char*)self->m_tok_store->GetToken("string source");
		// Ex. "color lpe:CD<L.>"
		RtToken   src_value = (char*)self->m_tok_store->GetToken(lpe_typed_text.c_str()); 
		RtToken   str_src_tokens[] = { src_token };
		RtPointer src_value_ptr[] = { &src_value };
		// DisplayChannel "color untitled_1" "string source" ["color lpe:CD<L.>"]
		RiDisplayChannelV(name_token, 1, str_src_tokens, src_value_ptr);
		
		// For "it" a Display statement can be injected immediately after each DisplayChannel.
		if(driverIsIT) {
			std::string	lpe_it_name = lpe_it_basepath + lpe_name;//std::to_string(static_cast<long long>(n));
			RtToken  it_path_token = (char*)self->m_tok_store->GetToken(lpe_it_name.c_str());
			// Display "+renderman/lpe_test/images/lpe_0" "it" "color untitled_1" 
			RiDisplayV(it_path_token, lpe_driver, name_token, 0, NULL, NULL);
			}
		else  //...prepare a comma separated string of lpe names
			{
			concat_alias_names += lpe_name;
			if(n < self->m_lpe_type.size() - 1)
				concat_alias_names += ",";
			}
		}
	// For openexr the outputs are specified by the string of lpe names
	if(driverIsIT == false && concat_alias_names.length() > 0) {
		std::string  lpe_openexr_path("+");
		lpe_openexr_path += img_parent_dir;
		lpe_openexr_path += "/lpe_aovs.";

		if(img_frame_num.length() > 0) 
			lpe_openexr_path += img_frame_num + ".exr";
	
		RtToken  lpe_name = (char*)self->m_tok_store->GetToken(lpe_openexr_path.c_str());
		RtToken  lpe_text = (char*)self->m_tok_store->GetToken(concat_alias_names.c_str());
		RiDisplayV(lpe_name, lpe_driver, lpe_text, 0, NULL, NULL);
		}
	comment = (" End LPE_Rif output______________________\n");
	RiArchiveRecord(RI_COMMENT, (char*)comment.c_str());
	}
  
//-----------------------------------------------------
// readLPEs
//-----------------------------------------------------
// The user may have specified one or more lpe's OR the name of
// a text file located either in the current Maya proj directory
// or by a full path. Either way this method returns a list of
// lpe strings.
// Called by the constructor.
LpeDB LPE_Rif::readLPEs(std::vector <std::string> inputs) {
	std::string path;
	LpeDB database;
	for(unsigned int n = 0; n < inputs.size(); n++)  {
		path = inputs[n];
		if(pathIsGood(path) == false) {
			// Check if the lpe file is located in the Maya project dir.
			path = join(getMayaProjPath(), path);
			if(pathIsGood(path) == false)
				continue; 
			}
		std::string   line;
		std::ifstream infile(path.c_str());
		std::string   name = "untitled_";
		if(infile.is_open()) {
			while(std::getline(infile, line)) {
				// Ignore empty lines
				if(line.find_first_not_of(" \t") == line.npos || line.empty())
					continue;
				line = strip(line);
				// Ignore lines that begin with a comment character...
				if(strncmp(line.c_str(), "#", 1) == 0)
					continue; 
				if(line.length() > 4) {
					// If datatype is not specified ie. lpe:CS<L.> it is considered to be "color".
					if(strncmp(line.c_str(), "lpe:", 4) == 0) {
						line = strip(line);
						line = "color " + line;
						}
					std::vector <std::string> tokens = tokenize((char*)line.c_str(), RtToken(" \t#"));
					if(tokens.size() < 2)
						continue;
					
					if(tokens.size() >= 3)
						name = tokens[2] + "_";
					database.lpe_type.push_back(tokens[0]);
					database.lpe_text.push_back(tokens[1]);
					database.lpe_name.push_back(name);
					}
				}
			}
		infile.close();
		}
	return database;
	}
	
//-----------------------------------------------------
// getMayaProjPath
//-----------------------------------------------------
// Can handle both Linux/OSX and Windows paths. Called by
// readLPEs().
std::string LPE_Rif::getMayaProjPath() {
	char	*resultStr = 0;
	RtInt	 resultCount, resultError, resultLen = sizeof(char*);
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
// This works on Windows and Linux/OSX. Called by readLPEs().
bool LPE_Rif::pathIsGood(std::string path) {
	struct stat info;
	return !stat(path.c_str(),&info );
	}
	
//-----------------------------------------------------
// join file paths
//-----------------------------------------------------
// Called by readLPEs().
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
// Called by readLPEs().
std::string LPE_Rif::strip(std::string str) {
	for(unsigned int i = 0; i < str.length(); ++i) 
		if (str[i] == '\t' || str[i] == '\r')
			  str[i] = ' ';
	size_t first = str.find_first_not_of(' ');
	if(first == std::string::npos)
		return str;
	size_t lastIndex = str.find_last_not_of(' ');
	if(lastIndex == std::string::npos)
		return str;
	return str.substr(first, (lastIndex - first + 1));
	}
  
//-----------------------------------------------------
// tokenize
//-----------------------------------------------------
// Called by readLPEs().
std::vector <std::string> LPE_Rif::tokenize(char *input, RtToken sep) {
	std::vector <std::string> out;
	char *ptr = strtok(input, sep);
	while(ptr) {
		out.push_back(std::string(ptr));
		ptr = strtok(NULL, sep);
		}
	return out;
	}
