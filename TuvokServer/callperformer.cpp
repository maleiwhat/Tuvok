#include "callperformer.h"
#include "dirent.h"
#include "uvfDataset.h"
#include <limits.h>
using tuvok::DynamicBrickingDS;
using tuvok::UVFDataset;

CallPerformer::CallPerformer()
:ds(NULL)
{
}

CallPerformer::~CallPerformer() {
    delete ds;
}

//File handling
vector<std::string> CallPerformer::listFiles() {
    const char* folder = getenv("IV3D_FILES_FOLDER");
    if(folder == NULL) {
        folder = "./";
    }

    vector<std::string> retVector;
    std::string extension = ".uvf";

    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (folder)) != NULL) {
        printf("\nFound the following files:\n");
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {

            //No directories
            if(ent->d_type != DT_DIR) {
                std::string fname = ent->d_name;

                //Only files ending in .uvf
                if (fname.find(extension, (fname.length() - extension.length())) != std::string::npos) {
                    std::string tmp_string = ent->d_name;
                    printf("%s,\n", tmp_string.c_str());
                    retVector.push_back(tmp_string);
                }
            }
        }
        printf("End of list!\n");
        closedir (dir);
    } else {
        /* could not open directory */
        perror("Could not open folder for uvf files!");
        abort();
    }

    return retVector;
}

void CallPerformer::openFile(const char* filename) {
    const char* folder = getenv("IV3D_FILES_FOLDER");
    if(folder == NULL) {
        folder = "./";
    }

    std::string effectiveFilename = folder;
    effectiveFilename.append(filename);
    //printf("Effective path: %s,\n", effectiveFilename.c_str());
    std::shared_ptr<UVFDataset> uvfDS(new UVFDataset(effectiveFilename, 256, false));

    //TODO: this needs to be made dynamic
    const size_t cacheByteSize = 256*1024*1024;
    ds = new DynamicBrickingDS(uvfDS, {1024, 1024, 1024}, cacheByteSize, DynamicBrickingDS::MM_DYNAMIC);
}

void CallPerformer::closeFile(const char* filename) {
    (void)filename; //TODO: maybe keep it around to see, which file to close... currently not planned
    delete ds;
    ds = NULL;
}

void CallPerformer::rotate(const float *matrix) {
    //TODO actually implement
}