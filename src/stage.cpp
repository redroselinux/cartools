#include <iostream>
#include <filesystem>
#include <string>
#include <cstdlib>
#include <cstring>

/* The 'stage' command helps you easily stage a package's content
 * and prepare it for package creation done with the 'unstage'
 * command [src/pkg-stage.cpp] */


/* The StageDir class manages the staging directory and its creation.
 * Arguments:
 *   string package_name - name of the package used as the name
 *                         of the staging directory. */
class StageDir {
  protected:
    std::string path;

    /* Tiny helper to create the staging dir and return its path.
     * Arguments:
     *   string package_name - the package name used as the dir
     *                         to stage the package in.
     *   string extra        - dir(s) to create after the staging. */
    static std::string create_path(std::string package_name, std::string extra) {
      auto wdir = std::filesystem::current_path() / package_name;
      try {
        std::filesystem::create_directories(wdir);
        if (!extra.empty()) {
          std::filesystem::create_directories(wdir / extra);
        }
      } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create staging dir:"
          << std::endl << e.what() << std::endl;
      }
      return wdir.string();
    }

  public:
    StageDir(std::string package_name, std::string extra = "")
    : path(create_path(package_name, extra)) {}

    /* A function to get the staging directory path. */
    std::string get_path() {
      return path;
    }
};

/* Ncurses-style prefix type stage dir.*/
class NcursesStyleStageDir : public StageDir {
  public:
    NcursesStyleStageDir(std::string package_name)
      : StageDir(package_name, "") {
      auto wdir = std::filesystem::current_path() / package_name / "usr";
      try {
        std::filesystem::create_directories(wdir);
        path = wdir.string();
      } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create staging dir:"
          << std::endl << e.what() << std::endl;
      }
    }
};

class BuildSystemInstall {
  protected:
    const std::string command;
    const std::string stage_dir;
  public:
    /* Tiny helper to initialize the StageDir class, and return the
     * path of it. It is used in the init list of this class.
     *
     * Arguments:
     *   string package_name - the package name (again) */
    static std::string get_stage_dir(std::string package_name) {
      StageDir stage(package_name);
      return stage.get_path();
    }

    BuildSystemInstall(std::string package_name)
    : stage_dir(get_stage_dir(package_name)) {}

    /* Helper to execute the */
    int execute() {
      return std::system(command.c_str());
    }
};

#define CREATE_BUILDSYSTEM_CLASS(Name, Cmd) \
  class Name : public BuildSystemInstall { \
    public: \
      using BuildSystemInstall::BuildSystemInstall; \
      int execute() { \
        return std::system((std::string(Cmd) + stage_dir).c_str()); \
      } \
  };

CREATE_BUILDSYSTEM_CLASS(MakeInstall, "make install DESTDIR=");
// In ncurses, 'DESTDIR' is swapped with 'prefix'
class MakeNcursesStyleInstall : public BuildSystemInstall {
  protected:
    NcursesStyleStageDir stage_dir;
  public:
    MakeNcursesStyleInstall(std::string package_name)
      : BuildSystemInstall(package_name), stage_dir(package_name) {}
    int execute() {
      std::string path = stage_dir.get_path();
      return std::system((std::string("make install prefix=") + path + " TICDIR=" + path + "/share/terminfo").c_str());
    }
};
CREATE_BUILDSYSTEM_CLASS(CMakeInstall, "cmake --install . --prefix=")
CREATE_BUILDSYSTEM_CLASS(MesonInstall, "meson install --destdir=");
CREATE_BUILDSYSTEM_CLASS(CargoInstall, "cargo install --root=");
CREATE_BUILDSYSTEM_CLASS(PipInstall, "pip install --prefix=");
CREATE_BUILDSYSTEM_CLASS(QMakeInstall, "make install INSTALL_ROOT=");
CREATE_BUILDSYSTEM_CLASS(SConsInstall, "scons install DESTDIR=");
CREATE_BUILDSYSTEM_CLASS(WafInstall, "waf install --destdir=");
CREATE_BUILDSYSTEM_CLASS(NpmInstall, "npm install --prefix=");
CREATE_BUILDSYSTEM_CLASS(NinjaInstall, "ninja install DESTDIR=");

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "\e[91mERROR\e[0m: required 2 commands" << std::endl;
    return 2;
  } else {
    std::string system(argv[1]);
    std::string name(argv[2]);

    if (argc > 3) {
      for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-C")) {
          std::filesystem::current_path(argv[i++]);
        }
      }
    }

    if (system == "make") {
      MakeInstall make(name);
      make.execute();
    } else if (system == "make-ncurses") {
      MakeNcursesStyleInstall make(name);
      make.execute();
    } else if (system == "cmake") {
      CMakeInstall cmake(name);
      cmake.execute();
    } else if (system == "meson") {
      MesonInstall meson(name);
      meson.execute();
    } else if (system == "cargo") {
      CargoInstall cargo(name);
      cargo.execute();
    } else if (system == "pip") {
      PipInstall pip(name);
      pip.execute();
    } else if (system == "qmake") {
      QMakeInstall qmake(name);
      qmake.execute();
    } else if (system == "scons") {
      SConsInstall scons(name);
      scons.execute();
    } else if (system == "waf") {
      WafInstall waf(name);
      waf.execute();
    } else if (system == "npm") {
      NpmInstall npm(name);
      npm.execute();
    } else if (system == "ninja") {
      NinjaInstall ninja(name);
      ninja.execute();
    } else if (system == "-h" || system == "--help") {
      std::cout << "stage <build system> <name> [-C DIR]" << std::endl;
      return 0;
    } else {
      std::cerr << "\e[91mERROR\e[0m: unknown build system: " << system << std::endl;
      return 1;
    }
  }
}
