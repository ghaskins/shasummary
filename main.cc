

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, char **argv)
{
  int ret;
  fs::path path(fs::current_path());
  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produces help message")
    ("path,p", po::value<fs::path>(&path),
     "path to tree (defaults to $CWD)")
    ("generate", "generate sums")
    ("verify", "verify sums")
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

}
