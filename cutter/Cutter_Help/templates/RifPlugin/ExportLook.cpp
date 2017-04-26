// Author: Malcolm Kesson
// Date: 10 April 2015
// Version: 4  12 April 2015
// 
// Useage:
//  Set the project directory (VERY IMPORTANT).
//	In the Mel script editor run this command,
// 		rman setPref DisableRifShaderAttachment "1";
//	Go to the "Render Setting->Advanced" tab and open the RiFilters panel.
//  Click "New Rif" and browse the the ExportLook.so file.
//  Assign RIS material and patterns to your geometry and render a frame.
//  Rib archive files for each piece of geometry can be found in your Maya
//  project directory "Looks" folder.
// Moved to fundza.com  May 23 2015
	
#include <RifPlugin.h>
#include <RifFilter.h>
#include <string>
#include <sstream>
#include <RixInterfaces.h>
#include <rx.h>
//#include <ri.h>
#include <time.h>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>

#define LINUX 0
#define WINDOWS 1
#if defined(unix) || defined(__unix__) || defined(__unix)  || (defined(__APPLE__) && defined(__MACH__))
	#define OS_HOST LINUX
	#define MKDIR(path) mkdir(path,0700)
#else
	#define OS_HOST WINDOWS
	#include <direct.h>
	#define MKDIR(path) mkdir(path)
#endif

#define ROOT_DIR_NAME "Looks/"

class ExportLook : public RifPlugin {
	public:
			 ExportLook(std::string archiveDirPath);
	virtual ~ExportLook() { }	
	virtual RifFilter &GetFilter() { return m_filter; }

	virtual std::string	 getMayaProjPath();
	virtual std::vector <std::string> tokenize(char *input, RtToken sep);
	virtual bool 		 pathIsGood(std::string path);
	virtual std::string	 join(std::string head, std::string tail);
	virtual std::string  getIdentifierName();
	virtual std::string  getSceneName();
	
	
	virtual std::string  getDeclaration(RtToken statement, RtToken name, RtToken handle, RtInt num, RtToken tokens[], RtPointer params[]);
	virtual void 		 writeArchive(std::string objectName, std::string text);

	private:
		RifFilter   	m_filter;
		RixMessages 	*m_msg;
		RixRenderState	*m_rstate;
		std::vector <std::string> m_objDB;	// initialized by the frameBegin() callback
		std::string 	m_dir_path, 
						m_looks_path;
		std::string 	m_sceneName;
		RtInt			m_frameNum;
		
		static RtVoid	bxdfV(RtToken name, RtToken handle, RtInt, RtToken[], RtPointer[]);
		static RtVoid	patternV(RtToken name, RtToken handle, RtInt, RtToken[], RtPointer[]);
		static RtVoid	surfaceV(RtToken name, RtInt, RtToken[], RtPointer[]);
		static RtVoid	frameBegin(RtInt);
		static RtVoid	displayV(char *name, RtToken type, RtToken mode, RtInt, RtToken[], RtPointer[]);
	};

RifPlugin *RifPluginManufacture(int argc, char **argv) {
	std::string archiveDirPath = "";
	if(argc == 1)
		archiveDirPath = *argv;
    return new ExportLook(archiveDirPath);
	}

//-----------------------------------------------------
// Constructor - ExportLook
//-----------------------------------------------------
// 
ExportLook::ExportLook(std::string userDirPath) {
	// Install our custom callbacks
	m_filter.BxdfV = bxdfV; 
	m_filter.PatternV = patternV;
	m_filter.SurfaceV = surfaceV;
	m_filter.FrameBegin = frameBegin;
	m_filter.DisplayV = displayV;
    m_filter.Filtering = RifFilter::k_Continue;

	// Connect to a couple of helpful rix contexts
	RixContext    *rixContext = RxGetRixContext();
	m_msg = (RixMessages*) rixContext->GetRixInterface(k_RixMessages);
	m_rstate = (RixRenderState*) rixContext->GetRixInterface(k_RixRenderState);

	// All "look" rib archives will be saved to a sub-directory of either the 
	// userDirPath specified by the user or, if that path was not provided
	// or it does not exist, to the Maya project directory.
	std::ifstream userDir(userDirPath.c_str());
	if(pathIsGood(userDirPath))
		m_dir_path = userDirPath;
	else
		m_dir_path = getMayaProjPath();

	m_msg->InfoAlways(m_dir_path.c_str());
	if(OS_HOST == LINUX)
		m_msg->InfoAlways("Operating system is linux");
	else
		m_msg->InfoAlways("Operating system is windows");
			
	m_looks_path = join(m_dir_path, std::string(ROOT_DIR_NAME)); //m_dir_path + std::string(ROOT_DIR_NAME);
	m_msg->InfoAlways(m_looks_path.c_str());
	
	std::ifstream dir(m_looks_path.c_str());
	if(pathIsGood(m_looks_path) == false)
		MKDIR(m_looks_path.c_str());
		
	std::string infoMsg("\nSaving \"Looks\" rib archive files to:\n\t");
	infoMsg += m_looks_path;
	m_msg->InfoAlways(infoMsg.c_str());
	}

//-----------------------------------------------------
// Utility - join
//-----------------------------------------------------
// Called by the constructor when making m_looks_path. In which case even on
// Windows the head and tail have forward slashes. However, when called by
// writeArchive() on Windows the head and tail will have back slashes.
std::string	 ExportLook::join(std::string head, std::string tail) {
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
// Utility - pathIsGood
//-----------------------------------------------------
// This works on Windows and Linux/OSX
bool ExportLook::pathIsGood(std::string path) {
	struct stat info;
	return !stat(path.c_str(),&info );
	}
	
//-----------------------------------------------------
// frameBegin
//-----------------------------------------------------
// Callback version of "RiFrameBegin". At the beginning a new frame the list
// of object names, used by writeArchive(), is cleared so it will not inadvertently
// aappend declarations to pre-existing "look" archive files.
RtVoid ExportLook::frameBegin(RtInt num) {
	// Pass through...
	RiFrameBegin(num);
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	self->m_frameNum = num;
	self->m_objDB.clear();
	}

//-----------------------------------------------------
// displayV
//-----------------------------------------------------
// Callback version of "RiDisplayV". Only the first arg is of interest because it
// contains the name of the scene immediately after "renderman".
RtVoid	ExportLook::displayV(char *name, RtToken type, RtToken mode, RtInt num, RtToken tokens[], RtPointer params[]) {
	// Pass through...
	RiDisplayV(name, type, mode, num, tokens, params);
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	
	std::vector <std::string> parts = self->tokenize(name, RtToken("/"));
	if(parts.size() >= 2)
		self->m_sceneName = parts[1];
	else
		self->m_sceneName = std::string("unknown_scene_name");
	}

//-----------------------------------------------------
// Utility - tokenize
//-----------------------------------------------------
std::vector <std::string> ExportLook::tokenize(char *input, RtToken sep) {
	std::vector <std::string> out;
	char *ptr = strtok(input, sep);
	while(ptr) {
		out.push_back(std::string(ptr));
		ptr = strtok(NULL, sep);
		}
	return out;
	}
			
//-----------------------------------------------------
// bxdfV
//-----------------------------------------------------
// Callback version of "RiBxdfV"
RtVoid ExportLook::bxdfV(RtToken name, RtToken handle, RtInt num, RtToken tokens[], RtPointer params[]) {
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	
	// Pass through the material description...
	RiBxdfV(name, handle, num, tokens, params);

	// Get the name of the object
	std::string identifier_name = self->getIdentifierName();

	// Ignore the default material - it is not associated with a named object,
	if(identifier_name.compare(std::string("<unnamed>")) == 0)
		return;
	
	std::string text = self->getDeclaration(RtToken("Bxdf "), name, handle, num, tokens, params);
	self->writeArchive(identifier_name, text);
	}

//-----------------------------------------------------
// patternV
//-----------------------------------------------------
// Callback version of "RiPatternV"
RtVoid ExportLook::patternV(RtToken name, RtToken handle, RtInt num, RtToken tokens[], RtPointer params[]) {
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	
	// Pass through the material description...
	RiPatternV(name, handle, num, tokens, params);

	// Get the name of the object
	std::string identifier_name = self->getIdentifierName();
	std::string text = self->getDeclaration(RtToken("Pattern "), name, handle, num, tokens, params);
	self->writeArchive(identifier_name, text);
	}

//-----------------------------------------------------
// surfaceV
//-----------------------------------------------------
// Callback version of "RiSurfaceV"
RtVoid ExportLook::surfaceV(RtToken name, RtInt num, RtToken tokens[], RtPointer params[]) {
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	
	// Pass through the material description...
	RiSurfaceV(name, num, tokens, params);

	// Get the name of the object
	std::string identifier_name = self->getIdentifierName();
	std::string text = self->getDeclaration(RtToken("Surface "), name, NULL, num, tokens, params);
	self->writeArchive(identifier_name, text);
	}

//-----------------------------------------------------
// Utility - getMayaProjPath
//-----------------------------------------------------
// Can handle both Linux/OSX and Windows paths, for example,
// Option "user" "string RMSPROJ" ["/home/jdoe/maya/projects/rif_dev/"]
// or
// Option "user" "string RMSPROJ" ["H:/maya/projects/rif_dev/"]
// Returns "/home/jdoe/maya" or "H:/maya". On Windows the path
// separator is a forward slash.
std::string ExportLook::getMayaProjPath() {
	char	*resultStr = 0;
	RtInt 	resultCount, resultError, resultLen = sizeof(char*);
	RixRenderState::Type resultType;
	resultError = m_rstate->GetOption("user:RMSPROJ", &resultStr, resultLen, 
	                               &resultType, &resultCount);
	if(resultStr == NULL)
        return std::string("");
    std::string str(resultStr);
    return str;
	}


//-----------------------------------------------------
// Utility - getIdentifierName
//-----------------------------------------------------
// Returns the name of the shape that is currently in scope. 
std::string ExportLook::getIdentifierName() {
	char	*resultStr = 0;
	RtInt 	resultCount, resultError, resultLen = sizeof(char*);
	RixRenderState::Type resultType;
	resultError = m_rstate->GetAttribute("identifier:name", &resultStr, resultLen,
                        &resultType, &resultCount);
	if(resultStr == NULL)
        return std::string("");
    std::string str(resultStr);
    return str;
    }

//-----------------------------------------------------
// Utility - getSceneName
//-----------------------------------------------------
// Returns the name of the shape that is currently in scope. 
std::string ExportLook::getSceneName() {
	RixContext   *rixContext = RxGetRixContext();
	RixRenderState *rstate = (RixRenderState*)rixContext->GetRixInterface(k_RixRenderState);
	RixRenderState::DspyInfo dspy;
	
	int	resultError = rstate->GetDspyInfo(0, &dspy); 
	if(resultError != 0)
        return std::string("");
    std::string str(dspy.mode); // also used type and name
    return str;
    }
//-----------------------------------------------------
// Utility - writeArchive
//-----------------------------------------------------
void ExportLook::writeArchive(std::string objectName, std::string text) {
	bool doAppend = false;
	// The object name is already in the database...
	if(std::find(m_objDB.begin(), m_objDB.end(), objectName) != m_objDB.end())
		doAppend = true;
	else
		m_objDB.push_back(objectName);
	
	// Full path to the output rib archive with frame number padded to 4 digits
	std::string fullpath;
	std::stringstream frame_num;
	frame_num << std::setfill('0') << std::setw(4) << m_frameNum;
	
	// Mangle the name ie. sphere:nurbsSphere1 becomes sphere_nurbsSphere1
	std::replace(objectName.begin(), objectName.end(), ':','_');
	
	// For example, 
	//	"/Users/user_name/maya/projects/proj_name/Looks/" + "0001/"
	fullpath = join(m_looks_path, frame_num.str() + std::string("/"));
	
	ExportLook *self = static_cast<ExportLook*> (RifGetCurrentPlugin());
	std::string sn = self->getSceneName();
	m_msg->InfoAlways(sn.c_str());

	// Make the frame directory if it does not exist.
	std::ifstream dir(fullpath.c_str());
	if(pathIsGood(fullpath) == false)
		MKDIR(fullpath.c_str());
			
	// For example, 
	//	"/Users/user_name/maya/projects/proj_name/Looks/0001/" + "headShape.0001.rib"
	fullpath = fullpath + objectName + "." + frame_num.str() + ".rib";

	// Write the Surface, Bxdf or Pattern declaration either by over-writing an old file
	// or by appending new text.	
	std::ofstream archive;
	if(doAppend)
		archive.open(fullpath, std::ofstream::out | std::ofstream::app);
	else
		{
		archive.open(fullpath, std::ofstream::out | std::ofstream::trunc);
		// Comments
		archive << "# Scene: " << m_sceneName << "\n";
		archive << "# Project: " << m_dir_path << "\n";
		time_t rawtime;
		time ( &rawtime );
		archive << "# Created: " << ctime(&rawtime);
		}
	archive << text;
	archive.close();
	}
		
//-----------------------------------------------------
// Utility - getDeclaration
//-----------------------------------------------------
// Returns the declaration of a Surface, Bxdf or Pattern
std::string ExportLook::getDeclaration(RtToken statement, RtToken name, RtToken handle, RtInt num, RtToken tokens[], RtPointer params[]) {
	std::string buffer("\n");
	if(handle != NULL)	// reading a Surface declaration
    	buffer = buffer + statement + "\"" + name + "\" \"" + handle + "\"\n";
	else				// reading either Bxdf or Pattern declaration
		buffer = buffer + statement + "\"" + name + "\"\n";
	char   buff[256];
    RtFloat  *fptr;
	for(int n = 0; n < num; n++) {
		RifTokenType     tokType;
        RifTokenDetail   tokDetail;
        RtInt            arraylen;
        RtInt result = RifGetDeclaration(tokens[n], &tokType, &tokDetail, &arraylen);
		if(result == 0) {
            switch(tokType) {
                case k_RifFloat:	sprintf(buff, "[%1.4f]", *(RtFloat*)params[n]); break;
                case k_RifPoint:
                case k_RifColor:               
                case k_RifVector:               
                case k_RifNormal: 	fptr = (RtFloat*)params[n];
									sprintf(buff, "[%1.4f %1.4f %1.4f]", *fptr, *(fptr+1), *(fptr+2));
                        			break;
                case k_RifMatrix:	fptr = (RtFloat*)params[n];
			                        sprintf(buff, "[%1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f\
			                                        %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f]", 
			                        *fptr,     *(fptr+1), *(fptr+2), *(fptr+3), 
									*(fptr+4), *(fptr+5), *(fptr+6), *(fptr+7), 
									*(fptr+8), *(fptr+9), *(fptr+10),*(fptr+11), 
									*(fptr+12),*(fptr+13),*(fptr+14),*(fptr+15));
			                        break;    
                case k_RifInteger:	sprintf(buff, "[%d]", *(RtInt*)params[n]); break;
                case k_RifString:	sprintf(buff, "[\"%s\"]", *(RtToken*)params[n]); break;
                default:	sprintf(buff, "[unhandled type]");
                }
			// Some basic formatting...
			std::string param("\t\"");
			param += std::string(tokens[n]);
			param += std::string("\"" );
			buffer += param;
			buffer += std::string(" ");
            buffer += std::string(buff);
			buffer += std::string("\n");
			}
		}
	return buffer;
	}
