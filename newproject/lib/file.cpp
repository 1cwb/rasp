#include "file.h"
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

namespace rasp
{
    Status File::getContent(const std::string& filename, std::string& cont)
    {
        int fd = open(filename.c_str(), O_RDONLY);
        if(fd < 0)
        {
            return Status::ioError("open", filename);
        }
        ExitCaller ecl([=](){ ::close(fd);});
        char buf[4096];
        int size = sizeof(buf);
        for(;;)
        {
            memset(buf, 0, size);
            int r = read(fd, buf, size);
            if(r < 0)
            {
                return Status::ioError("read", filename);
            }
            else if(r == 0)
            {
                break;
            }
            cont.append(buf, r);
        }
        return Status();
    }
    Status File::writeContent(const std::string& filename, const std::string& cont)
    {
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if(fd < 0)
        {
            return Status::ioError("open", filename);
        }
        ExitCaller ecl([=](){::close(fd);});
        size_t writecount = 0;
        while(cont.size() > writecount)
        {
            int r = write(fd, cont.data() + writecount, cont.size() - writecount);
            if(r < 0)
            {
                return Status::ioError("write", filename);
            }
            writecount += r;
        }
        return Status();
    }
    Status File::renameSave(const std::string& filename, const std::string& tmpName, const std::string& cont)
    {
        Status s = writeContent(tmpName, cont);
        if(s.ok())
        {
            unlink(filename.c_str());
            s = renameFile(tmpName, filename);
        }
        return s;
    }
    Status File::getChildren(const std::string& dirName, std::vector<std::string>* result)
    {
        result->clear();
        DIR* d = opendir(dirName.c_str());
        if(d == nullptr)
        {
            return Status::ioError("opendir", dirName);
        }
        ExitCaller ecl([=](){::closedir(d);});
        struct dirent *entry;
        while((entry = readdir(d)) != nullptr)
        {
            result->push_back(entry->d_name);
        }
        return Status();
    }
    Status File::deleteFile(const std::string& filename)
    {
        if(unlink(filename.c_str()) != 0)
        {
            return Status::ioError("unlink", filename);
        }
        return Status();
    }
    Status File::createDir(const std::string& dirName)
    {
        if(mkdir(dirName.c_str(), 0755) != 0)
        {
            return Status::ioError("mkdir", dirName);
        }
        return Status();
    }
    Status File::deleteDir(const std::string& dirName)
    {
        if(rmdir(dirName.c_str()) != 0)
        {
            return Status::ioError("rmdir", dirName);
        }
        return Status();
    }
    Status File::getFileSize(const std::string& filename, uint64_t* size)
    {
        struct stat sbuf;
        if(stat(filename.c_str(), &sbuf) != 0)
        {
            *size = 0;
            return Status::ioError("stat", filename);
        }
        else
        {
            *size = sbuf.st_size;
        }
        return Status();
    }
    Status File::renameFile(const std::string& filename, const std::string& target)
    {
        if(rename(filename.c_str(), target.c_str()) != 0)
        {
            return Status::ioError("rename", filename + " " + target);
        }
        return Status();
    }
    bool   File::fileExists(const std::string& filename)
    {
        return access(filename.c_str(), F_OK) == 0;
    }
}