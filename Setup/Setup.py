import os
import shutil
from pathlib import Path
import logging


IS_WINDOWS = os.name == 'nt' # We're on windows
PREMAKE_PATH = Path()
EDITOR_PATH = Path()
PROJECT_PATH = Path()
SDL_PATH = Path()

def findPremakePath() -> bool:
    global PREMAKE_PATH
    premake = shutil.which('premake5')
    logging.debug(f"Checking if {premake} is legit?")
    if premake:
        PREMAKE_PATH = Path(premake)
        logging.info(f"Premake found at {PREMAKE_PATH.absolute()}")
        return True
    else:
        logging.warning(f"Premake was NOT found in PATH (shutil.which)\nUsing local instead")
        PREMAKE_PATH = Path(__file__).parent / "Binaries/premake5.exe"
        if PREMAKE_PATH.is_file():
            logging.info(f"Using local Premake at {PREMAKE_PATH.absolute()}")
            return True
        else:
            logging.error(f"WE CAN'T EVEN FIND OUR OWN LOCAL PREMAKE!\n{PREMAKE_PATH.absolute()} isn't legit!\nCÁC")
            return False

def findEditorPath() -> bool:
    global EDITOR_PATH
    global PROJECT_PATH
    
    logging.debug(f"trying to find Build.lua in {Path(__file__).as_uri()}\nparent-2 is {Path(__file__).parent.parent.as_uri()}")
    PROJECT_PATH = Path(__file__).parent.parent
    luaBuild = PROJECT_PATH / "Build.lua"
    logging.debug(f"Attempting to find lua build at {luaBuild.absolute()}")
    logging.debug(f"Lua build is file: {luaBuild.is_file()}")
    if luaBuild.is_file():
        logging.info(f"Found build file for Editor at {luaBuild.absolute()}")
        EDITOR_PATH = luaBuild
        return True
    else:
        logging.error("build_editor.lua was NOT found where we thought it'd be... crap")
        logging.error("You'll have to run 'premake5.exe --file=<path to build_editor.lua> vs2022'")
        return False
    
    
def win_findDevTool() -> Path:
    possiblePaths = [
        "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\Visual Studio 2022\\Visual Studio Tools\\Developer Command Prompt for VS 2022.lnk",
        ]
    
    for file in possiblePaths:
        if Path(file).is_file():
            logging.info(f"Dev tools found at {file}")
            return Path(file)
    logging.error("Crap, we didnt find the ms build tools")
    return Path()


def findSDLPath():
    global SDL_PATH
    global PROJECT_PATH
    
    SDL_PATH = PROJECT_PATH / "SegFault Editor/Vendor/SDL"


# TODO: Automate building and linking SDL
def buildSDL():
    global EDITOR_PATH
    global PROJECT_PATH
    global SDL_PATH
    
    try:
        if PROJECT_PATH.is_dir() == False:
            PROJECT_PATH = Path(__file__).parent.parent
    except TypeError:
        logging.error("EditorPath was never defined... wtf??")
        
    logging.debug(f"ProjectDir is {PROJECT_PATH.absolute()}")
    
    win_tools_path = win_findDevTool()
    msbuild_args = [
        f"/p:Configuration=Release",
        f"/p:Platform=x64",
        "/m", # parallel build
        "/nologo",
        "/verbosity:minimal",
    ]
    
    findSDLPath()
    sol_path = SDL_PATH / "VisualC/SDL.sln"
    command = f"\"{win_tools_path.absolute()}\" \"{sol_path.absolute()}\" {" ".join(msbuild_args)}"
    logging.info(f"Try to run command: {command}")
    logging.info(f"When we try to build shit we got:\n{os.popen(f"{command}").read()}")
    pass 
    

# TODO: Automate copying the .lib and .dll from SDL3
def copySdlLib():
    pass


def main():
    global PREMAKE_PATH 
    global EDITOR_PATH
    
    if (findPremakePath() and findEditorPath()) == False:
        logging.error("Some crap went down!")
    else:
        command = f"\"{PREMAKE_PATH.absolute()}\" --file=\"{EDITOR_PATH.absolute()}\" vs2022"
        if not IS_WINDOWS: logging.warning(f"We don't know if {command} will run properly on linux, you've been warned!")
        logging.info(f"All seems good so we're going to run {command}")
        logging.info("Output is as follows:")
        logging.info(os.popen(command).read())
        
    #buildSDL()
    
    

if __name__ == "__main__":
    if IS_WINDOWS == False:
        print("""   HEY! 
            I HAVEN'T TESTED THIS ON LINUX OR MAC
            ...Just fyi!
            """)
    logging.basicConfig(level=logging.DEBUG,
                        format="[%(levelname)s]\t: %(message)s",
                        handlers=[
                        logging.FileHandler("Build attempt.log"),
                        logging.StreamHandler()
                    ], force=True)
    logging.info(f"Running Setup using { "Windows" if IS_WINDOWS else "BASH" } setup")

    main()
    #buildSDL()
    print("Done :)")
