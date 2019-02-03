// system includes
#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <utility>

// library includes
#include <boost/filesystem.hpp>

// local includes
#include "fs.h"
#include "error.h"

namespace bf = boost::filesystem;

class AppImageLauncherFS::PrivateData {
public:
    // a.k.a. where the filesystem will be mounted
    // this only needs to be calculated once, on initialization
    std::string mountpoint;

    // registered AppImages are supposed to be executed only by the user and the group
    // furthermore, they must be read-only, as we don't implement any sort of writing (we don't need it)
    static constexpr int DEFAULT_MODE = 0550;
    // mount point directory must be writable
    static constexpr int MOUNTPOINT_MODE = 0750;

    static const char REGISTER_MSG[];

    // used for internal data management
    class RegisteredAppImage {
    private:
        // store copy of assigned ID
        // not really useful now, but might make a few things easier
        int _id;
        bf::path _path;
        // open a file descriptor on the file on instantiation to keep files alive until the file is not needed any more
        int _fd;

    private:
        void openFile() {
            _fd = ::open(_path.c_str(), O_RDONLY);

            if (_fd < 0)
                throw CouldNotOpenFileError("");
        }

    public:
        RegisteredAppImage() : _id(-1), _fd(-1) {};

        RegisteredAppImage(const int id, bf::path path) : _id(id), _path(bf::absolute(path)), _fd(-1) {
            openFile();
        }

        ~RegisteredAppImage() {
            if (_fd >= 0)
                ::close(_fd);
            _fd = -1;
        }

//        RegisteredAppImage(const RegisteredAppImage& r) = delete;
        RegisteredAppImage(const RegisteredAppImage& r) : _id(r._id), _path(r._path), _fd(-1) {
            openFile();
        };

        RegisteredAppImage& operator=(const RegisteredAppImage& r) {
            _id = r._id;
            _path = r._path;
            openFile();
        }

        bool operator==(const RegisteredAppImage& r) const {
            return _id == r._id && _path == r._path;
        }

    public:
        bf::path path() const {
            return _path;
        }

        int id() const {
            return _id;
        }

        int fd() const {
            return _fd;
        }
    };

    typedef std::map<int, RegisteredAppImage> registered_appimages_t;

    // holds registered AppImages
    // they're indexed by a monotonically increasing counter, IDs may be added or removed at any time, therefore using
    // a map
    // numerical IDs are surely less cryptic than any other sort of identifier
    static registered_appimages_t registeredAppImages;
    static int counter;

    // time of creation of the instance
    // used to display atimes/mtimes of associated directories and the mountpoint
    static const time_t timeOfCreation;

public:
    PrivateData() {
        mountpoint = generateMountpointPath();

        // make sure new instances free old resources (terminating an old instance) and recreate everything from scratch
        // TODO: disables existing instance check
        // TODO: simple string concatenation = super evil
        system((std::string("fusermount -u ") + mountpoint).c_str());
        system((std::string("rmdir ") + mountpoint).c_str());

        // create mappings for all AppImages in ~/Applications, which are most used
        // TODO: allow "registration" of AppImages in any directory
        auto applicationsDir = std::string(getenv("HOME")) + "/Applications";
        if (bf::is_directory(applicationsDir)) {
            for (bf::directory_iterator it(applicationsDir); it != bf::directory_iterator(); ++it) {
                auto path = it->path();

                if (!bf::is_regular_file(path))
                    continue;

                registerAppImage(path);
            }
        }
    }

private:
    static std::string generateMountpointPath() {
        return std::string("/run/user/") + std::to_string(getuid()) + "/appimagelauncherfs/";
    }

    static std::string generateFilenameForId(int id) {
        std::ostringstream filename;
        filename << std::setfill('0') << std::setw(4) << id << ".AppImage";
        return filename.str();
    }

    static std::string generateTextMap() {
        std::vector<char> map(1, '\0');

        for (const auto& entry : registeredAppImages) {
            auto filename = generateFilenameForId(entry.first);

            std::ostringstream line;
            line << filename << " -> " << entry.second.path().string() << std::endl;
            auto lineStr = line.str();

            map.resize(map.size() + lineStr.size());
            strcat(map.data(), lineStr.c_str());
        }

        return map.data();
    }

    static int handleReadMap(void* buf, size_t bufsize, off_t offset) {
        auto map = generateTextMap();

        // cannot request more bytes than the file size
        if (offset > map.size())
            return -EIO;

        size_t bytesToCopy = std::min(bufsize, map.size()) - offset;

        // prevent int wraparound (FUSE uses 32-bit ints for everything)
        if (bytesToCopy > INT32_MAX) {
            return -EIO;
        }

        memcpy(buf, map.data() + offset, bytesToCopy);

        return static_cast<int>(bytesToCopy);
    }

    static int handleWriteRegister(char* buf, size_t bufsize, off_t offset) {
        const size_t bytesToWrite = std::min(bufsize, strlen(REGISTER_MSG)) - offset;

        // prevent int wraparound (FUSE uses 32-bit ints for everything)
        if (bytesToWrite > INT32_MAX) {
            return -EIO;
        }

        memcpy(buf + offset, REGISTER_MSG, bytesToWrite);

        return static_cast<int>(bytesToWrite);
    }

    static int handleReadRegisteredAppImage(char* buf, size_t bufsize, off_t offset, struct fuse_file_info* fi) {
        // read into buffer using pread, which was _made_ for these kinds of tasks
        ssize_t bytesRead = ::pread(static_cast<int>(fi->fh), buf, bufsize, offset);

        if (bytesRead < 0)
            return static_cast<int>(bytesRead);

        // prevent int wraparound (FUSE uses 32-bit ints for everything)
        if (bytesRead > INT32_MAX) {
            return -EIO;
        }

        // patch out (a.k.a. null) magic bytes (if necessary)
        constexpr auto magicBytesBegin = 8;
        constexpr auto magicBytesEnd = 10;
        if (offset <= magicBytesEnd) {
            auto beg = magicBytesBegin - offset;
            auto count = std::min(std::min((size_t) (magicBytesEnd - offset), (size_t) 2), bufsize) + 1;
            memset(buf + beg, '\x00', count);
        }

        return static_cast<int>(bytesRead);
    }

public:
    class CouldNotFindRegisteredAppImageError : public std::runtime_error {
    public:
        CouldNotFindRegisteredAppImageError() : runtime_error("") {};
    };

    bool otherInstanceRunning() const {
        // TODO: implement properly (as in, check for stale mountpoint)
        return bf::is_directory(mountpoint);
    }

    static int registerAppImage(const bf::path& path) {
        if (!bf::exists(path))
            throw FileNotFoundError();

        // TODO: implement check whether file is an AppImage (i.e., if it is a regular file and contains the AppImage magic bytes)

        // check whether file is registered already

        for (const auto& r : registeredAppImages) {
            if (path == r.second.path()) {
                throw AppImageAlreadyRegisteredError(r.first);
            }
        }

        const auto id = counter++;
        registeredAppImages.emplace(id, RegisteredAppImage(id, path));

        std::cout << "Registered new AppImage: " << path << " (ID: " << std::setfill('0') << std::setw(4) << id  << ")" << std::endl;
        return id;
    }

    /**
     * Maps a FUSE filesystem path to a registered AppImage's path
     * @param path path provided by FUSE (must begin with leading slash)
     * @return path of AppImage mapping to this path
     * @throws CouldNotFindRegisteredAppImageError if path doesn't refer to registered AppImage
     * @throws std::invalid_argument if path is invalid
     */
    static RegisteredAppImage& mapPathToRegisteredAppImage(const std::string& path) {
        std::vector<char> mutablePath(path.size() + 1);
        strcpy(mutablePath.data(), path.c_str());
        auto mutablePathPtr = mutablePath.data();

        char* firstPart = strsep(&mutablePathPtr, ".");

        // skip leading slash
        if (firstPart[0] != '/')
            throw InvalidPathError("Path doesn't begin with leading /");

        firstPart++;

        // try to convert string ID to int
        int id;
        try {
            id = std::stoi(firstPart);
        } catch (const std::invalid_argument&) {
            throw CouldNotFindRegisteredAppImageError();
        }

        // check if filename matches the one we'd generate for parsed id
        // that'll make sure only the listed files in the used scheme are covered by this function
        if (path != ("/" + generateFilenameForId(id))) {
            throw CouldNotFindRegisteredAppImageError();
        }

        return registeredAppImages[id];
    }

    static int getattr(const char* path, struct stat* st) {
        std::string strpath(path);

        // there must be exactly one slash in every path only
        if (std::count(strpath.begin(), strpath.end(), '/') != 1)
            throw InvalidPathError("Path must contain exactly one /");

        // path must start with said slash, it may not be in any other position
        if (strpath.find('/') != 0)
            throw InvalidPathError("Path does not start with /");

        // root directory entry
        if (strpath == "/") {
            st->st_atim = timespec{timeOfCreation, 0};
            st->st_mtim = timespec{timeOfCreation, 0};

            st->st_gid = getgid();
            st->st_uid = getuid();

            st->st_mode = S_IFDIR | DEFAULT_MODE;
            st->st_nlink = 2;

            return 0;
        }

        // virtual read-only file generated on demand from the data stored by the fs
        if (strpath == "/map") {
            st->st_atim = timespec{timeOfCreation, 0};
            st->st_mtim = timespec{timeOfCreation, 0};

            st->st_gid = getgid();
            st->st_uid = getuid();

            st->st_mode = S_IFREG | 0444;
            st->st_nlink = 1;

            auto map = generateTextMap();
            st->st_size = map.size();

            return 0;
        }

        // virtual readable and writable file
        // on read, the file will return a static help message (hardcoded in the codebase)
        // on write, it will interpret every line as a path of an AppImage that shall be registered
        if (strpath == "/register") {
            st->st_atim = timespec{timeOfCreation, 0};
            st->st_mtim = timespec{timeOfCreation, 0};

            st->st_gid = getgid();
            st->st_uid = getuid();

            st->st_mode = S_IFREG | 0660;
            st->st_nlink = 1;

            st->st_size = strlen(REGISTER_MSG);

            return 0;
        }

        // the only remaining entries might be registered AppImages, so let's try to find such an entry
        // on success, we read the original file's metadata, alter it a bit (e.g., change permissions to read-only)
        // if an entry cannot be found, we return an appropriate error code
        try {
            auto& registeredAppImage = mapPathToRegisteredAppImage(path);

            if (!bf::is_regular_file(registeredAppImage.path())) {
                return -EIO;
            }

            if (stat(registeredAppImage.path().c_str(), st) != 0)
                throw std::runtime_error("stat() failed");

            // overwrite permissions: read-only executable
            st->st_mode = S_IFREG | 0555;
            st->st_nlink = 1;

            return 0;
        } catch (const CouldNotFindRegisteredAppImageError&) {
            return -ENOENT;
        }

        // return generic I/O error if we couldn't generate a better reply until this point
        // (hint: this line _should_ be unreachable)
        return -EIO;
    }

    static int readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
        // we only have the root dir, so any other path shall be rejected
        if (strcmp(path, "/") != 0)
            return -ENOENT;

        // these entries must appear in every directory
        filler(buf, ".", nullptr, 0);
        filler(buf, "..", nullptr, 0);

        // two virtual files provided by the filesystem process
        filler(buf, "map", nullptr, 0);
        filler(buf, "register", nullptr, 0);

        // virtual entries mapping to the real registered AppImages
        for (const auto& entry : PrivateData::registeredAppImages) {
            auto filename = generateFilenameForId(entry.first);
            filler(buf, filename.c_str(), nullptr, 0);
        }

        return 0;
    }

    static int read(const char* path, char* buf, size_t bufsize, off_t offset, struct fuse_file_info* fi) {
        if (strcmp(path, "/map") == 0) {
            return handleReadMap(buf, bufsize, offset);
        }

        // shall be written to only
        // this is handled by getattr() already, but a bit more error checking doesn't hurt
        if (strcmp(path, "/register") == 0) {
            return handleWriteRegister(buf, bufsize, offset);
        }

        // only left option is that the path refers to a registered AppImage
        // in this case, we first check whether the path does resolve to a registered AppImage
        try {
            auto& registeredAppImage = mapPathToRegisteredAppImage(path);

            return handleReadRegisteredAppImage(buf, bufsize, offset, fi);
        } catch (const CouldNotFindRegisteredAppImageError&) {
            // must be an unknown file
            return -ENOENT;
        }

        // return generic I/O error if we couldn't generate a better reply until this point
        // (hint: this line _should_ be unreachable)
        return -EIO;
    }

    static int open(const char* path, struct fuse_file_info* fi) {
        // opening both files is permitted
        if (strcmp(path, "/register") == 0) {
            auto pathBuf = new std::vector<char>;
            fi->fh = reinterpret_cast<uint64_t>(pathBuf);
            return 0;
        }

        if (strcmp(path, "/map") == 0 && !(fi->flags & O_RDWR || fi->flags & O_WRONLY)) {
            return 0;
        }

        // opening registered files is permitted as well...
        try {
            auto& registeredAppImage = mapPathToRegisteredAppImage(path);

            // but only if they are opened read-only
            // TODO: check open flags

            // reuse stored fp for file I/O
            fi->fh = static_cast<uint64_t>(registeredAppImage.fd());

            return 0;
        } catch (const CouldNotFindRegisteredAppImageError&) {
            return -ENOENT;
        }
    }

    static int write(const char* path, const char* buf, size_t bufsize, off_t offset, struct fuse_file_info* fi) {
        if (strcmp(path, "/register") != 0)
            return -ENOENT;

        const std::string fragment(buf, bufsize);
        auto* dataBuf = reinterpret_cast<std::vector<char>*>(fi->fh);

        std::copy(fragment.begin(), fragment.end(), std::back_inserter(*dataBuf));

        std::cout << fragment << std::flush;
        return static_cast<int>(bufsize);
    }

    static int truncate(const char* path, off_t) {
        // files doesn't needed to be truncated
        if (strcmp(path, "/register") == 0)
            return 0;

        return -EPERM;
    }

    static int release(const char* path, struct fuse_file_info* fi) {
        if (strcmp(path, "/register") == 0 && (fi->fh != 0)) {
            auto* buf = reinterpret_cast<std::vector<char>*>(fi->fh);

            std::string requestedPath(buf->data(), buf->size());

            while (requestedPath.back() == '\n' || requestedPath.back() == '\r')
                requestedPath.pop_back();

            try {
                registerAppImage(requestedPath);
            } catch (const AppImageAlreadyRegisteredError& e) {
                std::cout << "AppImage already registered: " << requestedPath << std::endl;
            } catch (const AppImageLauncherFSError&) {
                // ignore other errors
            }

            delete buf;
            fi->fh = 0;
        }

        return 0;
    };
};

// static members must be initialized out-of-source, which makes this a bit ugly
int AppImageLauncherFS::PrivateData::counter = 0;
const time_t AppImageLauncherFS::PrivateData::timeOfCreation = time(nullptr);
AppImageLauncherFS::PrivateData::registered_appimages_t AppImageLauncherFS::PrivateData::registeredAppImages;
const char AppImageLauncherFS::PrivateData::REGISTER_MSG[] = "Write paths to AppImages into this virtual file, one per line, to register them\n";

// default constructor
AppImageLauncherFS::AppImageLauncherFS() : d(std::make_shared<PrivateData>()) {}

//
std::shared_ptr<struct fuse_operations> AppImageLauncherFS::operations() const {
    auto ops = std::make_shared<struct fuse_operations>();

    // available functionality
    ops->getattr = d->getattr;
    ops->read = d->read;
    ops->readdir = d->readdir;
    ops->open = d->open;
    ops->write = d->write;
    ops->truncate = d->truncate;
    ops->release = d->release;

    return ops;
}

std::string AppImageLauncherFS::mountpoint() {
    return d->mountpoint;
}

int AppImageLauncherFS::run() {
    // create FUSE state instance
    // the filesystem object encapsules all the functionality, and can generate a FUSE operations struct for us
    auto fuseOps = operations();

    // create fake args containing future mountpoint
    std::vector<char*> args;

    auto mp = mountpoint();

    // check whether another instance is running
    if (d->otherInstanceRunning())
        throw AlreadyRunningError("");

    // make sure mountpoint dir exists over lifetime of this object
    bf::create_directories(mp);
    bf::permissions(mp, static_cast<bf::perms>(d->MOUNTPOINT_MODE));

    // we need a normal char pointer
    std::vector<char> mpBuf(mp.size() + 1, '\0');
    strcpy(mpBuf.data(), mp.c_str());

    // path to this binary is none of FUSE's concern
    args.push_back("");
    args.push_back(mpBuf.data());

    // force foreground mode
    args.push_back("-f");

    // "sort of debug mode"
    if (getenv("DEBUG") != nullptr) {
        // disable multithreading for better debugging
        args.push_back("-s");

        // enable debug output (implies f)
        args.push_back("-d");
    }

    int fuse_stat = fuse_main(static_cast<int>(args.size()), args.data(), fuseOps.get(), this);

    return fuse_stat;
}

std::shared_ptr<AppImageLauncherFS> AppImageLauncherFS::instance = nullptr;

std::shared_ptr<AppImageLauncherFS> AppImageLauncherFS::getInstance() {
    if (instance == nullptr)
        instance = std::shared_ptr<AppImageLauncherFS>(new AppImageLauncherFS);

    return instance;
}
