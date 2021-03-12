#pragma once
#include <string>
#include "status.h"

namespace rasp
{
    struct File
    {
        static Status getContent(const std::string& filename, std::string& cont);
        static Status writeContent(const std::string& filename, const std::string& cont);
        static Status renameSave(const std::string& filename, const std::string& tmpName, const std::string& cont);
        static Status getChildren(const std::string& dirName, std::vector<std::string>* result);
        static Status deleteFile(const std::string& filename);
        static Status createDir(const std::string& dirName);
        static Status deleteDir(const std::string& dirName);
        static Status getFileSize(const std::string& filename, uint64_t* size);
        static Status renameFile(const std::string& filename, const std::string& target);
        static bool fileExists(const std::string& filename);
    };
}