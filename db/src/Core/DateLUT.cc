#include<Core/DateLUT.h>
#include<Ext/std_ext.h>
#include<Poco/Exception.h>
#include<Poco/DigestEngine.h>
#include<Poco/SHA1Engine.h>
#include<Poco/DigestStream.h>
#include<fstream>
#include<cstdlib>
#include<experimental/filesystem>
namespace DataBase {

static Poco::DigestEngine::Digest calcSHA1(const std::string & path)
{
    std::ifstream stream(path);
    if (!stream)
        throw Poco::Exception("Error while opening file: `" + path + "'.");
    Poco::SHA1Engine digest_engine;
    Poco::DigestInputStream digest_stream(digest_engine, stream);
    digest_stream.ignore(std::numeric_limits<std::streamsize>::max());
    if (!stream.eof())
        throw Poco::Exception("Error while reading file: `" + path + "'.");
    return digest_engine.digest();
}

static inline   std::pair<std::experimental::filesystem::path::iterator, std::experimental::filesystem::path::iterator> mismatch(std::experimental::filesystem::path::iterator it1,
        std::experimental::filesystem::path::iterator it1end, std::experimental::filesystem::path::iterator it2, std::experimental::filesystem::path::iterator it2end)
{
    for (; it1 != it1end && it2 != it2end && *it1 == *it2;)
    {
        ++it1;
        ++it2;
    }
    return std::make_pair(it1, it2);
}

static std::experimental::filesystem::path lexically_relative(const std::experimental::filesystem::path& path,const std::experimental::filesystem::path& base) 
{
    std::pair<std::experimental::filesystem::path::iterator, std::experimental::filesystem::path::iterator> mm
        = mismatch(path.begin(), path.end(), base.begin(), base.end());
    if (mm.first ==path. begin() && mm.second == base.begin())
        return std::experimental::filesystem::path();
    if (mm.first == path.end() && mm.second == base.end())
        return std::experimental::filesystem::path(".");
    std::experimental::filesystem::path tmp;
    for (; mm.second != base.end(); ++mm.second)
        tmp /= std::experimental::filesystem::path(".");
    for (; mm.first != path.end(); ++mm.first)
        tmp /= *mm.first;
    return tmp;
}

static std::string determineDefaultTimeZone()
{
    const char * tzdir_env_var = std::getenv("TZDIR");
    std::experimental::filesystem::path tz_database_path = tzdir_env_var ? tzdir_env_var : "/usr/share/zoneinfo/";

    std::experimental::filesystem::path tz_file_path;
    std::string error_prefix;
    const char * tz_env_var = std::getenv("TZ");
    if (tz_env_var)
    {
        error_prefix = std::string("Could not determine time zone from TZ variable value: `") + tz_env_var + "`: ";

        if (*tz_env_var == ':')
            ++tz_env_var;

        tz_file_path = tz_env_var;
    }
    else
    {
        error_prefix = "Could not determine local time zone: ";
        tz_file_path = "/etc/localtime";
    }

    try
    {
        tz_database_path = std::experimental::filesystem::canonical(tz_database_path);
        tz_file_path = std::experimental::filesystem::canonical(tz_file_path, tz_database_path);
        std::experimental::filesystem::path relative_path = lexically_relative(tz_file_path,tz_database_path);
        if (!relative_path.empty() && *relative_path.begin() != ".." && *relative_path.begin() != ".")
            return relative_path.native();
        size_t tzfile_size = std::experimental::filesystem::file_size(tz_file_path);
        Poco::SHA1Engine::Digest tzfile_sha1 = calcSHA1(tz_file_path.native());
        std::experimental::filesystem::recursive_directory_iterator begin(tz_database_path);
        std::experimental::filesystem::recursive_directory_iterator end;
        for (std::experimental::filesystem::recursive_directory_iterator candidate_it = begin; candidate_it != end; ++candidate_it)
        {
            const  std::experimental::filesystem::path & path = candidate_it->path();
            if (path.filename() == "posix" || path.filename() == "right")
            {
                /// Some timezone databases contain copies of toplevel tzdata files in the posix/ directory
                /// and tzdata files with leap seconds in the right/ directory. Skip them.
				candidate_it.disable_recursion_pending();
                continue;
            }

            if (candidate_it->status().type() != std::experimental::filesystem::v1::file_type::regular || path.filename() == "localtime")
                continue;

            if (std::experimental::filesystem::file_size(path) == tzfile_size && calcSHA1(path.native()) == tzfile_sha1)
                return lexically_relative(path,tz_database_path).native();
        }
    }
    catch (const Poco::Exception & ex)
    {
        throw Poco::Exception(error_prefix + ex.message(), ex);
    }
    catch (const std::exception & ex)
    {
        throw Poco::Exception(error_prefix + ex.what());
    }

    throw Poco::Exception(error_prefix + "custom time zone file used.");
}



DateLUT::DateLUT()
{
/// Initialize the pointer to the default DateLUTImpl.
    std::string default_time_zone = determineDefaultTimeZone();
    default_impl.store(&getImplementation(default_time_zone), std::memory_order_release);
}


const DateLUTImpl& DateLUT::getImplementation(const std::string& time_zone) const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = impls.emplace(time_zone, nullptr).first;
    if (!it->second)
        it->second = std_ext::make_unique<DateLUTImpl>(time_zone);
    return *it->second;
}


}

