

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/shared_ptr.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef std::string Sha;

Sha computeSha(fs::path path)
{
  return "sha";
}

class File : public fs::path
{
public:

  File(fs::path path) : fs::path(path)
  {
    m_future = boost::async(boost::launch::async, boost::bind(computeSha, path));
  }

  Sha sha()
  {
    m_future.wait();

    return m_future.get();
  }

  typedef boost::shared_ptr<File> Ptr;

private:
  typedef boost::unique_future<Sha> Future;
  
  Future m_future;
};

class Directory
{
public:
  Directory(fs::path path)
  {
    for (fs::directory_iterator end, iter(path); iter != end; ++iter)
      {
	fs::directory_entry entry(*iter);
	fs::path p(entry.path());
	
	if (p.filename() == ".verisum")
	  continue;

	if (fs::is_directory(p))
	  m_subdirs.push_back(Directory(p));
	else if (fs::is_regular_file(p))
	  {
	    m_files.push_back(File::Ptr(new File(p)));
	  }
      }
  }

  void verify()
  {
    for (auto iter : m_files)
      {
	std::cout << "Verify " << *iter << "=" << iter->sha() << std::endl;
      }

    for (auto iter : m_subdirs)
      iter.verify();
  }

  void generate()
  {
    for (auto iter : m_files)
      {
	
      }

    for (auto iter : m_subdirs)
      iter.generate();
  }

private:
  typedef std::list<Directory> Directories;
  typedef std::list<File::Ptr>  Files;

  Directories m_subdirs;
  Files       m_files;
};

int main(int argc, char **argv)
{
  int ret;
  fs::path path(fs::current_path());
  bool generate(false);
  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produces help message")
    ("path,p", po::value<fs::path>(&path),
     "path to tree (defaults to $CWD)")
    ("generate", "generate sums")
    ;
  
  po::positional_options_description p;
  p.add("path", -1);
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cerr << desc << std::endl;
      return -1;
    }

  if (!fs::is_directory(path))
    {
      std::cerr << "Error: " << path << " is not a directory" << std::endl;
      return -1;
    }

  if (vm.count("generate"))
      generate = true;

  std::cout << (generate ? "Generating" : "Verifying") << " sums in " << path << std::endl;

  Directory root(path);

  if (generate)
    root.generate();
  else
    root.verify();
}
