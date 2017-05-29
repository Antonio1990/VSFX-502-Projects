// Writes rib archives for any curves in a scene such as grass, hair and fur.
// If used with Maya and the "Rif Args" text field is empty the archives are
// written to "RIB_Archive" in the current project directory. Alternatively,
// the user may provide the name of a directory, say, "curve_archives". If the
// directory does not exist it is created - see displayV().
//
// Suppose the current maya project directory is,
//        FULLPATH_TO/maya/projects/chars/
// and the scene name is,
//        "curly"
// and the archives directory is,
//        "RIB_Archive"
// and the scene contains shapes named,
//        "BraidsShort1:pfxHairShape1", "BraidsShort1:pfxHairShape2"
//
// The following directory structure would be created for frame 1.
//        FULLPATH_TO/maya/projects/chars/RIB_Archive/curly_ribs/0001/BraidsShort1_pfxHairShape1_1.0001.rib
//        FULLPATH_TO/maya/projects/chars/RIB_Archive/curly_ribs/0001/BraidsShort1_pfxHairShape2_1.0001.rib
//        FULLPATH_TO/maya/projects/chars/RIB_Archive/curly.0001.rib
//
// The archive "curly.0001.rib" uses two ReadArchives to reference the ribs in "curly_ribs". 
// Malcolm A. Kesson
// Started Oct 10 2014
// Modified Sept 5 2016
#include <fstream>
#include <cstdio>
#include <sstream>
#include <RifPlugin.h>
#include <iomanip>
#include <RixInterfaces.h>
#include <rx.h>
#include <ri.h>
#include <unordered_map>
#include <sys/stat.h>
#include <vector>
  
//-----------------------------------------------------------------
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
//-----------------------------------------------------------------
  
class BakeCurves_Rif : public RifPlugin {
    public:
             BakeCurves_Rif(std::string ribdir);
    virtual ~BakeCurves_Rif() { }
    virtual RifFilter &GetFilter() { return m_filter; }
    
    private:
        // Callbacks _____________________________________________________________
        static RtVoid   displayV(char *name, RtToken type, RtToken mode, 
                                RtInt num, RtToken tokens[], RtPointer params[]);     // grabs the scene name
        static RtVoid    frameBegin(RtInt frame);                                    // grabs the frame number
        static RtVoid    frameEnd();                                                 // writes the "master" rib
        static RtVoid    curvesV(RtToken type, RtInt ncurves, RtInt nvert[],         // writes the individual ribs
                                RtToken wrap, RtInt nvals, RtToken tokens[], 
                                RtPointer params[]);    
        // Utilities _____________________________________________________________
        static std::vector <std::string> tokenize(char *input, RtToken sep);
        static void              getBBox(RtBound bbox, RtInt ncurves, RtInt nvert[],
                                      RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]);
        static RtPoint         *getCVs(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]);
        static void              writeBBoxAsComment(RtBound bbox);       
        static std::string    getIdentifierName();
        static RtToken        getRtToken(const char* str);
        static RtVoid         writeCameraVisibility(RtInt visFlag); // not used
        static std::string      getMayaProjPath();                    // not used
 
        RixMessages      *m_msg;
        RixRenderState  *m_rstate;
        RifFilter         m_filter;
       
        // An item in the hash table consists of,
        //    a key:   "identifier:name" of the curves Attribute block,
        //    a value: curve archives counter.
        std::unordered_map<std::string,int> m_identifiers;
        
        std::string   m_userArchivePath;// ex "FULLPATH_TO/maya/projects/chars/RIB_Archive/"
        std::string   m_curvesDirPath;    // ex "FULLPATH_TO/maya/projects/chars/RIB_Archive/curly_ribs/"
        std::string   m_frameDirPath;    // ex "FULLPATH_TO/maya/projects/chars/RIB_Archive/curly_ribs/0001/"
        std::string   m_frameStr;        // ex "0001"
        std::string      m_sceneName;        // ex "curly"
        RtBound          m_bbox;            // combined bbox for all curves
    };
//___________________________________________________________
// RifPluginManufacture
//___________________________________________________________
RifPlugin *RifPluginManufacture(int argc, char **argv) {
    if(argc == 1 && *argv[0] != '\0') {
        // Ensure there is a concluding forward slash.
        std::string path(argv[0]);
        size_t endpos = path.find_last_not_of("/");
        if( std::string::npos != endpos )
            path = path.substr( 0, endpos+1 ) + "/";
        return new BakeCurves_Rif(path);
        }
    else
        return new BakeCurves_Rif(argv[0]);//"");
    }
    
//___________________________________________________________
// CONSTRUCTOR - BakeCurves_Rif
//___________________________________________________________
BakeCurves_Rif::BakeCurves_Rif(std::string userdir) {
    // Get a "pipe" for messages, warnings and error messages
    RixContext  *rixContext = RxGetRixContext();
    m_msg =    (RixMessages*)    rixContext->GetRixInterface(k_RixMessages);
    m_rstate = (RixRenderState*) rixContext->GetRixInterface(k_RixRenderState);
  
    m_userArchivePath = userdir;        // tested for existance withing displayV()
  
    m_filter.DisplayV = displayV;        // the custom callbacks
    m_filter.CurvesV = curvesV;            // ...
    m_filter.FrameBegin = frameBegin;    // ...    
    m_filter.FrameEnd = frameEnd;        // ...
    }
  
//___________________________________________________________
//     CALLBACK - frameBegin()
//___________________________________________________________
// Captures the frame number as a padded string.
RtVoid BakeCurves_Rif::frameBegin(RtInt num) {
    BakeCurves_Rif *self = static_cast<BakeCurves_Rif*> (RifGetCurrentPlugin());
  
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(4) << num;
    self->m_frameStr = ss.str();
        
    // Begin new bouding box data
    self->m_bbox[0] = self->m_bbox[1] = self->m_bbox[2] = 9999999.0;
    self->m_bbox[3] = self->m_bbox[4] = self->m_bbox[5] = -9999999.0;
    RiFrameBegin(num);
    }
//-----------------------------------------------------
// CALLBACK - displayV()
//-----------------------------------------------------
// Only the first arg is of interest because it contains the name of 
// the scene immediately after "renderman".
void BakeCurves_Rif::displayV(char *name, RtToken type, RtToken mode, 
                          RtInt num, RtToken tokens[], RtPointer params[]) {
    BakeCurves_Rif *self = static_cast<BakeCurves_Rif*> (RifGetCurrentPlugin());
    // Grab the name of the scene
    std::vector <std::string> parts = self->tokenize(name, RtToken("/"));
    if(parts.size() >= 2)
        self->m_sceneName = parts[1];
    else
        self->m_sceneName = std::string("unknown_scene_name");
  
    // Check the existance of the directory that will store the main archive
    // and the sub-directory of curve archives.
    std::ifstream main_archive(self->m_userArchivePath);
    if( !main_archive.good() ) {
        if(self->m_userArchivePath.empty()) 
            self->m_userArchivePath = "RIB_Archive/";
        else
            {
            parts = self->tokenize((char*)self->m_userArchivePath.c_str(), RtToken("/"));
            if(parts.size() > 0)
                self->m_userArchivePath = parts[parts.size() - 1] + "/";
            else
                self->m_userArchivePath = "RIB_Archive/";
            }
        std::ifstream f1(self->m_userArchivePath);
        if( !f1.good() )
            MKDIR(self->m_userArchivePath.c_str());
        
        }
    // If necessary proceed to make the sub-directory to contain the curve archives.
    self->m_curvesDirPath = self->m_userArchivePath + self->m_sceneName + "_ribs/";
    std::ifstream curves(self->m_curvesDirPath.c_str());
        if( !curves.good() ) {
            MKDIR(self->m_curvesDirPath.c_str());
            }
  
    // Pass through...
    RiDisplayV(name, type, mode, num, tokens, params);
    }
  
//___________________________________________________________
//     CALLBACK - curvesV()
//___________________________________________________________
// This is where the curves are written as rib archives.
RtVoid BakeCurves_Rif::curvesV( RtToken type, RtInt ncurves, RtInt nvert[], RtToken wrap, 
                            RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    BakeCurves_Rif *self = static_cast<BakeCurves_Rif*> (RifGetCurrentPlugin());
  
    // Get the bounding box for this "clump" of curves.
    RtBound bbox;
    self->getBBox(bbox, ncurves, nvert, paramNum, paramNames, paramVals);
    
    // Update the overall bounding box.
    self->m_bbox[0] = (bbox[0] < self->m_bbox[0]) ? bbox[0] : self->m_bbox[0];
    self->m_bbox[1] = (bbox[1] < self->m_bbox[1]) ? bbox[1] : self->m_bbox[1];
    self->m_bbox[2] = (bbox[2] < self->m_bbox[2]) ? bbox[2] : self->m_bbox[2];
    self->m_bbox[3] = (bbox[3] > self->m_bbox[3]) ? bbox[3] : self->m_bbox[3];
    self->m_bbox[4] = (bbox[4] > self->m_bbox[4]) ? bbox[4] : self->m_bbox[4];
    self->m_bbox[5] = (bbox[5] > self->m_bbox[5]) ? bbox[5] : self->m_bbox[5];
    
    // Do we have a frame directory such as "0001" ?
    self->m_frameDirPath = self->m_curvesDirPath + self->m_frameStr;
    std::ifstream frames(self->m_frameDirPath.c_str());
    if( !frames.good() ) {
        MKDIR(self->m_frameDirPath.c_str());
        }
    self->m_frameDirPath += "/";
  
    //====================================
    // Update the hash table
    int count = 1;
    std::string         id_name(getIdentifierName());  // example, "BraidsShort1:pfxHairShape1"
    std::stringstream   curveID;
    std::unordered_map<std::string,int>::const_iterator found = self->m_identifiers.find(id_name);
    if (found == self->m_identifiers.end()) {
        self->m_identifiers.insert(make_pair(id_name, count));
        }
    else
        {
        count = found->second + 1;
        self->m_identifiers[id_name] = count;
        }
    curveID << count;
    
    // Example:
    // FULLPATH_TO/maya/projects/chars/RIB_Archive/curly_ribs/0001/BraidsShort1_pfxHairShape1_1.0001.rib
    std::string fullpath(self->m_frameDirPath + id_name + "_" + curveID.str() + "." + self->m_frameStr + ".rib");
      
    std::string msg(" Processing \"");
        msg += id_name + "\"\n";
        self->m_msg->InfoAlways(msg.c_str());
  
    // Mangle the name ie. BraidsShort1:pfxHairShape1 becomes BraidsShort1_pfxHairShape1
    std::replace(fullpath.begin(), fullpath.end(), ':','_');
  
    // Write a binary archive rib. If "ascii
    RiBegin( (char*)fullpath.c_str() );
        RiOption(RI_RIB, "format", &RI_BINARY, RI_NULL);
        writeBBoxAsComment(bbox); // #bbox: minx miny minz maxx maxy maxz
        RiCurvesV(type, ncurves, nvert, wrap, paramNum, paramNames, paramVals);
    RiEnd();
    // Uncomment the next line if you wish to render the curves as well as
    // baking them.
    //RiCurvesV(type, ncurves, nvert, wrap, paramNum, paramNames, paramVals);
    }
  
//___________________________________________________________
//     CALLBACK - frameEnd()
//___________________________________________________________
// This is where the master archive rib that references the curves
// archive rib files is written.
RtVoid BakeCurves_Rif::frameEnd() {
    BakeCurves_Rif *self = static_cast<BakeCurves_Rif*> (RifGetCurrentPlugin());
  
    std::string id_name;
    std::string masterArchivePath;
    std::string curveArchivePath;
    int numRibs;
  
    RtToken asciiToken = getRtToken("ascii");
    RiOption(RI_RIB, "format", &asciiToken, RI_NULL);
    std::stringstream ss;
  
    masterArchivePath = self->m_userArchivePath + self->m_sceneName + "." + self->m_frameStr + ".rib"; 
     std::replace(masterArchivePath.begin(), masterArchivePath.end(), ':','_');
    RiBegin( (char*)masterArchivePath.c_str() );
    
    writeBBoxAsComment(self->m_bbox);
    for(auto itr = self->m_identifiers.begin(); itr != self->m_identifiers.end(); itr++) {
        id_name = (*itr).first;         // ex. "BraidsShort1:pfxHairShape1"
        numRibs = (*itr).second;        // ex. number of curve archives - generally will be 1.
        for(int n = 1; n <= numRibs; n++) {
            std::stringstream curveID;
            curveID << n;
            curveArchivePath = self->m_frameDirPath + id_name + "_" + curveID.str() + "." + self->m_frameStr + ".rib";
            // Must avoid ":" in the path.
            std::replace(curveArchivePath.begin(), curveArchivePath.end(), ':','_');
            RiReadArchive((char*)curveArchivePath.c_str(), NULL, NULL); 
            }
        }
    RiEnd();
    std::string msg(" Processing completed.");
    
    // Pass through...
    RiFrameEnd();
    }
//___________________________________________________________
//     UTILITY - getCVs()
//___________________________________________________________
// Scans the paramNames until RI_P ("P") is found, then returns a
// pointer to the first RtPoint of the points array.
RtPoint* BakeCurves_Rif::getCVs(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    for(int n = 0; n < paramNum; n++) {
        if(paramNames[n] == RI_P || strcmp(paramNames[n], RI_P) == 0)
            return (RtPoint*)paramVals[n];
        }
    return NULL;
    }
//___________________________________________________________
//     UTILITY - getRtToken()
//___________________________________________________________
// Used only by BakeCurves_Rif::frameEnd() when it gets a token for "ascii".
RtToken BakeCurves_Rif::getRtToken(const char* str) {
    RixContext      *rix = RxGetRixContext();
    RixTokenStorage *tok_store = static_cast<RixTokenStorage*>(
                     rix->GetRixInterface(k_RixGlobalTokenData));
    return (char*)tok_store->GetToken(str);    
    }
//___________________________________________________________
//     UTILITY - getIdentifierName()
//___________________________________________________________
// Returns the name of the shape node to which the curves are "attached".
std::string BakeCurves_Rif::getIdentifierName() {
    char           *resultStr = 0;
    RxInfoType_t   resultType;
    RtInt          resultCount, resultError, resultLen = sizeof(char*);
    RtToken        attrname = getRtToken("identifier:name"); 
    resultError = RxAttribute(attrname, &resultStr, 
                                resultLen,     // unused
                                &resultType,   // ditto
                                &resultCount); // ditto
    if(resultStr == NULL)
        return std::string("");
    std::string str(resultStr);
    return str;
    }
//___________________________________________________________
//     UTILITY - getBBox()
//___________________________________________________________
void BakeCurves_Rif::getBBox(RtBound bbox, RtInt ncurves, RtInt nvert[],
                         RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    int numCvs = 0;
    for(int n = 0; n < ncurves; n++)
        numCvs += nvert[n];
  
    RtFloat *fPtr = (RtFloat*)getCVs(paramNum, paramNames, paramVals);
    RtFloat x,y,z;
    bbox[0] = bbox[1] = bbox[2] = 9999999.0;
    bbox[3] = bbox[4] = bbox[5] = -9999999.0;
    for(int n = 0; n < numCvs; n++) {
        x = *fPtr++;
        y = *fPtr++;
        z = *fPtr++;
        bbox[0] = (x < bbox[0]) ? x : bbox[0];
        bbox[1] = (y < bbox[1]) ? y : bbox[1];
        bbox[2] = (z < bbox[2]) ? z : bbox[2];
        bbox[3] = (x > bbox[3]) ? x : bbox[3];
        bbox[4] = (y > bbox[4]) ? y : bbox[4];
        bbox[5] = (z > bbox[5]) ? z : bbox[5];
        }
    }
//___________________________________________________________
//     UTILITY - writeBBoxAsComment()
//___________________________________________________________
// Used by BakeCurves_Rif::curvesV() for one "clump" of curves but
// also used by BakeCurves_Rif::frameEnd() when it write the main
// archive rib that references all the "clumps". 
void BakeCurves_Rif::writeBBoxAsComment(RtBound bbox) {
    std::stringstream ss;
    ss << "bbox: " << bbox[0] << " " << bbox[1] << " " << bbox[2] 
       << " " << bbox[3] << " " << bbox[4] << " " << bbox[5];
    RiArchiveRecord(RI_COMMENT, (char*)ss.str().c_str()); 
    }
//___________________________________________________________
//     UTILITY - writeCameraVisibility()
//___________________________________________________________
// Not used
RtVoid BakeCurves_Rif::writeCameraVisibility(RtInt visFlag) {
    RtToken   token = getRtToken("int camera"); 
    RtToken   visToken[] = { token };
    RtPointer visValue[] = { &visFlag };
    RiAttributeV(RI_VISIBILITY, 1, visToken, visValue);    
    }
//___________________________________________________________
//     UTILITY - getMayaProjPath()
//___________________________________________________________
// Returns the full path to the current project directory. For
// example - "/Users/jdoe/Documents/maya/projects/chars/
// Not used.
std::string BakeCurves_Rif::getMayaProjPath() {
    BakeCurves_Rif *self = static_cast<BakeCurves_Rif*> (RifGetCurrentPlugin());
    
    char    *resultStr = 0;
    RtInt    resultCount, resultError, resultLen = sizeof(char*);
    RixRenderState::Type resultType;
    resultError = self->m_rstate->GetOption("user:RMSPROJ", &resultStr, resultLen, 
                                   &resultType, &resultCount);
    if(resultStr == NULL)
        return std::string("");
    std::string str(resultStr);
    return str;
    }
//-----------------------------------------------------
// Utility - tokenize
//-----------------------------------------------------
std::vector <std::string> BakeCurves_Rif::tokenize(char *input, RtToken sep) {
    std::vector <std::string> out;
    char *ptr = strtok(input, sep);
    while(ptr) {
        out.push_back(std::string(ptr));
        ptr = strtok(NULL, sep);
        }
    return out;
    }
