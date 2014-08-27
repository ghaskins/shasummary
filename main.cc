#include <queue>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/iostreams/device/file.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

typedef std::string Sha;
typedef boost::packaged_task<Sha> Task;
typedef boost::shared_ptr<Task> TaskPtr;

class ThreadPool
{
public:
  ThreadPool()
  {
    int concurrency(boost::thread::hardware_concurrency());

    std::cerr << "Using " << concurrency << " threads" << std::endl;

    for (int i(0); i<concurrency; i++)
      {
	m_tg.create_thread(boost::bind(&ThreadPool::worker, this));
      }
  }

  ~ThreadPool()
  {
    m_tg.interrupt_all();
    m_tg.join_all();
  }

  void post(TaskPtr task)
  {
    Lock lock(m_mutex);

    m_tasks.push(task);
    m_condition.notify_one();
  }
  
private:
  void worker()
  {
    while (1)
      {
	Lock lock(m_mutex);
	
	while(m_tasks.empty())
	  m_condition.wait(lock);

	TaskPtr task(m_tasks.front());
	m_tasks.pop();

	(*task)();
      }
  }

  typedef boost::mutex              Mutex;
  typedef boost::unique_lock<Mutex> Lock;
  typedef boost::condition_variable Condition;
  typedef std::queue<TaskPtr>       Tasks;

  boost::thread_group m_tg;
  Mutex               m_mutex;
  Condition           m_condition;
  Tasks               m_tasks;
};

ThreadPool _pool;

class File : public fs::path
{
public:

  File(fs::path path) : fs::path(path)
  {
    TaskPtr task(new Task([path]()->Sha
			  {
			    using boost::uuids::detail::sha1;
			    
			    boost::iostreams::file_source is(path.string());
			    sha1                          sha1;
			    char                          buf[4096];
			    std::streamsize               len;
			    
			    while ((len = is.read(buf, sizeof(buf))) > 0)
			      sha1.process_bytes(buf, len);
			    
			    unsigned int digest[5];
			    
			    sha1.get_digest(digest);

			    std::ostringstream os;

			    os << std::hex << std::setfill('0') << std::setw(sizeof(int)*2);
			    for(std::size_t i=0; i<sizeof(digest)/sizeof(digest[0]); ++i) {
			      os << digest[i];
			    }

			    return os.str();
			  }
			  )
		 );
		 
    m_future = task->get_future();
    _pool.post(task);
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

#define METADIR ".verisum"

Sha loadSha(fs::path path)
{
  std::ifstream is(path.string().c_str());
  Sha sha;

  is >> sha;

  return sha;
}

class Directory
{
public:
  Directory(fs::path path)
  {
    for (fs::directory_iterator end, iter(path); iter != end; ++iter)
      {
	fs::directory_entry entry(*iter);
	fs::path p(entry.path());

	if (p.filename() == METADIR)
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
    fs::path metadir(METADIR);
    std::set<std::string> files;

    for (auto iter : m_files)
      {
	fs::path metafile(metadir / iter->filename());

	if (fs::exists(metafile))
	  {
	    if (loadSha(metafile) != iter->sha())
	      std::cout << "U\t" <<  *iter << std::endl;
	  }
	else
	  {
	    std::cout << "C\t" <<  *iter << std::endl;
	  }

	files.insert(iter->filename().string());
      }

    if (fs::exists(metadir))
      {
	for (fs::directory_iterator end, iter(metadir); iter != end; ++iter)
	  {
	    fs::directory_entry entry(*iter);
	    fs::path p(entry.path());
	    
	    if (files.find(p.filename().string()) == files.end())
	      std::cout << "D\t" <<  *iter << std::endl;
	  }
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

  std::cerr << (generate ? "Generating" : "Verifying") << " sums in " << path << std::endl;

  Directory root(path);

  if (generate)
    root.generate();
  else
    root.verify();
}
