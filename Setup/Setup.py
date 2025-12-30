import os
import shutil
import logging
from pathlib import Path



IS_WINDOWS = os.name == "nt" # We're on windows
PREMAKE_PATH = Path()
EDITOR_PATH = Path()
PROJECT_PATH = Path()
SDL_PATH = Path()



def Find_Premake_Path() -> bool:
    global PREMAKE_PATH
    premake = shutil.which('premake5')
    logging.debug(f"Checking if {premake} is legit?")
    
    if premake:
        PREMAKE_PATH = Path(premake)
        logging.info(f"Premake found at {PREMAKE_PATH.absolute()}")
        return True
    
    else:
        logging.warning(f"Premake was NOT found in PATH (shutil.which)\nUsing local instead")
        
        if IS_WINDOWS:
            PREMAKE_PATH = Path(__file__).parent / "Windows/Binaries/premake5.exe"
        else:
            PREMAKE_PATH = Path(__file__).parent / "Linux/Binaries/premake5"
                        
        if PREMAKE_PATH.is_file():
            logging.info(f"Using local Premake at {PREMAKE_PATH.absolute()}")
            return True
        else:
            logging.error(f"WE CAN'T EVEN FIND OUR OWN LOCAL PREMAKE!\n{PREMAKE_PATH.absolute()} isn't legit!\nCÁC")
            return False



def Find_Core_Path() -> bool:
    global EDITOR_PATH
    global PROJECT_PATH
    
    logging.debug(f"trying to find Build.lua in {Path(__file__).absolute()}\nparent-2 is {Path(__file__).parent.parent.absolute()}")
    
    PROJECT_PATH = Path(__file__).parent.parent
    
    luaBuild = PROJECT_PATH / "premake5.lua"
    
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
    
    
    
def main():
    global PREMAKE_PATH 
    global EDITOR_PATH
    
    if (Find_Premake_Path() and Find_Core_Path()) == False:
        logging.error("Some crap went down!")
        
    else:
        command = f"\"{PREMAKE_PATH.absolute()}\" --file=\"{EDITOR_PATH.absolute()}\" vs2022"
        
        if not IS_WINDOWS: logging.warning(f"We don't know if {command} will run properly on linux, you've been warned!")
        
        logging.info(f"All seems good so we're going to run {command}")
        logging.info("Output is as follows:")
        logging.info(os.popen(command).read())
        


if __name__ == "__main__":
    if IS_WINDOWS == False:
        print("""   HEY! 
            I HAVEN'T TESTED THIS ON LINUX OR MAC
            ...Just fyi!
            """)
        
    logging.basicConfig(level=logging.DEBUG,
                        format="[%(levelname)s]\t: %(message)s",
                        handlers=[
                        logging.FileHandler(Path(Path(__file__).parent / "Build attempt.log").absolute()),
                        logging.StreamHandler()
                    ], force=True)

    logging.info(f"Running Setup using { "Windows" if IS_WINDOWS else "BASH" } setup")
    main()
    print("Done :)")

