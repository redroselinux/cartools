#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

/* The 'mkcar' command creates the 'car' metadata file for packages. */

int main(int argc, char* argv[]) {
  std::vector<std::string> deps;
  std::vector<std::string> exec;
  bool interactive = false;
  bool anonymous = false;
  std::string version;
  std::string output;

  // no args, falling back to interactive mode
  if (argc == 1) interactive = true;

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);

    // show help message
    // --help/-h can only be used as the first argument.
    if (i == 1 && (arg == "-h" || arg == "--help")) {
      std::cout << "mkcar [-i (INTERACTIVE MODE)]"
        << std::endl << "      or" << std::endl;;
      std::cout << "mkcar [-v/--version VERSION] pkg version" << std::endl <<
        "      [-d/--dependency DEPENDENCY] deps" << std::endl <<
        "      [-x/--exec COMMAND] postinst" << std::endl <<
        "      [--anonymous] do not share username" << std::endl <<
        "      -o FILE" << std::endl;
      return 0;
    }

    if (arg == "-i" || arg == "--interactive") {
      interactive = true;
    } else if (arg == "-v" || arg == "--version") {
      version = argv[i++];
    } else if (arg == "-d" || arg == "--dependency") {
      deps.push_back(argv[i++]);
    } else if (arg == "--anonymous") {
      anonymous = true;
    } else if (arg == "-o") {
      output = argv[i++];
    }
  }

  if (interactive) {
    if (version.empty()) {
      std::cout << "package version: ";
      std::cin >> version;
    } if (output.empty()) {
      std::cout << "output file: ";
      std::cin >> output;
    } if (deps.empty()) {
      std::string input;
      while (input != ".") {
        std::cout << "[. to end] deps: ";
        std::cin >> input;
        if (input == "") { break; } // skip
      }
    } if (exec.empty()) {
      std::string input;
      while (input != ".") {
        std::cout << "[. to end] postinst cmds: ";
        std::cin >> input;
        if (input == "") { break; } // skip
        exec.push_back(input);
      }
    }
  } else {
    // not in interactive mode so we cannot ask the
    // user if they want to interactively set a var
    if (version.empty() || output.empty()) {
      std::cerr << "\e[91mERROR\e[0m: required args not used: ";
      if (version.empty()) std::cout << "version ";
      if (output.empty()) std::cout << "output";
      std::cout << std::endl;
      std::cerr << "not in interactive mode; exiting" << std::endl;
      return 2;
    }
  }

  // write to output/car if output is a dir
  if (std::filesystem::is_directory(output)) {
    std::string output_new = output + "/car";
    std::cout << output << " is a dir, changing output to"
      << output_new << std::endl;
    output = output_new;
  }

  std::string metadata = "( Auto-generated using cartools mkcar )\n";

  if (anonymous) {
    metadata += "  Anonymous <drop-all@redroselinux.org>\n";
  } else {
    std::string home = std::string(getenv("HOME"));
    std::ifstream file(home + "/.config/redrose-linux-maintainer");
    if (!file.is_open()) {
      std::cerr << "failed to open maintainer file; running" <<
        "cartools-maintainer to save config";

      int result = std::system("cartools-maintainer");
      if (result == 1) {
        return 1;
      } else if (result == 127) {
        std::cerr << "have you installed the full cartools pkg?" << std::endl;
        std::cerr << "(cartools-maintainer is missing)" << std::endl;
      }

      // re-open file
      file.clear();
      file.open(home + "/.config/redrose-linux-maintainer");
      if (!file.is_open()) {
        std::cerr << "still failed to read config. exiting" << std::endl;
        return 1;
      }
    }

    // append content's first line to metadata
    std::string content;
    std::getline(file, content);
    metadata += content + "\n\n";

    file.close();
  }

  // add deps
  for (std::string package : deps) {
    if (package == ".") continue;
    metadata += "dep " + package + "\n";
  }

  // add exec's
  for (std::string cmd : exec) {
    if (cmd == ".") continue;
    metadata += "exec " + cmd + "\n";
  }

  // and finally, add version
  metadata += "version " + version + "\n";

  // open output and write metadata to it
  std::ofstream mfile(output);
  if (!mfile.is_open()) {
    std::cerr << "failed to open output file" << std::endl;
    return 1;
  }
  mfile << metadata;
  mfile.close();

  std::cout << "finished" << std::endl;
  return 0;
}
