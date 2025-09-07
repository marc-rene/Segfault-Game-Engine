import os
import shutil
from pathlib import Path
import logging


IS_WINDOWS = os.name == 'nt' # We're on windows
PREMAKE_PATH = Path()
EDITOR_PATH = Path()

def findPremakePath() -> bool:
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
    luaBuild = Path(__file__).parent.parent / "SegFault Editor/build_editor.lua"
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
    if IS_WINDOWS == False:
        print("""   HEY! 
            I HAVEN'T TESTED THIS ON LINUX OR MAC
            ...Just fyi!
            """)
    logging.basicConfig(level=logging.INFO,
                        format="[%(levelname)s]\t: %(message)s")
    
    logging.info(f"Running Setup using { "Windows" if IS_WINDOWS else "BASH" } setup")
    
    if (findPremakePath() and findEditorPath()) == False:
        logging.error("Some crap went down!")

    
    

if __name__ == "__main__":
    main()
    print("Done :)")
