
#pragma once

#include "ofConstants.h"
#include "pugixml.hpp"
#include "ofAddon.h"
#include "BaseProject.h"


class VisualStudioProject: public BaseProject
{
public:
    void setup(const string& ofRoot= "../../../");

    void setup();

    bool createProjectFile();
    bool loadProjectFile();
    bool saveProjectFile();

    void addSrc(const string& srcFile, const string& folder);
    void addInclude(const string& includeName);
    void addLibrary(const string& libraryName, LibType libType);

    void addAddon(ofAddon & addon);

	static const std::string LOG_NAME;

	pugi::xml_document filterXmlDoc;


	void appendFilter(const string& folderName);

private:

};
