#include "ofx/PG/PGUtils.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

#include "Poco/DirectoryIterator.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/LocalDateTime.h"
#include "Poco/HMACEngine.h"
#include "Poco/MD5Engine.h"

//#ifdef TARGET_WIN32
//#include <direct.h>
//#define GetCurrentDir _getcwd
//#elif defined(TARGET_LINUX)
//#include <unistd.h>
//#define GetCurrentDir getcwd
//#else
//#include <mach-o/dyld.h>	/* _NSGetExecutablePath */
//#include <limits.h>		/* PATH_MAX */
//#endif

#include "ofUtils.h"


namespace ofx {
namespace PG {


Poco::Path PGUtils::OFRoot("../../..");
std::vector<std::string> PGUtils::platforms = std::vector<std::string>();


string PGUtils::generateUUID(string input){

    std::string passphrase("openFrameworks"); // HMAC needs a passphrase

    Poco::HMACEngine<Poco::MD5Engine> hmac(passphrase); // we'll compute a MD5 Hash
    hmac.update(input);

    const Poco::DigestEngine::Digest& digest = hmac.digest(); // finish HMAC computation and obtain digest
    std::string digestString(Poco::DigestEngine::digestToHex(digest)); // convert to a string of hexadecimal numbers
    
    digestString = digestString.substr(0, 24); 

    return digestString;
}


void PGUtils::findandreplace(std::string& tInput,
                             std::string tFind,
                             std::string tReplace)
{
	size_t uPos = 0;
	size_t uFindLen = tFind.length();
	size_t uReplaceLen = tReplace.length();

	if( uFindLen == 0 ){
		return;
	}

	for( ;(uPos = tInput.find( tFind, uPos )) != std::string::npos; ){
		tInput.replace( uPos, uFindLen, tReplace );
		uPos += uReplaceLen;
	}

}


void PGUtils::findandreplaceInTexfile(string fileName,
                                      std::string tFind,
                                      std::string tReplace)
{

   if (ofFile::doesFileExist(fileName))
   {
	
	    std::ifstream t(ofToDataPath(fileName).c_str());
	    std::stringstream buffer;
	    buffer << t.rdbuf();
		string bufferStr = buffer.str();
		t.close();
	    findandreplace(bufferStr, tFind, tReplace);
		ofstream myfile;
        myfile.open (ofToDataPath(fileName).c_str());
        myfile << bufferStr;
		myfile.close();

	/*
	std::ifstream ifile(ofToDataPath(fileName).c_str(),std::ios::binary);
	ifile.seekg(0,std::ios_base::end);
	long s=ifile.tellg();
	char *buffer=new char[s];
 	ifile.seekg(0);
	ifile.read(buffer,s);
	ifile.close();
	std::string txt(buffer,s);
	delete[] buffer;
	findandreplace(txt, tFind, tReplace);
	std::ofstream ofile(ofToDataPath(fileName).c_str());
	ofile.write(txt.c_str(),txt.size());
	*/  
		//return 0;
   } else {
       ; // some error checking here would be good.
   }
}


bool PGUtils::doesTagAndAttributeExist(pugi::xml_document& doc,
                                       string tag,
                                       string attribute,
                                       string newValue)
{

    char xpathExpressionExists[1024];

    sprintf(xpathExpressionExists, "//%s[@%s='%s']",
            tag.c_str(),
            attribute.c_str(),
            newValue.c_str());

    //cout <<xpathExpressionExists <<endl;

    pugi::xpath_node_set set = doc.select_nodes(xpathExpressionExists);

    return !set.empty();

}

pugi::xml_node PGUtils::appendValue(pugi::xml_document& doc,
                                    string tag,
                                    string attribute,
                                    string newValue,
                                    bool overwriteMultiple)
{
    if (overwriteMultiple)
    {
        // find the existing node...
        char xpathExpression[1024];

        sprintf(xpathExpression, "//%s[@%s='%s']", tag.c_str(), attribute.c_str(), newValue.c_str());

        pugi::xpath_node node = doc.select_single_node(xpathExpression);

        if (string(node.node().attribute(attribute.c_str()).value()).size() > 0)
        { // for some reason we get nulls here?
            // ...delete the existing node
            cout << "DELETING: " << node.node().name() << ": " << " " << node.node().attribute(attribute.c_str()).value() << endl;
            node.node().parent().remove_child(node.node());
        }
    }

    if (!doesTagAndAttributeExist(doc, tag, attribute, newValue))
    {
        // otherwise, add it please:
        char xpathExpression[1024];
        sprintf(xpathExpression, "//%s[@%s]", tag.c_str(), attribute.c_str());
        //cout << xpathExpression << endl;
        pugi::xpath_node_set add = doc.select_nodes(xpathExpression);
        pugi::xml_node node = add[add.size()-1].node();
        pugi::xml_node nodeAdded = node.parent().append_copy(node);
        nodeAdded.attribute(attribute.c_str()).set_value(newValue.c_str());
        return nodeAdded;
    }
    else
    {
    	return pugi::xml_node();
    }

}

//// todo -- this doesn't use ofToDataPath -- so it's broken a bit.  can we fix?
//void PGUtils::getFilesRecursively(const Poco::Path& path,
//                                  std::vector<std::string>& fileNames)
//{
//
//    ofDirectory dir;
//
//    //ofLogVerbose() << "in getFilesRecursively "<< path << endl;
//
//    dir.listDir(path);
//
//    for (int i = 0; i < dir.size(); ++i)
//    {
//        ofFile temp(dir.getFile(i));
//
//        if (dir.getName(i) == ".svn")
//            continue; // ignore svn
//
//        if (temp.isFile())
//        {
//            fileNames.push_back(dir.getPath(i));
//        }
//        else if (temp.isDirectory())
//        {
//            getFilesRecursively(dir.getPath(i), fileNames);
//        }
//    }
//    //folderNames.push_back(path);
//
//}


bool PGUtils::isFolderNotCurrentPlatform(string folderName, string platform)
{
	if (platforms.empty())
    {
		platforms.push_back("osx");
		platforms.push_back("win_cb");
		platforms.push_back("vs");
		platforms.push_back("ios");
		platforms.push_back("linux");
		platforms.push_back("linux64");
		platforms.push_back("android");
		platforms.push_back("iphone");
	}

	for (int i = 0; i < platforms.size(); ++i)
    {
		if (folderName == platforms[i] && folderName != platform)
        {
			return true;
		}
	}

	return false;
}


void PGUtils::splitFromLast(string toSplit,
                            string deliminator,
                            string& first,
                            string& second)
{
    size_t found = toSplit.find_last_of(deliminator.c_str());
    first = toSplit.substr(0,found);
    second = toSplit.substr(found+1);
}


void PGUtils::splitFromFirst(string toSplit,
                             string deliminator,
                             string& first,
                             string& second)
{
    size_t found = toSplit.find(deliminator.c_str());
    first = toSplit.substr(0,found );
    second = toSplit.substr(found+deliminator.size());
}


void PGUtils::getFoldersRecursively(const string & path,
                                    vector<string>& folderNames,
                                    string platform)
{
    ofDirectory dir;

    dir.listDir(path);

    for (int i = 0; i < dir.size(); ++i)
    {
        ofFile temp(dir.getFile(i));

        if (temp.isDirectory() &&
            isFolderNotCurrentPlatform(temp.getFileName(), platform) == false)
        {
            getFoldersRecursively(dir.getPath(i), folderNames, platform);
        }
    }

    folderNames.push_back(path);
}



void PGUtils::getLibsRecursively(const string & path,
                                 vector <string>& libFiles,
                                 vector < string >& libLibs,
                                 string platform)
    {
    
    if (ofFile::doesFileExist(ofFilePath::join(path, "libsorder.make")))
    {
        bool platformFound = false;
        
#ifdef TARGET_WIN32
        vector<string> splittedPath = ofSplitString(path,"\\");
#else
        vector<string> splittedPath = ofSplitString(path,"/");
#endif
        
        
        if (platform!="")
        {
            for (std::size_t j = 0; j < splittedPath.size(); ++j)
            {
                if (splittedPath[j] == platform)
                {
                    platformFound = true;
                    // break;
                }
            }
        }

        if (platformFound == true){
            vector < string > libsInOrder;
            ofFile libsorderMake(ofFilePath::join(path, "libsorder.make"));
            ofBuffer libsorderMakeBuff;
            libsorderMake >> libsorderMakeBuff;
            while(!libsorderMakeBuff.isLastLine() && libsorderMakeBuff.size() > 0){
                string line = libsorderMakeBuff.getNextLine();
                if (ofFile::doesFileExist(ofFilePath::join(path , line))){
                    
                    libLibs.push_back(ofFilePath::join(path , line) );
                } else {
                    libLibs.push_back(line);        // this might be something like ws2_32 or other libs no in this project
                }
            }
        }
    }
    else
    {
        
        
        ofDirectory dir;
        dir.listDir(path);
        
        
        for (int i = 0; i < dir.size(); ++i)
        {
#ifdef TARGET_WIN32
            vector<string> splittedPath = ofSplitString(dir.getPath(i),"\\");
#else
            vector<string> splittedPath = ofSplitString(dir.getPath(i),"/");
#endif
            
            ofFile temp(dir.getFile(i));
            
            if (temp.isDirectory())
            {
                //getLibsRecursively(dir.getPath(i), folderNames);
                getLibsRecursively(dir.getPath(i), libFiles, libLibs, platform);
                
            }
            else
            {
                bool platformFound = false;
                
                if (platform != "")
                {
                    for (std::size_t j = 0; j < splittedPath.size(); ++j)
                    {
                        if (splittedPath[j] == platform)
                        {
                            platformFound = true;
                        }
                    }
                }
                
                
                
                
                //string ext = ofFilePath::getFileExt(temp.getFile(i));
                string ext;
                string first;
                splitFromLast(dir.getPath(i), ".", first, ext);
                
                if (ext == "a" || ext == "lib" || ext == "dylib" || ext == "so" || ext == "dll"){
                    if (platformFound){
						libLibs.push_back(dir.getPath(i));
						
						//TODO: THEO hack
						if( platform == "ios" ){ //this is so we can add the osx libs for the simulator builds
							
							string currentPath = dir.getPath(i);
							
							//TODO: THEO double hack this is why we need install.xml - custom ignore ofxOpenCv 
							if( currentPath.find("ofxOpenCv") == string::npos ){
								ofStringReplace(currentPath, "ios", "osx");
								if( ofFile::doesFileExist(currentPath) ){
									libLibs.push_back(currentPath);
								}
							}
						}
					}
                } else if (ext == "h" || ext == "hpp" || ext == "c" || ext == "cpp" || ext == "cc"){
                    libFiles.push_back(dir.getPath(i));
                }
                
            }
        }
        
    }

    //folderNames.push_back(path);

    //    DirectoryIterator end;
    //        for (DirectoryIterator it(path); it != end; ++it){
    //            if (!it->isDirectory()){
    //            	string ext = ofFilePath::getFileExt(it->path());
    //            	vector<string> splittedPath = ofSplitString(ofFilePath::getEnclosingDirectory(it->path()),"/");
    //
    //                if (ext == "a" || ext == "lib" || ext == "dylib" || ext == "so"){
    //
    //                	if(platform!=""){
    //                		bool platformFound = false;
    //                		for(int i=0;i<(int)splittedPath.size();i++){
    //                			if(splittedPath[i]==platform){
    //                				platformFound = true;
    //                				break;
    //                			}
    //                		}
    //                		if(!platformFound){
    //                			continue;
    //                		}
    //                	}
    //                    libLibs.push_back(it->path());
    //                } else if (ext == "h" || ext == "hpp" || ext == "c" || ext == "cpp" || ext == "cc"){
    //                    libFiles.push_back(it->path());
    //                }
    //            }
    //
    //            if (it->isDirectory()){
    //                getLibsRecursively(it->path(), libFiles, libLibs, platform);
    //            }
    //        }
}



void PGUtils::fixSlashOrder(string& toFix)
{
    std::replace(toFix.begin(), toFix.end(),'/', '\\');
}


string PGUtils::unsplitString(vector <string> strings,
                              string deliminator)
{
    string result;

    for (int i = 0; i < strings.size(); ++i)
    {
        if (i != 0)
            result += deliminator;

        result += strings[i];
    }

    return result;
}


Poco::Path PGUtils::getOFRoot()
{
	return OFRoot;
}

Poco::Path PGUtils::getAddonsRoot()
{
    return Poco::Path(OFRoot, "addons/");
}


void PGUtils::setOFRoot(const Poco::Path& path)
{
	OFRoot = path;
}

Poco::Path PGUtils::makeRelativePath(const Poco::Path& from,
                                     const Poco::Path& to)
{
    Poco::Path fromAbsolutePath(from);
    fromAbsolutePath.makeAbsolute();

    Poco::Path toAbsolutePath(to);
    toAbsolutePath.makeAbsolute();

    Poco::Path relPath;

    int maximumDepth = MAX(fromAbsolutePath.depth(), toAbsolutePath.depth());

    for (int i = 0; i <= maximumDepth; ++i)
    {
        bool bRunOut = false;
        bool bChanged = false;

        if (i <= fromAbsolutePath.depth() &&
            i <= toAbsolutePath.depth())
        {
            if (fromAbsolutePath.directory(i) != toAbsolutePath.directory(i))
            {
                bChanged = true;
            }
        }
        else
        {
            bRunOut = true;
        }


        if (bRunOut == true || bChanged == true)
        {
            for (int j = i; j <= fromAbsolutePath.depth(); ++j)
            {
                relPath.pushDirectory("..");
            }
            
            for (int j = i; j <= toAbsolutePath.depth(); ++j)
            {
                relPath.pushDirectory(toAbsolutePath.directory(j));
            }
            
            break;
        }
    }

    ofLogVerbose("PGUtils::makeRelativePath") << "From: " << fromAbsolutePath.toString();
    ofLogVerbose("PGUtils::makeRelativePath") << "  To: " << toAbsolutePath.toString();
    ofLogVerbose("PGUtils::makeRelativePath") << " Rel: " << relPath.toString();

    return relPath;
}


Poco::Path PGUtils::getOFRelPath(const Poco::Path& from)
{
    return PGUtils::makeRelativePath(from, PGUtils::getOFRoot());
}

void PGUtils::parseAddonsDotMake(const string& path, std::vector<std::string>& addons)
{
    addons.clear();

    ofFile addonsmake(path);

    if (!addonsmake.exists())
    {
		return;
	}

    ofBuffer addonsmakebuff;

    addonsmake >> addonsmakebuff;

    ofBuffer::Line linesIter = addonsmakebuff.getLines().begin();

    while (linesIter != addonsmakebuff.getLines().end())
    {
        std::string line = *linesIter;

        if (!line.empty())
        {
            addons.push_back(line);
            std::cout << "ADDON: " << line << std::endl;
        }

        ++linesIter;
    }

    

//	while (!addonsmakebuff.isLastLine() && addonsmakebuff.size() > 0)
//    {
//        string line = addonsmakebuff.getNextLine();
//
//        if (!line.empty())
//        {
//			addons.push_back(line);
//		}
//	}
}

bool PGUtils::checkConfigExists()
{
	return ofFile::doesFileExist(ofFilePath::join(ofFilePath::getUserHomeDir(), ".ofprojectgenerator/config"));
}

bool PGUtils::askOFRoot()
{
	ofFileDialogResult res = ofSystemLoadDialog("OF project generator", "choose the folder of your OF install");

	if (res.fileName == "" || res.filePath == "")
        return false;

	ofDirectory config(ofFilePath::join(ofFilePath::getUserHomeDir(),".ofprojectgenerator"));

	config.create(true);

    ofFile configFile(ofFilePath::join(ofFilePath::getUserHomeDir(),".ofprojectgenerator/config"),ofFile::WriteOnly);

    configFile << res.filePath;

    return true;
}

string PGUtils::getOFRootFromConfig()
{
	if (!checkConfigExists())
        return "";

    ofFile configFile(ofFilePath::join(ofFilePath::getUserHomeDir(),
                                       ".ofprojectgenerator/config"),
                      ofFile::ReadOnly);

	ofBuffer filePath = configFile.readToBuffer();

	return filePath.getFirstLine();
}


} } // namespace ofx::PG
